# Local execution runtime optimizations: four candidates

All four requested candidates were implemented and tried independently. All
four remain enabled in the final runtime. Buffer reuse contributes the largest
whole-process improvement. The owner-map cache had no isolated advantage on
these fixtures, but disabling it in the combined configuration made BFS's
profiled region slower. No autotuner, effect-analysis or thread-budgeting
implementation was changed.

## Final combined measurement

Fresh randomized interleaving, 11 processes per configuration and case, CPU
only, affinity [0,2,4,6], four workers, identical fixtures and preserved
algorithm objects, same O3 runtime/link flags, and the previous frozen hardware
calibration. Builds and correctness tests were stopped before timing.
`before` is the runtime before these four changes, already containing the
earlier execution optimizations. Its source was saved before editing.

| Case | Before region ms | All four region ms | Before process ms | All four process ms |
|---|---:|---:|---:|---:|
| bfs_level | 6.054 | 3.825 | 66.667 | 67.161 |
| cc | 11.892 | 7.311 | 115.742 | 85.803 |
| kcore | 17.378 | 16.972 | 77.316 | 77.649 |
| pagerank | 42.884 | 42.285 | 74.411 | 73.520 |
| sssp | 44.129 | 34.480 | 176.957 | 149.670 |

CC's median region time falls 38.5% and its median process
time falls 25.9%. SSSP's corresponding reductions are
21.9% and 15.4%. The paired bootstrap intervals for
the process-time ratios exclude 1 for both cases. They do not establish a
combined whole-process improvement for BFS, k-core or PageRank.

The final comparison includes the all-four configuration and the same
configuration with only owner caching disabled (`selected`). Across paired BFS
samples, disabling the cache has a median region-time ratio of 1.191 versus
all four, with bootstrap 95% interval [1.102,1.249]. CC and SSSP's intervals
for that configuration difference include 1. Thus the final choice follows the
combined behavior, rather than summing isolated gains. The experiment does not
identify the cause of this interaction.

## Independent candidates

Each row enables only that candidate. These measurements are from a separate
11-set randomized ablation, so their absolute times must not be subtracted
from the final table. Ratios are paired medians against that ablation's before
configuration; smaller is faster. Full intervals are in `summary.json`.

| Candidate | CC region ms | CC paired ratio | SSSP region ms | SSSP paired ratio |
|---|---:|---:|---:|---:|
| Skip fresh log clear | 9.688 | 0.795 | 42.625 | 0.918 |
| Reuse buffers | 8.637 | 0.706 | 39.220 | 0.853 |
| Cache owner map | 12.523 | 1.010 | 46.369 | 0.986 |
| Batch pair dispatch | 11.664 | 0.957 | 44.765 | 0.967 |

The owner cache alone slightly worsens BFS/CC profiling and has no established
SSSP gain. Batch dispatch has a smaller measured region improvement in CC and
SSSP. Removing duplicate clearing reduces profiled preparation, but does not
establish a whole-process gain in this final isolated ablation. Reuse provides
the clear isolated whole-process improvement.

## Implementation and ownership

1. Preparation initializes every active append slot and marks the new log
   pristine. The first begin consumes that marker and skips the second clear.
   Every later execution of the same context clears the log again, preserving
   repeated-context behavior and preventing stale appends.
2. An execution context has an exclusive lease for record arrays, original
   source/work ordinals, weight hints, append logs and snapshot buffers. A
   context returns its lease only on destruction, after its normal join.
   At most one idle arena totaling at most 64 MiB is kept per thread. Thread
   exit frees it; overlapping contexts receive distinct arenas. Destruction
   can occur on another thread. Larger arenas are freed rather than cached.
   Used log slots and offsets are reset; snapshots are recopied at their
   original materialization boundary. Failed growth retains old storage.
3. A preparation-thread owner map is keyed by graph metadata identity, vertex
   count, partition count and all boundary values. Same-pointer boundary
   changes are compared by value. Adjacency changes that leave the map's inputs
   unchanged do not require rebuilding it. This supplies no proof facts.
4. A prepared single-operation, pair-only batch selects its grouping action
   outside the hot loop. The loop keeps each original occurrence ordinal,
   source gate, callback order, append-site counter and cancellation check.
   Lifecycle-bearing batches and multiple-operation contexts use the general
   path. OWNER_V source lifecycle still uses its existing coverage passes.

GPU record grouping already replaces/frees record arrays. A first ownership
test exposed a double free when a lease retained those arrays. The final
implementation transfers all active buffers out of the lease before GPU
preparation, so the existing GPU ownership and reorder path remains valid.
GPU-grouped buffers are not retained in the execution arena cache. CPU fallback
after grouping retains the previous edge-query fallback and occurrence order.

The new fields are appended to `sgpl_exec_ctx`; existing field offsets stay
unchanged. The compiler was rebuilt because GPU capture uses its full size.
Benchmarked preserved CPU objects use the existing opaque-context API and
unchanged offsets. No algorithm or optimization-proof predicates were edited.

## Validation

- Independent ablation: 330 timed processes; all five cases match their
  goldens and all configuration stdout values agree. Combined/cache-disabled
  comparison: another 165 timed processes, again with identical correct
  outputs. Predictions are identical across runtime variants for each case.
- Array executor at 1/2/4/8 workers over every supported layout, including
  duplicate sources/edges, source hooks, exact order, repeated contexts,
  forced growth, prefix retention, topology publication and repartitioning.
- New tests cover overlapping frozen snapshots, exclusive leases, physical
  buffer reuse, failed growth preserving a snapshot, cross-thread destruction
  and thread-exit cleanup.
- Real GPU array-lowering tests at 1/2/4/8 workers, ordered duplicates, weighted
  first-match behavior, and under-reservation fallback. Both all-four and
  cache-disabled configurations pass.
- GPU invocation tests, including failure recovery, and real GPU group
  parallelism tests pass with the final all-four configuration.
- Spatial and round/spatial executor fault-injection suites; composable
  executor; full spatial, temporal and round/spatial compiler/runtime suites.
- Budget, joint scheduling, DAG and snapshot tests at 1/2/4/8 workers;
  DOACROSS model tests at four workers.
- Array executor under ASan/UBSan: no detected memory/undefined-behavior errors,
  including cross-thread cleanup. Leak detection was disabled for the existing
  process-lifetime graph metadata; this is not a leak-clean claim.
- Source identity checks cover the complete cost model, PDG, loop outlining,
  frontier proof lowering, compiler entry point, parallel runtime and TDG
  scheduler. All six profiler functions match the saved before version.
  Cost-model files also still match effect_interim.

The exact stock autotuner assertion block was replayed against the final
all-four executables: 5 region-modelled checks pass and all 5 absolute
prediction-accuracy checks fail. None of those assertions was relaxed.
Hardware calibration was reused, not freshly rerun. This experiment measures
execution changes under the established comparison controls.

## Fresh reference check

A separate randomized 11-set run includes effect_interim for CC and SSSP and
the previously validated Graptor SSSP binary on the identical weighted graph.

| Case | Implementation | Internal timer ms | Complete process ms |
|---|---|---:|---:|
| cc | effect_interim | 2.025 | 37.106 |
| cc | all | 7.862 | 92.335 |
| cc | before | 12.263 | 120.394 |
| sssp | before | 45.177 | 183.068 |
| sssp | effect_interim | 6.449 | 71.118 |
| sssp | graptor | 1.280 | 55.600 |
| sssp | all | 35.684 | 155.104 |

Graptor's algorithm timer and SGPL's accumulated traversal regions have
different boundaries. The complete-process timers use the same measurement.
The extra rounds, snapshot semantics and ordered duplicate frontier work are
unchanged, and a substantial gap to effect_interim and Graptor remains.

## Artifacts and reproduction

`verify/bin/local_runtime_candidates_2026_10_14/` contains saved before sources,
the per-candidate runtime objects/executables, build and validation logs,
`raw.json` / `summary.json` (independent candidates),
`selected_raw.json` / `selected_summary.json` (all four versus owner disabled),
`reference_raw.json` / `reference_summary.json`, source hashes and protected
implementation verification. `initial_*` records the earlier cache-key trial.
The runner's compile-time `SGPL_EXEC_*` switches change only the execution
helpers; production enables all four.

```text
wsl -d Ubuntu -- python3 /mnt/d/sgpl/Graph-language-/verify/bin/local_runtime_candidates_2026_10_14/compare.py build
wsl -d Ubuntu -- python3 /mnt/d/sgpl/Graph-language-/verify/bin/local_runtime_candidates_2026_10_14/compare.py measure
wsl -d Ubuntu -- python3 /mnt/d/sgpl/Graph-language-/verify/bin/local_runtime_candidates_2026_10_14/compare.py build --selected
wsl -d Ubuntu -- python3 /mnt/d/sgpl/Graph-language-/verify/bin/local_runtime_candidates_2026_10_14/compare.py measure --selected
```
