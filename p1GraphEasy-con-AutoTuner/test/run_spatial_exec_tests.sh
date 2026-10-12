#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "spatial executor logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
gcc -O2 -std=gnu11 -pthread -I"$ROOT" "$ROOT/test/spatial_exec_test.c" \
  "$ROOT/autotuner_runtime.c" "$ROOT/parallel_runtime.c" -lnlopt -lm \
  -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/spatial"
for workers in 1 2 4; do
  timeout 90 env SGPL_NUM_THREADS="$workers" "$TMP/spatial" >"$TMP/$workers.log" 2>&1 || { cat "$TMP/$workers.log"; exit 1; }
  tail -n 1 "$TMP/$workers.log"
done
