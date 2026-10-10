# Complete Two-Axis Interference-Proofing — final report

Plan: `p1GraphEasy-con-AutoTuner/plan.md` (891 lines, 14 sections; copy of
`/home/user/Downloads/PLAN.md`).  Baseline: `104beb9` (== `origin/ARS-1`);
proof-layer commits `53473fc` (sections 3–10) and `45dc1ec` (section 11).
Rule: every section ends with live evidence; only measured environment limits
SKIP (with the number); nothing was weakened to make a check pass.

## Plan section status

| § | status | evidence |
|---|---|---|
| 1 | PASS | the objective is implemented: the decision comes from the complete relation/graph proof, R1–R7 stay as witness classes and transformations |
| 2 | PASS | the existing effect/runtime architecture was extended, not replaced (same analysis, same lifecycle, same emitters' contract) |
| 3.1 | PASS | `Effect::Footprints` (one footprint per access mode; RMW owns Read+Write), resource footprints in the `SGPL_RES_*` vocabulary; `GRAPH_FRONTIER_RELCHECK=1` census on every candidate → `[relcheck] OK` |
| 3.2 | PASS | `deriveAccessRelations` rewritten over footprints: RAW/WAR/WAW, no 64-cap (`RelationOverflow` = rejected), semantic order never invented for `Par` siblings, per-relation metadata (footprints/indices/regions/phases/scopes/instance/discharge/witness) |
| 4 | PASS | `Temporal` extended and per-relation (`SameRoundWrite/Ordered`, `PreviousRoundWrite`, `FixedOffset`, `Affine`, `Unknown`); both scopes materialized: within-round phase graph + across-round round graph |
| 5 | PASS | owner resolver: `ExactOwner` (canonical endpoint-slot identity), `MayCrossOwner` (different index in one region), `CrossOwner` (opposite endpoints), `NoOwner` (data/scalar/constant index), `UnknownOwner` (`Top`); `footprintHasOwner` resolves private copies, recognized-fold partials, staged claims and the wired envelope.  CleanCut range refinement beyond the endpoint identity remains an available refinement, not a contract gap |
| 6 | PASS (compiler) | phase/round/spatial graphs: nodes, edges, components (union-find over surviving edges), cycle check (Kahn), crossing test, unary obligations, per-edge realizable flag |
| 7 | PASS | `Transformation` records per mechanism with the conditions each checked; validation rejects a transformation that removes an undis-charged relation; the soundness rule ("only disjointness/ownership/privatization/snapshot/staging/fold remove an edge") is enforced by construction of the discharge logic |
| 8 | PASS (runtime) | `sgpl_exec_dag_product` materializes the `(unit × partition)` product (spatial edges inside each unit, cross-round edges between units, packed cell instances); the compiler side supplies the axis masks, phase/round/partition projections and the phase dimension |
| 9 | PASS | `AxisProof` with all relations, discharged/unresolved, transformations, components, overflow/invalid/inconclusive; an inconclusive proof is forced to non-independence |
| 10 | PASS | `selectSchedule` consumes the proof; R8 → its own guard+reason; overflow/invalid/unrealizable → serial+impl; effective axes = certificate ∨ graph (graph may only be stricter); a surviving `CrossOwner` edge adds the tagged `RealizationOrder` edge |
| 11 | PASS (runtime) | extended descriptor + fail-closed validation; `sgpl_exec_dag_spatial_graph`, `sgpl_exec_dag_temporal_units` (chain is a caller), `sgpl_exec_dag_product`; 17 new scheduler checks ALL PASS; TSan 0 races.  Open: the *emitted* programs declare their schedule via `autograph_exec_ctx_set_dag_axes` rather than passing a descriptor array |
| 12 | PASS (compiler/docs) | `buildTemplates` consumes the proof; `EFFECT_SYSTEM.md` and the CPU technical report state explicitly that R1–R7 are witness classes/transformations, not an exhaustive decision procedure; `verify/TWO_AXIS_STATUS.md` and `verify/PLAN_PROGRESS.md` carry the state of record |
| 13 | PASS | corpus sweep `verify/check_graphs.sh` with the cover gate + strictness census; `GRAPH_FRONTIER_RELCHECK`/`_GRAPHS` reports with invariant assertions; 17 new scheduler checks; adversarial `r6_order_sensitive` (both axes dirty, semantic serial, answers identical at 1/4/8); corpus differentials in the suite and the device diffs |
| 14 | see below | acceptance list checked item by item |

## §14 acceptance list

1. Complete spatial/temporal/product dependence graphs — **yes** (compiler: phase, round, spatial; runtime: product materialization).
2. No strongest-only loss — **yes**: every RAW/WAR/WAW relation is retained; the scalar `Effect::Temp` is a diagnostic maximum only.
3. Spatial components from owner predicates + surviving edges — **yes**.
4. Temporal partitions from all locality relations — **yes**.
5. Product combines both projections without losing cross-axis constraints — **yes** (product edges carry both the spatial index relation and the round offset; the axis mask records the origin).
6. Every edge removal justified by a recorded transformation — **yes** (validated; an undis-charged removal is an internal error that fails the proof).
7. R1–R7 no longer the sole completeness mechanism — **yes** (proof-derived verdict; the certificate is diagnostic + the transformation source).
8. Runtime consumes the proof-derived graph preserving lifecycle — **yes for the executors and validation; open for the emitted descriptor wiring** (documented).
9. Clean axes stay parallel while the other is ordered — **yes** (suite: `carried_read_state` spatial-dag at peak 4; `dual_same_base_big` staged dual, two ordered parallel unit runs).
10. Interfering axes keep their order, partial DAG concurrency where safe — **yes** (partial DAGs for temporal units and spatial partitions, mutex edges realization-tagged, `r6_order_sensitive` serial).
11. Unknown/incomplete/cyclic/overflowed proofs fail closed — **yes** (overflow → reject; cycle → invalid; unary obligation → no DAG; unknown axis bits / discharged edges / contradictory duplicates → rejected; runtime + compiler-validation + unit tests).
12. Sequential/parallel differential tests pass across the corpus — **yes** (suite, engine tests, algebra, composition, device diffs).

## Local evidence

| check | result |
|---|---|
| `verify/run.sh parallel` | 64–67 PASS / 0 FAIL; the one SKIP is load-dependent (`scaling/doall_scaling 4thr 1.52x, not asserted (load 4.30 > 4 cores)`) |
| `test/run_exec_engine_tests.sh` | exec engine 0 failures at 1/3/8; dag scheduler ALL PASS incl. 17 new §11 checks |
| TSan (`setarch -R`, 8 threads) | dag scheduler 0 races; exec engine 0 races |
| `validate_algebra.sh` | 21 fixtures, 0 failures |
| `validate_tdg_budget.sh` | ALL PASS (three configs) |
| `validate_composition.sh` | DONE, no MISMATCH / LINK FAIL |
| `verify/check_graphs.sh` | cover gate 0 failures; strictness deltas listed (R8-guarded min/max candidates) |
| `GRAPH_FRONTIER_RELCHECK=1` | `[relcheck] OK` for every candidate (kind/footprint consistency asserted) |

## Pod evidence (`root@213.173.110.198 -p 10574`)

Environment: 5.1-CPU quota (`nproc` 48), 31 GB (host 251 GB), `/usr/bin/time`
absent, RTX 2000 Ada 16 GB.  Toolchain brought up: LLVM 20.1.8 (apt.llvm.org,
NVPTX present), antlr4 4.13.1 under `/usr/local` (the distro's 4.10 lacks the
`antlr4::internal::call_once` the generated parser needs), `libpolly-20-dev` +
`libisl-dev`, symlinks for the include/lib paths, `ldconfig` entry.
Build: `LLVM_CONFIG=/usr/lib/llvm-20/bin/llvm-config bash build_lowmem.sh` →
rc=0, 0 errors, `GraphProgram` 7.5 MB.

<!--POD-RESULTS-->

## Honest open items

1. Emitted descriptor wiring (§11/§12): the compiler builds and validates the
   proof graphs and the runtime executes proof-derived descriptors, but the
   emitted programs still declare their schedule through
   `autograph_exec_ctx_set_dag_axes`; passing the compiler's descriptor array
   to `sgpl_exec_dag_spatial_graph/_temporal_units/_product` is the remaining
   integration step.
2. CleanCut range refinement of `MayCrossOwner` (a same-region, different-index
   pair) is conservative today: it stays a may-cross unless a transformation
   covers it, which is the fail-closed direction.
3. §15's pair-count work estimate is a ceiling-only estimate (unchanged from the
   previous round).
