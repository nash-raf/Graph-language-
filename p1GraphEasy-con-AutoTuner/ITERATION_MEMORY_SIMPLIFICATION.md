# Certified iteration memory simplification

This branch starts from accepted ARS-1 `82509de4`. It retains R1--R12 and
their fallback paths. It adds no cost model, work threshold, sampling,
calibration, graph-name test, or algorithm-name test. GPU lowering is unchanged.

## Seeded ordered iteration

Before LLVM vectorizes initialization, a compiler pass recognizes a bounded
iteration containing exactly a scalar full-vertex seed sweep, one existing
certified full-domain traversal, and a full-vertex destination-to-source copy.
Both sweep indices must start at zero, advance by one and test the traversed
graph's vertex extent. The traversal must lie between the two sweeps. Extra
conditional phases, user calls, other writes, escaping iteration values and
unmodeled counter updates refuse the transformation.

The callback must have the existing exact double-add destination accumulator
and disjoint source/read allocation proofs. Strict floating-point functions,
unsupported callbacks and destination-observing source gates refuse. Array
pointers may not escape, participate in observable pointer comparisons, or
have atomic/volatile observers. Scalars read by the callback and gate must
remain stable in the complete iteration window. The initial counter is zero
and its positive constant limit and single increment are checked explicitly.

An internal versioned descriptor retains the original pair identity and the
source/destination slot identities. Late lowering generates a seeded incoming
kernel and a source-map range function from the existing fullgraph generator.
The initial accumulator value comes directly from the original seed; every
destination, including empty and entirely gated destinations, receives one
final store. Original additions, operand order, source/duplicate contribution
order and floating-point flags remain unchanged. No reassociation or vector
reduction is introduced.

The runtime acquires and validates the ordinary incoming index and reserves all
scratch before effects. Independent gate/map source ranges use the existing
worker pool and worker budget. After their join, disjoint destination ranges
execute the seeded kernel. Backing pointers rotate only after that join.
After the final round, both original array identities are restored and both
arrays have the contents the original copy would have produced. This needs
at most one final full-array copy, instead of one copy per round. Allocation,
layout, descriptor or compilation refusal retains the original entire loop.
There is no fallback after the first alternative destination effect.

The private descriptor's slot uses are recognized by the allocation checker:
exchanging these two private buffers within the certified phases preserves
mutual disjointness, and their identities are restored before other code runs.
The descriptor is not a general permission for unknown pointer publication.

## Minimum rounds with set-only frontier observation

R8 now consumes the stronger private circulating-frontier/initialization proof
already used by the semantic-stage pass. It is independent of R12's closure
license. The exact signed i32 minimum recipe, modular optional weight addition,
matching source snapshot, ownership and complete consumer proof are required.
The circulating buffers may be swapped and tested for emptiness; their order,
multiplicity, intermediate counts, addresses and intermediate array values may
not escape. Unknown initialization, opaque calls, extra stores, floating-point
updates, nonfoldable operations and observed frontier state refuse R8.

The realization freezes values only for active source occurrences. Repeated
occurrences have the same frozen value. Count/fill preparation retains only
round-start-improving candidates, with the original signed comparison and
uint32 modular addition. First-match duplicate-edge weight semantics remain
intact. Each destination owner folds its candidates to the signed minimum,
writes that minimum once and emits its destination once. The changed set and
each round's final array are identical. Induction over these sets preserves
the original snapshot rounds and termination; no nonnegative-weight,
no-overflow or weighted whole-loop convergence claim is made.

This removes the edge-indexed notification log, restoration of notification
order, full-array snapshot copy and repeated notification writes. Destination
ownership count/fill preparation and candidate keys remain. An exclusive
thread-local workspace retains capacity for captures, keys, accumulators, epoch
tags and changed destinations. Its lease refuses nesting; fresh epochs prevent
stale tracking on reuse. Reallocation and output reservation finish before
destination effects. Topology and weight identities continue using the existing
validated caches and mutation invalidation. The private semantic descriptor
version is 4; older/incompatible descriptors refuse before effects. Public
execution-context layouts and binding APIs are unchanged.

R12's accepted fixed-point implementation is byte-identical and takes priority
when its separate stronger license applies. Generic R10/R11 operations retain
their original ordered execution and snapshot behavior.

## Controls and validation

- `SGPL_NO_ITERATION_MEMORY=1` disables both new realizations at runtime while
  retaining accepted R1--R12 execution.
- `SGPL_NO_ITERATION_MEMORY_LOWERING=1` refuses the seeded-iteration compiler
  transformation. Compilation with `none` disables all execution simplification;
  the iteration bundle currently requires R1, R2 and R3 to be enabled.
- R8 is independently selectable with `SGPL_EXEC_SIMPLIFY_RULES`; it also needs
  a licensed R10/R11 traversal. Absence of the complete consumer proof refuses
  it even if the callback is a minimum.
- `SGPL_NO_EXEC_SIMPLIFY=1` retains all unsimplified execution. Source and runtime
  activation diagnostics remain behind `SGPL_EXEC_SIMPLIFY_STATS` and
  `SGPL_SEMANTIC_STAGE_STATS`; benchmarks do not enable work-count diagnostics.

`tools/validate_iteration_memory.py` builds frozen external versions and runs
correctness-only corpus comparisons at 1/2/6 physical workers, exact PageRank
bits, structural/refusal fixtures, native normal/ASan/UBSan contracts, generic
ordered programs and instrumented race checks. Detector startup failures are
recorded separately. `tools/benchmark_iteration_memory.py` is a separate entry
point, freezes timed inputs/binaries and compares accepted, enabled, disabled
and identical-binary controls. It reads historical double-PageRank Graptor
observations without launching Graptor. Measurements and final outcomes belong
to the separate machine-readable reports; this document asserts no speedup.
