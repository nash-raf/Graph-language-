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
The realized destination kernel must also have no remaining reads of the old
rank allocation, except through its explicit contribution-map argument. A
mixed source/destination-indexed expression cannot acquire a reuse license just
because its hoisted source portion is local.

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

## Correctness outcome

Frozen native source `27f4d11b`, external build `iteration_lifetimes02`, passes
2,196 independent corpus validations and 183 exact PageRank bit-comparison groups
at 1/2/6 physical workers. R16, R17 and R18 activate on 61/61 PageRank programs;
the other three algorithms retain their preceding implementations. The CC R12
closure body is unchanged from the frozen baseline.

All 768 generated-program comparisons pass across eight new-rule subsets, R13
enabled/disabled, and 1/2/6 workers. Structural checks include ten count-premise
negative controls and six storage/liveness controls, including remaining kernel
rank reads. The worklist checks 256 lifetime-premise/subset combinations in
addition to its existing composition tests. Normal and ASan/UBSan runtime suites
pass, including 13,824 exact storage comparisons across final-use masks, odd/even
rounds, descriptor refusals and allocation failures. Existing fullgraph, frontier,
relaxation, context, scheduling and worker-pool contracts pass.

All twelve targeted TSan configurations pass and the injected-race positive
control is detected. WSL mapping failures and empty startup crashes were retried
and recorded separately. Auxiliary libgomp is serial in these detector checks;
actual SGPL workers are 1/2/6. This is not a universal absence-of-races claim.

Machine-readable correctness evidence: `reports/iteration_lifetimes_correctness.json`.

## Authorized local benchmark, 2026-10-24

Against frozen R1--R15 native source `c8d5bbff`, the six-physical-core campaign
records 30,500 timed observations and 2,440 warmups across 61 graphs and four
algorithms. Five primary and twenty confirmation observations use the previous
executable baseline, candidate, candidate with only R16--R18 disabled, and two
identical-binary controls. Both blocks' controls pass; no launches fail. The
integrity audit verifies 488 executable fingerprints, fixed inputs/calibration,
and unchanged compiler/runtime source throughout measurement. ARS-1 is unchanged.

Confirmation geometric-mean speedup (baseline / candidate; above one is faster):

| Algorithm | End-to-end | Instrumented compute region |
| --- | ---: | ---: |
| BFS | 1.0072x | 1.0068x |
| SSSP | 1.0045x | 1.0041x |
| CC | 1.0020x | 0.9994x |
| PageRank | 1.0358x | 0.8985x |

PageRank wins on 55/61 graphs versus the retained baseline end-to-end; summed
end-to-end time falls 3.92%. The same candidate with R16--R18 disabled is 1.0421x
slower geometrically end-to-end. No end-to-end case slowdown survives Holm
correction. These are modest improvements and do not establish a broad PageRank
win over Graptor: comparisons with saved historical results give 8/61 end-to-end
wins and 2/61 instrumented-compute wins. Graptor was not rerun.

The compute timer begins **after degree construction**. Nine untimed GDB traces
confirm that R16 shifts SGPL worker-pool startup across this boundary on two small
cases: `protein/DD_g422.edges` and `ia/ia-email-univ.edges`. The baseline and
rules-disabled executable initialize the pool before the timer; the enabled
executable first initializes it inside the timer. On the larger
`dimacs/DSJC1000-5.edges` all three initialize it before the timer. Thus the
geometric-mean compute slowdown includes shifted startup; it does not isolate
steady traversal. Summed compute time across the corpus falls 0.98%. No timed
observations or ratios were altered; debugger durations are excluded.

Machine evidence: `reports/iteration_lifetimes_benchmark.json` and
`reports/iteration_lifetimes_startup_diagnostic.json`. Full report, CSV, raw
observations, fixed protocol and diagnostic scripts/logs are retained under
`C:/p1-iteration-lifetimes-benchmark-20261024/`, entry point `final_report.html`.
