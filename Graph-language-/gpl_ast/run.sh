#!/usr/bin/env bash
set -e

# 1) regenerate the ANTLR C++ sources
antlr4 \
  -Dlanguage=Cpp \
  -visitor \
  -no-listener \
  Base.g4 \
  -o generated

# 2) get LLVM compiler/linker flags
LLVM_CXXFLAGS=$(llvm-config --cxxflags)
LLVM_LDFLAGS=$(llvm-config --ldflags --system-libs --libs core orcjit native)

# 3) compile GraphProgram
g++ -std=c++17 \
    $LLVM_CXXFLAGS \
    -I/usr/include/antlr4-runtime \
    -Igenerated \
    -I. \
    main.cpp \
    ASTBuilder.cpp \
    IRGenVisitor.cpp \
    generated/*.cpp \
    -lantlr4-runtime \
    $LLVM_LDFLAGS \
    -o GraphProgram

echo "Build done."

# 4a) Run it to dump LLVM IR
echo -e "\n=== Dumping LLVM IR to program.ll ==="
./GraphProgram > program.ll

# 4b) Option A: interpret IR with lli
echo -e "\n=== Running with lli ==="
lli program.ll

# 4c) Option B: compile IR to native executable
echo -e "\n=== Compiling IR to object & linking ==="
llc -filetype=obj program.ll -o program.o
clang++ program.o -o program_native
echo "→ You can now run './program_native' for a fully native build."
