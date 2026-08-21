#!/usr/bin/env bash
set -euo pipefail

# One-command pipeline:
# 1) Build runtime LLVM IR
# 2) Build GraphProgram compiler
# 3) Compile DSL input (default: test.graph) -> program.o
# 4) Link executable final_program
# 5) Run executable

INPUT_GRAPH="${1:-test.graph}"
OUTPUT_BIN="${2:-final_program}"

LLVM_CONFIG_BIN="${LLVM_CONFIG_BIN:-llvm-config-20}"
CLANG_BIN="${CLANG_BIN:-clang-20}"
CLANGXX_BIN="${CLANGXX_BIN:-clang++-20}"
CXX_BIN="${CXX_BIN:-g++}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

echo "=== [1/5] Resolve LLVM flags ==="
LLVM_CXXFLAGS="$(${LLVM_CONFIG_BIN} --cxxflags)"
LLVM_LDFLAGS="$(${LLVM_CONFIG_BIN} --ldflags)"
LLVM_LIBS="$(${LLVM_CONFIG_BIN} --libs core irreader analysis passes executionengine mcjit native support)"
NVPTX_LIBS="$(${LLVM_CONFIG_BIN} --libs nvptxcodegen nvptxdesc nvptxinfo 2>/dev/null || true)"
LLVM_SYSTEM_LIBS="$(${LLVM_CONFIG_BIN} --system-libs)"

echo "=== [2/5] Build runtime LLVM IR ==="
"${CLANG_BIN}" -S -emit-llvm -O2 autotuner_runtime.c -o autotuner_runtime.ll
"${CLANG_BIN}" -S -emit-llvm -O2 graph_mutation_runtime.c -o graph_mutation_runtime.ll

echo "=== [3/5] Build GraphProgram ==="
"${CLANGXX_BIN}" -O3 -mavx2 -march=native -fopenmp=libomp -L/usr/lib64 -lomp -g -std=c++17 -fexceptions \
  -I/usr/include/antlr4-runtime -Igenerated -I. ${LLVM_CXXFLAGS} -pthread \
  main.cpp IRGenVisitor.cpp ASTBuilder.cpp SemanticAnalyzer.cpp AutoTunerPass.cpp \
  pdg.cpp parallel_loop_outline.cpp roaring_bitmap.cpp \
  generated/BaseBaseVisitor.cpp generated/BaseLexer.cpp generated/BaseListener.cpp \
  generated/BaseParser.cpp generated/BaseVisitor.cpp generated/BaseBaseListener.cpp \
  ${LLVM_LDFLAGS} -lantlr4-runtime ${LLVM_LIBS} ${NVPTX_LIBS} ${LLVM_SYSTEM_LIBS} \
  -o GraphProgram

echo "=== [4/5] Compile DSL: ${INPUT_GRAPH} ==="
./GraphProgram "${INPUT_GRAPH}"

echo "=== [5/5] Link + Run: ${OUTPUT_BIN} ==="
"${CLANG_BIN}" -O3 -c parallel_runtime.c -o parallel_runtime.o
"${CLANG_BIN}" -O3 -c gpu_runtime.c -o gpu_runtime.o
"${CXX_BIN}" -O3 -mavx2 -march=native -fopenmp \
  program.o graph_loader_runtime.cpp roaring_bitmap.cpp parallel_runtime.o gpu_runtime.o \
  -ldl -o "${OUTPUT_BIN}"

echo "=== Running ${OUTPUT_BIN} ==="
"./${OUTPUT_BIN}"
