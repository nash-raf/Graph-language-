#!/bin/bash

# Set these paths before running
ANTLR4_RUNTIME_PATH=/usr/local/include/antlr4-runtime
ANTLR4_LIB_PATH=/usr/local/lib

# Step 1: Generate ANTLR C++ files from grammar
echo "Generating parser and lexer from Base.g4..."
# java -jar /usr/local/lib/antlr-4.13.0-complete.jar -Dlanguage=Cpp -visitor Base.g4
java -jar /usr/local/lib/antlr-4.13.0-complete.jar -Dlanguage=Cpp -visitor Base.g4 -o generated
# antlr4 -Dlanguage=Cpp Addition.g4

# Step 2: Compile the program
echo "Compiling..."
g++ -std=c++17 -I/usr/local/include/antlr4-runtime -Igenerated -I. \
    -L/usr/local/lib \
    main.cpp  node.cpp edge.cpp graph.cpp graph_algorithm.cpp generated/*.cpp \
    -lantlr4-runtime -o GraphProgram

# Step 3: Run the executable
echo "Running the program..."
export DYLD_LIBRARY_PATH=$ANTLR4_LIB_PATH:$DYLD_LIBRARY_PATH
./GraphProgram graph_code_test.txt
