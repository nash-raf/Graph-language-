#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ ! ( -d /kaggle/working && -d /kaggle/input ) && "${SGPL_ALLOW_LOCAL_CONTRACT_TESTS:-0}" != 1 ]];then
 echo 'Kaggle or explicitly enabled local contract tests required' >&2; exit 78
fi
ulimit -s 8192
task_tmp=$(mktemp -d)
trap 'rm -rf "$task_tmp"' EXIT
for feature in effect_fusion frontier_fusion append_journal;do
 [[ -f "$feature"_plan.h ]] || continue
 g++ -O1 -I. "test/$feature"_ir_test.cpp $(llvm-config-20 --cxxflags --ldflags --libs all --system-libs) -o "$task_tmp/ir"
 "$task_tmp/ir"
done
[[ "${1:-}" != ir-only ]] || exit 0
for feature in effect_fusion frontier_fusion append_journal;do
 [[ -f "$feature"_plan.h ]] || continue
 native="test/$feature"_test.c
 [[ "$feature" == effect_fusion ]] && native=test/effect_fusion_runtime_test.c
 modes=(sanitized normal no_scratch)
 [[ "$feature" == frontier_fusion ]] && modes+=(global_guard_negative_control)
 [[ "$feature" == append_journal ]] && modes+=(gate_alignment_control)
 for mode in "${modes[@]}";do
  flags=(-O2 -DSGPL_CUT_PARALLEL_GRAIN=1)
  [[ "$mode" == sanitized ]] && flags=(-O1 -g -no-pie -fsanitize=address,undefined -fno-omit-frame-pointer -DSGPL_CUT_PARALLEL_GRAIN=1)
  [[ "$mode" == no_scratch ]] && flags+=(-DSGPL_EXEC_REUSE_BUFFERS=0)
  [[ "$mode" == global_guard_negative_control ]] && flags+=(-DSGPL_FUSION_TEST_GLOBAL_OWNER_GUARD=1)
  [[ "$mode" == gate_alignment_control ]] && flags+=(-DSGPL_SOURCE_GATE_ALIGNMENT=16)
  [[ "$feature" == append_journal ]] && flags+=(-DSGPL_EXEC_APPEND_EPOCH_MAX=3)
  gcc "${flags[@]}" -std=gnu11 -fopenmp -pthread -I. -Itest "$native" autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$task_tmp/runtime"
  for width in ${P1_TEST_WIDTHS:?Select physical test widths before running contracts};do
   if [[ "$mode" == global_guard_negative_control ]];then
    SGPL_TEST_EXPECT_GLOBAL_GUARD_FAILURE=1 python3 test/physical_run.py "$width" "$task_tmp/runtime"
    continue
   fi
   echo "CONTRACT feature=$feature mode=$mode width=$width"
   SGPL_TEST_ALLOC_STACK=1 ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer-20)" ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_tmp/runtime"
   if [[ "$feature" == frontier_fusion ]];then
    SGPL_FUSION_B_BINARY_LOOKUP=1 SGPL_TEST_ALLOC_STACK=1 ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer-20)" ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_tmp/runtime"
   fi
   if [[ "$feature" == append_journal ]] && [[ "$(< append_journal_plan.h)" == *SGPL_JOURNAL_BITMAP_PROTOCOL* ]];then
    echo "CONTRACT feature=$feature mode=$mode width=$width merge=heap_control"
    SGPL_FUSION_C_HEAP_MERGE=1 SGPL_TEST_ALLOC_STACK=1 ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer-20)" ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_tmp/runtime"
   fi
   if [[ "$feature" == append_journal ]] && [[ "$(< append_journal_plan.h)" == *SGPL_JOURNAL_PROVEN_APPEND* ]];then
    echo "CONTRACT feature=$feature mode=$mode width=$width producer=checked_control"
    SGPL_FUSION_C_CHECKED_APPEND=1 SGPL_TEST_ALLOC_STACK=1 ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer-20)" ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_tmp/runtime"
    SGPL_FUSION_C_CHECKED_APPEND=1 SGPL_FUSION_C_HEAP_MERGE=1 SGPL_TEST_ALLOC_STACK=1 ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer-20)" ASAN_OPTIONS=detect_leaks=0 python3 test/physical_run.py "$width" "$task_tmp/runtime"
   fi
  done
 done
done
bash test/run_context_copy_tests.sh
bash test/run_static_schedule_tests.sh
bash test/run_cursor_alignment_tests.sh
SGPL_FULLGRAPH_NATIVE_ONLY=1 bash test/run_fullgraph_tests.sh
[[ "${1:-}" != native-only ]] || exit 0
python3 test/check_fullgraph.py
[[ ! -f test/check_execution_fusion.py ]] || python3 test/check_execution_fusion.py
