# Passive waiting, parallel preparation and lightweight cuts

> Follow-up: preparation simplification is implemented and tested. Its fresh
> measurements and limits appear in the final section below. Earlier tables
> preserve their original measurement window.
> A subsequent 64-pair SSSP verification finds 108.78 ms before versus 106.33 ms
> after: no established process-time change, but a small traversal-region
> regression. See the SSSP verification section for the stronger comparison.

All three requested changes are implemented on top of the previous four runtime
optimizations. The main comparison uses eleven randomized fresh-process sets per
workload on the same Ryzen 5 5500 / WSL Ubuntu machine, with four workers pinned
to CPUs 0, 2, 4 and 6. Every SGPL timed output matches its correctness golden.

## Implementation and preserved contracts

- `03_run.sh` defaults OpenMP support teams to passive waiting and zero libgomp
  spins. Explicit settings win. Direct launches with neither setting soft-pause
  the loader's idle OpenMP team on supported GCC runtimes.
- CSR record preparation counts and fills consecutive source-ordinal chunks
  through the existing persistent pool and budget ledger. Stable per-partition
  prefix offsets preserve original source/neighbor occurrence order, duplicates,
  lifecycle hooks, append ordinals and first-match weight hints. Preparation runs
  no user effects. Nested/unavailable teams fall back to serial preparation;
  frontiers with fewer than 128 source occurrences remain serial.
- Accepted array spatial stages build only partition boundaries. Full source
  slices and destination rows remain available on demand for other CPU and GPU
  consumers. Full-view upgrades preserve existing boundary pointers. Cut builds
  are serialized, and each graph has its own cache identity. Generated setup
  rejects cut-allocation failure before private-buffer initialization.

Effect analysis, accepted proof blocks, snapshots, ordered frontier semantics,
DOALL/DOACROSS budget equations, profiler bodies and existing scheduler functions
are unchanged. Preparation is a new unregistered dispatch through the original
reservation/release functions, not a change to their equations or assignments.
No motif dispatch was re-enabled and no autotuner decision was overridden.

`AutoTunerPass.cpp`, `AutoTunerPass.h` and `cost_model.py` are identical to HEAD
and `effect_interim` after line-ending normalization. `pdg.cpp`,
`parallel_loop_outline.cpp`, `main.cpp` and `tdg_scheduler.inc` are unchanged.
The lowering changes only the emitted cut API and allocation preflight after
the existing proof decision. Although execution support lives in
`autotuner_runtime.c`, its cost predictions and profiler functions were not
modified.

## Fresh complete-process comparison

“Before” is the exact working-tree state at the start of this request, including
the previous four optimizations and the original waiting defaults. The reference
is the saved `effect_interim` executable from commit
`59355db63a3ad7e192bd8c0530bd350fc9110017`. After and reference use passive waiting.
All times are medians in milliseconds; reductions below are medians of paired
after/before ratios, so they need not equal ratios of the displayed medians.

| Workload | Before process | After process | effect_interim process | Paired reduction |
|---|---:|---:|---:|---:|
| BFS | 73.31 | 39.42 | 45.83 | 46.2% |
| CC | 87.31 | 48.49 | 26.82 | 45.8% |
| k-core | 80.93 | 71.55 | 72.70 | 9.2% |
| PageRank | 79.83 | 73.72 | 68.03 | 8.8% |
| SSSP | 166.67 | 109.39 | 63.67 | 34.0% |

Paired bootstrap 95% intervals for after/before process ratios are respectively
[0.514, 0.597], [0.522, 0.601], [0.872, 0.946], [0.824, 0.945] and
[0.609, 0.674]. All exclude 1 in this measurement window.

The reported traversal region has different boundaries from complete-process
time, and some preparation lies outside it. It does not include every saving.

| Workload | Before region | After region | effect_interim region |
|---|---:|---:|---:|
| BFS | 4.041 | 4.260 | 4.279 |
| CC | 7.728 | 6.228 | 1.755 |
| k-core | 18.542 | 18.287 | 18.437 |
| PageRank | 47.627 | 48.234 | 43.270 |
| SSSP | 38.695 | 20.311 | 6.873 |

BFS's reported region is about 5.4% slower in the paired measurement, with a
ratio interval [1.011, 1.108], despite its lower process time. k-core and
PageRank have no established traversal-region improvement. CC and SSSP do.
These measurements do not establish that the effect system is intrinsically
slower; the reference and current paths execute different runtime machinery
and, for the specialized motifs, different propagation/frontier work.

## Component checks

The same main randomized sets include controls for CC and SSSP. All controls
keep passive waiting and the same generated executable. The full-cut control
uses `SGPL_ARRAY_FULL_CUT=1`; the preparation control uses
`SGPL_ARRAY_PREP_SERIAL=1`. Controls preserve correctness and predictions.

| Configuration | CC process | SSSP process | SSSP region |
|---|---:|---:|---:|
| Before, passive waiting only | 82.87 | 156.17 | 37.662 |
| After, full cuts | 68.50 | 130.96 | 37.383 |
| After, serial chunk preparation | 62.28 | 138.39 | 21.704 |
| After, all three changes | 48.49 | 109.39 | 20.311 |

Both new preparation and lightweight cuts help complete-process performance.
The controls are omissions from the complete implementation, not additive
estimates of independent savings. The serial chunk control still uses the new
CSR chunk representation; it is not the old generic preparation algorithm.

## Exact Graptor SSSP comparison

The weighted graph is unchanged: 20,000 vertices, 160,000 undirected input
edges / 320,000 CSR arcs, source 1. The text input, converted graph, weight file
and Graptor executable hashes match the previous comparison. A fresh untimed
export matches all 20,000 Dijkstra reference distances; the reachable count is
20,000 and distance checksum is 1,182,819. Weights/distances are exact integers
within float32 range on this fixture.

Graptor revision is `3ce3b6baa09679b0d187a6165c61fdaa7208b336`, executable
`BFExport`, eight partitions, SIMD width eight, one round, four OpenMP workers.
The main table uses passive waiting for both implementations:

| SSSP implementation | Process ms | Reported algorithm region ms |
|---|---:|---:|
| Before | 166.67 | 38.695 |
| After | 109.39 | 20.311 |
| effect_interim | 63.67 | 6.873 |
| Graptor, passive | 44.76 | 3.727 |

After still takes about 1.72x the reference process time and 2.44x Graptor's
process time in this main comparison. The gap is reduced, not closed.

Passive waiting can slow Graptor's own OpenMP kernel. A separate eleven-set
randomized comparison restores its original default policy:

| Supplemental SSSP configuration | Process ms | Region ms |
|---|---:|---:|
| After, launcher passive defaults | 108.28 | 19.436 |
| After, direct launch with no OpenMP policy variables | 118.79 | 20.037 |
| Graptor, passive | 43.42 | 3.767 |
| Graptor, original policy defaults | 47.48 | 1.380 |

The direct-launch loader pause preserves explicit configuration and remains
slower than initializing passive waiting before OpenMP startup. The normal
launcher does the latter. Do not subtract times across these measurement
windows or treat the supplemental numbers as part of the main paired set.

Graptor's algorithm timer excludes loading, VEBO/layout construction and problem
setup; SGPL's region boundaries differ. Region ratios are not matched-kernel
speedups. Complete-process measurements use identical launch-to-exit timing,
but the algorithms retain different intermediate propagation/frontier behavior.
This comparison covers the exact SSSP workload, not all-five equivalent Graptor
algorithms.

## Validation and autotuner status

All ten executor/compiler suites passed on final sources: array preparation,
array execution, spatial execution, round/spatial execution, array lowering,
spatial proofs, temporal proofs, round/spatial proofs, GPU invocation and GPU
group parallelism. Real GPU tests include duplicates, empty rows, snapshots,
private folds, alias relocation, resident captures and ordered dependent rounds.
Budget, joint scheduling, DOACROSS, DAG scheduler and snapshot tests also passed,
using 1, 2, 4 and 8 workers where applicable.

New tests cover source hooks, stable weighted occurrence records, parent grants,
nested serial dispatch, balanced releases, cold worker-creation failure,
invalid-input rejection without effects, lazy CPU/GPU full-view upgrades and
cross-graph cache reuse during active execution. ASan/UBSan passed regular and
cold-failure runs (`detect_leaks=0` for the existing process-lifetime metadata).
Injected cut failure is rejected by generated preflight before program results.
All five generated modules passed LLVM verification. Launcher defaults and
explicit waiting-policy overrides passed their checks.

The unchanged stock autotuner assertion block was replayed against the saved
before, after and reference executables under the same passive configuration.
Before and after both pass five region-modelled assertions and fail all five
absolute-accuracy assertions. The reference passes seven assertions and fails
three; its CC and SSSP predictions remain within the existing 5x tolerance.
The runtime is faster, but the autotuner accuracy problem is not fixed.

The existing calibration JSON is frozen and has SHA-256
`a867cfad9b1c288275f6d1220d43130baec654ffe17c7c23c16c621fa932a4ea`.
No fresh `hw_calib_bench` was run for this comparison and no calibration or
cost-model values were changed. Before/after predictions match for every
workload. Reference total predictions differ for k-core and PageRank because
their emitted modeled regions differ, despite identical cost-model sources.

## Reproduction artifacts

Ignored directory `verify/bin/prepared_runtime_2026_10_14/` contains frozen
before sources/compiler, current binaries and LLVM modules, build logs, suite
logs, sanitizer/failure logs, source hashes, protected-source verification,
fixture/binary/calibration controls, fresh Graptor vector export, all timed
outputs and the raw/summary JSON files.

From WSL, run the scripts sequentially. The compiler build reuses the pinned
compiler object manifest from the previous comparison; `build.py` uses its
normal compiler/runtime flags and the frozen calibration. Benchmark scripts
pin their own affinity and check outputs. Stop builds/tests before timing.

```text
python3 verify/bin/prepared_runtime_2026_10_14/build_compiler.py
python3 verify/bin/prepared_runtime_2026_10_14/build.py
python3 verify/bin/prepared_runtime_2026_10_14/validate.py
python3 verify/bin/prepared_runtime_2026_10_14/check_models.py
python3 verify/bin/prepared_runtime_2026_10_14/check_failures.py
python3 verify/bin/prepared_runtime_2026_10_14/compare.py
python3 verify/bin/prepared_runtime_2026_10_14/supplement.py
python3 verify/bin/prepared_runtime_2026_10_14/replay_accuracy.py
```

## Follow-up: preparation simplification

Baseline for this follow-up is commit `3a2d99e5b03b9432966a420a6f6173a028e11326`,
including all previous optimizations. Production changes are confined to
`array_preparation_runtime.inc`; the preparation test and storage documentation
are extended. There are no changes to effect analysis/proofs, record formats,
context ABI, generated callbacks, snapshots, scheduling, DOALL/DOACROSS,
thread-budget equations or autotuner sources/predictions.

Counting and filling now have separate callbacks. The append ordinal limit is
computed once per preparation instead of dividing in the edge loop. Counting
validates sources, CSR row bounds and destinations, checks per-row ordinal
capacity, and accumulates source-owned counts from row lengths. The host prefix
checks aggregate capacity across chunks before allocations/filling. Filling
writes validated records without repeating validation or testing a phase flag.
Both callbacks cache immutable topology and job metadata in local variables.
The original contract keeps topology and input immutable through preparation;
no user callback runs between passes. Chunk boundaries, stable prefix offsets,
occurrence identities, hooks and original neighbor order are unchanged.

New rejection tests cover negative/decreasing/out-of-span rows, negative and
out-of-span columns, per-row append overflow, and aggregate overflow across
otherwise valid chunks. They reject without effects or unbalanced reservations.
The repeated-source fixture now has 2,047 entries to exercise uneven chunks.
All ten executor/compiler suites passed on the final implementation, including
real GPU grouping, CPU/GPU equivalence, ordering and failure propagation.
ASan/UBSan passed at 1, 4 and 8 workers and with cold-team failure fallback
(`detect_leaks=0` for existing process-lifetime metadata). All five unchanged
generated LLVM modules passed verification. Final source hashes match the
tested and benchmarked files.

An initial split without locally cached metadata did not establish a process
speedup and showed a CC region regression in one window. A separate 20-pair
control experiment tested arithmetic hoisting alone and splitting while keeping
duplicate validation. Neither provided a useful overall improvement. Those
variants are retained only in ignored experiment artifacts. The final version
includes the metadata hoist and was rebuilt, retested and measured separately.

Final measurements use eleven randomized fresh-process sets, four workers on
CPUs 0, 2, 4 and 6, passive waiting, unchanged generated objects/runtime flags,
the same fixture files and the same frozen calibration. All SGPL outputs match
their goldens and before/after predictions are identical. Times below are
medians in milliseconds. Ratios are medians of paired after/before process
times, with bootstrap 95% intervals, so they need not equal ratios of medians.

| Workload | Before process | After process | Paired process ratio [95% interval] |
|---|---:|---:|---:|
| BFS | 44.277 | 44.254 | 0.979 [0.948, 1.063] |
| CC | 56.002 | 56.554 | 0.981 [0.922, 0.996] |
| k-core | 81.496 | 79.556 | 0.990 [0.932, 1.040] |
| PageRank | 87.060 | 86.367 | 0.992 [0.818, 1.046] |
| SSSP | 130.743 | 129.245 | 0.990 [0.971, 0.998] |

The paired CC and SSSP gains are small (about 1.9% and 1.0%) and their intervals
are close to 1. Other cases have no established improvement. No workload has an
established traversal-region improvement in the final measurement. For SSSP,
the region medians are 23.423 and 24.840 ms, with a paired ratio 1.029 and
interval [0.949, 1.112]. This simplification does not close the runtime gap and
does not establish a reduction in the extra algorithmic/frontier work.

The same final randomized set reruns `effect_interim` and exact Graptor SSSP:

| SSSP implementation | Process ms | Reported region ms |
|---|---:|---:|
| Before simplification | 130.743 | 23.423 |
| After simplification | 129.245 | 24.840 |
| effect_interim | 72.859 | 7.694 |
| Graptor, passive | 54.147 | 4.823 |

These remain different region boundaries and execution semantics, as explained
above. Graptor uses the unchanged executable, graph, source, weights and
configuration from the previous exact comparison. This follow-up checks SGPL
goldens and Graptor's reported timing; it does not repeat the full distance
export already validated in the preceding comparison. Do not subtract absolute
times from the earlier tables: they were measured in different windows.

Artifacts are in ignored `verify/bin/preparation_simplification_2026_10_14/`:
before source snapshots and executables, final binaries, source/control hashes,
initial and final raw/summary measurements, isolated variants, final regression
and sanitizer logs, and disassembly confirming removal of the per-edge append
division. Reproduction from WSL, with builds/tests stopped before measuring:

```text
python3 verify/bin/preparation_simplification_2026_10_14/build.py
python3 verify/bin/preparation_simplification_2026_10_14/validate.py
python3 verify/bin/preparation_simplification_2026_10_14/sanitize.py
python3 verify/bin/preparation_simplification_2026_10_14/compare.py
```

## SSSP verification: the 109 ms versus 129 ms question

The original executable which measured 109.387 ms is byte-identical to the
pre-simplification executable used in the later 130.743 / 129.245 ms comparison.
Its SHA-256 is
`a042f93b14894c7661ea3e1bddc6b8af4df608ade7ae7997ce860fceeb412747`.
Thus comparing 109 ms from the older window directly with 129 ms from the newer
window mixes time variation with the code change. The unchanged original binary
itself measured approximately 109, 131 and now 109 ms in different windows.
The available logs do not identify the specific host condition responsible for
the earlier shift; background contention, clocks or VM scheduling were not
controlled or measured across those historical windows.

The requested verification runs the saved original and simplified binaries
together, without rebuilding or changing runtime/autotuner sources. It uses
three untimed warmup pairs followed by 64 fresh-process pairs in eight blocks.
Each block contains four before-first and four after-first pairs in randomized
order. Both versions have the same working directory, input, frozen calibration,
four-worker settings, passive waiting, stack limit and affinity on CPUs 0, 2,
4 and 6. No builds, tests or CPU profiling run concurrently. Timed output is
captured identically and logs are written outside timing. Source, binary,
input and calibration hashes are verified before measurement.

| Version | Median process ms | Median reported traversal ms |
|---|---:|---:|
| Before simplification, original 109 ms executable | 108.781 | 19.760 |
| After simplification | 106.332 | 20.287 |

All 128 timed answers match: 20,000 reachable vertices, checksum 1,182,819.
Profiling layouts, visit counts and predictions match across all timed runs and
the original 109 ms run: CSR, region 0 visits 20,001, region 1 visits 22, and
total predicted time 2.343960 ms.

The paired process-time geometric-mean ratio is 0.9821, with bootstrap 95%
interval [0.9636, 1.0012]. The mean paired elapsed-time difference is -1.894 ms,
with interval [-4.123, +0.380] ms. The result establishes neither a process-time
slowdown nor a significant speedup. It does not reproduce or support a 20 ms
whole-process penalty from simplification under these tested conditions.
The earlier approximately 1% process gain should therefore be treated as a
small, uncertain estimate rather than an established performance improvement.

The reported traversal region does regress slightly: paired median ratio
1.0304, with interval [1.0096, 1.0477]. Its average paired increase is 0.719 ms,
with interval [+0.193, +1.301] ms. This is a roughly 3% region regression, which
must not be hidden by the whole-process result. Preparation and the reported
traversal region have different boundaries; this verification does not isolate
the mechanism for that small region regression.

Artifacts and all output logs are in
`verify/bin/preparation_simplification_2026_10_14/sssp_verification/`.
The summary includes both paired median and geometric-mean ratios, elapsed-time
differences, CPU usage, per-block medians, hashes and verified profiling counts.
Reproduce the requested direct comparison from WSL:

```text
python3 verify/bin/preparation_simplification_2026_10_14/verify_sssp.py
```
