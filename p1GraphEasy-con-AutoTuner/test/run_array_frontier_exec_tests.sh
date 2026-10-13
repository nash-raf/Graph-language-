#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
gcc -O2 -std=gnu11 -pthread -I"$ROOT" "$ROOT/test/array_frontier_exec_test.c" \
  "$ROOT/autotuner_runtime.c" "$ROOT/parallel_runtime.c" -lnlopt -lm \
  -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/array"
for workers in 1 2 4 8; do
  timeout 90 env SGPL_NUM_THREADS="$workers" "$TMP/array"
done
