#!/usr/bin/env bash
# DOALL loop: measured time at 1 and 4 threads across sizes.  The compiler
# emits no *time* prediction for these loops (the region cost model covers
# graph regions only; for array loops the budget allocator plans a *width*),
# so the prediction side here is that model's own scaling law: the planned
# width clamps to the work items, and the implied 4-thread time is
# t1/min(4, trips).  Emits CSV for plot_doall_pred.py.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
ulimit -s unlimited 2>/dev/null || true
mkdir -p "$R/bin/doall"
CSV="$R/bin/doall/doall_pred.csv"
echo "n,t1_ms,t4_ms,pred4_ms,speedup,pred_speedup" > "$CSV"
for n in 1000 4000 16000 65536 262144 1048576; do
  g="$R/bin/doall/doall_$n.graph"
  sed -E "s/int n = [0-9]+;/int n = $n;/" "$R/cases/parallel/array_doall.graph" > "$g"
  ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
    SGPL_GPU_BACKEND=0 GRAPH_FILE="$g" bash 03_run.sh ) >"$R/bin/doall/build.log" 2>&1
  [ -f "$C/final_program" ] || { echo "$n BUILD FAILED"; continue; }
  med(){ local thr="$1" t=() s e
    for i in 1 2 3 4 5; do
      s=$(date +%s%N); ( cd "$C" && SGPL_NUM_THREADS="$thr" ./final_program >/dev/null 2>&1 ); e=$(date +%s%N)
      t+=($(( (e-s)/1000 )))   # microseconds
    done
    printf '%s\n' "${t[@]}" | sort -n | sed -n 3p
  }
  t1=$(med 1); t4=$(med 4)
  python3 - "$n" "$t1" "$t4" >> "$CSV" <<'PY'
import sys
n, t1, t4 = int(sys.argv[1]), float(sys.argv[2]), float(sys.argv[3])
pred4 = t1 / min(4, n)
print(f"{n},{t1},{t4},{pred4:.0f},{t1/max(1,t4):.2f},{t1/max(1,pred4):.2f}")
PY
  tail -1 "$CSV"
done
