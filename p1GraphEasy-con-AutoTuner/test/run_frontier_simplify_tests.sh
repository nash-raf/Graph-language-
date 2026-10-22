#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
[[ "${SGPL_ALLOW_LOCAL_CONTRACT_TESTS:-0}" == 1 ]] || { echo 'Explicit local contract-test enablement required' >&2; exit 78; }
task_output="${P1_FRONTIER_SIMPLIFY_TEST_OUTPUT:?Select an external correctness output directory}"
mkdir -p "$task_output"
ulimit -s 8192
g++ -O1 -g -I. test/frontier_simplify_ir_test.cpp $(llvm-config-20 --cxxflags --ldflags --libs all --system-libs) -o "$task_output/ir-test"
"$task_output/ir-test" "$task_output"
[[ "${1:-}" != ir-only ]] || exit 0
for mode in normal sanitized no_scratch; do
  flags=(-O2)
  [[ "$mode" == sanitized ]] && flags=(-O1 -g -no-pie -fsanitize=address,undefined -fno-omit-frame-pointer)
  [[ "$mode" == no_scratch ]] && flags+=(-DSGPL_EXEC_REUSE_BUFFERS=0)
  gcc "${flags[@]}" -DSGPL_CUT_PARALLEL_GRAIN=1 -std=gnu11 -fopenmp -pthread -I. -Itest test/frontier_simplify_runtime_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_output/runtime-$mode"
  for width in ${P1_TEST_WIDTHS:?Select physical test widths}; do
    SGPL_TEST_EXPECT_SIMPLIFY=$([[ "$mode" == no_scratch ]] && echo '' || echo 1) ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_output/runtime-$mode"
  done
done
