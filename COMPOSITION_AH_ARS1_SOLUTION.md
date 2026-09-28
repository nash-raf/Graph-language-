# Composition A and H — problem and how the ARS-1 push solved them (generalised, not hardcoded)

Branch: `compositions-round2` (`b45c42f`), base `merge-ars1-roundsep` → `e8b89f9`.
The commits that solved A and H live on the ARS-1 line; `compositions-round2`
extends them.  See `COMPOSITION_STATUS.md` §1 ("Three different things get called
'solved'") and §2.

## Why A and H are the templates

Everything E, G, I does is derived from the loop's effect set / provenance.  A
and H are the two **solved-general** items in the table — solved-general because
the *verdict* and the *emitted code* are produced by recognising a property of
the effect set, never by matching an algorithm name or a syntactic blocklist:

| solved-general means | what it is NOT |
|---|---|
| classifier sees **carried-read × mutating-write, same base** → emits a shadow | `if (algo == "bfs") emitRoundsep()` |
| classifier sees **any associative op, tagged from the IR** → partials + identity + ordered combine | `switch(op) { case '+': ...; case 'min': ...; }` over a hand-list of operators |

A is solved by *recognising a dependence shape and emitting a generic transform*.
H is solved by *deriving the combine from the IR op and validating it*.

---

## A — round-separated in-place (cross-endpoint R×W on one base)

**The shape.**  The same array `A` is **read** on one endpoint region and
**written** on the other endpoint region within one owner step, all in place:
`dist[v] = min(dist[v], dist[u] + w(u,v))`.  A destination partition writing
`dist[v]` is concurrent with another partition, two rounds earlier, reading
`dist[u]`, and the read must observe a **frozen round-start** value, not the
live value a sibling is mid-write.  Naive parallelisation is a real race; a
blind "make it sequential" throwaway the parallelisation for no reason.

**How the ARS-1 push solved it (`4f0dc70`, +302 lines in
`graph_frontier_lowering.cpp`).**

1. **Effect-set detection, not a pattern match.**  `analyzeNeighborLoop` walks
   the loop's stores/loads and tags a base as a *round-separation base* when:
   - the base carries both a read effect and a mutating write effect,
   - the writes are single-region (all U or all V — mixed U+V is C, refused),
   - a read index has its origin in the *opposite* region (cross-endpoint),
   - the element type is uniform and `i32`/`double`.
   No algorithm name is consulted.

2. **Shadow emission.**  `emitRoundSepShadow` publishes a per-loop unique
   runtime scratch pointer via `autograph_scratch_shadow` (declared and wired
   in `autotuner_runtime.{c,h}` in the same push).  For every qualifying base it
   emits a **per-round `memcpy`** of the destination region into the shadow in
   the round preheader, and rewrites every *cross-endpoint read* on that base to
   load through the shadow.  Same-region reads (`dist[u]` being accumulated on
   the owner's own slot) are left live so a `min`/`+=-` relax stays a real
   accumulator instead of last-write-wins.

3. **Ownership follows the write.**  `WritesV` selects `DestOwner` vs
   `SourceOwner`; the pair work fn is untouched and just reads through the
   shadow, so the frozen-round semantics is local to the read path.

The whole thing is derived from the effect-set condition above — change the
operator in the body and the shadow still applies; move the read to the same
region and the shadow stops applying.  That is why the doc calls it
`solved-general`.

**Evidence (this checkout).**

```
$ bash class_census.sh cases/race/roundsep.graph
class=dest-owner  red=0  sep=1  data=0  shadow=1
```
`race/roundsep` asserts the answer is identical at 1 vs 4 threads across
1/3/4 partitions (`shadow=1`).  The push adds `roundsep_add.graph`,
`roundsep_min.graph`, `roundsep_sssp.graph` and `roundsep_small.txt` to drive the
`+`/`min` flavours through that same shadow path, and
`compute_roundsep_expected.py` as the independent reference model
(checking it against `validate_roundsep.sh`).

Note the semantic caveat in `COMPOSITION_STATUS.md`: the shadow implements
*frozen-round* semantics, which differs from the serial build for genuinely
in-place loops (serial overflows `int32`); the language definition still has to
say which is meant.  That is a language-law question, not a missing mechanism, so
it does not make A "unsolved".

---

## H — reduction emitted but never validated

**The defect.**  A body like `c = c + x` (or `c = min(c,x)`, `c = c or x`, ...)
over a neighbour sweep is a scalar-global reduction.  The original emitter *did*
emit a per-partition partial + an ordered combine (`emitRedCombiner` /
`identityFor`, machinery dating to `8ab6bef "effect algebra"` — upstream, present
at the last pull) — but nothing ever proved that the folded partial + combine
reproduced the serial fold.  The wrong identity, the wrong signedness, or the
wrong combine order silently gave a wrong number.  "Reduction emitted but never
validated" was literally the bug: the mechanism existed, the proof did not.

**How the ARS-1 push solved it (`4f0dc70`).**  The push did two things:

1. **Tagged the operator from the IR, not from a name.**  `IRGenVisitor.cpp` maps
   `c op= x` to a `RedOp` carrying the operator, the **signedness taken from the
   `icmp` predicate**, and the element type.  The combine then **clones the
   body's own operation** per flavour and seeds each partial with
   `identityFor(Op, ElemTy)` — `0` for add/or/xor, `1` for multiply, `INT_MAX` for
   `umin` (resp. the float min/max/num variants).  No operator is hard-coded in
   a `switch`; adding `min`/`max`/`or`/`xor`/`sub`/`mul` needed no new case
   because the op is read off the IR.

2. **Added the validation harness (`validate_reduction.sh` +
   `frontier_red_test.c` + `run_frontier_red_tests.sh` + the
   `reduce_and/and|reduce_or/mul|sum|min|max` `.graph` cases).**  It runs a
   24-config matrix over **operator × threads × partitions** and compares the
   parallel fold against an independent (single-thread Python) fold of the exact
   same body.  The combine is therefore only trusted inside the configs the test
   covers; H = "the emitted reduction is validated", and that is exactly what
   the harness proves.

The defect H was never "implement combine" — that pre-existed — it was "the
emitted combine is never checked against the serial fold".  The ARS-1 push added
the check itself, and tagged the op from the IR so the check covers every
operator the front-end emits (the "equation from the effect" rule), not a
hand-list.

**Evidence (this checkout).**

```
$ bash class_census.sh cases/race/reduce_int_ops.graph
class=reduction  red=1  temporal=Independent  compat=single
$ bash class_census.sh cases/race/reduce_real_ops.graph
class=reduction  red=1  temporal=Independent  compat=single
$ bash p1GraphEasy-con-AutoTuner/validate_reduction.sh
reduction end-to-end validation: PASS
```
`reduce_int_ops` covers the 7-operator family (guarded `a <op>= b` forms and the
`min()`/`max()` calls) at 1/4 threads × 1/3/4 partitions; the float residue
(raw `select(fcmp)`) is refused by the **totality prover** (see G), not by item
H, and so stays `sequential` — H never promises an invalid fold.

---

## Reproduced here from this checkout

```bash
cd p1GraphEasy-con-AutoTuner
./validate_reduction.sh           # H: 24-config reduction × threads × partitions
./validate_roundsep.sh            # A: roundsep shadow vs independent model
cd ../verify
./run.sh                          # 81 checks, 0 failures on compositions-round2
./class_census.sh race/roundsep.graph
./class_census.sh race/reduce_int_ops.graph
```
`verify/run.sh` rebuilds `GraphProgram` first when any source/header is newer, so
a green run cannot come from a stale binary.
