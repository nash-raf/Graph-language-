# Graptor OpenMP and Cilk API comparison

The 26 October 2026 local experiment compares the saved Graptor OpenMP
executables with Graptor's upstream `BACKEND_cilk` runtime API implementation.
SGPL compiler/runtime sources and ARS-1 are unchanged.

The application source is the exact 321-file Graptor snapshot in the frozen
B3safe2 export. Original OpenMP binary hashes match their historical manifest.
The application compiler remains Ubuntu GCC 13.3.0 for both backends, with
the existing C++23, O4, `-march=native`, `MAX_VL=8`, `LONG`, `UNDEBUG` and
`VEBOGraptorPullDataParCached` settings. Backend-specific changes are the config
selector, Cilk API headers/library instead of `-fopenmp`/libgomp, and the runtime
worker environment. The upstream Cilk backend is used without source patches.

The Cilk runtime is built from OpenCilk/cilkrts commit
`507a93234a480431ee8f7e49bbb464e3572618f5`. It needs a Cilk-capable compiler;
GCC 7.5 is extracted from hash-verified official Ubuntu packages into an
isolated external directory and used only for this runtime. The application
compiler and system toolchain are not replaced. Runtime sources, package
identities, commands, dependency hashes and output hashes are retained.

This is specifically the same-GCC **Cilk API backend**, not an OpenCilk frontend
comparison. The upstream Cilk header selects its serial `parallel_for` macro
fallback when the application compiler lacks Cilk language extensions; the
`parallel_loop` and partition APIs call the real parallel Cilk runtime. This
backend distinction is preserved and disclosed rather than hidden by a source
rewrite. Results cannot establish the performance of every Cilk frontend or
runtime configuration.

## Controls and correctness

Every run uses the existing 61 graphs and their exact binary/weight inputs.
PageRank remains double precision and uses 20 iterations; SSSP retains its
existing float32 sidecar weights. Algorithm arguments, vector width, source
vertices and eight graph partitions are fixed. Timings use workers on six
distinct physical cores: CPU IDs 0, 2, 4, 6, 8 and 10. The process affinity mask
is the same; OpenMP binds within these cores, while Cilk schedules within the
same mask. No NUMA extension, new graph ordering or new algorithm is introduced.

An initial correctness-only attempt with the generic one-partition command
hit the saved OpenMP build's existing VEBO assertion. Those failed logs are
preserved. Subsequent validation fixes the partition count at the historical
eight for every backend and width, including 1/2-worker correctness runs.

Original OpenMP, rebuilt OpenMP and Cilk pass 2,196 independent full-vector
validations across all four algorithms, all 61 graphs and 1/2/6 workers. All
183 exported PageRank comparisons are bitwise identical, with zero absolute
difference. Worker probes directly exercise each Graptor backend's loop API
and observe exactly 1/2/6 workers with no affinity violations. Probe-only sleeps
let workers start before a tiny loop finishes; probes are not timing samples.

The rebuilt OpenMP control uses the same frozen source and compiler settings.
Its `.text` and `.rodata` sections match the original BFS/SSSP executables
exactly; CC/PageRank have identical `.rodata` but differing `.text` hashes.
Both original and rebuilt OpenMP executables are therefore measured, and both
comparisons are reported rather than assuming byte-identical rebuilds.

## Measurement

`tools/benchmark_graptor_backends.py` builds, validates and measures independently.
The timed variants are original OpenMP, Cilk, rebuilt OpenMP, original OpenMP
A/A and Cilk A/A. The protocol specifies five primary and twenty confirmation
observations per variant/case, with one warmup per block. Execution order rotates
and alternates. A failed identical-binary control triggers at most one complete
retry of its block; every original block and retry is retained.

The same whole-process CLOCK_MONOTONIC wrapper and existing monotonic
`gettimeofday` shim serve every backend. Graptor's recorded compute region is
reported separately from end-to-end time. No profiling, tuning, calibration or
compilation occurs during measurement. Paired permutation tests use the retained
100,000-resample method and Holm correction. Raw win counts are descriptive.

Artifacts are external: `C:/p1-graptor-backends-benchmark-20261026` and
`/home/kazi/p1_graptor_backends_20261026`. The machine-readable protocol records
all graph, source, configuration, library and executable identities. Performance
outcomes are added only after measurement and the final integrity audit.

## Confirmed outcome

The confirmation block completed with stable identical-binary controls. The
experiment collected 30,500 timed observations and 2,440 warmups with no timed
launch failures. The final audit verified all 12 executable fingerprints and
305 input fingerprints, with frozen sources unchanged.

The following are geometric means of per-graph mean-time ratios across 61
graphs. Each ratio is OpenMP time divided by Cilk time: above one favors Cilk.
Compute means Graptor's recorded region; end-to-end includes the whole process.

| Algorithm | Saved OpenMP / Cilk E2E | Saved OpenMP / Cilk compute | Rebuilt OpenMP / Cilk E2E | Rebuilt OpenMP / Cilk compute | Cilk raw E2E wins |
|---|---:|---:|---:|---:|---:|
| BFS | 0.9037 | 1.3206 | 0.8998 | 1.2347 | 16/61 |
| SSSP | 0.8974 | 1.0570 | 0.8983 | 1.1273 | 11/61 |
| CC | 0.8310 | 1.1183 | 0.8362 | 1.1042 | 5/61 |
| PageRank | 0.8511 | 0.9398 | 0.8300 | 0.8693 | 8/61 |

Cilk's end-to-end time is approximately 10.7%, 11.4%, 20.3% and 17.5%
higher than saved OpenMP for BFS, SSSP, CC and PageRank respectively. The
algorithm-level end-to-end differences pass the recorded paired permutation
tests against both OpenMP controls (Holm-adjusted p = 0.00496). Raw graph wins
are descriptive and are not counts of statistically significant wins.

Cilk improves the recorded compute region for BFS, SSSP and CC, but the
whole-process result is slower for every algorithm. PageRank is slower in both
regions. These observations do not isolate the cause among backend scheduling,
initialization, preprocessing and upstream serial helper loops. No profiling or
backend tuning was performed. Both blocks and all raw observations remain in
the external report directory; the table uses the prespecified confirmation
block. SGPL was not remeasured, so these results do not establish a new paired
SGPL-versus-Cilk ratio.

The local HTML report is `C:/p1-graptor-backends-benchmark-20261026/final_report.html`.
The repository record is `reports/graptor_backends_benchmark.json`; its protocol,
correctness evidence and audit identify the measured builds and inputs.
