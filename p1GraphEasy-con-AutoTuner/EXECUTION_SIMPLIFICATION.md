# Proof-gated execution simplification

The reference is ARS-1 commit `2b46cb5ded54132e98dc052bd2caf548d1acfcb4` (accepted C3).
This experimental branch specializes certified CPU full-domain destination-owner
traversals and ordered frontier record executors. It does not change frontier multiplicity, propagation algorithms,
layout decisions, scheduling budgets, or GPU execution.

## Contract and fallback

The existing full-graph generator emits the original kernel and map and identifies
the transparent callback with a private certificate. The late simplifier consumes
that certificate after the existing lowering passes. Its rules operate on clones;
the original functions remain in the module. The new descriptor records baseline
function identity, replacement functions, rule bits, size, and version. The runtime
attaches it only to a matching successfully bound full-graph state. Descriptor
refusal changes no state. Topology/layout invalidation releases both selections.
Path selection precedes execution; there is no replay after partial effects.
Existing full-graph runtime prerequisites and fallback remain in force.

## Rules

| Rule | Premises | Removed machinery | Preserved observations |
|---|---|---|---|
| R1 | Context pointer fields are immutable until join; the certified callback cannot access them | Per-edge invariant null tests, using at most eight entry-selected paths; map has two paths | Gate element reads and acquire error polls at the original edge boundaries |
| R2 | Destination ownership; callback has no access to private tracking storage | Repeated destination-visited stores in the steady edge loop | One mark immediately before the first executed pair; no mark for empty, fully gated, or already-cancelled work |
| R3 | One destination-indexed ordinary double-add or modular integer-add update; no escaping old value; disjoint contributions; no unsupported effects | Steady-loop accumulator loads/stores | Original operand/contribution order, lazy initial load, final store, and partial-result flush on error |
| R4 | Existing frontend destination ownership/lifecycle witness; dead source ordinal; complete transparent context/effect chain; bounded, acyclic ordinary pair body | Per-record indirect pair dispatch and unobserved weight-index/context updates; known record-format loads | The original pair body, every occurrence, group tracking, append sequence, record order and acquire error boundary |
| R5 | R4; exact source-indexed scalar read from a private round snapshot; all snapshot pointer chains read-only and nonescaping | Repeated scalar snapshot loads for the last source evaluated at that load site | First read stays at its original dynamic evaluation point; no read for empty, skipped or cancelled work; exact scalar bits |

R2/R3 peel the initial search for an executable edge from the steady loop. They
add no per-edge first-occurrence flag. R3 also checks the original callback so
source-map substitution cannot hide an accumulator observation. Array disjointness
uses complete allocation-site sets for compiler-owned array slots, not variable
names. Unresolved pointers, additional stores, observed old values, unsupported
updates, integer division/remainder, and fast-math updates refuse R3. Opaque,
context-observing, TLS, atomic, or volatile callbacks refuse simplification.

R2/R3 run before R1. A refused rule does not prevent other justified rules.
Cleanup and LLVM verification apply only to generated replacement functions.
Proof metadata is an auditable record of checked premises, not an independently
machine-checked proof of the compiler implementation.

R4 uses a separate versioned private descriptor and additive binding entry point.
It retains both the original pair callback and C3's journal callback. Both have
generated executor variants; selection checks the currently installed callback
identity before any partition effects. Tracking mode is selected outside the
loop (none, dense, sparse, epoch-tagged). Interrupted owners retain the reference
executor's behavior of not publishing their local occupied-group counter.

The binding belongs to the context's exclusive scratch lease and is cleared when
the lease is released. Unsupported record formats, group modes, callback identity
changes, unavailable scratch or descriptor mismatch retain the existing executor.
No topology-dependent pointers are cached: every invocation uses its freshly
prepared records and current context. Existing topology/layout invalidation and
record rebuilding remain in charge. Public context and operation layouts do not
change. The private lease grows by one pointer.

R5 has independent caches for each supported load site, promoted to SSA state.
It supports ordinary scalar integer/float/double snapshot loads. It does not cache
live destination values, general source-array reads, arithmetic expressions, or
source gates. Mutable/escaping snapshot pointers refuse R5 even when R4 applies.
It introduces no reassociation, reduction, frontier deduplication or early exit.
Frontier record counting/construction and append compaction are retained.

## Controls

- Compile with `SGPL_EXEC_SIMPLIFY_RULES=all` (default), `none`, or a comma-separated
  subset of `R1,R2,R3,R4,R5`. Unknown rule IDs are errors. R5 requires R4;
  `R5` alone refuses frontier specialization. `R4` isolates executor specialization
  without snapshot caching. Existing eight R1/R2/R3 subsets remain independent.
- Set `SGPL_NO_EXEC_SIMPLIFY=1` at execution to use the retained baseline kernels
  and maps, including the retained frontier executor. Existing full-graph/C3 controls still apply.
- Set `SGPL_EXEC_SIMPLIFY_STATS=1` at compilation for activation/refusal messages.

No new public execution-context fields or existing ABI changes are required.
Compiler and runtime must both support the additive simplification binder.

## Correctness-only validation

Use the existing WSL investigation venv to run
`tools/validate_execution_simplification.py build`, then `contracts`, `programs`,
`detectors`, and `report`,
with the same `--version` value. Artifacts are written under
`/home/kazi/p1_execution_simplification/versions/<version>/`.
Frozen native versions cannot be overwritten; choose a new version after changes.

The focused JIT suite exercises all eight rule subsets and inspects loop IR. It
checks exact double/integer results, duplicate occurrence traces, tracking bytes,
pointer combinations, gates, and deterministic cancellation before/after effects.
Negative controls cover unsupported bodies and missing certificates. Native
contracts check descriptor refusal, runtime disablement, lifecycle, and invalidation
at 1/2/6 physical workers, including ASan/UBSan. Existing C3/fullgraph/context/pool
contracts and targeted TSan checks supplement these tests.

The corpus runner verifies frozen input hashes and executes only validation
programs for 61 graphs × four algorithms × three worker counts × baseline/enabled/
disabled. It uses independent validators and compares PageRank bits directly.
It invokes no benchmark, warmup, profiling, calibration, or performance gate.
Program stdout may contain the language's existing internal timer text; it is not
used as performance evidence. Correctness acceptance makes no speedup claim.

For the frontier extension, use `tools/validate_frontier_simplification.py` with
the same stages plus `activation`, and a fresh `--version`. It adds focused LLVM/JIT comparisons,
native descriptor/dispatch tests (normal, ASan/UBSan and scratch unavailable),
and targeted TSan checks. Tests cover repeated sources, duplicate/self edges,
lazy conditional snapshot reads, empty/cancelled work, invalid descriptors,
journal enable/disable transitions, topology notification and lease reuse.
Generated IR is saved externally for structural inspection. The report is
`reports/frontier_execution_simplification_correctness.json`; earlier correctness
and benchmark reports are preserved. No frontier benchmarks are authorized yet.
The `activation` stage uses a correctness-only link wrapper to count actual
generated-kernel invocations in BFS/SSSP/CC at 1/2/6 workers. It checks original
and journal variants, runtime disablement, and independently validates outputs.
It does not read a clock or report performance.
