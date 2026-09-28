#!/usr/bin/env bash
# tdg_budget_test: engine/budget integration gate.
#
# Builds the runtime harness (tdg_budget_test.c + parallel_runtime.c +
# roaring_bitmap.cpp) and runs it in three configurations:
#   1. 4 threads, integration on (default)
#   2. 4 threads, SGPL_NO_UNREGISTERED_POOL_SHARE=1 SGPL_NO_REGION_SCOPE=1
#   3. 1 thread
# Every configuration must report ALL PASS, and the reservation ledger must be
# balanced in each (checked inside the harness).
set -euo pipefail

R="$(cd "$(dirname "$0")" && pwd)"
C="$R/../p1GraphEasy-con-AutoTuner"
OUT="${TMPDIR:-/tmp}/tdg_budget_test.$$"

NLOPT=""
INC=""
LIB=""
if [[ -d "$C/.deps/nlopt" ]]; then
  NLOPT="$C/.deps/nlopt"
  INC="-I$NLOPT/include"
  LIB="-L$NLOPT/lib -L$NLOPT/lib64 -Wl,-rpath,$NLOPT/lib -Wl,-rpath,$NLOPT/lib64"
fi

echo "=== building tdg_budget_test ==="
gcc -O2 $INC -c "$C/tdg_budget_test.c" -o "$OUT.test.o"
gcc -O3 $INC -c "$C/parallel_runtime.c" -o "$OUT.rt.o"
g++ -O3 -mavx2 -march=native -fopenmp -c "$C/roaring_bitmap.cpp" -o "$OUT.rb.o"
g++ -O2 -fopenmp "$OUT.test.o" "$OUT.rt.o" "$OUT.rb.o" -o "$OUT.bin" \
  $LIB -lnlopt -lpthread -lm

fails=0
run() {
  local name="$1"; shift
  echo "=== $name ==="
  if env "$@" "$OUT.bin"; then
    :
  else
    echo "FAILED: $name"
    fails=$((fails + 1))
  fi
}

run "4 threads (integration on)" \
  SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4
run "4 threads (integration off)" \
  SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4 \
  SGPL_NO_UNREGISTERED_POOL_SHARE=1 SGPL_NO_REGION_SCOPE=1
run "1 thread" SGPL_NUM_THREADS=1 OMP_NUM_THREADS=1

rm -f "$OUT.test.o" "$OUT.rt.o" "$OUT.rb.o" "$OUT.bin"

if [[ "$fails" -eq 0 ]]; then
  echo "=== tdg_budget_test: PASS ==="
else
  echo "=== tdg_budget_test: $fails configuration(s) FAILED ==="
  exit 1
fi
