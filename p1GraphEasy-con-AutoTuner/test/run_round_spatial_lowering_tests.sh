#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "joint IR logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
LLVM_CONFIG="${LLVM_CONFIG:-llvm-config-20}"
flags="$($LLVM_CONFIG --cxxflags)"
flags="${flags//-fno-exceptions/}"
g++ -O1 $flags -iquote "$ROOT" "$ROOT/test/round_spatial_lowering_test.cpp" \
 $($LLVM_CONFIG --ldflags --libs all --system-libs) -pthread -o "$TMP/compiler"
GRAPH_FRONTIER_STATS=1 SGPL_FRONTIER_STRICT=1 "$TMP/compiler" "$TMP" >"$TMP/compiler.log" 2>&1 || { cat "$TMP/compiler.log"; exit 1; }
cat "$TMP/compiler.log"
for mode in source destination snapshot source_acc dynamic_scratch dynamic_snapshot exit_phi guard_phi frontier_source frontier_destination unsafe_extent prefix_phi; do
 llc-20 -O2 -filetype=obj "$TMP/$mode.ll" -o "$TMP/$mode.o"
 gcc -O2 -no-pie -std=gnu11 -pthread -I"$ROOT" "$ROOT/test/round_spatial_ir_reference.c" "$TMP/$mode.o" \
  "$ROOT/autotuner_runtime.c" "$ROOT/parallel_runtime.c" -lnlopt -lm \
  -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/$mode"
 for workers in 1 2 4 8; do
  timeout 30 env SGPL_NUM_THREADS="$workers" SGPL_CLEANCUT_PARTITIONS=3 "$TMP/$mode" "$mode"
 done
done
