# Parallel boost ablation

Runs=5, saxpy/prefix n=6000000, matmul n=448, combo n=288

| kernel | mode | median ms | speedup | GOMP | pfr | doacross |
|---|---|---:|---:|---:|---:|---:|
| doall_saxpy | serial | 52.8 | 1.00× | 0 | 0 | 0 |
| doall_saxpy | pdg | 105.8 | 0.50× | 0 | 2 | 0 |
| doall_saxpy | polly | 52.5 | 1.00× | 0 | 0 | 0 |
| doall_saxpy | both | 67.3 | 0.78× | 0 | 0 | 0 |
| doacross_prefix | serial | 22.8 | 1.00× | 0 | 0 | 0 |
| doacross_prefix | pdg | 149.3 | 0.15× | 0 | 2 | 38 |
| doacross_prefix | polly | 36.6 | 0.62× | 0 | 0 | 0 |
| doacross_prefix | both | 33.2 | 0.69× | 0 | 0 | 42 |
| polly_matmul | serial | 78.1 | 1.00× | 0 | 0 | 0 |
| polly_matmul | pdg | 90.4 | 0.86× | 0 | 2 | 24 |
| polly_matmul | polly | 42.8 | 1.83× | 6 | 0 | 0 |
| polly_matmul | both | 21.2 | 3.68× | 6 | 1 | 0 |
| combo_all3 | serial | 22.3 | 1.00× | 0 | 0 | 0 |
| combo_all3 | pdg | 27.0 | 0.83× | 0 | 4 | 20 |
| combo_all3 | polly | 9.2 | 2.43× | 6 | 0 | 0 |
| combo_all3 | both | 16.4 | 1.36× | 6 | 1 | 12 |

## Modes
- **serial**: no Polly, no PDG/outliner
- **pdg**: DOALL/DOACROSS outlining only (`GRAPH_DISABLE_POLLY=1`)
- **polly**: Polly O3 + `-polly-parallel` only (`GRAPH_DISABLE_PDG=1`)
- **both**: Polly + PDG

## Notes
- `SGPL_FORCE_DOALL_PARALLEL=1` is set for `pdg`/`both` so single-shot DOALL
  kernels skip cost-model warmup (otherwise they stay serial within one process).
- DOACROSS is left to the cost model (forcing it on multi-million prefixes hangs).
- Nested-loop pool miss previously forced `P=1`; fixed in `parallel_runtime.c`.

Plots: `parallel_boost.png`, `combo_boost.png`