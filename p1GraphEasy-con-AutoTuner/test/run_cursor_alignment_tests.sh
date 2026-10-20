#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
ulimit -s 8192
task_tmp=$(mktemp -d)
trap 'rm -rf "$task_tmp"' EXIT
for mode in normal sanitized no_scratch; do
  flags=(-O2)
  [[ "$mode" == sanitized ]] && flags=(-O1 -g -no-pie -fsanitize=address,undefined -fno-omit-frame-pointer)
  [[ "$mode" == no_scratch ]] && flags+=(-DSGPL_EXEC_REUSE_BUFFERS=0)
  if [[ "$mode" != no_scratch ]]; then
    gcc "${flags[@]}" -std=gnu11 -pthread -I. test/cursor_storage_test.c \
      -Wl,--wrap=malloc -o "$task_tmp/storage"
    ASAN_OPTIONS=detect_leaks=1 "$task_tmp/storage"
    gcc "${flags[@]}" -std=gnu11 -pthread -I. test/array_frontier_lease_test.c \
      autotuner_runtime.c parallel_runtime.c -lnlopt -lm \
      -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_tmp/lease"
    for width in ${P1_TEST_WIDTHS:-1 2 6}; do
      ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_tmp/lease"
    done
  fi
  gcc "${flags[@]}" -DSGPL_PREPARATION_TEST=1 -std=gnu11 -fopenmp -pthread -I. -Itest \
    test/array_preparation_exec_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm \
    -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_tmp/check"
  for width in ${P1_TEST_WIDTHS:-1 2 6}; do
    for partitions in 3 8 12; do
      env -u SGPL_NO_CURSOR_ALIGNMENT ASAN_OPTIONS=detect_leaks=0 P1_TEST_PARTITIONS="$partitions" \
        python3 test/physical_run.py "$width" "$task_tmp/check"
    done
    env SGPL_NO_CURSOR_ALIGNMENT=1 ASAN_OPTIONS=detect_leaks=0 \
      python3 test/physical_run.py "$width" "$task_tmp/check"
    env -u SGPL_NO_CURSOR_ALIGNMENT ASAN_OPTIONS=detect_leaks=0 \
      python3 test/physical_run.py "$width" "$task_tmp/check" cold-failure
  done
done
echo 'PASS aligned and padded cursors, dense control, cached/uncached/sanitized ordering, weights, failures and nested budgets'
