#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "ASTBuilder.h"
#include "ASTNode.h"        // for ProgramNodePtr
#include "IRGenVisitor.h"   // your LLVM‐backend visitor

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

using namespace antlr4;

int main(int argc, const char* argv[]) {
    //Read the source
    std::ifstream stream("test.graph");
    if (!stream) {
        std::cerr << "Failed to open test.graph\n";
        return 1;
    }

    //Lex & parse
    ANTLRInputStream input(stream);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BaseParser parser(&tokens);
    BaseParser::ProgramContext* tree = parser.program();

    //Build the AST
    ASTBuilder builder;
    auto anyRoot  = builder.visitProgram(tree);
    auto progRoot = std::any_cast<ProgramNodePtr>(anyRoot);

    //Set up LLVM
    llvm::LLVMContext Context;
    auto TheModule = std::make_unique<llvm::Module>("my_module", Context);
    llvm::IRBuilder<> Builder(Context);

    //Generate IR
    IRGenVisitor irgen(Context, *TheModule, Builder);
    irgen.visitProgram(progRoot);

    //Print the IR to stdout
    TheModule->print(llvm::outs(), nullptr);

    return 0;
}
