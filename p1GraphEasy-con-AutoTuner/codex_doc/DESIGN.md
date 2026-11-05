# Design audit: layout-native execution

## Baseline responsibilities

1. spatial_partition_proof.inc chooses an accepted symbolic proof group after
   checked transformations. It must remain the authority for independence.
2. graph_frontier_lowering.cpp emits construction of complete CleanCut or array
   owner ranges, private state, snapshots, callbacks, and lifecycle envelopes.
3. autotuner_runtime.c constructs source-pair and destination-row copies of the
   native layout. GPU preparation also consumes those copies.
4. array_frontier_runtime.inc / array_preparation_runtime.inc separately create
   per-occurrence records, ordinals and append slots for dynamic array frontiers.
5. fullgraph_runtime.inc already bypasses record execution for some CSR stages
   using a stable incoming index and generated kernels. Do not mistake that
   specialization for a generic source-owned traversal.
6. static_schedule_runtime.inc preserves source preparation -> owner tasks ->
   join. It is independent of how owner tasks enumerate their actual pairs.

## Constraints for the replacement

- Use the selected native layout for traversal; never silently reset it to CSR.
- Preserve every duplicate and source occurrence, source/neighbor order within
  dependent groups, isolated source hooks, and array append order.
- A source witness permits direct source-range iteration. A destination witness
  requires destination-owned enumeration or filtering; an unproved change of
  witness is forbidden even if privatization is theoretically possible.
- If destination tasks filter outgoing rows, acknowledge the repeated scans in
  the cost model and benchmarks. Do not label such filtering a new proof.
- Complete lifecycle/storage before dispatch; joins precede folds/publication.
- Do not add record-level DAG nodes/edges.
- Keep ABI compatibility explicit. Compiler and runtime layouts must agree.
- New-path tests must prove the path ran and no full CleanCut payload was built;
  passing old-path tests is insufficient.
- Race detector results support the proof audit but cannot prove all executions.

## Final native07 implementation (CPU first)

CPU frontend batch builders use autograph_build_layout_batches. It retains only
P+1 source range boundaries in existing metadata; no source-pair or push-row edge
arrays are allocated. Existing autotuner cost and feasibility models are retained.
The runtime binds the selected layout, graph identity, vertex count, layout epoch,
topology generation and partition count into a private SgplLayoutExecution.

Source owners traverse their own source rows using CSR pointers or the native
neighbor iterator for PCSR, BCSR and SET. Destination owners scan native rows and
filter destinations to their exclusive range. This preserves order but scans
E edges P times; it is a deliberate performance limitation, not a new proof or
new algorithm. Existing certified CSR incoming-index specialization still applies.

Ordered-array generic execution copies source occurrences and stores degree
prefixes; append slots retain original occurrence/site order. It creates no
pair records. Existing certified relax simplifications may instead retain their
existing native CSR worklists. The GPU backend retains its original preparation,
CleanCut and device grouping paths, as requested. There is no record-level DAG.

Legacy single/dual-owner composition receives a representation-only bind. Its
existing accepted schedule remains authoritative; it gets no fabricated spatial
certificate. Native contexts bypass the old CleanCut-dependent legacy GPU hook;
GPU compilation keeps its original contexts and dedicated invocation/fallback.
Old public CleanCut construction/execution APIs remain for GPU and compatibility.
New CPU compiler lowering emits none of those edge-materialization calls.

The autotuner places native execution's layout conversion before batch/context
preparation and its profile end after execution. Native staged and combined
round/spatial schedules are also layout region boundaries. Forced-layout checks
verify actual conversion/activation for all four layouts.

## Race and ordering obligations

- The accepted source/destination/occurrence proof and runtime owner are unchanged.
  A layout choice never changes the proof witness or introduces a transformation.
- All conflicting writes to a given source/destination remain in one owner task,
  in the original source/neighbor order. Destination ranges retain the baseline's
  ceiling boundaries; source and array ranges retain its floor boundaries.
- Source occurrences repeated in an array belong to the same source owner. Each
  pair occurrence retains a unique work ordinal; append site slots do not overlap.
- Snapshots, membership copies, source gates and native topology are read-only
  during dispatch. The existing coverage DAG/joins order source lifecycle.
- Private partials/frontier lanes retain one writer per partition. Shared
  cancellation and occurrence counters use existing atomic access. Context copies
  use the existing atomic-aware helper, not a racy aggregate read.
- Finish/join precedes compaction, folds and output publication. No fallback
  replays partially executed user work. Stale native bindings fail before dispatch;
  graph/layout mutation during an invocation remains outside the accepted contract.
- Tests exercise these obligations, including sanitizer and overlap probes;
  finite test success is not a universal race-freedom proof.

## Validation result

Completed the core regression/correctness campaign, verified GPU compatibility and
fallback, and ran matched CPU end-to-end timings. RESULTS.md documents mixed performance,
29/31 passing suites, baseline failures and finite-test limitations.
