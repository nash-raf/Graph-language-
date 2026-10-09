# Two-axis DAG scheduler — status (gate split landed)

Branch: `two-axis-dag-scheduler` (local `92e34c1` + this work, uncommitted until
the commits below).  Pod: `root@213.173.110.201 -p 18657` (48 cores, RTX 2000
Ada) — pod tree carries the same content; its HEAD is behind (739226c) with the
newer commits present as working-tree changes.

## What this change does (complete certificate → per-axis gate → schedule)

Pipeline: `R1..R7 → (R_S,R_T) → (NI_S,NI_T) → (G_S,G_T) → schedule`
(`selectSchedule`, `graph_frontier_lowering.cpp`).

- **Complete certificate**: `supportedByAlgebra` no longer short-circuits
  (`RecordRefusal`), so both axes are populated; the first refusal keeps the
  historical reason string and the verdict keeps the old `admit()/refuse()`
  contract.
- **Gate split**: `Rewritable = Modelable && schedule != serial && emitted`.
  The old conjunction `!(R_S ∨ R_T ∨ R8)` no longer forces whole-nest serial
  when only one axis has a witness.
- **Realizations**: `¬R_S ∧ R_T` → `spatial-dag` (partitions concurrent, round
  sequence preserves R4's writer→reader order); `R_S ∧ ¬R_T` → `temporal-dag`
  selected but **not emitted** in V1 (explicit implementation reason — never a
  semantic refusal); `R_S ∧ R_T` → serial (theorem); R8/unmodelable/failed emit
  → serial + `sgpl.frontier.impl.serial`.
- **Markers**: `sgpl.frontier.dag.owner=engine`, `.dag.axes` (compat),
  `.dag.spatial`/`.dag.temporal` (`parallel|ordered|serial`); `pdg.cpp` reads
  the *value* into its classification note.  `impl.serial` separates
  implementation failure from theorem-serial.
- **Consumption invariant**: `witnessesConsumed` fails the emission closed if
  an unresolved witness has no graph constraint and no schedule-level order
  proof.
- `buildTemplates` now builds *constrained* templates for representable dirty
  axes (R6 same-base → a `RealizationOrder` U→V edge; R1/R3/R7 stay
  unrepresentable with their own reasons).

## Evidence (2026-10-09)

| check | result |
|---|---|
| `verify/cases/parallel/carried_read_state.graph` (new, R4) | `R_S=0 R_T=1`, `#1 R4 discharge=none`, `schedule=spatial-dag emitted=1`, `spatial=parallel temporal=ordered`, `class=dest-owner` |
| its answer vs unrewritten build | identical `deg_sum 0` at threads 1/4/8 (local) and 1/4/8/16/32 (pod, 48 cores) |
| `verify/run.sh parallel` | 60 PASS / 1 FAIL — the FAIL (`race/derived_default`, no derived DOALL) is **pre-existing**: baseline binary at HEAD shows 0 DOALL lines too |
| corpus census (78 cases, new binary) | 78/78 builds rc=0; schedules seen: 42×nested, 2×serial (R8 reductions) — nothing else changed class |
| `validate_algebra.sh` | 52/53 — only the pre-existing `ultimate_pagerank` golden `class=` diff |
| `validate_roundsep.sh` / `validate_reduction.sh` / `validate_rt_expr.sh` | PASS |
| `validate_composition.sh` | all cases MATCH; `reduce_add` SKIP (input file absent from the tree) |
| `test/run_exec_engine_tests.sh` | PASS (0 failures) |
| `test/run_exec_r2_tests.sh` | PASS after harness fix (link line lacked `gpu_runtime.c` — pre-existing breakage from the GPU commit) |
| `test/run_frontier_shadow_tests.sh` | PASS after harness fixes (same link gap + the stale 12-arg `autograph_exec_ctx_create` call) |
| `claim_driver` | `#1 R2 discharge=claim-staging`, emit fails closed with `impl_failure=1 reason=emit failed` + `impl.serial`; `class=sequential` kept (R2 proof obligation documented in `proof/R2_REJECTION.md`) |
| pod: gpu_device_diff `carried_read_state` | P=1/4/8 **identical**, 16 device dispatches / 1 CPU fallback |
| pod: gpu_device_diff `dual_shadow` | P=1/4/8 identical (device-ineligible, CPU fallback) |
| pod: `verify/run.sh parallel` | 57 PASS / 4 FAIL — `nested_step` = `./final_program` **SIGKILLed** (rc=137, environmental OOM; locally PASS), `marker_derived` + `derived_default` = no `classification=DOALL` in the trace on the pod's NVPTX build (`derived_default` fails locally too, pre-existing), `doall_scaling` = sentinel `99999 -> 99999` (measurement path, locally PASS).  `dual_shadow`, `claim_driver`, `carried_read_state` all PASS on the pod. |

## Not done (explicit gaps, in plan order)

1. **Temporal-dag emission** (§12): selected, template built, `Emitted=false`
   with an explicit reason; needs multi-unit emission + constrained spatial
   dispatch (the `SGPL_DAG_SPATIAL` runtime path) wired per-step.
2. **Per-step runtime dispatch ABI** (§13/§16): `SGPL_DAG_SPATIAL` is still an
   env-global switch (`autotuner_runtime.c:3312,3598`); the axes/class do not
   cross the ABI; `sgpl_exec_dag_temporal_chain` still has no production call
   site (tests only).
3. **Budget policy** (§15): only the nested share `max(1, W/A)` + TLS clamp;
   no ready-set reservation/redistribution, no `SGPL_DAG_DEBUG` tracing.
4. **§19 fixture set**: only the R4 carried-read case + the existing claim/
   shadow regressions exist; the partial-DAG (U1→U3, P1→P3), hazard
   (RAW/WAR/WAW discharge), and failure (cyclic/unsupported/descriptor/budget)
   fixtures are not written; per-witness R1-R7 fixtures are missing.
5. **GPU corpus device differential** with the new paths (the two-fixture
   device diff was run; the full corpus device diff was not completed).
