#ifndef MYVISITOR_H
#define MYVISITOR_H

#include "generated/BaseBaseVisitor.h"
#include "generated/BaseParser.h"
#include "graph.h"
#include "node.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <stdexcept>
#include <any>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

class node;

class MyVisitor : public BaseBaseVisitor {
public:

    
    MyVisitor();

    antlrcpp::Any visitProgram(BaseParser::ProgramContext *ctx) override;
    antlrcpp::Any visitStatement(BaseParser::StatementContext *ctx) override;
    void executeProgram();
    // Graph-related methods

    // Conditional-related methods
    

    // graph condition
    //antlrcpp::Any visitGraphCondition(BaseParser::GraphConditionContext *ctx);

    //queryrr

    // printing
    antlrcpp::Any visitPrintStatement(BaseParser::PrintStatementContext *ctx) override;
    llvm::Value* visitPrintExprllvm(BaseParser::PrintExprContext *ctx);
    antlrcpp::Any visitPrintExpr(BaseParser::PrintExprContext *ctx) override;
    llvm::Value* visitCondition(BaseParser::ConditionContext *ctx);
    antlrcpp::Any visitWhileStatement(BaseParser::WhileStatementContext *ctx) override;
    antlrcpp::Any visitLoopStatement(BaseParser::LoopStatementContext   *ctx) override;
    antlrcpp::Any visitBlock(BaseParser::BlockContext *ctx) override;
    
    // antlrcpp::Any visitLogicalAnd(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitLogicaLOr(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitRelational(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitNodeCheck(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitEdgeCheck(BaseParser::ParenExprContext *ctx) override;

    // assign
    // virtual antlrcpp::Any visitVariableAssignment(BaseParser::VariableAssignmentContext *ctx) override;
    virtual antlrcpp::Any visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx) override;

    // Expression-related methods
    llvm::Value* visitExpr(BaseParser::ExprContext *ctx);

    // antlrcpp::Any visitMulDivExpr(BaseParser::MulDivExprContext *ctx) override;
    // antlrcpp::Any visitIdExpr(BaseParser::IdExprContext *ctx) override;
    // antlrcpp::Any visitIntExpr(BaseParser::IntExprContext *ctx) override;
    // antlrcpp::Any visitParenExpr(BaseParser::ParenExprContext *ctx) override;
    //  funcitons
    


    // loops
   ;

    // varDecl
    antlrcpp::Any visitVarDecl(BaseParser::VarDeclContext *ctx);
    antlrcpp::Any arrayAssignStatement(BaseParser::ArrayAssignStatementContext *ctx) ;



    llvm::LLVMContext& getContext() { return context; }
    llvm::Module* getModule() const { return module.get(); }
    llvm::IRBuilder<>& getBuilder() { return builder; }

    // antlrcpp::Any visitSimpleDeclaration(BaseParser::SimpleDeclarationContext *ctx) override;
    // antlrcpp::Any visitArrayDeclaration(BaseParser::ArrayDeclarationContext *ctx) override;
    // antlrcpp::Any visitAssignedDeclaration(BaseParser::AssignedDeclarationContext *ctx) override;
    
    //assignment
    
    // Utility methods for graph management


    // Get the graph as an adjacency list
   

private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    struct VarInfo {
        llvm::AllocaInst* alloca;
        llvm::Type* type;
    };
    std::map<std::string, std::pair<llvm::AllocaInst*, llvm::Type*>> symbolTable;

    bool typesMatch(llvm::Type* varType, llvm::Value* exprValue) {
        llvm::Type*  exprType = exprValue->getType();

        if(varType->isPointerTy() && exprType->isPointerTy()) {
            return true;
        }

        return varType == exprType;
    }

    std::string typeToString(llvm::Type* type) {
        std::string typeName;
        llvm::raw_string_ostream rso(typeName);
        type->print(rso);
        return rso.str();
    }

    // Add this to MyVisitor.h (replace the existing createEntryBlockAlloca)
llvm::AllocaInst *createEntryBlockAllocaWithType(llvm::Function *function, const std::string &name, llvm::Type* type)
{
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, name.c_str());
}

// Keep the old function for backward compatibility if needed
llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function, const std::string &name)
{
    return createEntryBlockAllocaWithType(function, name, llvm::Type::getInt32Ty(context));
}

    llvm::Value* convertToDouble(llvm::Value* value) 
    {
        if(value->getType()->isIntegerTy())
        {
            return builder.CreateSIToFP(
                value,
                llvm::Type::getDoubleTy(context),
                "cast_to_double"
            );
        }
        return value;
    }

    llvm::Value* convertToInt(llvm::Value* value) 
    {
        if(value->getType()->isFloatingPointTy())
        {
            return builder.CreateFPToSI(
                value,
                llvm::Type::getInt32Ty(context),
                "cast_to_int"
            );
        }

        if (value->getType()->isIntegerTy(1)) { // bool
            return builder.CreateZExt(value, builder.getInt32Ty(), "bool_to_int");
        }

        if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() < 32) {
            return builder.CreateSExt(value, builder.getInt32Ty(), "sext_int");
        }
        return value;
    }

    llvm::Type* getLLVMType(const std::string& type) {
        if (type == "real")    return llvm::Type::getDoubleTy(context);
        if (type == "int")     return llvm::Type::getInt32Ty(context);
        if (type == "bool")    return llvm::Type::getInt1Ty(context);
        if (type == "string")  return llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0);
        // Handle other types (graph, vertex, edge) if needed
        return nullptr; // Error for unsupported types
    }

    llvm::Value* getDefaultValue(llvm::Type* type) {
        if(type->isIntegerTy()) return llvm::ConstantInt::get(type, 0);
        else if(type->isFloatingPointTy()) return llvm::ConstantFP::get(type, 0.0);
        else if(type->isIntegerTy(1)) return llvm::ConstantInt::get(type, 0);
        else if(type->isPointerTy()) return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(type));
    }
};

#endif // MYVISITOR_H