#!/usr/bin/env bash
# Certificate census: compile every fixture with the rewrite on and record the
# full two-axis certificate -- axis summary, every witness (id/axis/base/relation/
# discharge), every schedule (kind/emitted/impl_failure/reason) and the class.
# This is the §19 evidence table: it shows which witness each fixture exercises.
#
# Usage: bash verify/sweep_certificates.sh [outfile]
#   default outfile: verify/bin/certificate_census.txt
set -uo pipefail
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
OUT="${1:-$R/bin/certificate_census.txt}"
: > "$OUT"

cd "$C" || exit 1
emit() {
  local f="$1"; rm -f program.o
  SGPL_STRICT=0 timeout 300 bash -c "GRAPH_FRONTIER_REWRITE=1 GRAPH_FRONTIER_STATS=1 \
      ./GraphProgram --ir-backend=cpu $(printf '%q' "$f")" >/dev/null 2>"$f.err" || true
  echo "=== $(basename "$f")" >> "$OUT"
  grep -oE "R_S=[0-9] R_T=[0-9]" "$f.err" | sort -u | sed 's/^/axes /' >> "$OUT"
  grep -oE "schedule=[a-z-]+ emitted=[0-9] impl_failure=[0-9]" "$f.err" | sort -u \
      | sed 's/^/sched /' >> "$OUT"
  grep -oE "reason=[^ ]+( [^ \"]+){0,6}" "$f.err" | sort -u | head -6 | sed 's/^/  why /' >> "$OUT"
  grep -oE "#[0-9]+ R[0-9] (spatial|temporal).*discharge=[a-z-]+" "$f.err" \
      | sed 's/ reason=.*discharge/ discharge/' | sort -u | sed 's/^/witness /' >> "$OUT"
  grep -oE "class=[a-z-]+ .*temporal=[A-Za-z]+" "$f.err" | head -1 | sed 's/^/class /' >> "$OUT"
  rm -f "$f.err"
}

# repo fixtures (verify/cases) plus the compiler-dir regression fixtures
for f in "$R"/cases/*/*.graph; do emit "$f"; done
for f in "$C"/*.graph; do emit "$f"; done
echo "census written to $OUT: $(grep -c '^===' "$OUT") fixtures"
