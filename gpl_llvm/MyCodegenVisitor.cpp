#include "MyCodegenVisitor.h"
#include <iostream>
#include <stdexcept>

using namespace llvm;

// Constructor: initialize LLVMContext, IRBuilder, Module (no `main` yet)
MyCodegenVisitor::MyCodegenVisitor()
    : builder(context), mainFunction(nullptr)
{
    module = std::make_unique<Module>("graph_module", context);
}

// visitProgram: create an LLVM function `main`, then lower all top‐level statements into it
antlrcpp::Any MyCodegenVisitor::visitProgram(BaseParser::ProgramContext *ctx) {
    // 1) Create `i32 @main()`
    FunctionType *FT = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    mainFunction = Function::Create(FT,
                                    Function::ExternalLinkage,
                                    "main",
                                    module.get());

    // 2) Create the entry basic block
    BasicBlock *entryBB = BasicBlock::Create(context, "entry", mainFunction);
    builder.SetInsertPoint(entryBB);

    // 3) Lower each top‐level statement
    for (auto stmt : ctx->statement()) {
        visitStatement(stmt);
    }

    // 4) At the end of main, `ret i32 0`
    builder.CreateRet(ConstantInt::get(builder.getInt32Ty(), 0));

    // 5) Verify `main` and print the full module IR to stdout
    verifyFunction(*mainFunction);
    module->print(outs(), nullptr);

    return nullptr;
}

// visitStatement: dispatch to the appropriate child
antlrcpp::Any MyCodegenVisitor::visitStatement(BaseParser::StatementContext *ctx) {
    // if (ctx->printStatement()) {
    //     return visitPrintStatement(ctx->printStatement());
    // }
    if (ctx->loopStatement()) {
        return visitLoopStatement(ctx->loopStatement());
    }
    // if (ctx->varDecl()) {
    //     // The actual context type is SimpleDeclarationContext
    //     return visitSimpleDeclaration(dynamic_cast<BaseParser::SimpleDeclarationContext*>(ctx->varDecl()));
    // }
    else if (ctx->varDecl()) {
        // The actual context type is SimpleDeclarationContext
        return visitVarDecl(ctx->varDecl());
    }
    else if (ctx->assignmentStatement()) {
        return visitAssignmentStatement(ctx->assignmentStatement());
    }
    return nullptr;
}

// visitSimpleDeclaration: “type ID ('=' expr)? ';'”
// We handle only `int ID = expr;` or `int ID;` (allocate an i32)
antlrcpp::Any MyCodegenVisitor::visitVarDecl(BaseParser::VarDeclContext *ctx) {
    // Extract variable name
    std::string name = ctx->ID()->getText();

    // Allocate a 32‐bit integer slot on the stack
    AllocaInst *alloca = builder.CreateAlloca(
        builder.getInt32Ty(), nullptr, name);

    // Compute initializer: either expr or default 0
    Value *initVal = nullptr;
    if (ctx->expr()) {
        initVal = std::any_cast<Value*>(visitExpr(ctx->expr()));
    } else {
        initVal = ConstantInt::get(builder.getInt32Ty(), 0);
    }

    // Store initial value
    builder.CreateStore(initVal, alloca);

    // Remember this slot for future loads/stores
    symbolTable[name] = alloca;
    return (Value*)alloca;
}

// visitAssignmentStatement: “ID '=' expr ';'”
antlrcpp::Any MyCodegenVisitor::visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx) {
    std::string varName = ctx->ID()->getText();

    // Ensure the variable was declared
    auto it = symbolTable.find(varName);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Assignment to undeclared variable: " + varName);
    }
    AllocaInst *slot = cast<AllocaInst>(it->second);

    // Evaluate RHS expression
    Value *rhsVal = std::any_cast<Value*>(visitExpr(ctx->expr()));

    // Store into the existing alloca
    builder.CreateStore(rhsVal, slot);
    return rhsVal;
}

antlrcpp::Any MyCodegenVisitor::visitBlock(BaseParser::BlockContext *ctx) {
    for (auto stmt : ctx->statement()) {
        visit(stmt);
    }
    return nullptr;
}


// visitExpr: support ( *, / ), ( +, - ), int literals, ID loads, parens, bool literals
antlrcpp::Any MyCodegenVisitor::visitExpr(BaseParser::ExprContext *ctx) {
    // Helper lambda to print debug info
    auto debugReturn = [](llvm::Value* val, const std::string& msg) -> antlrcpp::Any {
        if (val) {
            llvm::errs() << "[visitExpr] " << msg << ": " << *val << "\n";
        } else {
            llvm::errs() << "[visitExpr] " << msg << ": nullptr\n";
        }
        return val;
    };

    // 1) Multiplication / Division
    if (auto mCtx = dynamic_cast<BaseParser::MulDivExprContext*>(ctx)) {
        Value *L = std::any_cast<Value*>(visitExpr(mCtx->expr(0)));
        Value *R = std::any_cast<Value*>(visitExpr(mCtx->expr(1)));
        if (mCtx->TIMES())  return debugReturn(builder.CreateMul(L, R, "multmp"), "CreateMul");
        if (mCtx->DIVIDE()) return debugReturn(builder.CreateSDiv(L, R, "divtmp"), "CreateSDiv");
    }
    // 2) Addition / Subtraction
    if (auto aCtx = dynamic_cast<BaseParser::AddSubExprContext*>(ctx)) {
        Value *L = std::any_cast<Value*>(visitExpr(aCtx->expr(0)));
        Value *R = std::any_cast<Value*>(visitExpr(aCtx->expr(1)));
        if (aCtx->PLUS())  return debugReturn(builder.CreateAdd(L, R, "addtmp"), "CreateAdd");
        if (aCtx->MINUS()) return debugReturn(builder.CreateSub(L, R, "subtmp"), "CreateSub");
    }
    // 3) Integer literal
    if (auto iCtx = dynamic_cast<BaseParser::IntExprContext*>(ctx)) {
        int v = std::stoi(iCtx->getText());
        return debugReturn(ConstantInt::get(builder.getInt32Ty(), v), "ConstantInt (int literal)");
    }
    // 4) Identifier: load from stack slot
    if (auto idCtx = dynamic_cast<BaseParser::IdExprContext*>(ctx)) {
        std::string name = idCtx->getText();
        auto it = symbolTable.find(name);
        if (it == symbolTable.end()) {
            throw std::runtime_error("Use of undeclared variable: " + name);
        }
        return debugReturn(
            builder.CreateLoad(builder.getInt32Ty(), cast<AllocaInst>(it->second), name.c_str()), 
            "Load variable " + name);
    }
    // 5) Parenthesized expression
    if (auto pCtx = dynamic_cast<BaseParser::ParenExprContext*>(ctx)) {
        return visitExpr(pCtx->expr());
    }
    // 6) Boolean literals (produce i1)
    if (dynamic_cast<BaseParser::BoolTrueExprContext*>(ctx)) {
        return debugReturn(ConstantInt::get(builder.getInt1Ty(), 1), "BoolTrue");
    }
    if (dynamic_cast<BaseParser::BoolFalseExprContext*>(ctx)) {
        return debugReturn(ConstantInt::get(builder.getInt1Ty(), 0), "BoolFalse");
    }
    // 7) Real literals, strings, etc. not handled here → default to 0
    return debugReturn(ConstantInt::get(builder.getInt32Ty(), 0), "Default int 0");
}
// visitCondition: produce an i1 (boolean) value
antlrcpp::Any MyCodegenVisitor::visitCondition(BaseParser::ConditionContext *ctx) {
    // Logical AND
    if (auto andCtx = dynamic_cast<BaseParser::LogicalAndContext*>(ctx)) {
        Value *L = std::any_cast<Value*>(visitCondition(andCtx->condition(0)));
        Value *R = std::any_cast<Value*>(visitCondition(andCtx->condition(1)));
        return builder.CreateAnd(L, R, "andtmp");
    }
    // Logical OR
    if (auto orCtx = dynamic_cast<BaseParser::LogicalOrContext*>(ctx)) {
        Value *L = std::any_cast<Value*>(visitCondition(orCtx->condition(0)));
        Value *R = std::any_cast<Value*>(visitCondition(orCtx->condition(1)));
        return builder.CreateOr(L, R, "ortmp");
    }
    // Relational (==, !=, <, >, <=, >=)
    if (auto relCtx = dynamic_cast<BaseParser::RelationalContext*>(ctx)) {
        Value *L = std::any_cast<Value*>(visitExpr(relCtx->expr(0)));
        Value *R = std::any_cast<Value*>(visitExpr(relCtx->expr(1)));
        if (relCtx->EQUAL())        return builder.CreateICmpEQ(L, R, "eqtmp");
        if (relCtx->NOTEQUAL())     return builder.CreateICmpNE(L, R, "netmp");
        if (relCtx->LESSTHAN())     return builder.CreateICmpSLT(L, R, "lttmp");
        if (relCtx->GREATERTHAN())  return builder.CreateICmpSGT(L, R, "gttmp");
        if (relCtx->LESSEQUAL())    return builder.CreateICmpSLE(L, R, "letmp");
        if (relCtx->GREATEREQUAL()) return builder.CreateICmpSGE(L, R, "getmp");
    }
    throw std::runtime_error("Invalid condition node.");
}

// visitLoopStatement: delegate to visitWhileStatement
antlrcpp::Any MyCodegenVisitor::visitLoopStatement(BaseParser::LoopStatementContext *ctx) {
    if (ctx->whileStatement()) {
        return visitWhileStatement(ctx->whileStatement());
    }
    return nullptr;
}

// visitWhileStatement: lower “while (cond) { block }” into IR
antlrcpp::Any MyCodegenVisitor::visitWhileStatement(BaseParser::WhileStatementContext *ctx) {
    // Current function must be `mainFunction`
    Function *F = builder.GetInsertBlock()->getParent();

    // Create basic blocks for cond, body, after
    BasicBlock *condBB  = BasicBlock::Create(context, "loopcond", F);
    BasicBlock *bodyBB  = BasicBlock::Create(context, "loopbody", F);
    BasicBlock *afterBB = BasicBlock::Create(context, "afterloop", F);

    // Jump into condBB
    builder.CreateBr(condBB);

    // Fill condBB
    builder.SetInsertPoint(condBB);
    Value *condVal = std::any_cast<Value*>(visitCondition(ctx->condition()));
    builder.CreateCondBr(condVal, bodyBB, afterBB);

    // Fill bodyBB
    builder.SetInsertPoint(bodyBB);
    visitBlock(ctx->block());
    builder.CreateBr(condBB);

    // Continue at afterBB
    builder.SetInsertPoint(afterBB);
    return nullptr;
}


// antlrcpp::Any MyCodegenVisitor::visitPrintStatement(BaseParser::PrintStatementContext *ctx) {
//     Value *val = std::any_cast<Value*>(visitExpr(ctx->printExpr()->expr()));

//     // Use Type::getInt8PtrTy(context)
//     std::vector<Type*> printfArgs{ PointerType::get(Type::getInt8Ty(context), 0) };
//     FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), printfArgs, true);
//     FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

//     Value *fmtStr = builder.CreateGlobalStringPtr("%d\n");
//     builder.CreateCall(printfFunc, { fmtStr, val });
//     return nullptr;
// }
