#!/usr/bin/env bash
# GPU device-step differential for one fixture: build with FORCE_GPU=1 (device
# kernels + PTX), compare device step off vs on, report device vs fallback counts.
set -u
ulimit -s unlimited 2>/dev/null || true   # generated frames can exceed the 8 MB default
FX="${1:?usage: gpu_device_diff.sh <fixture.graph> [threads...]}"; shift || true
THREADS=("$@"); [ ${#THREADS[@]} -eq 0 ] && THREADS=(1 4 8)
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
P1="$ROOT/p1GraphEasy-con-AutoTuner"
export PATH="${LLVM_NVPTX_PREFIX:-/root/llvm-20.1.8-nvptx}/bin:$PATH"
cd "$P1"
rm -f final_program program.o gpu_runtime.o kernels.ptx
FORCE_GPU=1 GRAPH_FILE="$FX" timeout 500 bash 03_run.sh > /tmp/gd_build.log 2>&1
if ! [ -x final_program ]; then echo "BUILD FAILED"; tail -6 /tmp/gd_build.log; exit 1; fi
echo "fixture: $(basename "$FX") | kernels.ptx: $(wc -c < kernels.ptx 2>/dev/null || echo 0) bytes | nvptx-unavailable: $(grep -c 'NVPTX target unavailable' /tmp/gd_build.log)"
for P in "${THREADS[@]}"; do
  SGPL_NUM_THREADS=$P ./final_program 2>&1 | grep -v AutoTunerProfile > /tmp/gd_off_$P.txt
  SGPL_NUM_THREADS=$P SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -v AutoTunerProfile > /tmp/gd_on_$P.txt
  if diff -q /tmp/gd_off_$P.txt /tmp/gd_on_$P.txt >/dev/null; then echo "P=$P identical"; else echo "P=$P DIFF"; diff /tmp/gd_off_$P.txt /tmp/gd_on_$P.txt | head -4; fi
done
DEV=$(SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -c "ran on device")
REF=$(SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -cE "kept on the CPU|not found; CPU")
echo "device dispatches: $DEV | cpu-fallback dispatches: $REF"
