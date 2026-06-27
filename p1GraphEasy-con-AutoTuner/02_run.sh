#!/usr/bin/env bash
set -euxo pipefail

# ─── LLVM toolchain ────────────────────────────────────────────────────────────
LLVM_PREFIX="${LLVM_PREFIX:-/usr/local/llvm-20-polly-rtti}"
if [[ -x "$LLVM_PREFIX/bin/llvm-config" ]]; then
  LLVM_CONFIG="${LLVM_CONFIG_BIN:-$LLVM_PREFIX/bin/llvm-config}"
  OPT_BIN="${OPT_BIN:-$LLVM_PREFIX/bin/opt}"
  CLANGXX="${CLANGXX_BIN:-$LLVM_PREFIX/bin/clang++}"
  CLANG_BIN="${CLANG_BIN:-$LLVM_PREFIX/bin/clang}"
else
  LLVM_CONFIG="${LLVM_CONFIG_BIN:-llvm-config-20}"
  OPT_BIN="${OPT_BIN:-opt-20}"
  CLANGXX="${CLANGXX_BIN:-clang++-20}"
  CLANG_BIN="${CLANG_BIN:-clang-20}"
fi
CXX_BIN="${CXX_BIN:-g++}"

# ────────────────────────────────────────────────────────────────────────────────

# Usage: ./run.sh <input.gpl> <dsl-input> [existing-IR.ll]
if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <input.gpl> <dsl-input> [existing-IR.ll]"
  exit 1
fi

GPL_SRC="$1"
GP_INPUT="$2"
IR_OVERRIDE="${3:-}"
BASE_GPL="${GPL_SRC%.*}"

# Determine IR source
if [[ -n "$IR_OVERRIDE" ]]; then
  IR_SRC="$IR_OVERRIDE"
  BASE="${IR_OVERRIDE%.*}"
  echo ">>> Using existing IR: ${IR_SRC}"
else
  IR_SRC="${BASE_GPL}.ll"
  BASE="$BASE_GPL"
  echo ">>> Will generate IR to: ${IR_SRC}"
fi

echo ">>> DSL input: ${GP_INPUT}"

# ----------------------------------------------------------------------------- 
# 1) ANTLR generation (if needed)
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  if [[ ! -d generated ]]; then
    echo "=== [1] Generating ANTLR parser/lexer ==="
    antlr4 -Dlanguage=Cpp -visitor Base.g4 -o generated
  else
    echo "=== [1] Skipping ANTLR generation (generated/ exists)"
  fi
fi

# ----------------------------------------------------------------------------- 
# 2) Compile GraphProgram
# -----------------------------------------------------------------------------



if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [2] Compiling GraphProgram ==="

  RAW_LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"
  LLVM_CXXFLAGS="${RAW_LLVM_CXXFLAGS//-fno-exceptions/}"
  LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"
  LLVM_LIBS="$($LLVM_CONFIG --libs core irreader analysis passes executionengine mcjit native support)"
  LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"

  if [[ -n "${ANTLR_INCLUDE:-}" ]]; then
    :
  elif [[ -d /usr/local/include/antlr4-runtime ]]; then
    ANTLR_INCLUDE="-I/usr/local/include/antlr4-runtime"
  else
    ANTLR_INCLUDE="-I/usr/include/antlr4-runtime"
  fi

  echo "=== [2/5] Build runtime LLVM IR ==="
  "${CLANG_BIN}" -S -emit-llvm -O2 autotuner_runtime.c -o autotuner_runtime.ll
  "${CLANG_BIN}" -S -emit-llvm -O2 graph_mutation_runtime.c -o graph_mutation_runtime.ll
  "${CLANG_BIN}" -x c++ -S -emit-llvm -O2 bfs_runtime.cpp -o bfs_runtime.ll
  "${CLANG_BIN}" -x c++ -S -emit-llvm -O2 bfs_runtime_src.cpp -o bfs_runtime_src.ll
  "${CLANG_BIN}" -x c++ -S -emit-llvm -O2 dfs_runtime.cpp -o dfs_runtime.ll
  "${CLANG_BIN}" -x c++ -S -emit-llvm -O2 dfs_runtime_src.cpp -o dfs_runtime_src.ll


  "$CXX_BIN" \
    -g -std=c++17 -fexceptions \
    -mavx2 -march=native \
    $ANTLR_INCLUDE \
    -Igenerated -I. \
    $LLVM_CXXFLAGS \
    -pthread \
    main.cpp IRGenVisitor.cpp ASTBuilder.cpp pdg.cpp parallel_loop_outline.cpp SemanticAnalyzer.cpp roaring_bitmap.cpp AutoTunerPass.cpp\
    generated/*.cpp \
    $LLVM_LDFLAGS \
    -lantlr4-runtime \
    $LLVM_LIBS \
    $LLVM_SYSTEM_LIBS \
    -o GraphProgram

  echo ">>> GraphProgram build complete"
fi
