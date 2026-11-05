# Results: CPU execution over selected native layouts

## Outcome

Implemented CPU range batches and native traversal for autotuner-selected CSR,
PCSR, BCSR and SET. CPU lowering no longer materializes CleanCut edge copies.
The existing effect proofs, ownership groups, lifecycle and joins still determine
legal execution. GPU keeps its existing backend and CleanCut construction.
Existing certified CPU specializations/worklists remain. Context ABI is now 5.

The replacement is correct on the tested workloads, but is **not an overall
performance improvement**. Destination-owned generic traversal scans the active
native rows once per partition. This trades edge-copy preparation for repeated
scans and substantially hurts some BFS cases.

## End-to-end CPU benchmark

AMD Ryzen 5 5500, six physical cores, Ubuntu WSL. Four algorithms across 61 graphs:
244 cases, five measured process executions per version, 2,440 samples. Each case
gets one warmup per version, then alternating baseline/candidate ordering.
Affinity is CPUs 0,2,4,6,8,10; six workers, CPU forced on both versions. Compiler
and runtime flags, generated source and graph inputs match; hashes are retained.

Baseline: local checkpoint `509f7602daa42d4c114c0b80d4b1d0e8a6959f77`.
Candidate: frozen native07. Measurements use CLOCK_MONOTONIC around native
fork/exec/wait4. They include process launch, graph loading, conversion, runtime
preparation, execution and exit. Compilation is excluded. Loader/filesystem caches
are warmed, so these do not measure first-ever graph-cache construction.
No build/test workload ran concurrently. No samples or cases were excluded.

| Algorithm | Cases | Geometric mean speedup | Sum of baseline case medians | Sum of candidate case medians |
|---|---:|---:|---:|---:|
| BFS | 61 | 0.801x | 524.77 ms | 863.53 ms |
| SSSP | 61 | 1.001x | 814.99 ms | 832.98 ms |
| Connected components | 61 | 0.973x | 505.17 ms | 513.79 ms |
| PageRank | 61 | 1.165x | 717.10 ms | 620.53 ms |
| All | 244 | 0.976x | 2,562.03 ms | 2,830.82 ms |

Speedup means baseline/candidate; above one is faster. Sum-of-medians is an
aggregate over cases, not the duration of one run. That aggregate rose 10.49%.
BFS geometric-mean runtime rose about 24.8%; PageRank fell about 14.1%.
SSSP/CC differences are small relative to this short-run measurement variation:
median relative sample MAD was 4.56%. Five samples do not establish statistical
significance for small differences. 79 cases improved by over 5%, 87 regressed by
over 5%, and 78 remained within that band; these are descriptive counts only.

Largest regression: BFS on `dimacs10/inf-luxembourg_osm.edges`, 18.879 ms to
97.469 ms (5.16x slower). The other Luxembourg road input shows the same pattern.
Largest improvement: PageRank on `misc/delaunay_n13.edges`, 10.653 ms to 6.474 ms
(1.65x faster).

The BFS validation diagnostics show destination ownership with 24 partitions and
no prepared pair records. The new executor scans active source rows per owner;
that repeated work is a concrete added cost. Attribution of the entire slowdown
to that cost is an inference, not a measured phase-level profile. The unchanged
autotuner cost model does not explicitly price the new P-fold filtering cost.
Do not deploy this as an unconditional speed optimization. A next optimization
would need to price/reduce owner enumeration work while keeping existing proof
and ordering obligations; no new transformation was added in this task.

Raw [samples](evidence/benchmarks/native07/samples.json),
[all comparisons](evidence/benchmarks/native07/comparison.json),
[analysis](evidence/benchmarks/native07/analysis.json),
[variation](evidence/benchmarks/native07/variability.json), and
[protocol](evidence/benchmarks/native07/protocol.json) are retained.

## Correctness and race checks

- Baseline, native06 and final native07 each pass all 732 independent full-vector
  checks: 244 cases at 1, 2 and 6 workers. Existing SciPy/NumPy reference checks
  cover BFS, SSSP, connected components and PageRank.
- 36 compiled fixture/layout comparisons pass at all three worker widths. Actual
  CSR/PCSR/BCSR/SET activation is checked, including staged and round/spatial paths.
- All 12 temporal and 13 remaining claims fixtures match complete baseline stdout
  at 1, 2 and 6 workers in separate differential runs.
- Current CPU runtime passes ASan/UBSan at 1/2/6 (native15_asan). TSan passes at
  1/2/6 (native14_tsan); only its known pre-main WSL address-map/startup failures
  were retried. All attempts are retained; no race reports were suppressed.
- Native tests cover source/destination ownership, duplicates, isolated sources,
  ordered array occurrences, multiple append sites, lifecycle/order, overlap,
  allocation failure, stale topology/layout, empty graphs and all four layouts.
  Generic native tests assert no CleanCut edge payload or GPU record buffers.
- 29 of 31 existing suites pass after the test harness corrections below.
  All seven GPU suites pass, including real device submissions, batched launch
  counts, storage/synchronization, driver fallback and no replay after effects.
  The final runtime also reproduces the checkpoint's >64 descriptor early gate
  with matching output in both batch modes.
- Three existing autotuner frequency, iterator and cost-model suites pass.

These checks support the unchanged proof/ownership invariants. They are not a
universal proof that arbitrary programs are race-free. Layout/topology mutation
during an invocation remains outside the existing accepted contract.

## Existing failures and harness corrections

The original suite failures remain recorded, not marked passed:

1. `run_parallel_claims_tests` first fails compiling `membership_call_aliases`:
   raw CSR access conflicts with a graph bitmap query. The checkpoint compiler
   rejects the identical fixture too. Skipping only that fixture lets seven more
   fixtures pass, then `reduce_plus_write` expects the old source-phase-dag
   classification; both versions emit a conservative sequential refusal for a
   mutable source guard. All 13 remaining fixtures pass separate output checks.
2. `run_temporal_tests` expects `separate components members=0,1,2,` on its first
   fixture. Both versions fail that same diagnostic assertion; sequential output
   is 414. All 12 fixtures pass separate complete-output comparisons.

Corrections committed with the implementation:
- Frequency test accepts the new native batch API; cost/count checks unchanged.
- Round/spatial overlap probe wraps the new batch builder as well as the old one,
  forcing the same two partitions and retaining the same overlap/order checks.
- GPU batching test accepts the existing >64 descriptor rejection before launch
  logging, asserts no device effects and compares output with supported widths.
  All supported-width device and launch-reduction checks remain unchanged.

## Version provenance and retained artifacts

Broad suites, forced layouts, temporal/claims comparisons and TSan exercised the
native06 core. Native07 has the identical compiler binary and only changes one
runtime debug message to report actual source/record counts. Final native07 was
relinked for every case, passed all 732 checks plus ASan/UBSan, and is the version
benchmarked. Reused generated objects required identical compiler and source;
all final runtime objects were relinked. Final audit compares production source
against the frozen version. See [audit](evidence/final_audit.json).

Full logs/binaries: `/home/kazi/p1_layout_native_20261105` in Ubuntu WSL.
Portable evidence is under `codex_doc/evidence`; index_native06.json and
index_native07.json contain hashes and original paths. Large logs have excerpts.
Failed intermediate versions are retained in WSL. No push or contribution
attribution. README.md and LATEST.md contain resume instructions.
