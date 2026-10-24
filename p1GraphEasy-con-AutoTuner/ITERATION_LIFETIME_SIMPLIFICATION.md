# Composable iteration lifetime simplification

Branch `codex/iteration-lifetime-simplification` builds on the frozen R1--R15
implementation `c8d5bbff`. It retains the previous ordered iteration, gate
removal, SSSP paths and CC closure. No new cost model, SIMD, calibration or
algorithm-name eligibility check is added. ARS-1 remains unchanged.

## R16: source-local counting from existing row lengths

The compiler recognizes a complete, unobserved, full-source-domain context with
one transparent pair operation: load a private source-indexed i32 element, add
one with modular arithmetic, and store to that same location. Destination
dependence, other reads/writes, gates, lifecycle callbacks, escaping arrays,
opaque calls, atomic/volatile accesses and no-wrap additions refuse the rule.
It does not assume the initial counts are zero.

The runtime validates the current CSR identity, extents and every row boundary
before touching the array. It adds the native row length modulo 2^32 once per
nonempty source. Duplicate edges and self-loops retain their multiplicity; empty
rows do not access the count array. No neighbor-indexed counting loop remains
on this selected path. Context preparation, profiling and result-status handling
stay in place. Unknown layouts/invalid witnesses select the original callback
execution before effects. A trusted certificate records that the helper neither
publishes the count array nor mutates it inside a later iteration; existing
alias and stable-gate proofs consume this fact.

## R17: omit unobserved final restoration

The existing complete iteration proof permits buffer rotation but originally
restored both arrays' final contents. The new analysis walks all blocks reachable
from the iteration exit and follows private allocation pointers, global-slot
loads, derived pointers and stack aliases. A post-loop array access keeps the
array live. Unknown consumers and callbacks usable outside the certified window
refuse deadness. Both original pointer identities are always restored.

The compiler records a separate observation bit for each array. Only a proven
unobserved array loses its final restoration. Observing both arrays retains both
restorations; observing one retains that one's required copy. Overwrites are
conservatively treated as observations. This rule does not remove any ordered
edge contributions or make a previously observable intermediate value disappear.

## R18: reuse source storage after its last read

This rule requires one double map field, source element reads at exactly the
current source index, and all other contribution arrays disjoint from the source
allocation. The gate must be immutable and disjoint from both rotating arrays,
even when R13 is disabled or its dynamic implication witness fails. Opaque,
atomic/volatile, nonlocal and unresolved reads refuse reuse.

Each source-map worker reads its own old rank and writes its contribution to
that same cell. The map join still precedes every destination traversal. The
seeded destination kernel still overwrites every destination, including empty
ones, before rotation. The next round uses the newly produced rank buffer.
The contribution scratch allocation disappears. No arithmetic, floating flags,
addition order, duplicate contributions or mandatory joins change.

All fallible preparation completes before source storage is overwritten. Every
observed final array is restored using R17's observation mask (or the default
mask for both arrays). Thus destructive reuse remains correct independently of
copy removal and with either odd or even iteration counts.

## Composition, fallback and controls

The existing shared compiler/runtime worklist carries explicit counting,
post-loop use, source locality and gate-disjointness facts. Missing facts refuse.
New internal storage descriptors check version, size, rule bits, observation
bits, baseline plan and callback identities. Existing descriptor layouts and
entry points remain compatible. An invalid extension retains ordinary iteration
execution. Selection happens before effects; there is no partial-work replay.

`SGPL_EXEC_SIMPLIFY_RULES=all|none|R1,...,R18` isolates compiler rules.
`SGPL_NO_ITERATION_LIFETIMES=1` disables R16--R18 while retaining the previous
improvements. Existing full simplification, iteration-memory and R13--R15
controls remain intact. Compile diagnostics use `SGPL_EXEC_SIMPLIFY_STATS=1`;
runtime selection diagnostics use `SGPL_SEMANTIC_STAGE_STATS=1`.

`tools/validate_iteration_lifetimes.py` performs correctness-only validation,
including all eight new subsets with R13 enabled and disabled, negative proof
fixtures, exact floating-point outputs, odd/even rounds, all final-use masks,
allocation failures, normal/ASan/UBSan contracts, targeted TSan and the frozen
61-graph/four-algorithm corpus at 1/2/6 physical workers.
`tools/benchmark_iteration_lifetimes.py` is a separate, authorized benchmark
entry point. It retains previous executables/reports and reuses saved Graptor
observations without launching Graptor.
