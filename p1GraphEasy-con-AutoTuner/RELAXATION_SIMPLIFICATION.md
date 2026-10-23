# Proof-gated minimum-relaxation work elimination

The experimental branch retains the R1--R5 compiler and runtime paths. R6--R9
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
  append. Counting and filling skip the later row. Duplicate edges in the first
  row remain ordered and intact.
* **R7 — omit relaxations failing the round-start bound.** If the candidate is
  greater than or equal to the destination before traversal, it cannot succeed
  after other minimum updates. Fuse this check into the existing count/fill
  passes. Reads use exact signed comparisons and modular candidate arithmetic;
  no nonnegative-weight or no-overflow assumption is introduced. Removed
  destinations retain their final occupied-group accounting through private
  chunk bitsets, merged before owner execution. There is no shared per-edge or
  per-destination atomic counter during preparation.
* **R8 — reduce a destination to its minimum and one activation.** Additionally
  prove the complete circulating frontier consumer observes only traversal,
  buffer swapping and emptiness. Counts, order, multiplicity and pointer escapes
  refuse. A bounded first implementation chooses a minimum contribution in
  serial preparation and emits one representative record per changed
  destination. The original callback performs the final update. Each round's
  final array, changed-vertex set and termination condition are preserved.
  The consumer proof currently requires a directly initialized, nonescaping
  initial buffer. Outlined/opaque initialization is refused. The existing
  campaign's CC/SSSP programs therefore use R6/R7 and eligible R9, while R8 is covered by
  positive restricted-consumer fixtures. Extending this proof is separate work.
* **R9 — omit later destination contributions under ordered candidates.** For
  an unweighted copy-minimum callback, check that the input source values are
  nondecreasing in signed order. The first improving contribution to each
  destination is then its minimum; all later contributions are no-ops. Chunk
  bitsets identify destinations with an improving candidate. An ordered prefix
  removes destinations already covered by an earlier chunk, and filling emits
  the first improving edge in the remaining chunk. This retains the exact
  successful notification sequence and needs no whole-frontier consumer proof.
  An unknown order refuses R9 while independently justified rules may continue.
  Its execution realization additionally verifies the actual snapshot binding
  (slot, base, extent, element size and successful capture). A selected record
  must win and is the only record for its destination, so a dedicated copy and
  notification loop removes the target load, minimum comparison and callback
  dispatch. The complete context proof makes the visited bitmap unobservable;
  wordwise mask unions provide the same occupied-group count without bitmap
  materialization. Reuse and invalidation restore ordinary tracking before
  rebuilding baseline records. Unknown snapshot bindings select baseline before
  effects. Original records and callbacks remain usable by the retained path.

R6, R7 and R9 retain the exact sequence of successful frontier notifications. R8
intentionally changes that sequence only under its additional consumer proof.
The original source frontier and output reservation domain are retained.
Removed records have no notifications, and the complete callback proof excludes
user observation of work ordinals. Surviving records therefore receive compact
keys in the same global source/edge order. This removes the unused append-log
cells and journal bitmap domain as well as the records. Journals retain their
existing protocol with fewer slots. No partial work is replayed.

## Binding, fallback and controls

A version-2 private descriptor records rule bits, arithmetic recipe, baseline
callback identity and ABI sizes. Binding also takes the actual source-snapshot
extent; it must cover the graph vertex domain before preparation reads values.
An unavailable extent refuses the rule. Legacy version-1 bindings keep baseline
execution. Runtime guards
require the matching CPU CSR pointers, destination ownership, a single pair
operation, one append site, no lifecycle hooks, and disjoint output/accumulator
ranges. Accumulator storage must also be disjoint from graph headers, CSR rows,
columns and weights; a frozen source does not make an aliased weight immutable.
Optional proof-storage allocation failure selects existing preparation
before effects. Storage is private to an exclusive lease and retains bounded
capacity. Topology/layout invalidation, runtime disablement after preparation,
device materialization and context reuse rebuild the baseline before effects.

* Compilation: `SGPL_EXEC_SIMPLIFY_RULES=all|none|R1,...,R9`.
* All execution simplifications off: `SGPL_NO_EXEC_SIMPLIFY=1`.
* R6--R9 off while retaining R1--R5: `SGPL_NO_RELAX_SIMPLIFY=1`.
* Compiler activation/refusal diagnostics: `SGPL_EXEC_SIMPLIFY_STATS=1`.
* Validation-only physical record counts: `SGPL_RELAX_SIMPLIFY_STATS=1`.
* A fixed 4096-edge profitability floor retains ordinary execution on tiny
  traversals. `SGPL_RELAX_MIN_WORK=0` exercises all supported shapes in focused
  correctness tests. This control does not weaken any proof premise.

GPU execution, schedule selection, calibration, ARS policy and motif semantics
retain their existing implementations. Work-count diagnostics are disabled in
benchmarks. Historical Graptor observations are reused; Graptor is not rerun.

## Correctness evidence

`tools/validate_relax_simplification.py` freezes builds externally, runs all
sixteen new rule subsets with compiler refusal controls, checks physical record
counts, and compares native outputs at 1/2/6 physical workers. Fixtures cover
duplicates, self-loops, isolated vertices, empty graphs/partitions, signed wrap,
negative weights, first-match duplicate weights, descriptor refusal, optional
allocation failures, context reuse and topology invalidation. ASan/UBSan and
targeted TSan results are recorded separately. Escaping/error-observing contexts
are compiler negatives; existing cancellation contracts continue exercising the
retained executor. The frozen 61-graph/four-algorithm corpus compares baseline,
enabled and disabled executions, with independent validators and exact PageRank
bits. Machine-readable correctness and benchmark reports are separate artifacts.

## Measured outcome of the realized executor

The six-physical-core confirmation campaign compared frozen source
`a162e9f64002171fad27481c729d47446e28cade` against the previous R1--R5 build,
on 61 graphs per algorithm with 20 repetitions. Both primary and confirmation
A/A controls passed. SSSP achieved a 1.3978x end-to-end geometric-mean speedup
and 17.01% less summed time. CC achieved only 1.0300x geometrically and used
62.35% more summed time. These rules are therefore not a successful general
CC optimization. They remain experimental; ARS-1 was not changed or merged.
Saved Graptor observations were reused without new Graptor launches.

The realized correctness report records 2,196 independent corpus checks,
732 exact array comparison groups, 183 exact PageRank bit groups, 400 compiler
checks and 88,290 focused runtime comparisons. ASan/UBSan passed. TSan passed
at one and two workers after startup retries; six-worker TSan remained
unresolved after two shadow-memory startup failures and one empty crash.
That detector limitation is not counted as a passing six-worker check.

Reports: `reports/relax_execution_simplification_realized_correctness.json`
and `reports/relax_execution_simplification_realized_benchmark.json`.
The complete local benchmark is retained in
`C:\p1-relax-realized-simplification-benchmark-20261022\final_report.html`.
