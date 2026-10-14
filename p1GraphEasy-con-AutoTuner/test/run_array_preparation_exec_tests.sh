#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'status=$?; if ((status)); then echo "array preparation logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
gcc -O2 -std=gnu11 -pthread -I"$ROOT" "$ROOT/test/array_preparation_exec_test.c" \
  "$ROOT/autotuner_runtime.c" "$ROOT/parallel_runtime.c" -lnlopt -lm \
  -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/preparation"
for workers in 1 2 4 8; do
  timeout 90 env SGPL_NUM_THREADS="$workers" SGPL_PREPARE_DEBUG=1 "$TMP/preparation" >"$TMP/$workers.log" 2>&1
  grep -q 'parallel array preparation: 0 failures' "$TMP/$workers.log"
  if ((workers>1)); then grep -q "actual=$workers rc=0" "$TMP/$workers.log"; fi
done
timeout 90 env SGPL_NUM_THREADS=4 SGPL_PREPARE_DEBUG=1 "$TMP/preparation" cold-failure >"$TMP/cold.log" 2>&1
grep -q 'parallel array preparation: 0 failures' "$TMP/cold.log"
grep -q 'granted=4 actual=1 rc=-' "$TMP/cold.log"
echo 'PASS stable parallel preparation, source hooks, weighted hints, nested/parent budgets, cold-team failure and lazy full-view upgrade'
