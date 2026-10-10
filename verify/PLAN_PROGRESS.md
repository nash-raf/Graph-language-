# Complete Two-Axis Interference-Proofing — progress ledger

Plan: `p1GraphEasy-con-AutoTuner/plan.md` (copy of `/home/user/Downloads/PLAN.md`,
891 lines, 14 sections).  Baseline: branch `two-axis-dag-scheduler` at `104beb9`
== `origin/ARS-1`.  Rule: every section ends with live evidence; only measured
environment limits SKIP (with the number); never weaken fail-closed.

| § | what | status | evidence |
|---|---|---|---|
| 3.1 | expanded effect footprints (read+write per effect, resource footprints in the SGPL_RES_* vocabulary) | PASS | `SmallVector<Footprint,2> Effect::Footprints`; `buildFootprints()`; `GRAPH_FRONTIER_RELCHECK=1` census on 8 fixtures all `OK` (e.g. `data_index_write` rmw=1 → Read+Write; `claim_driver` rmw=2 → guard read + atomic write; envelope/claim/snapshot/private/partial resources) |
| 3.2 | complete RAW/WAR/WAW generation, no 64-cap, overflow = reject | PASS | `deriveAccessRelations` rewritten over footprints; 4096 cap + `RelationOverflow`; RMW self-pairs produce their own RAW/WAR; witness linking orientation-insensitive; adversarial `r6_order_sensitive` now relations `discharged=0` (matches `R_S=1`) |
| 4 | temporal relations for all kinds + both scopes | PASS | `Temporal` extended (SameRoundWrite/SameRoundOrdered/PreviousRoundWrite/FixedOffset/Affine/Unknown) + `PhaseRelation`/`RoundOffset`/`InstanceRelation` per relation; `relateAccessTemporal` covers WAR/WAW/RMW/claims/snapshot reads/cross-phase/cross-round; within-round phase graph + across-round round graph built (`ProofGraphs`) |
| 5 | owner resolution + partition maps | PARTIAL | `OwnerClass` live per relation/footprint: exact endpoint-slot identity (canonicalized across the U/V alloca loads) = `ExactOwner`; different index in one region = `MayCrossOwner`; cross-endpoint = `CrossOwner`; data/global = `NoOwner`; Top = `UnknownOwner`; `footprintHasOwner()` resolves private copies, recognized-fold partials, staged claims and the wired envelope (§7.6), and rejects constant-index writes (a constant element is written by every iteration). CleanCut partition-range refinement + the emitted symbolic partition schema still open |
| 6 | spatial dependency graph + runtime expansion | PARTIAL | compiler-side phase/round/spatial graphs with nodes, edges, components, cycle check, crossing test, and **unary obligations** (a mutating footprint no partition can own blocks the spatial axis even without a pair relation); `verify/check_graphs.sh` corpus validator with a soundness gate (cover_S/cover_T: the graph is never cleaner than the certificate) and a strictness census; runtime expansion (descriptors) still open |
| 7 | transformations as proof objects | PASS | `Transformation` records per mechanism (privatization, owner-computes, snapshot, claim staging, algebraic fold, frontier envelope) naming the relations removed, the conditions checked and the replacement edges; validation rejects a transformation that removes an undis-charged relation |
| 8 | product graph | PARTIAL | the phase graph carries the (round × phase) structure that the product graph is built from; a materialized product executor is open (§11) |
| 9 | AxisProof certificate | PASS | `AxisProof` (relations/discharged/unresolved, transformations, NIS/NIT/Realizable, overflow/invalid/inconclusive, components, unary) — three-valued, and an inconclusive proof is forced to non-independence |
| 10 | proof-derived schedule selection | PASS | `selectSchedule` now consumes the proof: R8 → serial+impl (unchanged reasons); overflow/invalid → serial+impl; a surviving edge without a realization → serial+impl; otherwise the effective axes are `certificate ∨ graph` (the graph can only be stricter). Suite: **67 PASS / 0 FAIL** |
| 11 | runtime executors | PARTIAL | §11.1 `sgpl_dag_relation_desc` extended (axis mask, relation kind, access kind, discharge id, phase/round/partition projection) with hardened fail-closed validation (discharged relations never scheduled, unary obligations have no DAG, mutex edges must be realization-tagged, no contradictory duplicate edges, unknown axis bits rejected); §11.2 `sgpl_exec_dag_spatial_graph` (real proof-derived spatial edges, wrong-axis rejected); §11.3 `sgpl_exec_dag_temporal_units` (partial DAG; the chain is now a caller); §11.4 `sgpl_exec_dag_product` ((unit × partition) cells, conservative product of the two projections, out-of-range/null rejected); 17 new `dag_scheduler_test` checks ALL PASS. Compiler-emitted descriptor wiring (the emitted program passing its proof graph to these entry points) still open |
| 12 | integration + docs | PARTIAL | `buildTemplates` adds the tagged RealizationOrder edge from a surviving CrossOwner proof edge even when no witness fired; the emitted programs still use the declared-schedule path (`autograph_exec_ctx_set_dag_axes`) rather than a descriptor array; docs open |
| 13 | tests | PARTIAL | `verify/check_graphs.sh` (cover gate + strictness census), `GRAPH_FRONTIER_RELCHECK`/`_GRAPHS` reports, relcheck invariant assertions, 17 new scheduler checks (proof metadata, discharged/unary/mutex/contradictory/axis validation, product exactly-once), adversarial `r6_order_sensitive` |
| 14 | acceptance | pending | |

## Live evidence log

- build: `bash build_lowmem.sh` (LLVM_CONFIG=/usr/local/llvm-20-polly-rtti) →
  `rc=0`, 0 errors, after moving `DomainKind`/`PhaseSegment`/`AccessMode` above
  the footprint layer (struct Effect stores footprints by value).
- relcheck (8 fixtures): every candidate `[relcheck] OK`; per-loop census lines
  record footprints/read/write/rmw/resources/relations/raw/war/waw/cross-owner/
  linked/discharged/overflow.
- suite: `verify/run.sh parallel` after §3.1+§3.2 → **67 PASS / 0 FAIL** local.
- graph validator (`verify/check_graphs.sh`, `GRAPH_FRONTIER_GRAPHS=1`): three
  real bugs found and fixed by the diff against the witness certificate —
  (1) a relation's round-crossing must not be counted on the spatial graph and
  vice versa (per-graph survival masks); (2) the same logical index compares
  equal only after canonicalizing the U/V alloca loads (two `load %v_slot`
  uses are the same element); (3) an endpoint node must be materialized at edge
  time or a resource relation silently loses its edge.  After the fixes:
  `dual_same_base` (R6) spatial dirty / temporal clean; `carried_read_state`
  (R4) temporal dirty / spatial clean; `r7_append_unwired` (R7) spatial dirty
  with **no realization** → serial; `r6_order_sensitive` both dirty; all agree
  with the certificate.

## Decisions taken in this round

- The relation layer is rebuilt **after** σ (ValueSource), round separation and
  the privatization decision are final (called from `finalizeProofInputs` at the
  pass, before `buildTemplates`), because the previous call site inside
  `deriveAllTemporal` ran before any of them existed (the old Snapshot discharge
  test could never fire).
- Discharge is now per-relation and mechanism-precise: Snapshot only for a
  snapshot read, Privatization only when the WRITE side is the private write,
  AlgebraicFold only for the same recognized operator on the same element, the
  frontier envelope maps onto the snapshot discharge (§7.6) and atomicity alone
  never discharges (§7.4).
- No semantic order is invented for `Par` siblings: a same-phase pair carries
  `SourcedFromOrder=false`, `OriginalOrder=0`; only phase-separated pairs and
  the internal read→write order of one RMW are ordered.
