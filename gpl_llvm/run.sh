# #!/bin/bash

# # Set these paths before running
# ANTLR4_RUNTIME_PATH=/usr/local/include/antlr4-runtime
# ANTLR4_LIB_PATH=/usr/local/lib

# # Step 1: Generate ANTLR C++ files from grammar
# echo "Generating parser and lexer from base.g4..."
# # java -jar /usr/local/lib/antlr-4.13.0-complete.jar -Dlanguage=Cpp -visitor Base.g4
# java -jar /usr/local/lib/antlr-4.10.1-complete.jar -Dlanguage=Cpp -visitor Base.g4 -o generated
# # antlr4 -Dlanguage=Cpp Addition.g4

# LLVM_DIR=$(brew --prefix llvm)
# if [ -z "$LLVM_DIR" ]; then
#     echo "LLVM not found. Please install it using 'brew install llvm'."
#     exit 1
# fi



# # Step 2: Compile the program
# echo "Compiling..."
# # g++ -std=c++17 -I/usr/local/include/antlr4-runtime -Igenerated -I. \
# #     -L/usr/local/lib \
# #     main.cpp  generated/*.cpp \
# #     -lantlr4-runtime -o GraphProgram

# g++ -std=c++17 -I/usr/local/include/antlr4-runtime -Igenerated \
#     -I"$LLVM_DIR/include" -L"$LLVM_DIR/lib" \
#     main.cpp generated/*.cpp \
#     -lantlr4-runtime -lLLVM -o GraphProgram

# # Step 3: Run the executable
# echo "Running the program..."
# export DYLD_LIBRARY_PATH=$ANTLR4_LIB_PATH:$DYLD_LIBRARY_PATH
# ./GraphProgram graph_code_test.txt


# # # Step 2: Locate LLVM
# # LLVM_DIR=$(brew --prefix llvm)
# # if [ -z "$LLVM_DIR" ]; then
# #     echo "LLVM not found. Please install it using 'brew install llvm'."
# #     exit 1
# # fi

# # # Step 3: Compile the project
# # echo "Compiling the project..."
# # g++ -std=c++17 -I/usr/local/include/antlr4-runtime -Igenerated \
# #     -I"$LLVM_DIR/include" -L"$LLVM_DIR/lib" \
# #     main.cpp generated/*.cpp MyVisitor.cpp \
# #     -lantlr4-runtime -lLLVM -o GraphProgram



#!/usr/bin/env bash
set -e

#-------------------------------------
# 1) Generate ANTLR C++ sources
#-------------------------------------
echo "=== Generating ANTLR C++ sources from Base.g4 ==="
ANTLR_JAR="/usr/local/lib/antlr-4.10.1-complete.jar"
if [ ! -f "$ANTLR_JAR" ]; then
  echo "Error: ANTLR JAR not found at $ANTLR_JAR"
  exit 1
fi

mkdir -p generated
java -jar "$ANTLR_JAR" -Dlanguage=Cpp -visitor Base.g4 -o generated
echo ANTLR sources generated."

#-------------------------------------
# 2) Locate LLVM using llvm-config
#-------------------------------------
echo "=== Locating LLVM via llvm-config ==="
if ! command -v llvm-config &> /dev/null; then
  echo "Error: llvm-config not found. Please install LLVM and ensure llvm-config is on PATH."
  exit 1
fi
LLVM_CXXFLAGS=$(llvm-config --cxxflags)
LLVM_LDFLAGS=$(llvm-config --ldflags)
LLVM_LIBS=$(llvm-config --libs core)
LLVM_SYSTEM_LIBS=$(llvm-config --system-libs)

echo "llvm-config --cxxflags  => $LLVM_CXXFLAGS"
echo "llvm-config --ldflags  => $LLVM_LDFLAGS"
echo "llvm-config --libs     => $LLVM_LIBS"
echo "llvm-config --sys-libs => $LLVM_SYSTEM_LIBS"

#-------------------------------------
# 3) Compile everything
#-------------------------------------
echo "=== Compiling C++ sources ==="
ANTLR_INCLUDE="-I/usr/local/include/antlr4-runtime"
g++ -g -std=c++17 \
    $ANTLR_INCLUDE \
    $LLVM_CXXFLAGS \
    -Igenerated \
    -I. \
    main.cpp \
    MyCodegenVisitor.cpp \
    generated/*.cpp \
    $LLVM_LDFLAGS \
    -lantlr4-runtime \
    $LLVM_LIBS \
    $LLVM_SYSTEM_LIBS \
    -o GraphProgram

echo "Compilation successful."

#-------------------------------------
# 4) Run the binary
#-------------------------------------
echo "=== Running GraphProgram on graph_code_test.txt ==="
./GraphProgram graph_code_test.txt
# gdb ./GraphProgram
# run graph_code_test.txt
## lldb ./GraphProgram
## run graph_code_test.txt
## bt




















#!/usr/bin/env bash
set -e

# Add llvm-config and java to PATH if not already
export PATH=/usr/bin:/usr/local/bin:$PATH

echo "PATH is: $PATH"

#-------------------------------------
# 1) Locate ANTLR jar
#-------------------------------------
# ANTLR_JAR="/usr/local/lib/antlr-4.13.0-complete.jar"
# if [ ! -f "$ANTLR_JAR" ]; then
#   echo "Error: ANTLR JAR not found at $ANTLR_JAR"
#   echo "Please download it from https://www.antlr.org/download.html"
#   echo "and place it at $ANTLR_JAR"
#   exit 1
# fi

#-------------------------------------
# 2) Generate ANTLR C++ sources
#-------------------------------------
echo "=== Generating ANTLR C++ sources from Base.g4 ==="
# java -jar "$ANTLR_JAR" -Dlanguage=Cpp -visitor Base.g4 -o generated
echo "ANTLR sources generated."

#-------------------------------------
# 3) Locate LLVM using llvm-config
#-------------------------------------
echo "=== Locating LLVM via llvm-config ==="
if ! command -v llvm-config &> /dev/null; then
  echo "Error: llvm-config not found. Please install LLVM and ensure llvm-config is on PATH."
  exit 1
fi
# antlr4 -Dlanguage=Cpp -visitor -no-listener Base.g4 -o generated

LLVM_CXXFLAGS=$(llvm-config --cxxflags)
LLVM_LDFLAGS=$(llvm-config --ldflags)
LLVM_LIBS=$(llvm-config --libs core)
LLVM_SYSTEM_LIBS=$(llvm-config --system-libs)

echo "llvm-config --cxxflags  => $LLVM_CXXFLAGS"
echo "llvm-config --ldflags  => $LLVM_LDFLAGS"
echo "llvm-config --libs     => $LLVM_LIBS"
echo "llvm-config --sys-libs => $LLVM_SYSTEM_LIBS"

#-------------------------------------
# 4) Compile everything
#-------------------------------------
echo "=== Compiling C++ sources ==="
ANTLR_INCLUDE="-I/usr/local/include"

# g++ -g -std=c++17 \
#     -I/usr/local/include/antlr4-runtime \
#     $LLVM_CXXFLAGS \
#     -Igenerated \
#     -I. \
#     main.cpp \
#     MyCodegenVisitor.cpp \
#     generated/*.cpp \
#     $LLVM_LDFLAGS \
#     -L/usr/local/lib -lantlr4-runtime \
#     $LLVM_LIBS \
#     $LLVM_SYSTEM_LIBS \
#     -o GraphProgram
clang++-18 \
  -I/usr/local/include/antlr4-runtime \
  -Igenerated \
  main.cpp MyCodegenVisitor.cpp generated/*.cpp \
  `llvm-config-14 --cxxflags --ldflags --libs core` \
  -std=c++17 -fexceptions \
  -L/usr/local/lib -lantlr4-runtime \
  -o GraphProgram


echo "Compilation successful."

#-------------------------------------
# 5) Run the binary with given argument
#-------------------------------------
if [ -z "$1" ]; then
  echo "No input file provided. Usage: ./run.sh <inputfile>"
  exit 1
fi

echo "=== Running GraphProgram on $1 ==="
./GraphProgram "$1"
#!/usr/bin/env bash
set -e

# Add llvm-config and java to PATH if not already
export PATH=/usr/bin:/usr/local/bin:$PATH

echo "PATH is: $PATH"

#-------------------------------------
# 1) Locate ANTLR jar
#-------------------------------------
# ANTLR_JAR="/usr/local/lib/antlr-4.13.0-complete.jar"
# if [ ! -f "$ANTLR_JAR" ]; then
#   echo "Error: ANTLR JAR not found at $ANTLR_JAR"
#   echo "Please download it from https://www.antlr.org/download.html"
#   echo "and place it at $ANTLR_JAR"
#   exit 1
# fi

#-------------------------------------
# 2) Generate ANTLR C++ sources
#-------------------------------------
echo "=== Generating ANTLR C++ sources from Base.g4 ==="
# java -jar "$ANTLR_JAR" -Dlanguage=Cpp -visitor Base.g4 -o generated
echo "ANTLR sources generated."

#-------------------------------------
# 3) Locate LLVM using llvm-config
#-------------------------------------
echo "=== Locating LLVM via llvm-config ==="
if ! command -v llvm-config &> /dev/null; then
  echo "Error: llvm-config not found. Please install LLVM and ensure llvm-config is on PATH."
  exit 1
fi
# antlr4 -Dlanguage=Cpp -visitor -no-listener Base.g4 -o generated

LLVM_CXXFLAGS=$(llvm-config --cxxflags)
LLVM_LDFLAGS=$(llvm-config --ldflags)
LLVM_LIBS=$(llvm-config --libs core)
LLVM_SYSTEM_LIBS=$(llvm-config --system-libs)

echo "llvm-config --cxxflags  => $LLVM_CXXFLAGS"
echo "llvm-config --ldflags  => $LLVM_LDFLAGS"
echo "llvm-config --libs     => $LLVM_LIBS"
echo "llvm-config --sys-libs => $LLVM_SYSTEM_LIBS"

#-------------------------------------
# 4) Compile everything
#-------------------------------------
echo "=== Compiling C++ sources ==="
ANTLR_INCLUDE="-I/usr/local/include"

# g++ -g -std=c++17 \
#     -I/usr/local/include/antlr4-runtime \
#     $LLVM_CXXFLAGS \
#     -Igenerated \
#     -I. \
#     main.cpp \
#     MyCodegenVisitor.cpp \
#     generated/*.cpp \
#     $LLVM_LDFLAGS \
#     -L/usr/local/lib -lantlr4-runtime \
#     $LLVM_LIBS \
#     $LLVM_SYSTEM_LIBS \
#     -o GraphProgram
clang++-18 \
  -I/usr/local/include/antlr4-runtime \
  -Igenerated \
  main.cpp MyCodegenVisitor.cpp generated/*.cpp \
  `llvm-config-14 --cxxflags --ldflags --libs core` \
  -std=c++17 -fexceptions \
  -L/usr/local/lib -lantlr4-runtime \
  -o GraphProgram


echo "Compilation successful."

#-------------------------------------
# 5) Run the binary with given argument
#-------------------------------------
if [ -z "$1" ]; then
  echo "No input file provided. Usage: ./run.sh <inputfile>"
  exit 1
fi

echo "=== Running GraphProgram on $1 ==="
./GraphProgram "$1"

