#ifndef IRGENVISITOR_H
#define IRGENVISITOR_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <unordered_map>

#include "ASTNode.h"
#include "GraphStorage.h"

class IRGenVisitor
{
public:
    // IRGenVisitor(llvm::LLVMContext &C,
    //              llvm::Module &M,
    //              llvm::IRBuilder<> &B)
    //     : Context(C), Module(M), Builder(B)
    // {
    //     // Build the Graph struct type: { i64, i64, i64*, i32* }
    //     llvm::Type *I64 = llvm::Type::getInt64Ty(Context);
    //     llvm::Type *I32P = llvm::PointerType::get(llvm::Type::getInt32Ty(Context), 0);
    //     llvm::Type *I64P = llvm::PointerType::get(llvm::Type::getInt64Ty(Context), 0);
    //     GraphTy = llvm::StructType::create(
    //         Context,
    //         {I64, I64, I64P, I32P},
    //         "struct.Graph");
    // }

    IRGenVisitor(llvm::LLVMContext &C,
                 llvm::Module &M,
                 llvm::IRBuilder<> &B)
        : Context(C), Module(M), Builder(B)
    {
        // Build the Graph struct type:
        // { i64 n, i64 m, i64* row_ptr, i32* col_idx, i32* nodes_orig }
        llvm::Type *I64 = llvm::Type::getInt64Ty(Context);
        llvm::Type *I32P = llvm::PointerType::get(llvm::Type::getInt32Ty(Context), 0);
        llvm::Type *I64P = llvm::PointerType::get(llvm::Type::getInt64Ty(Context), 0);

        GraphTy = llvm::StructType::create(
            Context,
            {I64, I64, I64P, I32P, I32P},
            "struct.Graph");
    }

    /// Entry point: lower the AST root into LLVM IR
    void visitProgram(ProgramNodePtr prog);

    llvm::AllocaInst *visitVarDecl(VarDeclNode *decl);
    llvm::Value *visitExpr(ASTNode *expr);
    void visitAssignment(AssignmentStmtNode *assign);
    void visitConditional(ConditionalNode *ifs);
    void visitBlock(BlockStmtNode *block);
    void visitReturnStmt(ReturnStmtNode *ret);
    void visitStatement(ASTNode *node);
    void visitWhile(WhileStmtNode *ws);
    //void visitForEach(ForEachStmtNode *fs);
    void visitFunctionDecl(FunctionDeclNode *funcDecl);
    llvm::Value *visitGraphDecl(GraphDeclNode *G);
    void visitQuery(QueryNode *Q);
    void emitBFS(QueryNode *Q);
    void emitDFS(QueryNode *Q);
    void emitBK(QueryNode *Q);
    void emitChromacity(QueryNode *Q);
    void emitDijkstra(QueryNode *Q);
    void emitFloydWarshall(QueryNode *Q);
    void visitPrintStmt(PrintStmtNode *PS);
    // Needed for printing query arrays as original IDs
    void printQueryArrayAsOrigIds(const std::string& queryName, const std::string& graphName);
    //void visitPrintArray(PrintArrayNode *PA);
    llvm::Value *evaluate(ASTNode *node);
    llvm::Function* getPrintfFunction();
    


private:
    llvm::LLVMContext &Context;
    llvm::Module &Module;
    llvm::IRBuilder<> &Builder;

    std::unordered_map<std::string, llvm::Function *> FunctionProtos;

    std::unordered_map<std::string, llvm::AllocaInst *> NamedValues;
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function, llvm::Type *ty, const std::string &name)
    {
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        return tmpBuilder.CreateAlloca(ty, nullptr, name);
    }

    // Keep the old one (backwards compatible)
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function, const std::string &name)
    {
        return createEntryBlockAlloca(function, Builder.getInt32Ty(), name);
    }
    llvm::StructType *GraphTy;
    std::unordered_map<std::string, llvm::Value *> GraphMap;

    // NEW: Graph name -> GraphDeclNode (for query-driven autotune/materialization)
    std::unordered_map<std::string, GraphDeclNode*> GraphDecls;

    // NEW: autotuner + chosen layout record
    gpl::GraphAutotuner Autotuner;
    std::unordered_map<std::string, gpl::GraphLayoutKind> GraphLayoutMap;

    // NEW: big-graph lowering knob (bytes). Beyond this we avoid giant globals by default.
    uint64_t GlobalEmbedThresholdBytes = 32ull * 1024ull * 1024ull; // 32 MiB

    std::unordered_map<std::string, std::string> QueryResultGraphName; 

};

#endif // IRGENVISITOR_H