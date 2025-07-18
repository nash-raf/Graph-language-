#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "ASTBuilder.h"

using namespace antlr4;

int main(int argc, const char* argv[]) {
    std::ifstream stream("test.graph");
    if (!stream) {
        std::cerr << "Failed to open test.graph\n";
        return 1;
    }

    ANTLRInputStream input(stream);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BaseParser parser(&tokens);

    BaseParser::ProgramContext* tree = parser.program();

    ASTBuilder builder;
    builder.visitProgram(tree);

    return 0;
}
