#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "GPU IR logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
cd "$ROOT"
flags="$(llvm-config-20 --cxxflags)"
flags="${flags//-fno-exceptions/}"
libs="$(llvm-config-20 --ldflags --libs all --system-libs)"
g++ -O1 $flags -iquote . -c test/round_spatial_lowering_test.cpp -o "$TMP/test.o"
g++ -O1 $flags -iquote . -c parallel_loop_outline.cpp -o "$TMP/outline.o"
g++ "$TMP/test.o" "$TMP/outline.o" $libs -pthread -o "$TMP/compiler"
SGPL_GPU_DEBUG=1 "$TMP/compiler" "$TMP" gpu >"$TMP/compiler.log" 2>&1 || { cat "$TMP/compiler.log"; exit 1; }
for source in gpu_runtime autotuner_runtime parallel_runtime; do gcc -O1 -iquote . -c "$source.c" -o "$TMP/$source.o"; done
for mode in source destination snapshot source_acc dynamic_scratch dynamic_snapshot exit_phi guard_phi frontier_source frontier_destination prefix_phi; do
 llc-20 -O2 -filetype=obj "$TMP/$mode.ll" -o "$TMP/$mode.o"
 gcc -O1 -no-pie -pthread -DSGPL_GPU_TEST -iquote . test/round_spatial_ir_reference.c "$TMP/$mode.o" \
  "$TMP/gpu_runtime.o" "$TMP/autotuner_runtime.o" "$TMP/parallel_runtime.o" \
  -ldl -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/$mode"
 for workers in 1 2 4 8; do
  timeout 30 env SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS="$workers" SGPL_CLEANCUT_PARTITIONS=3 \
   "$TMP/$mode" "$mode" >"$TMP/$mode.$workers.out" 2>"$TMP/$mode.$workers.log" || { cat "$TMP/$mode.$workers.log"; cat "$TMP/$mode.$workers.out"; exit 1; }
  grep -q '\[gpu-task\]' "$TMP/$mode.$workers.log" || { cat "$TMP/$mode.$workers.log"; echo "FAIL no device execution: $mode"; exit 1; }
 done
 label="production proofs"
 [[ "$mode" == frontier_* ]] && label="emitter fixture with assumed round result"
 echo "PASS GPU IR $mode ($label), workers=1,2,4,8"
done
