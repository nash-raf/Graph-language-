# Composable proof removals

Branch: `codex/composable-proof-removals`, based on retained iteration-memory
checkpoint `a74c93f3`. ARS-1 and the rejected SIMD/team/per-worker-reduction
experiment are unchanged. Correctness validation and the subsequently authorized
local benchmark are recorded below. There are no new cost models or changes to
ARS/layout selection.

## Composition

`execution_proof_worklist.h` supplies the same allocation-free, monotone worklist
to the compiler and C runtime. Consumers precede producers deliberately. A new
fact requeues its consumers; each rule fires at most once. Missing premises
refuse, and planning completes before lowering or execution.

Compiler witnesses license eligibility, not unobserved dynamic facts. In the
semantic-stage worklist R10/R11 establish traversal, R8 establishes set output,
R14 consumes the set and private recurrence facts, and R15 consumes weighted
traversal. Runtime composition then requires actual range/topology witnesses.
R13 similarly needs both compiler-proved immutability and a pre-effect witness.
This does not equate different callbacks or grant algebra to arbitrary code.

## R13: remove an edge-entailed immutable source gate

The existing bounded seed/traversal/copy proof establishes the complete window
and its only permitted writes. The additional proof requires read-only gate
code, stable private scalar slots, and private gate arrays whose allocation
sites are disjoint from both rotating arrays. Mutable, escaped, atomic,
volatile, opaque or unresolved reads refuse. No degree-array name is special.

Before any array effects, evaluate the actual gate once and check that every
source with an outgoing edge passes. This witnesses the predicate rather than
assuming a degree count cannot wrap. Gates on empty sources may be false.
If the witness succeeds, select ungated kernel/map clones and free the temporary
gate bitmap before iteration. Per-round gate initialization, callback execution,
and kernel/map bitmap reads disappear. All arithmetic and incoming edge order
remain scalar and unchanged. If the witness fails, retain the existing gated
iteration bundle. The witness is checked anew for each invocation.

The original 64-byte iteration descriptor/entry point remain intact. The additive
gate descriptor checks version, size, rule, reserved bits and both baseline
descriptor and callback identities before selecting its clones.

## R14: remove a frontier reservation scan, propagate range facts

Only R8's separately certified minimum/set producer may establish the bounded
output fact. It emits at most `n` unique, valid destination IDs, regardless of
input duplicates or outgoing degree. Reserve against `min(n, candidates)` using
the retained contribution count already computed by R8, instead of summing
input degrees; no other R8 consumer needs that sum. The range check remains for
the initial input and every input without an inherited producer witness.

The compiler's existing complete, private, unobserved recurrence proof excludes
frontier mutation/publication between producer and consumer. A private stack
key scopes an internal thread-local witness. Reset occurs before each new loop
invocation, including reuse of the same stack address. After a successful R8
round, the runtime records the produced pointer/count, descriptor, graph,
CSR pointers, extents, layout epoch and topology generation. An exact match
licenses the next round's range-check removal. Every refusal clears the fact
before fallback executes; mutation/layout changes and another/nested invocation
discard it. The old entry point cannot inherit a frontier witness.

This rule neither folds new operations nor removes the snapshot required by
the round semantics. Arbitrary/nonfoldable callbacks do not gain a set license.
All fallible reservations still precede destination writes; there is no replay.

## R15: remove an identity first-match weight index

The existing topology validation already tracks first neighbor occurrences.
When each source row has unique neighbor IDs, its first-match map is identity.
The shared worklist derives direct weight access from that fact and the weighted
traversal contract. R8 selects separate direct count/fill workers once before
dispatch; their optimized code contains no first-match map load. Generic R10
ordered callbacks may also consume identity, including nonfoldable updates.
R11's existing mapped row representation remains a conservative fallback.

No first-match map is allocated for identity-capable consumers. Legacy consumers
still obtain a real map, materialized lazily if necessary. Duplicates retain the
first matching edge's weight; self-loops alone do not defeat identity. Weight
values are always read live, with original modular arithmetic and signed
comparisons. No positivity or absence-of-overflow assumption is introduced.
The cache uses the existing graph/pointer/extent/layout/generation invalidation.

## Controls and validation

Eligible rules are enabled by default here. Compilation accepts
`SGPL_EXEC_SIMPLIFY_RULES=all|none|R1,...,R15`. New rules still require their
independent prerequisite rules; requesting only R14 cannot fabricate R8/R10.
`SGPL_NO_PROOF_REMOVALS=1` retains the preceding iteration-memory execution.
The existing full simplification and iteration-memory disables remain available.
Compiler diagnostics use `SGPL_EXEC_SIMPLIFY_STATS=1`; dynamic witness/selection
diagnostics use `SGPL_SEMANTIC_STAGE_STATS=1`.

`tools/validate_proof_removals.py` builds frozen external artifacts, strips timing
blocks from corpus programs, and compares the preceding frozen implementation,
candidate enabled, and candidate fully disabled. It has no benchmark stage.
Additional fixtures exercise all eight new rule subsets, exact floating bits,
false/mutable gates, unique/duplicate topology, nonfoldable weighted code,
range induction, invalid inputs, descriptor rejection, allocation failures and
invalidation at 1/2/6 physical workers. Structural checks inspect generated LLVM
and optimized native direct workers. The report records detector startup
limitations separately from passing race checks.

Run correctness stages with the existing WSL validation venv:

```
python tools/validate_proof_removals.py all --version proof_removals04
```

Results: `reports/proof_removals_correctness.json`; external artifacts:
`/home/kazi/p1_execution_simplification/versions/proof_removals04`.

Validated native source: `c8d5bbff3be010e2c9ca301e3f723021182ce076`.
The completed checks include 2,196 independent corpus validations, 183 exact
PageRank bit-comparison groups, 384 generated-program runs across all eight new
rule subsets, 540 compiler premise checks, the existing contracts, ASan/UBSan,
and the worklist subset/registration-order checks. Corpus activation is R13 on
61/61 PageRank cases and R14/R15 on 61/61 SSSP cases; range induction also
propagates on all 61 SSSP cases.

All 12 targeted TSan configurations pass, with the injected-race positive control
detected. The comparison disables exactly R13/R14/R15. WSL detector startup
failures and empty startup crashes were retried and are recorded as separate
attempts. Auxiliary OpenMP runs serially; actual SGPL workers are 1/2/6. These
correctness checks contain no benchmark runs and make no performance claim.

## Authorized local benchmark, 2026-10-24

`tools/benchmark_proof_removals.py` compares the frozen retained iteration-memory
baseline (`fc54b6f8`) with candidate native source `c8d5bbff`, plus candidate with
R13--R15 disabled and identical-binary controls. ARS-1 remains unchanged.
The corpus contains 61 graphs and four algorithms at six physical workers on
this PC. Five primary and twenty independent confirmation observations per
case/variant produced 30,500 timed observations and 2,440 warmups. Both blocks'
identical-binary controls passed; no launches failed. The integrity audit passed
and verified 488 executable fingerprints. No native source changed during timing.

Confirmed geometric-mean speedup (retained baseline / candidate; above 1 wins):

| Algorithm | Whole-process speedup | Whole-algorithm compute speedup | Whole-process wins vs baseline |
| --- | ---: | ---: | ---: |
| BFS | 0.997x | 0.994x | 25/61 |
| SSSP | 1.190x | 1.364x | 60/61 |
| CC | 0.994x | 0.988x | 26/61 |
| PageRank | 1.032x | 1.106x | 47/61 |

Compute includes algorithm preparation, not just pair callbacks. R13--R15
disablement produces consistent SSSP and PageRank comparisons. CC is broadly
flat, but `dimacs10/wing.edges` has a 7.3% mean whole-process slowdown confirmed
by the paired permutation test with Holm correction (adjusted p = 0.0223).
Win counts compare means and do not imply individual statistical significance.

Graptor was not rerun. Comparisons reuse historical observations from this PC
and are unpaired. Candidate whole-process wins against saved Graptor are BFS
48/61, SSSP 56/61, CC 54/61 and PageRank 8/61; whole-algorithm compute wins are
respectively 1/61, 0/61, 1/61 and 5/61. The SSSP compute gap therefore remains.

Machine-readable audit: `reports/proof_removals_benchmark.json`. Full report,
per-graph CSV, raw observations, executable identities and protocol are retained
under `C:/p1-proof-removals-benchmark-20261024/`; the report entry point is
`final_report.html`. Original benchmark reports and accepted binaries were not
overwritten.
