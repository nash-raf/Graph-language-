# GPU execution for proved schedules

## Selection

The compiler uses the existing spatial and temporal results. Device lowering adds no independence proof.
Schedule priority remains combined rounds and spatial batches, rounds, source phases, spatial batches, then serial work.
Phase and spatial composition remains outside this implementation.

Select the GPU backend with `--gpu` or `FORCE_GPU=1 FORCE_CPU=0`.
Device capability is a separate check. Unsupported calls, missing allocation extents, or unavailable CUDA infrastructure select the CPU realization before effects.
An inconclusive proof cannot select a concurrent GPU schedule.

The runtime uses the existing GPU cost function.
`SGPL_GPU_MIN_TRIPS` controls the work threshold. Its default is 4096.
Tests set this threshold to zero to require device execution for small fixtures.
GPU execution uses no thread-budget or device-budget model.
`SGPL_NUM_THREADS`, parent CPU grants, `SGPL_GPU_BLOCK_BUDGET`, and
`SGPL_GPU_BUDGET_FROM_LEDGER` do not limit device task submission.
The legacy block-budget setter and descriptor budget fields remain ABI-compatible but have no device effect.
CPU fallback and host lifecycle work retain their CPU execution rules.
Spatial kernels assign separate lanes to whole proved source or destination groups.
An independent pair witness assigns separate lanes to occurrences when lifecycle hooks permit it.
Source-owned lifecycle hooks can require a source group as a coarsening.
Each lane visits its group's occurrences in their original order.
Ordered round and phase callbacks retain one lane when no finer spatial witness exists.

## Available realizations

| Selected schedule | Device work | Host lifecycle |
| --- | --- | --- |
| Spatial CleanCut | Complete accepted source or destination batches, including their owned source hooks | Preparation, accepted destination source coverage, snapshot publication, combination, and completion |
| Round | Complete outlined round components. Each component retains its round order | Graph preparation and final induction publication |
| SourcePhase | Each accepted phase component, with its original phase and neighbor order | Source frame reset and join before the next source |
| RoundSpatialDag | CleanCut batches from independent rounds, with separate round frames | Round setup and finalization, with bounded transfer epochs |
| Legacy DAG | Tasks with supplied device kernels and explicit relocation records | Existing precedence and lifecycle callbacks, with explicit transfer nodes when needed |

The compiler generates NVPTX variants from the already accepted callbacks.
It preserves callback value maps and consumes the original proof results.
It does not run interference analysis on device callbacks.
Host callback pointers alone cannot execute on the device. A legacy descriptor must supply a supported kernel.

Device helpers support immutable neighbor traversal, source claims, snapshot reads, partition storage, source gates, reductions, and private frontier lanes.
The compiler keeps source hooks at their accepted positions.
The host combines partial results in ascending partition order.

## Descriptor and ownership

`gpu_runtime.h` defines the versioned `sgpl_gpu_plan` interface.
It records the schedule, temporal scope, tasks, kernel names, buffers, pointer relocations, and DAG edges.
Its legacy worker and block budget fields are ignored during device execution.
The operations are reserve, upload, execute, download, and release.
Kernel arguments are an explicit frame pointer, a task index, and a task status pointer.

Each invocation owns its streams, staging buffers, statuses, and task descriptors.
Generated GPU programs also own a device residency scope for their language arrays and immutable layouts.
Invocation frames and runtime scratch retain explicit mirrors and transfers.
Each active round owns its frame, contexts, claims, snapshots, partials, frontier lanes, and scratch.
Immutable graph views can be shared. Concurrent kernels do not publish round pointers through device globals.
The legacy module-global routes use a lock around their upload, execution, and download interval.

Overlapping host buffer ranges share one device mirror.
Relocation preserves pointer offsets and aliases. Download restores host pointer values before publication.
The descriptor must provide valid allocation extents. Address overlap does not establish independence.

Compiler context packing currently supports at most 64 partitions, 64 operations, 64 snapshot entries, and eight private scalar and array entries each.
Larger contexts use the CPU realization before effects. These limits concern device capability, not proof status.

## DAG and transfer order

Device execution uses `autograph_execute_dag_async`, an asynchronous mode of the existing DAG scheduler.
It shares descriptor validation, ready queues, dependency counters, and completion bookkeeping with CPU execution.
One host coordinator submits every ready kernel without allocating a worker team or consulting resource grants.
Each device task owns a stream. CUDA determines when submitted kernels execute.
Stream polling records completion before the scheduler releases dependent nodes.
On failure the scheduler stops submission and drains all submitted work.
Host callbacks execute directly and retain their required lifecycle ordering.
The implementation uses the [CUDA Driver stream API](https://docs.nvidia.com/cuda/cuda-driver-api/cuda_driver_api/group__CUDA__STREAM.html).

Combined execution uses these boundaries for each bounded window:

1. Reserve frames, buffers, streams, kernels, and descriptors.
2. Execute round setup.
3. Upload each shared allocation once.
4. Execute batches from the active rounds.
5. Join device work and download each modified allocation once.
6. Execute successful round finalization and release resources.

Round groups are contiguous. Each group advances through its rounds in order.
Batches contain contiguous ranges of whole CleanCut partitions.
GPU groups and batches are bounded by descriptor representation capacity, not CPU worker counts.
The compiler uses up to 65,536 round components, the runtime DAG node capacity.
Combined windows also obey the supported frame-field representation limits.
Storage allocation and overflow checks precede effects. Allocation failure is reported through the existing failure contract.
Default GPU CleanCut geometry uses 64 partitions, the supported device context representation.
`SGPL_CLEANCUT_PARTITIONS` can specify another geometry. CPU worker counts do not change it.
These barriers preserve lifecycle order. They add no proof of independence.

For a legacy DAG, `SGPL_GPU_TASK_UPLOAD` and `SGPL_GPU_TASK_DOWNLOAD` delimit explicit transfer epochs.
Host/device dependency boundaries must include these nodes and their required precedence edges.
`explicit_transfer_epochs` disables the convenience dispatch's automatic initial upload and final download.

## Outcomes and failures

- `SGPL_GPU_DONE`: device execution succeeded.
- `SGPL_GPU_UNAVAILABLE`: device dispatch remained unstarted. The original CPU realization can execute.
- Negative status: execution failed. The caller must propagate failure without replay.

Reservation performs no user effects. Kernel lookup and pointer validation precede dispatch.
Once setup begins, an upload error is an execution failure.
Launch, synchronization, and download errors cancel pending tasks and join active tasks.
Failed work does not run finalization. A later-window failure does not replay earlier windows.
Download stages transfers before it publishes modified storage.
Resident language arrays publish their device version at the next host access or scope exit.
Failure during that publication terminates execution without CPU replay.

## Proof groups inside a CleanCut batch

Layout preparation groups records by the accepted CleanCut owner.
It preserves source hooks and the order of duplicate occurrences inside each group.
It does not inspect conflicts or discover partitions.
The spatial kernel uses a grid of 32 blocks with 128 lanes per block.
Lanes visit whole groups through a grid-stride loop. CPU worker grants do not determine this geometry.
Combined kernels use the same group execution within each round's batches.

Source accumulators use separate lane-local storage for supported source frames.
Accepted private folds use compare-and-swap updates on partition-private cells.
The update preserves the accepted operator, including signed, unsigned, and floating-point variants.
The host still combines partition results in ascending partition order.
These atomics implement the accepted fold contract. Atomicity does not establish independence.
Unsupported cell widths or device operations retain the CPU capability fallback.

Frontier lanes use atomic membership claims and append positions.
The existing set contract permits unordered append positions.
Duplicate activations produce one membership entry. Source-owned lanes retain the final cross-partition set merge.
Programs that observe append order retain their existing proof refusal.

## Data residency and host coherence

The compiler inserts a residency scope and host access hooks after device lowering.
It does not instrument device callbacks or change proof selection.
The frontend marks language arrays for the compiler's explicit residency registration.
Only registered language allocations enter the mutable cache. Runtime scratch retains eager copies.
Language arrays retain one device allocation across dispatches and iterations.
Interior pointers retain their offsets in the complete allocation mirror.
Overlapping descriptors merge their access flags.
Read-only device captures do not cause false host publication of unchanged arrays.

Host reads download a modified allocation before the read.
Host writes first obtain the current version, then invalidate its device version.
The next GPU dispatch uploads that changed allocation once.
Unknown external host calls and CPU fallback synchronize and invalidate resident arrays before execution.
Scope exit publishes remaining results and releases device allocations.
Free operations remove resident allocations and their extent records.
Array swaps retain physical allocation identities and refresh pointer bindings without copying array contents.
Stack lifetime endings remove their registrations before storage can be reused.
Pointer rebinding does not improve proof coverage. The compiler can retain CPU execution for an inconclusive swap loop.
The generated retention fixture uses stable bindings and a production spatial proof.
Direct API tests also check changed pointer bindings and interior aliases.

Opaque immutable graph and execution records retain device copies when their contents match.
The cache compares complete bytes. It does not use hash equality as evidence.
Other read-only allocations retain their address identity and true aliases.
A bounded cache of immutable versions prevents changing topology from accumulating unlimited history.
Runtime frames, snapshots, private arrays, and other uninstrumented storage retain eager synchronization.
Required host setup, combination, and finalization can still require transfers.
Keeping an allocation resident does not remove a required host dependency.

`SGPL_GPU_DEBUG=1` reports uploaded bytes and bytes reused on the device.
`SGPL_GPU_GROUP_TRACE=1` records device timer intervals and lane identifiers for proof groups.
`[gpu-groups]` reports actual lanes and overlap inside a batch.

`SGPL_GPU_DEBUG=1` reports capability refusals and completed kernel tasks.
`[gpu-task]` records actual kernel execution. CPU fallback produces no such record.
Proof metadata, backend capability, and observed overlap remain separate facts.
Zero or one device unit cannot produce component overlap. One CPU worker can submit multiple independent device tasks.

## Validation

Run in Ubuntu WSL with LLVM NVPTX and a CUDA device:

```sh
bash test/run_gpu_schedule_tests.sh
bash test/run_gpu_ir_tests.sh
bash test/run_gpu_invocation_tests.sh
bash test/run_gpu_group_parallel_tests.sh
```

The generated-program tests compare outputs at 1, 2, 4, and 8 workers.
They require actual kernel records and also test refusal before effects with CPU execution.
Fixtures cover phases, rounds, both CleanCut ownership choices, snapshots, claims, private arrays, folds, guards, and frontier membership.
Combined fixtures use production proof selection. They contain no proof-status override.

The LLVM fixture suite checks complete round frames, dynamic scratch, snapshots, guards, PHI outputs, and both CleanCut ownership choices.
Its two bitmap frontier fixtures supply an accepted round result. Their output identifies this assumed premise.
All other LLVM fixtures use production proofs.

The invocation tests use device `%globaltimer` intervals to check overlap across rounds and within batches.
They also check source joins, ordered legacy tasks, aliases, multiple transfer epochs, bounded windows, and failure without replay.
GPU stream scheduling can serialize some eligible components. A proof permits concurrency but does not require hardware overlap on every invocation.

`SGPL_GPU_FAIL` injects deterministic driver, module, stream, reservation, upload, launch, synchronization, and download failures.
Use `launch:N`, `sync:N`, or `stream:N` to select a task or stream index.
Fault injection changes runtime capability or execution. It does not change a proof result.

### Validation record: 2026-10-12

Ubuntu WSL validation used LLVM 20.1.2 and an RTX 3050 with driver 610.88.

- All 15 generated-program fixtures passed at 1, 2, 4, and 8 workers. Each required actual device execution and sequential-reference equivalence.
- All 11 LLVM device fixtures passed at those worker counts. The two bitmap frontier fixtures retain their stated round assumption.
- Invocation tests passed alias relocation, ordered transfer epochs, source joins, bounded windows, allocation and driver faults, and failure without CPU replay.
- Device timer intervals showed spatial overlap and round overlap during one combined dispatch.
- Existing DOALL and cooperative GPU launch compatibility tests passed with zero CPU callback executions.
- All 14 spatial, temporal, executor, snapshot, composition, reduction, round-separation, and expression regression suites passed.
- The existing GPU cost and worker-budget integration gate passed all three configurations.

### Resource-budget removal: 2026-10-12

GPU dispatch ignores CPU thread grants and all legacy block-budget controls.
The asynchronous DAG mode submits ready tasks without a host worker team.
Invocation tests run with one and four CPU workers. Device timer intervals must
show independent phase overlap under a parent grant of one, and overlap on both
axes of a two-round, two-batch dispatch.
The test uses a reduced 32-node descriptor capacity to exercise multiple windows.
It checks failures without replay, dependencies, aliases, and legacy launch compatibility.
Proof algorithms, admissibility rules, and CPU worker budgeting are unchanged.
All 13 suites in the budget-removal validation record passed in Ubuntu WSL.
The record covers generated device schedules, LLVM device fixtures, asynchronous
DAG bookkeeping, CPU pool regressions, fork/join, snapshots, temporal and spatial
execution, composition, reductions, round separation, and expression execution.

### Proof groups and residency validation: 2026-10-12

All 14 regression suites passed in Ubuntu WSL on the RTX 3050.
Generated dense fixtures used production proofs at 1, 2, 4, and 8 CPU workers and 1, 3, and 8 CleanCut partitions.
Device timers showed overlap inside one batch for source, destination, and occurrence groups.
Occurrence fixtures executed all 2049 records, including duplicate edges.
Source hooks and accumulators retained zero-neighbor behavior.
Private folds, staged claims, frontier sets, and combined round execution matched sequential outputs.

The generated retention fixture ran 20 GPU iterations with one final array download.
API tests checked host writes, pointer rebinding, interior aliases, distinct read-only storage, and stack expiration.
An inconclusive compiler swap fixture retained CPU execution and matched its reference.
GPU-only helpers were removed from host IR, including unoptimized LLVM fixtures.
Failure, cancellation, CPU fallback, and worker pool regressions passed.

The five-algorithm benchmark matrix covered all 11 original and synthetic graphs.
All 220 independent-reference checks passed across 880 runs with no process failures.
GPU kernels executed in 100 configurations. The other 120 retained CPU execution.
The original algorithm files were unchanged.
With eight CPU workers and eight fixed partitions, Facebook PageRank process time decreased from 3890.610 ms to 1012.542 ms.
Bio-CE-CX PageRank decreased from 2941.335 ms to 510.305 ms.
Both timings include loading, preparation, CUDA startup, computation, transfers, summary, and cleanup.
Host lifecycle work and runtime scratch can still require transfers.
GPU invocation tests also submit device work from inside a CPU scheduler node.

## Unchanged proof limits

The deferred bitmap round summaries and unsupported outlining shapes remain in
`TEMPORAL_SPATIAL_IMPLEMENTATION_GAPS.md`.
GPU frontier helpers do not make an inconclusive bitmap round result independent.
The existing frontier emitter tests that supply a round result remain tests with an assumed premise.
The supplied theorem PDF remains unchanged.
