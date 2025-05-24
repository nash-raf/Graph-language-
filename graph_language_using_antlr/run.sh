#!/usr/bin/env bash
set -e

# Usage: ./run.sh <input.gpl> <graph-program-input>
if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <input.gpl> <graph-program-input>"
  exit 1
fi

GPL_SRC="$1"
GP_INPUT="$2"
BASE="${GPL_SRC%.*}"

# -----------------------------------------------------------------------------
# 1) Generate parser/lexer from Base.g4 (only if generated/ is missing)
# -----------------------------------------------------------------------------
if [[ ! -d generated ]]; then
  echo "=== Generating parser/lexer from Base.g4 ==="
  antlr4 -Dlanguage=Cpp -visitor Base.g4 -o generated
fi

# -----------------------------------------------------------------------------
# 2) Compile GraphProgram (your GPL frontend) with LLVM linkage
# -----------------------------------------------------------------------------
echo "=== Compiling GraphProgram ==="
gcc -c runtime.c -o runtime.o
c++ -std=c++17 \
    -I/usr/include/antlr4-runtime -Igenerated \
    $(llvm-config --cxxflags) \
    -pthread \
    main.cpp node.cpp edge.cpp graph.cpp graph_algorithm.cpp generated/*.cpp runtime.o \
    -lantlr4-runtime \
    $(llvm-config --ldflags --system-libs --libs core irreader analysis passes executionengine mcjit native support) \
    -o GraphProgram

# -----------------------------------------------------------------------------
# 3) Build LLVM pass plugin (no CMake)
# -----------------------------------------------------------------------------
echo "=== Building LLVM pass plugin ==="
clang++ -fPIC -shared DependencyGraphBuilder.cpp \
  -o libDepGraphBuilder.so \
  $(llvm-config --cxxflags --ldflags --system-libs --libs core analysis passes support)

# -----------------------------------------------------------------------------
# 4) Invoke GraphProgram and extract *only* the LLVM IR block into .ll
# -----------------------------------------------------------------------------
echo "=== Generating LLVM IR (.ll) from ${GPL_SRC} ==="
./GraphProgram "${GPL_SRC}" | awk '
  /^=== Generated LLVM IR ===/ { p=1; next }
  /^===/                      { p=0 }
  p
' > "${BASE}.ll"

# Ensure the output is text, not bitcode
if file "${BASE}.ll" | grep -q "bitcode"; then
  echo "Warning: Generated file is bitcode, not textual IR"
  llvm-dis "${BASE}.ll" -o "${BASE}.text.ll"
  mv "${BASE}.text.ll" "${BASE}.ll"
fi

# -----------------------------------------------------------------------------
# 5) Run the dependency-graph pass on the clean IR
# -----------------------------------------------------------------------------
DOT_OUT="${BASE}-depgraph.dot"
PASS_LOG="${BASE}-pass.log"

echo "=== Running DepGraphBuilder on ${BASE}.ll ==="

# Create a clean file to capture ONLY the DOT output
TEMP_DOT_OUT=$(mktemp)

# Run the pass and capture stderr (DOT output)
opt \
  -load-pass-plugin=./libDepGraphBuilder.so \
  -passes="dep-graph-builder" \
  "${BASE}.ll" -o /dev/null > "${PASS_LOG}" 2> "${TEMP_DOT_OUT}"

# Extract only the DOT content
awk 'BEGIN {p=0} 
  /^digraph/ {p=1} 
  {if(p) print} 
  /^}/ {if(p) p=0}' "${TEMP_DOT_OUT}" > "${DOT_OUT}"

# Validate the DOT file
if ! grep -q "^digraph" "${DOT_OUT}"; then
  echo "Error: Failed to extract valid DOT content"
  echo "Raw output:"
  cat "${TEMP_DOT_OUT}"
  echo "Removing temporary file..."
  rm "${TEMP_DOT_OUT}"
  exit 1
fi

echo "Dependency graph written to: ${DOT_OUT}"
echo "Pass log written to: ${PASS_LOG}"

# Clean up temp file
rm "${TEMP_DOT_OUT}"

# -----------------------------------------------------------------------------
# 6) (Optional) Render graph with Graphviz
# -----------------------------------------------------------------------------
if command -v dot &>/dev/null; then
  PNG_OUT="${BASE}-depgraph.png"
  echo "=== Rendering PNG ${PNG_OUT} ==="
  
  if dot -Tpng "${DOT_OUT}" -o "${PNG_OUT}" 2>/dev/null; then
    echo "Successfully rendered: ${PNG_OUT}"
  else
    echo "Error: Failed to render DOT file. Content of ${DOT_OUT}:"
    cat "${DOT_OUT}"
    echo "Trying alternate rendering approach..."
    
    # Create a minimal valid DOT file if the current one fails
    TMP_DOT=$(mktemp)
    echo "digraph fallback {" > "${TMP_DOT}"
    grep "node.*\[label=" "${DOT_OUT}" >> "${TMP_DOT}" 2>/dev/null || echo '  node0 [label="No valid data found"];' >> "${TMP_DOT}"
    grep "->.*\[label=" "${DOT_OUT}" >> "${TMP_DOT}" 2>/dev/null
    echo "}" >> "${TMP_DOT}"
    
    if dot -Tpng "${TMP_DOT}" -o "${PNG_OUT}" 2>/dev/null; then
      echo "Successfully rendered fallback: ${PNG_OUT}"
      # If fallback worked, replace the original DOT file
      cp "${TMP_DOT}" "${DOT_OUT}"
    else
      echo "CRITICAL ERROR: Both rendering approaches failed."
    fi
    
    rm "${TMP_DOT}"
  fi
fi

# -----------------------------------------------------------------------------
# 7) Finally, run GraphProgram on its DSL input
# -----------------------------------------------------------------------------
echo "=== Running GraphProgram on ${GP_INPUT} ==="
export LD_LIBRARY_PATH="/usr/lib:${LD_LIBRARY_PATH}"
./GraphProgram "${GP_INPUT}"