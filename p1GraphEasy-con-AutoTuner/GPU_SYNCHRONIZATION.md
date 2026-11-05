# Completion-certified synchronization removal

This runtime-only change builds on spatial batching. It removes three redundant
explicit waits while preserving status transfers, dependency edges, dirty-data
publication and conservative failure drains:

1. A successful `cuStreamQuery` certifies that a stream has completed, so its
   poll does not immediately repeat `cuStreamSynchronize`. Missing query support
   or a query error retains the blocking drain. Pending queries remain pending.
2. A successfully joined invocation does not synchronize each stream again
   during release. Failed or unjoined invocations retain those waits.
3. A resident allocation can omit a context-wide wait before host publication
   or unregister only after its owning invocations have successfully joined.
   Dirty bytes are still copied to the host. Active-use counters account for
   nested invocations sharing the thread-local allocation; an inner join cannot
   certify an outer invocation's pending work.

The CUDA stream completion contract is documented in the
[NVIDIA Driver API](https://docs.nvidia.com/cuda/cuda-driver-api/cuda_driver_api/group__CUDA__STREAM.html).
`autograph_execute_dag_async` returns success only after its active set is empty.
No dependence proof is inferred from a stream query, and no CPU replay is added
after device effects.

`SGPL_GPU_SYNC_ELIDE=0` retains the original explicit waits in the same binary.
The default enables removal. `SGPL_GPU_SYNC_STATS=1` reports actual poll/release
wait calls, certified omissions, and resident publication/unregister waits.
Statistics are disabled for timed samples. Status copies can themselves
synchronize; zero reported explicit waits does not mean zero synchronization.

`test/gpu_sync_elision_test.c` tests pending work, query errors, missing query
support, device status errors, failure cleanup, host publication and nested
invocation accounting with deterministic CUDA stubs. The existing hardware
invocation suite exercises relocation, transfer epochs, overlapping work and
failure without replay.

`tools/validate_gpu_sync.py` freezes a new runtime, reuses the batching compiler's
objects, independently validates all 244 saved cases, and runs four selected
alternating same-binary controls with five fresh-process samples per mode.
Its results are in `bench_results/gpu_sync_20261104/attempt02/` and native
artifacts in `/home/kazi/p1_gpu_sync_20261104/attempt02/`. Selected timing controls
do not establish a full-corpus speedup.

Run the deterministic contracts with an existing compatible native build:

```sh
bash test/run_gpu_sync_tests.sh /home/kazi/p1_gpu_batch_20261104/versions/candidate02
bash test/run_gpu_invocation_tests.sh
/home/kazi/p1_local_regressions_20261019/venv/bin/python tools/validate_gpu_sync.py
```

## Local results, 2026-11-04

All 244 cases passed independent validation, with unchanged offload selection
and launch counts. The audited runs omitted 3,988 poll waits, 3,988 release waits
and 4,118 resident publication waits; 2,828 conservative resident waits remained.
Unregister waits and implicit synchronization inside copies are not included in
those totals. Deterministic contracts and the existing hardware invocation suite
passed, including nested ownership and failure cleanup.

Four selected same-binary controls alternated original waits and wait removal,
with batching enabled in both modes and five samples per mode. Speedups above
one mean faster:

| Case | Whole process | Compute region |
|---|---:|---:|
| PageRank, hamming8-2 | 1.225x | 1.064x |
| PageRank, Slashdot | 1.032x | 1.034x |
| BFS, Slashdot | 0.999x | 1.007x |
| CC, fe-4elt2 | 1.007x | 1.012x |

This small sample supports only descriptive selected-case results; in particular,
fresh-process startup variance affects whole-process observations. It is not a
full-corpus performance campaign or statistical significance claim. The GPU also
serves the Windows desktop. Required status reads and CPU/device phase boundaries
remain substantial candidates for further profiling.
