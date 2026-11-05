# GPU execution realization benchmarks

This is the supplementary matched `-O2` campaign. It used correctness-build flags, while the saved performance artifacts used `-O3`. Its relative matched comparison is valid, but its historical ratios mix compiler configurations. Use the [production-flag rerun](../attempt03_o3_native_io/report.md) for the primary performance result.

61 graphs × four algorithms; five fresh-process samples per version, with alternating AB/BA order and one diagnostic warmup per version. Six CPU workers. The previous runtime includes batching and synchronization elision. Both runtimes use the same native compiler flags (`-O2 -g -fopenmp -pthread`), generated program objects, inputs, and timing interposers. No runtime implementation changed during the campaign.

Ratios above 1 mean the new runtime is faster. Each case uses arithmetic mean time; aggregate ratios are geometric means. Compute-region time includes any GPU initialization, preparation, transfers and synchronization performed inside that region; it is not isolated kernel time. Whole-process time also includes process startup, input loading and cleanup.

The saved CPU and saved batched GPU comparisons are historical and unpaired. The CPU was not rebenchmarked. The GPU also serves the Windows desktop; five-sample means and descriptive bootstrap intervals do not remove that source of variation.

## Matched pre-change runtime versus current runtime

| Algorithm | Cases using GPU | Whole-process speedup | Compute-region speedup | Compute 95% descriptive interval |
|---|---:|---:|---:|---:|
| bfs | 49 | 1.105× | 1.103× | 0.948–1.294× |
| sssp | 0 | CPU fallback | CPU fallback | — |
| cc | 58 | 1.054× | 1.054× | 0.908–1.226× |
| pagerank | 58 | 1.095× | 1.461× | 1.309–1.633× |
| All GPU-executing cases | 165 | 1.083× | 1.198× | — |

SSSP’s saved fixtures use the existing CPU fallback. GPU execution counts match the correctness corpus and the pre-change runtime for every case. CPU fallback cases are excluded from the GPU aggregates above.

## Historical comparisons for cases using the GPU

| Algorithm | Saved GPU / current, process | Saved GPU / current, compute | Saved CPU / current, process | Saved CPU / current, compute |
|---|---:|---:|---:|---:|
| bfs | 0.275× | 0.247× | 0.0037× | 0.0015× |
| cc | 0.309× | 0.285× | 0.0026× | 0.0007× |
| pagerank | 0.281× | 0.561× | 0.0043× | 0.0075× |

Across the 165 GPU-executing cases, the new runtime is 0.347× faster in compute time and 0.289× faster in whole-process time than the saved batched-GPU measurements. Its compute-region time is still 488.6× the saved CPU time; whole-process time is 288.6× the saved CPU time. These are historical, unpaired geometric-mean comparisons.

## CPU fallback cases

| Algorithm | Fallback cases | Previous / current, process | Previous / current, compute | Saved CPU / current, compute |
|---|---:|---:|---:|---:|
| bfs | 12 | 10.183× | 15.225× | 0.155× |
| sssp | 61 | 0.956× | 1.035× | 0.057× |
| cc | 3 | 2.355× | 2.587× | 0.157× |
| pagerank | 3 | 1.229× | 2.641× | 0.108× |

## Selected same-binary controls

These five-sample controls disable all four switches, or one switch at a time, in the current binary. Ratios are the disabled mode’s compute time divided by all-on compute time. They cover selected cases and interactions; they are not additive contributions or a full-corpus attribution. Unconditional preparation cleanup remains enabled in every same-binary mode; the separate pre-change binary captures its effect.

| Case | All off / on | Records off / on | Early gate off / on | Transfers off / on | Capacity off / on |
|---|---:|---:|---:|---:|---:|
| bfs, soc/soc-slashdot.edges | 2.182× | 2.642× | 1.133× | 2.261× | 2.108× |
| cc, dimacs10/fe-4elt2.edges | 0.944× | 1.993× | 1.050× | 0.939× | 1.488× |
| pagerank, dimacs/hamming8-2.edges | 1.071× | 0.737× | 0.594× | 0.612× | 0.708× |
| pagerank, soc/soc-slashdot.edges | 2.771× | 2.507× | 1.068× | 1.623× | 0.843× |

## Concrete selected timings

| Case | GPU tasks in warmup | Previous process ms | Current process ms | Previous compute ms | Current compute ms | Saved CPU compute ms |
|---|---:|---:|---:|---:|---:|---:|
| pagerank, dimacs/hamming8-2.edges | 21 | 2520.376 | 1238.803 | 113.080 | 56.518 | 1.466 |
| cc, dimacs10/fe-4elt2.edges | 92 | 3303.402 | 2026.699 | 3064.520 | 1841.106 | 1.404 |
| bfs, dimacs10/inf-luxembourg_osm.edges | 0 | 8781.184 | 80.762 | 8772.088 | 72.937 | 13.426 |
| cc, dimacs10/inf-luxembourg_osm.edges | 750 | 14753.606 | 6154.148 | 14638.547 | 6046.886 | 19.669 |
| bfs, soc/soc-slashdot.edges | 4 | 2893.009 | 3025.505 | 2532.321 | 2758.108 | 6.619 |
| pagerank, soc/soc-slashdot.edges | 21 | 2827.756 | 2363.236 | 1421.398 | 1044.501 | 12.489 |

## More recent saved synchronization-elision controls

These four saved measurements used synchronization elision enabled. They are historical, unpaired, and used the previously frozen runtime build; the fresh matched comparison above is the basis for attributing the four new changes.

| Case | Saved sync process ms | Current process ms | Saved sync compute ms | Current compute ms |
|---|---:|---:|---:|---:|
| pagerank, dimacs/hamming8-2.edges | 658.228 | 1238.803 | 57.053 | 56.518 |
| cc, dimacs10/fe-4elt2.edges | 823.140 | 2026.699 | 733.045 | 1841.106 |
| bfs, soc/soc-slashdot.edges | 530.023 | 3025.505 | 428.479 | 2758.108 |
| pagerank, soc/soc-slashdot.edges | 933.891 | 2363.236 | 499.203 | 1044.501 |

## GPU utilization observation

The runtime can alternate GPU steps with CPU fallback steps inside one algorithm. During the earlier exploratory `-O2` geo1k_50k CC warmup, the trace showed five completed GPU tasks followed by an existing-threshold CPU fallback for 1,511 records, below the 4,096 minimum. A contemporaneous one-time `nvidia-smi` query reported 3% utilization. This is not an average for the primary run or a GPU kernel-time measurement. CPU preparation, driver work, transfers, polling and CPU fallback remain part of execution. The four changes do not change offload selection or increase the work inside the GPU kernels.

## Audit

- Main campaign: 244 cases × two runtimes × five timed samples = 2,440 timed executions.
- Selected controls: four cases × six switch modes × five timed samples = 120 timed executions.
- Existing correctness audit: 488 independent output validations and 160 feature-subset CPU comparisons passed before timing.
- `baseline_audit.json` verifies source isolation, identical shared native objects and historical timer identity.
- `results.json` preserves protocol, executable hashes, diagnostics and all main raw samples; `controls.json` preserves selected control samples.
- `comparison.csv` contains per-case arithmetic mean times and ratios.
- Native binaries, compiler output and stdout/stderr are under `/home/kazi/p1_gpu_four_benchmark_20261104/attempt01/`.
- Speedup means an elapsed-time ratio; no kernel-only throughput measurement or new profiling campaign was performed.

A Windows sharing lock interrupted a status-file save before a timed launch. The final control reused 23 valid raw samples and collected the seven missing samples. No successful timed sample was rerun or discarded. `recovery.json` records the recovery.
