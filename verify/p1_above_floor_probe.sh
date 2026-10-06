#!/usr/bin/env bash
# Above-the-floor engine-step check: the cost gate (SGPL_GPU_ENGINE_MIN_PAIRS,
# default 2M arcs) keeps engine steps on the CPU below the floor -- a region we
# have measured -- and admits them above it, a region we had never measured.
# This probe builds BFS on a graph whose step work is above the floor
# (synth_v_2000000_e_16000000: 16M edges -> ~32M arcs) and compares the device
# step against the CPU step, medians of 3, with the answers compared and the
# device execution confirmed from the debug log.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
GRAPH="${1:-/home/user/Course/msk1/synth_graphs_unweighted/synth_v_2000000_e_16000000.txt}"
ulimit -s unlimited 2>/dev/null || true
mkdir -p "$R/bin/floor"

CASE="$R/cases/algo/bfs_level.graph"   # the engine-step (activation) shape
[ -f "$CASE" ] || CASE="$R/algo_validation/bfs_level.graph"
out="$R/bin/floor/bfs_big.graph"
sed -E "s|[^\" ]*\.txt|$GRAPH|" "$CASE" > "$out"
grep -q "$(basename "$GRAPH")" "$out" || { echo "fixture rewrite failed"; exit 1; }

echo "=== building on $(basename "$GRAPH") ($(wc -l < "$GRAPH") edges)"
( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
  SGPL_GPU_BACKEND=1 GRAPH_FILE="$out" bash 03_run.sh ) >"$R/bin/floor/build.log" 2>&1
if [ ! -f "$C/final_program" ]; then echo "BUILD FAILED"; tail -5 "$R/bin/floor/build.log"; exit 1; fi

t3(){
  local ev="$1" t=() s e
  for i in 1 2 3; do
    s=$(date +%s%N)
    ( cd "$C" && env $ev SGPL_NUM_THREADS=4 timeout 1800 ./final_program ) >"$R/bin/floor/out.txt" 2>"$R/bin/floor/err.txt"
    e=$(date +%s%N); t+=($(( (e-s)/1000000 )))
  done
  printf '%s\n' "${t[@]}" | sort -n | sed -n 2p
}

cpu=$(t3 "SGPL_NO_GPU_ENGINE_STEP=1")
anscpu=$(tr '\n' ' ' < "$R/bin/floor/out.txt" | sed 's/ *$//')
row=$(t3 "SGPL_GPU_DEBUG=1 SGPL_GPU_ENGINE_MIN_PAIRS=0")
ansrow=$(tr '\n' ' ' < "$R/bin/floor/out.txt" | sed 's/ *$//')
rowsteps=$(grep -cE 'step ran on device' "$R/bin/floor/err.txt")
pull=$(t3 "SGPL_GPU_DEBUG=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 SGPL_GPU_ACTIVATION_PULL=1")
anspull=$(tr '\n' ' ' < "$R/bin/floor/out.txt" | sed 's/ *$//')
pullsteps=$(grep -cE 'step ran on device' "$R/bin/floor/err.txt")
echo "cpu        : ${cpu}ms  '$anscpu'"
echo "device row : ${row}ms  '$ansrow'  (device steps: $rowsteps)"
echo "device pull: ${pull}ms  '$anspull'  (device steps: $pullsteps)"
echo "ratios     : cpu/row=$(python3 -c "print('%.2f' % ($cpu/max(1,$row)))")  cpu/pull=$(python3 -c "print('%.2f' % ($cpu/max(1,$pull)))")  (>1 device faster)"
if [ "$anscpu" = "$ansrow" ] && [ "$anscpu" = "$anspull" ]; then echo "answers: identical"; else echo "answers: MISMATCH"; fi

# confirm the device actually executed the step above the floor
( cd "$C" && SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 timeout 1800 ./final_program ) >/dev/null 2>"$R/bin/floor/dbg.txt"
echo "device steps logged: $(grep -cE 'activation step ran|engine step ran' "$R/bin/floor/dbg.txt")"
grep -m2 -E "activation step ran|engine step ran|cost model|min_pairs" "$R/bin/floor/dbg.txt"
