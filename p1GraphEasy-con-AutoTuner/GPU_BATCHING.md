# Independent spatial launch batching, v1

The spatial executor already constructs a dependency-free family of partition
tasks between upload and download. The batching rule replaces only that family
with one device launch. Setup, transfer epochs, publication, finalization,
partition ownership, per-group record order, and joins remain unchanged.

The compiler marks compatible spatial component kernels with `_batch_v1`.
Their index argument's high 32 bits contain a partition count, or zero for
the original single-partition ABI. The low 32 bits contain the first partition.
With batching, blocks `32*p` through `32*p+31` execute partition `p` using the
same 4096 local lanes and group stride as the former individual launch. Ordered
partitions still execute on their local lane zero. Error status is atomically
combined into the invocation's status cell and checked at the existing join.

Only the spatial executor's independent antichain is eligible. Temporal DAGs
and round-spatial span kernels retain their own index contracts and execution.
Kernel naming is a compiler capability marker, not a dependence proof: the
existing spatial executor establishes the independent family.

`SGPL_GPU_BATCH=0` selects the retained unbatched path in the same binary.
Other values, or an unset variable, enable batching for compatible kernels.
Older kernels retain the original path. New kernels run unbatched with older
runtimes because an ordinary partition index has a zero high word. Launch
geometry overflow refuses batching. There is no CPU replay after device effects.

Build and focused tests (WSL, existing investigation venv):

```sh
/home/kazi/p1_local_regressions_20261019/venv/bin/python tools/build_gpu_batch.py
bash test/run_gpu_batch_tests.sh /home/kazi/p1_gpu_batch_20261104/versions/candidate02
bash test/run_gpu_invocation_tests.sh
```

The focused suite compares exact outputs with batching disabled/enabled at
1/4 workers and requested partition counts 1/3/64/65. A 129-vertex fixture with
duplicate edges and self edges exercises actual 64-partition launches and the
existing context-descriptor refusal above 64 partitions (CPU fallback).
The other fixtures cover source/destination ownership, snapshots, accumulators,
private targets, claims, folds and frontier output. Driver failure checks the
pre-effect CPU fallback. Invocation contracts additionally exercise transfer,
launch and synchronization failure handling and legacy/round-spatial paths.

`tools/benchmark_gpu_batch.py` uses the saved 61-graph, four-algorithm sources,
independent output validators, one warmup and five fresh-process timed samples.
It preserves the original timer boundaries and compares with immutable saved
GPU and six-core CPU observations. These comparisons are historical/unpaired.
CPU-only cases are excluded from GPU speedup summaries. Results are written to
`bench_results/gpu_batch_20261104/benchmark/` and native artifacts to
`/home/kazi/p1_gpu_batch_20261104/`.

This rule does not remove context startup, allocation, host coherence hooks,
inter-phase transfers, or required round boundaries. Those costs can limit
whole-program improvement even when the launch count falls sharply.

## Local results, 2026-11-04

All 244 algorithm/graph cases passed independent output validation. The 165
GPU-executed cases retained their offload selection and reduced device task
launches by exactly 64x. The other 79 cases remained CPU-only. Nine focused
fixtures, the invocation contract suite, and an old-kernel/new-runtime
compatibility check passed. The completion audit verified 1,220 timing samples,
305 input hashes, executable hashes, and the saved timer identities.

Historical/unpaired geometric-mean speedups over the saved GPU runs:

| Algorithm | GPU cases | Whole process | Compute region |
|---|---:|---:|---:|
| BFS | 49 | 0.969x | 0.977x |
| CC | 58 | 1.000x | 1.012x |
| PageRank | 58 | 1.079x | 1.646x |

Values above 1 mean faster. No case beat its saved six-core CPU time.
The resulting whole-process GPU slowdowns versus saved CPU were approximately
73.7x for BFS, 117.6x for CC and 65.6x for PageRank.

A separate alternating AB/BA same-binary check used five samples per mode on
three selected cases. Batching-on versus batching-off speedups were:

| Case | Whole process | Compute region |
|---|---:|---:|
| PageRank, hamming8-2 | 1.095x | 2.060x |
| BFS, Slashdot | 1.004x | 1.005x |
| CC, fe-4elt2 | 1.407x | 1.449x |

These selected controls are descriptive, not a full paired dataset or a
statistical significance claim. The GPU also serves the Windows desktop.
Full observations are in `bench_results/gpu_batch_20261104/benchmark/comparison.csv`;
control samples and the completion audit are in the parent directory.

To use the frozen candidate compiler with the existing build/run script:

```sh
GP_BIN=/home/kazi/p1_gpu_batch_20261104/versions/candidate02/GraphProgram \
SGPL_GPU_BACKEND=1 GRAPH_FILE=test/spatial_destination.graph bash 03_run.sh
```
