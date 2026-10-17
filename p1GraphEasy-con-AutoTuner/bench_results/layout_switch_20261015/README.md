# SGPL AutoTuner layout-switch wins

Five synthetic BFS and edge-churn workloads were compared against four fixed graph layouts.
The same SGPL source, compiler, optimization settings, graph input and runtime objects are used for all five variants of each case.

## Results

Times below are median total executable wall times in milliseconds. Speedup is the fastest fixed-layout median divided by the AutoTuner median.

| Case | AUTO ms | CSR ms | PCSR ms | BCSR ms | SET ms | Best fixed | Speedup |
| --- | ---: | ---: | ---: | ---: | ---: | --- | ---: |
| 1 | 41.563 | 53.909 | 58.592 | 230.879 | 3299.230 | CSR | 1.297x |
| 2 | 62.402 | 86.306 | 88.325 | 410.996 | 6431.177 | CSR | 1.383x |
| 3 | 54.126 | 89.713 | 68.867 | 368.996 | 7013.014 | PCSR | 1.272x |
| 4 | 67.497 | 128.166 | 89.270 | 497.845 | 11211.063 | PCSR | 1.323x |
| 5 | 93.010 | 222.906 | 120.467 | 776.672 | 23166.228 | PCSR | 1.295x |

The machine-readable five-case comparison is [layout_switch_wins.csv](layout_switch_wins.csv).
All 175 timed observations are retained in [validation_raw_runs.csv](validation_raw_runs.csv).
Per-variant spread is retained in [runtime_spread.csv](runtime_spread.csv).

## Saved winning SGPL programs

1. [bfs_n200_d64_i4_s300_c1_r1000_t4_loop.sgpl](winning_codes/bfs_n200_d64_i4_s300_c1_r1000_t4_loop.sgpl): 200 vertices, 12800 undirected edges, 300 BFS traversals before churn and 4 afterward. Each of 1000 churn batches adds and removes 4 absent edges.
2. [bfs_n256_d96_i4_s240_c1_r1000_t4_loop.sgpl](winning_codes/bfs_n256_d96_i4_s240_c1_r1000_t4_loop.sgpl): 256 vertices, 24576 undirected edges, 240 BFS traversals before churn and 4 afterward. Each of 1000 churn batches adds and removes 4 absent edges.
3. [bfs_n300_d100_i4_s180_c1_r1000_t4_loop.sgpl](winning_codes/bfs_n300_d100_i4_s180_c1_r1000_t4_loop.sgpl): 300 vertices, 30000 undirected edges, 180 BFS traversals before churn and 4 afterward. Each of 1000 churn batches adds and removes 4 absent edges.
4. [bfs_n384_d128_i4_s140_c1_r1000_t4_loop.sgpl](winning_codes/bfs_n384_d128_i4_s140_c1_r1000_t4_loop.sgpl): 384 vertices, 49152 undirected edges, 140 BFS traversals before churn and 4 afterward. Each of 1000 churn batches adds and removes 4 absent edges.
5. [bfs_n512_d192_i4_s100_c1_r1000_t4_loop.sgpl](winning_codes/bfs_n512_d192_i4_s100_c1_r1000_t4_loop.sgpl): 512 vertices, 98304 undirected edges, 100 BFS traversals before churn and 4 afterward. Each of 1000 churn batches adds and removes 4 absent edges.

Every winning program is saved unchanged from its measured source. Its input edge list is in `graphs/`.
Executable variants, generated IR, compiler logs, timing results and independent audit logs are in `binaries/<case>/`.

To run a saved executable directly, use `codes/` or `winning_codes/` as the working directory so its relative graph-input path resolves.

## Timing protocol

- Seven timed launches per variant after one warm-up, with variant order randomized within every repetition using seed 20261015. Benchmark jobs do not overlap.
- Wall time spans subprocess launch through process exit. It includes graph loading, allocation, all graph conversions, traversal, updates, profiling and output. Compilation and diagnostic audits are excluded.
- Executables and input files are staged on the WSL local filesystem to avoid Windows mount metadata overhead. All timed launches use the same warm graph-loader sidecar cache.
- Ubuntu WSL2 on an AMD Ryzen 5 5500. SGPL, OpenMP and Roaring thread counts are all one.
- AUTO uses the existing AutoTuner cost model and generated schedule. `AUTOTUNER_FORCE_LAYOUT` is absent. There are no manually inserted layout changes in the SGPL programs.
- Fixed variants compile the identical source with `AUTOTUNER_FORCE_LAYOUT=CSR`, `PCSR`, `BCSR` or `SET`.
- The graph loader initially constructs CSR. A requested fixed layout is installed before the first workload operation and kept through the final operation. This initial conversion is included in its runtime.
- The AutoTuner makes its decisions at compile time; the emitted program executes the selected layout transitions at runtime.

## Correctness and layout audits

Separate diagnostic executables wrap every graph mutation, neighbor-iterator initialization and BFS call. These wrappers are absent from timed executables.
For every BFS, the audit checks the visited vertices, exact arc count and an order-independent 64-bit digest of all arcs against a Python reference generated from the graph and updates.
It checks that each inserted edge becomes present in both directions and each removed edge becomes absent. It also checks the executed mutation count, asserts the requested fixed layout at every graph operation, and records actual AUTO layout transitions.
All 25 case/variant audits passed. The graph edge count and standard-output hash were also checked on every timed run.

## Reproduce

Run from the repository root in Ubuntu/Linux with the existing GraphProgram compiler and GCC/G++ runtime dependencies:

```bash
python3 tools/run_layout_switch_suite.py
python3 tools/summarize_layout_switch.py
```

The CSV report builder is `report/build_report.mjs`; it authors and validates the requested CSV through the bundled spreadsheet runtime.
The exact source/compiler hashes, calibration and environment are recorded in [manifest.json](manifest.json).

These cases were selected in a search for wins. They are synthetic workload examples, not a claim that AutoTuner wins on arbitrary SGPL programs.
Earlier pilot timings and discarded candidates remain separate in `raw_runs.csv` and `codes/`; final conclusions use only the seven-run confirmation data.
