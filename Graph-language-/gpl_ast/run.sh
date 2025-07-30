#!/usr/bin/env bash
set -euxo pipefail

# Usage: ./run.sh <input.gpl> <dsl-input> [existing-IR.ll]
if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <input.gpl> <dsl-input> [existing-IR.ll]"
  exit 1
fi

GPL_SRC="$1"
GP_INPUT="$2"
IR_OVERRIDE="${3:-}"      # optional third arg, defaults to empty string
BASE_GPL="${GPL_SRC%.*}"

# Determine IR source and base name
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
# 1) Generate parser/lexer from Base.g4 (only if generated/ is missing)
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
# 2) Compile GraphProgram (your GPL frontend) with LLVM linkage
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [2] Compiling GraphProgram ==="
  gcc -c runtime.c -o runtime.o
  c++ -std=c++17 \
      -I/usr/include/antlr4-runtime -Igenerated \
      $(llvm-config --cxxflags) \
      -pthread \
      main.cpp IRGenVisitor.cpp ASTBuilder.cpp generated/*.cpp runtime.o \
      -lantlr4-runtime \
      $(llvm-config --ldflags --system-libs --libs core irreader analysis passes executionengine mcjit native support) \
      -o GraphProgram
  echo ">>> GraphProgram build complete"
fi

# -----------------------------------------------------------------------------
# 3) Build LLVM pass plugin
# -----------------------------------------------------------------------------
echo "=== [3] Building LLVM pass plugin ==="
clang++ -v -fPIC -shared \
  DependencyGraphBuilder.cpp LoopClassifier.cpp \
  -o CombinedPlugin.so \
  $(llvm-config --cxxflags) \
  $(llvm-config --ldflags --system-libs --libs core analysis passes support)
echo ">>> Plugin CombinedPlugin.so built"

# -----------------------------------------------------------------------------
# 4) Generate LLVM IR (skip if IR_OVERRIDE)
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [4] Generating LLVM IR from ${GPL_SRC} ==="
  ./GraphProgram "${GPL_SRC}" | awk '
    /^=== Generated LLVM IR ===/ { p=1; next }
    /^===/                      { p=0 }
    p
  ' > "${IR_SRC}"

  file "${IR_SRC}"
  echo ">>> IR written to ${IR_SRC}"

  # If bitcode, disassemble
  if file "${IR_SRC}" | grep -q "bitcode"; then
    echo "=== [4.1] Converting bitcode to textual IR"
    TMP="${IR_SRC%.ll}.text.ll"
    llvm-dis "${IR_SRC}" -o "${TMP}"
    mv "${TMP}" "${IR_SRC}"
    echo ">>> Disassembled IR written to ${IR_SRC}"
  fi
else
  echo "=== [4] Skipping IR generation (using override) ==="
fi

# -----------------------------------------------------------------------------
# 5) Run analysis and transformation passes
# -----------------------------------------------------------------------------
DOT_OUT="${BASE}-depgraph.dot"
CLASS_OUT="${BASE}-loop-classes.txt"
PASS_LOG="${BASE}-pass.log"

echo "=== [5] Running LLVM passes on ${IR_SRC} ==="
which opt; opt --version

# Capture all opt output (stdout+stderr) into PASS_LOG
opt -load-pass-plugin=./CombinedPlugin.so \
    -passes="module(dep-graph-builder,function(loop-classify-print,loop-metadata-injector))" \
    "${IR_SRC}" -S -o "${BASE}-optimized.ll" \
  2>&1 | tee "${PASS_LOG}"

echo ">>> opt finished, return code $?"

# -----------------------------------------------------------------------------
# 6) Post-process outputs (if pass log exists)
# -----------------------------------------------------------------------------
if [[ -f "${PASS_LOG}" ]]; then
  echo "=== [6] Extracting dependency graph to ${DOT_OUT} ==="
  awk 'BEGIN{p=0} /^digraph/{p=1} p{print} /^}/{ if(p) exit }' "${PASS_LOG}" > "${DOT_OUT}"
  echo "Dependency graph written to: ${DOT_OUT}"

  echo "=== [6] Extracting loop classifications to ${CLASS_OUT} ==="
  if grep -q "^  Loop at block" "${PASS_LOG}"; then
    grep "^  Loop at block" "${PASS_LOG}" | tee "${CLASS_OUT}"
    echo "Loop classifications written to: ${CLASS_OUT}"
  else
    echo "(no 'Loop at block' lines found in pass log)"
  fi
else
  echo "Warning: pass log ${PASS_LOG} not found; skipping post-processing."
fi

# -----------------------------------------------------------------------------
# 7) Render dependency graph
# -----------------------------------------------------------------------------
echo "=== [7] Rendering dependency graph ==="
if command -v dot &>/dev/null; then
  PNG_OUT="${BASE}-depgraph.png"
  if dot -Tpng "${DOT_OUT}" -o "${PNG_OUT}"; then
    echo "Rendered graph to: ${PNG_OUT}"
  else
    echo "Failed to render DOT; skipping."
  fi
else
  echo "dot not found; skipping graph render."
fi

# -----------------------------------------------------------------------------
# 8) Run GraphProgram on DSL input (if built)
# -----------------------------------------------------------------------------
if [[ -z "$IR_OVERRIDE" ]]; then
  echo "=== [8] Running GraphProgram on ${GP_INPUT} ==="
  export LD_LIBRARY_PATH="/usr/lib:${LD_LIBRARY_PATH:-}"
  ./GraphProgram "${GP_INPUT}"
else
  echo "=== [8] Skipping DSL run (IR override provided) ==="
fi

echo ">>> run.sh completed successfully"
