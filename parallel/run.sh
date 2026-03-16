#!/usr/bin/env bash
set -euo pipefail

# ─── LLVM-18 toolchain ─────────────────────────────────────────────────────────
LLVM_CONFIG="llvm-config-18"
OPT_BIN="opt-18"
CLANGXX="clang++-18"
# ────────────────────────────────────────────────────────────────────────────────

# Usage: ./run.sh <input.gpl> <dsl-input> [existing-IR.ll]
if [[ $# -lt 2 ]]; then
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
else
  IR_SRC="${BASE_GPL}.ll"
  BASE="$BASE_GPL"
fi

# ----------------------------------------------------------------------------- 
# 1) ANTLR generation (if needed)
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  if [[ ! -d generated ]]; then
    antlr4 -Dlanguage=Cpp -visitor Base.g4 -o generated
  fi
fi

# ----------------------------------------------------------------------------- 
# 2) Compile GraphProgram
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  RAW_LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"
  LLVM_CXXFLAGS="${RAW_LLVM_CXXFLAGS//-fno-exceptions/}"
  LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"
  LLVM_LIBS="$($LLVM_CONFIG --libs core irreader analysis passes executionengine mcjit native support)"
  LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"

  ANTLR_INCLUDE="-I/usr/local/include/antlr4-runtime"

  g++ \
    -g -std=c++17 -fexceptions \
    $ANTLR_INCLUDE \
    -Igenerated -I. \
    $LLVM_CXXFLAGS \
    -pthread \
    main.cpp IRGenVisitor.cpp ASTBuilder.cpp pdg.cpp parallel_loop_outline.cpp\
    generated/*.cpp \
    $LLVM_LDFLAGS \
    -lantlr4-runtime \
    $LLVM_LIBS \
    $LLVM_SYSTEM_LIBS \
    -o GraphProgram

fi
./GraphProgram>test.ll test.graph
