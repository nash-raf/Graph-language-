#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
gcc -O2 -std=gnu11 -pthread -I"$ROOT" "$ROOT/test/array_frontier_exec_test.c" \
  "$ROOT/autotuner_runtime.c" "$ROOT/parallel_runtime.c" -lnlopt -lm \
  -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/array"
export SGPL_ALLOW_HT=0
for workers in 1 2 6; do
  timeout 90 env SGPL_NUM_THREADS="$workers" "$TMP/array"
done
gcc -O2 -std=gnu11 -pthread -I"$ROOT" "$ROOT/test/declared_array_exec_test.c" \
  "$ROOT/autotuner_runtime.c" "$ROOT/parallel_runtime.c" -lnlopt -lm \
  -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/declared"
for workers in 2 6; do
  for mode in full nested; do
    args=()
    [[ "$mode" == nested ]] && args=(nested)
    timeout 90 env SGPL_NUM_THREADS="$workers" SGPL_ALLOW_HT=0 SGPL_DAG_DEBUG=1 \
      "$TMP/declared" "${args[@]}" >"$TMP/declared_${workers}_${mode}.log" 2>&1
    grep -q 'sgpl-dag-independent' "$TMP/declared_${workers}_${mode}.log"
    tail -n 1 "$TMP/declared_${workers}_${mode}.log"
  done
done
