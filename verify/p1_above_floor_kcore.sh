#!/usr/bin/env bash
# Above-the-floor engine-step measurement on a SOURCE-OWNED step (kcore's
# degree/peel step) over a graph whose step work is far above the floor
# (synth_v_2000000_e_16000000: 16M edges -> ~32M arcs).  Arms: CPU partitions,
# device row kernel, device pull kernel; medians of 3, answers compared.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
GRAPH="${1:-/home/user/Course/msk1/synth_graphs_unweighted/synth_v_2000000_e_16000000.txt}"
ulimit -s unlimited 2>/dev/null || true
mkdir -p "$R/bin/floor"
out="$R/bin/floor/kcore_big.graph"
sed -E "s|[^\" ]*\.txt|$GRAPH|" "$R/cases/algo/kcore.graph" > "$out"
grep -q "$(basename "$GRAPH")" "$out" || { echo "rewrite failed"; exit 1; }

echo "=== building kcore on $(basename "$GRAPH") ($(wc -l < "$GRAPH") edges)"
( cd "$C" && rm -f final_program program.o gpu_runtime.o && SGPL_GPU_BACKEND=1 GRAPH_FILE="$out" bash 03_run.sh ) \
  >"$R/bin/floor/kbuild.log" 2>&1
[ -f "$C/final_program" ] || { echo "BUILD FAILED"; grep -E "error:" "$R/bin/floor/kbuild.log" | head -3; exit 1; }

t3(){
  local ev="$1" i s e t=()
  for i in 1 2 3; do
    s=$(date +%s%N)
    ( cd "$C" && env $ev SGPL_NUM_THREADS=4 timeout 1800 ./final_program ) >"$R/bin/floor/kout.txt" 2>"$R/bin/floor/kerr.txt"
    e=$(date +%s%N); t+=($(( (e-s)/1000000 )))
  done
  printf '%s\n' "${t[@]}" | sort -n | sed -n 2p
}

cpu=$(t3 "SGPL_NO_GPU_ENGINE_STEP=1");  a1=$(tr '\n' ' ' < "$R/bin/floor/kout.txt" | sed 's/ *$//')
row=$(t3 "SGPL_GPU_DEBUG=1");           a2=$(tr '\n' ' ' < "$R/bin/floor/kout.txt" | sed 's/ *$//'); r1=$(grep -c "step ran on device" "$R/bin/floor/kerr.txt")
pull=$(t3 "SGPL_GPU_DEBUG=1 SGPL_GPU_ACTIVATION_PULL=1"); a3=$(tr '\n' ' ' < "$R/bin/floor/kout.txt" | sed 's/ *$//'); r2=$(grep -c "step ran on device" "$R/bin/floor/kerr.txt")
echo "cpu        : ${cpu}ms  '$a1'"
echo "device row : ${row}ms  '$a2'   (device steps: $r1)"
echo "device pull: ${pull}ms  '$a3'   (device steps: $r2)"
echo "ratios     : cpu/row $(python3 -c "print('%.2f' % ($cpu/max(1,$row)))")   cpu/pull $(python3 -c "print('%.2f' % ($cpu/max(1,$pull)))")"
[ "$a1" = "$a2" ] && [ "$a1" = "$a3" ] && echo "answers: identical" || echo "answers: MISMATCH"
grep -m2 -E "activation step ran|engine step ran|kept on the CPU" "$R/bin/floor/kerr.txt" || true
