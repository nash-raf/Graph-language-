#!/usr/bin/env bash
set -euxo pipefail

# ─── LLVM-18 toolchain ─────────────────────────────────────────────────────────
# LLVM_CONFIG="llvm-config-20"
# CLANGXX="clang++-20"
LLVM_CONFIG=/usr/local/llvm-20-polly-rtti/bin/llvm-config
CLANGXX=/usr/local/llvm-20-polly-rtti/bin/clang++

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
  if [[ ! -d generated ]] || [[ Base.g4 -nt generated/BaseParser.h ]]; then
    echo "=== [1] Generating ANTLR parser/lexer ==="
    antlr4 -Dlanguage=Cpp -visitor Base.g4 -o generated
  else
    echo "=== [1] Skipping ANTLR generation (generated/ up-to-date)"
  fi
fi

# ----------------------------------------------------------------------------- 
# 2) Compile GraphProgram (with Polly support)
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [2] Compiling GraphProgram ==="
  gcc -c runtime.c -o runtime.o

  RAW_LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"

  LLVM_CXXFLAGS="$RAW_LLVM_CXXFLAGS"
  # Allow exceptions for our code and ANTLR, but
  # keep LLVM's -fno-rtti to match its build.
  LLVM_CXXFLAGS="${LLVM_CXXFLAGS//-fno-exceptions/}"

  LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"
  LLVM_LIBS="$($LLVM_CONFIG --libs all)"
  LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"

  ANTLR_INCLUDE="-I/usr/local/include/antlr4-runtime"
  # -L/usr/lib64 -lomp \

  g++ \
  -O3 -mavx2 -march=native \
  -fopenmp \
    -g -std=c++17 -fopenmp \
    $ANTLR_INCLUDE \
    -Igenerated -I. \
    $LLVM_CXXFLAGS \
    -fexceptions \
    -pthread \
    main.cpp IRGenVisitor.cpp ASTBuilder.cpp SemanticAnalyzer.cpp roaring_bitmap.cpp AutoTunerPass.cpp\
    generated/*.cpp runtime.o \
    $LLVM_LDFLAGS \
    -lantlr4-runtime \
    -lPolly -lPollyISL -lisl \
    $LLVM_LIBS \
    $LLVM_SYSTEM_LIBS \
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