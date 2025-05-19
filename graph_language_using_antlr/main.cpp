#include <iostream>
#include <fstream>
#include <string>
#include "antlr4-runtime.h"
#include "generated/BaseLexer.h"
#include "generated/BaseParser.h"
#include "MyVisitor.h"
#include "MyVisitor.cpp"


using namespace antlr4;

int main(int argc, const char* argv[]) {
    // std::cout << "Press ENTER to exit...";
    // std::cin.get();
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input-file>" << std::endl;
        return 1;
    }

    // Read the input file
    std::ifstream stream(argv[1]);
    if (!stream) {
        std::cerr << "Cannot open file: " << argv[1] << std::endl;
        return 1;
    }

    // Create an input stream and lexer
    ANTLRInputStream input(stream);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Create a parser
    BaseParser parser(&tokens);

    // Get the parse tree
    tree::ParseTree *tree = parser.program();

    // Create a custom visitor
    MyVisitor visitor;

    // Visit the parse tree
    visitor.visit(tree);
    // visitor.addEdge("g",8,9);
    // visitor.printGraph("g");
    visitor.executeProgram();

    return 0;
}
