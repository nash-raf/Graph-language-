# Deferred temporal and spatial implementation gaps

Recorded: 2026-10-12.

Status: Deferred by the user. This document records the current limits for future work.

## Scope

This record covers the gaps discussed after implementation of combined round and spatial execution.
It also records the remaining combined-emission guards and related scope limits visible in the current code.
It is not a complete audit of the compiler.

The combined schedule is `RoundSpatialDag`, reported as `round-spatial-dag`.
It consumes the existing round proof and spatial partition witness.
Its CPU runtime executes round setup, independent CleanCut batches, and round finalization through one shared DAG.

Keep these three facts separate:

1. The analyzer establishes a proof result.
2. The compiler can emit a runtime that satisfies that result and its transformation contracts.
3. A particular input and worker budget permit actual overlap.

Success at one stage does not establish success at the next stage.

## 1. Missing bitmap effect summaries in round analysis

### Current behavior

`collectTemporalAccesses` in `temporal_scope_lowering.inc` recognizes loads, stores, atomics, iterator operations, and memory intrinsics.
It does not provide specific effect summaries for bitmap operations.
A bitmap call that accesses memory or has side effects therefore enters the opaque-call list.
Here, an opaque call means a call whose relevant effects the analyzer cannot describe.

`proveRounds` records these calls as `unknown-round-resource` relations.
When no definite conflict takes priority, these unknown relations produce `TemporalProofStatus::Inconclusive`.
The diagnostic is `complete round dependencies are incomplete`.

The spatial analyzer has more specific coverage.
`spatial_partition_proof.inc` recognizes certain bitmap reads and frontier additions under its existing contracts.
Those spatial contracts do not automatically establish round independence.

### Consequence

Production selection requires the actual round result to be `Independent` before it attempts combined emission.
Bitmap frontier loops with an inconclusive round result therefore cannot automatically select `RoundSpatialDag`.
The compiler can select combined execution automatically for supported loops when both actual proofs and runtime checks succeed.

The missing summaries can hide valid independence between rounds with separate frontier state.
They can also cover programs with real dependencies. An unknown call must not be assumed independent.

### Correctness effect

This restriction prevents unproved round concurrency. It can reduce parallelism.
An inconclusive result establishes neither independence nor interference.

### Future work, if resumed

- Describe each supported bitmap operation's reads, writes, allocation effects, and observable effects.
- Establish whether bitmap handles and underlying storage are shared or separate across rounds.
- Include membership reads, additions, clearing, output publication, and any other relevant observers.
- Preserve unknown results for unsupported calls or unresolved aliases.
- Test independent bitmap state and genuinely shared bitmap state through the production pass.

Relevant symbols: `collectTemporalAccesses`, `proveRounds`, and the production gate on `T.Rounds.Status` in `temporal_scope_lowering.inc`.

## 2. Frontier tests assume the round result

### Current behavior

The frontier branch of `test/round_spatial_lowering_test.cpp` runs the spatial prover and checks its accepted runtime contracts.
It also calls `proveRounds`.
It then explicitly assigns:

```cpp
Info.TemporalResult.Rounds.Status = TemporalProofStatus::Independent;
```

The test calls `emitRoundSpatial` directly after that assignment.
The comment calls this a supplied certificate. More precisely, it is a test assumption supplied by a status override.
It is not a round proof produced by the compiler.

### What these tests establish

- The emitter can consume an assumed round result and checked spatial contracts for the tested frontier fixtures.
- The generated runtime can execute those fixtures with separate frontier storage.
- Source and destination ownership paths can avoid the old graph-scratch preparation and commit calls.
- The tested outputs agree with their sequential references at the tested worker counts.

### What these tests do not establish

- Automatic round proof for bitmap frontier loops.
- Automatic production selection of combined execution for those frontier fixtures.
- Validity of a supplied round assumption for arbitrary frontier programs.

The override exists in the test harness. It does not bypass the production selection gate.

### Future work, if resumed

- Retain these tests as emitter and runtime tests with explicit assumed premises.
- Add production-pass tests that contain no proof-status override.
- Check the actual round result, selected schedule, diagnostics, and generated-program behavior.
- Include a dependent-frontier case that must retain ordered rounds.

## 3. Some allocation sizes cannot be reserved before setup

### Current behavior

Each execution window must reserve its infrastructure before its first user effect.
The compiler can calculate supported variable array sizes from round indices, immutable captures, unambiguous scalar seeds, and branch-selected values.
It only clones instructions into reservation when they are safe to evaluate there.

Some sizes require a setup effect, an unresolved mutable value, or an instruction that cannot safely execute early.
A conditional allocation can have the same problem in its guard.
For example, division can be valid on its original guarded path but unsafe during early reservation.

The combined emitter then refuses the realization. Relevant diagnostics include:

```text
allocation extent requires executing user effects during reservation
conditional allocation requires executing setup effects during reservation
round setup value cannot be reserved without user effects
```

### Consequence and correctness effect

An accepted proof can remain valid while combined execution is unavailable.
The compiler attempts round-only execution after a combined-emission refusal.
If that route is unavailable, the existing selection logic considers the later available routes.

This restriction avoids moving user effects or unsafe evaluation into reservation.
It can reduce parallelism. It does not establish interference.

### Future work, if resumed

Identify a reservation strategy that preserves the original evaluation conditions and failure semantics.
Do not execute setup effects merely to determine infrastructure sizes before dispatch.
Retain tests for skipped guards, unsafe division, allocation failure, and failure without replay.

## 4. Combined outlining still supports only certain control-flow and value shapes

Outlining means moving the original round work into callbacks while preserving its behavior.
The compiler supports several setup and source-exit PHI shapes, including skipped traversal paths.
A PHI selects a value from the control-flow path that reached its block.
Support is not general for every LLVM control-flow graph or value leaving the traversal.

The remaining guards in `emitRoundSpatial` are:

| Diagnostic | Current limit |
| --- | --- |
| `round traversal has no supported continuation` | The source traversal lacks a supported exit or required graph availability. |
| `round setup or finalization crosses the traversal boundary` | Setup or finalization cannot be separated using the supported round boundaries. |
| `round continuations overlap` | The discovered setup and finalization regions overlap. |
| `round capture is unavailable at dispatch` | A required captured value does not dominate the dispatch location. |
| `spatial stage cannot expose its lifecycle` | The existing spatial emitter cannot provide the required reservation, setup, partition, and completion parts. |
| `traversal SSA output requires an unavailable completion value` | The compiler cannot reconstruct a required value after the spatial traversal joins. |
| `outlined callback has an unsupported capture or CFG` | LLVM verification rejects a generated callback. CFG means control-flow graph. |

The reservation diagnostics from section 3 are additional combined-emission limits.

These checks run before replacement of the original round loop.
A combined realization failure retains the proof results and attempts round-only execution.
It reports runtime unavailability separately from proof failure.

### Future work, if resumed

- Add a small reproducer for each unsupported shape before extending the emitter.
- Preserve guards, source lifecycle effects, traversal outputs, and final induction values.
- Validate the generated callbacks and compare their execution with a sequential reference.
- Keep refusal before original-loop replacement when a shape remains unsupported.

Relevant implementation: `round_spatial_lowering.inc`, especially `emitRoundSpatial` and its `Refuse` paths.

## 5. Related limits and intentional scope boundaries

These items explain cases where concurrency remains unavailable. They are not all missing runtime features.

| Item | Current behavior | Classification |
| --- | --- | --- |
| Frontier progression depends on an earlier round | Rounds remain ordered. | Required dependency preservation. |
| Round enumeration | Supports stable, ascending signed 32-bit domains with a unit increment. Other progression or unresolved bounds can remain inconclusive. | Analyzer and enumerator coverage limit. |
| Spatial grouping families | Uses pair-occurrence, destination, and source witnesses. Unsupported topology-dependent connectivity remains inconclusive. | Intentional static proof coverage limit. |
| Source-phase and spatial composition | The phase fallback exists, but the combined schedule composes rounds with spatial work only. | Explicitly deferred scope. |
| GPU realization | Supported spatial, round, source-phase, and combined callbacks have device realizations. Unsupported device capabilities retain CPU execution before effects. | Runtime capability, separate from proof coverage. |
| Zero or one occupied group or round | Execution need not overlap even when a conditional proof family is valid. One CPU worker can submit multiple GPU tasks. CPU execution still obeys worker budgets. | Runtime cardinality and CPU execution condition. |

A later round whose frontier comes from an earlier round is not an independent round.
Bitmap summaries must preserve that dependency if future work adds them.

## 6. Restrictions already removed

These earlier restrictions are no longer blanket reasons to reject combined execution:

- Frontier envelopes that previously used graph-owned preparation and commit scratch.
- All variable-size local arrays.
- All source-loop exit PHIs.

Each active round now owns frontier membership, claim, and merged append storage through its execution context.
Setup materializes membership after the original prefix.
Finalization commits the result after the batches join and before the original suffix.

Supported variable-size arrays and supported exit PHIs use round-frame storage.
The narrower reservation and outlining limits in sections 3 and 4 remain.

Runtime support for frontier storage does not resolve the missing bitmap summaries in round analysis.

## 7. Validation record and interpretation

The preceding implementation validation ran in Ubuntu WSL.
The recorded result was a pass for all 14 suites:

- `test/run_round_spatial_tests.sh`
- `test/run_round_spatial_exec_tests.sh`
- `test/run_round_spatial_lowering_tests.sh`
- `test/run_spatial_tests.sh`
- `test/run_spatial_exec_tests.sh`
- `test/run_spatial_static_tests.sh`
- `test/run_exec_engine_tests.sh`
- `test/run_exec_r2_tests.sh`
- `test/run_frontier_shadow_tests.sh`
- `validate_composition.sh`
- `validate_reduction.sh`
- `validate_roundsep.sh`
- `validate_rt_expr.sh`
- `test/run_temporal_tests.sh`

That validation included generated fixtures at 1, 2, 4, and 8 workers, overlap instrumentation, and failure cases.
The frontier emitter fixtures have the assumed-premise limitation described in section 2.
Passing tests do not establish general compiler correctness or automatic bitmap frontier acceptance.

The later GPU implementation adds device realizations without changing these proof limitations.
See `GPU_RUNTIME.md` for GPU capability limits, transfer boundaries, and validation.
GPU resource budgeting is removed. Device submission no longer follows CPU
grants or legacy block-budget controls. Spatial and combined kernels now run
whole proof groups on separate device lanes. Ordered round-only and phase
components retain their internal order when no finer spatial realization applies.
Generated programs retain language arrays and immutable layouts across dispatches.
Required host lifecycle work, runtime-owned scratch, and unsupported device operations
can still require transfer boundaries. See `GPU_RUNTIME.md` for the residency contract.

## 8. Starting points for a future revisit

- `temporal_scope_lowering.inc`: temporal access collection, round proof, and production schedule selection.
- `spatial_partition_proof.inc`: recognized bitmap operations and spatial transformation contracts.
- `round_spatial_lowering.inc`: combined reservation, callback outlining, and realization refusals.
- `autotuner_runtime.c` and `autotuner_runtime.h`: invocation-owned frontier operations and the shared DAG runtime API.
- `test/round_spatial_lowering_test.cpp`: frontier status override and emitter fixtures.
- `test/round_spatial_ir_reference.c`: sequential references for generated LLVM fixtures.
- `test/run_round_spatial_lowering_tests.sh`: generated-fixture validation.
- `EFFECT_SYSTEM.md`: current proof and execution design, including the combined schedule.
- `proof/EFFECT_ALGEBRA_DESIGN.md`: effect algebra and runtime interpretation.

The user deferred this work. No item in this document authorizes implementation.
