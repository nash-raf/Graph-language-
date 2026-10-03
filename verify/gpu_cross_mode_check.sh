#!/usr/bin/env bash
# Cross-mode validation: every fixture whose loops the effect system
# parallelizes (or refuses) is built twice -- CPU-only IR backend and GPU IR
# backend -- and the observable outputs must agree bit-for-bit:
#
#   CPU 1thr == CPU 4thr == GPU 1thr (x2) == GPU 4thr (x2)
#
# The device paths are forced past the cost policy (min trips / min pairs /
# wave cap dropped) so the comparison actually exercises the kernels rather
# than the CPU fallback; with the policy in place the engine step would stay on
# the CPU for these small fixtures by design.  A disagreement between threads
# is a race or a wrong device realization; a disagreement between repeats is
# nondeterminism.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
ulimit -s unlimited 2>/dev/null || true
pass=0; fail=0; skip=0
ok(){ printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass+1)); }
no(){ printf '  \033[31mFAIL\033[0m  %s\n' "$1"; fail=$((fail+1)); }
sk(){ printf '  \033[33mSKIP\033[0m  %s\n' "$1"; skip=$((skip+1)); }

FORCE_COMMON="SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_MAX_WAVES=0 SGPL_GPU_ENGINE_MIN_PAIRS=0"
case_env(){ case "$1" in
  array_doall|gpu_compute|mixed_regions|reduce_int_ops|algo/pagerank|algo/cc|algo/kcore|algo/bfs_level)
    printf 'SGPL_FORCE_DOALL_PARALLEL=1' ;;
  doacross_carry|doacross_scan) printf 'SGPL_FORCE_DOACROSS_PARALLEL=1' ;;
  *) printf '' ;;
esac; }

build(){
  # bare names are parallel/ fixtures; path-like names are cases/<name>.graph
  local graph
  if [[ "$2" == */* ]]; then graph="$R/cases/$2.graph"; else graph="$R/cases/parallel/$2.graph"; fi
  ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
    SGPL_GPU_BACKEND="$1" GRAPH_FILE="$graph" bash 03_run.sh ) >"$R/bin/cm/$3.log" 2>&1
}
runcase(){
  ( cd "$C" && env $1 SGPL_NUM_THREADS=$2 ./final_program 2>/dev/null </dev/null \
      | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' )
}

mkdir -p "$R/bin/cm"
cases="array_doall gpu_compute doacross_carry doacross_scan reduce_int_ops mixed_regions algo/kcore algo/pagerank algo/bfs_level algo/cc algo/sssp"
for case in $cases; do
  cenv=$(case_env "$case")
  # CPU-only mode
  if ! build 0 "$case" "cpu_$(echo "$case" | tr / _)"; then no "$case: CPU-only build failed"; continue; fi
  cpu1=$(runcase "$cenv" 1); cpu4=$(runcase "$cenv" 4)
  # GPU mode, forced device execution, two repeats per thread count
  if ! build 1 "$case" "gpu_$(echo "$case" | tr / _)"; then no "$case: GPU build failed"; continue; fi
  g1a=$(runcase "$cenv $FORCE_COMMON" 1); g1b=$(runcase "$cenv $FORCE_COMMON" 1)
  g4a=$(runcase "$cenv $FORCE_COMMON" 4); g4b=$(runcase "$cenv $FORCE_COMMON" 4)
  if [ "$cpu1" != "$cpu4" ]; then
    no "$case: CPU 1thr '$cpu1' != 4thr '$cpu4'"; continue
  fi
  if [ "$g1a" != "$g1b" ] || [ "$g4a" != "$g4b" ] || [ "$g1a" != "$g4a" ]; then
    no "$case: device nondeterminism (1:'$g1a' 1r:'$g1b' 4:'$g4a' 4r:'$g4b')"; continue
  fi
  if [ "$g1a" != "$cpu1" ]; then
    no "$case: device '$g1a' != CPU '$cpu1'"; continue
  fi
  ok "$case (CPU 1=4, device 1=4=repeats, device==CPU: '$cpu1')"
done
printf 'passed: %d  failed: %d  skipped: %d\n' "$pass" "$fail" "$skip"
[ "$fail" = 0 ]
