#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ "${SGPL_ALLOW_LOCAL_CONTRACT_TESTS:-0}" != 1 ]]; then
  echo 'Explicit local contract-test enablement required' >&2; exit 78
fi
task_output="${P1_SIMPLIFY_TEST_OUTPUT:?Select an external correctness output directory}"
mkdir -p "$task_output"
ulimit -s 8192
g++ -O1 -g -I. test/execution_simplify_ir_test.cpp $(llvm-config-20 --cxxflags --ldflags --libs all --system-libs) -o "$task_output/ir-test"
"$task_output/ir-test" "$task_output"
for mode in normal sanitized; do
  flags=(-O2)
  [[ "$mode" == sanitized ]] && flags=(-O1 -g -no-pie -fsanitize=address,undefined -fno-omit-frame-pointer)
  gcc "${flags[@]}" -DSGPL_CUT_PARALLEL_GRAIN=1 -std=gnu11 -fopenmp -pthread -I. -Itest test/execution_simplify_runtime_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_output/runtime-$mode"
  for width in ${P1_TEST_WIDTHS:?Select physical test widths}; do
    ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_output/runtime-$mode"
  done
done
