#!/usr/bin/env bash
set -euo pipefail

# validate_refactor.sh — compatibility gate for the effect-algebra refactor.
#
# Regenerates the witness/statistics dump for every fixture and requires it to
# be byte-identical to proof/refactor_golden/*.txt.  Also regenerates the
# emitted-IR goldens for the representative subset and requires byte equality.
# This is the "emission and runtime unchanged" check: the goldens were captured
# before the refactor and must survive it.
#
# Run from this directory with the prebuilt ./GraphProgram.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

GOLDEN=proof/refactor_golden
[[ -d "$GOLDEN" ]] || { echo "missing golden dir: $GOLDEN"; exit 1; }
[[ -x ./GraphProgram ]] || { echo "missing ./GraphProgram"; exit 1; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

norm_stats() { grep -E '^\[graph-frontier\]|^\[witness\]' || true; }

FAILURES=0

echo "=== [1/4] acceptance fixtures (witness + stats) ==="
: > "$TMP/acceptance.txt"
ACCEPTANCE=(
  ultimate_pagerank ultimate_kcore ultimate_sssp ultimate_cc ultimate_bfs_parent
  roundsep_sssp roundsep_min roundsep_add
  reduce_sum reduce_sub reduce_mul reduce_min reduce_max
  small_bfs small_sssp small_cc small_kcore
  dual_uv mixed_same_array cross_dep test_2phase
)
for f in "${ACCEPTANCE[@]}"; do
  [ -f "$f.graph" ] || continue
  echo "### $f.graph" >> "$TMP/acceptance.txt"
  SGPL_WITNESS_DUMP=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram "$f.graph" 2>&1 >/dev/null \
    | norm_stats >> "$TMP/acceptance.txt"
done
if diff -q "$GOLDEN/acceptance.txt" "$TMP/acceptance.txt" > /dev/null; then
  echo "PASS acceptance"
else
  echo "FAIL acceptance"
  diff "$GOLDEN/acceptance.txt" "$TMP/acceptance.txt" | head -30
  FAILURES=$((FAILURES + 1))
fi

echo "=== [2/4] all small fixtures (witness + stats) ==="
: > "$TMP/small_fixtures.txt"
for f in *.graph; do
  sz=$(stat -c %s "$f")
  [ "$sz" -gt 2000000 ] && continue
  echo "### $f" >> "$TMP/small_fixtures.txt"
  SGPL_WITNESS_DUMP=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram "$f" 2>&1 >/dev/null \
    | norm_stats >> "$TMP/small_fixtures.txt"
done
if diff -q "$GOLDEN/small_fixtures.txt" "$TMP/small_fixtures.txt" > /dev/null; then
  echo "PASS small fixtures"
else
  echo "FAIL small fixtures"
  diff "$GOLDEN/small_fixtures.txt" "$TMP/small_fixtures.txt" | head -30
  FAILURES=$((FAILURES + 1))
fi

echo "=== [3/4] independence checks (acyclic dependency invariant) ==="
SRC=graph_frontier_lowering.cpp
if awk '/^static void deriveAllTemporal/,/^}/' "$SRC" | grep -qE 'RoundSepBases|VSource'; then
  echo "FAIL tau consults RoundSepBases/sigma"
  FAILURES=$((FAILURES + 1))
else
  echo "PASS tau excludes sigma/RoundSepBases"
fi
if awk '/^static Temporal relateReadWrite/,/^}/' "$SRC" | grep -qE 'RoundSepBases|VSource'; then
  echo "FAIL relateReadWrite consults RoundSepBases/sigma"
  FAILURES=$((FAILURES + 1))
else
  echo "PASS relateReadWrite excludes sigma/RoundSepBases"
fi
if awk '/Round-separation bases \(composition A\)/,/Value source/' "$SRC" | grep -qE '\.Temp|->Temp'; then
  echo "FAIL RoundSep consults tau"
  FAILURES=$((FAILURES + 1))
else
  echo "PASS RoundSep excludes tau"
fi
if awk '/Value source .*assigned after round separation/,/^}/' "$SRC" | grep -qE '\.Temp|->Temp'; then
  echo "FAIL sigma assignment consults tau"
  FAILURES=$((FAILURES + 1))
else
  echo "PASS sigma assignment excludes tau"
fi

echo "=== [4/4] emitted IR subset ==="
IR_SUBSET=(ultimate_pagerank ultimate_kcore ultimate_sssp ultimate_cc roundsep_sssp reduce_sum dual_uv)
for f in "${IR_SUBSET[@]}"; do
  [ -f "$GOLDEN/ir_$f.ll" ] || continue
  GRAPH_FRONTIER_DUMP=1 ./GraphProgram "$f.graph" > /dev/null 2>&1 || true
  if diff -q "$GOLDEN/ir_$f.ll" /tmp/post_emit_module.ll > /dev/null 2>&1; then
    echo "PASS ir_$f"
  else
    echo "FAIL ir_$f"
    diff "$GOLDEN/ir_$f.ll" /tmp/post_emit_module.ll | head -20
    FAILURES=$((FAILURES + 1))
  fi
done

echo "=== summary: $FAILURES failures ==="
if [[ "$FAILURES" -eq 0 ]]; then
  echo "refactor compatibility validation: PASS"
  exit 0
fi
echo "refactor compatibility validation: FAIL ($FAILURES)"
exit 1
