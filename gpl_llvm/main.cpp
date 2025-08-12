#include <fstream>
#include <iostream>
#include "generated/BaseLexer.h"
#include "generated/BaseParser.h"
#include "MyCodegenVisitor.h"
#include <antlr4-runtime.h>
// #include "MyVisitor.h"

using namespace antlr4;

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <inputfile>\n";
        return 1;
    }

    std::ifstream stream;
    stream.open(argv[1]);
    if (!stream.is_open()) {
        std::cerr << "Could not open file " << argv[1] << "\n";
        return 1;
    }
    ANTLRInputStream input(stream);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BaseParser parser(&tokens);

    BaseParser::ProgramContext* tree = parser.program();
    //MyVisitor visitor;
    //visitor.visitProgram(tree);
    MyCodegenVisitor codegen;
    codegen.visitProgram(tree);
    return 0;
}
