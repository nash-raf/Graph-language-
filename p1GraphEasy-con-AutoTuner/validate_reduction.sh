#!/usr/bin/env bash
set -euo pipefail

# validate_reduction.sh — acceptance run for the CleanCut Reduction class.
#
# Proves, end-to-end through the DSL compiler + runtime, that a scalar
# reduction (MutG + Uop) is:
#   (1) RECOGNISED  -> the pass reports class=reduction red=1
#   (2) EMITTED     -> the parallel red step replaces the driver loop
#   (3) CORRECT     -> printed result == sequential build == independent
#                      python fold, across single- and multi-partition runs.
#
# Environment: Linux.  Builds runtime objects with gcc/g++ (no clang needed);
# uses the prebuilt ./GraphProgram (rebuild it via ./build_gp.sh if you change
# the compiler).  Run from this directory.

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
[[ -f semiring_runtime.c ]] && "$CC_BIN" -O3 -fopenmp -iquote . -c semiring_runtime.c -o semiring_runtime.o || true
"$CXX_BIN" -O3 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o roaring_bitmap.o
"$CXX_BIN" -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o graph_loader_runtime.o
"$CXX_BIN" -O2 -std=c++17 -c graph_runtime.cpp -o graph_runtime.o

LINK_OBJS=(
  runtime.o parallel_runtime.o gpu_runtime.o autotuner_runtime.o
  graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o
)
[[ -f semiring_runtime.o ]] && LINK_OBJS+=(semiring_runtime.o)

# Optional hardware calibration the autotuner cost model reads at runtime.
if [[ ! -f "${HOME}/.config/sgpl/hw_calib.json" ]]; then
  if [[ ! -x hw_calib_bench ]]; then
    "$CC_BIN" -O2 -o hw_calib_bench hw_calib_bench.c
  fi
  mkdir -p "${HOME}/.config/sgpl"
  ./hw_calib_bench > "${HOME}/.config/sgpl/hw_calib.json"
fi

GRAPHS=(reduce_sum reduce_sub reduce_mul reduce_min reduce_max)
FAILURES=0

echo "=== [1/4] End-to-end reduction validation ==="
for G in "${GRAPHS[@]}"; do
  EXPECTED="$(python3 compute_reduce_expected.py "$G.graph" rtest_small.txt)"

  # --- Parallel (CleanCut red step) build ---
  GRAPH_FRONTIER_STATS=1 ./GraphProgram "$G.graph" > "$TMP/$G.compile.out" 2> "$TMP/$G.compile.log"
  if ! grep -q 'class=reduction' "$TMP/$G.compile.log"; then
    echo "FAIL $G: pass did not classify as reduction"
    cat "$TMP/$G.compile.log"
    FAILURES=$((FAILURES + 1))
    continue
  fi
  if ! grep -q 'red=1' "$TMP/$G.compile.log"; then
    echo "FAIL $G: pass did not detect a ReducePtr (red=1 missing)"
    cat "$TMP/$G.compile.log"
    FAILURES=$((FAILURES + 1))
    continue
  fi
  mv program.o "$TMP/$G.o"
  "$CXX_BIN" -O3 -fopenmp -no-pie \
    "$TMP/$G.o" "${LINK_OBJS[@]}" -ldl -lnlopt -o "$TMP/final_$G"

  # --- Sequential (no rewrite) ground-truth build ---
  GRAPH_FRONTIER_REWRITE_OFF=1 ./GraphProgram "$G.graph" > "$TMP/$G.seq.out" 2> "$TMP/$G.seq.log"
  mv program.o "$TMP/$G.seq.o"
  "$CXX_BIN" -O3 -fopenmp -no-pie \
    "$TMP/$G.seq.o" "${LINK_OBJS[@]}" -ldl -lnlopt -o "$TMP/final_$G.seq"

  # --- Runs ---
  export OMP_NUM_THREADS=8
  OUT_P8="$(SGPL_CLEANCUT_PARTITIONS=8 SGPL_NUM_THREADS=8 "$TMP/final_$G")"
  OUT_P1="$(SGPL_CLEANCUT_PARTITIONS=1 SGPL_NUM_THREADS=8 "$TMP/final_$G")"
  OUT_SEQ="$(SGPL_NUM_THREADS=8 "$TMP/final_$G.seq")"

  OK=1
  [[ "$OUT_P8" != "$EXPECTED" ]] && OK=0
  [[ "$OUT_P1" != "$EXPECTED" ]] && OK=0
  [[ "$OUT_SEQ" != "$EXPECTED" ]] && OK=0

  if [[ "$OK" -eq 1 ]]; then
    echo "PASS $G  (part8=$OUT_P8 part1=$OUT_P1 seq=$OUT_SEQ expected=$EXPECTED)"
  else
    echo "FAIL $G  (part8=$OUT_P8 part1=$OUT_P1 seq=$OUT_SEQ expected=$EXPECTED)"
    FAILURES=$((FAILURES + 1))
  fi
done

echo "=== [2/4] Summary ==="
if [[ "$FAILURES" -eq 0 ]]; then
  echo "reduction end-to-end validation: PASS"
  exit 0
else
  echo "reduction end-to-end validation: FAIL ($FAILURES)"
  exit 1
fi