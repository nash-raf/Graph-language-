#ifndef IRGENVISITOR_H
#define IRGENVISITOR_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <unordered_map>

#include "ASTNode.h"

class IRGenVisitor
{
public:
    IRGenVisitor(llvm::LLVMContext &C,
                 llvm::Module &M,
                 llvm::IRBuilder<> &B)
        : Context(C), Module(M), Builder(B)
    {
        // Build the Graph struct type: { i64, i64, i64*, i32* }
        llvm::Type *I64 = llvm::Type::getInt64Ty(Context);
        llvm::Type *I32P = llvm::PointerType::get(llvm::Type::getInt32Ty(Context), 0);
        llvm::Type *I64P = llvm::PointerType::get(llvm::Type::getInt64Ty(Context), 0);
        GraphTy = llvm::StructType::create(
            Context,
            {I64, I64, I64P, I32P},
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
    void visitForEach(ForEachStmtNode *fs);
    void visitFunctionDecl(FunctionDeclNode *funcDecl);
    llvm::Value *visitGraphDecl(GraphDeclNode *G);
    llvm::Value *visitWeightedGraphDecl(WeightedGraphDeclNode *G);
    void visitQuery(QueryNode *Q);
    void emitBFS(QueryNode *Q);
    void emitBFSSrc(QueryNode *Q);
    void emitDFS(QueryNode *Q);
    void emitDFSSrc(QueryNode *Q);
    void emitBK(QueryNode *Q);
    void emitDijkstra(QueryNode *Q);
    void emitChromacity(QueryNode *Q);
    void emitFloydWarshall(QueryNode *Q);
    void emitMinCut(QueryNode *Q);
    void visitPrintStmt(PrintStmtNode *PS);
    void visitPrintArray(PrintArrayNode *PA);
    void visitSleepStmt(SleepStmtNode *SS);
    llvm::Value *evaluate(ASTNode *node);
    llvm::Function *getPrintfFunction();
    void visitGraphUpdate(GraphUpdateNode *upd);
    void visitShowGraph(ShowGraphNode *S);
    void visitGraphComprehension(GraphComprehensionNode *GC);

    void visitSetDecl(SetDeclNode *SD);
    void visitSetOperation(SetOperationNode *setOp);
    llvm::Value *visitSetExpr(ASTNode *expr);
    llvm::Value *visitSetBinaryExpr(SetBinaryExprNode *binExpr);
    llvm::SmallVector<ASTNode *, 8> flattenSetOperation(ASTNode *expr, const std::string &targetOp);
    void visitSetMethodCall(SetMethodCallNode *node);
    llvm::Value *visitSetContainsExpr(SetContainsExprNode *node);
    llvm::Value *visitSetPopExpr(SetPopExprNode *node);

private:
    llvm::LLVMContext &Context;
    llvm::Module &Module;
    llvm::IRBuilder<> &Builder;

    enum class SetValueKind
    {
        Unknown,
        Nodes,
        Edges
    };

    std::unordered_map<std::string, llvm::Function *> FunctionProtos;

    std::unordered_map<std::string, llvm::AllocaInst *> NamedValues;
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function, const std::string &name, llvm::Type *ty = nullptr)
    {
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        if (!ty)
            ty = Builder.getInt32Ty();
        return tmpBuilder.CreateAlloca(ty, nullptr, name);
    }
    llvm::Type *getLLVMTypeForName(const std::string &typeName);
    llvm::Type *getLLVMTypeFromTypeKind(TypeKind kind);
    llvm::StructType *GraphTy;
    std::unordered_map<std::string, llvm::Value *> GraphMap;
    std::unordered_map<std::string, GraphDeclNode*> GraphAstMap;
    
    std::unordered_map<std::string, llvm::Value *> GraphNodesMap;
    std::unordered_map<std::string, llvm::Value *> GraphEdgesMap;
    std::unordered_map<std::string, SetValueKind> SetKinds;

    // Loop stack for break/continue support
    struct LoopInfo {
        llvm::BasicBlock *condBB;   // where continue jumps to
        llvm::BasicBlock *mergeBB;  // where break jumps to
    };
    std::vector<LoopInfo> LoopStack;

    // 2D array metadata: name -> {cols alloca}
    struct Array2DMeta {
        llvm::Value *colsVal;  // number of columns (i32)
    };
    std::unordered_map<std::string, Array2DMeta> Array2DMap;

    std::unordered_map<uint64_t, uint32_t> EdgePairToId;
    std::vector<std::pair<int32_t, int32_t>> GlobalEdgePairs;
    llvm::GlobalVariable *EdgePairsGV = nullptr;
    uint64_t EdgePairsCount = 0;

    void buildGlobalEdgeTable(ProgramNodePtr prog);
    void emitEdgePairsGlobal();
    std::vector<uint8_t> buildEdgeBlobForGraph(GraphDeclNode *G);
    SetValueKind inferSetKind(ASTNode *expr);
    uint32_t getOrAddEdgeId(int32_t u, int32_t v);
};  


#endif // IRGENVISITOR_H