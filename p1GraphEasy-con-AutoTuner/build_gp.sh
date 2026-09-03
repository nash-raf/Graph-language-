#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CXXFLAGS="$(llvm-config-20 --cxxflags)"
CXXFLAGS="${CXXFLAGS//-fno-exceptions/}"
LDFLAGS="$(llvm-config-20 --ldflags)"
LIBS="$(llvm-config-20 --libs core irreader analysis passes executionengine mcjit native support)"
NVPTX_LIBS="$(llvm-config-20 --libs nvptxcodegen nvptxdesc nvptxinfo 2>/dev/null || true)"
SYS_LIBS="$(llvm-config-20 --system-libs)"
g++ -O2 -mavx2 -march=native -std=c++17 -fexceptions -pthread \
  -I/usr/include/antlr4-runtime -Igenerated -iquote . -I/usr/lib/llvm-20/include/polly \
  $CXXFLAGS \
  main.cpp IRGenVisitor.cpp ASTBuilder.cpp graph_frontier_lowering.cpp pdg.cpp parallel_loop_outline.cpp \
  SemanticAnalyzer.cpp roaring_bitmap.cpp AutoTunerPass.cpp generated/*.cpp \
  $LDFLAGS -lantlr4-runtime $LIBS $NVPTX_LIBS $SYS_LIBS \
  -o GraphProgram
echo "GraphProgram linked OK"