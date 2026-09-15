# Composition R3: privatization — the shapes B, C and D, and how they were fixed

Written 2026-09-15 on branch `compositions-round3` (base `compositions-round2`
@ `b45c42f`).  Companion to `COMPOSITION_STATUS.md`, which listed these shapes as
*refused-general . . . parallel side unsolved*.  This document records what each
problem was, what the general rule is, how the mechanism is built, and what the
verification pins.  Two items of that list (A-dual and the float-`select`
residue of G) are **not** in this round; see §6 for what remains and why.

Everything here is derived from the loop's effect set.  There is no shape
blocklist, no algorithm name, and no per-program special case: the same
`privLayout()` proof decides all three shapes, and its refusals say why.

---

## 1. The problem in one sentence

Three different-looking loops are the *same object*:

| # | shape | why the owner model could not take it |
|---|---|---|
| **B** | `cnt[deg[u]] = cnt[deg[u]] + 1` | the destination house is a **data value**, not a vertex id — the CleanCut owner table `CC_PART_OF` is keyed by destination vertex, so there is nothing to key |
| **C** | `arr[u] = arr[u] + 1; arr[v] = arr[v] + 1` | one base written through **both endpoint regions** — no single region owns it, and `DualOwner` requires `EU ∩ EV = ∅` |
| **D** | `w[u] = w[u] + 1;` … `acc = acc + 1` | the write is in the **driver preamble** (once per source) while the reduction runs per pair; the reduction engine maps exactly one scalar slot (`ReducePtr`) and has no per-source phase |
| extra | `a = a + 1; b = b + 2` | same as D's second half: the engine gives per-partition storage to `ReducePtr` **only**, so `b` would have been written by every partition (lost updates) — it was refused rather than miscompiled |

They are the same object because of a single observation:

> Every mutating effect in these loops is a recognized **U_⊕ update** — an
> update whose value is the old value combined with an operand through an
> associative, commutative operator (`+`, `-`, `*`, `min`, `max`, …).
> The final value of a written location is then the operator folded over the
> updates, and **that fold does not depend on which partition applied which
> update**.

A loop like that needs no owner at all.  It needs only a place for each
partition to accumulate privately, and a fold at the end.  That is exactly what
privatization gives it — and it is the *only* mechanism that can serve a base
whose destination house is a data value, because it never has to name an owner.

## 2. The rule (derived, not enumerated)

`privLayout()` (in `graph_frontier_lowering.cpp`) proves the following; if any
obligation fails the loop falls back to the sequential verdict with a printed
reason (`[graph-frontier] priv: refused -- <reason>`):

1. **Every mutating effect is a recognized update** (`EffectKind::Uop` with a
   non-`None` operator).  An unrecognized update has no combine to fold with, so
   it is refused.
2. **No claim and no append**: no first-wins claim (driver or body), no frontier
   append — those carry per-source / per-round semantics a per-pair step cannot
   reproduce.
3. **One operator per base**: two operators on one base (`s = s + x; s = s * y`)
   would be folded with the wrong one.
4. **Every load from a privatized base is the update's own old value**: each load
   must be a link in a chain that ends in a store to the *same* base, must reach
   at least one such store, and must not feed an index, a call, a branch or a
   load.  A load used as an index or a condition would observe the real array,
   not this partition's copy, and the copy would answer a different question.
5. **The operator is order-independent for the element type**: integer
   arithmetic is exact modulo the width, and min/max are idempotent and
   commutative, so the fold reproduces the serial result; **float `+` and `*`
   are refused** because the fold re-associates and would round differently from
   the serial order.

The verdict is `Klass::Privatized`, and it is taken where the effect set says so
— *before* the ownership decisions for the data-valued subscript, *instead of*
the same-base-U+V refusal for C, and *before* the legacy single-slot reduction
path whenever the layout needs more than one slot or any array.

## 3. The mechanism

Three pieces, all compiler-side except one buffer:

* **The reduction record becomes the partition's private state.**  The preheader
  allocates `partitions × RecSize` bytes; scalar accumulators get 8 bytes each
  (initialized to their operator identity, per partition), and every privatized
  array gets an 8-byte *pointer* filled by the new runtime call

  ```
  autograph_priv_bind(graph, rec, stride, offset, elems, elem_bytes,
                      identity_bits, slot)
  ```

  which allocates (or reuses) `partitions` buffers of `elems × elem_bytes`,
  re-initializes them to the operator identity, and publishes each partition's
  pointer into its record.
* **The pair work function re-points every GEP on a privatized base** at the
  partition's own copy (the same clone-and-rewrite the round-separation shadow
  uses for reads, here applied to reads *and* writes).  The body's own
  load/op/store chain therefore accumulates into the copy.
* **The emitted combine folds the copies into the live targets** in ascending
  partition order, reusing the per-operator combiner the scalar reduction path
  already had (`*out = *out ⊕ *partial`), element-wise for arrays.  The private
  copies start at the identity, so the running total starts from the array's own
  pre-round value — no separate "apply the initial value" step.

**D adds a phase.**  A driver-preamble write (`w[u] = w[u] + 1`) runs once per
source, so it cannot live in the pair work function.  The step gets a second
work function (the same `PairPhase::UOnly` clone the dual-owner path uses) and a
runtime entry `autograph_frontier_step_owner_red_pre()` that walks each
partition's source range from the CleanCut source slices and invokes the
preamble exactly once per source — *including sources with no arcs*, which the
serial program also visits.  Both phases write only into private state, so no
ownership is involved anywhere.

### The recognizer this depended on

The front end does not CSE, so one source-level slot lowers to several
equivalent SSA chains: `cnt[deg[u]]` is emitted as `load @cnt; gep; load elem`
and again, with *different but equal* GEPs and index values, for the store.
Identity comparison (and even one level of "two loads from the same pointer")
missed the update, so `cnt[deg[u]] = cnt[deg[u]] + 1` was classified as a plain
store `W(cnt,D)` and never had a chance to be privatized.

`sameAddressValue()` replaces that comparison with the structural same-address
relation, recursively: same value; two loads from the same address; two GEPs
with the same base and equal indices; two casts of the same kind over equal
operands.  This is a general fix — it recognizes read-modify-writes the old
check silently dropped, in every path that uses it, not just R3.

## 4. What changed, file by file

| file | change |
|---|---|
| `graph_frontier_lowering.cpp` | `Klass::Privatized`; `privLayout()` proof + `privLayoutNeeded()`; `sameAddressValue()`; R3 record mapping in `emitPairWorkFn` (slots + base redirection); `emitPrivScalarInit()`; `emitPrivCombiner()`; `emitPrivatizedStep()` (pair-only and pair+preamble forms); dispatch; refusal reasons under `GRAPH_FRONTIER_STATS=1` |
| `autotuner_runtime.h` | per-partition private buffers (`priv_buf/priv_bytes/priv_stride[4]`) |
| `autotuner_runtime.c` | `autograph_priv_bind()`; `autograph_frontier_step_owner_red_pre()` (per-source preamble + per-pair work + deterministic combine) |
| `verify/run.sh` | class assertions updated to `privatized`; every privatized case now also builds with `GRAPH_FRONTIER_REWRITE_OFF=1` and requires the outputs to be **equal**; `compile()` no longer trusts mtime (it deletes the binary first, so a failed or racing build reports as a build failure instead of a wrong answer) |
| `verify/cases/parallel/priv_two_slots_big.graph`, `priv_data_index_big.graph`, `priv_reduce_write_big.graph` | the scale cases (g20k) |

## 5. Verification

Full suite: **85 passed, 0 failed** (`verify/run.sh`, up from 81 checks); the
parallel subtree is 53 checks.  What each privatized case pins:

| case | verdict | value (independent) | threads | serial-equality |
|---|---|---|---|---|
| `two_reduce_slots` | `privatized` | `a 10 b 20` | 1, 4 | ✅ `GRAPH_FRONTIER_REWRITE_OFF` |
| `data_index_write` | `privatized` | `cntsum 10` | 1, 4 | ✅ |
| `mixed_regions` | `privatized` | `tot 340000` = n + 2·m | 1, 4 | ✅ |
| `reduce_plus_write` | `privatized` | `acc 160000 w0 1` | 1, 4 | ✅ |
| `priv_two_slots_big` | `privatized` | `a 160000 b 2068247825` (Σv over arcs) | 1, 4 | ✅ |
| `priv_data_index_big` | `privatized` | `cntsum 160000 cnt1 1335` (# vertices with out-degree 1) | 1, 4 | ✅ |
| `priv_reduce_write_big` | `privatized` | `acc 160000 wsum 20000` (Σw = n) | 1, 4 | ✅ |

The expectations are derived from the fixture (like the existing `arcs`
variable) rather than from the compiler's own output, and the serial-equality
check is the stronger statement: the rewritten build must produce **byte-identical
output** to the same program compiled with the rewrite switched off.

Discriminating probes, specifically: `cnt1` and `b` catch lost or double
counting; `w0`/`wsum` catch a preamble that ran per arc instead of per source
(with the preamble folded into the pair loop, `w0` would be the out-degree of 0
and `wsum` the arc count); `tot` catches an array written from both regions
without privatization.

## 6. What is still unsolved (and why)

| # | status | blocker |
|---|---|---|
| **A-dual** (`small_kcore` shape) | still `refused-general` | the shadow is a per-*base* decision, but this array carries both round-separated and within-round reads: the fix is a per-*read* temporal model (classify each read as round-separated or within-round, shadow only the former).  A shape list cannot express it, because the same base needs both |
| **G residue** (raw float `select(fcmp olt/ogt)` min/max) | still `refused-general` | folding partials with that select is not reorder-invariant under NaN/signed zero, so it cannot reproduce the serial fold.  Needs a finiteness / NaN-free certificate for the stream (fast-math flag or a value-domain proof); no such proof exists today |
| multi-slot beyond 4 arrays / `Top`-provenance subscripts | conservative | `priv_buf[4]` slots and the "unknown index provenance" refusal are capacity/soundness limits, not design limits |
| `nested_while2` verdict-by-accident | unchanged | its `source-owner` emit fails, so it is sequential by fallback; unrelated to R3 |

Also unchanged: A and H remain the two `solved-general` items of the earlier
list, E/G-main/I keep their verdicts, and the census/class assertions for the
other cases are untouched or strengthened, never weakened.

## 7. Reproduce

```bash
cd verify
./run.sh                       # 85 checks, 0 failures
./run.sh parallel              # 53 checks (the composition cases)
SGPL_CLEANCUT_PARTITIONS=3 ./run.sh parallel/...   # partition-count sweep

# see a verdict and its refusal reason:
GRAPH_FRONTIER_STATS=1 ../p1GraphEasy-con-AutoTuner/GraphProgram cases/parallel/mixed_regions.graph

# compare against the unrewritten build by hand:
GRAPH_FRONTIER_REWRITE_OFF=1 GRAPH_FILE=cases/parallel/priv_data_index_big.graph bash ../p1GraphEasy-con-AutoTuner/03_run.sh
```
