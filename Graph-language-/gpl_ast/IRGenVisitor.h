#ifndef IRGENVISITOR_H
#define IRGENVISITOR_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "ASTNode.h"

class IRGenVisitor
{
public:
    IRGenVisitor(llvm::LLVMContext &C,
                 llvm::Module &M,
                 llvm::IRBuilder<> &B)
        : Context(C), Module(M), Builder(B) {}

    /// Entry point: lower the AST root into LLVM IR
    void visitProgram(ProgramNodePtr prog);

    llvm::AllocaInst *visitVarDecl(VarDeclNode *decl);
    llvm::Value *visitExpr(ASTNode* expr);
    void visitAssignment(AssignmentStmtNode* assign);
    void visitConditional(ConditionalNode* ifs);
    void visitBlock(BlockStmtNode* block);
    void visitReturnStmt(ReturnStmtNode* ret);
    void visitStatement(ASTNode* node);
    void visitWhile(WhileStmtNode* ws);

private:
    llvm::LLVMContext &Context;
    llvm::Module &Module;
    llvm::IRBuilder<> &Builder;

    std::unordered_map<std::string, llvm::AllocaInst *> NamedValues;

    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function, const std::string &name)
    {
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        return tmpBuilder.CreateAlloca(Builder.getInt32Ty(), 0, name);
    }
};

#endif // IRGENVISITOR_H
