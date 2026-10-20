#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
task_tmp=$(mktemp -d)
trap 'rm -rf "$task_tmp"' EXIT
g++ -O1 -I. test/static_schedule_ir_test.cpp $(llvm-config-20 --cxxflags --ldflags --libs core asmparser --system-libs) -o "$task_tmp/ir-check"
"$task_tmp/ir-check"
for mode in normal sanitized;do
 flags=(-O2)
 [[ "$mode" == sanitized ]] && flags=(-O1 -g -no-pie -fsanitize=address,undefined -fno-omit-frame-pointer)
 gcc "${flags[@]}" -std=gnu11 -fopenmp -pthread -I. -Itest test/static_schedule_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_tmp/check"
 for width in ${P1_TEST_WIDTHS:-1 2 6};do
  ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_tmp/check"
 done
done
