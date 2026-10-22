# Proof-gated minimum-relaxation work elimination

The experimental branch retains the R1--R5 compiler and runtime paths. R6--R8
operate on certified CPU array-frontier stages, without algorithm-name tests.
No public execution-context or operation layout changes.

The common proof checks the entire acyclic callback: one signed i32 minimum
guard, one destination-indexed store and one conditional notification of that
destination. Its candidate is a frozen source snapshot, optionally plus the
compiler's checked, first-match edge weight with modular i32 addition. The
snapshot must originate from the same live array that the destination updates.
Fresh language allocation facts, ownership, explicit address checks, accessor
provenance and a complete private-context use check are all required. Unknown
calls, stores, aliasing, gates, lifecycle hooks, atomics, volatile accesses,
division, alternate update shapes and externally published contexts refuse.

The compiler proves the full window from context creation through preparation,
source capture and execution has no intervening user writes or opaque calls.
Consequently preparation can inspect the values that the source capture will
freeze. The context has one bounded callback and no external error publisher;
valid ordinals and one append site cannot introduce a callback error. This is
why removing polls for omitted work is permitted. A callback-only no-op proof
does not establish this property for an escaping/cancellable context.

## Rules

* **R6 — omit repeated source traversals.** Preserve the first occurrence of a
  source in each round. A later traversal has exactly the same candidate on
  every edge; destination values have only decreased. It can neither store nor
  append. Counting and filling skip the later row, but advance its full logical
  ordinal domain. Duplicate edges in the first row remain ordered and intact.
* **R7 — omit relaxations failing the round-start bound.** If the candidate is
  greater than or equal to the destination before traversal, it cannot succeed
  after other minimum updates. Fuse this check into the existing count/fill
  passes. Reads use exact signed comparisons and modular candidate arithmetic;
  no nonnegative-weight or no-overflow assumption is introduced. Removed
  destinations retain their final occupied-group accounting through private
  preparation witnesses, published before owner execution.
* **R8 — reduce a destination to its minimum and one activation.** Additionally
  prove the complete circulating frontier consumer observes only traversal,
  buffer swapping and emptiness. Counts, order, multiplicity and pointer escapes
  refuse. A bounded first implementation chooses a minimum contribution in
  serial preparation and emits one representative record per changed
  destination. The original callback performs the final update. Each round's
  final array, changed-vertex set and termination condition are preserved.
  The consumer proof currently requires a directly initialized, nonescaping
  initial buffer. Outlined/opaque initialization is refused. The existing
  campaign's CC/SSSP programs therefore use R6/R7, while R8 is covered by
  positive restricted-consumer fixtures. Extending this proof is separate work.

R6 and R7 retain the exact sequence of successful frontier notifications. R8
intentionally changes that sequence only under its additional consumer proof.
The original source frontier, logical work count and append reservation domain
are retained. Journals retain original keys but allocate owner capacities from
the smaller physical record counts. No partial work is replayed.

## Binding, fallback and controls

A version-1 private descriptor records rule bits, arithmetic recipe, baseline
callback identity and ABI sizes. Binding validates those fields. Runtime guards
require the matching CPU CSR pointers, destination ownership, a single pair
operation, one append site, no lifecycle hooks, and disjoint output/accumulator
ranges. Optional proof-storage allocation failure selects existing preparation
before effects. Storage is private to an exclusive lease and retains bounded
capacity. Topology/layout invalidation, runtime disablement after preparation,
device materialization and context reuse rebuild the baseline before effects.

* Compilation: `SGPL_EXEC_SIMPLIFY_RULES=all|none|R1,...,R8`.
* All execution simplifications off: `SGPL_NO_EXEC_SIMPLIFY=1`.
* R6--R8 off while retaining R1--R5: `SGPL_NO_RELAX_SIMPLIFY=1`.
* Compiler activation/refusal diagnostics: `SGPL_EXEC_SIMPLIFY_STATS=1`.
* Validation-only physical record counts: `SGPL_RELAX_SIMPLIFY_STATS=1`.

GPU execution, schedule selection, calibration, ARS policy and motif semantics
retain their existing implementations. Work-count diagnostics are disabled in
benchmarks. Historical Graptor observations are reused; Graptor is not rerun.

## Correctness evidence

`tools/validate_relax_simplification.py` freezes builds externally, runs all
eight new rule subsets with compiler refusal controls, checks physical record
counts, and compares native outputs at 1/2/6 physical workers. Fixtures cover
duplicates, self-loops, isolated vertices, empty graphs/partitions, signed wrap,
negative weights, first-match duplicate weights, descriptor refusal, optional
allocation failures, context reuse and topology invalidation. ASan/UBSan and
targeted TSan results are recorded separately. Escaping/error-observing contexts
are compiler negatives; existing cancellation contracts continue exercising the
retained executor. The frozen 61-graph/four-algorithm corpus compares baseline,
enabled and disabled executions, with independent validators and exact PageRank
bits. Machine-readable correctness and benchmark reports are separate artifacts.
