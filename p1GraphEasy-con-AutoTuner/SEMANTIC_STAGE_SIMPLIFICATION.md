# Proof-gated semantic stage simplification

This extends R1–R9 on the experimental branch. It adds no cost model, trip
threshold, sampling, calibration, or adaptive performance selection. All new
guards concern semantics, representation, explicit disablement, or allocation
failure. Performance is unmeasured until the user authorizes benchmarks.

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
the source, then invokes the original arithmetic directly in source/edge order.
There are no per-edge input records, partition count/fill passes, or append
journal. This realization is serial; it makes no parallel speedup claim.

**R11: ordered destination state promotion.** The runtime additionally proves
that the frontier is exactly the unique ordered full domain 0..n-1. A CSR
transpose built in source/edge order therefore retains each destination's
original arithmetic sequence. A generated row function promotes the private
destination state to SSA, initializes only a nonempty row, and flushes once
only if an original store executed. Exact edge keys restore the original
global notification order after destination traversal.

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
