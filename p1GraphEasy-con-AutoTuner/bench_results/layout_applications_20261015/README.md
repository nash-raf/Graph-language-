# Distinct SGPL application workloads

Five different programs use the same 544-vertex, 128032-edge input graph. These are synthetic workload models, not measurements from production traces.

| Workflow | AUTO ms | CSR ms | PCSR ms | BCSR ms | SET ms | Fastest fixed | Speedup |
| --- | ---: | ---: | ---: | ---: | ---: | --- | ---: |
| Contact exposure tracing | 261.801 | 34239.644 | 1156.927 | 127882.645 | 1096.585 | SET | 4.189x |
| Service outage islands | 188.610 | 30826.203 | 904.634 | 82320.020 | 1331.949 | PCSR | 4.796x |
| Sensor backbone pruning | 169.825 | 4781.085 | 722.876 | 38748.897 | 507.419 | SET | 2.988x |
| Gateway failover recovery | 305.762 | 33682.004 | 1322.545 | 116395.318 | 1325.182 | PCSR | 4.325x |
| Trust-boundary audit | 165.195 | 28666.076 | 606.555 | 78874.584 | 473.340 | SET | 2.865x |

Times are total program wall times. Speedup is the fastest fixed-layout time divided by AUTO time.
The full comparison is [layout_application_wins.csv](layout_application_wins.csv). Raw launches are retained in [validation_raw_runs.csv](validation_raw_runs.csv).

## Winning programs and useful results

- [contact_trace.sgpl](winning_codes/contact_trace.sgpl): BFS exposure cohort and maximum hop count.
- [service_islands.sgpl](winning_codes/service_islands.sgpl): Connected components and largest island.
- [sensor_backbone.sgpl](winning_codes/sensor_backbone.sgpl): 490-core retention and failed-sensor eligibility.
- [route_failover.sgpl](winning_codes/route_failover.sgpl): Outage BFS, alternate-gateway recovery, and DFS inspection.
- [trust_audit.sgpl](winning_codes/trust_audit.sgpl): DFS inventories on both sides of a revoked gateway.

Contact tracing expires old contacts, introduces a new window, then reports potential exposure cohorts and furthest hop count.
Outage detection identifies two disconnected service islands and their largest membership count.
Backbone pruning maintains a supplied degree cache with every radio-link change, then computes the 490-core. It retains 511 sensors and excludes the failed sensor.
Failover recovery reports stranded clients, commissions an alternate gateway, and verifies that all 544 vertices are reachable again.
Trust auditing reports the 512-vertex authorized component and the isolated 32-vertex component after a gateway revocation.

The shared fixture contains a dense projected cohort and a sparse peripheral chain. All programs replay recurring hot-edge event windows, but their event semantics, analysis code, and outputs differ. The benchmark source is generated with literal endpoint IDs because this SGPL grammar accepts only literal mutation endpoints.

## Compiler and measurement protocol

- All cases use the supported motif backend: `GRAPH_FRONTIER_REWRITE_OFF=1` during compilation, for AUTO and every fixed variant. Ordinary iterator loops in the current default backend pin the graph to CSR, so this report does not claim these gains for default lowering.
- AUTO has no layout-forcing variable. Its executed schedule is CSR, then SET for the update burst, then CSR for analysis. The SGPL sources contain no manually inserted layout operations.
- Fixed variants compile identical source with `AUTOTUNER_FORCE_LAYOUT=CSR`, `PCSR`, `BCSR`, or `SET`. The loader starts in CSR; install the fixed representation before the first traversal or update and keep it through the remaining traversal and mutation phases. Include the initial conversion in runtime.
- SGPL, OpenMP, and Roaring thread counts are one. Input and executables are staged on WSL local storage, with the graph-loader sidecar prepared before timing.
- Timers cover subprocess launch through exit, including loading, allocation, conversions, all updates, analysis, profiling, and printed results. No kernel-only timings are used.
- Contact tracing has three full launches per variant. The remaining four cases have seven launches each of AUTO, PCSR, and SET, and one full launch each of CSR and BCSR. Sample counts are explicit CSV columns. Those slow baselines are orders of magnitude away from the winning competitor.
- All 107 full timed launches were checked against independent Python reference traversals, components, and k-core calculations. Traversal outputs are compared as vertex inventories, allowing different valid traversal orders.
- Separate diagnostics check every graph-call layout, actual AUTO transitions, graph fingerprints, exact update-call counts, and physical edge changes in the first and last two event windows plus permanent changes. AUTO/PCSR/SET diagnostic runs execute all native updates. CSR/BCSR diagnostics skip native work in middle recurring windows; this optimization is absent from timed executables.

## Reproduce

Run from the repository root under Ubuntu/Linux:

```bash
python3 tools/layout_application_benchmark.py --rounds 80000 --modes AUTO,CSR,PCSR,BCSR,SET --adaptive-confirmation --warmups 0 --timeout 600
python3 tools/audit_layout_applications.py
```

The first command uses seven fast-variant runs and one slow-variant run for every case. The retained contact-tracing measurements instead use three of each, as recorded in the CSV.
Use `codes/` or `winning_codes/` as the working directory when executing a saved binary directly, so the shared graph path resolves.
Source, compiler and binary hashes, exact sample counts, and audit coverage are in [manifest.json](manifest.json). Generated IR and logs are in `binaries/<case>/`.

Cases were selected in a search for layout-switch wins. The earlier graph-size variants are retained separately in `../layout_switch_20261015/` and do not count as these five application cases.
