# Proof-gated execution simplification

The reference is ARS-1 commit `2b46cb5ded54132e98dc052bd2caf548d1acfcb4` (accepted C3).
This experimental branch specializes certified CPU full-domain destination-owner
traversals. It does not change frontier multiplicity, propagation algorithms,
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

## Controls

- Compile with `SGPL_EXEC_SIMPLIFY_RULES=all` (default), `none`, or a comma-separated
  subset of `R1,R2,R3`. Unknown rule IDs are errors.
- Set `SGPL_NO_EXEC_SIMPLIFY=1` at execution to use the retained baseline kernels
  and maps. Existing full-graph/C3 controls still apply.
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
