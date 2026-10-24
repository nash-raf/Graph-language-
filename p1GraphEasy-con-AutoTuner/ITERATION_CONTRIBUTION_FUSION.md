# Exact producer-map fusion (R19)

This experiment starts from accepted ARS-1 `7df87a26` and retains R1--R18.
It adds no cost model, threshold, calibration, SIMD, asynchronous traversal,
or algorithm-name eligibility test. ARS-1 is the frozen reference.

## Transformation and proof

The existing complete, bounded seed/traversal/copy certificate must hold.
R19 additionally requires a single double map field, source-local rank reads,
immutable disjoint gate inputs, and no remaining old-rank reads in the
destination kernel. The generated map must have a checked canonical range
loop and a single map-element store. Volatile/atomic operations, opaque calls,
nonlocal rank reads, constrained/strict floating point, intermediate observers,
and unproved aliases refuse the corresponding prerequisite.

Let `q = map(rank)` and let `fold(q, v)` be the existing ordered destination
calculation including its original seed. Initially compute `q0 = map(rank0)`.
For each nonfinal round, calculate `rank_next[v] = fold(q, v)` in the original
edge order, then immediately compute and store `map(rank_next[v])` in the other
buffer. The last round stores the actual rank using the retained kernel.
The map's original arithmetic, operands, flags, zero-outdegree guard and
duplicate-edge order remain intact. No reciprocal replacement, reassociation,
contraction or floating reduction is introduced. A singleton clone of the
original map is inlined at the destination producer's store; its source rank
loads are replaced with the just-produced scalar rank.

The complete window proves the absence of intermediate observers and other
writes to map/gate inputs. Thus shifting a next-round map calculation to the
end of the preceding producer does not change its inputs or observable effects.
Each destination still belongs to one worker. Current contributions and next
contributions use separate buffers. The existing destination join publishes
all produced cells before rotation; no new barrier or worker-team mechanism
is introduced. Zero-outdegree vertices need no intermediate contribution,
but the final kernel still calculates and stores their actual ranks.

The initial source map is retained. Remaining rounds omit the separate source
map dispatch/join, rank materialization and subsequent rank reread. The runtime
executes one initial map sweep and exactly the original number of destination
sweeps. One-round execution remains one map and one rank-producing traversal.

## Composition, selection and fallback

The allocation-free proof worklist records every required premise. R19 needs
R13's compiler certificate and successful pre-effect dynamic gate witness.
A false immutable gate on any edge-bearing source retains the accepted gated
iteration. R16, R17 and R18 are independently selectable; R18 allows in-place
initial mapping, while all subsequent fused rounds use distinct buffers.
R17's liveness mask controls final restoration, and all original array pointer
identities are restored. Observed arrays retain exact final rank contents.

An additive versioned private fusion descriptor identifies the original plan,
pair, optional storage descriptor, R13 descriptor and fused kernel. Existing
descriptors, entry points and public context layouts remain intact. Version,
size, rule, reserved bits and baseline identities are checked before selection.
All index acquisition, allocations and gate witnesses finish before effects.
Allocation or topology refusal returns to the original loop before effects;
an unsupported extension retains accepted iteration execution. There is no
fallback by replaying partial work. Existing index cache invalidation applies.

## Controls and evidence

- `SGPL_NO_ITERATION_FUSION=1` retains accepted R1--R18 execution at runtime.
- `SGPL_EXEC_SIMPLIFY_RULES=all|none|R1,...,R19` selects compiler rules.
- `SGPL_EXEC_SIMPLIFY_STATS=1` emits compiler activation/refusal diagnostics.
- `SGPL_SEMANTIC_STAGE_STATS=1` reports runtime activation and sweep counts.

Correctness: `tools/validate_iteration_fusion.py` freezes an external build,
checks 32 compiler rule subsets, negative premises, retained contracts, native
normal/ASan/UBSan execution and exact generated-program floating-point bits at
1/2/6 physical workers. It compares the frozen 61-graph/four-algorithm corpus
against the accepted baseline and a same-candidate R19-disabled control.
Targeted instrumented TSan execution records detector startup failures
separately and uses an injected-race positive control.

Benchmark: `tools/benchmark_iteration_fusion.py` is a separate entry point and
refuses measurement before correctness passes. It reuses hash-verified accepted
timed executables, measures candidate/disabled/identical-binary controls, and
reads saved same-PC six-worker Graptor observations without running Graptor.
Compute starts after degree construction and includes preparation first needed
inside that region; it does not isolate steady edge-loop throughput. Historical
Graptor win counts are descriptive cross-session comparisons. Results and any
regressions are recorded after the full campaign; no speedup is presumed.

## Correctness outcome

Frozen `iteration_fusion02`, native source `a3108f93`, passes 2,196 independent
corpus validations and 183 exact PageRank bit groups at 1/2/6 physical workers.
R19 activates on all 61 PageRank programs and none of the BFS/SSSP/CC programs.
All 960 new generated-program comparisons pass, including gate refusal,
intermediate observers, remaining kernel rank reads, empty graphs, one-round
execution and special floating-point values. The 32 compiler subsets, negative
premises, runtime descriptor faults, allocation refusal, repeated topology
registration and final liveness masks pass. Native normal and ASan/UBSan
checks pass at all three widths, alongside the retained execution contracts.

All 12 targeted TSan configurations pass after separately recorded WSL mapping
startup retries; the positive control detects its intentionally injected race.
Auxiliary system OpenMP work is serialized for these checks, while SGPL runs
at 1/2/6 workers. This is not a universal proof of race absence. The machine
report is `reports/iteration_fusion_correctness.json`. The earlier frozen
`iteration_fusion01` is superseded by the explicit separation of static
eligibility and dynamic gate entailment; it was not benchmarked.

## Confirmed local performance

The six-physical-core campaign collected 30,500 timed observations and 2,440
warmups, with five primary and twenty confirmation observations per variant
and graph. Both blocks passed their identical-binary controls, with zero failed
launches and zero new Graptor launches. The audit verified all 488 timed
executable fingerprints and unchanged native sources, inputs and calibration.

| Algorithm | E2E geometric speedup vs accepted | Compute geometric speedup | E2E wins vs accepted | E2E wins vs saved Graptor |
| --- | ---: | ---: | ---: | ---: |
| BFS | 0.9961x | 0.9945x | 26/61 | 30/61 |
| SSSP | 0.9975x | 0.9964x | 28/61 | 46/61 |
| CC | 0.9976x | 0.9823x | 27/61 | 53/61 |
| PageRank | 1.0054x | 1.0205x | 36/61 | 8/61 |

Values greater than one favor the candidate. These are measured geometric
ratios, not guarantees of improvement. No end-to-end slowdown survives Holm
correction. PageRank's summed E2E time decreases 0.8066%, and summed compute
time decreases 3.2007%. The same candidate with R19 disabled is 1.0280x slower
end-to-end geometrically, but the net gain against the accepted executable is
only 1.0054x. The difference between these controls has not been attributed to
specific code-placement, dispatch or instruction effects.

Both the accepted and candidate executables win 8/61 PageRank graphs against
the same saved Graptor E2E means in this campaign; the candidate wins 4/61
recorded compute comparisons. Saved Graptor is 1.1422x faster geometrically
end-to-end and 1.6057x faster in the recorded compute region. These historical
comparisons do not isolate steady traversal throughput or establish an exact
cause of the residual gap. R19 removes the intended repeated phase, but the
measured benefit remains small; it does not demonstrate broad PageRank wins.

Full report: `C:/p1-iteration-fusion-benchmark-20261024/final_report.html`.
Machine report: `reports/iteration_fusion_benchmark.json`. The experimental
branch retains the complete change and fallback; accepted ARS-1 remains at
`7df87a26` pending a separate acceptance decision.
