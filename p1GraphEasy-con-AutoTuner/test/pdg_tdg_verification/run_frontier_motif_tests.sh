#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd -P)"
source "$ROOT/test/pdg_tdg_verification/runtime_test_helpers.sh"
TMP="$(sgpl_runtime_test_temp)"
trap 'sgpl_runtime_test_cleanup "$TMP"' EXIT
TEST_BIN="$TMP/frontier_motif_test"

gcc -O2 -std=gnu11 -pthread \
  "$ROOT/test/pdg_tdg_verification/frontier_motif_test.c" \
  "$ROOT/autotuner_runtime.c" \
  "$ROOT/parallel_runtime.c" \
  "$ROOT/gpu_runtime.c" \
  -lnlopt -lm \
  -o "$TEST_BIN"

for threads in 1 3 8; do
  SGPL_NUM_THREADS="$threads" "$TEST_BIN"
done

echo "frontier motif test matrix: PASS"
