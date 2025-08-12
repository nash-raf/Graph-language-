#!/bin/bash

# Generate C++ files
echo "Generating parser and lexer..."
java -jar /usr/local/lib/antlr-4.10.1-complete.jar -Dlanguage=Cpp -visitor -no-listener Base.g4 -o generated

# Compile the generated files, additional files, and your main program
echo "Compiling the project..."
g++ -std=c++17 -I/usr/local/include/antlr4-runtime -Igenerated -I. \
    -L/usr/local/lib \
    main.cpp generated/*.cpp \
    -lantlr4-runtime -o GraphProgram

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Run the program
echo "Running the program..."
if [ -z "$1" ]; then
    echo "No input file provided. Please provide a file to parse."
    exit 1
fi

# Check if the input file exists
if [ ! -f "$1" ]; then
    echo "File not found: $1"
    exit 1
fi

# Run the program with the provided input file
./GraphProgram "$1"
