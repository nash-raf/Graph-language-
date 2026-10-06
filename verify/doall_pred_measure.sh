#!/usr/bin/env bash
# DOALL predicted-vs-actual measurement.
#
# Two DOALL shapes:
#   doall_scaling  compute-bound `for each vertex` loop (the shape the suite's
#                  scaling check times); work is swept by the round count.
#   array_doall    trivial memory-bound loop, kept as a contrast series (its
#                  program time is startup-dominated, so no scaling is visible).
#
# The compiler emits no *time* prediction for these loops: the region cost
# model covers graph regions only, and the thread-budget allocator plans a
# *width*.  The prediction side below is therefore that model's own scaling
# law -- the planned width clamps to the work items, so the implied 4-thread
# time is t1/min(4, trips).  Answers at 1 and 4 threads must be identical.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
ulimit -s unlimited 2>/dev/null || true
mkdir -p "$R/bin/doall"
CSV="$R/bin/doall/doall_pred.csv"
echo "shape,work,t1_s,t4_s,pred4_s,speedup,pred_speedup,answers_equal" > "$CSV"

best(){ # threads -> min of 5 wall seconds (own clock: /usr/bin/time reports T=0.00 here)
  local thr="$1" b=99999 v s2 e2
  for i in 1 2 3 4 5; do
    s2=$(date +%s%N)
    ( cd "$C" && SGPL_NUM_THREADS="$thr" OMP_NUM_THREADS="$thr" ./final_program >/dev/null 2>&1 )
    e2=$(date +%s%N)
    v=$(python3 -c "print(f'{(int($e2)-int($s2))/1e9:.6f}')")
    b=$(python3 -c "print(min($b, ${v:-99999}))")
  done
  echo "$b"
}
ans(){ ( cd "$C" && SGPL_NUM_THREADS="$1" ./final_program 2>/dev/null | tr '\n' ' ' | sed 's/ *$//' ); }

run_case(){ # shape work graphfile trips
  local shape="$1" work="$2" graph="$3" trip="$4"
  ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
    SGPL_GPU_BACKEND=0 GRAPH_FILE="$graph" bash 03_run.sh ) >"$R/bin/doall/build.log" 2>&1
  [ -f "$C/final_program" ] || { echo "$shape/$work BUILD FAILED"; return; }
  local a1 a4 t1 t4 eq
  a1=$(ans 1); a4=$(ans 4)
  eq=$([ "$a1" = "$a4" ] && echo yes || echo NO)
  t1=$(best 1); t4=$(best 4)
  python3 - "$shape" "$work" "$t1" "$t4" "$eq" "$trip" >> "$CSV" <<'PY'
import sys
shape, work, t1, t4, eq, trip = sys.argv[1], sys.argv[2], float(sys.argv[3]), float(sys.argv[4]), sys.argv[5], int(sys.argv[6])
pred4 = t1 / min(4, trip)
print(f"{shape},{work},{t1},{t4},{pred4:.4f},{t1/max(1e-9,t4):.2f},{t1/max(1e-9,pred4):.2f},{eq}")
PY
  tail -1 "$CSV"
}

for r in 5 10 25 50 100 200; do
  g="$R/bin/doall/scaling_${r}rounds.graph"
  sed -E "s/while \(round < [0-9]+\)/while (round < $r)/" "$R/cases/parallel/doall_scaling.graph" > "$g"
  run_case doall_scaling "${r}x20000" "$g" $((r * 20000))
done
for n in 100000 1000000; do
  g="$R/bin/doall/array_$n.graph"
  sed -E "s/int n = [0-9]+;/int n = $n;/" "$R/cases/parallel/array_doall.graph" > "$g"
  run_case array_doall "$n" "$g" "$n"
done
