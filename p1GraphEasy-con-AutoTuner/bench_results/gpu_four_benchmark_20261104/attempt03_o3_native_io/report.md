# GPU execution realization benchmarks

61 graphs × four algorithms; five fresh-process samples per version, with alternating AB/BA order and one diagnostic warmup per version. Six CPU workers. The previous runtime includes batching and synchronization elision. Both runtimes use the same native compiler flags (`-O3 -g -fopenmp -pthread`), generated program objects, inputs, and timing interposers. No runtime implementation changed during the campaign.

Ratios above 1 mean the new runtime is faster. Each case uses arithmetic mean time; aggregate ratios are geometric means. Compute-region time includes any GPU initialization, preparation, transfers and synchronization performed inside that region; it is not isolated kernel time. Whole-process time also includes process startup, input loading and cleanup.

The saved CPU and saved batched GPU comparisons are historical and unpaired. The CPU was not rebenchmarked. The GPU also serves the Windows desktop; five-sample means and descriptive bootstrap intervals do not remove that source of variation.

## Matched pre-change runtime versus current runtime

| Algorithm | Cases using GPU | Whole-process speedup | Compute-region speedup | Compute 95% descriptive interval |
|---|---:|---:|---:|---:|
| bfs | 49 | 1.041× | 1.048× | 1.002–1.115× |
| sssp | 0 | CPU fallback | CPU fallback | — |
| cc | 58 | 1.082× | 1.094× | 1.045–1.154× |
| pagerank | 58 | 1.008× | 1.216× | 1.118–1.322× |
| All GPU-executing cases | 165 | 1.044× | 1.121× | — |

SSSP’s saved fixtures use the existing CPU fallback. GPU execution counts match the correctness corpus and the pre-change runtime for every case. CPU fallback cases are excluded from the GPU aggregates above.

## Historical comparisons for cases using the GPU

| Algorithm | Saved GPU / current, process | Saved GPU / current, compute | Saved CPU / current, process | Saved CPU / current, compute |
|---|---:|---:|---:|---:|
| bfs | 1.117× | 1.128× | 0.0152× | 0.0067× |
| cc | 1.182× | 1.198× | 0.0101× | 0.0031× |
| pagerank | 1.081× | 1.335× | 0.0165× | 0.0179× |

Across the 165 GPU-executing cases, the new runtime is 1.222× faster in compute time and 1.126× faster in whole-process time than the saved batched-GPU measurements. Its compute-region time is still 138.6× the saved CPU time; whole-process time is 74.0× the saved CPU time. These are historical, unpaired geometric-mean comparisons.

## CPU fallback cases

| Algorithm | Fallback cases | Previous / current, process | Previous / current, compute | Saved CPU / current, compute |
|---|---:|---:|---:|---:|
| bfs | 12 | 9.895× | 17.400× | 0.455× |
| sssp | 61 | 1.012× | 1.040× | 0.248× |
| cc | 3 | 1.598× | 2.505× | 0.260× |
| pagerank | 3 | 1.543× | 1.903× | 0.154× |

## Selected same-binary controls

These five-sample controls disable all four switches, or one switch at a time, in the current binary. Ratios are the disabled mode’s compute time divided by all-on compute time. They cover selected cases and interactions; they are not additive contributions or a full-corpus attribution. Unconditional preparation cleanup remains enabled in every same-binary mode; the separate pre-change binary captures its effect.

| Case | All off / on | Records off / on | Early gate off / on | Transfers off / on | Capacity off / on |
|---|---:|---:|---:|---:|---:|
| bfs, soc/soc-slashdot.edges | 1.156× | 1.024× | 1.071× | 1.021× | 1.040× |
| cc, dimacs10/fe-4elt2.edges | 1.088× | 1.001× | 0.996× | 1.028× | 1.062× |
| pagerank, dimacs/hamming8-2.edges | 1.392× | 1.276× | 1.097× | 1.132× | 1.035× |
| pagerank, soc/soc-slashdot.edges | 1.770× | 2.075× | 1.001× | 1.535× | 0.845× |

## Concrete selected timings

| Case | GPU tasks in warmup | Previous process ms | Current process ms | Previous compute ms | Current compute ms | Saved CPU compute ms |
|---|---:|---:|---:|---:|---:|---:|
| pagerank, dimacs/hamming8-2.edges | 21 | 480.562 | 506.139 | 45.076 | 37.671 | 1.466 |
| cc, dimacs10/fe-4elt2.edges | 92 | 919.917 | 860.044 | 826.102 | 763.556 | 1.404 |
| bfs, dimacs10/inf-luxembourg_osm.edges | 0 | 7672.466 | 75.458 | 7664.635 | 66.907 | 13.426 |
| cc, dimacs10/inf-luxembourg_osm.edges | 750 | 15716.438 | 6347.895 | 15591.513 | 6239.973 | 19.669 |
| bfs, soc/soc-slashdot.edges | 4 | 578.139 | 534.346 | 466.788 | 425.861 | 6.619 |
| pagerank, soc/soc-slashdot.edges | 21 | 944.608 | 913.402 | 507.236 | 473.336 | 12.489 |

## More recent saved synchronization-elision controls

These four saved measurements used synchronization elision enabled. They are historical, unpaired, and used the previously frozen runtime build; the fresh matched comparison above is the basis for attributing the four new changes.

| Case | Saved sync process ms | Current process ms | Saved sync compute ms | Current compute ms |
|---|---:|---:|---:|---:|
| pagerank, dimacs/hamming8-2.edges | 658.228 | 506.139 | 57.053 | 37.671 |
| cc, dimacs10/fe-4elt2.edges | 823.140 | 860.044 | 733.045 | 763.556 |
| bfs, soc/soc-slashdot.edges | 530.023 | 534.346 | 428.479 | 425.861 |
| pagerank, soc/soc-slashdot.edges | 933.891 | 913.402 | 499.203 | 473.336 |

## GPU utilization observation

The runtime can alternate GPU steps with CPU fallback steps inside one algorithm. During the earlier exploratory `-O2` geo1k_50k CC warmup, the trace showed five completed GPU tasks followed by an existing-threshold CPU fallback for 1,511 records, below the 4,096 minimum. A contemporaneous one-time `nvidia-smi` query reported 3% utilization. This is not an average for the primary run or a GPU kernel-time measurement. CPU preparation, driver work, transfers, polling and CPU fallback remain part of execution. The four changes do not change offload selection or increase the work inside the GPU kernels.

## Primary-run method

The primary run uses the original frozen pre-change `-O3` runtime objects and a correctness-checked new `-O3` runtime. Compiler settings are checked from both objects’ DWARF metadata. Generated program objects, including GPU kernel code, are identical between the two versions. All per-sample progress files and intermediate reports remain in the native Linux benchmark directory. The completed results are copied to the watched workspace after sampling ends.

The initial matched `-O2` campaign is supplementary because it used validation-build flags rather than the saved performance flags. The partial workspace-I/O `-O3` campaign was superseded to keep file watching out of the sampling loop. Neither contributes samples to the primary results. Their raw measurements remain preserved separately. A separate 20-sample rerun of the untouched saved executables is in [saved-binary calibration](../attempt01/saved_binary_calibration.json); those samples are outside the primary 2,560 timed executions.

## Audit

- Main campaign: 244 cases × two runtimes × five timed samples = 2,440 timed executions.
- Selected controls: four cases × six switch modes × five timed samples = 120 timed executions.
- Existing correctness audit: 488 independent output validations and 160 feature-subset CPU comparisons passed before timing.
- `baseline_audit.json` verifies source isolation, identical shared native objects and historical timer identity.
- `results.json` preserves protocol, executable hashes, diagnostics and all main raw samples; `controls.json` preserves selected control samples.
- `comparison.csv` contains per-case arithmetic mean times and ratios.
- Native binaries, compiler output and stdout/stderr are under `/home/kazi/p1_gpu_four_benchmark_20261104/attempt03_o3_native_io/`.
- Speedup means an elapsed-time ratio; no kernel-only throughput measurement or new profiling campaign was performed.
