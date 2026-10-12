#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
gcc -O2 -std=gnu11 -pthread -I"$ROOT" "$ROOT/test/worker_pool_test.c" \
  "$ROOT/autotuner_runtime.c" "$ROOT/parallel_runtime.c" -lnlopt -lm \
  -Wl,--wrap=pthread_create -Wl,--wrap=pthread_join -Wl,--wrap=calloc -o "$TMP/pool"
for workers in 1 2 4 8; do
  echo "== persistent pool: workers=$workers =="
  timeout 60 env SGPL_NUM_THREADS="$workers" "$TMP/pool"
  if ((workers>1)); then
    for fault in 1 "$workers" allocation; do
      timeout 30 env SGPL_NUM_THREADS="$workers" "$TMP/pool" "$fault"
    done
  fi
done
