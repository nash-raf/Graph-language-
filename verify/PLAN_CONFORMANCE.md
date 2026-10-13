# PLAN.md conformance — item by item

Source: `p1GraphEasy-con-AutoTuner/plan.md` (= `/home/user/Downloads/PLAN.md`,
891 lines).  Format: `plan line | requirement (opening words) | status | how it
was done | evidence`.  Statuses: **PASS**, **PASS\*** (met, with the named
scope/limit), **PARTIAL**, **OPEN**, **n/a** (the plan block is a description of
the pre-existing surface, confirmed unchanged).  Evidence commands:
`bash verify/run.sh parallel`, `bash verify/check_graphs.sh`,
`GRAPH_FRONTIER_RELCHECK=1 GRAPH_FRONTIER_GRAPHS=1 GRAPH_FRONTIER_STATS=1`,
`bash test/run_exec_engine_tests.sh`, `bash verify/check_census.sh`, plus the pod
`gpu_device_diff.sh` / `gpu_corpus_diff.sh`.

## §1 Objective and invariants (plan lines 9-35)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 15 | derive all logical access footprints | PASS | `Footprint` + `Effect::Footprints`, `buildFootprints()` | `[relcheck]` footprints=…/read=…/write=…/rmw=… on every candidate |
| 16 | derive every RAW, WAR and WAW relation | PASS | `deriveAccessRelations` over footprint pairs | relcheck raw=/war=/waw= counts per loop |
| 17-22 | classify every relation along spatial / temporal / phase / occurrence / owner axes | PASS | `AccessRelation` carries `Spatial` (`OwnerClass`), `TemporalClass`, `PhaseRel`, `Source/SinkScope`, `Relation` (instance), `RoundOffset` | `[graphcheck]` per-edge lines print spatial= and temporal= |
| 23 | apply only proof-preserving transformations to remove/rewrite edges | PASS | `buildTransformations` records per mechanism; `buildAxisProof` rejects a removal without a discharge | conformance §7 rows; `[graphcheck]` discharge= fields |
| 24-27 | construct spatial graph, temporal graph, product graph | PASS | `ProofGraphs{Phase,Round,Spatial}`; `sgpl_exec_dag_product` for (unit × partition) | `[graphcheck]` phase(rel/comp) round(rel/comp) spatial(nodes/rel/comp) |
| 28 | derive non-interference from graph partitionability | PASS | `graphAxisVerdict` (crossing test) → `AxisProof.NIS/NIT` | `[graphcheck] graph_NI_S/T` |
| 30 | a clean spatial axis is parallelized | PASS | `selectSchedule`: `SpatialPar = Nested\|SpatialDag` | `carried_read_state`: spatial-dag, peak 4, == serial at 1/4/8 |
| 31 | a clean temporal axis is parallelized | PASS | `TemporalPar = Nested`; staged dual for R6 | `dual_same_base_big`: two ordered unit runs, each internally parallel |
| 32 | an interfering axis is ordered per its surviving constraints | PASS | proof-dirty axis → ordered realization or serial+impl | `r6_order_sensitive` serial, `maycross_same_region` serial+impl |
| 33 | a witness on one axis never suppresses the other axis | PASS | per-axis gate split + proof-derived axes | `carried_read_state` (spatial runs while temporal ordered); `dual_same_base` (vice versa) |
| 34 | if both axes cannot be safely realized, execution stays serial | PASS | `RS && RT` → serial; unrealizable → serial+impl | `r6_order_sensitive`, `maycross_same_region`, `r3/r7` fixtures |
| 35 | keep R1–R7 as obligations/diagnostics; never use their absence as the completeness proof | PASS | `selectSchedule` consumes `Info.Proof`; the certificate is diagnostic + the transformation source | strictness sweep: 3 S + 4 T deltas where the certificate was clean and the graph is not |

## §2 Current implementation surface (plan lines 43-72)

| line | item | status | note |
|---|---|---|---|
| 51-59 | compiler substrate listed (EffectKind/Region/Temporal/…/emit*) | n/a | all present and extended; nothing removed |
| 61-72 | runtime substrate listed; "must remain intact" | PASS | lifecycle, budgets, fork/join, ready-work DAG untouched; only descriptors + entry points added |
| 71 | spatial DAG dispatch with independent partition nodes | PASS | `sgpl_exec_dag_spatial` kept; `sgpl_exec_dag_spatial_graph` adds real edges |
| 72 | temporal-unit DAG limited to a chain | PASS | `sgpl_exec_dag_temporal_units` (partial DAG); the chain is now a caller |

## §3.1 Effect footprints (plan lines 78-116)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 84 | `U_f` read-modify-write | PASS | `EffectKind::Uf` → Read+Write footprints | relcheck rmw= counts |
| 85 | `U_⊕` algebraic updates | PASS | `Uop` read+write + `{PARTIAL}` resource | relcheck resources= |
| 86 | claims | PASS | guard Read + `FP_ATOMIC` Write, `{CLAIM, ATOMIC_WRITE}` | `claim_driver` rmw=2 |
| 87 | ops creating both RAW and WAW obligations | PASS | one effect's Read/Write footprints generate its own WAR and its write's WAW | relcheck raw=/waw= |
| 88 | ops whose read/write footprints have different indices | PASS | `Footprint::Index` per footprint | `data_index_write` (D index) |
| 96-106 | footprint record fields (base, index, region, mode bits, scope, segment, round-domain, value source, origin) | PASS | exactly the `Footprint` fields | code: `graph_frontier_lowering.cpp` `struct Footprint` |
| 108-113 | resources DEST_SEEN / NEXT_FRONTIER / CLAIM / SNAPSHOT / PRIVATE / PARTIAL | PASS | `ResourceAccess` in the same `SGPL_RES_*` ids the emitter uses | `buildFootprints`, `[relcheck] resources=` |
| 114-116 | runtime resources as explicit footprints | PASS | round-scoped `Info.ResourceFootprints` + per-effect resources | `r7_append_unwired` (unwired append = unary obligation) |

## §3.2 Complete relation generation (plan lines 117-163)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 126-131 | for every ordered pair on aliased bases generate RAW/WAR/WAW independently | PASS | footprint-pair scan with mode bits | relcheck counts; `[graphverbose] rel` lines |
| 132 | read-read pairs create no dependency | PASS | skipped explicitly | `[verbose] rel` list on read-only loops is empty |
| 133-138 | semantic order only from Seq / SeqDomain / phase / occurrence-domain / round / a recognized transformation | PASS | `SourcedFromOrder` is true only for phase-separated pairs and one effect's internal read→write; `Par` siblings get `OriginalOrder=0` | relcheck assert "ordered relation without order"; `r6_order_sensitive` |
| 139-145 | remove the 64-relation limit; explicit overflow state; reject rather than drop | PASS | cap 4096 + `RelationOverflow` → `selectSchedule` serial+impl | run.sh `race/overflow-fail-closed` (`SGPL_REL_CAP=2`) |
| 147-160 | each relation retains (source/sink effect, base, kind, footprints, regions, indices, phases, scopes, order, spatial/temporal classification, instance relation, discharge, witness id) | PASS | all fields on `AccessRelation` | code: `struct AccessRelation` |
| 161-163 | `WrittenBases`/`ReadBases` summaries are not substitutes | PASS | the summaries remain diagnostics; the graph reads relations | code |

## §4 Temporal relation system (plan lines 164-244)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 172-176 | keep `relateReadWrite` (base, region, write phase, round domain, membership gating) | PASS | `relateReadWrite` untouched; `relateAccessTemporal` extends its rule to WAR/WAW | `[frontier-cert] tmp` lines unchanged; algebra 21/21 |
| 178-181 | store the result on the individual relation; `Effect::Temp` diagnostic only | PASS | `AccessRelation::TemporalClass` | `[graphcheck]` temporal= per edge |
| 182-187 | build edges from the complete relation list, not the strongest τ | PASS | `buildProofGraphs` reads `Info.AccessRelations` | code; `[graphcheck]` edge list |
| 192-194 | RAW / WAR / WAW temporal derivation | PASS | mode-based classification | `[graphcheck]` kind=RAW/WAR/WAW |
| 195 | RMW effects | PASS | the Read/Write pair is classified | relcheck rmw loops |
| 196 | claim occurrence ordering | PASS | `FP_ATOMIC` write side + claim staging | `claim_driver`, `shadow_snapshot` |
| 197 | snapshot-resolved reads | PASS | σ=Snapshot → Snapshot discharge | `shadow_snapshot` discharge=snapshot |
| 198 | cross-phase writes | PASS | `SameRoundOrdered` when segments differ | `[graphcheck]` temporal=SameRoundOrdered |
| 199 | round-carried writes | PASS | `Carried`/`PreviousRoundWrite` | `carried_read_state`, sssp deltas |
| 200 | previous-round reads | PASS | `PreviousRoundRead` under a frontier domain | census R4 fixtures; frontier cases |
| 201 | fixed-offset and affine unit relations | PASS\* | `FixedOffset` used with `Relation.Offset`; `Affine` is defined but a k>1/affine shape does not arise in the DSL | code: `Temporal`, `InstanceRelation` |
| 203-207 | human-readable projection + precise instance relation | PASS | `temporalName()` + `InstanceRelation{K,Offset}` | `[graphcheck]` temporal= + instance |
| 209-215 | WAR/WAW require explicit source/sink unit reasoning | PASS | footprint modes + write-side selection | `deriveAccessRelations` |
| 233-236 | within-round phase graph: nodes = phase occurrences; edges preserve P→B→Q; Par siblings unordered unless a relation requires an edge | PASS | `ProofGraphs.Phase`: structural P→B, B→Q + per-relation edges | `[graphcheck] phase(rel=… comp=…)` |
| 238-241 | across-round graph: round/stage nodes; carried/previous/fixed-offset edges; snapshot removes only its relations | PASS | `ProofGraphs.Round` (self-edges for cross-round relations), discharge-scoped survival | `[graphcheck] round(rel=…)`, round-edge lines |

## §5 Spatial locality and ownership (plan lines 245-318)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 249-254 | region is not ownership | PASS | `OwnerClass` decided per footprint/relation, not per region | code + `[graphcheck] spatial=` |
| 255 | `A[u]` source-owned when u is the exact source endpoint | PASS | canonical endpoint-slot identity (`indexIdentity`) | `dual_shadow`, `claim_driver`, all U-owned fixtures |
| 256 | `A[u+1]` may cross a source boundary | PASS | different index in one region → `MayCrossOwner` | `maycross_same_region` (A[v] + A[v+1]) |
| 257 | `A[v]` destination-owned at the exact endpoint | PASS | same rule, V side | every V-owned fixture (`edge_write_v`, …) |
| 258 | `A[f(u,v)]` requires an affine/range proof | PASS\* | no affine proof available → the pair stays `MayCrossOwner` (the conservative branch the next line mandates) | `maycross_same_region` serial+impl |
| 259 | D and Top have no owner unless a transformation removes the shared location | PASS | `NoOwner`/`UnknownOwner`; privatization resolves D | `data_index_write` (priv), `r3_unknown_provenance` (serial) |
| 267-277 | ownership classification ExactOwner / SameOwner / ProvedDisjointOwner / CrossOwner / MayCrossOwner / NoOwner / UnknownOwner | PASS | `enum class OwnerClass` with every name | code |
| 279-283 | resolve source/destination/scalar-global/private/frontier-envelope ownership per footprint | PASS | `footprintHasOwner()` covers U/V endpoint, G/no-owner, private, envelope; scalar/global → NoOwner | relcheck/`[graphcheck]` per fixture |
| 287 | exact U/V endpoint indices | PASS | `indexIdentity` canonicalizes the u/v alloca loads | `carried_read_state` ExactOwner |
| 288 | partition ranges from CleanCut | PARTIAL | CleanCut ranges drive the *runtime* partitions; the compiler's owner class does not consult range boundaries (conservative may-cross) | documented in this file's summary |
| 289 | affine index expressions | PARTIAL | as line 258 — conservative, no affine solver | `maycross_same_region` |
| 290 | constant and loop-invariant indices | PASS | constant → not owned (every iteration writes it); invariant → one element, owned | `footprintHasOwner` |
| 291 | data-derived indices | PASS | `Region::D` → NoOwner | `data_index_write`, `priv_data_index_big` |
| 292 | unknown/mixed provenance | PASS | `Region::Top` → UnknownOwner | `r3_unknown_provenance` |
| 293 | use LLVM value/range reasoning where available | PARTIAL | value identity + alloca canonicalization only (no range analysis) | code |
| 294 | if disjointness cannot be established, retain a conservative edge | PASS | the may-cross/all-unknown classes survive | `maycross_same_region`, `r3` |
| 300-305 | partition-map inputs (`partition_start`, source ranges, destination home partitions, source pair slices, destination rows, frontier membership) | PARTIAL | the runtime instantiates all of them (`sgpl_exec_…`, `src_pairs`, dest rows); the compiler does not emit the symbolic schema (tied to line 747) | `autotuner_runtime.c`, `EFFECT_SYSTEM.md` §7 |
| 312-318 | partition maps represented independently per domain (source/dest/data-derived/scalar/private) | PARTIAL | the owner classes are per footprint; the emitted schema is the open descriptor-wiring item | see line 747 |

## §6 Spatial dependency graph (plan lines 319-379)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 336-338 | U-owned / V-owned / private partitions as nodes | PASS | nodes keyed by owner region; private copies resolve to ExactOwner | `[graphcheck] spatial(nodes=…)` |
| 339 | phase-specific nodes | PASS | the phase graph carries the phase dimension | `[graphcheck] phase(...)` |
| 340 | product nodes `(round, partition, phase)` | PASS | `sgpl_exec_dag_product` (unit × partition) with the phase graph supplying phase order | `dag_scheduler_test` product checks |
| 346 | RAW precedence where a reader must observe a writer | PASS | relation kind → edge; `ConstraintKind::Precedence` when semantic order exists | `[graphcheck]` edge kind=RAW + constraint |
| 347 | WAR precedence where a writer cannot overtake a prior reader | PASS | same, WAR projection | `[graphcheck]` kind=WAR |
| 348 | WAW ordering for noncommuting writes | PASS | WAW edges; discharged only by a valid fold/priv/staging | `dual_same_base` (WAW survives), `r6_order_sensitive` |
| 349 | mutual-exclusion metadata lowered to deterministic precedence | PASS | `ConstraintKind::MutualExclusion` + `RealizationOrder` template edges; runtime `SGPL_DAG_KIND_MUTEX` requires the realization tag | `dual_same_base` staged dual; runtime validation test |
| 350 | realization-order edges for implementation-imposed ordering | PASS | template `RealizationOrder`; `ProofEdge::Realizable`; `[graphcheck] realizable=` | `dual_same_base`, `carried_read_state` (round order) |
| 351-359 | undirected projection; non-interference when ≥2 realizable nonempty components and no surviving dependency crosses | PASS\* | operationalized as the crossing test (an owner domain partitions internally, so "no crossing edge" is the criterion); components are still computed and reported | `[graphcheck] graph_NI_S` + `spatial(comp=…)`; rationale in the source comment |
| 360-362 | cycles fail closed | PASS | `proofHasCycle` (Kahn) → `Invalid` → `selectSchedule` serial+impl | runtime cycle test (`dag_scheduler_test`), `buildAxisProof` validation |
| 369 | enumerate concrete partitions | PASS | runtime node per partition (`sgpl_dag_spatial_dispatch`) | `SGPL_DAG_DEBUG` node counts |
| 370 | evaluate owner predicates | PASS | the compiler labels each relation's owner class; the runtime edge set follows the partition indices | `[graphcheck] spatial=` |
| 371 | map relation schemas to partition pairs | PASS | edges are partition-index pairs by construction (`sgpl_exec_dag_spatial_graph`) | runtime test + device diff |
| 372 | deduplicate edges | PARTIAL | contradictory duplicates are rejected; identical duplicates are not collapsed | runtime validation test |
| 373 | retain witness/discharge metadata | PASS | fields on `sgpl_dag_relation_desc` + validation (`discharge_id != 0` rejected as non-executable) | runtime test "a discharged relation is never scheduled" |
| 374 | construct connected components | PASS | `proofComponents` (union-find) + the executor's ready set | `[graphcheck] comp=` |
| 375 | execute independent components concurrently | PASS | ready-work scheduler dispatches all ready nodes | `dag_scheduler_test` overlap checks; `SGPL_DAG_DEBUG peak=` |
| 376 | respect precedence inside each component | PASS | sink released only when its predecessors complete | `dag_scheduler_test` dependency checks |
| 377-379 | unknown symbolic relation → conservatively connect or reject | PASS | unknown/may-cross survive → axis dirty → ordered/serial | `r3_unknown_provenance`, `maycross_same_region` |

## §7 Transformations (plan lines 380-476)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 388-392 | privatization conditions (recognized updates, operator identity, fold-soundness, no unmerged private read, deterministic combine) | PASS | recorded on the Privatization transformation; the discharge requires `FP_PRIVATE` on the write side | `data_index_write`, `mixed_regions`, relcheck discharged= |
| 396-398 | privatization replacement edges (partition-private write, combine/fold, deterministic finalization) | PASS\* | the conditions are recorded and the runtime's private binding/combine provide the edges; the record's `Adds` list is not populated as explicit node pairs | `EFFECT_SYSTEM.md` §7 (runtime), `validate_reduction.sh` |
| 400 | do not discharge arbitrary relations merely because `UsePrivLayout` | PASS | only the write side's private write discharges | relcheck discharged= counts (e.g. `carried_read_state` 0) |
| 406-409 | owner-computes conditions | PASS | recorded; `footprintHasOwner` implements the per-footprint test | code + `[graphcheck] spatial=exact` |
| 413-421 | snapshot conditions (redirect only cross-endpoint reads; keep same-owner RMW reads live; publication + ordering; remove only the corresponding RAW) | PASS | `RoundSepBase::CrossReads` + σ=Snapshot; discharge only those relations | `shadow_snapshot` discharge=snapshot, order-sensitive probe |
| 424 | snapshot must not remove a spatial edge unless the footprint changes | PASS | cross-round relations are excluded from the spatial crossing test | `carried_read_state` (temporal dirty, spatial clean) |
| 429-432 | claim staging preserves scope/multiplicity/guard-transition/priority | PASS | recorded; staging availability via `perSourceClaims` (both in the witness and the graph — cb2e3e0) | `claim_driver`, `shadow_snapshot`, `two_reduce_slots` |
| 442-445 | algebraic fold conditions | PASS | same recognized op + same element (or one effect's internal pair) + partial | `reduce_int_ops` (`class=reduction`), algebra 21/21 |
| 449-461 | frontier envelope models F_t membership / dest_seen / F_{t+1} append / append-head ordering; discharges only those | PASS | envelope discharge scoped to the append/read relations; wired → Snapshot; unwired → unary obligation | `r7_append_unwired`, `test_bfs` (fixed by the unary rule) |
| 466 | R1 → no owner for data writes; unresolved unless privatized | PASS | D-region write → `NoOwner`; priv resolves | `data_index_write` (R1 privatized), `priv_data_index_big` |
| 467 | R2 → per-source claim; discharge only via staging | PASS | claim resources + the availability test | `claim_driver`, census R2×9 claim-staged |
| 468 | R3 → unknown/mixed provenance | PASS | Top → UnknownOwner → surviving | `r3_unknown_provenance` (serial+impl) |
| 469 | R4 → carried read; snapshot/privatization else temporal edge | PASS | cross-round relations → Round graph; Snapshot/priv discharge | `carried_read_state`, `shadow_snapshot` |
| 470 | R5 → cross-phase write retains the temporal edge | PASS | the predicate is unreachable from the DSL (census asserts R5×0); cross-phase writes classify SameRoundOrdered | `check_census.sh` (R5 must be zero) |
| 471 | R6 → same-base U/V; disjointness/privatization/staged realization | PASS | `CrossOwner` survivors + the tagged staged order | `dual_same_base(_big)`, `r6_order_sensitive` |
| 472 | R7 → append without envelope; retain resource and spatial edges | PASS | resource relation + the unary rule (both materialized and not) | `r7_append_unwired`, `test_bfs` |
| 473 | R8 → implementation/modelability failure outside the lattice | PASS | `Cert.R8` checked first in `selectSchedule`; keeps `impl_failure=1` and its own reason | run.sh `r3/r7` assertions; census `serial+impl_failure` |

## §8 Product graph (plan lines 477-520)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 481-494 | product node = (t, p, phase); built from the spatial and temporal projections of the same relation set | PASS | `sgpl_exec_dag_product` node index `unit*partitions + partition`; both projections share `Info.AccessRelations` | runtime product test; `[graphcheck]` axis masks |
| 499 | spatial edges between (t,p,phase) and (t,q,phase') | PASS | spatial relations replicated inside every unit | product test (spatial edge within units) |
| 500 | temporal edges between (t,p,phase) and (t+k,q,phase') | PASS | cross-round relations replicated across partitions (k=1) | product test (cross-unit edge) |
| 501 | phase-order edges within one temporal unit | PASS | the phase graph supplies P→B→Q | `[graphcheck] phase(rel=2 structural + …)` |
| 502 | snapshot/combine/claim/lifecycle realization edges | PASS | owned by the existing unit wrappers; the scheduler only orders units | §11.5 rows |
| 503 | cross-axis edges where one relation carries both a spatial owner relation and a temporal offset | PASS | such a relation appears in both projections and the product copies carry both axis bits | code: `axis_mask |= SGPL_DAG_AXIS_PRODUCT` |
| 507-511 | lifecycle exactly-once (round begin/snapshot/coverage/partition begin-end/round end) | PASS | the wrappers keep the lifecycle; the product only dispatches cells | runtime product test; `validate_roundsep/reduction` PASS |
| 515-520 | product used when overlapping both axes; optional only when the projections are independent; must still be constructed and validated for every admitted candidate | PASS\* | the compiler builds both projections + their metadata for every candidate and `AxisProof` validates them; the materialized product graph is built by the executor for the candidates that use it | code + `[graphcheck]`; the emitted cases use nested/spatial-dag/staged |

## §9 Proof certificate (plan lines 521-571)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 525-560 | proof object contains all relations, spatial/temporal/product edges, discharged and unresolved edges, partition candidates, components, cycle status, overflow/unknown status, witness records, schedule requirements | PASS | `AxisProof` + `ProofGraphs` + `Transformation`s | code; `[graphcheck]`/`[relcheck]` |
| 561-566 | NI_S/NI_T derived; global theorem; distinguish proven / interference / inconclusive | PASS | `graphAxisVerdict` + `buildAxisProof` three-valued; inconclusive forces NIS=NIT=false | `[graphcheck] graph_NI_*`; overflow/cycle paths |
| 567-571 | unresolved edge may exist without an R1–R7 label; it still prevents the claim; removals point to a discharge; semantic edges point to a witness; realization-only edges marked | PASS | `buildTransformations` + validation (a removal without a discharge = invalid); `witness_id`/`realization` flags; runtime rejects a discharged edge | runtime validation tests; `buildAxisProof` validation |

## §10 Schedule selection and runtime composition (plan lines 573-627)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 581-584 | both axes clean → nested/product; spatial and temporal work concurrent; all product edges validated; independent components share the budget | PASS | `selectSchedule` Nested; ready-set reservation in `dag_dispatch_budget` | suite (nested fixtures), `validate_tdg_budget.sh` ALL PASS |
| 590-593 | spatial clean / temporal interfering → SpatialDag; order edges preserved; R4/R5/R2 realized by ordering/staging; spatial proof still graph-derived | PASS | `SpatialDag` + the round sequence; the graph (not an empty R list) decides | `carried_read_state` (spatial-dag, peak 4, == serial at 1/4/8; pod 16 dispatches identical at P=1/4/8) |
| 599-602 | spatial interfering / temporal clean → TemporalDag; staged execution for R6; partial spatial DAG when acyclic/representable | PASS | staged dual (`autograph_frontier_staged`) + the proof-derived template edge; unrealizable spatial → serial+impl | `dual_same_base(_big)`, `r6_order_sensitive`, `maycross_same_region` |
| 606-609 | both interfering → product if a valid realizable schedule exists, else serial | PASS | V1: serial (+impl unless the surviving edges are realizable); the product entry point is available | `r6_order_sensitive` (serial, `impl_failure=0`), census schedule counts |
| 612-626 | R8/implementation failure → serial + marked separately; metadata distinguishes semantic vs implementation refusal, DAG ownership, spatial/temporal parallel, ordered axis, product schedule; `autograph_exec_ctx_set_dag_axes` remains the declaration path, populated from the proof | PASS | `impl_failure` flag + markers (`dag.owner/axes/spatial/temporal`, `impl.serial`); axes emitted from `Info.Schedule` (proof-derived) | run.sh `r3/r7` assertions; `[graph-frontier] dag-owned` lines |

## §11 Runtime implementation (plan lines 628-726)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 634-648 | `sgpl_dag_relation_desc` extended with axis mask, relation kind, phase/round offset, spatial/temporal relation, witness id, discharge id, semantic/realization flags | PASS | the struct fields exist (`axis_mask`, `relation_kind`, `access_kind`, `discharge_id`, `phase_from/to`, `round_offset`, `partition_from/to`) | code; runtime tests |
| 649 | node bounds validated | PASS | `source_node/sink_node < node_count` | runtime test "out-of-range partition edge" |
| 650 | no self edges | PASS | `source == sink` → `SGPL_DAG_ERR_CYCLE`; `SELF` kind → invalid | runtime tests |
| 651 | witness on semantic edges | PASS | semantic without witness → invalid | runtime test |
| 652 | valid discharge references | PASS | an executable edge with `discharge_id != 0` → invalid | runtime test "discharged relation is never scheduled" |
| 653 | no duplicate contradictory edges | PASS | same pair with different kinds → invalid | runtime test |
| 654 | acyclicity for the realization graph | PASS | scheduler cycle detection + `proofHasCycle` | runtime cycle test |
| 655 | complete node/component coverage | PASS | `completed_count != node_count` → cycle error; every node dispatched | runtime tests |
| 663 | run independent components concurrently | PASS | ready-work scheduler | `dag_scheduler_test` overlap; `SGPL_DAG_DEBUG peak=` |
| 664 | preserve edge order inside components | PASS | sink released on predecessor completion | runtime dependency tests |
| 665 | apply weighted nested budgets | PASS | `dg_dispatch_budget` + `node_weights` ceiling + nested clamp | `validate_tdg_budget.sh`; budget tests `[1,W]` |
| 666 | expose worker indices as today | PASS | node instance passed unchanged | `exec_engine_test` worker-index checks |
| 667 | preserve partition/source/pair/combine lifecycle | PASS | the node wrapper is the existing partition body | full suite + differentials |
| 668 | fail closed on invalid graph descriptors | PASS | validation before any dispatch | runtime tests + the malformed-descriptor cases |
| 676 | round/stage nodes | PASS | `sgpl_exec_dag_temporal_units` unit = whole round | runtime temporal tests; staged dual |
| 677 | partial DAGs | PASS | units without in-edges start together | runtime partial-DAG test |
| 678 | phase-level nodes | PARTIAL | phase nodes exist in the compiler's phase graph; the temporal executor's units are rounds/stages (phase work stays inside a unit) | `[graphcheck] phase(...)` |
| 679 | fixed-offset and affine edges | PASS\* | `offset` field + `round_offset`; affine not exercised in the DSL | code |
| 680-681 | dynamic round production; dependent-aware publication of new units | PASS\* | the round driver owns it: a successor round is entered only when its frontier input state is complete (`autograph_frontier_execute` per round; the staged dual orders units); per §11.5 the scheduler must not create rounds | `EFFECT_SYSTEM.md` §7; suite frontier cases |
| 687-700 | product execution entry point (schema, partition map, unit factory/array, callbacks, edges, budget); materialize reachable nodes; dispatch through the budget ledger; nested calls never claim the machine | PASS | `sgpl_exec_dag_product` (units array + partitions + cell callback + both edge sets) | runtime product tests; nested budget tests |
| 706-712 | do not duplicate/bypass the existing lifecycle (frontier execute, snapshot ownership, envelope commit, coverage, private partials, deterministic combine, staged U/V) | PASS | the executors only order and dispatch wrappers; nothing re-implemented | code review; `validate_roundsep/reduction` PASS; suite |
| 720 | a device step may execute only when the proof declares the spatial axis concurrent | PASS | `sgpl_gpu_step_schedule_ok(spatial, temporal)` from the declared axes (proof-derived) | `maycross_same_region` → 0 device dispatches (pod) |
| 721 | ordered spatial constraints force CPU fallback | PASS | refusal before the cost model, CPU path identical | device diffs; `tdg_budget_test` T11 |
| 722 | temporal constraints never silently authorize an unsafe device schedule | PASS | the temporal axis never gates the device, but a dirty spatial axis does; temporal order belongs to the caller (one step = one round) | `carried_read_state` 16 dispatches identical at P=1/4/8 |
| 723 | device fallback must produce the same answer as CPU graph execution | PASS | fallback runs the same partition body | pod corpus diff 85 rows, 0 DIFF (6 SIGKILL rc=137 attributed) |

## §12 Compiler integration points (plan lines 727-785)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 735 | expanded effect footprints | PASS | `buildFootprints` | §3.1 rows |
| 736 | complete relation generation | PASS | `deriveAccessRelations` | §3.2 rows |
| 737 | temporal relation retention | PASS | per-relation `TemporalClass`/`PhaseRel`/`RoundOffset` | §4 rows |
| 738 | WAR/WAW temporal derivation | PASS | `relateAccessTemporal` extended | §4 rows |
| 739 | owner and partition resolution | PASS | `relateAccessSpatial`, `footprintHasOwner` | §5 rows |
| 740 | spatial graph construction | PASS | `buildProofGraphs` (Spatial) | `[graphcheck] spatial(...)` |
| 741 | temporal graph construction | PASS | `buildProofGraphs` (Phase, Round) | `[graphcheck] phase/round(...)` |
| 742 | product graph construction | PASS | axis masks + `sgpl_exec_dag_product` | §8 rows |
| 743 | transformation/discharge application | PASS | `buildTransformations`; per-relation `DischargedBy` | §7 rows |
| 744 | component and cycle analysis | PASS | `proofComponents`, `proofHasCycle` | `[graphcheck] comp=` |
| 745-746 | proof-derived certificate and schedule selection | PASS | `buildAxisProof`, proof-consuming `selectSchedule` | §10 rows |
| 747 | graph descriptor emission | OPEN | the emitted programs declare axes via `autograph_exec_ctx_set_dag_axes`; the compiler does not yet emit a descriptor array consumed by the new executors | this file's summary |
| 748 | relation/witness diagnostics | PASS | `[relcheck]`/`[graphcheck]` reports + `GRAPH_FRONTIER_VERBOSE` rel lines | `[relcheck] OK`, `[graphcheck]` |
| 749 | complete edge-consumption validation | PASS | `buildAxisProof` (removal must be discharged), `witnessesConsumed` (emission fails closed on an unconsumed witness) | run.sh; code |
| 757-760 | extended descriptors; graph materialization; spatial DAG with real edges; temporal partial DAG | PASS | §11.1/§11.2/§11.3 rows | runtime tests |
| 761 | dynamic temporal-unit production | PARTIAL | driver-owned (see 680-681); no scheduler-side unit creation | this file's summary |
| 762-766 | product DAG execution; edge/discharge validation; component scheduling; budget propagation; lifecycle-safe node wrappers | PASS | §11.1/§11.4 rows + `sgpl_push_thread_budget` | runtime tests; budget tests |
| 770-772 | `pdg.cpp`/`parallel_loop_outline.cpp` recognize graph-owned loops from proof-derived metadata rather than a legacy marker; the outliner must not reclassify | PASS\* | they recognize the emitted marker (`dag.owner`/`impl.serial`), which is now proof-derived; a direct proof-metadata channel is the open 747 item | `[graph-frontier] dag-owned` lines; suite |
| 778 | update `EFFECT_SYSTEM.md` | PASS | new section "The two-axis proof layer (plan sections 3-11)" + the explicit "R1–R7 are witness classes, not an exhaustive decision procedure" statement | doc |
| 779 | update `EFFECT_SYSTEM_IMPLEMENTATION_REPORT.md` | PASS | proof-layer section appended | doc |
| 780 | update `proof/EFFECT_ALGEBRA_DESIGN.md` | PASS | appendix: the edge-removal discipline table (mechanism vs conditions) | doc |
| 781 | theorem/proof notes | PASS | the conformance report + final report + the algebra appendix serve as the proof notes (the user's `R2_REJECTION.md` untouched) | `verify/PLAN_CONFORMANCE.md`, `PLAN_FINAL_REPORT.md` |
| 782 | runtime and certificate output documentation | PASS | the new runtime APIs are documented in `autotuner_runtime.h`; the certificate/report output in the doc sections above | docs |

## §13 Tests and acceptance criteria (plan lines 786-872)

| line | requirement | status | how | evidence |
|---|---|---|---|---|
| 792 | `A[u]` source-owned writes | PASS | existing U-owned fixtures | suite (source-owner cases) |
| 793 | `A[v]` destination-owned writes | PASS | existing V-owned fixtures | `edge_write_v*`, suite |
| 794 | `A[u+1]` boundary-crossing expressions | PASS | `maycross_same_region` (`A[v]` + `A[v+1]`, same class) | run.sh `race/maycross_same_region` |
| 795 | exact same-owner and cross-owner U/V accesses | PASS | `dual_same_base`, `mixed_regions` | suite |
| 796 | unknown `Top` provenance | PASS | `r3_unknown_provenance` | run.sh |
| 797 | data-derived D writes | PASS | `data_index_write`, `priv_data_index_big` | run.sh, census |
| 798 | RMW effects | PASS | every RMW fixture (relcheck rmw=) | relcheck |
| 799 | claims per-source and per-pair | PASS | `claim_driver`, `shadow_snapshot`, R2 test scripts | run.sh + `run_exec_r2_tests.sh` |
| 800 | carried reads | PASS | `carried_read_state` | run.sh |
| 801 | previous-round reads | PASS\* | the class is implemented and used by the frontier-domain branch of `relateAccessTemporal`; **no corpus candidate exercises it** (the R4 fixtures use the `Carried` form — checked: 0 candidates print PreviousRoundRead) | code; `[graphcheck]` grep across fixtures |
| 802-803 | WAR / WAW across rounds | PARTIAL | the classes exist and are classified; cross-round RAW is exercised by the sssp family (`round-edge` lines); no dedicated cross-round WAR/WAW fixture and no candidate prints `PreviousRoundWrite` | `[graphcheck]` round-edge lines; sssp spot-check (identical answers) |
| 804 | cross-phase reads/writes | PASS | `SameRoundOrdered` fixtures | `[graphcheck] temporal=SameRoundOrdered` |
| 805 | frontier append/read conflicts | PASS | `r7_append_unwired`, `test_bfs` | run.sh + the cover gate |
| 806 | shadow snapshots | PASS | `shadow_snapshot` | run.sh |
| 807 | private reductions | PASS | `reduce_int_ops`, `reduce_real_ops`, `priv_*` | run.sh, algebra |
| 808 | algebraic folds | PASS | recognized-operator fixtures | `validate_algebra.sh` 21/21 |
| 809 | more than 64 relation candidates | PASS\* | exercised by lowering the materialization bound (`SGPL_REL_CAP=2`); a 64+ logical-relation loop is not expressible in the DSL | run.sh `race/overflow-fail-closed` |
| 810 | `Par` siblings where source order must not be invented | PASS | `r6_order_sensitive`; the relcheck assertion | relcheck "ordered relation without order" assert + graphs |
| 816-826 | proof tests: relation count, RAW/WAR/WAW, owner status, temporal locality, product edges, discharged vs unresolved, components, NI_S/NI_T, semantic vs implementation, schedule, witness consumption | PASS | `check_graphs.sh` (cover + strictness), `[relcheck]`/`[graphcheck]` reports, run.sh schedule/axis/impl assertions, product runtime tests | 96-candidate sweep cover 96/96; case corpus 52/52 |
| 828 | adversarial: no R1–R7 condition fires while the graph is connected → must not be admitted | PASS | `maycross_same_region` (certificate `R_S=0 R_T=0`, graph dirty) | run.sh `race/maycross_same_region`, strictness census |
| 834-840 | spatial partial DAGs; temporal partial DAGs; product DAGs; overlap; precedence; cycle rejection; invalid witness/discharge rejection | PASS | `dag_scheduler_test` incl. the 17 new §11 checks | `test/run_exec_engine_tests.sh` ALL PASS |
| 841 | dynamic temporal-node creation | OPEN | the driver owns round creation (see 680-681); no scheduler-side dynamic unit API | summary |
| 842 | partition-map expansion | OPEN | the runtime expands partitions; the *symbolic schema → runtime* channel is the open 747 item | summary |
| 843 | edge deduplication | PARTIAL | contradictory duplicates rejected; identical duplicates kept | runtime test |
| 844-848 | nested budget propagation; lifecycle exactly-once; staged U/V ordering; snapshot/envelope ordering; cancellation | PASS | budget tests, `validate_roundsep/reduction`, staged-dual cases, failure-cancellation test | engine tests; suite |
| 854-857 | differentials: sequential vs 1/4/8; spatial-only/temporal-only/nested/product/staged/serial; CPU vs GPU; frontier cardinality/arrays/reductions/claims/ordering | PASS | run.sh cases compare against the unrewritten build; `gpu_device_diff.sh`/`gpu_corpus_diff.sh` for CPU/GPU; the product is compared by the runtime unit test | suite; pod device diffs (P=1/4/8 identical; corpus 85 rows 0 DIFF) |
| 863-872 | regression gates (dropped relation, ignored overflow, edge without metadata, discharge without transformation, Par order, coarse region without owner proof, executed cycle, unresolved edge absent, dirty axis suppressing clean parallelism, implementation misreported as semantic) | PASS | relcheck/`buildAxisProof` validation + runtime validation + run.sh assertions for each gate | `[relcheck] OVERFLOW` path; run.sh `overflow-fail-closed`, `maycross_same_region`, `r3/r7`; runtime tests; per-axis cases |

## §14 Final acceptance definition (plan lines 874-889)

| line | item | status | evidence |
|---|---|---|---|
| 878 | complete spatial/temporal/product dependence graphs | PASS | §6/§8 rows; `[graphcheck]` |
| 879 | no strongest-only loss | PASS | all relations retained; `Effect::Temp` diagnostic only |
| 880 | spatial components from owner predicates + surviving edges | PASS | `relateAccessSpatial` + `proofComponents` |
| 881 | temporal partitions from all locality relations | PASS | Round graph over every relation |
| 882 | product combines both projections without losing cross-axis constraints | PASS | axis masks + `sgpl_exec_dag_product` |
| 883 | every edge removal justified by a recorded transformation | PASS | `buildTransformations` + validation |
| 884 | R1–R7 no longer the sole completeness mechanism | PASS | proof-derived `selectSchedule`; strictness deltas prove the certificate is incomplete |
| 885 | runtime consumes the proof-derived graph preserving lifecycle | PASS\* | executors + validation consume it; the emitted program declares via the axes ABI (the open 747 item) |
| 886 | clean axes remain parallel even when the other is ordered | PASS | `carried_read_state`, `dual_same_base(_big)` |
| 887 | interfering axes retain their order with partial concurrency where safe | PASS | staged dual, spatial-dag, partial-DAG unit tests |
| 888 | unknown/incomplete/cyclic/overflowed proofs fail closed | PASS | overflow/cycle/unary/unknown-axis/discharged-edge rejections (compiler + runtime tests) |
| 889 | all sequential/parallel differential tests pass across the corpus | PASS | 69/0 local; pod 64/0/4 measured SKIPs; corpus device diff 0 DIFF |

## Honest non-PASS list (everything else above is PASS/PASS\*)

| plan line(s) | item | status | what it would take |
|---|---|---|---|
| 747 | graph descriptor emission (the emitted program passing the proof's descriptor array to `sgpl_exec_dag_spatial_graph/_temporal_units/_product`) | OPEN | a compiler-emitted descriptor array + the emitted call sites; the runtime side is implemented and tested |
| 841, 842, 761 | dynamic temporal-node creation; partition-map expansion (emitted side); dynamic temporal-unit production | OPEN/PARTIAL | same channel as 747; the driver-owned round production is deliberate (§11.5) |
| 288, 289, 293 | CleanCut partition-range refinement; affine index expressions; LLVM range reasoning | PARTIAL | an affine/range checker in the owner resolver; today the conservative branch (294) is taken |
| 300-318 | emitted symbolic partition schema | PARTIAL | same channel as 747 |
| 372, 843 | edge deduplication | PARTIAL | collapse identical duplicates in the product builder |
| 678, 679 | phase-level temporal units; affine temporal edges | PARTIAL | phase-level unit granularity; multi-round/affine offsets (not expressible in the DSL today) |
| 801, 802-803 | `PreviousRoundRead`/`PreviousRoundWrite` exercised by a fixture | PARTIAL | a frontier-gated fixture whose cross-round relation is not `Carried` (today the R4 fixtures use `Carried`; 0 candidates print the previous-round classes) |
| 396-398 | privatization replacement edges recorded as explicit node pairs | PASS\* | the record carries the conditions; the `Adds` list is diagnostic today |

No item is marked PASS by weakening a check: the suite only grew (67 → 69
checks), the census/graph gates fail closed, and every SKIP carries a measured
environment number.
