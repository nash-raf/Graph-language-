#!/usr/bin/env bash
# Remeasure ER + BA with guaranteed-fresh inserts (duplicate scan offline),
# then align predictions with cost_model.py and plot density ranking tables.
#
# Hypothesis: high-density insert mismatches were caused by random duplicate
# inserts (BCSR early-return).  Fresh-only workloads should eliminate that
# failure mode.  Density 1.0 is skipped (no non-edges exist).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

DATASET_ROOT="${DATASET_ROOT:-/home/kazisahib/sgpl_main/dataset}"
ER_DIR="${DATASET_ROOT}/erdos_renyi"
BA_DIR="${DATASET_ROOT}/barabasi_albert"
RUNS="${RUNS:-10}"
N_INSERTS="${N_INSERTS:-50}"
TIMEOUT_SEC="${TIMEOUT_SEC:-600}"

if [[ ! -d "$ER_DIR" ]]; then
  echo "error: missing ER edge folder: $ER_DIR" >&2
  exit 1
fi
if [[ ! -d "$BA_DIR" ]]; then
  echo "error: missing BA edge folder: $BA_DIR" >&2
  exit 1
fi

export TIMEOUT_SEC
# Reuse GraphProgram if already built; set SKIP_BUILD=0 to force rebuild.
export SKIP_BUILD="${SKIP_BUILD:-1}"

echo "=== Fresh-insert remasurement ==="
echo "  ER:        $ER_DIR"
echo "  BA:        $BA_DIR"
echo "  runs:      $RUNS (plus 1 warmup)"
echo "  n_inserts: $N_INSERTS (guaranteed non-duplicate)"
echo "  SKIP_BUILD=$SKIP_BUILD"
echo

echo ">>> [1/4] Erdős–Rényi"
python3 bench_folder.py "$ER_DIR" \
  --out erdos_renyi_runs10.csv \
  --runs "$RUNS" \
  --n-inserts "$N_INSERTS" \
  --timeout "$TIMEOUT_SEC"

echo
echo ">>> [2/4] Barabási–Albert"
python3 bench_folder.py "$BA_DIR" \
  --out barabasi_albert_runs10.csv \
  --runs "$RUNS" \
  --n-inserts "$N_INSERTS" \
  --timeout "$TIMEOUT_SEC"

echo
echo ">>> [3/4] Align predicted_ns with cost_model.py"
python3 update_predictions.py erdos_renyi_runs10.csv barabasi_albert_runs10.csv

echo
echo ">>> [4/4] Plot density ranking tables"
python3 plot_ranking_density.py

echo
echo "Done."
echo "  CSVs:  erdos_renyi_runs10.csv  barabasi_albert_runs10.csv"
echo "  Plots: erdos_renyi_density_ranking.png  barabasi_albert_density_ranking.png"
