#!/usr/bin/env bash
# §4.3/§6 two-graph validator over the whole corpus.
#
# For every fixture and every candidate loop it records the proof-graph verdict
# next to the witness certificate of the same candidate:
#
#   cover_S / cover_T   soundness gate: a witness that fires is sound, so the
#                       graph must never be CLEANER than the certificate
#                       (cert dirty => graph dirty).  Any cover=0 is a missing
#                       graph rule and fails the run.
#   stricter_S/_T       completeness delta: certificate clean but the graph is
#                       dirty — a candidate the witness shortcut would have
#                       admitted while the complete relation/graph proof does
#                       not ("no witness remains => parallelize" is unsound in
#                       principle).  These are the counterexamples to the
#                       exhaustiveness of R1-R7 and are listed, not silenced.
#
# Usage: bash verify/check_graphs.sh [outfile]   (default verify/bin/graph_check.txt)
set -uo pipefail
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
OUT="${1:-$R/bin/graph_check.txt}"
mkdir -p "$R/bin"
: > "$OUT"

cd "$C" || exit 1
emit() {
  local f="$1"; rm -f program.o
  SGPL_STRICT=0 timeout 300 bash -c "GRAPH_FRONTIER_REWRITE=1 GRAPH_FRONTIER_STATS=1 \
      GRAPH_FRONTIER_GRAPHS=1 ./GraphProgram --ir-backend=cpu $(printf '%q' "$f")" \
      >/dev/null 2>"$f.err" || true
  local n
  n=$(grep -c '^\[graphcheck\] hdr=' "$f.err" 2>/dev/null || true)
  [ "${n:-0}" -eq 0 ] && { rm -f "$f.err"; return; }
  echo "=== $(basename "$f")" >> "$OUT"
  grep '^\[graphcheck\] hdr=' "$f.err" \
      | sed -E 's/^\[graphcheck\] //; s/ phase\(.*$//' >> "$OUT"
  grep '^\[graphcheck\]   ' "$f.err" | sed 's/^\[graphcheck\]//' >> "$OUT"
  rm -f "$f.err"
}

for f in "$R"/cases/*/*.graph; do emit "$f"; done
for f in "$C"/*.graph; do emit "$f"; done

CAND=$(grep -c '^hdr=' "$OUT" || true)
COVER_S=$(grep -c 'cover_S=1' "$OUT" || true)
COVER_T=$(grep -c 'cover_T=1' "$OUT" || true)
STRICT_S=$(grep -c 'stricter_S=1' "$OUT" || true)
STRICT_T=$(grep -c 'stricter_T=1' "$OUT" || true)
{
  echo "graph-check: candidates=$CAND cover_S=$COVER_S/$CAND cover_T=$COVER_T/$CAND stricter_S=$STRICT_S stricter_T=$STRICT_T"
  if [ "$STRICT_S" -ne 0 ] || [ "$STRICT_T" -ne 0 ]; then
    echo "strictness deltas (certificate clean, complete graph dirty):"
    awk '/^=== /{fx=$2} /^hdr=/{if ($0 ~ /stricter_S=1/ || $0 ~ /stricter_T=1/) print "  "fx": "$0}' "$OUT"
  fi
  if [ "$COVER_S" -ne "$CAND" ] || [ "$COVER_T" -ne "$CAND" ]; then
    echo "COVER FAILURES (witness fires, graph clean -> missing graph rule):"
    awk '/^=== /{fx=$2} /^hdr=/{if ($0 ~ /cover_S=0/ || $0 ~ /cover_T=0/) print "  "fx": "$0}' "$OUT"
  fi
} | tee -a "$OUT"
[ "$COVER_S" -eq "$CAND" ] && [ "$COVER_T" -eq "$CAND" ]
