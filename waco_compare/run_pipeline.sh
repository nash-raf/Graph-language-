#!/usr/bin/env bash
# End-to-end WACO vs AutoTuner comparison pipeline (pilot-scale by default).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
export GRAPH_DISABLE_POLLY=1
export FORCE_CPU=1
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-8}"
export SGPL_NUM_THREADS="${SGPL_NUM_THREADS:-8}"

LIMIT_GRAPHS="${LIMIT_GRAPHS:-48}"
LIMIT_CASES="${LIMIT_CASES:-24}"
RUNS="${RUNS:-3}"
PYTHON="${PYTHON:-python3}"

echo "=== [1/7] corpus ==="
$PYTHON corpus/build_corpus.py --limit "$LIMIT_GRAPHS" --reps 2

echo "=== [2/7] workloads ==="
$PYTHON workloads/instantiate.py --split train
$PYTHON workloads/instantiate.py --split test

echo "=== [3/7] ground truth (forced layouts) ==="
$PYTHON measure_regions.py --split train --runs "$RUNS" --warmup 1 --limit "$LIMIT_CASES" \
  --out results/ground_truth_train.csv
$PYTHON measure_regions.py --split test --runs "$RUNS" --warmup 1 --limit "$LIMIT_CASES" \
  --out results/ground_truth_test.csv
# merge
$PYTHON - <<'PY'
import csv
from pathlib import Path
rows=[]
keys=None
for p in [Path('results/ground_truth_train.csv'), Path('results/ground_truth_test.csv')]:
    if not p.is_file():
        continue
    with p.open() as f:
        r=csv.DictReader(f)
        keys=r.fieldnames
        rows.extend(list(r))
if rows:
    with open('results/ground_truth.csv','w',newline='') as f:
        w=csv.DictWriter(f, fieldnames=keys)
        w.writeheader(); w.writerows(rows)
    print('merged', len(rows), 'gt rows')
PY

echo "=== [4/7] AutoTuner predictions ==="
$PYTHON autotuner_predictions.py --split test --limit "$LIMIT_CASES" --out results/autotuner_pred.csv

echo "=== [5/7] WACO-G train + predict ==="
$PYTHON wacog/train.py --gt results/ground_truth.csv --epochs "${EPOCHS:-20}" --out results/wacog.pt
$PYTHON wacog/predict.py --gt results/ground_truth.csv --ckpt results/wacog.pt \
  --out results/wacog_pred.csv --plans results/wacog_plans.json

echo "=== [6/7] Track 2 overlap ==="
$PYTHON waco_artifact/run_track2.py --gt results/ground_truth.csv --out results/waco_artifact_pred.csv

echo "=== [7/7] analysis ==="
$PYTHON analysis/compare.py \
  --gt results/ground_truth.csv \
  --at results/autotuner_pred.csv \
  --wacog results/wacog_pred.csv \
  --art results/waco_artifact_pred.csv \
  --out results/comparison_report.json \
  --md results/comparison_report.md

echo "DONE. See results/comparison_report.md"
