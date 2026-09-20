#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
TEST_BIN="$TMP/exec_engine_test"

gcc -O2 -std=gnu11 -pthread \
  "$ROOT/test/exec_engine_test.c" \
  "$ROOT/autotuner_runtime.c" \
  "$ROOT/parallel_runtime.c" \
  -lnlopt -lm \
  -o "$TEST_BIN"

for threads in 1 3 8; do
  echo "== threads=$threads =="
  timeout 60 env SGPL_NUM_THREADS="$threads" "$TEST_BIN"
done

echo "composable exec engine tests: PASS"
