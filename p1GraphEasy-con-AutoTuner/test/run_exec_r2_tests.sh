#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

SRC_BIN="$TMP/exec_source_cover_test"
FJ_BIN="$TMP/exec_forkjoin_test"
SNAP_BIN="$TMP/exec_snapshot_test"

gcc -O2 -std=gnu11 -pthread \
  "$ROOT/test/exec_source_cover_test.c" \
  "$ROOT/autotuner_runtime.c" \
  "$ROOT/parallel_runtime.c" \
  -lnlopt -lm -o "$SRC_BIN"

gcc -O2 -std=gnu11 -pthread \
  "$ROOT/test/exec_forkjoin_test.c" \
  "$ROOT/autotuner_runtime.c" \
  "$ROOT/parallel_runtime.c" \
  -lnlopt -lm -o "$FJ_BIN"

gcc -O2 -std=gnu11 -pthread \
  "$ROOT/test/exec_snapshot_test.c" \
  "$ROOT/autotuner_runtime.c" \
  "$ROOT/parallel_runtime.c" \
  -lnlopt -lm -o "$SNAP_BIN"

for threads in 1 3 8; do
  echo "== threads=$threads =="
  echo "-- source coverage --"
  timeout 60 env SGPL_NUM_THREADS="$threads" "$SRC_BIN"
  echo "-- fork/join (incl. nested) --"
  timeout 60 env SGPL_NUM_THREADS="$threads" "$FJ_BIN"
  echo "-- snapshot + source-state channel --"
  timeout 60 env SGPL_NUM_THREADS="$threads" "$SNAP_BIN"
done

echo "composable exec R2 tests: PASS"
