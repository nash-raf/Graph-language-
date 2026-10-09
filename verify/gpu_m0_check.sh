#!/usr/bin/env bash
# GPU port M0: device-step differential (BFS fixture).
# Build with the NVPTX toolchain (FORCE_GPU=1 -> device kernels -> kernels.ptx),
# then run with the device step on (SGPL_GPU_ENGINE_STEP=1, min-pairs 0) and off
# (SGPL_NO_GPU_ENGINE_STEP=1) and compare the answers.
set -u
ulimit -s unlimited 2>/dev/null || true   # generated frames can exceed the 8 MB default
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
P1="$ROOT/p1GraphEasy-con-AutoTuner"
export PATH="${LLVM_NVPTX_PREFIX:-/workspace/llvm-20.1.8-nvptx}/bin:$PATH"
cd "$P1"
rm -f final_program program.o gpu_runtime.o kernels.ptx
FORCE_GPU=1 GRAPH_FILE="$ROOT/verify/cases/algo/bfs_level.graph" timeout 500 bash 03_run.sh > /tmp/m0_build.log 2>&1
if ! [ -x final_program ]; then echo "BUILD FAILED"; tail -6 /tmp/m0_build.log; exit 1; fi
echo "kernels.ptx: $(ls -la kernels.ptx 2>/dev/null | awk '{print $5" bytes"}' || echo MISSING)"
grep -c "NVPTX target unavailable" /tmp/m0_build.log | sed 's/^/nvptx-unavailable lines: /'
for P in 1 4 8; do
  SGPL_NUM_THREADS=$P ./final_program 2>&1 | grep -v AutoTunerProfile > /tmp/m0_off_$P.txt
  SGPL_NUM_THREADS=$P SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -v AutoTunerProfile > /tmp/m0_on_$P.txt
  if diff -q /tmp/m0_off_$P.txt /tmp/m0_on_$P.txt >/dev/null; then echo "P=$P identical: $(grep -o 'level_checksum [0-9]*' /tmp/m0_off_$P.txt | head -1)"; else echo "P=$P DIFF"; diff /tmp/m0_off_$P.txt /tmp/m0_on_$P.txt | head -4; fi
done
