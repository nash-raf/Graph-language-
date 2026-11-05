# GPU preparation, transfers and allocation reuse

These four runtime changes use the existing graph generations, CleanCut ownership,
compiler effects, transfer epochs and residency scopes. They do not change graph
algorithms, observable occurrence order, dependence edges or the offload policy.
The public execution-context layout is unchanged.

## 1. Reuse preparation

`gpu_record_reuse.inc` retains one bounded record template in the existing
thread-owned owner-map cache. Only complete full-vertex CleanCut views without
membership filters qualify. The key includes graph metadata identity, topology
generation, layout epoch and representation, graph sizes, copied partition
boundaries, traversal/grouping mode, source lifecycle hooks and private/parallel
execution modes. Each live context receives independent copies of the template.
Frontier occurrences, snapshots and mutable gates are rebuilt as before.

Grouping scratch retains capacity in that same cache. It visits and sorts only
the keys touched in each partition, preserving the original ascending key order
and stable order within each group. Serial private execution no longer builds an
ordered record copy that it immediately discards. The spatial executor also
avoids preparing the same context twice within one invocation.

Retained templates and grouping scratch share a 64 MiB limit per thread. Optional
allocation failures leave the old template intact or use ordinary preparation.
The existing owner-map thread destructor frees all retained preparation storage.

## 2. Evaluate existing rejection gates earlier

The existing late GPU path refuses contexts with more than 64 partitions,
operations or snapshots, or an unsupported destination-seen configuration.
It also calls `sgpl_gpu_policy_verdict` with estimated work and
`SGPL_GPU_MIN_TRIPS` (default 4096). In this path, work is the number of execution
records, including required source lifecycle records; zero work uses task count.
This is a minimum-work threshold, not a measurement of GPU time or money.

`sgpl_gpu_spatial_preflight` obtains the same positive record count from existing
complete CleanCut counts or a CSR frontier's degrees, preserving repeated source
occurrences and lifecycle hooks. A single-context descriptor can therefore make
the same refusal before constructing GPU records. Unknown facts, zero work,
relaxation contexts, compact full-graph contexts, and descriptors with additional
contexts/graphs retain the original late decision. The late gate remains in all
paths, and neither its threshold nor its policy changes.

## 3. Reduce transfers and host staging

For compiler spatial kernels carrying the existing `_batch_v1` ABI marker,
membership metadata, operation descriptors, private target metadata and snapshot
inputs are copied as read-only. Mutable state pointees retain their writable
effects. Aliases union their access flags: any writer upgrades the mirror and
removes immutable-layout status. The context itself remains writable because
execution status can alias its fields. Generic and unversioned callbacks keep
their conservative metadata access flags.

Valid pointer-free resident mirrors skip a redundant host staging copy as well
as the upload that was already skipped. Compatible adjacent host spans can use
one mirror only when they have identical access flags, contain no gap, preserve
256-byte device offsets, and do not cross registered language or mutable resident
storage boundaries. This combines storage/copies within an existing epoch; it
does not remove an epoch or infer task independence from addresses.

## 4. Reuse allocation capacity

Transient device buffers, their host staging capacity, and device status capacity
can be reused inside the existing compiler-owned residency scope. A reservation
leases capacity immediately, so nested or not-yet-started invocations cannot
share it. Current contents and relocations are initialized on every use. Only a
successfully joined invocation can return capacity for reuse; failed/unjoined
invocations discard it. Scope exit frees it.

The pool holds at most 32 allocations and 64 MiB of device capacity, with matching
host staging capacity. CUDA out-of-memory or host staging allocation failure can
evict idle capacity and retry. Leased allocations are never evicted. Pool misses
use ordinary invocation-owned allocation. This does not retain transient values
as valid language state.

## Controls and correctness

All four changes are enabled by default. Set a control to `0` to disable it:

| Control | Change |
|---|---|
| `SGPL_GPU_REUSE_RECORDS` | Templates and temporary grouping capacity |
| `SGPL_GPU_EARLY_REJECT` | Early evaluation of existing gates |
| `SGPL_GPU_TRANSFER_REDUCE` | Read-only metadata, resident staging and adjacent mirrors |
| `SGPL_GPU_REUSE_CAPACITY` | Transient device/staging/status capacity |

Existing batching and completion-certified synchronization controls remain
independent. Required joins, status reads, relocation restoration, transfer
publication and failure drains remain. Device failures after effects do not
replay graph work on the CPU.

The checks added for these changes are correctness-only:

```sh
python tools/check_gpu_four.py --version correct02 --build-only
native=/home/kazi/p1_gpu_four_correctness_20261104/correct02/native
bash test/run_gpu_storage_tests.sh "$native"
SGPL_TEST_SANITIZE=1 bash test/run_gpu_storage_tests.sh "$native"
bash test/run_gpu_four_tests.sh "$native"
python tools/check_gpu_four.py --version correct02
```

The Python runner requires the local validation environment and existing frozen
validation objects described in its source. It links only output-validation
executables and replaces the validation timer with a constant. It does not run
timed benchmark executables or a profiler. The focused script checks every subset
of the four controls against CPU references, plus fallback and failure paths.
The deterministic storage tests also cover invalidation, independent contexts,
allocation failure, nested leases, alias writers, alignment and residency
boundaries. Hardware invocation, synchronization and CPU frontier regressions
are checked separately. Results belong under
`bench_results/gpu_four_correctness_20261104/`; that existing results directory
name does not imply that performance was measured.

The correctness phase excluded timings. Benchmarking was subsequently authorized;
the production-flag results are linked below.

## Correctness results, 2026-11-04

The final `correct02` runtime passed 488 independent full-vector validations:
61 graphs, four algorithms, and both all-enabled/all-disabled controls. All 244
paired cases retained identical GPU task counts. GPU execution was exercised in
49 BFS, 58 CC and 58 PageRank cases. The saved SSSP fixtures stayed on their
existing CPU path; their checks establish fallback correctness rather than GPU
SSSP coverage. Enabled runs exercised 1,102 record-template hits and 4,471 early
refusals. These are structural counts, not performance measurements.

Ten focused fixture families passed all 16 feature subsets against CPU reference
output (160 comparisons) and ten driver-failure fallbacks. Existing-threshold
refusal and post-setup failure without replay passed. Deterministic preparation
and storage contracts passed AddressSanitizer/UndefinedBehaviorSanitizer,
including allocation failure and a partition-boundary change with unchanged
partition count. Invocation/coherence regressions passed at one and four
workers; CPU array/frontier regressions passed at one, two and six workers.
Partition boundaries 1, 3, 64 and 65 passed at one and four workers, including the
retained late capability fallback. Synchronization contracts also passed.

The 305 saved validation input hashes match the original manifest. Native source
hashes match the frozen final build. Machine-readable results and check summaries
are in [the correctness audit](bench_results/gpu_four_correctness_20261104/correct02/summary.json).
These correctness checks did not run benchmarks or a profiler.

## Benchmark results, 2026-11-04–05

The primary campaign uses matching `-O3 -g -fopenmp -pthread` host-runtime flags,
the original frozen pre-change runtime with batching/synchronization elision,
identical generated program objects, and six workers. Five fresh-process samples
per version alternate AB/BA. All progress and intermediate result writes stay in
the native Linux benchmark directory during sampling. Finished reports are copied
back to the workspace afterward.

All 244 cases completed: 165 cases executed GPU tasks and 79 remained on CPU
fallback. GPU cases alone contribute to the table. Ratios above one mean faster;
these are geometric means of per-case arithmetic-mean time ratios.

| Algorithm | GPU cases | Compute-region speedup | Whole-process speedup |
|---|---:|---:|---:|
| BFS | 49 | 1.048× | 1.041× |
| CC | 58 | 1.094× | 1.082× |
| PageRank | 58 | 1.216× | 1.008× |
| All GPU cases | 165 | 1.121× | 1.044× |

PageRank's whole-process interval includes no improvement. Regressions remain on
some inputs. In the selected PageRank/Slashdot control, disabling capacity reuse
was faster. The four controls are not uniformly beneficial or additive. Required
phase boundaries and identical GPU kernel work remain.

Compared with the saved batched-GPU measurements, the primary run is 1.222× faster
in compute time and 1.126× faster in whole-process time. Relative to the saved CPU
times, GPU compute-region time remains about 139 times the CPU time and
whole-process time about 74 times the CPU time. These historical comparisons are unpaired; the CPU was not
remeasured. The saved SSSP fixtures use CPU fallback throughout.

The primary audit contains 2,440 main timed executions and 120 selected-control
executions, all successful. The `-O3` build passed 488 independent output
validations and 160 focused feature-subset comparisons before timing. No runtime
implementation changed during sampling. The initial `-O2` experiment and partial
workspace-I/O run are preserved as supplementary/superseded experiments and do
not contribute to these primary figures.

[Full benchmark report](bench_results/gpu_four_benchmark_20261104/attempt03_o3_native_io/report.md)
includes intervals, absolute selected timings, switch controls, saved GPU/CPU
comparisons and the low-utilization observation. [Per-case times](bench_results/gpu_four_benchmark_20261104/attempt03_o3_native_io/comparison.csv)
and [raw measurements](bench_results/gpu_four_benchmark_20261104/attempt03_o3_native_io/results.json)
are retained with source and executable hashes.

The production-flag workflow is:

```sh
python tools/check_gpu_four.py --version correct03_o3 --optimization O3
bash test/run_gpu_four_tests.sh /home/kazi/p1_gpu_four_correctness_20261104/correct03_o3/native
python tools/benchmark_gpu_four_o3.py
python tools/summarize_gpu_four.py bench_results/gpu_four_benchmark_20261104/attempt03_o3_native_io
```

Use the existing validation venv. The scripts resume preserved experiments;
another campaign requires a fresh attempt directory.
