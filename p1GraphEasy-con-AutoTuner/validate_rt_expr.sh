#!/usr/bin/env bash
set -euo pipefail

# validate_rt_expr.sh — post-cutover acceptance gate for the direct
# effect-expression execution path (the sole emitter since R5).
#
# For every fixture: compile+link+run the program at 1 and 8 partitions and
# require identical program output.  Before the R5 cutover this script was a
# differential gate against the legacy strategy emitters (SGPL_RT_EXPR=0);
# that control is gone, so the recorded legacy outputs survive as the expected
# values inside the end-to-end suites (reduction/roundsep/composition).
#
# Usage: ./validate_rt_expr.sh [fixture ...]   (defaults to the matrix below)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

FIXTURES=("$@")
if [[ ${#FIXTURES[@]} -eq 0 ]]; then
  FIXTURES=(rt_expr_pagerank rt_expr_source_red rt_expr_priv_multi
            rt_expr_firstwins rt_expr_kcore rt_expr_dual_claim
            rt_expr_reduce_activate rt_expr_priv_activate
            reduce_sum reduce_sub reduce_mul reduce_min reduce_max
            small_sssp small_cc roundsep_min roundsep_add
            cross_dep dual_uv mixed_same_array)
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "=== [0/2] runtime objects ==="
gcc -O3 -c autotuner_runtime.c -o "$TMP/autotuner_runtime.o"
gcc -O3 -c graph_mutation_runtime.c -o "$TMP/graph_mutation_runtime.o"
gcc -O3 -c parallel_runtime.c -o "$TMP/parallel_runtime.o"
gcc -O3 -c gpu_runtime.c -o "$TMP/gpu_runtime.o"
gcc -O3 -c runtime.c -o "$TMP/runtime.o"
if [[ -f semiring_runtime.c ]]; then
  gcc -O3 -fopenmp -iquote . -c semiring_runtime.c -o "$TMP/semiring_runtime.o" || true
fi
g++ -O3 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o "$TMP/roaring_bitmap.o"
g++ -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o "$TMP/graph_loader_runtime.o"
g++ -O2 -std=c++17 -c graph_runtime.cpp -o "$TMP/graph_runtime.o"

LINK_OBJS=(
  "$TMP/runtime.o" "$TMP/parallel_runtime.o" "$TMP/gpu_runtime.o"
  "$TMP/autotuner_runtime.o" "$TMP/graph_mutation_runtime.o"
  "$TMP/roaring_bitmap.o" "$TMP/graph_loader_runtime.o" "$TMP/graph_runtime.o"
)
[[ -f "$TMP/semiring_runtime.o" ]] && LINK_OBJS+=("$TMP/semiring_runtime.o")

echo "=== [1/2] partition-count determinism ==="
FAILURES=0
for f in "${FIXTURES[@]}"; do
  [[ -f "$f.graph" ]] || { echo "SKIP missing $f.graph"; continue; }
  EXTRA_ENV=()
  env "${EXTRA_ENV[@]}" ./GraphProgram "$f.graph" > /dev/null 2>&1
  mv program.o "$TMP/$f.o"
  g++ -O3 -fopenmp -no-pie "$TMP/$f.o" "${LINK_OBJS[@]}" -ldl -lnlopt \
    -o "$TMP/$f"
  OUT_P1="$(SGPL_CLEANCUT_PARTITIONS=1 SGPL_NUM_THREADS=8 "$TMP/$f" 2>/dev/null)"
  OUT_P8="$(SGPL_CLEANCUT_PARTITIONS=8 SGPL_NUM_THREADS=8 "$TMP/$f" 2>/dev/null)"
  if [[ "$OUT_P1" == "$OUT_P8" ]]; then
    echo "PASS $f (p1=$OUT_P1 p8=$OUT_P8)"
  else
    echo "FAIL $f (p1=$OUT_P1 p8=$OUT_P8)"
    FAILURES=$((FAILURES + 1))
  fi
done

echo "=== [2/2] summary ==="
if [[ "$FAILURES" -eq 0 ]]; then
  echo "direct expression execution acceptance: PASS"
  exit 0
fi
echo "direct expression execution acceptance: FAIL ($FAILURES)"
exit 1
