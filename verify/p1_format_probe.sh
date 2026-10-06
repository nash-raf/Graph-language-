#!/usr/bin/env bash
# p1 layout/format probe: build and run one case under each layout, once with
# the cost model choosing (default) and once per forced layout.  Reports the
# median wall time of 3 runs, the answer (must be identical across layouts),
# and the runtime's own conversion accounting.
#
# Usage: bash p1_format_probe.sh <case.graph> [graph-file-override] [tag]
#   AUTOTUNER_FORCE_LAYOUT is the p1 switch (CSR|PCSR|BCSR|SET).
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
CASE="${1:?usage: p1_format_probe.sh <case.graph> [graph] [tag]}"
case "$CASE" in /*) ;; *) CASE="$PWD/$CASE" ;; esac
[ -f "$CASE" ] || { echo "no such case: $CASE"; exit 1; }
GRAPH_OVERRIDE="${2:-}"
TAG="${3:-$(basename "$CASE" .graph)}"
ulimit -s unlimited 2>/dev/null || true
mkdir -p "$R/bin/fmt"

graph="$CASE"
if [ -n "$GRAPH_OVERRIDE" ]; then
  graph="$R/bin/fmt/${TAG}_$(basename "$GRAPH_OVERRIDE")"
  sed -E "s|[^\" ]*\.txt|$GRAPH_OVERRIDE|" "$CASE" > "$graph"
  grep -q "$(basename "$GRAPH_OVERRIDE")" "$graph" || { echo "$TAG: graph rewrite failed"; exit 1; }
fi

t3(){ local L="$1" t=() s e
  for i in 1 2 3; do
    s=$(date +%s%N)
    ( cd "$C" && if [ -n "$L" ]; then AUTOTUNER_FORCE_LAYOUT="$L" SGPL_NUM_THREADS=4 timeout 600 ./final_program; \
                   else SGPL_NUM_THREADS=4 timeout 600 ./final_program; fi ) >"$R/bin/fmt/out.txt" 2>"$R/bin/fmt/err.txt"
    e=$(date +%s%N); t+=($(( (e-s)/1000000 )))
  done
  printf '%s\n' "${t[@]}" | sort -n | sed -n 2p
}

for L in "" CSR PCSR BCSR SET; do
  # AUTOTUNER_FORCE_LAYOUT is read by the *compiler* (AutoTunerPass), so the
  # forced mode must be set at build time; run time only needs the answer/timing.
  ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
    env ${L:+AUTOTUNER_FORCE_LAYOUT=$L} SGPL_GPU_BACKEND=0 GRAPH_FILE="$graph" bash 03_run.sh ) >"$R/bin/fmt/build.log" 2>&1
  if [ ! -f "$C/final_program" ]; then echo "$TAG ${L:-cost-model}: BUILD FAILED"; continue; fi
  ms=$(t3 "$L")
  ans=$(tr '\n' ' ' < "$R/bin/fmt/out.txt" | sed 's/ *$//')
  conv=$(grep -oE "injected [0-9]+ layout conversions total, conversion_ns=[0-9]+" "$R/bin/fmt/err.txt" | head -1)
  printf '%s,%s,%s,%s,%s\n' "$TAG" "${L:-cost-model}" "$ms" "$ans" "$conv" >> "$R/bin/fmt/results.csv"
  echo "$TAG ${L:-cost-model}: ${ms}ms ans='$ans' ${conv:+| $conv}"
done
