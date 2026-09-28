# Problems and how they were solved — consolidated log

Scope: every problem found and fixed in this project — the GraphEasy compiler
(PDG classification, frontier lowering, autotuner, runtime), the effect-algebra
"composition" shapes A–I, the language/DSL bugs, the verification harness, and
the Milo motif reproductions (2002 / 2003 / 2004).

Written 2026-09-17 on branch `compositions-round3` (HEAD `f83fe16`).  Companion
documents with deeper per-area evidence:

| Area | Document |
|---|---|
| Compiler open problems (P1–P14) | `OPEN_PROBLEMS_HANDOFF.md` |
| Composition status (A–I, solved/refused/unsolved) | `COMPOSITION_STATUS.md` |
| Round 3 privatization (B, C, D, extra) | `COMPOSITION_R3_PRIVATIZATION.md` |
| Composition A and H (ARS-1 push) | `COMPOSITION_AH_ARS1_SOLUTION.md` |
| Composition E, G, I | `COMPOSITION_EGI_PROBLEM_AND_SOLUTION.md` |
| Autotuner edgemap region (P1) | `p1GraphEasy-con-AutoTuner/CC_AUTOTUNER_EDGEMAP_REGION.md` |
| Milo 2002 Table 1 report (compiled PDF + TeX) | `p2GraphEasy/milo2002_reproduction/results/milo2002_reproduction_report.{tex,pdf}` |
| Milo 2003 Table I/II + 2002/2004 motif write-up | `p2GraphEasy/milo2002_reproduction/paper/motif_recreation.tex` |
| Milo 2003 Table I yeast/www columns | `p2GraphEasy/milo2002_reproduction/results/tableI_yeast_www_columns.md` |
| Milo 2004 status | `p2GraphEasy/milo2004_reproduction/results/status.md` |

Status vocabulary used throughout (from `COMPOSITION_STATUS.md`):

- **solved-general** — the verdict *and* the emitted code are derived from the
  loop's effect set / provenance.  No blocklist, no algorithm name, no
  per-program special case.
- **refused-general** — the loop is refused, but by a *derived rule* that says
  why, pinned by a suite check, with the old behaviour behind a kill switch.
  This is a legitimate outcome, **not** a solution.
- **unsolved** — the parallel case has no mechanism at all; today's correctness
  comes from an emit failing, an ad-hoc blocklist, or luck.

---

## 0. Where each state lives

### 0.1 Branch lineage (current tree)

```
e8b89f9  upstream "ARS interaction with effect system fixed" (= origin/ARS-1)
  └─ 9ab4056  session fixes (real arrays, INF, …)
      └─ 2ca99ad  proof-based frontier lowering, emit postcondition, PDG second chance; autotuner/cc fix
          ├─ 4f0dc70  upstream: parallel runtime support for roundsep   (composition A + H)
          └─ 9115210  merge of the above (merge-ars1-roundsep)
              └─ c7e98b6  pdg: fail-closed dependence verdicts (confused / EQ / invariant slot / calls)
                  └─ 6dc3619  frontier lowering / pass pipeline / runtime: conservative refusals + shadow emission
                      └─ 4c2055d  verify: the harness is now tracked in git
                          └─ 3ae52c5  F refusal, multi-accumulator refusal, min/max fidelity (G)
                              └─ fe2abbd  class assertions, census tool, stale-compiler guard; cases B/F/G
                                  └─ 93ae5a9  per-source reductions (I / P10), switch-gated
                                      └─ 90b87ad  P10 case + class assertions for A/C/D refusals
                                          └─ b45c42f  COMPOSITION_STATUS.md
                                              └─ fddb55b  R3: privatize pure accumulations (B, C, extra)
                                                  └─ cc7b4e0  R3+D: per-source preamble phase + write-up
                                                      └─ f83fe16  (HEAD) drop debug print
```

`main` / `ars1-latest` were not touched; the tags below restore any earlier
state with `git reset --hard <tag>` or `git checkout <tag> -- <file>`:

| Tag | Content |
|---|---|
| `pdg-prework-original` | upstream `e8b89f9` |
| `pdg-prework-baseline` | the uncommitted work-in-progress diff (554 lines) found in the tree |
| `pdg-fixes-snapshot` | baseline + first round of fixes |
| `pdg-session2/3/4-snapshot` | successive session checkpoints |
| `pdg-before-p2p7p12` | `proof-lowering` before the P2/P7/P12 fixes |

### 0.2 Suite size over time (all green at each point)

| State | `verify/run.sh` |
|---|---|
| upstream `e8b89f9` baseline | 45 pass / 1 fail (autotuner/cc) |
| `proof-lowering` (+P1/P8) | 50–51 pass / 0 fail |
| `merge-ars1-roundsep` (+A/H, +P2/P7/P12) | 53 pass / 0 fail / 1 skip |
| night session §2b (+P3/P5/P6/P11) | 62 pass / 0 fail |
| `compositions-round2` | 81 checks, 0 failures |
| `compositions-round3` (current) | **85 pass / 0 fail** (parallel subtree 53) |

---

## 1. Compiler and parallelization

### 1.1 Soundness bugs (produced wrong programs)

---

**P4 — LLVM "confused" read as "independent" (`isConfused()` misuse).**
The most dangerous bug found in this project.

- **Symptom.** PageRank's scatter driver and both neighbour `+=` loops were
  certified `hasProofOfNoCarriedDeps=1` and marked DOALL — with a real race
  (the same bug, without the marker guard, is what produced the old
  nested-gather race).
- **Root cause.** `DependenceInfo::depends()` returns a **non-null** base-class
  `Dependence` for pairs it cannot analyse, and that object carries worst-case
  defaults: `isConfused() == true` *and* `isLoopIndependent() == true`
  (`DependenceAnalysis.h:138-145`).  `pdg.cpp` checked `isLoopIndependent()`
  first, so "understands nothing" was consumed as "no dependence".
- **Fix** (`pdg.cpp:872`): check `isConfused()` first.  Try the alias-analysis
  escape hatch (`AA->alias(...) == NoAlias` on load/store pairs) and only then
  accept independence; otherwise set `hasUnknownAttributedDep = true` and clear
  `hasProofOfNoCarriedDeps` — fail closed.  `AAResults` plumbed into
  `analyzeAndAnnotateLoop` (`pdg.cpp:1504/1512`).
- **Verification.** A/B with `SGPL_NO_PDG_CONFUSED_GUARD=1`: guard off →
  scatter loops `proof=1`; guard on → `unknown=1 proof=0` on every previously
  "proved" traversal loop, while `doall_scaling`'s two real compute loops keep
  their honest DOALL.
- **Consequence to know:** the AA escape hatch is nearly inert *before*
  canonicalization (DSL arrays are indirect pointer globals → `MayAlias`), so
  the guard fails closed very broadly — which is what P5/P6 then fixed.

---

**P2 — `EQ` / zero-distance trust without an invariance check.**

- **Symptom.** None observed on the suite, but unsound in principle: a store
  and a load both addressing `A[%v]` where `%v` is a load/call SCEV cannot prove
  loop-invariant are the *same symbolic value*, so DI can answer `EQ`
  (distance 0) and the classifier read that as "no loop-carried dependence".
  If the value differs between iterations, the loop is wrongly marked DOALL.
- **Fix** (`pdg.cpp`, next to `proveCarrierForDependence`):
  `zeroDistanceOnLoopMemorySubscript()` — a `ProvenZero` verdict is rejected when
  either access's GEP subscripts have a load or call inside the loop in their
  operand DAG (`isLoopMemoryDerived()`, cycle-safe, index operands only).
  Downgrades to unknown.  Kill switch `SGPL_NO_PDG_EQ_GUARD=1`, diagnostics
  `SGPL_PDG_EQ_DEBUG=1`.
- **Honest scope.** Defence in depth, not a demonstrated miscompile: every
  control built (`A[0] += 1` in a while; a scalar-slot index that never changes)
  was refused earlier by DI as *confused*, so the EQ branch is not reached by
  any program we have.  It becomes reachable once canonicalization (P6) turns
  memory-carried induction variables into SCEV AddRecs.

---

**P11 — `while` inside `for each vertex`: wrong answer and effective hang.**

- **Symptom.** `for each vertex u { acc[u]=0; while (k<3) { for each neighbor v of u {…} k=k+1 } }`
  ran one whole-graph engine step *per vertex* (20 000 × 320 000 edge visits —
  a hang) and would have been wrong anyway.
- **Root cause.** `analyzeNeighborLoop()` took the neighbour loop's immediate
  parent (`Info.DriverLoop = L->getParentLoop()`) as the frontier driver without
  checking that it is a graph-iteration loop.  Here the parent is the `while`,
  so the pass replaced the while body with `build_clean_cut + frontier_step_owner_source`
  and dropped the trip count (loopcond header branched to the merge on both
  edges; body unreachable).
- **Fix (fail closed).** The driver must have an SSA induction phi
  (`driverIndVar(Info.DriverLoop)`), which graph-iteration loops always have and
  DSL `while` loops never have before canonicalization.
- **Verification.** `race/nested_while` (wrapped in `timeout 120`, so a
  regression fails rather than hangs): `acc0 57` (= 3 × degree(0) = 3 × 19) at
  1/4/4 threads; a decision sweep over ten suite programs shows no other
  candidate changed class.

---

**P8 — the frontier lowering was shape-fragile (rewrote loops it could not model).**

- **Symptoms seen.** (a) compiler **crash** (later inside LLVM's
  CalledValuePropagation) for neighbour loops containing an inline `hasEdge`
  scan subloop; (b) silent `checksum 0.000000` for the per-vertex FP gather
  (`s += f(v); acc[u] = s`) — the rewrite cloned the body with the loop-carried
  FP PHI replaced by a pointer (`fadd ptr, double`), deactivated the driver and
  emitted nothing.
- **Fix, first round.** `neighborLoopHasForeignCalls()` guard
  (`graph_frontier_lowering.cpp`) refuses loops with subloops, foreign calls or
  FP header PHIs → conservative sequential path; the gather now yields the
  correct `160136.013072` at 1 and 4 threads.
- **Fix, second round — replace the blocklist with a prover**
  (`proof-lowering` branch, default there):
  1. `provesModelable()` — a totality prover: no subloops; every call in the
     modelled allowlist (`autograph_neighbor_iter_{init,next}`,
     `roaring_bitmap_{add,remove}`, profile helpers, intrinsics), indirect/unknown
     calls are opaque; every header PHI integer and non-escaping (an escaping
     scalar is a reduction register the engine has no state for).  Anything else
     is refused **with a named reason**.
  2. **Emit-side postcondition, always on**: after every successful emission the
     function is run through LLVM `verifyFunction`; `SGPL_FRONTIER_STRICT=1`
     (the suite default) aborts the compile instead of trusting a malformed
     module.
  3. **Priority-2 hand-off**: `SGPL_PDG_SECOND_CHANCE=1` lets a refusal skip
     `markSequential` and release the loop to dependence analysis, which still
     must issue its own fail-closed certificate.  Default stays the terminal
     refusal (traversal state is invisible to DI until call effects are modelled).
- **Kept for A/B only:** `SGPL_FRONTIER_BLOCKLIST_GUARD=1`.
- **Verification.** Refusal-reason sweep: pagerank 2/2 modelable, bfs_level 1/1,
  kcore 1/1, sssp/cc no candidates; the only refusals are
  `nested_gather` ("loop-carried non-integer PHI (reduction register)") and
  `mutual_deg` ("contains subloops").  With second chance on: suite 50/0 and
  forced-parallel (4 threads ×3 + `SGPL_FORCE_DOALL_PARALLEL=1`) bit-identical
  for every algo program.

---

**P7 — dead metadata reader with a correctness-shaped intent.**

- `pdg.cpp` *read* `sgpl.frontier.first_wins.candidate`; nothing in the repo
  ever wrote it, so the `IsVerifiedFrontier` branch was permanently dead.
- **Fix:** deleted the reader, the unreachable `sgpl.frontier.first_wins.doall`
  write inside it, and the now-unused `markNestedLoopsSequential()`.
  Renaming the reader to `.doall` was rejected: the branch would still be dead
  (the marker check `IsNestedFrontierLoop → SEQUENTIAL` is evaluated first), and
  activating it would reintroduce the unproven "traversal state is parallel"
  path that P4/P8 exist to remove.
- **Evidence:** whole-repo grep (`.candidate` only in docs; `.doall` has exactly
  one writer and one reader); bfs_level IR: `.doall` 0, `.candidate` 0,
  `sgpl.frontier.nested.sequential` 2, two `autograph_frontier_step_owner_push`
  emitted; suite 50/0.

---

### 1.2 Analysis coverage and performance

---

**P6 — the PDG was blind without pre-PDG canonicalization.**

- **Symptom.** Most DSL memory accesses go through `internal global` pointer
  slots (`%p = load ptr, @arr`), so DI returns *confused* for large clusters of
  pairs; with the P4 guard on, whole loops then fail closed (PageRank's init
  loop moved DOALL → SEQUENTIAL).
- **Fix.** `canonicalizeLoopsForAnalysis()` on **every** pipeline path
  (`SGPL_NO_PDG_CANON=1` restores GPU-only), plus a new step the baseline diff
  did not have: `promoteSingleFunctionGlobals()` turns every internal
  non-constant global whose uses are all non-volatile load/stores in one
  function into an entry-block alloca, then `PromotePass` (mem2reg) lifts it to
  SSA.  `GlobalOpt` alone was verified **not** to do this (two ways: `opt
  -passes='globalopt,mem2reg'` on the real module leaves `@i`/`@a`; a minimal
  loop-used scalar behaves the same).  Switches: `SGPL_NO_PDG_GLOBAL_PROMOTE=1`,
  `SGPL_PDG_CANON_DEBUG=1`.
- **Verification (positive control):** `parallel/array_doall` —
  `localized @a in @main`, `localized @i in @main`; the loop flips from
  `SEQUENTIAL unknown=1` to `classification=DOALL hasProofOfNoCarriedDeps=1`;
  `a_last 199999` at 1/4/4 threads.

---

**P5 — the confused guard was maximally conservative.**

- Consequence of P4 before P6, not a separate bug.  With promotion on,
  PageRank's loops carry positive evidence where they used to say `unknown=1`
  (leaf loops `proof=1`); traversal nests stay SEQUENTIAL.  The AA escape hatch
  is still mostly inert because DSL arrays escape to calls.

---

**P3 — DOACROSS was never exercised.**

- The wait/post runtime path existed but no program ever classified DOACROSS
  (it needs store-forwarded PHIs, i.e. canonicalization).
- **Now:** `pref[i] = pref[i-1] + 1` classifies **DOACROSS**
  (`hasProvenCarriedDep=1`), the IR carries `doacross.wait`/`doacross.post`
  metadata (2 each), and `race/doacross_scan` (n = 200000) asserts the class,
  the metadata and `last 199999` at 1/4/4 threads under
  `SGPL_FORCE_DOACROSS_PARALLEL=1` (2M-element variant also exact).

---

**P1 — `autotuner/cc` was never region-modelled (the one red suite check).**

- **Symptom.** `predicted_ms=0.000000`, `injected 0 layout conversions`; the
  kernel ran (3.16 ms) but only as "pure kernel", never as a region.
- **Root cause.** cc is the only suite algorithm lowered to the legacy
  `autograph_edgemap` engine; `AutoTunerPass`'s classifiers did not match it, so
  `allEvents.empty()` early-out returned; and even past that, the step-annotation
  block sits below an `regions.empty() continue`.
- **Fix (4 edits in `AutoTunerPass.cpp`):** `isCleanCutStepCall()` also matches
  `autograph_edgemap`; early-out becomes `allEvents.empty() && !moduleHasStepKernels(M)`;
  iterate `eventsByGraph ∪ metaByGraphPtr`; move `regions.empty()` after the step
  block; `totalOps = 1` when there are no sibling loops (`uTrav` already prices a
  whole graph pass — with `estM = 320000` the model predicted 394 725 ms for a
  3.9 ms kernel).
- **Verification.** cc emits 2 `autograph_profile_region_enter` calls;
  `region=0 kind=Traverse layout=CSR visits=2 predicted_ms=2.467 measured_ms≈2.6`;
  `autotuner/cc region-modelled` and `prediction within 5x` PASS.

---

**P14 — CleanCut re-partitioned the whole graph on every round.**

- **Symptom.** Every CleanCut program ran *slower* with 4 threads than 1
  (0.81–0.93×), worse on bigger graphs; CPU/wall ≈ 1.0 regardless of threads.
- **Root cause (measured).** The lowering emits `autograph_build_clean_cut(graph, 0)`
  **inside the round loop** — ~85 ms of re-enumeration and re-partitioning per
  round, against ~9 ms of actual parallel step work.  Amdahl caps the speedup at
  ~1.05×; no partition count can fix it.
- **Fix.** Reuse cache keyed on (graph, `layout_epoch`, topology counters,
  partition count); explicit invalidation in all six BCSR mutation entry points;
  `SGPL_NO_CLEANCUT_CACHE=1` restores rebuild-every-call; `SGPL_CLEANCUT_TIMING=1`
  prints build/reuse times.
- **Verification.** pagerank 100k: 1 thread 1.95 s → **0.28 s**, 4 threads
  2.45 s → **0.35 s**, output bit-identical; 200-round run 23.9 s → **2.4 s**
  (13×); cached vs uncached bit-identical on pagerank/bfs_level/kcore/sssp;
  suite 61 pass / 1 skip / 0 fail.
- **Still open (measurement only):** step-level wall-clock speedup needs a quiet
  machine — dispatch logs already show `threads=4` with no serial fallback, so do
  not "fix" a serialisation that is not happening.

---

**P12 — the scaling check flaked on a busy machine.**

- `scaling/doall_scaling` requires 4-thread ≥ 1.2×; once, under load 7.7 on
  4 cores, it recorded 0.87× and failed (a standalone re-measurement immediately
  showed 1.39–1.60×).
- **Fix.** Correctness (`1thr==4thr`) is always asserted; the throughput
  assertion runs only while the 1-minute load average ≤ core count, best-of-5,
  with `SGPL_SCALING_MAX_LOAD=N` override (`0` forces the skip branch) and a
  `SKIP` line that still reports the measured speedup.

---

### 1.3 Language / DSL bugs (all fixed on the current line)

| # | Symptom | Root cause | Fix |
|---|---|---|---|
| 1 | `real r[4]` **crashed the compiler** | static arrays hardcoded `[N x i32]`; a double stored into an int array raised an uncaught `runtime_error` | allocate with the declared element type; int→real conversion in stores/initializers.  `real_array_const` prints `1.500000` |
| 2 | `real m[n][n]` rejected | no semantic/IRGen support for 2D real | element-typed GEPs, 8-byte alignment, element-typed assignment checks |
| 3 | `int INF = 5` failed to parse | `INF` reserved as the infinity-literal token (`Base.g4:298`) | `GraphLangLexer` subclass in `main.cpp` reclassifies `T__66` as an identifier.  Grammar regeneration was **rejected**: the committed `generated/` parser came from a divergent grammar copy and regenerating would drift the language |
| 4 | `print` of a real showed `0.000000` for `1e-07` | `%f` = 6 decimals | `%.13g` at both print sites + updated `verify/expected/lang.manifest` (`real_precision|1e-07`) |
| 5 | `for each edge u, v` on a **directed** graph silently dropped every reverse arc | unconditional `u >= v` dedup | dedup gated on the runtime `directed` field |
| 6 | compiler **segfault** compiling a neighbour loop with an inline `hasEdge` | frontier rewrite produced malformed IR for loops with subloops | `neighborLoopHasForeignCalls()` refusal (see P8) |
| 7 | gather program silently printed `checksum 0.000000` | loop-carried FP PHI cloned as a pointer (`fadd ptr, double`), driver deactivated, epilogue lost | same guard; correct `160136.013072` at 1/4 threads |

Note items 6–7 matter beyond the compiler: the DSL's motif/graph-comprehension
programs (`motifs … = [G where motif {…}]`, `for each neighbor … { s += … }`)
lower through exactly these frontier paths, so those guards are what keep
motif-counting-adjacent programs from silently printing `0`.

---

### 1.4 Build and test infrastructure

| Problem | Fix |
|---|---|
| `build_lowmem.sh` did not build the original commit at all (missing `graph_frontier_lowering.cpp` in SOURCES and Polly's include dir) | both restored |
| `03_run.sh` has a commented-out shebang → falls back to `sh`, which chokes on `set -o pipefail` | always invoke through `bash`; it also reads stdin (`</dev/null`) |
| **The suite drove a stale compiler**: `03_run.sh` compiles the `.graph` but never the compiler, so a source edit + suite run silently reported on the previous binary (the first "verification" of the F fix ran against yesterday's binary — that is how the gap was found) | `run.sh` rebuilds incrementally when any top-level `*.cpp`/`*.h` is newer than `GraphProgram`, exits 2 on build error; later hardened: `compile()` deletes the binary first, so a failed/racing build reports as a build failure instead of a wrong answer |
| Debugging a hanging program cost a full timeout each iteration | `SKIP_PROGRAM_RUN=1` compiles without executing |
| `verify/` was an untracked directory (so the harness itself could not be diffed) | tracked at `4c2055d` |
| Anomaly, investigated, not a bug: un-rewritten order-sensitive programs gave different values on successive compiles, with byte-identical binaries | per-binary behaviour, not a race; recorded so nobody re-chases it |

Harness growth that came with these fixes:

- `expect_class` — every composition case now asserts the *verdict* recorded in
  the build log (`class=…`, plus filters like `red=1`, `data=1`), so a loop
  silently turning sequential (or silently claimed parallel) fails the suite.
- `verify/class_census.sh` — one line per candidate loop with the full diagnosis
  (`driver=`, `red=`, `fw=`, `env=`, `shadow=`, `class=`, effects, `[loop-classify]`).
- `verify/shape_fuzz.sh` — eight loop-shape templates × forced DOALL/DOACROSS ×
  thread counts; fails on compile error, timeout, empty output or any
  1-thread/4-thread difference (`FUZZ DONE failed=0`).
- Serial-equality checks: privatized/parallel cases are also compiled with
  `GRAPH_FRONTIER_REWRITE_OFF=1` and must produce **byte-identical output**.

---

### 1.5 Design item never implemented: replacing the sequential marker (P9)

- `sgpl.frontier.nested.sequential` (attached in `graph_frontier_lowering.cpp`,
  read in `pdg.cpp`) forces SEQUENTIAL for loops under a frontier driver.  It is
  a syntactic stand-in for the traversal state (neighbour variable, iterator
  slot) that the analysis cannot see — exactly the "forced, not derived"
  situation the project wants to remove.  This is the PageRank `next_rank[] +=`
  scatter question.
- **Designed replacement (proposal, no code written):**
  1. honest memory attributes (`argmemonly`, `memory(read)`, `nounwind`, …) on
     the runtime/engine declarations, so a call's effects are visible;
  2. use **LoopAccessAnalysis** as the sound DOALL gate — its fail-closed NO for
     unanalysable subscripts/opaque calls derives SEQUENTIAL instead of relying
     on the marker; **MemorySSA** makes hidden iterator-state clobbers real
     loop-carried dependencies; any EQ result additionally checked with
     `ScalarEvolution::isLoopInvariant`;
  3. reshape-then-certify for gather/private-reduction shapes.
  The marker veto would then be deleted (it currently also gates
  `reconstructParallelIR`), with `SGPL_NO_FRONTIER_MARKER=1` as the A/B switch
  already in place.  The call-effect barrier from §2b is the first piece and is
  already default-on; the invariant-slot guard is the other.

---

## 2. Compositions A–I (effect algebra)

### 2.1 Status after round 3

| # | Shape | Status | Derived from / mechanism | Evidence |
|---|---|---|---|---|
| **A** | in-place cross-endpoint R×W on one base (`dist[v] = min(dist[v], dist[u]+w)`) | **solved-general** | classifier recognises carried-read × mutating-write on the same base → per-round shadow snapshot (`emitRoundSepShadow`, `autograph_scratch_shadow`) | `race/roundsep`: `shadow=1 class=dest-owner`, identical at 1/4 threads × 1/3/4 partitions, vs `compute_roundsep_expected.py`.  Caveat: implements *frozen-round* semantics (see §2.5) |
| **A-dual** | dual-owner step whose cross-region read needs a shadow (claim/activate state machine, `small_kcore`) | **refused-general** (parallel side unsolved) | `Info.RoundSepBases` non-empty disqualifies `DualOwner` — the frozen snapshot breaks within-round observations | `race/dual_shadow`: 18898 vs 18959 over-peel before the refusal; now `class=sequential`, 18959 |
| **B** | write at a data-valued subscript (`cnt[deg[u]] += 1`) | **solved-general (R3)** | privatization: every mutating effect is a recognised `U_⊕` update → per-partition private copy + ordered fold; the owner table is never needed | `priv_data_index_big`: `cntsum 160000 cnt1 1335`, 1/4 threads, serial-equality ✅; `data_index_write` `cntsum 10` |
| **C** | one array written through both endpoint regions (`arr[u] += 1; arr[v] += 1`) | **solved-general (R3)** | same privatization rule; no single region needs to own it | `mixed_regions`: `tot 340000` = n + 2m, 1/4 threads, serial-equality ✅ |
| **D** | reduction next to a per-vertex array write (write in the driver preamble) | **solved-general (R3+D)** | privatization + a second work function / runtime entry (`autograph_frontier_step_owner_red_pre`) that runs the preamble exactly once per source, including arc-less sources | `reduce_plus_write`: `acc 160000 w0 1`; `priv_reduce_write_big`: `wsum 20000` (= Σw = n), 1/4 threads, serial-equality ✅ |
| **E** | cross-phase dependence between the two regions of a dual-owner loop | **solved-general** | `DualOwner = EU ∪ EV`, disjoint bases, `Dependence ⊆ Control ∪ Membership`; front-end effect string shows it | `race/bfs_level`: `R(visited,V):SameRoundRead ⊗ Claim(visited,V) ⊗ R(lvl,U):PreviousRoundRead`, dest-owner, race-clean |
| **F** | first-wins claim in the **driver preamble**, gating the neighbour body | **refused-general** (parallel side unsolved) | claim sits in the driver preamble; every emitted work function runs once per (u,v) pair, so the claim would run per pair and its guard is not reproduced | `race/claim_driver` + `SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1` (switch on → `class=dual-owner` **and** `emit failed → stays sequential`: the old verdict was an accident) |
| **G** | scalar-global reductions with exotic operators | **solved-general** for the recognised op family; one **refused-general** residue | combine clones the body's own operation per flavour (`smin`/`umin`/`minnum`/`minimum`), signedness from the `icmp` predicate, identity per flavour — no op table | `race/reduce_int_ops` (7 operators) + `race/reduce_real_ops` vs an independent Python fold |
| **H** | reduction emitted but never validated | **solved-general** | `validate_reduction.sh`: 24-config matrix (operator × threads × partitions) against a single-thread Python fold of the same body; op tagged from the IR | harness PASS; `class=reduction red=1` on both cases |
| **I** | array-frontier / per-source gathers (`c=0; for each neighbor v { c += f(v) } deg[u]=c`) | **solved-general behind a switch** (`SGPL_COMP_I_SOURCE_REDUCTION=1`, default off) | source-owned step + per-partition partials + a finish hook *cloned from the driver epilogue*; arc-less sources handled by a source-range pass in the runtime | `parallel/int_gather_source_red`: `class=source-red`, `degsum 40` at 1/4 threads × 1/3/4 partitions (bipartite.txt has 10 arc-less sources); default refusal pinned by `race/int_gather`, `race/mutual_deg` |
| **extra** | two scalar accumulators in one body | **solved-general (R3)** | privatization gives every slot its own private storage; the legacy engine's single `ReducePtr` limitation disappears | `two_reduce_slots`: `a 10 b 20`; `priv_two_slots_big`: `a 160000 b 2068247825`, 1/4 threads, serial-equality ✅ |

### 2.2 Round 3 (privatization) — the general rule behind B, C, D, extra

All four are the *same object*: every mutating effect is a recognised `U_⊕`
update (old value combined with an operand through an associative, commutative
operator).  The final value of a written location is the operator folded over
the updates, and **that fold does not depend on which partition applied which
update** — so the loop needs no owner at all, only a private place per partition
and a fold at the end.

`privLayout()` in `graph_frontier_lowering.cpp` proves five obligations and
otherwise refuses *with a printed reason* (`[graph-frontier] priv: refused -- …`):

1. every mutating effect is a recognised update with a non-`None` operator;
2. no claim and no frontier append (per-source / per-round semantics);
3. one operator per base;
4. every load from a privatized base is the update's own old value (must not
   feed an index, call, branch or load) — otherwise the copy answers a different
   question;
5. the operator is order-independent for the element type: integer arithmetic
   (exact mod width) and min/max are accepted; **float `+` and `*` are refused**
   (the fold re-associates and would round differently from the serial order).

Mechanism:

- the reduction record becomes the partition's private state — scalar slots at
  8-byte offsets, and each privatized array bound by
  `autograph_priv_bind(graph, rec, stride, offset, elems, elem_bytes, identity_bits, slot)`
  (allocates/reuses per-partition buffers, initialises to the operator identity,
  publishes each partition's pointer);
- `emitPairWorkFn` re-points every GEP on a privatized base at the copy (the same
  clone-and-rewrite the shadow uses — here for reads *and* writes);
- the emitted combine folds copies into the live targets in ascending partition
  order, reusing the per-operator combiner; buffers start at the identity, so the
  running total starts from the array's own pre-round value;
- **D adds a phase**: `autograph_frontier_step_owner_red_pre()` walks each
  partition's source range and runs the preamble once per source (including
  arc-less sources, which the serial program visits too).

Enabling recognizer fix: the front end does not CSE, so one source-level slot
lowers to several *equivalent* SSA chains (`cnt[deg[u]]` = different-but-equal
GEPs and index values for load and store).  Identity comparison missed the
read-modify-write and the loop was classified a plain store.  `sameAddressValue()`
replaces it with a structural same-address relation (same value; two loads from
the same address; equal-base GEPs with equal indices; equal-operand casts) — a
general fix used by every path, not just R3.

Bug found and fixed inside R3 itself: the combiner's record GEPs were built with
a pointer element type, so byte offsets were multiplied by 8 (dropped in
`f83fe16` after debugging).

### 2.3 Refusals that remain (and what a general fix needs)

| # | Status | Blocker / required mechanism |
|---|---|---|
| **A-dual** | refused-general | the shadow is a per-**base** decision, but the array carries both round-separated and within-round reads → needs a per-**read** temporal model (classify each read, shadow only the round-separated ones).  A shape list cannot express it |
| **F** | refused-general | prove the driver visits each source once (or the claim is idempotent and its result unused), or gate the body from a per-source phase.  Note the runtime now *has* a per-source phase (built for D) — but the claim's **gating** semantics are what is unmodelled, so this is adjacent machinery, not a fix |
| **G residue** | refused-general | raw float `select(fcmp olt/ogt)` min/max is not reorder-invariant under NaN/±0.  Needs a finiteness / NaN-free certificate (fast-math flag or value-domain proof) |
| multi-slot capacity | conservative limit | `priv_buf[4]` slots; "unknown index provenance" (`Top`) refusals — capacity/soundness limits, not design limits |
| `nested_while2` | verdict-by-accident | classified `source-owner` and the **emit fails**, so it is sequential by fallback.  Answers correct and race-clean, but a future emit fix would have to re-derive the verdict |

### 2.4 Hardcodes still in the tree (explicitly not the solution path)

- `SGPL_FRONTIER_BLOCKLIST_GUARD=1` — legacy shape blocklist, A/B only.
- The IRGen front-end recognisers (`detectPeelKFrontierLoop`, first-wins
  frontier loops, synthesized frontier arrays) are shape/name based and run
  *before* the effect algebra; the frontier forms of kcore/bfs_level are served
  by `autograph_edgemap` rather than CleanCut.
- The syntactic `sgpl.frontier.nested.sequential` marker still vetoes
  reconstruction (P9); `SGPL_NO_FRONTIER_MARKER=1` is the A/B switch.

### 2.5 One open semantic question (not a bug)

The shadow implements *frozen-round* semantics for in-place loops — that is what
`compute_roundsep_expected.py` encodes.  It is observably different from a serial
execution of the same source: on `verify/cases/parallel/roundsep.graph` the
rewrite answers `sum 180000` while the un-rewritten build overflows int32.  Both
are deterministic; the language definition has to say which one
`for each vertex { for each neighbor … }` in-place updates mean.  The upstream
validators cannot detect this because they compare against the frozen-read model,
not the serial build.

---

## 3. Milo motif reproductions

### 3.1 The rule set (why some rows will never be filled)

1. **Fingerprint first**: a network is accepted only if its node and edge counts
   match the paper's table *and* its real motif counts match; a newer revision
   of the same network is rejected rather than substituted (this is why S.
   cerevisiae was excluded from the 2002 table).
2. **No invented files**: missing historical inputs stay missing and are
   documented with the expected fingerprint.
3. **Three independent counters must agree on every real count**: GraphEasy's
   `motifs … = [G where motif {…}]` comprehension, a plain Python enumerator,
   and the archived **mfinder 1.20** from the Alon group.
4. **Null statistics** come from mfinder (`-s 3 -r 1000` switch randomisation;
   `-s 4 -r 1000 -met -eth 0` Metropolis with exact triad-census preservation),
   or — for the 2003 paper — from the closed-form Eq. (5) instead of random
   networks at all.
5. Provenance (URLs, hashes, normalisation notes) is recorded in
   `data/provenance.json`, `data/table1_sources.json`,
   `data/originals_inventory.json`.

### 3.2 Milo 2002, Science 298:824 — Table 1 (19 networks, 48 rows)

**Method.** GraphEasy DSL counts → Python `motif_reference.py` → mfinder real
counts (three-way equality), then mfinder null ensembles; acceptance rule
`paper_nreal == grapheasy_nreal == python_nreal == mfinder_nreal` and mean/SD
inside the paper's rounded error bars.  Report + CSVs:
`results/table1_reproduced_rows.csv`, `table1_candidate_diagnostics.csv`,
`table1_status.md`.

**Current state: 6 of 19 networks complete, 17 of 48 rows accepted.**
Plus 2 partial networks whose real counts are exact but whose nulls are
pending/discrepant.

| Network | Status | How the input was obtained / why not |
|---|---|---|
| E. coli | **reproduced** | archived ColiNet-1.0 `coliInterNoAutoRegVec.txt` (Wayback); 424/519 matches; FFL 40, bi-fan 203 all four counters |
| S. cerevisiae | **rejected** | surviving official file is 688/1079; Table 1 expects 685/1052 — not substituted |
| C. elegans | **partial** | weighted `celegansneural` matrix thresholded at weight ≥ 5 → 252/509, real counts exact (FFL 125 accepted); the two 4-node null ensembles are still running/pending because exact triad-preserving Metropolis is slow |
| Little Rock, Ythan, Chesapeake, Coachella | **missing** | exact historical inputs not recovered; no substitutes used |
| St. Martin | **reproduced** | recovered from the archived FoodWeb3D installer, embedded `Gold.wdf`; 42/205; three-chain 469, bi-parallel 382 |
| Skipwith | **reproduced** | FoodWeb3D `WARREN.WDF`; 25/189; three-chain 184, bi-parallel 397 |
| B. Brook | **partial** | FoodWeb3D `HAVENS.WDF`; 25/104; three-chain 181 accepted; **bi-parallel real count 267 is correct on all four counters, but the null ensemble gives 90.0 ± 23.4 against the paper's 30 ± 7** — kept as a diagnostic row, not accepted |
| s15850, s38584, s38417, s9234, s13207 | **missing** | exact historical parser outputs not recovered; replacements rejected (small parser differences change counts) |
| s208, s420, s838 | **reproduced** | historical circuit edge lists available locally (`s208_st.txt` et al.); each row: three-node feedback loop, bi-fan, four-node feedback loop |
| nd.edu (WWW) | **missing** | exact historical web graph not recovered; the paper also used only 100 random networks for this row |

Accepted rows (17) span six motif shapes — mfinder ids 12 (three-chain),
38 (FFL), 98 (3-node feedback), 204 (bi-fan), 904 (bi-parallel), 4740 (4-node
feedback) — so the pipeline is not "one motif done": the counting side has been
exercised across the table's motif families, and the DSL motif engine itself is
general (any directed pattern up to 6 role variables, induced matching,
automorphism-based canonical dedup; compile-time specialization in
`MotifPattern`/`MotifIRBuilder`).

**mfinder build changes (only build + convergence honesty, no counting
semantics):** modern GCC `common` symbols; `-lm` link ordering; and a fix that
**rejects Metropolis candidates that exhausted their iteration budget before
reaching the requested `-eth`** — the unpatched tool can silently return a
non-converged random network.  Every accepted 4-node ensemble was audited to
contain 1,000 networks that reached exactly zero triad-census energy.

**Why nulls are the weakest link here:** mfinder seeds from the wall clock, so
ensembles are not bit-reproducible — and that is precisely where the B. Brook
bi-parallel row departs from the paper.  This is what motivated the 2003
analytic route below.

### 3.3 Milo 2003, PRE 68:026127 ("Subgraphs in random networks") — Table I

The check the user asked for is **equation-only**: Eq. (5) plus the Appendix B
induced corrections, no random networks.

**The model.** Split the degree sequence into single out-degree `K`, single
in-degree `R`, and mutual degree `M` (an edge whose reverse exists contributes
to `M`, not to `K`/`R`; self-loops excluded).  Eq. (5) is a closed form in the
nine moments `⟨K⟩, ⟨R⟩, ⟨M⟩, ⟨K(K−1)⟩, ⟨R(R−1)⟩, ⟨M(M−1)⟩, ⟨KR⟩, ⟨KM⟩, ⟨RM⟩`;
Appendix B recovers the *induced* counts by inclusion–exclusion
(e.g. `⟨id6*⟩ = ⟨id6⟩ − ⟨id38⟩ − ⟨id108⟩`).  A symmetry factor `a` built from
the automorphism group appears — the same automorphism group the motif matcher
already computes.

**What was computed and verified.**

- All three paper columns plus extra networks: transcription (yeast), neurons
  (C. elegans), www — plus E. coli and s208 for the observed-vs-expected
  enrichment reading.
- **Neurons column: 12 of 13 cells match the paper.**
- **id98 (three-node feedback loop) does not — and the paper is wrong.**  Paper
  prints 4.5; Eq. (5) with the paper's own formula gives 3.0.  Back-solving the
  constant shows the printed value needs `c = 2` instead of the printed `c = 3`;
  the paper's own symmetry-factor definition (`a₀ = |Aut| = 3` for the directed
  three-cycle) and its own Appendix C formula (`Σ(A'²·A)/3`) both specify 3.
  Ratio check: `4.5 / 3.004 ≈ 3/2`.  The same slip appears in the www column
  (ours 22.38 × 1.5 ≈ 3.3e1 as printed) — independent confirmation on a second
  network.
- **Yeast column: 11 of 13 cells match** at printed precision; i12/i36 are
  1.5–2.6 % low, explained by a network revision: the exact 685/1052 file the
  authors used is not present in any archive (earliest capture of the Alon dump
  is already 688/1079).  Documented in provenance as
  `yeast_surviving_alon_dump`.
- **WWW column: 12 of 13 match** at printed precision (largest deviation ≈ 2 %).
  Data problem: the paper's own source (`www.nd.edu/~networks/database`,
  `www/www.dat.gz`) was **never successfully archived — every Wayback memento is
  a 404**.  Used the SNAP mirror `web-NotreDame` (1999 Notre Dame crawl,
  Albert–Jeong–Barabási) with provenance documented in `data/raw/www_SOURCE.md`;
  325,729 nodes, 1,497,134 arcs, 27,455 self-loops excluded.
- **A fourth, independent counter**: the paper's Appendix C gives the thirteen
  triad counts as adjacency-matrix products (`M = A + S`; FFL = `Σ A²·A`,
  mutual triangle = `Σ (S²·S)/6`) — pure linear algebra, sharing no code with
  GraphEasy or the Python enumerator.  Implemented in `appendixC.py`.
  Result: **the 13 triads × 3 networks = 39/39 entries agree across all three
  independent methods** (Table in `motif_recreation.tex`).
- Scientific reading from the same table: E. coli's only enriched triad is the
  FFL (40 observed vs 7.4 expected, 5.4×); every degree-driven triad sits at
  0.8–1.0× (null model behaving); s208 enriches the 3-node feedback loop
  (10 vs 1.0, 10.3×).

Working programs: the whole computation (degree decomposition, nine moments,
thirteen equations, six corrections, thirteen motif declarations) runs inside
GraphEasy with **no external tool and no randomized network**
(`milo2003_reproduction/scripts/eq5_moments.py`, `triad_counter.py`,
`table2_formulas.py`; consolidated column table in
`milo2002_reproduction/results/tableI_yeast_www_columns.md`).

**Known limitation stated plainly:** Eq. (5) gives means only — no standard
deviation — so these are enrichment factors, not Z-scores.  What was removed is
the expensive, irreproducible part (the mean over 1,000 random networks).

### 3.4 Milo 2003 — Table II (scaling exponents α): **not completed**

The plan: recreate the scaling exponents for scale-free random networks
(`⟨G⟩ ~ N^α`, three regimes with `γ_c = s + 1`) from `table2_formulas.py`
(analytic) plus `table2_simulation.py` (log–log fits over network sizes
30…3000).  Scripts exist; `milo2003_reproduction/results/table2_simulation_*.{csv,json}`
are present but **0 bytes** — the runs were never captured, so no Table II
numbers are claimed.  This is the main open reproduction item.

### 3.5 Milo 2004, Science 304:1878 (Superfamilies) — triad-significance profiles

Separate effort; same three-counter rule.

- **7 original networks accepted** with fingerprint checks: TRANSC-E.COLI
  424/519; SOCIAL-1 67/182; SOCIAL-3 32/96; ENGLISH 7724/46281; FRENCH 9424/24295;
  SPANISH 12642/45129; JAPANESE 3177/8300.
- Three-way exact agreement on FFL and three-chain for all seven (e.g. ENGLISH
  82031/82031/82031 and 4988512/4988512/4988512).
- Social null snapshots (mfinder `-s 3 -r 1000`): SOCIAL-1 FFL Z ≈ 4.49,
  three-chain Z ≈ −3.95; SOCIAL-3 FFL Z ≈ 0.75, three-chain Z ≈ −1.3.
- Rejected/missing, documented: TRANSC-YEAST (688/1079 vs 685/1052), SOCIAL-2
  (28/110, not found), B. subtilis, signal transduction, Drosophila, sea urchin,
  neurons, WWW, power grid, protein structure.
- Not claimed: the full 13-triad TSP/SP clustering for every network.

### 3.6 Cross-cutting reproduction problems

| Problem | Resolution |
|---|---|
| Original graph files scattered/lost (2002-era) | Wayback Machine; FoodWeb3D installer (`.WDF` files) for three food webs; Alon-lab collection dumps for the rest; every source recorded with URL + hash |
| `www.dat.gz` never archived | every Wayback memento 404 → SNAP mirror used, deviation documented |
| Yeast revision mismatch | rejected for 2002; used with an explicit caveat for 2003 (i12/i36 1.5–2.6 % low) |
| C. elegans graph only available as a weighted matrix | threshold weight ≥ 5 reproduces 252/509 and the real counts |
| mfinder does not compile on modern toolchains | 3 build fixes (common symbols, `-lm`, convergence honesty) — counting semantics untouched |
| mfinder ensembles not reproducible (wall-clock seed) | documented as the reason for the B. Brook discrepancy; 2003 Eq. (5) removes the dependence for the mean |
| "Did the original do it this way?" | every accepted row is pinned by the fingerprint + four-way real-count equality; the paper's own rounding is respected (comparisons at printed precision) |
| Motif counting in GraphEasy depends on the frontier rewrite path | the §1.3 guards (P8) keep these programs from silently miscompiling; the DSL motif engine itself is general (any ≤6-variable induced directed pattern) |

### 3.7 Remaining reproduction tasks (exact list)

1. Run/finish the two C. elegans 4-node null ensembles (bi-fan, bi-parallel).
2. Investigate the B. Brook bi-parallel null discrepancy (90.0 ± 23.4 vs 30 ± 7;
   the real count 267 is confirmed correct).
3. Recover or definitively document as unrecoverable: S. cerevisiae (685/1052),
   Little Rock, Ythan, Chesapeake, Coachella, the five large circuits, nd.edu.
4. Complete Table II (scaling exponents) — scripts exist, results empty.
5. Optional write-up: `milo2003_reproduction/paper/` is empty; the 2003 material
   currently lives inside `milo2002_reproduction/paper/motif_recreation.tex`
   (§"Analytic null model (Itzkovitz et al. 2003)") and
   `results/tableI_yeast_www_columns.md`.

---

## 4. What pins each claim (quick reference)

```bash
# Compiler suite (85 checks on this branch; rebuilds the compiler if stale)
cd verify && ./run.sh
./run.sh parallel                 # 53 composition/race checks
SGPL_CLEANCUT_PARTITIONS=3 ./run.sh parallel/...     # partition sweep
./shape_fuzz.sh                   # loop shapes x forced DOALL/DOACROSS x threads
./class_census.sh                 # per-loop verdict table incl. refusal reasons

# Composition validators (upstream + ours)
cd ../p1GraphEasy-con-AutoTuner
bash validate_reduction.sh        # H: 24-config operator x threads x partitions
bash validate_roundsep.sh         # A: shadow vs independent frozen-read model
bash test/run_frontier_shadow_tests.sh   # 3 configs
bash test/run_frontier_red_tests.sh      # 24 configs

# Predicate checks
SGPL_COMP_I_SOURCE_REDUCTION=1 bash class_census.sh parallel/int_gather.graph
SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1 bash class_census.sh parallel/claim_driver.graph
GRAPH_FRONTIER_REWRITE_OFF=1 ...   # serial-equality control for privatized cases
GRAPH_FRONTIER_STATS=1 ...         # verdict + refusal reason per loop

# Milo
cd p2GraphEasy/milo2002_reproduction/scripts && summarise/run scripts
python3 ../../milo2003_reproduction/scripts/eq5_moments.py <edge-list>   # Eq. (5)
```

The single most important habit the harness enforces: **a green run cannot come
from a stale binary** (rebuild-on-stale + delete-before-compile), and a loop
silently changing its verdict fails the suite (`expect_class`), not just a race.

---

## 5. Open items — one list

**Compiler / parallelization**

1. P9 — replace the `sgpl.frontier.nested.sequential` marker with a derived
   verdict (LAA as sound DOALL gate + MemorySSA + honest memory attributes).
   Designed, not implemented; this is the "provably sequential PageRank scatter"
   question.
2. A-dual — per-read temporal model (round-separated vs within-round reads).
3. G residue — finiteness / NaN-free certificate for raw float `select` min/max.
4. F — driver-preamble claim: gating semantics unmodelled (the per-source phase
   now exists; the rule does not).
5. `nested_while2` — verdict-by-accident (emit fails → sequential); make the
   verdict honest.
6. Privatization capacity limits (`priv_buf[4]`, `Top` subscripts).
7. P14 follow-up — measure the parallel step's wall-clock speedup on an idle box.
8. P13 residue — `Base.g4` vs `generated/` drift (treat `generated/` as
   authoritative); the local loop-classify logger diff.
9. One language-law question: frozen-round vs serial semantics for in-place
   neighbour loops (§2.5).

**Milo reproductions** — see §3.7 (items 1–5).
