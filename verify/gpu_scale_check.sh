#!/usr/bin/env bash
# Scale check: the device paths must also agree with the CPU on a graph that is
# two orders of magnitude larger than the small fixtures (ia-dbpedia: 780132
# edges -> 1.56M arcs, 780k+ vertices), where the per-round state copies, the
# claim-map scan and the frontier arrays are all large.  Same discipline as the
# broad check: CPU baseline, then device forced, 1 and 4 threads, repeats.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
BIG="$R/../real_graphs/ia-dbpedia.txt"
ulimit -s unlimited 2>/dev/null || true
pass=0; fail=0
ok(){ printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass+1)); }
no(){ printf '  \033[31mFAIL\033[0m  %s\n' "$1"; fail=$((fail+1)); }
mkdir -p "$R/bin/scale"
[ -f "$BIG" ] || { echo "missing $BIG"; exit 1; }
FORCE="SGPL_GPU_ENGINE_MIN_PAIRS=0 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_MAX_WAVES=0 SGPL_FORCE_DOALL_PARALLEL=1"
for case in algo/kcore algo/bfs_level algo/pagerank; do
  # BFS on the directed dpbedia starts at a sink (reaches 1 vertex, a trivial
  # comparison); use the graph where it actually sweeps many frontier rounds.
  case_big="$BIG"
  [ "$case" = "algo/bfs_level" ] && case_big="$R/../real_graphs/bio-grid-yeast.txt"
  # the compiler resolves the file relative to the graph's own directory, so
  # rewrite the whole path (any prefix) to the absolute big-graph path
  sed -E "s|[^\" ]*fixtures/g20k\.txt|$case_big|" "$R/cases/$case.graph" > "$R/bin/scale/$(basename $case).graph"
  grep -q "$case_big" "$R/bin/scale/$(basename $case).graph" || { no "$case: fixture rewrite failed"; continue; }
  graph="$R/bin/scale/$(basename $case).graph"
  build(){ ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
             SGPL_GPU_BACKEND="$1" GRAPH_FILE="$graph" bash 03_run.sh ) >"$R/bin/scale/build.log" 2>&1 \
             && [ -f "$C/final_program" ]; }
  run(){ ( cd "$C" && env $1 SGPL_NUM_THREADS=$2 timeout 900 ./final_program 2>/dev/null </dev/null \
           | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }
  if ! build 0; then no "$case: CPU build failed"; continue; fi
  base=$(run "" 4)
  if ! build 1; then no "$case: GPU build failed"; continue; fi
  g1=$(run "$FORCE" 1); g4=$(run "$FORCE" 4); g4b=$(run "$FORCE" 4)
  if [ "$g1" != "$base" ]; then no "$case: device 1thr '$g1' != CPU '$base'"; continue; fi
  if [ "$g4" != "$base" ]; then no "$case: device 4thr '$g4' != CPU '$base'"; continue; fi
  if [ "$g4" != "$g4b" ]; then no "$case: repeat differs '$g4' vs '$g4b'"; continue; fi
  ok "$case on $(basename $case_big) (device 1=4=repeats == CPU: '$base')"
done
printf 'passed: %d  failed: %d\n' "$pass" "$fail"
[ "$fail" = 0 ]
