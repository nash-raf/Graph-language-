# Proof-gated semantic stage simplification

This extends R1–R9 on the experimental branch. It adds no cost model, trip
threshold, sampling, calibration, or adaptive performance selection. All new
guards concern semantics, representation, explicit disablement, or allocation
failure. The semantic08 benchmark is retained separately; the subsequent R10/R11 repairs have correctness results only until new benchmarks are requested.

The original callbacks, lowered loop, and existing fallback remain present.
An additive private descriptor and one pre-effect runtime attempt select an
alternative. Refusal enters the original loop body before any user effect.
Completed work is never replayed. Public context and operation layouts stay
unchanged; GPU lowering is unchanged.

## Independent certificates

**R10: direct ordered callback execution.** A bounded, acyclic callback has
one destination-local store, one bounded notification of that destination,
one source snapshot of the same fresh array, and no other observable effects.
Exact address/use checks supplement destination ownership. Unknown calls,
aliasing, atomics, volatile accesses, strict/constrained floating point,
unsupported potentially trapping arithmetic, gates, lifecycle hooks, escaping
contexts, output prefixes and unresolved capture windows refuse.

The first supported element types are i32 and double. The operation need not
be minimum or a reduction: ordered subtraction and conditionals are supported.
Trusted first-match edge-weight access is supported without changing modular
integer arithmetic. Source occurrence order, duplicate edges, per-destination
contribution order and successful notification order are retained.

The runtime validates native CPU CSR, array extent, all endpoint bounds and
disjoint storage, reserves the original worst-case output capacity, freezes
the source, then executes destination-owned partitions on the existing worker pool. Stable chunk prefixes retain original per-destination contribution order; separate notification slots restore global source/edge order. R10 uses eight-byte edge/occurrence keys rather than redundant source/destination/weight records. It retains count/fill preparation needed for ownership, and removes the general append journal. No associativity is assumed.

Native topology validation and source indices are cached per preparation thread and invalidated by the existing mutation generation, layout epoch, pointer and size identity. First-match weight indices reuse the existing topology cache. Every callback receives the first matching edge index, including duplicate edges with different weights; weight values remain live. No full topology scan is repeated for an unchanged graph on each frontier round.

**R11: ordered destination state promotion.** The runtime additionally proves
that the frontier is exactly the unique ordered full domain 0..n-1. A CSR
transpose built in source/edge order therefore retains each destination's
original arithmetic sequence. A generated row function promotes the private
destination state to SSA, initializes only a nonempty row, and flushes once
only if an original store executed. Exact edge keys restore the original
global notification order after destination traversal. Different destinations execute on the existing worker pool; their private arithmetic sequences remain unchanged.

Associativity, commutativity, reassociation and idempotence are not assumed.
Nonassociative integer subtraction and cancellation-sensitive floating-point
examples are positive controls. Permuted or repeated source streams refuse
R11; independently licensed R10 or the original executor remains available.

**R12: whole-loop fixed-point execution.** A separate complete recurrence proof
requires that only final array values and termination are observed. Frontier
buffers, capacities, counts and intermediate values cannot escape. Initialization
through private compiler task wrappers is inspected for pointer publication;
unknown effects and observable loop state refuse. The actual baseline operation
identity and existing C3 journal-clone identity are checked separately.

The first operation-specific licenses are strict signed i32 copy-minimum,
copy-maximum, bitwise OR and bitwise AND with notification exactly on change.
Each is a monotone finite-height lattice transfer, so fair propagation has the
same closure as the original snapshot rounds. Arbitrary starting labels,
directed graphs, initial active subsets, duplicates and self-loops are supported.
Signed extrema use a label-setting priority worklist; identity transfer proves
that a removed extremum cannot improve later. Bitwise joins use a bounded fair
pending-set queue. Live reads and writes execute in one owner thread, avoiding
unsynchronized asynchronous access. There are no round snapshots, per-round
records, or per-round global barriers in this realization.

Foldability alone never earns R12. Weighted modular SSSP, floating-point
arithmetic, unsigned/unsupported comparisons, other transfer shapes and observed
round/frontier counts refuse this license. Weighted SSSP can still use R10/R11
when their independent premises hold. No nonnegative-weight or no-overflow fact
is asserted.

## Controls and correctness evidence

Compilation: `SGPL_EXEC_SIMPLIFY_RULES=all|none|R1,...,R12`. All rules are
independently selectable. `SGPL_NO_EXEC_SIMPLIFY=1` retains all original kernels;
`SGPL_NO_SEMANTIC_STAGE=1` disables only R10–R12 and retains eligible R1–R9.
Compiler diagnostics use `SGPL_EXEC_SIMPLIFY_STATS=1`. Validation-only activation
diagnostics use `SGPL_SEMANTIC_STAGE_STATS=1`; they collect no timing samples.

`tools/validate_semantic_stages.py` runs structural/refusal fixtures, generated
programs with exact floating-point bits, native allocation/bounds/alias controls,
the frozen 61-graph/four-algorithm corpus at 1/2/6 physical workers, existing
contracts and instrumented race checks. New validation sources contain no timer
or warmup blocks. Reports record build/source identities and detector startup
limitations separately. Accepted binaries and historical reports are preserved.

The frozen `semantic08` build from native source commit `0acf2bd5` passes:

- 2,196 independent corpus checks, 732 exact array groups and 183 PageRank bit groups.
- 361 compiler checks across all eight new-rule subsets.
- 3,876 native cases under normal execution and ASan/UBSan.
- 405 generated-program checks, including exact floating bits and nonfoldable operations.
- Existing full-graph, frontier, relaxation, context and worker-budget contracts.
- 30 targeted LLVM/runtime TSan checks, including actual CC and generated floating-point kernels.

The activation checks confirm one whole-loop R12 execution per CC graph and
R10 execution for SSSP on all 61 graphs each. Weighted SSSP refuses R12. The
worklist currently uses a single owner; parallel worklist execution is not claimed.

TSan's successful checks used serial auxiliary OpenMP work to isolate the known
system libgomp barrier-visibility issue while retaining SGPL worker budgets of
1/2/6. The report retains 54 shadow-mapping startup failures and five empty launch
crashes separately from the 30 passing checks. An injected LLVM race was detected
by the positive control. This is evidence for the tested paths, not a proof that
all programs are race-free.

Machine-readable evidence is in
`reports/semantic_stage_simplification_correctness.json`. Those correctness runs contained no benchmarks. The later semantic08 performance campaign is recorded in `reports/semantic_stage_simplification_benchmark.json`; its SSSP regression motivated the repair below. ARS-1 remains unchanged.

## R10/R11 repair after semantic08 measurements

The frozen semantic08 benchmark showed an R10 SSSP regression and an R12 CC improvement. Its sources, binaries and reports remain unchanged. The repair removes repeated adjacency searches for weights, repeated validation of unchanged topology, and forced serial destination execution. It adds no cost model. It does not claim a measured speedup before a new benchmark.

The internal descriptor now uses version 2 with the same private layout and additive entry point. A previous runtime rejects the new descriptor before effects and enters the retained baseline. This prevents a new generated weighted row from assuming a first-match index table supplied only by the repaired runtime. Public execution context layouts and APIs remain unchanged.

`tools/validate_semantic_repair.py` compares the repair against semantic08 and disabled execution, without timer or warmup blocks. Native controls count weight-access fallback calls (required zero), check multi-worker execution, preserve unequal duplicate-weight semantics, and exercise cache invalidation. The original R12 worklist body is retained.

The frozen repair `semantic_repair01` (native source `7beb336d`) passes 2,196 independent corpus checks, 732 exact array groups, 183 exact PageRank bit groups, 361 IR checks, 4,260 native normal/ASan/UBSan cases, 405 generic generated-program cases, 12 weighted generated-row cases, and the existing contracts. Activation checks verify one topology validation per SSSP graph. All 36 targeted TSan checks passed; 54 mapping startup failures and five empty launch crashes are retained separately. TSan used the auxiliary-OpenMP limitation described above while retaining actual SGPL worker budgets of 1/2/6.

Machine-readable repair evidence is in `reports/semantic_stage_repair_correctness.json`. R10 still requires source snapshots, stable ownership preparation and notification compaction, and does not yet compose all R6�R9 pruning rules. These are remaining costs; correctness results do not establish a performance recovery. No benchmarks were run for the repair.
