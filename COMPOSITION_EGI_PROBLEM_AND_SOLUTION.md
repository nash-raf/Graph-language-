# Composition E, G, I — problems and how they were solved (engineered tree)

Branch: `compositions-round2` (`b45c42f`), base `e8b89f9` (the last pull).
Companion: `COMPOSITION_STATUS.md` §2 / `OPEN_PROBLEMS_HANDOFF.md`.

This branch was built to generalise the way A and H were solved by the ARS-1
push: every verdict and every emitted transform is **derived from the loop's
effect set / provenance**, not from a shape list, an algorithm name, or a
per-program special case.  The hardcodes that still exist (`SGPL_FRONTIER_BLOCKLIST_GUARD=1`,
the `detectPeelKFrontierLoop` IRGen front-ends, the `sgpl.frontier.nested.sequential`
marker) are explicitly **not** the E/G/I solutions — E/G/I are all refused
*soundly* or *parallelised soundly* by the effect algebra.

---

## E — cross-phase dependence between the two regions of a dual-owner loop

**The problem.**  In a dual-owner step the driver walks a frontier and, for each
*p*, launches a neighbour sweep that does work for both endpoints.  If the
neighbour body both *reads* a value that was *written by a neighbour* in the same
round (a cross-region, cross-phase dependence), naïve parallelisation makes
`thread T1` observe `T2`'s in-flight write.  The classic victim is BFS: the body
reads `lvl[u]` (written last round by other sources) and writes `lvl[v]` +
`visited[v]` this round.  A raw DOALL on the body is a data race.

**How it was solved (derived).**  `DualOwner = EU ∪ EV` over two disjoint bases.
The classifier (`classify` in `graph_frontier_lowering.cpp`) splits the effect
string into per-region provenance, then checks the dependence containment:

```
Dependence ⊆ Control ∪ Membership    (the only legal relations)
```

Each read is labelled by its **temporal kind**:
- `SameRoundRead` — same round, same partition → must stay *live* (running
  accumulator style; freezing it would turn a relax into last-write-wins).
- `PreviousRoundRead` — reads a value written in an earlier round → eligible for
  the shadow snapshot (A's frozen round-start view).

For the cross-region neighbour reads that are the actual hazard, the dependence
is recognised as `Control`/`Membership`-only: the driver claims the destination
first (`Claim(visited,V)`), so a source's pair work can only ever observe a
destination that was already claimed — no same-round cross write is reachable.
The two regions are also provably disjoint (`EU` vs `EV`), so the body parallelises
as a `dest-owner` step, and `Activate(next_frontier,V)` is the only cross-region
write and it is itself owner-computes (single-writer per destination).

There is **no edge-case list**: any loop whose effect set reduces to
`R:PreviousRoundRead ⊗ Claim ⊗ W ⊗ Activate` with disjoint bases is parallel by
the same rule.

**Evidence (verified on this tree).**

```
$ bash class_census.sh cases/race/bfs_level.graph
[graph-frontier] candidate: main driver=loopcond21 inner=foreach_nbr.cond kind=1 ...
    class=dest-owner  shadow=1  temporal=PreviousRoundRead  compat=single
    R(visited,V):SameRoundRead ⊗ Claim(visited,V) ⊗ R(lvl,U):PreviousRoundRead
    ⊗ W(lvl,V) ⊗ Activate(next_frontier,V)
```
The same census line shows `compat=single`; the runtime guard is exercised by
`race/bfs_level` which compares the parallel result against the serial build and
an independent Python model across 1/3/4 partitions.

---

## G — scalar-global reductions with exotic operators

**The problem.**  A body like `c = c op x` over the neighbour sweep is a
*scalar-global* reduction: every thread touches the same accumulator.  Naive
parallelisation either races on `c` (raw parallel) or, for operators whose
identity/order matters, gives the wrong answer even when race-free (e.g. integer
`min`/`max`, signedness-dependent `umin`/`umax`, floating `minnum`/`maximum`
vs `fmin`/`fmax` vs `fadd`, and the non-associativity of float `min`/`max` with
NaN/signed-zero).  The original emission folded a per-partition partial and
combined it back, but **never validated** that the chosen combine reproduced the
serial fold — so "reduction emitted but never validated" was its own defect (that
is H).

**How it was solved (derived).**  `IRGenVisitor.cpp` turns `c op= x` into a
`RedOp` **tagged from the IR**: the operator, its signedness (taken from the
`icmp` predicate), and the element type.  The lowering then:

1. allocates one per-partition partial, seeded with `identityFor(Op, ElemTy)`
   (`emitRedCombiner`/`identityFor` in `graph_frontier_lowering.cpp` — these are
   the upstream `8ab6bef "effect algebra"` machinery),
2. folds each partition's contribution with the *body's own* operation
   (the combine literally clones the same `op` node, so the per-flavour combine
   reproduces the body's semantics — `smin`/`umin`/`minnum`/`minimum`, etc.),
3. combines the partials in a deterministic order.

There is **no operator table to extend**: the op is read off the IR, signedness
from the predicate, identity per flavour.  `min`/`max` etc. are therefore just
"the IR op", recognised generically.

**The one refused residue (soundness boundary).**  A raw floating `select(fcmp
olt/ogt)` written by the user instead of a canonical min/max is **not
reorder-invariant** in the presence of NaN or signed zeros, so partial folding
cannot be proven equivalent to the serial fold.  That body is refused
(`temporal=Independent` on the recognised forms; the raw `select` float form is
classified `sequential`) rather than guessed at.  General fix = prove the stream
finite/NaN-free (fast-math or a finite-value certificate) — noted as an open edge
in `COMPOSITION_STATUS.md` §1.

**Evidence.**  `race/reduce_int_ops` (7 operators, guarded + `min()`/`max()`
forms) and `race/reduce_real_ops` are both checked against an independent Python
fold in `validate_reduction.sh` (24-config red×threads×partitions matrix).
Current census:

```
reduce_int_ops  : class=reduction  red=1  temporal=Independent  compat=single
reduce_real_ops : class=reduction  red=1  temporal=Independent  compat=single
```

---

## I — array-frontier / per-source gathers (`c = 0; for each neighbor v { c += f(v) } deg[u] = c`)

**The problem.**  `deg[u]` is written once per source *after* the neighbor
accumulator `c` is exhausted.  Two partitions that own different sources `u`
write *different* slots of `deg`, so the destination array itself is
race-free — but the *reduction register* `c` is per-source, and the engine's
owner-step emits a pair work function that is called **once per edge with no
per-source state**.  Cloning `c += f(v)` into that work fn substitutes the
per-source reduction register with a pointer → it emits a `fadd ptr, double`
through an unsized slot and, with the driver deactivated, silently prints `0.0`
(this is the exact failure mode seen on `nested_gather`, where `160136.013072`
collapsed to `0.000000`).  Worse, arc-less sources (sources with out-degree 0)
are never visited by a neighbour sweep, so `deg[u]` never gets its `0` seed
unless a separate range pass seeds it — the `bipartite.txt` case has exactly
this (sources with no out-arcs).

**How it was solved (derived, gated).**  `graph_frontier_lowering.cpp` recognises
the shape via provenance: the accumulator `c` is a *source-owned* (U-region)
scalar, the write-back `deg[u] = c` is a `W(SameRound)` on a U-region base.
The classifier returns `Klass::SourceReduction` and the step emits:

- a source-owned step that carries **per-source, per-partition partial** storage
  (`U+(c,G)` effect), and
- a **finish hook** cloned from the *driver epilogue* — not from a hardcoded
  `deg[u] = c` template, but from whatever the program actually wrote as the
  post-loop write-back — followed by an ordered combine of the partials.

Arc-less sources are handled by a **source-range pass** in the runtime
(`parallel_runtime.{c,h}`): the driver seeds every source slot over the source
range, so out-degree-0 vertices get `0`.

It is general (the finish hook is cloned, the op is effect-modelled) but it is
**switch-gated** (`SGPL_COMP_I_SOURCE_REDUCTION=1`, default off) so the
well-exercised refusal stays the default until more body shapes (bodies with
inline `hasEdge`, the `mutual_deg` case) are proven sound — those are refused by
the totality prover, not by item I.

**Evidence.**

```
# default (refused, sequential, pinned by a check):
$ bash class_census.sh cases/race/int_gather.graph
--- ... kind=3 ... class=sequential ...   (acc 160000, w0 1)

# under the switch (parallel):
$ SGPL_COMP_I_SOURCE_REDUCTION=1 bash class_census.sh cases/parallel/int_gather.graph
class=source-red  U+(c,G)  temporal=Independent  compat=single

# runtime: degsum 40 at 1/4 threads across 1/3/4 partitions (bipartite.txt)
$ parallel/int_gather_source_red ...  -> PASS (1 thr == 4 thr)
```

---

## Reproduced here from this checkout

```bash
cd verify
./run.sh                 # 81 checks, 0 failures on compositions-round2
./class_census.sh        # per-loop verdict census
SGPL_COMP_I_SOURCE_REDUCTION=1 ./class_census.sh parallel/int_gather.graph
SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1 ./class_census.sh parallel/claim_driver.graph
```

`run.sh` rebuilds the compiler first when any source/header is newer than
`GraphProgram`, so a green run cannot come from a stale binary.
