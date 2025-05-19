#!/usr/bin/env bash
set -e

# -----------------------------------------------------------------------------
# 1) Locate ANTLR JARs (Fedora defaults under /usr/share/java) 
# -----------------------------------------------------------------------------
# Tool jar (generates parser/lexer)
: "${ANTLR_TOOL_JAR:=$(ls /usr/share/java/antlr4/antlr4.jar 2>/dev/null)}"
# Runtime jar (contains runtime classes—used only for Java; C++ runtime is separate)
: "${ANTLR_RUNTIME_JAR:=$(ls /usr/share/java/antlr4/antlr4-runtime.jar 2>/dev/null)}"

if [[ ! -f "$ANTLR_TOOL_JAR" ]]; then
  echo "Error: antlr4.jar not found. Install via 'sudo dnf install antlr4' or adjust path."
  exit 1
fi

if [[ ! -f "$ANTLR_RUNTIME_JAR" ]]; then
  echo "Error: antlr4-runtime.jar not found. Install via 'sudo dnf install antlr4-runtime' or adjust path."
  exit 1
fi

echo "Using ANTLR tool jar:    $ANTLR_TOOL_JAR"
echo "Using ANTLR runtime jar: $ANTLR_RUNTIME_JAR"

# -----------------------------------------------------------------------------
# 2) Generate C++ parser/lexer
# -----------------------------------------------------------------------------
GRAMMAR=Base.g4
GEN_DIR=generated

echo "=== Generating parser/lexer from ${GRAMMAR} ==="
antlr4 -Dlanguage=Cpp -visitor Base.g4 -o generated


# -----------------------------------------------------------------------------
# 3) Gather build flags (ANTLR4 C++ runtime + LLVM)
# -----------------------------------------------------------------------------
# C++ runtime include and link flags (installed via antlr4-cpp-runtime-devel)
ANTLR_CXXFLAGS="-I/usr/include/antlr4-runtime -I${GEN_DIR}"
ANTLR_LDFLAGS="-lantlr4-runtime"

# LLVM flags (if llvm-config is present)
if command -v llvm-config &>/dev/null; then
  LLVM_CXXFLAGS="$(llvm-config --cxxflags)"
  LLVM_LDFLAGS="$(llvm-config --ldflags) $(llvm-config --system-libs) $(llvm-config --libs core executionengine mcjit interpreter native)"
else
  echo "Warning: llvm-config not found; skipping LLVM integration flags."
  LLVM_CXXFLAGS=""
  LLVM_LDFLAGS=""
fi

# -----------------------------------------------------------------------------
# 4) Compile and link
# -----------------------------------------------------------------------------
echo "=== Compiling and linking ==="
gcc -c runtime.c -o runtime.o
g++ -std=c++17 \
    ${ANTLR_CXXFLAGS} ${LLVM_CXXFLAGS} \
    -pthread \
    main.cpp node.cpp edge.cpp graph.cpp graph_algorithm.cpp ${GEN_DIR}/*.cpp runtime.o \
    -o GraphProgram \
    ${ANTLR_LDFLAGS} ${LLVM_LDFLAGS}

# -----------------------------------------------------------------------------
# 5) Run the program
# -----------------------------------------------------------------------------
INPUT_FILE=${1:-graph_code_test.txt}
echo
echo "=== Running GraphProgram on ${INPUT_FILE} ==="
# Ensure the C++ runtime can be found at runtime
export LD_LIBRARY_PATH="/usr/lib:${LD_LIBRARY_PATH}"
./GraphProgram "${INPUT_FILE}"
