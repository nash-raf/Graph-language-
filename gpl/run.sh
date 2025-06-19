#!/usr/bin/env bash
set -euxo pipefail

# Usage: ./run.sh <input.gpl> <graph-program-input>
if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <input.gpl> <graph-program-input>"
  exit 1
fi

GPL_SRC="$1"
GP_INPUT="$2"
BASE="${GPL_SRC%.*}"

echo ">>> Starting run.sh for ${GPL_SRC} / ${GP_INPUT}"

# -----------------------------------------------------------------------------
# 1) Generate parser/lexer from Base.g4 (only if generated/ is missing)
# -----------------------------------------------------------------------------
if [[ ! -d generated ]]; then
  echo "=== [1] Generating ANTLR parser/lexer ==="
  antlr4 -Dlanguage=Cpp -visitor Base.g4 -o generated
else
  echo "=== [1] Skipping ANTLR generation (generated/ exists)"
fi

# -----------------------------------------------------------------------------
# 2) Compile GraphProgram (your GPL frontend) with LLVM linkage
# -----------------------------------------------------------------------------
echo "=== [2] Compiling GraphProgram ==="
gcc -c runtime.c -o runtime.o
c++ -std=c++17 \
    -I/usr/include/antlr4-runtime -Igenerated \
    $(llvm-config --cxxflags) \
    -pthread \
    main.cpp node.cpp edge.cpp graph.cpp graph_algorithm.cpp generated/*.cpp runtime.o \
    -lantlr4-runtime \
    $(llvm-config --ldflags --system-libs --libs core irreader analysis passes executionengine mcjit native support) \
    -o GraphProgram

echo ">>> GraphProgram build complete"

# -----------------------------------------------------------------------------
# 3) Build LLVM pass plugin (fixed build command)
# -----------------------------------------------------------------------------
echo "=== [3] Building LLVM pass plugin ==="
clang++ -v -fPIC -shared \
  DependencyGraphBuilder.cpp LoopClassifier.cpp \
  -o CombinedPlugin.so \
  $(llvm-config --cxxflags) \
  $(llvm-config --ldflags --system-libs --libs core analysis passes support)

echo ">>> Plugin CombinedPlugin.so built"

# -----------------------------------------------------------------------------
# 4) Generate LLVM IR
# -----------------------------------------------------------------------------
echo "=== [4] Generating LLVM IR from ${GPL_SRC} ==="
./GraphProgram "${GPL_SRC}" | awk '
  /^=== Generated LLVM IR ===/ { p=1; next }
  /^===/                      { p=0 }
  p
' > "${BASE}.ll"

file "${BASE}.ll"
echo ">>> IR written to ${BASE}.ll"

# If bitcode, disassemble
if file "${BASE}.ll" | grep -q "bitcode"; then
  echo "=== [4.1] Converting bitcode to textual IR"
  llvm-dis "${BASE}.ll" -o "${BASE}.text.ll"
  mv "${BASE}.text.ll" "${BASE}.ll"
  echo ">>> Disassembled IR written to ${BASE}.ll"
fi

# -----------------------------------------------------------------------------
# 5) Run analysis and transformation passes (updated pipeline)
# -----------------------------------------------------------------------------
DOT_OUT="${BASE}-depgraph.dot"
CLASS_OUT="${BASE}-loop-classes.txt"
PASS_LOG="${BASE}-pass.log"

echo "=== [5] Running LLVM passes on ${BASE}.ll ==="
which opt; opt --version

opt -load-pass-plugin=./CombinedPlugin.so \
    -passes="module(dep-graph-builder,function(loop-classify-print,loop-metadata-injector))" \
    "${BASE}.ll" -S -o "${BASE}-optimized.ll"



echo ">>> opt finished, return code $?"

# -----------------------------------------------------------------------------
# 6) Post-process outputs
# -----------------------------------------------------------------------------
echo "=== [6] Extracting dependency graph to ${DOT_OUT} ==="
awk 'BEGIN{p=0} /^digraph/{p=1} p{print} /^}/{ if(p) exit }' "${PASS_LOG}" > "${DOT_OUT}"
echo "Dependency graph written to: ${DOT_OUT}"

echo "=== [6] Extracting loop classifications to ${CLASS_OUT} ==="
if grep -q "^  Loop at block" "${PASS_LOG}"; then
  grep "^  Loop at block" "${PASS_LOG}" | tee "${CLASS_OUT}"
  echo "Loop classifications written to: ${CLASS_OUT}"
else
  echo "(no 'Loop at block' lines found in pass log)"
  echo "---- full pass log ----"
  cat "${PASS_LOG}"
  echo "-----------------------"
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
# 8) Run GraphProgram on DSL input
# -----------------------------------------------------------------------------
echo "=== [8] Running GraphProgram on ${GP_INPUT} ==="
export LD_LIBRARY_PATH="/usr/lib:${LD_LIBRARY_PATH:-}"
./GraphProgram "${GP_INPUT}"

echo ">>> run.sh completed successfully"
