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
