#ifndef MYCODEGENVISITOR_H
#define MYCODEGENVISITOR_H

#include "generated/BaseBaseVisitor.h"
#include "generated/BaseParser.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <unordered_map>

class MyCodegenVisitor : public BaseBaseVisitor {
public:
    // LLVM components
    llvm::LLVMContext                  context;
    llvm::IRBuilder<>                  builder;
    std::unique_ptr<llvm::Module>      module;
    llvm::Function                    *mainFunction;

    // Map: variable name -> AllocaInst* (the stack slot)
    std::unordered_map<std::string, llvm::Value*> symbolTable;

    MyCodegenVisitor();  // Only declare
    ~MyCodegenVisitor() = default;

    // Only these methods need to be declared/implemented:
    antlrcpp::Any visitProgram(BaseParser::ProgramContext *ctx);
    antlrcpp::Any visitStatement(BaseParser::StatementContext *ctx);
    // antlrcpp::Any visitSimpleDeclaration(BaseParser::SimpleDeclarationContext *ctx);
    antlrcpp::Any visitVarDecl(BaseParser::VarDeclContext *ctx);
    antlrcpp::Any visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx);
    antlrcpp::Any visitExpr(BaseParser::ExprContext *ctx);
    antlrcpp::Any visitConditionalStatement(BaseParser::ConditionalStatementContext *ctx);

    antlrcpp::Any visitCondition(BaseParser::ConditionContext *ctx);
    antlrcpp::Any visitLoopStatement(BaseParser::LoopStatementContext *ctx);
    antlrcpp::Any visitWhileStatement(BaseParser::WhileStatementContext *ctx);
    antlrcpp::Any visitBlock(BaseParser::BlockContext *ctx);
    // antlrcpp::Any visitPrintStatement(BaseParser::PrintStatementContext *ctx);
    
};

#endif // MYCODEGENVISITOR_H
