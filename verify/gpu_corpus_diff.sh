#!/usr/bin/env bash
# GPU device-step differential over the corpus: build each fixture with
# FORCE_GPU=1 (device kernels + PTX), diff device-off vs device-on (P=4), and
# report how many dispatches really ran on the device.
set -u
ulimit -s unlimited 2>/dev/null || true
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
P1="$ROOT/p1GraphEasy-con-AutoTuner"
export PATH="${LLVM_NVPTX_PREFIX:-/root/llvm-20.1.8-nvptx}/bin:$PATH"
REPORT="${REPORT:-/tmp/gpu_corpus_report.txt}"
: > "$REPORT"
cd "$P1"
for fx in "$ROOT"/verify/cases/*/*.graph; do
  [ -f "$fx" ] || continue
  name="$(basename "$fx")"
  rm -f final_program program.o gpu_runtime.o kernels.ptx
  if ! FORCE_GPU=1 GRAPH_FILE="$fx" timeout 400 bash 03_run.sh > /tmp/gcd_build.log 2>&1; then
    echo "$name BUILD-FAILED" >> "$REPORT"; continue
  fi
  [ -x final_program ] || { echo "$name BUILD-FAILED(no bin)" >> "$REPORT"; continue; }
  SGPL_NUM_THREADS=4 ./final_program 2>&1 | grep -v "^\[AutoTunerProfile" > /tmp/gcd_off.txt
  rc_off=$?
  SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -v "^\[AutoTunerProfile" > /tmp/gcd_on.txt
  rc_on=$?
  dev=$(SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -c "ran on device")
  ref=$(SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -cE "kept on the CPU|not found; CPU")
  ans="$(grep -m1 -vE '^\[AutoTunerProfile' /tmp/gcd_off.txt | head -c 40)"
  if diff -q /tmp/gcd_off.txt /tmp/gcd_on.txt >/dev/null; then d=same; else d=DIFF; fi
  echo "$name rc=$rc_off/$rc_on $d device=$dev fallback=$ref ans=[$ans]" >> "$REPORT"
done
echo "CORPUS-DONE $(grep -c '' "$REPORT") fixtures" >> "$REPORT"
