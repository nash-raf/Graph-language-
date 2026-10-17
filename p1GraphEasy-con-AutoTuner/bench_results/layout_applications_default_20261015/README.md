# Default-backend application benchmarks

These are the exact five programs and identical input bytes from the earlier motif experiment. They now use the repaired default compiler. No GRAPH_FRONTIER_REWRITE_OFF setting is used for any variant. These are synthetic application workload models, not production trace measurements.

| Application | AUTO ms | CSR ms | PCSR ms | BCSR ms | SET ms | Best fixed | Speedup |
| --- | ---: | ---: | ---: | ---: | ---: | --- | ---: |
| Contact exposure tracing | 610.497 | 33331.756 | 1470.676 | 119100.386 | 2624.596 | PCSR | 2.409x |
| Service outage islands | 548.014 | 28611.301 | 1079.520 | 77550.595 | 4078.513 | PCSR | 1.970x |
| Sensor backbone pruning | 156.626 | 4300.112 | 670.878 | 34378.765 | 1184.856 | PCSR | 4.283x |
| Gateway failover recovery | 278.643 | 31917.882 | 1257.533 | 111343.424 | 1175.230 | SET | 4.218x |
| Trust-boundary audit | 160.055 | 28363.849 | 566.958 | 83184.447 | 443.360 | SET | 2.770x |

Speedup is the fastest fixed-layout total runtime divided by AUTO total runtime. Values below 1 would indicate a slowdown. Every case is retained, regardless of outcome.

## Method

- Shared graph: 544 vertices and 128032 undirected edges, with a dense projected cohort and peripheral chain. Each program replays 80000 event windows with eight endpoint pairs. Program control flow and application outputs differ. Sources contain no manual layout commands.
- AUTO and fixed variants compile identical SGPL. Only the AUTOTUNER_FORCE_LAYOUT setting changes between variants. The fixed layout is installed before the first update/traversal and retained for those operations; the loader starts in CSR and its initial conversion is included.
- All thread counts are one. Executables and identical input are staged on WSL local storage. The graph-loader sidecar is prebuilt before measurement.
- AUTO, PCSR and SET have seven full launches each; CSR and BCSR have one full launch each because they are much slower. Reported values are medians. Sample counts, AUTO range and variability are explicit columns in the CSV. The slow baselines do not have repeat-based variability estimates.
- No full-workload warmup samples are discarded. Variant order is randomized with seed 20261015, and timing covers subprocess launch through exit. All 115 measured executions must match the independent Python reference outputs.
- Separate audited executables reuse the exact measured program objects. Audits are excluded from timing. They check real layout transitions, fixed layouts, native iterator epochs, graph fingerprints, update-call counts and sampled physical mutation effects. CSR/BCSR diagnostic runs skip the middle recurring updates; all measured binaries run the entire update stream.
- Actual AUTO transitions are CSR, SET, CSR in all five cases.

## Results and reproduction

The full five-variant data are in [layout_application_comparison.csv](layout_application_comparison.csv). Individual timed launches are retained in [validation_raw_runs.csv](validation_raw_runs.csv). Compiler, source, input and binary hashes and audit coverage are in [manifest.json](manifest.json).

- [Contact exposure tracing](winning_codes/contact_trace.sgpl)
- [Service outage islands](winning_codes/service_islands.sgpl)
- [Sensor backbone pruning](winning_codes/sensor_backbone.sgpl)
- [Gateway failover recovery](winning_codes/route_failover.sgpl)
- [Trust-boundary audit](winning_codes/trust_audit.sgpl)

Run from the repository root in Ubuntu/WSL:

```sh
python3 tools/benchmark_default_layout_applications.py --resume
```

Use `codes/` or `winning_codes/` as the working directory when launching saved binaries directly, so their graph path resolves. The earlier motif results remain in `../layout_applications_20261015/`.
