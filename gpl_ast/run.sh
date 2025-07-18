#!/bin/bash


# antlr4 -Dlanguage=Cpp -visitor -no-listener Base.g4 -o generated

g++ -std=c++17 -I/usr/local/include/antlr4-runtime -Igenerated -I.     -L/usr/local/lib     main.cpp  ASTBuilder.cpp ASTNode.cpp generated/*.cpp     -lantlr4-runtime -o GraphProgram


echo "Build done. Run "
./GraphProgram
