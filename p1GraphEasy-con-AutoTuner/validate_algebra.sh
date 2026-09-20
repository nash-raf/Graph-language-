#!/usr/bin/env bash
set -euo pipefail

# validate_algebra.sh — golden regression harness for the algebraic
# classification (Phase 3 cutover).
#
# The classification verdicts of the acceptance fixtures were locked into
# proof/algebra_golden_classify.txt at the cutover commit.  This script reruns
# every fixture through the prebuilt ./GraphProgram and requires the candidate
# lines (driver/inner/kind/class + effect set + temporal + compat) to match the
# golden file exactly.  A mismatch means the algebraic interpretation
# (interpretPar / deriveTemporalWorst / privLayout) changed behaviour.
#
# Environment: Linux.  Uses the prebuilt ./GraphProgram (rebuild it via the
# phase build if you change the compiler).  Run from this directory.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

GOLDEN="${GOLDEN:-proof/algebra_golden_classify.txt}"
[[ -f "$GOLDEN" ]] || { echo "missing golden file: $GOLDEN"; exit 1; }
[[ -x ./GraphProgram ]] || { echo "missing ./GraphProgram"; exit 1; }

FIXTURES=(
  ultimate_pagerank ultimate_kcore ultimate_sssp ultimate_cc ultimate_bfs_parent
  roundsep_sssp roundsep_min roundsep_add
  reduce_sum reduce_sub reduce_mul reduce_min reduce_max
  small_bfs small_sssp small_cc small_kcore
  dual_uv mixed_same_array cross_dep test_2phase
)

norm() {
  grep "candidate:" | sed 's/^\[graph-frontier\] candidate: //' | sed 's/  expr:.*//' || true
}

golden_block() {
  awk -v name="### $1.graph" '
    $0 == name { on = 1; next }
    /^### / { on = 0 }
    on { print }
  ' "$GOLDEN"
}

FAILURES=0
RUN=0
echo "=== algebraic classification golden check ==="
for f in "${FIXTURES[@]}"; do
  [ -f "$f.graph" ] || continue
  RUN=$((RUN + 1))
  EXPECTED="$(golden_block "$f")"
  ACTUAL="$(GRAPH_FRONTIER_STATS=1 ./GraphProgram "$f.graph" 2>&1 | norm)"
  if [[ "$EXPECTED" == "$ACTUAL" ]]; then
    echo "PASS $f"
  else
    echo "FAIL $f"
    diff <(echo "$EXPECTED") <(echo "$ACTUAL") | head -12
    FAILURES=$((FAILURES + 1))
  fi
done

echo "=== summary: $RUN fixtures, $FAILURES failures ==="
if [[ "$FAILURES" -eq 0 ]]; then
  echo "algebraic classification validation: PASS"
  exit 0
fi
echo "algebraic classification validation: FAIL ($FAILURES)"
exit 1
