#!/usr/bin/env bash
# For each fixture: build with FORCE_GPU=1, run with the device step forced and
# debug on, and report how many dispatches ran on the device plus the reasons the
# runtime refused the rest.
set -u
ulimit -s unlimited 2>/dev/null || true
export LD_LIBRARY_PATH=/usr/local/lib
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; P1="$ROOT/p1GraphEasy-con-AutoTuner"; cd "$P1"
for fx in "$@"; do
  rm -f final_program program.o gpu_runtime.o kernels.ptx
  if ! FORCE_GPU=1 GRAPH_FILE="$fx" timeout 400 bash 03_run.sh > /tmp/gp_build.log 2>&1; then
    echo "=== $(basename "$fx"): BUILD-FAIL ($(tail -1 /tmp/gp_build.log | cut -c1-60))"; continue
  fi
  SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program > /tmp/gp_run.txt 2>&1
  echo "=== $(basename "$fx"): device=$(grep -c 'ran on device' /tmp/gp_run.txt) routed=$(grep -c 'routed to the source-owned' /tmp/gp_run.txt) refused=$(grep -cE 'kept on the CPU|refus' /tmp/gp_run.txt) ans=$(grep -v '^\[' /tmp/gp_run.txt | head -1 | cut -c1-24)"
  grep -E 'refus|kept on the CPU|no device|routed to the source-owned' /tmp/gp_run.txt | sed 's/^\[gpu\] //' | sort | uniq -c | sort -rn | head -4
done
