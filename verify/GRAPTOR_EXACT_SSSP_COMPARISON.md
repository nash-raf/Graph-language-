# Graptor on the exact autotuner SSSP workload

Measured October 14, 2026 in Ubuntu WSL on the same AMD Ryzen 5 5500 host.
These are fresh measurements, not timings taken from Graptor's papers or the
earlier comparison on other graph datasets.

## Results

Medians of 11 fresh processes per implementation, with randomized interleaving
after one warmup per implementation. All processes inherit CPU affinity
`[0,2,4,6]` and use four workers. CPU execution only.

| Implementation | Internal measured region (ms) | Complete process (ms) |
|---|---:|---:|
| effect_interim | 6.109 | 67.837 |
| Repaired merge, before execution optimizations | 57.451 | 313.354 |
| Optimized merge | 43.512 | 175.394 |
| Graptor BFExport | 1.258 | 54.578 |

The internal column has different boundaries: SGPL reports its accumulated
`Traverse` profiler regions, including the previously identified setup charged
to region 0. Graptor times `problem.run()`, after graph loading, graph/layout
construction, and benchmark-object construction. Result export and validation
are outside that timer. Do not interpret ratios between these internal numbers
as an exactly matched kernel speedup.

The complete-process column uses the same Python launch-to-pipe-completion
measurement for all implementations. It includes loading, layout/runtime setup,
computation, ordinary summary output and teardown. Median process-time ratios
relative to Graptor are 1.243 for effect_interim, 5.742 for the repaired merge,
and 3.214 for the optimized merge. SGPL uses its existing warm graph-cache
sidecar; Graptor reads a preconverted CSR file and still performs its normal
VEBO/layout construction each process. Text-to-binary conversion is outside
timing for both systems.

## Exact input and correctness

- Original fixture: `verify/fixtures/g20k_w.txt`, SHA-256
  `1b0acbc386492f91e68becc7665dfc270bcfc7549c94f2e9fcebeff0f9711afe`.
  The comparison's SGPL fixture copy has the identical hash.
- 20,000 vertices and 160,000 undirected weighted input edges, producing
  320,000 CSR adjacency entries. The graph declaration's `TRUE` token selects
  weighted storage; no directed property is present.
- Source vertex 1. No generated random weights, symmetrization changes,
  weight scaling, vertex dropping or deduplication was applied.
- The Graptor conversion explicitly emits both directions, sorts each CSR row,
  retains all edge occurrences and writes aligned float32 weights. Its binary
  header, row offsets, destinations and weights were re-read and checked.
- An independent directed-adjacency Dijkstra run on the resulting symmetric
  adjacency supplies every reference distance. Reachable count is 20,000,
  distance checksum is 1,182,819, and maximum distance is 114.
- Graptor's result export matched every logical vertex's distance exactly.
  It stores float32 distances versus SGPL's integer distances; all input
  weights and the small nonnegative integer distances are exactly representable
  here. This does not establish general integer/float equivalence on other
  inputs.
- Every timed SGPL process retained reachable count 20,000 and checksum
  1,182,819. Graptor's full-vector export was enabled in the separate correctness
  run and disabled for timed runs.

This matches the SSSP input, source and final mathematical result. The engines
may use different frontier representations, update schedules, iteration counts
and SIMD strategies. It does not require Graptor to reproduce SGPL's ordered
array append semantics or intermediate states.

## Implementation and controls

Graptor checkout: `D:/sgpl/graptor`, revision
`3ce3b6baa09679b0d187a6165c61fdaa7208b336`. Existing `BFExport` is derived
from upstream `BFv.C` with only a conditional full-vector export added to
`post_process()`, after the algorithm timer. No Graptor source or binary was
changed for this experiment. Its existing build uses GCC 13, OpenMP, native
instructions, `VEBOGraptorPullDataParCached`, and SIMD width 8. Eight graph
partitions are fixed; the worker count is four.

The three preserved SGPL CPU binaries are the same ones used for the prior
effect-interim/merged/optimized comparison. No SGPL compiler, runtime, autotuner
equation or calibration was changed. The previous frozen calibration file
remains selected, so these numbers do not claim fresh hardware calibration.

Graptor command:

```text
BFExport -s -b -c 8 -l 8 -rounds 1 -start 1 
  -weights g20k_undirected.weights g20k_undirected.graptor
```

`-rounds 1` is one complete benchmark invocation, not a limit of one
Bellman-Ford iteration.

Artifacts: `verify/bin/graptor_exact_sssp_2026_10_14/` contains `run.py`,
`metadata.json`, `raw.json`, `summary.json`, the converted CSR/weights,
independent reference distances, validation export, and every stdout/stderr log.
Reproduce with:

```text
wsl -d Ubuntu -- python3 /mnt/d/sgpl/Graph-language-/verify/bin/graptor_exact_sssp_2026_10_14/run.py
```
