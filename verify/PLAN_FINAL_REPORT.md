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
| `verify/run.sh parallel` | **69 PASS / 0 FAIL** (includes `race/overflow-fail-closed` and `race/maycross_same_region`) |
| `test/run_exec_engine_tests.sh` | exec engine 0 failures at 1/3/8; dag scheduler ALL PASS incl. 17 new §11 checks |
| TSan (`setarch -R`, 8 threads) | dag scheduler 0 races; exec engine 0 races |
| `validate_algebra.sh` | 21 fixtures, 0 failures |
| `validate_tdg_budget.sh` | ALL PASS (three configs) |
| `validate_composition.sh` | DONE, no MISMATCH / LINK FAIL |
| `verify/check_census.sh` | **PASS** over 161 fixtures (nested 60, spatial-dag 5, temporal-dag 3, serial 1, serial+impl 5; axis splits R_S=1 R_T=0 ×6 and R_S=0 R_T=1 ×1) |
| `verify/check_graphs.sh` (case corpus) | 52 candidates, **cover_S 52/52, cover_T 52/52** (the graph is never cleaner than the certificate); **3 strictness deltas**: `maycross_same_region` (no witness, graph connected) and the two R8-guarded `reduce_real_ops` min/max candidates |
| `GRAPH_FRONTIER_RELCHECK=1` | `[relcheck] OK` for every candidate (kind/footprint consistency asserted) |

The `maycross_same_region` fixture is the plan's section-13 adversarial case
made permanent: two different index expressions on one base in one region
(`A[v]` and `A[v+1]`), so two iterations may touch the same element while the
certificate reports `R_S = R_T = 0`.  The proof layer classifies the pair
`MayCrossOwner`, keeps the edges and realizes the nest as serial with an
implementation refusal — the answer is identical to the unrewritten build at
1 and 4 threads, and the strictness delta is reported.

## Pod evidence (`root@213.173.110.198 -p 10574`)

Environment: 5.1-CPU quota (`nproc` 48), 31 GB (host 251 GB), `/usr/bin/time`
absent, RTX 2000 Ada 16 GB.  Toolchain brought up: LLVM 20.1.8 (apt.llvm.org,
NVPTX present), antlr4 4.13.1 under `/usr/local` (the distro's 4.10 lacks the
`antlr4::internal::call_once` the generated parser needs), `libpolly-20-dev` +
`libisl-dev`, symlinks for the include/lib paths, `ldconfig` entry.
Build: `POLLY_LIBS="" LLVM_CONFIG=/root/llvm-cfg bash build_lowmem.sh` → rc=0,
0 errors.  (`/root/llvm-cfg` prepends `--link-static`: the pod's
`libLLVM-20.so` registers Polly's options internally, so linking the static
Polly archives as well aborts with "Option 'polly-dependences-computeout'
registered more than once".)

| check | result |
|---|---|
| `verify/run.sh parallel` | **64 PASS / 0 FAIL / 4 SKIP** — SKIPs are measured: `scaling/doall_scaling` (quota 6 CPUs < nproc 48), `parallel/nested_step`, `race/marker_derived`, `race/derived_default` (final_program SIGKILLed rc=137 — container memory limit) |
| `test/run_exec_engine_tests.sh` | exec engine 0 failures at 1/3/8; dag scheduler ALL PASS (incl. the 17 new §11 checks) |
| overflow fail-closed (`SGPL_REL_CAP=2`, `mixed_regions`) | serial == unrewritten (`tot 340000`), `impl_failure=1`, "relation overflow" traced |
| adversarial completeness (`maycross_same_region`) | serial == rewritten at 1 and 4 threads (`s 20248`), refusal emitted, certificate clean (`R_S=0 R_T=0`), strictness delta reported |

<!--POD-DEVICE-->
| device diff `carried_read_state` | P=1/4/8 **identical**, 16 device dispatches / 1 CPU fallback |
| device diff `dual_same_base` | P=1/4/8 **identical**, 0 device dispatches (the staged path emits no kernel — by design) |
| device diff `shadow_snapshot` | P=1/4/8 **identical**, 16 dispatches / 1 fallback |
| device diff `maycross_same_region` | P=1/4/8 **identical**, **0 device dispatches** — the proof-refused spatial axis also denies the device step (`sgpl_gpu_step_schedule_ok`) |
| `gpu_corpus_diff.sh` | **85 rows, 0 DIFF** (79 identical device-on == device-off), 6 rows `SIGKILL(rc=137: container memory limit)` (pagerank, dg_nbr_keyed, edge_write_v, edge_write_v5, edge_write_v_rev, nested_step — each attributed), **0 BUILD-FAILED** |

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
