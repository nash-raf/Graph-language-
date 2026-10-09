#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd -P)"
SUITE="$ROOT/test/pdg_tdg_verification"
cd "$ROOT"

mode="${1:---full}"
case "$mode" in
  --focused|--full) ;;
  *) echo "usage: bash $SUITE/run_all.sh [--focused|--full]" >&2; exit 2 ;;
esac

bash "$SUITE/build_tdg_validation.sh"
bash "$SUITE/run_pdg_argmem_expansion.sh"
bash "$SUITE/run_tdg_formal.sh"
bash "$SUITE/run_tdg_shared_graph_levels.sh"
python3 "$SUITE/test_autotuner_frequency.py"
python3 "$SUITE/test_layout_cost_model.py"
for script in run_exec_engine_tests.sh run_exec_r2_tests.sh \
              run_frontier_motif_tests.sh run_frontier_owner_pull_tests.sh \
              run_frontier_red_tests.sh run_frontier_shadow_tests.sh; do
  bash "$SUITE/$script"
done

if [[ "$mode" == --full ]]; then
  # These established repository-wide suites exercise the effect algebra,
  # emitted expression, and TDG budget used by the final PDG/TDG changes.
  # Their shared historical paths remain in place for other workflows.
  for script in validate_algebra.sh validate_totality.sh validate_rt_expr.sh \
                validate_reduction.sh validate_roundsep.sh validate_composition.sh \
                validate_tdg_budget.sh; do
    bash "$script"
  done
  python3 "$SUITE/audit_pdg_argmem_calls.py" \
    "$ROOT/build_tdg_validation/GraphProgram_tdg"
fi

echo "PASS PDG/TDG verification suite ($mode)"
