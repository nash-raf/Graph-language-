#!/usr/bin/env bash
set -euxo pipefail

GRAPH_FILE="${GRAPH_FILE:-test.graph}"
LLVM_PREFIX="${LLVM_PREFIX:-/usr/local/llvm-20-polly-rtti}"
if [[ -x "$LLVM_PREFIX/bin/clang" ]]; then
  CLANG_BIN="${CLANG_BIN:-$LLVM_PREFIX/bin/clang}"
else
  CLANG_BIN="${CLANG_BIN:-clang-20}"
fi

if [[ ! -x ./GraphProgram ]]; then
  echo "GraphProgram not found. Run ./1run.sh test.graph test.graph first." >&2
  exit 1
fi

g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime.cpp -o graph_loader_runtime.o
g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o
"$CLANG_BIN" -S -emit-llvm -O2 autotuner_runtime.c -o autotuner_runtime.ll
"$CLANG_BIN" -S -emit-llvm -O2 graph_mutation_runtime.c -o graph_mutation_runtime.ll

./GraphProgram "$GRAPH_FILE"
# ./GraphProgram --debug-polly "$GRAPH_FILE"
# ./GraphProgram "$GRAPH_FILE" -polly-parallel -polly-vectorizer=stripmine

g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o -fopenmp -no-pie -o final_program
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-4}"
ulimit -s unlimited
time ./final_program
