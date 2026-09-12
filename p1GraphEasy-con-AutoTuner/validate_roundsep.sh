#!/usr/bin/env bash
set -euo pipefail

# validate_roundsep.sh — acceptance run for composition A: round-separated
# in-place loops (same-base cross-endpoint R x W).
#
# Proves, end-to-end through the DSL compiler + runtime:
#   (1) RECOGNITION   -> the pass reports class=dest-owner shadow=N
#   (2) SAFETY        -> outputs are IDENTICAL across partitions and threads
#                        (the shadow removed the cross-partition data race)
#   (3) CORRECTNESS   -> outputs equal the independent python round-separated
#                        reference (frozen-read semantics)
#
# Modes: min/add = ungated single-sweep in-place; sssp = gated frontier
# in-place (smallest frontier-driven weighted relax).  Requires the rebuilt
# ./GraphProgram (run ./build_gp.sh first) and rebuilt runtime objects
# (gcc rebuild happens here).  Run from this directory on Linux/WSL.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

CXX_BIN="${CXX_BIN:-g++}"
CC_BIN="${CC_BIN:-gcc}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "=== [0/4] Build runtime objects ==="
"$CC_BIN" -O3 -c autotuner_runtime.c -o autotuner_runtime.o
"$CC_BIN" -O3 -c graph_mutation_runtime.c -o graph_mutation_runtime.o
"$CC_BIN" -O3 -c parallel_runtime.c -o parallel_runtime.o
"$CC_BIN" -O3 -c gpu_runtime.c -o gpu_runtime.o
"$CC_BIN" -O3 -c runtime.c -o runtime.o
"$CXX_BIN" -O3 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o roaring_bitmap.o
"$CXX_BIN" -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o graph_loader_runtime.o
"$CXX_BIN" -O2 -std=c++17 -c graph_runtime.cpp -o graph_runtime.o

LINK_OBJS=(
  runtime.o parallel_runtime.o gpu_runtime.o autotuner_runtime.o
  graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o
)

TESTS="min:roundsep_min.graph:roundsep_small.txt
add:roundsep_add.graph:roundsep_small.txt
sssp:roundsep_sssp.graph:roundsep_sssp_small.txt"

FAILURES=0

echo "=== [1/4] Round-separation validation ==="
while IFS=: read -r MODE GRAPH EDGES; do
  [[ -n "$MODE" ]] || continue
  EXPECTED="$(python3 compute_roundsep_expected.py "$MODE" "$GRAPH" "$EDGES")"

  GRAPH_FRONTIER_STATS=1 ./GraphProgram "$GRAPH" > "$TMP/$MODE.out" 2> "$TMP/$MODE.log"
  if ! grep -q 'shadow=[1-9]' "$TMP/$MODE.log"; then
    echo "FAIL $MODE: shadow snapshot not emitted"
    grep 'graph-frontier' "$TMP/$MODE.log" | head -5
    FAILURES=$((FAILURES + 1))
    continue
  fi
  if ! grep -q 'class=dest-owner' "$TMP/$MODE.log"; then
    echo "FAIL $MODE: not classified dest-owner (expected owner step + shadow)"
    grep 'graph-frontier' "$TMP/$MODE.log" | head -5
    FAILURES=$((FAILURES + 1))
    continue
  fi
  mv program.o "$TMP/$MODE.o"
  "$CXX_BIN" -O3 -fopenmp -no-pie \
    "$TMP/$MODE.o" "${LINK_OBJS[@]}" -ldl -lnlopt -o "$TMP/final_$MODE"

  OUT_P8="$(SGPL_CLEANCUT_PARTITIONS=8 SGPL_NUM_THREADS=8 OMP_NUM_THREADS=8 "$TMP/final_$MODE")"
  OUT_P1="$(SGPL_CLEANCUT_PARTITIONS=1 SGPL_NUM_THREADS=1 OMP_NUM_THREADS=1 "$TMP/final_$MODE")"
  OUT_P3="$(SGPL_CLEANCUT_PARTITIONS=3 SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4 "$TMP/final_$MODE")"

  OK=1
  [[ "$OUT_P8" != "$EXPECTED" ]] && OK=0
  [[ "$OUT_P1" != "$EXPECTED" ]] && OK=0
  [[ "$OUT_P3" != "$EXPECTED" ]] && OK=0
  [[ "$OUT_P8" != "$OUT_P1" || "$OUT_P8" != "$OUT_P3" ]] && OK=0

  if [[ "$OK" -eq 1 ]]; then
    echo "PASS $MODE  (deterministic p1/p3/p8, matches round-separated reference)"
  else
    echo "FAIL $MODE"
    echo "  p8 : $OUT_P8"
    echo "  p1 : $OUT_P1"
    echo "  p3 : $OUT_P3"
    echo "  exp: $EXPECTED"
    FAILURES=$((FAILURES + 1))
  fi
done <<< "$TESTS"

echo "=== [2/4] Summary ==="
if [[ "$FAILURES" -eq 0 ]]; then
  echo "round-separation validation: PASS"
  exit 0
else
  echo "round-separation validation: FAIL ($FAILURES)"
  exit 1
fi