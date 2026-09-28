# Compositions A–I — the short list, the expansion, and what is still broken

Companion to `COMPOSITION_STATUS.md` (full status), `COMPOSITION_R3_PRIVATIZATION.md`
(B/C/D/extra mechanism), `COMPOSITION_AH_ARS1_SOLUTION.md` (A, H),
`COMPOSITION_EGI_PROBLEM_AND_SOLUTION.md` (E, G, I).
State: branch `compositions-round3`, HEAD `f83fe16`, suite **85 pass / 0 fail**.

---

## 1. The short list (one line each)

| # | Problem, in one line | Status, in one line |
|---|---|---|
| **A** | Same array read on one endpoint and written on the other, in place (`dist[v] = min(dist[v], dist[u]+w)`) — a naive parallel write races a sibling's read | **Solved-general** — per-round shadow snapshot of the array; cross-endpoint reads see a frozen round-start value |
| **A-dual** | Dual-owner loop (claim/activate state machine, `small_kcore`) whose cross-region reads need a shadow | **Not fixed** — refused (fail closed); needs a per-*read* temporal model |
| **B** | Write at a *data-valued* subscript (`cnt[deg[u]] += 1`) — the "house" is a data value, not a vertex, so the owner table has nothing to key on | **Solved-general (R3)** — privatization: per-partition copies + ordered fold |
| **C** | One array written through *both* endpoint regions (`arr[u] += 1; arr[v] += 1`) — no single region owns it | **Solved-general (R3)** — same privatization rule |
| **D** | Reduction next to a write in the *driver preamble* (`w[u] = w[u]+1` once per source; `acc += …` once per pair) | **Solved-general (R3+D)** — privatization + a per-source preamble phase in the runtime |
| **E** | Cross-phase dependence between the two regions of a dual-owner loop (BFS-style `lvl[u]` read vs `lvl[v]` write) | **Solved-general** — dependences proven to be `Control ∪ Membership` only; regions disjoint |
| **F** | First-wins claim in the driver *preamble* gating the neighbour body — work fns run it once per pair and drop the guard | **Not fixed** — refused by a derived rule; the gating semantics are unmodelled |
| **G** | Scalar-global reductions with exotic operators (signed/unsigned min/max, `minnum`/`minimum`, float `select`) | **Mostly solved-general** — per-flavour combine clones the body's op; **raw float `select` min/max still not fixed** (refused) |
| **H** | Reduction emitted (partials + combine) but *never validated* against the serial fold | **Solved-general** — `validate_reduction.sh`, 24-config matrix vs an independent Python fold; op tagged from the IR |
| **I** | Array-frontier / per-source gathers (`c=0; for each neighbor v { c += f(v) } deg[u]=c`) — the accumulator has no home in a per-pair work fn | **Solved-general behind a switch** (`SGPL_COMP_I_SOURCE_REDUCTION=1`); **default is still the refusal** |
| **extra** | Two scalar accumulators in one body (`a += 1; b += 2`) — the legacy engine stores per-partition state for one slot only | **Solved-general (R3)** — every slot gets private storage |

Reading the status column: "solved-general" = verdict and emitted code derived
from the loop's effect set (no name/shape list); "refused" = a derived, tested
refusal that says why — a legitimate outcome, not a solution.

---

## 2. The expansion

### A — round-separated in-place update

- **Problem.** `dist[v] = min(dist[v], dist[u]+w)` reads `u`'s side and writes
  `v`'s side of the *same* array. A destination partition writing `dist[v]` is
  concurrent with another partition, from an earlier round, reading `dist[u]`;
  the read must observe a frozen round-start value, not a live one. Blind
  sequentialisation would throw the parallelism away for no reason.
- **Solution (ARS-1 push `4f0dc70`, +302 lines in `graph_frontier_lowering.cpp`).**
  The classifier recognises the effect shape — same base carrying a read and a
  mutating write, single-region writes, read index originating in the *opposite*
  region, uniform `i32`/`double` (no algorithm name). It then emits a per-round
  `memcpy` of the destination region into a scratch shadow
  (`autograph_scratch_shadow`, declared/wired in `autotuner_runtime.{c,h}`) in
  the round preheader, and rewrites every *cross-endpoint* read to load through
  the shadow. Same-region reads stay live, so a `min`/`+=` relax is still a real
  accumulator.
- **Why it's general.** Change the operator (`min` → `+` → SSSP relax) and the
  shadow still applies; move the read into the same region and the shadow stops
  applying. Nothing is matched by name.
- **Evidence.** `race/roundsep`: `shadow=1 class=dest-owner`, answer identical
  at 1 vs 4 threads × 1/3/4 partitions, equal to the independent
  `compute_roundsep_expected.py`; `validate_roundsep.sh` PASS (min/add/sssp).
- **Caveat (see §3).** Implements *frozen-round* semantics, which differs from a
  serial build for genuinely in-place loops (serial overflows int32).

### A-dual — the dual-owner variant (not fixed)

- **Problem.** Upstream's `small_kcore` shape: a claim/activate state machine
  whose cross-region reads would need the shadow inside a dual-owner step.
- **What happened.** Two defects were found and one was fixed: (1)
  `emitDualCleanCut()` never called `emitRoundSepShadow()`, so the object failed
  to link (`undefined reference to 'main.alive.shadow.0'`) and the validator
  reported a spurious mismatch; fixed by emitting the shadow once per round
  before both phases. (2) With the link fixed the answer was **still wrong**:
  18898 survivors vs 18959 for serial and an independent 10-core Python count —
  the frozen snapshot let a vertex already killed earlier in the round read
  `alive[v] == 1` and decrement its neighbours, over-peeling 61 vertices.
- **Current handling.** Fail closed: `Info.RoundSepBases` non-empty disqualifies
  `Klass::DualOwner`, so the loop is `class=sequential` and yields 18959 at 1 and
  4 threads (`race/dual_shadow`).
- **What a fix needs.** A per-*read* temporal model — classify each read as
  round-separated or within-round and shadow only the former. A shape list
  cannot do it, because the *same* array carries both kinds.

### B — data-valued subscript

- **Problem.** `cnt[deg[u]] = cnt[deg[u]] + 1`: the destination house is a data
  value, not a vertex id, and CleanCut's owner table (`CC_PART_OF`) is keyed by
  destination vertex — there is no owner to steal work from.
- **Solution (R3).** Privatization (see §2.1): the loop's every mutating effect
  is a recognised `U_⊕` update, so each partition gets a private copy of `cnt`
  and the copies are folded with `+` at the end. No owner is ever named.
- **Evidence.** `data_index_write`: `privatized`, `cntsum 10` at 1/4 threads,
  byte-identical to the rewrite-off build; `priv_data_index_big`:
  `cntsum 160000 cnt1 1335` (# vertices with out-degree 1 — catches lost/double
  counting).

### C — one base written through both regions

- **Problem.** `arr[u] += 1; arr[v] += 1` writes the same base from both
  endpoint regions; `DualOwner` requires disjoint region bases, so this used to
  be refused (`class=sequential`, `tot 340000`).
- **Solution (R3).** Same privatization rule — the fold does not care which
  partition applied which update, so no region needs to own `arr`.
- **Evidence.** `mixed_regions`: `privatized`, `tot 340000` = n + 2m, 1/4
  threads, serial-equality ✅.

### D — reduction next to a driver-preamble write

- **Problem.** `w[u] = w[u] + 1` runs once per **source** (driver preamble),
  while `acc = acc + …` runs once per **pair** (neighbour body). A per-pair step
  cannot carry the preamble — running it per pair would turn `w[u] += 1` into
  "+1 per arc" (`w0` would end at the source's out-degree, `sum(w)` at the arc
  count).
- **Solution (R3+D).** Privatization plus a second work function (the
  `PairPhase::UOnly` clone the dual-owner path already uses) and a new runtime
  entry `autograph_frontier_step_owner_red_pre()` that walks each partition's
  CleanCut source slices and invokes the preamble **exactly once per source**,
  including sources with no arcs, which the serial program also visits. Both
  phases write only into the partition's private state — no ownership anywhere.
- **Evidence.** `reduce_plus_write`: `privatized`, `acc 160000 w0 1`;
  `priv_reduce_write_big`: `wsum 20000` (= Σw = n).  Both 1/4 threads,
  serial-equality ✅.

### E — cross-phase dependence in a dual-owner loop

- **Problem.** In a dual-owner step the body reads values written by a
  neighbour in the same round (BFS reads `lvl[u]` while writing `lvl[v]`); naive
  DOALL is a race.
- **Solution.** `DualOwner = EU ∪ EV` over two disjoint bases, and the classifier
  proves `Dependence ⊆ Control ∪ Membership` — the only legal relations. The
  destination is claimed first (`Claim(visited,V)`), so a source can only ever
  observe a destination that was already claimed; `Activate(next_frontier,V)` is
  the only cross-region write and is itself owner-computes (single writer per
  destination). Reads are labelled `SameRoundRead` (stay live) vs
  `PreviousRoundRead` (shadow-eligible).
- **Evidence.** `race/bfs_level` is race-clean, `class=dest-owner`, effect string
  `R(visited,V):SameRoundRead ⊗ Claim(visited,V) ⊗ R(lvl,U):PreviousRoundRead ⊗
  W(lvl,V) ⊗ Activate(next_frontier,V)`, verified at 1/3/4 partitions.

### F — driver-preamble claim (not fixed)

- **Problem.** `if (claim[u] == 1) { claim[u] = 0; <neighbour loop> }` — the
  guard decides whether a source's body runs, and the serial program visits each
  source **once**. Every emitted work function is called once per **(u,v) pair**
  (the `autograph_*_owner_*` bodies), so the claim would run per pair and the
  body would run for sources whose claim failed.
- **Before the fix the verdict was an accident:** the classifier said
  `class=dual-owner fw=1 Claim(claim,U)` and `emitDualCleanCut()` returned false,
  so the fall-through `markSequential()` kept it serial — right answer, wrong
  reason.
- **Current handling.** Refusal moved into the classifier (`HasDriverClaim`),
  kill switch `SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1`; emit failures now print
  `emit failed for class=… -> stays sequential`, so a silent fallback can no
  longer masquerade as a classification. `race/claim_driver`: only u∈{0,1} claim
  → exactly 5 of tiny.txt's 10 arcs may be processed; asserts `class=sequential`,
  the serial answer, 1thr == 4thr.
- **What a fix needs.** Prove the driver visits each source once (or that the
  claim is idempotent and its result unused), or gate the body from a per-source
  pre-pass.  Note the runtime now *has* a per-source phase (built for D), but the
  claim's **gating** is what is unmodelled — adjacent machinery, not a fix.

### G — exotic reduction operators

- **Problem.** `RedOp` collapsed `smin/umin/minnum/minimum` into one `Min`, so an
  unsigned minimum was combined with a *signed* one (wrong identity: INT_MAX vs
  UINT_MAX) and a NaN-propagating `minimum` with `minnum` — silently wrong
  numbers even when race-free.  Also: the raw float `select(fcmp olt/ogt)` the
  DSL's `min()`/`max()` builtins emit is not reorder-invariant with NaN/±0.
- **Solution.** Each form now has its own flavour; the combine reproduces the
  body's own operation exactly, signedness taken from the `icmp` predicate
  (`ICmpInst::isUnsigned`) in the `select(icmp)` and guarded-store recognisers;
  identity per flavour (`identityFor`).  There is no operator table to extend —
  the op is read off the IR.
- **Evidence.** `race/reduce_int_ops` (7 operators, guarded and `min()`/`max()`
  forms), `race/reduce_real_ops` (float sum parallel, both float min/max pinned
  as refusals), `validate_reduction.sh` 24-config matrix.
- **Not fixed.** The float `select` residue — see §3.

### H — reduction emitted but never validated

- **Problem.** The per-partition partial + ordered combine machinery existed
  (upstream `8ab6bef`), but nothing ever proved the fold reproduced the serial
  fold: wrong identity, wrong signedness or wrong combine order silently gave a
  wrong number.  The defect was never "implement combine" — it was "the emitted
  combine is never checked".
- **Solution (ARS-1 `4f0dc70`).** The operator is tagged from the IR
  (`IRGenVisitor.cpp` maps `c op= x` to a `RedOp` with op, signedness from the
  `icmp` predicate, element type); the combine **clones the body's own
  operation** per flavour and seeds each partial with `identityFor(Op, ElemTy)`.
  Then the validation harness (`validate_reduction.sh` + `frontier_red_test.c` +
  the `reduce_*` cases) runs a **24-config matrix (operator × threads ×
  partitions)** comparing the parallel fold against an independent single-thread
  Python fold of the exact same body.
- **Evidence.** `class=reduction red=1` on both int and real cases; harness
  PASS, 24 configs, 0 failures.

### I — per-source gathers (array frontier)

- **Problem.** `for each vertex u { c = 0; for each neighbor v { c += f(v) }
  deg[u] = c }`: the accumulator is per **source**, but the pair work fn is
  called once per edge with a shared env — no per-source state.  The naive clone
  substituted the loop-carried PHI with a pointer (`fadd ptr, double`),
  deactivated the driver and printed `0.000000` instead of `160136.013072`.
  Arc-less sources are never visited by a neighbour sweep, so `deg[u]` would also
  miss its `0` seed.
- **Solution (behind `SGPL_COMP_I_SOURCE_REDUCTION=1`, default **off**).**
  Class `source-red`: the pair work fn accumulates into this partition's partial;
  a finish hook — *cloned from the driver epilogue*, not a template — stores the
  per-source result and resets the partial to the operator identity; the runtime
  calls it on source changes inside the partition's CSR slice **and** from a
  source-range pass for arc-less sources.  Source ranges are disjoint, so the
  epilogue writes are race-free.
- **Evidence.** `parallel/int_gather_source_red`: `class=source-red`, `degsum 40`
  at 1/4 threads × 1/3/4 partitions (bipartite.txt deliberately has 10 arc-less
  sources, so the range pass is exercised).  Default refusal still pinned by
  `race/int_gather`; `mutual_deg` stays refused in both modes because its body
  contains an inline `hasEdge` scan the totality prover rejects.

### extra — several accumulators in one body

- **Problem.** `Info.ReducePtr` is set by the *first* scalar store and only that
  pointer is mapped to per-partition storage; a second slot (`b = b + 2`) was not
  recorded at all — the loop still classified `reduction` and every partition
  wrote `b` (lost updates).  A second, different recognised operator on the same
  slot was equally invisible.
- **Solution (R3).** Both are now recorded as unrecognised global effects and
  refused, and privatization then serves them generally: each slot gets its own
  private storage per partition.
- **Evidence.** `two_reduce_slots`: `privatized`, serial `a 10 b 20` (a broken
  emit gives `b ≈ 20/partitions`); `priv_two_slots_big`: `a 160000 b 2068247825`
  (Σv over arcs), 1/4 threads, serial-equality ✅.

### 2.1 What B, C, D, extra share (the R3 rule, in one box)

All four are the same object: **every mutating effect is a recognised `U_⊕`
update** — value = old ⊕ operand for an associative, commutative operator.  The
final value of a written location is the operator folded over the updates, and
that fold does not depend on which partition applied which update, so **the loop
needs no owner at all** — only a private place per partition and a fold.

`privLayout()` (`graph_frontier_lowering.cpp`) proves five obligations and
otherwise refuses with a printed reason (`[graph-frontier] priv: refused -- …`):

1. every mutating effect is a recognised update (non-`None` operator);
2. no claim and no frontier append;
3. one operator per base;
4. every load from a privatized base is that update's own old value (must not
   feed an index, call, branch or load);
5. the operator is order-independent for the element type — integer arithmetic
   and min/max accepted; **float `+` and `*` refused** (the fold re-associates).

Mechanism: the reduction record becomes the partition's private state (scalar
slots at 8-byte offsets; each privatized array bound by
`autograph_priv_bind(graph, rec, stride, offset, elems, elem_bytes, identity_bits, slot)`);
`emitPairWorkFn` re-points every GEP on a privatized base at the copy; the emitted
combine folds the copies into the live targets in ascending partition order
(reusing the per-operator combiner), starting from the array's own pre-round
value because the copies begin at the identity.

Enabling recognizer: the front end does not CSE, so one source-level slot lowers
to several equivalent SSA chains (`cnt[deg[u]]` → different-but-equal GEPs for
load and store).  `sameAddressValue()` (structural same-address relation: same
value; two loads from the same address; equal-base GEPs with equal indices;
equal-operand casts) recognises read-modify-writes the old identity check
silently dropped — a general fix used by every path.

---

## 3. The ones that were NOT fixed

| # | What is not fixed | Why (blocker) | Current safe behaviour |
|---|---|---|---|
| **A-dual** | dual-owner loop needing a shadow on cross-region reads (`small_kcore`, `dual_shadow`) | the shadow is a per-**base** decision, but the array carries both round-separated and within-round reads → needs a per-**read** temporal model | refused → `class=sequential`, correct 18959 at 1 and 4 threads |
| **F** | driver-preamble first-wins claim gating the neighbour body (`claim_driver`) | prove the driver visits each source once (or the claim is idempotent + unused), or gate the body from a per-source phase; the per-source *execution* exists (built for D), the *gating* rule does not | refused by classification (`HasDriverClaim`), serial, `race/claim_driver` pins it |
| **G residue** | raw float `select(fcmp olt/ogt)` min/max | not reorder-invariant under NaN / signed zero, so folding partials cannot reproduce the serial fold → needs a finiteness / NaN-free certificate (fast-math flag or value-domain proof) | those loops stay `sequential`; pinned by `race/reduce_real_ops` |
| **multi-slot capacity** | more than 4 privatized arrays; `Top`-provenance (unknown index) subscripts | `priv_buf[4]` slots and the unknown-provenance refusal are capacity/soundness limits, not design limits | refused with a printed reason |
| **`nested_while2`** | verdict-by-accident: classified `source-owner`, the **emit fails**, so it is sequential by fallback | a future emit fix would have to re-derive the verdict; today the class claim is not honest even though the answer is correct and race-clean | sequential by emit failure (census prints `emit failed for class=source-owner -> stays sequential`) |
| **hardcodes still in the tree** | `SGPL_FRONTIER_BLOCKLIST_GUARD=1`; the IRGen front-end recognisers (`detectPeelKFrontierLoop`, first-wins frontier loops) that run *before* the effect algebra; the `sgpl.frontier.nested.sequential` marker (P9) | these are the legacy policy, not the A–I solutions; replacing the marker needs the loop-access-certificate work (honest runtime memory attributes + LAA/MemorySSA) | each has an A/B switch; the derived rules above are the default where implemented |

**Also unresolved, deliberately listed apart — a semantics question, not a
mechanism gap:** A's shadow implements *frozen-round* semantics; a serial
execution of the same in-place loop differs (on `parallel/roundsep.graph` the
rewrite gives `sum 180000`, the rewrite-off build overflows int32).  Both are
deterministic; the language definition has to say which one
`for each vertex { for each neighbor … }` in-place updates mean.

### Reproduce any status above

```bash
cd verify
./run.sh                                  # 85 checks, 0 failures (branch round3)
./class_census.sh                         # per-loop verdict + refusal reasons
./class_census.sh parallel                # the composition cases
SGPL_COMP_I_SOURCE_REDUCTION=1 ./class_census.sh parallel/int_gather.graph
SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1 ./class_census.sh parallel/claim_driver.graph
GRAPH_FRONTIER_STATS=1 ../p1GraphEasy-con-AutoTuner/GraphProgram cases/parallel/mixed_regions.graph
GRAPH_FRONTIER_REWRITE_OFF=1 ...          # serial-equality control for privatized cases
```
