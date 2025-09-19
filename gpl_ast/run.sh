#!/usr/bin/env bash
set -euxo pipefail

# ─── LLVM-18 toolchain ─────────────────────────────────────────────────────────
LLVM_CONFIG="llvm-config-20"
CLANGXX="clang++-20"
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
# 2) Compile GraphProgram (with Polly support)
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [2] Compiling GraphProgram ==="
  gcc -c runtime.c -o runtime.o

  RAW_LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"
  LLVM_CXXFLAGS="${RAW_LLVM_CXXFLAGS//-fno-exceptions/}"
  LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"
  LLVM_LIBS="$($LLVM_CONFIG --libs core irreader analysis passes executionengine mcjit native support)"
  LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"

  ANTLR_INCLUDE="-I/usr/include/antlr4-runtime"

  g++ \
    -g -std=c++17 -fexceptions -fopenmp \
    $ANTLR_INCLUDE \
    -Igenerated -I. \
    $LLVM_CXXFLAGS \
    -pthread \
    main.cpp IRGenVisitor.cpp ASTBuilder.cpp \
    generated/*.cpp runtime.o \
    $LLVM_LDFLAGS \
    -lantlr4-runtime \
    $LLVM_LIBS \
    $LLVM_SYSTEM_LIBS \
    -lPolly -lPollyISL -lisl \
    -o GraphProgram

  echo ">>> GraphProgram build complete"
fi

# ----------------------------------------------------------------------------- 
# 3) Run GraphProgram
# -----------------------------------------------------------------------------
echo "=== [3] Running GraphProgram ==="
./GraphProgram "$GPL_SRC" > "$IR_SRC"
echo ">>> IR written to $IR_SRC"

# ----------------------------------------------------------------------------- 
# 4) Run DSL input through GraphProgram (with Polly inside main)
# -----------------------------------------------------------------------------
echo "=== [4] Running GraphProgram on DSL input ${GP_INPUT} ==="
./GraphProgram "$GP_INPUT"