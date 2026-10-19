#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
task_tmp=$(mktemp -d)
trap 'rm -rf "$task_tmp"' EXIT
ulimit -s 8192
for mode in normal sanitized no_scratch;do
 flags=(-O2)
 [[ "$mode" == sanitized ]] && flags=(-O1 -g -no-pie -fsanitize=address,undefined -fno-omit-frame-pointer)
 [[ "$mode" == no_scratch ]] && flags+=(-DSGPL_EXEC_REUSE_BUFFERS=0)
 flags+=(-DSGPL_CUT_PARALLEL_GRAIN=1)
 gcc "${flags[@]}" -std=gnu11 -fopenmp -pthread -I. -Itest test/fullgraph_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_tmp/check"
 for width in 1 2 6;do
  for policy in default legacy serial;do
   feature=()
   [[ "$policy" == legacy ]] && feature=(SGPL_FULL_INDEX_LEGACY=1)
   [[ "$policy" == serial ]] && feature=(SGPL_FULL_INDEX_SERIAL=1)
   env -u SGPL_FULL_INDEX_LEGACY -u SGPL_FULL_INDEX_SERIAL ASAN_OPTIONS=detect_leaks=0 "${feature[@]}" python3 test/physical_run.py "$width" "$task_tmp/check"
  done
 done
done
echo 'PASS stable incoming occurrences, lifecycle, exact rounding and fallbacks'
if [[ "${SGPL_FULLGRAPH_NATIVE_ONLY:-0}" != 1 ]];then python3 test/check_fullgraph.py;fi
