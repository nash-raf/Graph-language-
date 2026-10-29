# WACO vs ARS AutoTuner — region-level prediction comparison

Apple-to-apple comparison of **WACO** (ASPLOS'23) and the **ARS AutoTuner** in
`p1GraphEasy-con-AutoTuner/`. Both tools answer the same question on the same
regions, inputs, candidate layouts, and hardware, scored against measured
ground truth.

## Comparison rules

| Axis | Choice |
|------|--------|
| Regions | AutoTuner Traverse/Insert regions (SetQuery/CSRQuery excluded) |
| Candidates (Track 1) | CSR, PCSR, BCSR, SET |
| Candidates (Track 2) | CSR vs BCSR (WACO UC / UCU·UCUU overlap) |
| Hardware | This machine (Ryzen 7 4800H), 8 threads, `gcc`, `GRAPH_DISABLE_POLLY=1` |
| Search | Exhaustive over 4 layouts (ANNS unnecessary) |
| Ground truth | Median of ≥3 warm runs; within 5% of fastest counts as best |
| Scoring | Kernel-only (`operationCost` / WACO-G score) and end-to-end DP plans |

## Tracks

1. **WACO-G** — WACO's method retrained on AutoTuner's layout space (occupancy
   CNN + region embedder, pairwise hinge ranking). Covers all decision regions.
2. **WACO artifact overlap** — CSR/BCSR classifier on read-only traverse
   regions (SpMV-like), with optional SuperSchedule format mapping when
   artifact top-k files exist.

## Instrumentation (env-gated, default unchanged)

| Env | Effect |
|-----|--------|
| `AUTOTUNER_PREDICTION_DUMP=path` | JSONL per-region costs + local/DP choice |
| `AUTOTUNER_PROFILE_DUMP=path` | JSONL measured region ns (conversion separate) |
| `AUTOTUNER_LAYOUT_PLAN=r0:CSR,r1:BCSR,...` | Force an end-to-end layout plan |
| `AUTOTUNER_FORCE_LAYOUT=CSR\|PCSR\|BCSR\|SET` | Existing forced-layout instrument |

## Quick start (pilot)

```bash
# Rebuild compiler once — see HANDOFF.md for full deps / antlr regen
cd ../p1GraphEasy-con-AutoTuner
LLVM_CONFIG=llvm-config-20 POLLY_LIBS="-lPolly -lPollyISL" \
  GRAPH_DISABLE_POLLY=1 FORCE_CPU=1 bash build_lowmem.sh

cd ../waco_compare
chmod +x run_pipeline.sh
LIMIT_GRAPHS=48 LIMIT_CASES=16 RUNS=3 EPOCHS=15 ./run_pipeline.sh
```

Results land in `results/comparison_report.md` and `results/comparison_report.json`.

**Full-scale run, training guidance, and friend handoff:** see
[`HANDOFF.md`](HANDOFF.md).

## Layout

```
waco_compare/
  corpus/build_corpus.py
  workloads/{templates,instantiate.py}
  measure_regions.py
  autotuner_predictions.py
  wacog/{model,train,predict}.py
  waco_artifact/run_track2.py
  analysis/compare.py
  third_party/Workload-Aware-Co-Optimization/   # upstream artifact clone
  results/
```

## Dependencies

- System: `g++`, `llvm-20`, `antlr4-cpp-runtime`, `libPolly`, `nlopt` (`.deps/nlopt`)
- Python: `numpy`, `torch` (system site-packages OK), `scipy`/`pandas` optional

MinkowskiEngine is **not** required; WACO-G uses a WACONet-inspired CNN
fallback documented under threats to validity.
