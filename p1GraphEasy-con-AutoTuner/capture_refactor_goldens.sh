#!/usr/bin/env bash
# capture_refactor_goldens.sh — Stage G golden capture for the effect-algebra
# refactor.  Records, per fixture, the emission-relevant witness dump and the
# candidate/statistics lines.  The captured files are the compatibility oracle
# for Stages A/B/C (see validate_refactor.sh).
#
# Emitted-IR goldens are captured for a representative subset (IR files are
# large); the acceptance suites cover end-to-end behavior.
set -euo pipefail
cd /mnt/d/sgpl/Graph-language-/p1GraphEasy-con-AutoTuner

OUTDIR=proof/refactor_golden
mkdir -p "$OUTDIR"

ACCEPTANCE=(
  ultimate_pagerank ultimate_kcore ultimate_sssp ultimate_cc ultimate_bfs_parent
  roundsep_sssp roundsep_min roundsep_add
  reduce_sum reduce_sub reduce_mul reduce_min reduce_max
  small_bfs small_sssp small_cc small_kcore
  dual_uv mixed_same_array cross_dep test_2phase
)
IR_SUBSET=(ultimate_pagerank ultimate_kcore ultimate_sssp ultimate_cc roundsep_sssp reduce_sum dual_uv)

norm_stats() { grep -E '^\[graph-frontier\]|^\[witness\]' || true; }

: > "$OUTDIR/acceptance.txt"
for f in "${ACCEPTANCE[@]}"; do
  [ -f "$f.graph" ] || { echo "MISSING $f.graph" >> "$OUTDIR/acceptance.txt"; continue; }
  echo "### $f.graph" >> "$OUTDIR/acceptance.txt"
  SGPL_WITNESS_DUMP=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram "$f.graph" 2>&1 >/dev/null \
    | norm_stats >> "$OUTDIR/acceptance.txt"
done

: > "$OUTDIR/small_fixtures.txt"
for f in *.graph; do
  sz=$(stat -c %s "$f")
  [ "$sz" -gt 2000000 ] && continue
  echo "### $f" >> "$OUTDIR/small_fixtures.txt"
  SGPL_WITNESS_DUMP=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram "$f" 2>&1 >/dev/null \
    | norm_stats >> "$OUTDIR/small_fixtures.txt"
done

for f in "${IR_SUBSET[@]}"; do
  GRAPH_FRONTIER_DUMP=1 ./GraphProgram "$f.graph" > /dev/null 2>&1 || true
  if [ -f /tmp/post_emit_module.ll ]; then
    cp /tmp/post_emit_module.ll "$OUTDIR/ir_$f.ll"
  fi
done

echo "goldens written to $OUTDIR"
wc -l "$OUTDIR/acceptance.txt" "$OUTDIR/small_fixtures.txt"
ls "$OUTDIR" | head -20
