#!/usr/bin/env bash
set -euxo pipefail

# ─── LLVM-18 toolchain ─────────────────────────────────────────────────────────
LLVM_CONFIG="llvm-config-18"
OPT_BIN="opt-18"
CLANGXX="clang++-18"
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
  gcc -c runtime.c -o runtime.o

  RAW_LLVM_CXXFLAGS="$($LLVM_CONFIG --cxxflags)"
  LLVM_CXXFLAGS="${RAW_LLVM_CXXFLAGS//-fno-exceptions/}"
  LLVM_LDFLAGS="$($LLVM_CONFIG --ldflags)"
  LLVM_LIBS="$($LLVM_CONFIG --libs core irreader analysis passes executionengine mcjit native support)"
  LLVM_SYSTEM_LIBS="$($LLVM_CONFIG --system-libs)"

  ANTLR_INCLUDE="-I/usr/include/antlr4-runtime"

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
./GraphProgram test.graph

# # ----------------------------------------------------------------------------- 
# # 3) Build LLVM pass plugin
# # -----------------------------------------------------------------------------
# echo "=== [3] Building LLVM pass plugin ==="
# $CLANGXX -v -fPIC -shared \
#   DependencyGraphBuilder.cpp LoopClassifier.cpp \
#   -o CombinedPlugin.so \
#   $($LLVM_CONFIG --cxxflags) \
#   $($LLVM_CONFIG --ldflags --system-libs --libs core analysis passes support)
# echo ">>> Plugin CombinedPlugin.so built"

# # ----------------------------------------------------------------------------- 
# # 4) Generate LLVM IR (no awk filtering!)
# # -----------------------------------------------------------------------------
# if [[ -z "$IR_OVERRIDE" ]]; then
#   echo "=== [4] Generating LLVM IR from ${GPL_SRC} ==="
#   # <<< replace the awk pipeline with a direct redirect:
#   ./GraphProgram "${GPL_SRC}" > "${IR_SRC}"

#   file "${IR_SRC}"
#   echo ">>> IR written to ${IR_SRC}"

#   if file "${IR_SRC}" | grep -q "bitcode"; then
#     echo "=== [4.1] Disassembling bitcode to textual IR"
#     TMP="${IR_SRC%.ll}.text.ll"
#     llvm-dis "${IR_SRC}" -o "${TMP}"
#     mv "${TMP}" "${IR_SRC}"
#     echo ">>> Disassembled IR written to ${IR_SRC}"
#   fi
# else
#   echo "=== [4] Skipping IR generation (using override) ==="
# fi

# # ----------------------------------------------------------------------------- 
# # 5) Run LLVM passes
# # -----------------------------------------------------------------------------
# DOT_OUT="${BASE}-depgraph.dot"
# CLASS_OUT="${BASE}-loop-classes.txt"
# PASS_LOG="${BASE}-pass.log"

# echo "=== [5] Running LLVM passes on ${IR_SRC} ==="
# $OPT_BIN -load-pass-plugin=./CombinedPlugin.so \
#     -passes="module(dep-graph-builder,function(loop-classify-print,loop-metadata-injector))" \
#     "${IR_SRC}" -S -o "${BASE}-optimized.ll" \
#   2>&1 | tee "${PASS_LOG}"

# echo ">>> opt finished"

# # ----------------------------------------------------------------------------- 
# # 6) Post-process pass output
# # -----------------------------------------------------------------------------
# if [[ -f "${PASS_LOG}" ]]; then
#   echo "=== [6] Extracting dependency graph to ${DOT_OUT} ==="
#   awk 'BEGIN{p=0} /^digraph/{p=1} p{print} /^}/{ if(p) exit }' "${PASS_LOG}" > "${DOT_OUT}"
#   echo "Dependency graph written to: ${DOT_OUT}"

#   echo "=== [6] Extracting loop classifications to ${CLASS_OUT} ==="
#   if grep -q "^  Loop at block" "${PASS_LOG}"; then
#     grep "^  Loop at block" "${PASS_LOG}" | tee "${CLASS_OUT}"
#     echo "Loop classifications written to: ${CLASS_OUT}"
#   else
#     echo "(no 'Loop at block' lines found)"
#   fi
# else
#   echo "Warning: pass log ${PASS_LOG} not found; skipping."
# fi

# # ----------------------------------------------------------------------------- 
# # 7) Render graph
# # -----------------------------------------------------------------------------
# echo "=== [7] Rendering dependency graph ==="
# if command -v dot &>/dev/null; then
#   PNG_OUT="${BASE}-depgraph.png"
#   dot -Tpng "${DOT_OUT}" -o "${PNG_OUT}" && echo "Rendered graph to: ${PNG_OUT}"
# else
#   echo "dot not found; skipping render."
# fi

# # ----------------------------------------------------------------------------- 
# # 8) Run GraphProgram on DSL input
# # -----------------------------------------------------------------------------
# if [[ -z "$IR_OVERRIDE" ]]; then
#   echo "=== [8] Running GraphProgram on ${GP_INPUT} ==="
#   export LD_LIBRARY_PATH="/usr/lib:${LD_LIBRARY_PATH:-}"
#   ./GraphProgram --run "${GP_INPUT}"
# else
#   echo "=== [8] Skipping DSL run (IR override provided) ==="
# fi
