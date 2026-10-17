#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
task_tmp=$(mktemp -d)
trap 'rm -rf "$task_tmp"' EXIT
ulimit -s 8192
for name in large_compaction_test source_snapshot_contract spatial_exec_test; do
 extra=()
 [[ "$name" == spatial_exec_test ]] && extra+=(-DSGPL_CUT_PARALLEL_GRAIN=1)
 for mode in normal sanitized; do
  flags=(-O2)
  [[ "$mode" == sanitized ]] && flags=(-O1 -g -no-pie -fsanitize=address,undefined -fno-omit-frame-pointer -DSGPL_GROUP_EPOCH_MAX=3)
  gcc "${flags[@]}" "${extra[@]}" -std=gnu11 -pthread -I. -Itest "test/$name.c" autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_tmp/check"
  if [[ "$mode" == normal ]];then widths=(1 4 32);else widths=(8);fi
  for width in "${widths[@]}"; do
   ASAN_OPTIONS=detect_leaks=0 SGPL_NUM_THREADS="$width" SGPL_ALLOW_HT=1 "$task_tmp/check"
  done
 done
done
echo 'PASS general frontier optimization contracts, nested budgets and sanitizers'
