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

# V1 ready-work DAG scheduler: dependency order, cancellation, cycle and
# edge/validation rules, section 19 partial-DAG overlap (temporal U1->U3 and
# spatial P1->P3 with the middle node independent) and the section 15 budget
# grants (ready-set reservation, work-estimate ceilings, nested clamp).
DAG_BIN="$TMP/dag_scheduler_test"
gcc -O2 -std=gnu11 -pthread -I"$ROOT" \
  "$ROOT/test/dag_scheduler_test.c" \
  "$ROOT/autotuner_runtime.c" \
  "$ROOT/parallel_runtime.c" \
  -lnlopt -lm \
  -o "$DAG_BIN"
echo "== dag scheduler =="
timeout 180 env SGPL_NUM_THREADS=8 "$DAG_BIN"

echo "composable exec engine tests: PASS"
