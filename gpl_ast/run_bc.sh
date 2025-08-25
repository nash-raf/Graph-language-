#!/usr/bin/env bash
set -euxo pipefail

# ─── LLVM-18 toolchain ─────────────────────────────────────────────────────────
LLVM_CONFIG="llvm-config-18"
OPT_BIN="opt-18"
CLANGXX="clang++-18"
LLVM_LINK="llvm-link-18"   # try this first; fallback to llvm-link if not present
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

# Determine IR source file name (textual .ll)
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
# 2) Compile GraphProgram (the compiler)
# ----------------------------------------------------------------------------- 
if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [2] Compiling GraphProgram ==="
  gcc -c runtime.c -o runtime.o || true    # keep this if you actually have runtime.c; tolerate missing

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
    main.cpp IRGenVisitor.cpp ASTBuilder.cpp \
    generated/*.cpp runtime.o \
    $LLVM_LDFLAGS \
    -lantlr4-runtime \
    $LLVM_LIBS \
    $LLVM_SYSTEM_LIBS \
    -o GraphProgram

  echo ">>> GraphProgram build complete"
fi

# ----------------------------------------------------------------------------- 
# 3) Run GraphProgram to emit textual IR to $IR_SRC
#    (your main prints textual IR to stdout when not run with --run)
# ----------------------------------------------------------------------------- 
if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [3] Generating textual IR (${IR_SRC}) by running GraphProgram ==="
  # Run the compiler and capture textual IR to file
  ./GraphProgram "${GPL_SRC}" > "${IR_SRC}"
  echo ">>> IR written to ${IR_SRC}"
else
  echo "=== [3] Skipping IR generation (IR override provided) ==="
fi

# ----------------------------------------------------------------------------- 
# 4) Compile bfs_runtime.cpp to bitcode and link with generated IR
# ----------------------------------------------------------------------------- 
# produce runtime bitcode
RUNTIME_CPP="bfs_runtime.cpp"
RUNTIME_BC="bfs_runtime.bc"
LINKED_BC="${BASE}-linked.bc"
EXE="${BASE}-exe"

echo "=== [4] Compiling runtime -> bitcode ==="
$CLANGXX -O2 -std=c++17 -emit-llvm -c "${RUNTIME_CPP}" -o "${RUNTIME_BC}"

# llvm-link tool name fallback check
if ! command -v "${LLVM_LINK}" >/dev/null 2>&1; then
  LLVM_LINK="llvm-link"
fi

echo "=== [4.1] Linking generated IR + runtime bitcode ==="
# If generated IR is textual, turn it into bitcode if necessary using llvm-as
if file "${IR_SRC}" | grep -qi "LLVM bitcode"; then
  # IR_SRC already bitcode
  "${LLVM_LINK}" "${IR_SRC}" "${RUNTIME_BC}" -o "${LINKED_BC}"
else
  # IR_SRC is textual .ll -> assemble then link
  TMP_BC="${IR_SRC%.ll}.bc"
  llvm-as -opaque-pointers "${IR_SRC}" -o "${TMP_BC}"

  "${LLVM_LINK}" "${TMP_BC}" "${RUNTIME_BC}" -o "${LINKED_BC}"
fi

# ----------------------------------------------------------------------------- 
# 5) Compile linked bitcode to native executable and run with DSL input
# ----------------------------------------------------------------------------- 
echo "=== [5] Building native executable from linked bitcode ==="
$CLANGXX "${LINKED_BC}" -O2 -o "${EXE}"

echo "=== [6] Running executable ==="
./"${EXE}" "${GP_INPUT}"
