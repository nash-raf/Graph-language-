# Ordered array frontiers

## Capacity

The compiler keeps capacity beside a dynamic array pointer. A pointer swap also
swaps capacity. Append guards are installed before O3 can unroll a source loop.
The storage pass runs again before effect analysis and outlining to add bounds
for recognized traversal domains. It also runs when `GRAPH_DISABLE_PDG=1`
selects sequential execution.

For a recognized ascending array-frontier traversal, the append bound is:

```text
existing output length + append sites per pair × sum(degree(frontier[i]))
```

The sum includes each source occurrence. Duplicate source entries and duplicate
edges count separately. The compiler derives this expression from the append
and traversal structure. Runtime evaluation counts topology; it does not test
conflicts or establish independence.

The runtime reserves capacity before a recognized traversal. A checked append guard remains
in the sequential path. If capacity is still too small, it allocates a larger
buffer, copies the existing entries, and updates the pointer and capacity.
It frees an old heap buffer only when that buffer belongs to this mechanism.
It never frees the original stack allocation. Relocation requires checked fresh
publications or reciprocal swaps. Pointer escapes prevent relocation.

Counts, byte sizes, and index ranges are checked. Allocation failure preserves
the old pointer and capacity. The generated program propagates a failure and
does not replay completed work.

## Spatial transformation

The proof retains symbolic pair occurrences `(u,v,o)`. An accepted array-frontier
contract requires checked append sites, unit counter increments, no intermediate
counter observer, and a complete ascending source-ordinal domain. Opaque calls,
unresolved aliases, and unsupported lifecycle shapes retain conservative results.

Every pair occurrence owns private append slots. The callback writes its payload
and validity into those slots. No shared append counter or destination
deduplication remains in the pair body. After the join, the runtime compacts
valid entries in original source, neighbor, and append order. It retains every
duplicate. A nonempty output prefix is retained.

The compiler computes all spatial results before it rewrites any candidate.
Each result retains its original access sites and contracts. GPU capture tables
created for an earlier loop cannot change a later loop's alias proof.

Only covered append dependencies are discharged. Snapshot contracts remove
covered read conflicts. Live property WAW dependencies remain within their
proved destination group. A source claim stores its result per source ordinal,
so repeated frontier entries retain their once-only guard result. Source
coverage includes zero-neighbor entries.

An array-frontier induction value is a source ordinal. It is not a source
vertex identity. Ordinal accesses do not receive vertex ownership through
that induction value. Callbacks receive the saved ordinal separately, and
the compiler publishes an observed final induction value after the join.

The allocation analysis follows all pointer publications. Distinct fresh
allocation families can establish separation. Reciprocal swaps can establish
separation only within a traversal that contains no swap, after both unique
initial publications. This swap argument does not establish separation between
different rounds. Unknown publications and pointer escapes reject the argument.

## Runtime

CPU array preparation counts and fills stable partition records in two topology
passes. Native CSR capacity reservation uses row lengths. Device group offsets
and reordered records are prepared only when a device submission needs them.
Normal output compaction counts and emits in one pass when reserved capacity
covers every slot; smaller reservations keep the checked growth path.

For CSR, consecutive source-ordinal chunks count and fill in parallel through
the existing worker pool and thread-budget ledger. Per-chunk counts are prefixed
in original chunk order within each partition. Thus every source occurrence,
neighbor occurrence, source hook and append ordinal keeps its original position.
Preparation runs no user callbacks. Allocation and validation finish before
effects begin; unavailable or nested teams use serial preparation. Small
frontiers retain serial preparation. `SGPL_ARRAY_PREP_SERIAL=1` is an ablation
control. Other layouts retain their original preparation path.

CSR counting and filling use separate loops. Counting validates every source,
row bound and destination; row-degree and host-prefix checks enforce the append
ordinal limit computed once per preparation. Filling then writes the validated
records without repeating validation or testing a phase flag. Both loops cache
immutable graph and job metadata locally. The input frontier and topology stay
immutable through the preparation join, and no user effect runs between passes.
This changes preparation implementation only; record formats and proof decisions
are unchanged. Rejection tests cover malformed rows/columns and per-row and
cross-chunk append overflow without allocating huge buffers.

On GNU-compatible ELF builds, the counting/filling callbacks and record driver
occupy the executable `.sgpl_preparation` section instead of ordinary `.text`.
The driver is not inlined into its callers. Preparation code growth therefore
does not shift executor and worker-pool instruction placement in the tested
builds. Other toolchains retain ordinary placement and the same semantics.
This is a code-placement measure, with no change to scheduling or record data.
The SSSP layout controls and final paired verification are recorded in
`../verify/PREPARED_RUNTIME_COMPARISON.md`.

Accepted array spatial stages request partition boundaries without eagerly
constructing full source slices and destination rows. Full-view consumers,
including direct GPU record preparation, materialize those views before use.
An upgrade preserves the boundary allocation already held by array contexts.
Cut construction is serialized and cache identities belong to each graph;
looking up another graph does not evict an active graph's views. Allocation
failure stops generated setup before private-buffer initialization or effects.
`SGPL_ARRAY_FULL_CUT=1` restores eager construction for comparisons.

The normal launcher defaults supporting OpenMP teams to `OMP_WAIT_POLICY=PASSIVE`
and `GOMP_SPINCOUNT=0`, while preserving explicit settings. Direct executable
launches with neither variable set soft-pause the loader's idle OpenMP team on
supported GCC runtimes. SGPL preparation and execution still use the existing
grants, nested-dispatch rules and DOALL/DOACROSS budget equations.

CPU contexts lease reusable record, append-log and snapshot buffers. A buffer
belongs to one live context; overlapping contexts never share a lease. At most
one idle lease of at most 64 MiB is retained per thread, with thread-exit
cleanup. Every used append slot is zeroed during preparation; the first
execution skips the redundant clear, while later executions clear it again.
Snapshot contents are still copied at the original snapshot boundary. Failed
growth retains the previous storage. Device record preparation transfers live
buffers out of the lease before the original GPU reorder/free path takes over.

Single-operation batches containing only pair records select the grouping
action outside the record loop. Per-occurrence source gates, callback order,
append identities and cancellation checks remain in place. Lifecycle-bearing
batches and multiple-operation contexts retain the general dispatch path.

A preparation-thread vertex-to-partition cache compares
the vertex count and complete partition boundary values, including changes
made through the same boundary pointer. Its isolated BFS/CC measurements were
slightly worse, but the complete combination improved BFS's profiled time
compared with omitting the cache. All four execution optimizations are enabled
by default. Compile-time `SGPL_EXEC_*` switches support controlled ablations
without changing proof decisions.

For native weighted CSR, preparation caches the first matching neighbor index
for each column, including unsorted rows and unequal-weight duplicate edges.
Contexts own their copied index hints. The preparation-thread cache follows
topology publication/mutation and layout/pointer/size changes, and is released
at thread exit. CPU callbacks validate the graph and edge before using a hint
and read the weight value live. Other layouts, other queries and a device-
reordered CPU fallback use the original lookup. This cache establishes no
independence fact and does not suppress any pair or append occurrence.

CleanCut batches contain whole accepted proof blocks. CPU callbacks preserve
the order assigned to each batch. Device lanes preserve each dependent group's
order. Both paths use the same occurrence identities and append slots.

Records include source ordinal and pair ordinal. They are immutable layout
inputs. GPU relocation includes those records and the private append log.
Host compaction follows device completion and download. A resized output buffer
replaces its old device registration. Required host writes mark the resident
buffer current before compaction.

The bitmap frontier route retains its set semantics. Array frontiers retain
sequence semantics. This change does not resolve deferred bitmap round proofs
or add temporal independence to a dynamically progressing frontier.

The current storage pass supports dynamic array pointer slots with function
lifetime represented by globals. Local pointer frames and unsupported append
shapes do not receive this spatial realization. The compiler reports runtime
capability separately from the spatial proof.

## Validation

`test/run_array_frontier_exec_tests.sh` checks repeated source entries, duplicate
edges, original append order, source hooks, zero-neighbor entries, all supported
layouts, growth with copying, prefix bounds, and allocation failure at 1, 2, 4,
and 8 workers.

`test/run_array_preparation_exec_tests.sh` checks parallel chunk order and
weighted hints, source hooks, nested and parent budgets, balanced reservations,
cold-team failure, invalid-input rejection, lazy full-view upgrades and
independent-graph cache reuse at 1, 2, 4 and 8 workers. The comparison with
`effect_interim` and Graptor is recorded in `../verify/PREPARED_RUNTIME_COMPARISON.md`.

`test/run_array_frontier_lowering_tests.sh` checks automatic compiler acceptance,
two append sites, duplicates, exact intermediate frontier order, normal bounds,
forced fallback growth, real device execution, and completely unrolled source
loops. Unrolled loops retain storage guards even when they have no supported
spatial realization.

Compiler acceptance and full algorithm comparisons are recorded under
`D:/sgpl/graptor-comparison-20261012/frontier-storage-proof-20261012`.
The original algorithm sources and theorem PDF are unchanged.

The final WSL run passed all 20 regression suites and all 495 full-vector checks
across five algorithms, 11 graphs, and 1, 2, 4, and 8 workers. Both formerly
failing CC datasets passed. Normal reservation required no fallback growth.
The forced-growth fixtures passed on sequential, CPU, and real GPU paths.

The matrix recorded 176 runs with device kernels. SSSP's 44 GPU-requested runs
used CPU execution because `graph_get_edge_weight` has no device implementation.
The four main frontier proofs use production analysis, with no status override.
The CSV files record observed execution separately from requested execution.
