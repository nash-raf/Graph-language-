# Remaining runtime gap: profile and next targets

This follow-up profiles the all-four optimized CPU executables. No production
compiler/runtime source or default configuration was changed in this follow-up.

## New measured configuration improvement

Setting `OMP_WAIT_POLICY=PASSIVE` and `GOMP_SPINCOUNT=0` only in child processes
reduces complete-process time on this machine. The four-worker limit remains
unchanged. Eleven randomized pairs per case used the same optimized binaries,
fixtures, four physical cores and frozen calibration as the prior comparisons.
All 110 timed processes matched their correctness goldens.

| Case | Default process ms | Passive process ms | Paired passive/default ratio |
|---|---:|---:|---:|
| BFS | 65.20 | 55.25 | 0.851 |
| CC | 78.45 | 70.38 | 0.908 |
| k-core | 71.82 | 64.28 | 0.896 |
| PageRank | 70.23 | 62.66 | 0.891 |
| SSSP | 156.02 | 146.29 | 0.928 |

Paired bootstrap process-time intervals exclude 1 for all five cases. These
are new paired measurements; do not subtract their absolute times from earlier
tables taken at different times. The data does not establish a corresponding
traversal-region speedup: the principal gain is elsewhere in process execution.
For example, SSSP's median user CPU consumption falls from 125.18 to 78.24 ms,
while its reported traversal region is 35.05 versus 34.77 ms. CPU consumption
is summed across threads and differs from elapsed process time.

The settings were tested, not installed as persistent defaults. The result
supports testing a startup policy that avoids busy waiting in the supporting
OpenMP team while SGPL's own execution team runs. It does not justify changing
the DOALL/DOACROSS thread-budget equations or the effect proofs.

## Current SSSP CPU profile

Forty fresh correct SSSP runs were sampled with the installed native perf tool,
using `cpu-clock`, frequency 997 and DWARF call stacks. There were no lost
samples. The leading self samples are:

| Function/region | Share of sampled user CPU |
|---|---:|
| `sgpl_array_records` | 27.80% |
| Two libgomp wait loops | 22.52% + 13.71% |
| Generated `sgpl_pair_work` | 15.28% |
| `autograph_build_clean_cut` | 6.74% |
| `sgpl_exec_partition_body` | 3.56% |
| libc memset | 2.56% |
| `cc_arc_next` | 1.95% |
| `sgpl_exec_spatial_finish` | 1.25% |

The sampled libgomp instructions were disassembled: both addresses are `pause`
polling loops followed by a syscall wait path. These percentages are samples
of user CPU execution, not fractions of elapsed wall time and not predicted
speedups. Kernel work, blocking and other overhead are not attributed here.

## Next implementation candidates

1. **Passive support-team waiting.** This is the immediately measured local
   improvement. Apply a suitable default before OpenMP initialization while
   preserving explicit user configuration, then rerun CPU/GPU and budget tests.
2. **Parallel record preparation through the existing ledger.** Counts and
   fills currently traverse the frontier serially. Per-chunk counts and stable
   prefix offsets could distribute preparation while preserving original
   source/neighbor order and every append ordinal. Use the existing pool and
   its grants; retain serial fallback for unavailable/nested teams. Allocate
   and validate before any user callback. This needs a localized runtime
   implementation and ordering/failure tests, not a new effect system.
3. **Lightweight cut setup for array execution.** The emitted array stage calls
   `autograph_build_clean_cut`, whose initial build constructs both source
   slices and destination rows. The array executor then builds its own
   occurrence records from native adjacency. Investigate which cut views are
   actually consumed, and build unused views lazily. Preserve any metadata
   consumed by scheduling, lifecycle coverage and other stages; do not simply
   omit it without auditing those callers.
4. **Generated batch callbacks and native traversal fast paths.** Dispatch one
   compiled callback for a whole accepted batch, with invariant context checks
   outside its inner loop. Native CSR traversal could further avoid temporary
   record traffic. Destination grouping, duplicate source occurrences and
   original within-group order make this more involved than the prior local
   changes; source-owned streaming alone would not address CC/SSSP's current
   destination-owned path. Effect proofs should select the valid fast path.
5. **Proof-driven reduction of algorithmic work.** Deduplicating relaxations or
   admitting live propagation could reduce the extra rounds/frontier volume,
   but needs explicit equivalence rules covering array order, multiplicity,
   intermediate values and convergence. Re-enabling legacy behavior without
   those rules would not retain the current contract.

Candidates 2–5 have not been implemented or benchmarked in this follow-up.
These measurements do not promise that any one candidate closes the entire
remaining gap. Fresh hardware calibration is also still a separate task for
assessing autotuner accuracy; it does not eliminate the recorded runtime work.

## Artifacts

`verify/bin/local_runtime_candidates_2026_10_14/` contains
`profile_remaining.py`, `remaining_sssp.perf.data`, `check_omp_wait.py`,
`omp_wait_raw.json` and `omp_wait_summary.json`. Reproduction:

```text
wsl -d Ubuntu -- python3 /mnt/d/sgpl/Graph-language-/verify/bin/local_runtime_candidates_2026_10_14/check_omp_wait.py
```
