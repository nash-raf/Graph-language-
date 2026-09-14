#!/usr/bin/env bash
# Loop verdict census: compile every verify case with the lowering and PDG
# diagnostics on and print the per-loop verdicts, so "the suite is green"
# can be checked against "we are still parallelising the loops we mean to".
#
#   ./class_census.sh                 all cases
#   ./class_census.sh parallel        one subtree (cases/<arg>)
#   ./class_census.sh parallel/roundsep.graph   a single case file
#
# Output per case:
#   [graph-frontier] candidate: <fn> driver=<loop> ... fw=<0/1> env=<0/1>
#                             shadow=<n> class=<source-owner|dest-owner|
#                             reduction|dual-owner|sequential>
#   [loop-classify] <fn> <loop> ... class=<DOALL|DOACROSS|SEQUENTIAL>
# Compiles the current compiler tree read-only (writes only build products).
set -uo pipefail
R="$(cd "$(dirname "$0")" && pwd)"
C="$R/../p1GraphEasy-con-AutoTuner"
export LD_LIBRARY_PATH="$C/.deps/nlopt/lib:$C/.deps/nlopt/lib64:${LD_LIBRARY_PATH:-}"
ulimit -s unlimited 2>/dev/null
export SGPL_FRONTIER_STRICT=1

SEL="${1:-}"
if [[ -z "$SEL" ]]; then
  FILES=("$R"/cases/*/*.graph)
elif [[ -f "$R/cases/$SEL" ]]; then           # path relative to cases/ , e.g. parallel/roundsep.graph
  FILES=("$R/cases/$SEL")
elif [[ -f "$SEL" ]]; then                    # path as given
  FILES=("$SEL")
elif [[ -d "$R/cases/$SEL" ]]; then           # subtree, e.g. parallel
  FILES=("$R"/cases/"$SEL"/*.graph)
else                                          # bare case name, e.g. roundsep
  FILES=("$R"/cases/*/"$SEL".graph "$R"/cases/*/"$SEL")
fi

for f in "${FILES[@]}"; do
  [[ -f "$f" ]] || { echo "--- (missing) $f"; continue; }
  name="${f#"$R"/cases/}"; name="${name%.graph}"
  out=$( cd "$C" && GRAPH_FILE="$f" GRAPH_FRONTIER_STATS=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
         bash ./03_run.sh 2>&1 </dev/null )
  rc=$?
  echo "--- $name (rc=$rc)"
  if ! grep -qE '\[graph-frontier\]|\[loop-classify\]' <<<"$out"; then
    echo "    (no diagnostics: build failed, or no graph loop in this program)"
    grep -iE 'error|abort' <<<"$out" | head -3 | sed 's/^/    /'
    continue
  fi
  grep -E '\[graph-frontier\] candidate|\[graph-frontier\]   modelable|\[loop-classify\]' <<<"$out" \
    | sed 's/^/    /'
done
