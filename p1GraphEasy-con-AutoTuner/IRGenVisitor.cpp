#include "IRGenVisitor.h"
#include "SemanticAnalyzer.h" // For TypeKind enum
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>

static uint64_t packEdgeKey(int32_t u, int32_t v)
{
    return (static_cast<uint64_t>(static_cast<uint32_t>(u)) << 32) |
           static_cast<uint32_t>(v);
}

namespace
{
struct FileGraphEstimate
{
    int64_t n = 0;
    int64_t logical_m = 0;
};

std::optional<FileGraphEstimate> estimateUnweightedGraphFile(const std::string &sourceDir,
                                                             const std::string &edgeFileName)
{
    namespace fs = std::filesystem;
    std::vector<fs::path> candidates;
    fs::path raw(edgeFileName);
    candidates.push_back(raw);
    if (raw.is_relative())
    {
        candidates.push_back(fs::path(sourceDir) / raw);
        candidates.push_back(fs::path("test") / raw);
    }

    fs::path chosen;
    for (const auto &candidate : candidates)
    {
        std::error_code ec;
        if (fs::exists(candidate, ec) && !ec)
        {
            chosen = candidate;
            break;
        }
    }
    if (chosen.empty())
        return std::nullopt;

    std::ifstream in(chosen);
    if (!in.good())
        return std::nullopt;

    int64_t undirectedEdges = 0;
    int64_t u = 0;
    int64_t v = 0;
    int64_t maxNode = -1;
    while (in >> u >> v)
    {
        maxNode = std::max(maxNode, std::max(u, v));
        undirectedEdges++;
    }
    if (undirectedEdges <= 0 && maxNode < 0)
        return std::nullopt;

    return FileGraphEstimate{maxNode + 1, undirectedEdges};
}
}

uint32_t IRGenVisitor::getOrAddEdgeId(int32_t u, int32_t v)
{
    uint64_t key = packEdgeKey(u, v);
    auto it = EdgePairToId.find(key);
    if (it != EdgePairToId.end())
        return it->second;

    uint32_t id = static_cast<uint32_t>(GlobalEdgePairs.size());
    EdgePairToId[key] = id;
    GlobalEdgePairs.emplace_back(u, v);
    return id;
}

void IRGenVisitor::buildGlobalEdgeTable(ProgramNodePtr prog)
{
    for (auto &node : prog->topLevel)
    {
        if (node->type == ASTNodeType::GraphDecl)
        {
            auto *G = static_cast<GraphDeclNode *>(node.get());
            if (G->isFileGraph)
                continue;
            for (auto &e : G->edge_list)
            {
                getOrAddEdgeId(static_cast<int32_t>(e.first), static_cast<int32_t>(e.second));
            }
        }
        else if (node->type == ASTNodeType::WeightedGraphDecl)
        {
            auto *G = static_cast<WeightedGraphDeclNode *>(node.get());
            for (auto &e : G->edge_list)
            {
                getOrAddEdgeId(static_cast<int32_t>(e.first), static_cast<int32_t>(e.second));
            }
        }
    }

    emitEdgePairsGlobal();
}

void IRGenVisitor::emitEdgePairsGlobal()
{
    if (GlobalEdgePairs.empty())
    {
        EdgePairsGV = nullptr;
        EdgePairsCount = 0;
        return;
    }

    auto *i32Ty = Builder.getInt32Ty();
    llvm::ArrayType *arrTy = llvm::ArrayType::get(i32Ty, GlobalEdgePairs.size() * 2);

    llvm::SmallVector<llvm::Constant *, 128> elems;
    elems.reserve(GlobalEdgePairs.size() * 2);
    for (auto &p : GlobalEdgePairs)
    {
        elems.push_back(llvm::ConstantInt::get(i32Ty, p.first));
        elems.push_back(llvm::ConstantInt::get(i32Ty, p.second));
    }

    llvm::Constant *arrConst = llvm::ConstantArray::get(arrTy, elems);
    EdgePairsGV = new llvm::GlobalVariable(
        Module,
        arrTy,
        true,
        llvm::GlobalValue::PrivateLinkage,
        arrConst,
        "__edge_pairs");

    EdgePairsCount = GlobalEdgePairs.size();
}

std::vector<uint8_t> IRGenVisitor::buildEdgeBlobForGraph(GraphDeclNode *G)
{
    RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);

    for (auto &e : G->edge_list)
    {
        uint32_t id = getOrAddEdgeId(static_cast<int32_t>(e.first),
                                     static_cast<int32_t>(e.second));
        roaring_bitmap_add(bm, id);
    }

    size_t sz = roaring_bitmap_portable_size_in_bytes(bm);
    std::vector<uint8_t> blob(sz);
    roaring_bitmap_portable_serialize(bm, blob.data());
    roaring_bitmap_free(bm);

    return blob;
}

IRGenVisitor::SetValueKind IRGenVisitor::inferSetKind(ASTNode *expr)
{
    if (!expr)
        return SetValueKind::Unknown;

    switch (expr->type)
    {
    case ASTNodeType::GraphMemberSet:
    {
        auto *gm = static_cast<GraphMemberSetNode *>(expr);
        return (gm->member == GraphMemberKind::Nodes) ? SetValueKind::Nodes : SetValueKind::Edges;
    }
    case ASTNodeType::SetLiteral:
        return SetValueKind::Nodes;
    case ASTNodeType::Variable:
    {
        auto *var = static_cast<VariableNode *>(expr);
        auto it = SetKinds.find(var->name);
        return it == SetKinds.end() ? SetValueKind::Unknown : it->second;
    }
    case ASTNodeType::SetBinaryExpr:
    {
        auto *bin = static_cast<SetBinaryExprNode *>(expr);
        auto lk = inferSetKind(bin->lhs.get());
        auto rk = inferSetKind(bin->rhs.get());
        if (lk == SetValueKind::Unknown)
            return rk;
        if (rk == SetValueKind::Unknown)
            return lk;
        if (lk != rk)
            throw std::runtime_error("Set kind mismatch in union/intersect");
        return lk;
    }
    default:
        return SetValueKind::Unknown;
    }
}

llvm::Type *getBitmapPtrTy(llvm::LLVMContext &Context)
{
    return llvm::PointerType::get(Context, 0);
}

llvm::Type *IRGenVisitor::getLLVMTypeFromTypeKind(TypeKind kind)
{
    switch (kind)
    {
    case TypeKind::Int:
        return Builder.getInt32Ty();
    case TypeKind::Bool:
        return Builder.getInt1Ty();
    case TypeKind::Real:
        return Builder.getDoubleTy();
    case TypeKind::String:
        return llvm::PointerType::getUnqual(Builder.getInt8Ty());
    case TypeKind::IntArray:
        // For arrays, we'll need the size, so this returns a pointer to i32
        // The actual array type will be determined at the declaration site
        return llvm::PointerType::getUnqual(Builder.getInt32Ty());
    case TypeKind::RealArray:
        return llvm::PointerType::getUnqual(Builder.getDoubleTy());
    case TypeKind::Graph:
        return GraphTy->getPointerTo();
    case TypeKind::WeightedGraph:
        // Assuming WeightedGraph uses the same struct type for now
        return GraphTy->getPointerTo();
    case TypeKind::Set:
        return llvm::PointerType::get(Context, 0); // bitmap pointer (opaque)
    case TypeKind::Void:
        return llvm::Type::getVoidTy(Context);
    case TypeKind::Unknown:
    default:
        // Fallback to i32 for unknown types
        return Builder.getInt32Ty();
    }
}

llvm::Type *IRGenVisitor::getLLVMTypeForName(const std::string &typeName)
{
    if (typeName == "void")
        return llvm::Type::getVoidTy(Context);
    if (typeName == "int" || typeName == "vertex" || typeName == "edge")
        return Builder.getInt32Ty();
    if (typeName == "bool")
        return Builder.getInt1Ty();
    if (typeName == "real")
        return Builder.getDoubleTy();
    if (typeName == "string")
        return llvm::PointerType::getUnqual(Builder.getInt8Ty());
    if (typeName == "graph")
        return GraphTy->getPointerTo();
    if (typeName == "set")
        return llvm::PointerType::get(Context, 0); // bitmap pointer (opaque)
    return Builder.getInt32Ty();
}

llvm::Type *IRGenVisitor::getStorageValueType(llvm::Value *storage)
{
    if (auto *alloca = llvm::dyn_cast<llvm::AllocaInst>(storage))
        return alloca->getAllocatedType();
    if (auto *global = llvm::dyn_cast<llvm::GlobalVariable>(storage))
        return global->getValueType();
    throw std::runtime_error("IRGenVisitor: unsupported storage value");
}

llvm::Value *IRGenVisitor::lookupNamedStorage(const std::string &name)
{
    auto it = NamedValues.find(name);
    if (it == NamedValues.end())
        throw std::runtime_error("Undefined variable: " + name);
    return it->second;
}

llvm::Value *IRGenVisitor::loadGraphValue(const std::string &name)
{
    auto it = GraphMap.find(name);
    if (it == GraphMap.end())
        throw std::runtime_error("Graph not allocated: " + name);
    return Builder.CreateLoad(GraphTy->getPointerTo(), it->second, name + ".graph");
}

void IRGenVisitor::visitProgram(ProgramNodePtr prog)
{

    for (auto &node : prog->topLevel)
    {
        if (node->type == ASTNodeType::FunctionDecl)
        {
            auto *FD = static_cast<FunctionDeclNode *>(node.get());

            // Collect parameter types - prefer annotated types, fallback to string parsing
            std::vector<llvm::Type *> paramTys;
            for (size_t i = 0; i < FD->parameters.size(); ++i)
            {
                if (FD->parameters[i]->resolvedType != TypeKind::Unknown)
                {
                    paramTys.push_back(getLLVMTypeFromTypeKind(FD->parameters[i]->resolvedType));
                }
                else
                {
                    // Fallback to string parsing for unannotated nodes
                    paramTys.push_back(getLLVMTypeForName(FD->parameters[i]->typeName));
                }
            }

            // Determine return type - prefer annotated type, fallback to string parsing
            llvm::Type *retTy;
            if (FD->resolvedReturnType != TypeKind::Unknown)
            {
                retTy = getLLVMTypeFromTypeKind(FD->resolvedReturnType);
            }
            else
            {
                retTy = getLLVMTypeForName(FD->returnType);
            }

            // Create the function prototype and insert into the module
            llvm::FunctionType *funcType =
                llvm::FunctionType::get(retTy, paramTys, /*isVarArg=*/false);
            llvm::Function *fn =
                llvm::Function::Create(funcType,
                                       llvm::Function::ExternalLinkage,
                                       FD->name,
                                       &Module);

            // Remember it for later call-site lookups
            FunctionProtos[FD->name] = fn;
        }
    }

    // Create: int main()
    auto *intTy = Builder.getInt32Ty();
    auto *mainFT = llvm::FunctionType::get(intTy, /*isVarArg=*/false);
    auto *mainF = llvm::Function::Create(
        mainFT,
        llvm::Function::ExternalLinkage,
        "main",
        &Module);

    // Build global edge table (needed for edge set printing)
    buildGlobalEdgeTable(prog);

    // Entry block
    auto *BB = llvm::BasicBlock::Create(Context, "entry", mainF);
    Builder.SetInsertPoint(BB);

    // Lower every top‑level AST node
    EmittingTopLevel = true;
    for (auto &node : prog->topLevel)
    {
        switch (node->type)
        {
        case ASTNodeType::VarDecl:
            visitVarDecl(static_cast<VarDeclNode *>(node.get()));
            break;
        case ASTNodeType::BinaryExpr:
        case ASTNodeType::AssignmentStmt:
            // you’ll need an AST node for assignment too; lower it here
            visitAssignment(static_cast<AssignmentStmtNode *>(node.get()));
            break;
        case ASTNodeType::Conditional:
            visitConditional(static_cast<ConditionalNode *>(node.get()));
            break;
        case ASTNodeType::PrintStmt:
            visitPrintStmt(static_cast<PrintStmtNode *>(node.get()));
            break;
        case ASTNodeType::SleepStmt:
            visitSleepStmt(static_cast<SleepStmtNode *>(node.get()));
            break;
        case ASTNodeType::WhileStmt:
            visitWhile(static_cast<WhileStmtNode *>(node.get()));
            break;
        case ASTNodeType::ForEachStmt:
            visitForEach(static_cast<ForEachStmtNode *>(node.get()));
            break;
        case ASTNodeType::GraphDecl:
            // Lower graph h { … }
            visitGraphDecl(static_cast<GraphDeclNode *>(node.get()));
            break;
        case ASTNodeType::WeightedGraphDecl:
            visitWeightedGraphDecl(static_cast<WeightedGraphDeclNode *>(node.get()));
            break;
        case ASTNodeType::QueryNode:
            visitQuery(static_cast<QueryNode *>(node.get()));
            break;
        case ASTNodeType::GraphUpdate:
            visitGraphUpdate(static_cast<GraphUpdateNode *>(node.get()));
            break;
        case ASTNodeType::ShowGraph:
            visitShowGraph(static_cast<ShowGraphNode *>(node.get()));
            break;
        case ASTNodeType::GraphComprehension:
            visitGraphComprehension(static_cast<GraphComprehensionNode *>(node.get()));
            break;
        case ASTNodeType::SetDecl:
            visitSetDecl(static_cast<SetDeclNode *>(node.get()));
            break;
        case ASTNodeType::SetOperation:
            visitSetOperation(static_cast<SetOperationNode *>(node.get()));
            break;
        case ASTNodeType::SetMethodCall:
            visitSetMethodCall(static_cast<SetMethodCallNode *>(node.get()));
            break;
        case ASTNodeType::FunctionCall:
            visitExpr(node.get());
            break;
        case ASTNodeType::FunctionDecl:
            // Function bodies are lowered in the dedicated post-pass below.
            break;
        case ASTNodeType::SwapStmt:
            visitSwapStmt(static_cast<SwapStmtNode *>(node.get()));
            break;
        case ASTNodeType::BreakStmt:
        case ASTNodeType::ContinueStmt:
            break;
        default:
            visitStatement(node.get());
            break;
        }
    }
    EmittingTopLevel = false;

    // Return 0
    Builder.CreateRet(llvm::ConstantInt::get(intTy, 0));

    llvm::verifyFunction(*mainF, &llvm::errs());
    for (auto &node : prog->topLevel)
    {
        if (node->type == ASTNodeType::FunctionDecl)
        {
            visitFunctionDecl(static_cast<FunctionDeclNode *>(node.get()));
        }
    }
}

void IRGenVisitor::visitShowGraph(ShowGraphNode *S)
{
    llvm::Value *graphPtr = loadGraphValue(S->graphName);

    llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);

    // Load n, row_ptr, col_idx from struct.Graph { i64 n, i64 m, i64* row_ptr, i32* col_idx }
    llvm::Value *nPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
    llvm::Value *nVal = Builder.CreateLoad(i64Ty, nPtr, "g_n");

    llvm::Value *rpPtrGEP = Builder.CreateStructGEP(GraphTy, graphPtr, 2, "g_rp_ptr");
    llvm::Value *rowPtr = Builder.CreateLoad(
        llvm::PointerType::getUnqual(i64Ty), rpPtrGEP, "row_ptr");

    llvm::Value *ciPtrGEP = Builder.CreateStructGEP(GraphTy, graphPtr, 3, "g_ci_ptr");
    llvm::Value *colPtr = Builder.CreateLoad(
        llvm::PointerType::getUnqual(i32Ty), ciPtrGEP, "col_ptr");

    // extern "C" void show_graph_runtime(int64_t, int64_t*, int32_t*);
    llvm::FunctionType *showFT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(Context),
        {i64Ty,
         llvm::PointerType::getUnqual(i64Ty),
         llvm::PointerType::getUnqual(i32Ty)},
        false);

    llvm::FunctionCallee showDecl =
        Module.getOrInsertFunction("show_graph_runtime", showFT);

    Builder.CreateCall(showDecl, {nVal, rowPtr, colPtr});
}

void IRGenVisitor::visitGraphComprehension(GraphComprehensionNode *GC)
{
    llvm::Value *srcGraphPtr = loadGraphValue(GC->graphName);
    GraphDeclNode *astGraph = nullptr;
    auto astIt = GraphAstMap.find(GC->graphName);
    if (astIt != GraphAstMap.end())
        astGraph = astIt->second;

    auto *i64Ty = llvm::Type::getInt64Ty(Context);
    auto *i32Ty = llvm::Type::getInt32Ty(Context);
    auto *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
    auto *graphPtrTy = GraphTy->getPointerTo();

    llvm::Function *fn = Builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpB(&fn->getEntryBlock(), fn->getEntryBlock().begin());

    auto buildGraphStruct = [&](llvm::Value *newN, llvm::Value *newM,
                                llvm::Value *newRP, llvm::Value *newCI) -> llvm::Value * {
        llvm::FunctionType *mallocFT = llvm::FunctionType::get(i8PtrTy, {i64Ty}, false);
        llvm::FunctionCallee mallocDecl = Module.getOrInsertFunction("malloc", mallocFT);
        uint64_t graphSize = Module.getDataLayout().getTypeAllocSize(GraphTy);
        llvm::Value *newGraphRaw = Builder.CreateCall(mallocDecl, {llvm::ConstantInt::get(i64Ty, graphSize)});
        llvm::Value *newGraphPtr = Builder.CreateBitCast(newGraphRaw, graphPtrTy);

        Builder.CreateStore(newN, Builder.CreateStructGEP(GraphTy, newGraphPtr, 0));
        Builder.CreateStore(newM, Builder.CreateStructGEP(GraphTy, newGraphPtr, 1));
        Builder.CreateStore(newRP, Builder.CreateStructGEP(GraphTy, newGraphPtr, 2));
        Builder.CreateStore(newCI, Builder.CreateStructGEP(GraphTy, newGraphPtr, 3));
        Builder.CreateStore(llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(i32Ty)),
                            Builder.CreateStructGEP(GraphTy, newGraphPtr, 4));
        return newGraphPtr;
    };

    auto loadCSR = [&](llvm::Value *graphPtr,
                       llvm::Value *&nVal,
                       llvm::Value *&rowPtr,
                       llvm::Value *&colPtr) {
        nVal = Builder.CreateLoad(i64Ty, Builder.CreateStructGEP(GraphTy, graphPtr, 0), "g_n");
        rowPtr = Builder.CreateLoad(llvm::PointerType::getUnqual(i64Ty),
                                    Builder.CreateStructGEP(GraphTy, graphPtr, 2),
                                    "g_row_ptr");
        colPtr = Builder.CreateLoad(llvm::PointerType::getUnqual(i32Ty),
                                    Builder.CreateStructGEP(GraphTy, graphPtr, 3),
                                    "g_col_ptr");
    };

    auto allocOutGraph = [&](const std::string &name) {
        llvm::AllocaInst *outN = tmpB.CreateAlloca(i64Ty, nullptr, name + "_out_n");
        llvm::AllocaInst *outM = tmpB.CreateAlloca(i64Ty, nullptr, name + "_out_m");
        llvm::AllocaInst *outRP = tmpB.CreateAlloca(llvm::PointerType::getUnqual(i64Ty), nullptr, name + "_out_rp");
        llvm::AllocaInst *outCI = tmpB.CreateAlloca(llvm::PointerType::getUnqual(i32Ty), nullptr, name + "_out_ci");
        return std::make_tuple(outN, outM, outRP, outCI);
    };

    if (!GC->graphOperands.empty())
    {
        llvm::FunctionType *combFT = llvm::FunctionType::get(
            llvm::Type::getVoidTy(Context),
            {i64Ty,
             llvm::PointerType::getUnqual(i64Ty),
             llvm::PointerType::getUnqual(i32Ty),
             llvm::PointerType::getUnqual(i64Ty),
             llvm::PointerType::getUnqual(i32Ty),
             llvm::PointerType::getUnqual(i64Ty),
             llvm::PointerType::getUnqual(i64Ty),
             llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(i64Ty)),
             llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(i32Ty))},
            false);

        llvm::FunctionCallee unionDecl = Module.getOrInsertFunction("graph_union_runtime", combFT);
        llvm::FunctionCallee interDecl = Module.getOrInsertFunction("graph_intersection_runtime", combFT);

        for (size_t i = 0; i < GC->graphOperands.size(); ++i)
        {
            llvm::Value *rhsGraphPtr = loadGraphValue(GC->graphOperands[i]);

            llvm::Value *nL = nullptr;
            llvm::Value *rpL = nullptr;
            llvm::Value *ciL = nullptr;
            loadCSR(srcGraphPtr, nL, rpL, ciL);

            llvm::Value *nR = nullptr;
            llvm::Value *rpR = nullptr;
            llvm::Value *ciR = nullptr;
            loadCSR(rhsGraphPtr, nR, rpR, ciR);
            (void)nR;

            auto [outN, outM, outRP, outCI] = allocOutGraph(GC->targetName + "_combine_" + std::to_string(i));
            llvm::FunctionCallee opDecl = (GC->ops[i] == GraphExprOp::And) ? interDecl : unionDecl;
            Builder.CreateCall(opDecl, {nL, rpL, ciL, rpR, ciR, outN, outM, outRP, outCI});

            llvm::Value *newN = Builder.CreateLoad(i64Ty, outN);
            llvm::Value *newM = Builder.CreateLoad(i64Ty, outM);
            llvm::Value *newRP = Builder.CreateLoad(llvm::PointerType::getUnqual(i64Ty), outRP);
            llvm::Value *newCI = Builder.CreateLoad(llvm::PointerType::getUnqual(i32Ty), outCI);
            srcGraphPtr = buildGraphStruct(newN, newM, newRP, newCI);
        }
    }

    if (GC->condition)
    {
        std::vector<int32_t> tokenKinds;
        std::vector<int32_t> tokenArg1;
        std::vector<int32_t> tokenArg2;

        std::function<void(const std::shared_ptr<GraphConditionNode> &)> emitPostfix =
            [&](const std::shared_ptr<GraphConditionNode> &cond) {
                if (!cond)
                    return;
                emitPostfix(cond->left);
                emitPostfix(cond->right);

                switch (cond->op)
                {
                case GraphConditionOp::Connected:
                {
                    int mappedId = cond->nodeId;
                    if (astGraph && !astGraph->isFileGraph)
                    {
                        auto it = std::find(astGraph->node_ids.begin(), astGraph->node_ids.end(), cond->nodeId);
                        if (it != astGraph->node_ids.end())
                            mappedId = static_cast<int>(std::distance(astGraph->node_ids.begin(), it));
                    }
                    tokenKinds.push_back(1);
                    tokenArg1.push_back(mappedId);
                    tokenArg2.push_back(0);
                    break;
                }
                case GraphConditionOp::Cycle:
                    tokenKinds.push_back(2);
                    tokenArg1.push_back(0);
                    tokenArg2.push_back(0);
                    break;
                case GraphConditionOp::Degree:
                {
                    int opCode = 0;
                    switch (cond->degreeOp)
                    {
                    case GraphDegreeOp::Eq: opCode = 1; break;
                    case GraphDegreeOp::Ne: opCode = 2; break;
                    case GraphDegreeOp::Le: opCode = 3; break;
                    case GraphDegreeOp::Ge: opCode = 4; break;
                    case GraphDegreeOp::Lt: opCode = 5; break;
                    case GraphDegreeOp::Gt: opCode = 6; break;
                    default: break;
                    }
                    tokenKinds.push_back(3);
                    tokenArg1.push_back(opCode);
                    tokenArg2.push_back(cond->degreeValue);
                    break;
                }
                case GraphConditionOp::And:
                    tokenKinds.push_back(4);
                    tokenArg1.push_back(0);
                    tokenArg2.push_back(0);
                    break;
                case GraphConditionOp::Or:
                    tokenKinds.push_back(5);
                    tokenArg1.push_back(0);
                    tokenArg2.push_back(0);
                    break;
                }
            };
        emitPostfix(GC->condition);

        llvm::ArrayType *tokenArrTy = llvm::ArrayType::get(i32Ty, tokenKinds.size());
        llvm::AllocaInst *kindsAlloca = tmpB.CreateAlloca(tokenArrTy, nullptr, GC->targetName + "_gc_kinds");
        llvm::AllocaInst *arg1Alloca = tmpB.CreateAlloca(tokenArrTy, nullptr, GC->targetName + "_gc_arg1");
        llvm::AllocaInst *arg2Alloca = tmpB.CreateAlloca(tokenArrTy, nullptr, GC->targetName + "_gc_arg2");

        for (size_t i = 0; i < tokenKinds.size(); ++i)
        {
            llvm::Value *idx = llvm::ConstantInt::get(i32Ty, i);
            llvm::Value *kindPtr = Builder.CreateGEP(tokenArrTy, kindsAlloca, {Builder.getInt32(0), idx});
            llvm::Value *arg1Ptr = Builder.CreateGEP(tokenArrTy, arg1Alloca, {Builder.getInt32(0), idx});
            llvm::Value *arg2Ptr = Builder.CreateGEP(tokenArrTy, arg2Alloca, {Builder.getInt32(0), idx});
            Builder.CreateStore(llvm::ConstantInt::get(i32Ty, tokenKinds[i]), kindPtr);
            Builder.CreateStore(llvm::ConstantInt::get(i32Ty, tokenArg1[i]), arg1Ptr);
            Builder.CreateStore(llvm::ConstantInt::get(i32Ty, tokenArg2[i]), arg2Ptr);
        }

        llvm::FunctionType *compFT = llvm::FunctionType::get(
            llvm::Type::getVoidTy(Context),
            {i64Ty,
             llvm::PointerType::getUnqual(i64Ty),
             llvm::PointerType::getUnqual(i32Ty),
             llvm::PointerType::getUnqual(i32Ty),
             llvm::PointerType::getUnqual(i32Ty),
             llvm::PointerType::getUnqual(i32Ty),
             i32Ty,
             llvm::PointerType::getUnqual(i64Ty),
             llvm::PointerType::getUnqual(i64Ty),
             llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(i64Ty)),
             llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(i32Ty))},
            false);

        llvm::FunctionCallee compDecl = Module.getOrInsertFunction("graph_comprehension_runtime", compFT);

        llvm::Value *nVal = nullptr;
        llvm::Value *rowPtr = nullptr;
        llvm::Value *colPtr = nullptr;
        loadCSR(srcGraphPtr, nVal, rowPtr, colPtr);

        auto [outN, outM, outRP, outCI] = allocOutGraph(GC->targetName + "_filter");
        llvm::Value *kindsPtr = Builder.CreateGEP(tokenArrTy, kindsAlloca, {Builder.getInt32(0), Builder.getInt32(0)});
        llvm::Value *arg1Ptr = Builder.CreateGEP(tokenArrTy, arg1Alloca, {Builder.getInt32(0), Builder.getInt32(0)});
        llvm::Value *arg2Ptr = Builder.CreateGEP(tokenArrTy, arg2Alloca, {Builder.getInt32(0), Builder.getInt32(0)});

        Builder.CreateCall(compDecl,
                           {nVal, rowPtr, colPtr,
                            kindsPtr, arg1Ptr, arg2Ptr,
                            llvm::ConstantInt::get(i32Ty, tokenKinds.size()),
                            outN, outM, outRP, outCI});

        llvm::Value *newN = Builder.CreateLoad(i64Ty, outN);
        llvm::Value *newM = Builder.CreateLoad(i64Ty, outM);
        llvm::Value *newRP = Builder.CreateLoad(llvm::PointerType::getUnqual(i64Ty), outRP);
        llvm::Value *newCI = Builder.CreateLoad(llvm::PointerType::getUnqual(i32Ty), outCI);
        srcGraphPtr = buildGraphStruct(newN, newM, newRP, newCI);
    }

    llvm::Value *graphStorage = nullptr;
    if (EmittingTopLevel)
    {
        graphStorage = new llvm::GlobalVariable(
            Module,
            graphPtrTy,
            false,
            llvm::GlobalValue::InternalLinkage,
            llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(graphPtrTy)),
            GC->targetName);
    }
    else
    {
        graphStorage = createEntryBlockAlloca(fn, GC->targetName, graphPtrTy);
    }

    Builder.CreateStore(srcGraphPtr, graphStorage);
    GraphMap[GC->targetName] = graphStorage;
    NamedValues[GC->targetName] = graphStorage;

    llvm::FunctionType *registerFT = llvm::FunctionType::get(Builder.getVoidTy(), {graphPtrTy}, false);
    auto registerFn = Module.getOrInsertFunction("graph_register_csr_metadata", registerFT);
    Builder.CreateCall(registerFn, {srcGraphPtr});

    llvm::FunctionType *getBmFT = llvm::FunctionType::get(i8PtrTy, {graphPtrTy}, false);
    auto getNodeBmFn = Module.getOrInsertFunction("graph_get_node_bitmap", getBmFT);
    auto getEdgeBmFn = Module.getOrInsertFunction("graph_get_edge_bitmap", getBmFT);
    GraphNodesMap[GC->targetName] = Builder.CreateCall(getNodeBmFn, {srcGraphPtr}, GC->targetName + "_nodes_bm");
    GraphEdgesMap[GC->targetName] = Builder.CreateCall(getEdgeBmFn, {srcGraphPtr}, GC->targetName + "_edges_bm");
}

void IRGenVisitor::visitFunctionDecl(FunctionDeclNode *funcDecl)
{
    // 1) Grab the LLVM Function* from your prototype map
    llvm::Function *function = FunctionProtos[funcDecl->name];
    assert(function && "No prototype for function");

    // 2) Create entry block and set builder
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(Context, "entry", function);

    // 3) Save current state (for returning to main after function body generation)
    auto savedNamedValues = NamedValues;
    auto savedGraphMap = GraphMap;
    auto savedArray2DMap = Array2DMap;
    auto savedInsertPoint = Builder.GetInsertBlock();

    Builder.SetInsertPoint(BB);

    // 4) Allocate space for each parameter and store the incoming arg
    //    For graph parameters, also register in GraphMap so for-each works
    unsigned idx = 0;
    for (auto &arg : function->args())
    {
        arg.setName(funcDecl->parameters[idx]->paramName);

        llvm::AllocaInst *alloca =
            createEntryBlockAlloca(function, arg.getName().str(), arg.getType());

        Builder.CreateStore(&arg, alloca);
        NamedValues[arg.getName().str()] = alloca;

        // If parameter is a graph type, register in GraphMap for forEach lookups
        TypeKind paramKind = funcDecl->parameters[idx]->resolvedType;
        if (paramKind == TypeKind::Graph || paramKind == TypeKind::WeightedGraph)
        {
            GraphMap[arg.getName().str()] = alloca;
        }

        idx++;
    }

    // 5) Lower the function body statements
    auto *bodyBlock = static_cast<BlockStmtNode *>(funcDecl->body.get());
    for (auto &stmt : bodyBlock->statements)
    {
        visitStatement(stmt.get());
    }

    // 6) If no explicit return (and non-void), insert a default
    if (!Builder.GetInsertBlock()->getTerminator())
    {
        TypeKind returnTypeKind = funcDecl->resolvedReturnType;
        if (returnTypeKind == TypeKind::Unknown)
        {
            if (funcDecl->returnType == "void")
                returnTypeKind = TypeKind::Void;
            else
                returnTypeKind = TypeKind::Int;
        }

        if (returnTypeKind == TypeKind::Void)
        {
            Builder.CreateRetVoid();
        }
        else
        {
            llvm::Type *retTy = getLLVMTypeFromTypeKind(returnTypeKind);
            Builder.CreateRet(llvm::Constant::getNullValue(retTy));
        }
    }

    // 7) Verify this function
    llvm::verifyFunction(*function, &llvm::errs());

    // 8) Restore state
    NamedValues = savedNamedValues;
    GraphMap = savedGraphMap;
    Array2DMap = savedArray2DMap;
    if (savedInsertPoint)
        Builder.SetInsertPoint(savedInsertPoint);
}

namespace
{
struct FirstWinsPattern
{
    ArrayAccessNode *claimAccess = nullptr;
    int64_t expectedValue = 0;
    int64_t desiredValue = 0;
};

static bool firstWinsExprsEquivalent(const ASTNode *lhs, const ASTNode *rhs)
{
    if (lhs == rhs)
        return true;
    if (!lhs || !rhs || lhs->type != rhs->type)
        return false;

    switch (lhs->type)
    {
    case ASTNodeType::Variable:
        return static_cast<const VariableNode *>(lhs)->name ==
               static_cast<const VariableNode *>(rhs)->name;
    case ASTNodeType::IntLiteral:
        return static_cast<const IntLiteralNode *>(lhs)->value ==
               static_cast<const IntLiteralNode *>(rhs)->value;
    case ASTNodeType::ArrayAccess:
    {
        auto *lhsAccess = static_cast<const ArrayAccessNode *>(lhs);
        auto *rhsAccess = static_cast<const ArrayAccessNode *>(rhs);
        return firstWinsExprsEquivalent(lhsAccess->arrayExpr.get(), rhsAccess->arrayExpr.get()) &&
               firstWinsExprsEquivalent(lhsAccess->indexExpr.get(), rhsAccess->indexExpr.get());
    }
    default:
        return false;
    }
}

static std::optional<FirstWinsPattern> detectFirstWinsPattern(ConditionalNode *ifs)
{
    if (!ifs || ifs->elseBlock || !ifs->condition ||
        ifs->condition->type != ASTNodeType::BinaryExpr)
        return std::nullopt;

    auto *condition = static_cast<BinaryExprNode *>(ifs->condition.get());
    if (condition->op != "==")
        return std::nullopt;

    ArrayAccessNode *claimAccess = nullptr;
    IntLiteralNode *expected = nullptr;
    if (condition->lhs->type == ASTNodeType::ArrayAccess &&
        condition->rhs->type == ASTNodeType::IntLiteral)
    {
        claimAccess = static_cast<ArrayAccessNode *>(condition->lhs.get());
        expected = static_cast<IntLiteralNode *>(condition->rhs.get());
    }
    else if (condition->rhs->type == ASTNodeType::ArrayAccess &&
             condition->lhs->type == ASTNodeType::IntLiteral)
    {
        claimAccess = static_cast<ArrayAccessNode *>(condition->rhs.get());
        expected = static_cast<IntLiteralNode *>(condition->lhs.get());
    }
    else
    {
        return std::nullopt;
    }

    auto *thenBlock = dynamic_cast<BlockStmtNode *>(ifs->thenBlock.get());
    if (!thenBlock || thenBlock->statements.empty() ||
        thenBlock->statements.front()->type != ASTNodeType::AssignmentStmt)
        return std::nullopt;

    auto *claimAssignment =
        static_cast<AssignmentStmtNode *>(thenBlock->statements.front().get());
    if (!claimAssignment->lhs || claimAssignment->lhs->type != ASTNodeType::ArrayAccess ||
        !claimAssignment->rhs || claimAssignment->rhs->type != ASTNodeType::IntLiteral)
        return std::nullopt;

    auto *assignedAccess = static_cast<ArrayAccessNode *>(claimAssignment->lhs.get());
    if (!firstWinsExprsEquivalent(claimAccess, assignedAccess))
        return std::nullopt;

    auto *desired = static_cast<IntLiteralNode *>(claimAssignment->rhs.get());
    if (expected->value == desired->value)
        return std::nullopt;

    auto *arrayVariable = dynamic_cast<VariableNode *>(claimAccess->arrayExpr.get());
    if (!arrayVariable || claimAccess->resolvedType != TypeKind::Int)
        return std::nullopt;

    return FirstWinsPattern{claimAccess, expected->value, desired->value};
}

static const VariableNode *asVariable(const ASTNode *node)
{
    return node && node->type == ASTNodeType::Variable
               ? static_cast<const VariableNode *>(node)
               : nullptr;
}

static const ArrayAccessNode *asArrayAccess(const ASTNode *node)
{
    return node && node->type == ASTNodeType::ArrayAccess
               ? static_cast<const ArrayAccessNode *>(node)
               : nullptr;
}

static bool matchesIncrementByOne(const ASTNode *node, const std::string &name)
{
    if (!node || node->type != ASTNodeType::AssignmentStmt)
        return false;
    auto *assignment = static_cast<const AssignmentStmtNode *>(node);
    auto *lhs = asVariable(assignment->lhs.get());
    if (!lhs || lhs->name != name || !assignment->rhs ||
        assignment->rhs->type != ASTNodeType::BinaryExpr)
        return false;

    auto *add = static_cast<const BinaryExprNode *>(assignment->rhs.get());
    if (add->op != "+")
        return false;
    auto isNamedVariable = [&](const ASTNode *operand) {
        auto *variable = asVariable(operand);
        return variable && variable->name == name;
    };
    auto isOne = [](const ASTNode *operand) {
        return operand && operand->type == ASTNodeType::IntLiteral &&
               static_cast<const IntLiteralNode *>(operand)->value == 1;
    };
    return (isNamedVariable(add->lhs.get()) && isOne(add->rhs.get())) ||
           (isOne(add->lhs.get()) && isNamedVariable(add->rhs.get()));
}

static bool isFirstWinsFrontierLoop(const WhileStmtNode *loop)
{
    if (!loop || !loop->condition || loop->condition->type != ASTNodeType::BinaryExpr)
        return false;

    auto *condition = static_cast<const BinaryExprNode *>(loop->condition.get());
    if (condition->op != "<")
        return false;
    auto *induction = asVariable(condition->lhs.get());
    auto *bound = asVariable(condition->rhs.get());
    if (!induction || !bound || induction->name == bound->name)
        return false;

    auto *body = dynamic_cast<const BlockStmtNode *>(loop->body.get());
    if (!body || body->statements.size() != 3)
        return false;

    if (body->statements[0]->type != ASTNodeType::VarDecl ||
        body->statements[1]->type != ASTNodeType::ForEachStmt ||
        !matchesIncrementByOne(body->statements[2].get(), induction->name))
        return false;

    auto *vertexDecl = static_cast<const VarDeclNode *>(body->statements[0].get());
    auto *frontierRead = asArrayAccess(vertexDecl->initializer.get());
    auto *frontierArray = frontierRead ? asVariable(frontierRead->arrayExpr.get()) : nullptr;
    if (!frontierRead || !frontierArray ||
        !firstWinsExprsEquivalent(frontierRead->indexExpr.get(), induction))
        return false;

    auto *neighbors = static_cast<const ForEachStmtNode *>(body->statements[1].get());
    auto *source = asVariable(neighbors->adjNodeExpr.get());
    if (neighbors->targetType != ForEachTargetType::Neighbor || !source ||
        source->name != vertexDecl->name)
        return false;

    auto *neighborBody = dynamic_cast<const BlockStmtNode *>(neighbors->body.get());
    if (!neighborBody || neighborBody->statements.size() != 1 ||
        neighborBody->statements[0]->type != ASTNodeType::Conditional)
        return false;

    auto *claimIf = static_cast<ConditionalNode *>(neighborBody->statements[0].get());
    auto claim = detectFirstWinsPattern(claimIf);
    if (!claim)
        return false;

    auto *claimArray = asVariable(claim->claimAccess->arrayExpr.get());
    auto *thenBody = dynamic_cast<const BlockStmtNode *>(claimIf->thenBlock.get());
    if (!claimArray || !thenBody || thenBody->statements.size() != 4)
        return false;

    auto *parentAssignment =
        dynamic_cast<const AssignmentStmtNode *>(thenBody->statements[1].get());
    auto *appendAssignment =
        dynamic_cast<const AssignmentStmtNode *>(thenBody->statements[2].get());
    if (!parentAssignment || !appendAssignment)
        return false;

    auto *parentAccess = asArrayAccess(parentAssignment->lhs.get());
    auto *parentArray = parentAccess ? asVariable(parentAccess->arrayExpr.get()) : nullptr;
    auto *parentValue = asVariable(parentAssignment->rhs.get());
    if (!parentAccess || !parentArray || !parentValue ||
        parentValue->name != vertexDecl->name ||
        !firstWinsExprsEquivalent(parentAccess->indexExpr.get(),
                                  claim->claimAccess->indexExpr.get()))
        return false;

    auto *appendAccess = asArrayAccess(appendAssignment->lhs.get());
    auto *appendArray = appendAccess ? asVariable(appendAccess->arrayExpr.get()) : nullptr;
    auto *appendSize = appendAccess ? asVariable(appendAccess->indexExpr.get()) : nullptr;
    if (!appendAccess || !appendArray || !appendSize ||
        !firstWinsExprsEquivalent(appendAssignment->rhs.get(),
                                  claim->claimAccess->indexExpr.get()) ||
        !matchesIncrementByOne(thenBody->statements[3].get(), appendSize->name))
        return false;

    return claimArray->name != parentArray->name &&
           claimArray->name != appendArray->name &&
           parentArray->name != appendArray->name &&
           frontierArray->name != claimArray->name &&
           frontierArray->name != parentArray->name &&
           frontierArray->name != appendArray->name &&
           appendSize->name != induction->name &&
           appendSize->name != bound->name;
}
} // namespace

void IRGenVisitor::visitConditional(ConditionalNode *ifs)
{
    std::optional<FirstWinsPattern> firstWins = detectFirstWinsPattern(ifs);
    llvm::Value *condBool = nullptr;
    if (firstWins)
    {
        auto *arrayVariable =
            static_cast<VariableNode *>(firstWins->claimAccess->arrayExpr.get());
        llvm::Value *arrayStorage = lookupNamedStorage(arrayVariable->name);
        llvm::Value *index = visitExpr(firstWins->claimAccess->indexExpr.get());
        if (index->getType() != Builder.getInt32Ty())
            index = Builder.CreateIntCast(index, Builder.getInt32Ty(), true);

        llvm::Value *elementPtr = nullptr;
        if (IndirectArrays.count(arrayVariable->name))
        {
            llvm::Value *dataPtr =
                Builder.CreateLoad(Builder.getPtrTy(), arrayStorage,
                                   arrayVariable->name + ".claim.ptr");
            elementPtr = Builder.CreateGEP(Builder.getInt32Ty(), dataPtr, {index},
                                           arrayVariable->name + ".claim.elemptr");
        }
        else if (auto *arrayTy =
                     llvm::dyn_cast<llvm::ArrayType>(getStorageValueType(arrayStorage)))
        {
            if (!arrayTy->getElementType()->isIntegerTy(32))
                firstWins.reset();
            else
                elementPtr = Builder.CreateGEP(
                    arrayTy, arrayStorage, {Builder.getInt32(0), index},
                    arrayVariable->name + ".claim.elemptr");
        }
        else
        {
            firstWins.reset();
        }

        if (firstWins && elementPtr)
        {
            llvm::Value *expected =
                llvm::ConstantInt::get(Builder.getInt32Ty(), firstWins->expectedValue, true);
            llvm::Value *desired =
                llvm::ConstantInt::get(Builder.getInt32Ty(), firstWins->desiredValue, true);
            llvm::AtomicCmpXchgInst *claim = Builder.CreateAtomicCmpXchg(
                elementPtr, expected, desired, llvm::MaybeAlign(4),
                llvm::AtomicOrdering::Monotonic, llvm::AtomicOrdering::Monotonic);
            claim->setMetadata(
                "sgpl.first_wins.claim",
                llvm::MDNode::get(Context, llvm::MDString::get(Context, arrayVariable->name)));
            condBool = Builder.CreateExtractValue(claim, 1, "first_wins.success");
        }
    }

    if (!condBool)
    {
        firstWins.reset();
        llvm::Value *condVal = visitExpr(ifs->condition.get());
        condBool = condVal;
        if (condVal->getType()->isIntegerTy(32))
            condBool = Builder.CreateICmpNE(condVal, Builder.getInt32(0), "ifcond");
        else if (condVal->getType()->isDoubleTy())
            condBool = Builder.CreateFCmpONE(
                condVal, llvm::ConstantFP::get(Builder.getDoubleTy(), 0.0), "ifcond");
    }

    // gets the current function. LLVM needs to know which function the blocks are part of
    llvm::Function *parent = Builder.GetInsertBlock()->getParent();
    auto *thenBB = llvm::BasicBlock::Create(Context, "then", parent);
    auto *elseBB = ifs->elseBlock
                       ? llvm::BasicBlock::Create(Context, "else")
                       : nullptr;
    auto *mergeBB = llvm::BasicBlock::Create(Context, "ifcont");

    // CreateCondBr creates a conditional jump instruction
    if (elseBB)
    {
        Builder.CreateCondBr(condBool, thenBB, elseBB);
    }
    else
    {
        Builder.CreateCondBr(condBool, thenBB, mergeBB);
    }

    Builder.SetInsertPoint(thenBB);
    auto *thenBlockNode = static_cast<BlockStmtNode *>(ifs->thenBlock.get());

    size_t firstThenStatement = firstWins ? 1 : 0;
    for (size_t i = firstThenStatement; i < thenBlockNode->statements.size(); ++i)
    {
        visitStatement(thenBlockNode->statements[i].get());
    }
    // after then, always jump to merge
    if (!Builder.GetInsertBlock()->getTerminator())
        Builder.CreateBr(mergeBB);

    // Emit else block (if present)
    if (elseBB)
    {
        elseBB->insertInto(parent);
        Builder.SetInsertPoint(elseBB);
        auto *elseBlockNode = static_cast<BlockStmtNode *>(ifs->elseBlock.get());
        for (auto &stmt : elseBlockNode->statements)
        {
            visitStatement(stmt.get());
        }
        if (!Builder.GetInsertBlock()->getTerminator())
            Builder.CreateBr(mergeBB);
    }
    mergeBB->insertInto(parent);
    Builder.SetInsertPoint(mergeBB);
}

void IRGenVisitor::visitStatement(ASTNode *node)
{
    switch (node->type)
    {
    case ASTNodeType::VarDecl:
        visitVarDecl(dynamic_cast<VarDeclNode *>(node));
        break;
    case ASTNodeType::AssignmentStmt:
        visitAssignment(dynamic_cast<AssignmentStmtNode *>(node));
        break;
    case ASTNodeType::ReturnStmt:
        visitReturnStmt(dynamic_cast<ReturnStmtNode *>(node));
        break;
    case ASTNodeType::Conditional:
        visitConditional(dynamic_cast<ConditionalNode *>(node));
        break;
    case ASTNodeType::BlockStmt:
        visitBlock(dynamic_cast<BlockStmtNode *>(node));
        break;
    // case ASTNodeType::ReturnStmt:
    //     visitReturnStmt(static_cast<ReturnStmtNode*>(node));
    //     break;
    case ASTNodeType::WhileStmt:
        visitWhile(static_cast<WhileStmtNode *>(node));
        break;
    case ASTNodeType::PrintStmt:
        visitPrintStmt(static_cast<PrintStmtNode *>(node));
        break;
    case ASTNodeType::SleepStmt:
        visitSleepStmt(static_cast<SleepStmtNode *>(node));
        break;

    case ASTNodeType::QueryNode:
        // std::cerr << "Entered queryNODe\n";
        visitQuery(static_cast<QueryNode *>(node));
        break;
    case ASTNodeType::GraphUpdate:
        visitGraphUpdate(static_cast<GraphUpdateNode *>(node));
        break;
    case ASTNodeType::ShowGraph:
        visitShowGraph(static_cast<ShowGraphNode *>(node));
        break;
    case ASTNodeType::GraphComprehension:
        visitGraphComprehension(static_cast<GraphComprehensionNode *>(node));
        break;
    case ASTNodeType::SetDecl:
        visitSetDecl(static_cast<SetDeclNode *>(node));
        break;
    case ASTNodeType::SetOperation:
        visitSetOperation(static_cast<SetOperationNode *>(node));
        break;
    case ASTNodeType::SetMethodCall:
        visitSetMethodCall(static_cast<SetMethodCallNode *>(node));
        break;
    case ASTNodeType::ForEachStmt:
        visitForEach(static_cast<ForEachStmtNode *>(node));
        break;
    case ASTNodeType::FunctionCall:
        visitExpr(node); // standalone function call as statement (e.g. timer())
        break;
    case ASTNodeType::BreakStmt:
    {
        if (LoopStack.empty())
            throw std::runtime_error("break outside of loop");
        Builder.CreateBr(LoopStack.back().mergeBB);
        // Create unreachable block for any code after break
        llvm::Function *parent = Builder.GetInsertBlock()->getParent();
        auto *deadBB = llvm::BasicBlock::Create(Context, "after.break", parent);
        Builder.SetInsertPoint(deadBB);
        break;
    }
    case ASTNodeType::ContinueStmt:
    {
        if (LoopStack.empty())
            throw std::runtime_error("continue outside of loop");
        Builder.CreateBr(LoopStack.back().condBB);
        llvm::Function *parent = Builder.GetInsertBlock()->getParent();
        auto *deadBB = llvm::BasicBlock::Create(Context, "after.continue", parent);
        Builder.SetInsertPoint(deadBB);
        break;
    }
    case ASTNodeType::SwapStmt:
        visitSwapStmt(static_cast<SwapStmtNode *>(node));
        break;
    default:
        std::cerr << "Unsupported statement type: " << static_cast<int>(node->type) << "\n";
        break;
    }
}

void IRGenVisitor::visitGraphUpdate(GraphUpdateNode *upd)
{
    llvm::Value *graphPtr = loadGraphValue(upd->graphName);

    auto *i32Ty = llvm::Type::getInt32Ty(Context);
    auto *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
    auto toI32 = [&](ASTNode *expr, const std::string &name) -> llvm::Value * {
        llvm::Value *value = visitExpr(expr);
        if (value->getType() == i32Ty)
            return value;
        if (value->getType()->isIntegerTy())
            return Builder.CreateIntCast(value, i32Ty, true, name);
        throw std::runtime_error("graph update expression did not lower to integer");
    };

    if (upd->kind == GraphUpdateKind::Add)
    {
        for (const auto &target : upd->targets)
        {
            if (target.kind == GraphUpdateTargetKind::Node)
            {
                llvm::FunctionType *addNodeFT = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(Context),
                    {i8PtrTy, i8PtrTy, i8PtrTy, i32Ty}, false);
                auto addNodeFn = Module.getOrInsertFunction("graph_add_node", addNodeFT);
                llvm::Value *nodesBmp = GraphNodesMap.count(upd->graphName)
                    ? GraphNodesMap[upd->graphName]
                    : llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(i8PtrTy));
                llvm::Value *edgePairs = GraphEdgesMap.count(upd->graphName)
                    ? GraphEdgesMap[upd->graphName]
                    : llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(i8PtrTy));
                Builder.CreateCall(addNodeFn, {graphPtr, nodesBmp, edgePairs,
                                               toI32(target.value.get(), "graph.add.node")});
                continue;
            }

            llvm::FunctionType *addEdgeFT = llvm::FunctionType::get(
                llvm::Type::getVoidTy(Context),
                {i8PtrTy, i8PtrTy, i32Ty, i32Ty, i32Ty}, false);
            auto addEdgeFn = Module.getOrInsertFunction("graph_add_edge", addEdgeFT);
            llvm::Value *edgesBmp = GraphEdgesMap.count(upd->graphName)
                ? GraphEdgesMap[upd->graphName]
                : llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(i8PtrTy));
            Builder.CreateCall(addEdgeFn, {graphPtr, edgesBmp,
                                           toI32(target.src.get(), "graph.add.src"),
                                           toI32(target.dst.get(), "graph.add.dst"),
                                           llvm::ConstantInt::get(i32Ty, 0)});
        }
    }
    else // Remove
    {
        for (const auto &target : upd->targets)
        {
            if (target.kind == GraphUpdateTargetKind::Node)
            {
                llvm::FunctionType *removeNodeFT = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(Context),
                    {i8PtrTy, i8PtrTy, i8PtrTy, i32Ty}, false);
                auto removeNodeFn = Module.getOrInsertFunction("graph_remove_node", removeNodeFT);
                llvm::Value *nodesBmp = GraphNodesMap.count(upd->graphName)
                    ? GraphNodesMap[upd->graphName]
                    : llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(i8PtrTy));
                llvm::Value *edgePairs = GraphEdgesMap.count(upd->graphName)
                    ? GraphEdgesMap[upd->graphName]
                    : llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(i8PtrTy));
                Builder.CreateCall(removeNodeFn, {graphPtr, nodesBmp, edgePairs,
                                                  toI32(target.value.get(), "graph.remove.node")});
                continue;
            }

            llvm::FunctionType *removeEdgeFT = llvm::FunctionType::get(
                llvm::Type::getVoidTy(Context),
                {i8PtrTy, i8PtrTy, i32Ty, i32Ty, i32Ty}, false);
            auto removeEdgeFn = Module.getOrInsertFunction("graph_remove_edge", removeEdgeFT);
            llvm::Value *edgesBmp = GraphEdgesMap.count(upd->graphName)
                ? GraphEdgesMap[upd->graphName]
                : llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(i8PtrTy));
            Builder.CreateCall(removeEdgeFn, {graphPtr, edgesBmp,
                                              toI32(target.src.get(), "graph.remove.src"),
                                              toI32(target.dst.get(), "graph.remove.dst"),
                                              llvm::ConstantInt::get(i32Ty, 0)});
        }
    }
}

void IRGenVisitor::visitBlock(BlockStmtNode *block)
{
    for (auto &stmt : block->statements)
    {
        visitStatement(stmt.get());
    }
}

void IRGenVisitor::visitReturnStmt(ReturnStmtNode *ret)
{
    if (ret->returnValue)
    {
        llvm::Value *val = visitExpr(ret->returnValue.get());
        Builder.CreateRet(val);
    }
    else
    {
        Builder.CreateRetVoid();
    }
}

void IRGenVisitor::visitAssignment(AssignmentStmtNode *assign)
{
    // std::cerr << "[IRGen] visitAssignment called for LHS type = " << static_cast<int>(assign->lhs->type) << "\n";

    // Evaluate RHS first
    llvm::Value *rhsVal = visitExpr(assign->rhs.get());

    // Case 1: LHS is a scalar variable
    if (assign->lhs->type == ASTNodeType::Variable)
    {
        auto *varNode = static_cast<VariableNode *>(assign->lhs.get());
        llvm::Value *storage = lookupNamedStorage(varNode->name);
        llvm::Type *allocTy = getStorageValueType(storage);

        // Cast RHS if needed
        if (rhsVal->getType() != allocTy)
        {
            if (allocTy->isDoubleTy() && rhsVal->getType()->isIntegerTy())
                rhsVal = Builder.CreateSIToFP(rhsVal, allocTy);
            else if (allocTy->isIntegerTy() && rhsVal->getType()->isIntegerTy())
                rhsVal = Builder.CreateIntCast(rhsVal, llvm::cast<llvm::IntegerType>(allocTy), true);
            else if (allocTy->isIntegerTy() && rhsVal->getType()->isDoubleTy())
                rhsVal = Builder.CreateFPToSI(rhsVal, llvm::cast<llvm::IntegerType>(allocTy));
            else
                throw std::runtime_error("IRGenVisitor: type mismatch in scalar assignment");
        }

        Builder.CreateStore(rhsVal, storage);
        return;
    }

    // Case 2: LHS is an array element access
    else if (assign->lhs->type == ASTNodeType::ArrayAccess)
    {
        auto *A = static_cast<ArrayAccessNode *>(assign->lhs.get());
        auto *baseVar = dynamic_cast<VariableNode *>(A->arrayExpr.get());
        if (!baseVar)
            throw std::runtime_error("IRGenVisitor: unsupported array base expression");

        llvm::Value *baseAlloca = lookupNamedStorage(baseVar->name);

        // Evaluate the index
        llvm::Value *idxVal = visitExpr(A->indexExpr.get());
        if (idxVal->getType() != Builder.getInt32Ty())
            idxVal = Builder.CreateIntCast(idxVal, Builder.getInt32Ty(), true);

        // Indirect (dynamic) array: alloca ptr → load → GEP
        if (IndirectRealArrays.count(baseVar->name))
        {
            auto *f64Ty = Builder.getDoubleTy();
            llvm::Value *dataPtr = Builder.CreateLoad(Builder.getPtrTy(), baseAlloca, baseVar->name + ".ptr");
            llvm::Value *elemPtr = Builder.CreateGEP(f64Ty, dataPtr, {idxVal}, baseVar->name + "_elemptr");

            if (rhsVal->getType() != f64Ty)
            {
                if (rhsVal->getType()->isIntegerTy())
                    rhsVal = Builder.CreateSIToFP(rhsVal, f64Ty);
                else if (!rhsVal->getType()->isDoubleTy())
                    throw std::runtime_error("IRGenVisitor: type mismatch in indirect real array assignment");
            }
            Builder.CreateStore(rhsVal, elemPtr);
            return;
        }

        if (IndirectArrays.count(baseVar->name))
        {
            auto *i32Ty = Builder.getInt32Ty();
            llvm::Value *dataPtr = Builder.CreateLoad(Builder.getPtrTy(), baseAlloca, baseVar->name + ".ptr");
            llvm::Value *elemPtr = Builder.CreateGEP(i32Ty, dataPtr, {idxVal}, baseVar->name + "_elemptr");

            if (rhsVal->getType() != i32Ty)
            {
                if (rhsVal->getType()->isIntegerTy())
                    rhsVal = Builder.CreateIntCast(rhsVal, i32Ty, true);
                else
                    throw std::runtime_error("IRGenVisitor: type mismatch in indirect array assignment");
            }
            Builder.CreateStore(rhsVal, elemPtr);
            return;
        }

        // Static array: alloca [N x i32]
        if (auto *arrTy = llvm::dyn_cast<llvm::ArrayType>(getStorageValueType(baseAlloca)))
        {
            llvm::Value *elemPtr = Builder.CreateGEP(
                arrTy,      // pointee type
                baseAlloca, // pointer
                {Builder.getInt32(0), idxVal},
                baseVar->name + "_elemptr");

            llvm::Type *elemTy = arrTy->getElementType();

            if (rhsVal->getType() != elemTy)
            {
                if (rhsVal->getType()->isIntegerTy() && elemTy->isIntegerTy())
                    rhsVal = Builder.CreateIntCast(rhsVal, llvm::cast<llvm::IntegerType>(elemTy), true);
                else
                    throw std::runtime_error("IRGenVisitor: type mismatch in array assignment");
            }

            Builder.CreateStore(rhsVal, elemPtr);
            return;
        }

        // Legacy dynamic array case (should not be reached with new indirect approach)
        else if (getStorageValueType(baseAlloca)->isIntegerTy(32))
        {
            auto *i32Ty = Builder.getInt32Ty();
            llvm::Value *elemPtr = Builder.CreateGEP(
                i32Ty, baseAlloca, {idxVal},
                baseVar->name + "_elemptr");

            if (rhsVal->getType() != i32Ty)
            {
                if (rhsVal->getType()->isIntegerTy())
                    rhsVal = Builder.CreateIntCast(rhsVal, i32Ty, true);
                else
                    throw std::runtime_error("IRGenVisitor: type mismatch in dynamic array assignment");
            }

            Builder.CreateStore(rhsVal, elemPtr);
            return;
        }

        else if (auto *ptrTy = llvm::dyn_cast<llvm::PointerType>(getStorageValueType(baseAlloca)))
        {
            llvm::Type *elemTy = ptrTy->getContainedType(0);

            llvm::Value *elemPtr = Builder.CreateGEP(
                elemTy,     // pointee type
                baseAlloca, // pointer
                {idxVal},   // index
                baseVar->name + "_elemptr");

            if (rhsVal->getType() != elemTy)
            {
                if (rhsVal->getType()->isIntegerTy() && elemTy->isIntegerTy())
                    rhsVal = Builder.CreateIntCast(rhsVal, llvm::cast<llvm::IntegerType>(elemTy), true);
                else
                    throw std::runtime_error("IRGenVisitor: type mismatch in pointer array assignment");
            }

            Builder.CreateStore(rhsVal, elemPtr);
            return;
        }

        else
        {
            throw std::runtime_error("IRGenVisitor: unsupported array storage type for " + baseVar->name);
        }
    }

    // Case 3: LHS is a 2D array element access
    else if (assign->lhs->type == ASTNodeType::Array2DAccess)
    {
        auto *a2d = static_cast<Array2DAccessNode *>(assign->lhs.get());
        auto *baseVar = dynamic_cast<VariableNode *>(a2d->arrayExpr.get());
        if (!baseVar)
            throw std::runtime_error("2D array base must be variable in assignment");

        auto metaIt = Array2DMap.find(baseVar->name);
        if (metaIt == Array2DMap.end())
            throw std::runtime_error("2D array metadata not found: " + baseVar->name);

        llvm::Value *rowIdx = visitExpr(a2d->rowExpr.get());
        llvm::Value *colIdx = visitExpr(a2d->colExpr.get());
        if (rowIdx->getType() != Builder.getInt32Ty())
            rowIdx = Builder.CreateIntCast(rowIdx, Builder.getInt32Ty(), true);
        if (colIdx->getType() != Builder.getInt32Ty())
            colIdx = Builder.CreateIntCast(colIdx, Builder.getInt32Ty(), true);

        llvm::Value *cols = metaIt->second.colsVal;
        llvm::Value *offset = Builder.CreateMul(rowIdx, cols, "row_offset");
        llvm::Value *flatIdx = Builder.CreateAdd(offset, colIdx, "flat_idx");

        auto *i32Ty = Builder.getInt32Ty();
        if (rhsVal->getType() != i32Ty)
        {
            if (rhsVal->getType()->isIntegerTy())
                rhsVal = Builder.CreateIntCast(rhsVal, i32Ty, true);
            else
                throw std::runtime_error("2D array assignment requires int value");
        }

        llvm::Value *baseAlloca = lookupNamedStorage(baseVar->name);
        llvm::Value *elemPtr = Builder.CreateGEP(i32Ty, baseAlloca, {flatIdx}, baseVar->name + "_2d_ptr");
        Builder.CreateStore(rhsVal, elemPtr);
        return;
    }

    // Unsupported LHS
    else
    {
        throw std::runtime_error("IRGenVisitor: Unsupported LHS in assignment");
    }
}

llvm::Value *IRGenVisitor::visitVarDecl(VarDeclNode *decl)
{
    llvm::Function *currentFunction = Builder.GetInsertBlock()->getParent();

    // --- 2D Array path: int arr[rows][cols] ---
    if (decl->isArray2D)
    {
        auto *i32Ty = Builder.getInt32Ty();

        // Get row count
        llvm::Value *rowsVal;
        if (decl->array2DRowsExpr)
            rowsVal = visitExpr(decl->array2DRowsExpr.get());
        else
            rowsVal = Builder.getInt32(static_cast<int>(decl->array2DRows));
        if (rowsVal->getType() != i32Ty)
            rowsVal = Builder.CreateIntCast(rowsVal, i32Ty, true);

        // Get col count
        llvm::Value *colsVal;
        if (decl->array2DColsExpr)
            colsVal = visitExpr(decl->array2DColsExpr.get());
        else
            colsVal = Builder.getInt32(static_cast<int>(decl->array2DCols));
        if (colsVal->getType() != i32Ty)
            colsVal = Builder.CreateIntCast(colsVal, i32Ty, true);

        // total = rows * cols
        llvm::Value *totalElems = Builder.CreateMul(rowsVal, colsVal, "arr2d_total");
        llvm::AllocaInst *arrAlloca = Builder.CreateAlloca(i32Ty, totalElems, decl->name);

        // Zero-initialize
        llvm::Value *sizeBytes = Builder.CreateMul(totalElems, Builder.getInt32(4), "arr2d_bytes");
        llvm::Value *sizeBytes64 = Builder.CreateZExt(sizeBytes, Builder.getInt64Ty());
        Builder.CreateMemSet(arrAlloca, Builder.getInt8(0), sizeBytes64, llvm::MaybeAlign(4));

        llvm::Value *storage = arrAlloca;
        if (EmittingTopLevel)
        {
            auto *global = new llvm::GlobalVariable(
                Module,
                arrAlloca->getType(),
                false,
                llvm::GlobalValue::InternalLinkage,
                llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(arrAlloca->getType())),
                decl->name);
            Builder.CreateStore(arrAlloca, global);
            storage = global;
        }
        NamedValues[decl->name] = storage;
        Array2DMap[decl->name] = {colsVal};
        return storage;
    }

    // --- Array path (explicit array or array-initializer) ---
    if (decl->isArray)
    {
        const bool isRealArray = (decl->typeName == "real");
        llvm::Type *elemTy = isRealArray ? Builder.getDoubleTy() : Builder.getInt32Ty();
        const unsigned elemBytes = isRealArray ? 8u : 4u;

        // --- Dynamic array: int arr[n] where n is a runtime expression ---
        // Uses pointer indirection so that swap(a,b) is O(1).
        // Layout: alloca ptr  →  points to  →  alloca elemTy, i32 %n  (zero-filled)
        if (decl->arraySizeExpr)
        {
            llvm::Value *sizeVal = visitExpr(decl->arraySizeExpr.get());
            if (sizeVal->getType() != Builder.getInt32Ty())
                sizeVal = Builder.CreateIntCast(sizeVal, Builder.getInt32Ty(), true);

            llvm::AllocaInst *dataAlloca = Builder.CreateAlloca(elemTy, sizeVal, decl->name + ".data");

            llvm::Value *sizeBytes = Builder.CreateMul(sizeVal, Builder.getInt32(elemBytes), "size_bytes");
            llvm::Value *sizeBytes64 = Builder.CreateZExt(sizeBytes, Builder.getInt64Ty());
            Builder.CreateMemSet(dataAlloca, Builder.getInt8(0), sizeBytes64, llvm::MaybeAlign(elemBytes));

            auto *ptrTy = Builder.getPtrTy();
            llvm::AllocaInst *ptrAlloca = Builder.CreateAlloca(ptrTy, nullptr, decl->name);
            Builder.CreateStore(dataAlloca, ptrAlloca);

            llvm::Value *storage = ptrAlloca;
            if (EmittingTopLevel)
            {
                auto *global = new llvm::GlobalVariable(
                    Module,
                    ptrTy,
                    false,
                    llvm::GlobalValue::InternalLinkage,
                    llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ptrTy)),
                    decl->name);
                Builder.CreateStore(dataAlloca, global);
                storage = global;
            }

            NamedValues[decl->name] = storage;
            if (isRealArray)
                IndirectRealArrays.insert(decl->name);
            else
                IndirectArrays.insert(decl->name);
            ArraySizes[decl->name] = sizeVal;
            return storage;
        }

        // --- Static array: int arr[5] or int arr[] = [1,2,3] ---
        size_t N = decl->arraySize;

        // If initializer present and is ArrayLiteral, we can verify/adjust
        if (decl->initializer && decl->initializer->type == ASTNodeType::ArrayLiteral)
        {
            auto *arrLit = static_cast<ArrayLiteralNode *>(decl->initializer.get());
            if (N == 0)
                N = arrLit->elements.size();
        }

        if (N == 0)
        {
            throw std::runtime_error("IRGenVisitor::visitVarDecl: array size is zero for " + decl->name);
        }

        // Build the LLVM array type [N x i32]
        llvm::ArrayType *arrTy = llvm::ArrayType::get(Builder.getInt32Ty(), N);

        // Create alloca in the entry block manually with arrTy
        llvm::IRBuilder<> tmpB(&currentFunction->getEntryBlock(),
                               currentFunction->getEntryBlock().begin());
        llvm::AllocaInst *arrAlloca = tmpB.CreateAlloca(arrTy, nullptr, decl->name);

        // If initializer exists, store each initializer element and zero-fill remaining
        if (decl->initializer && decl->initializer->type == ASTNodeType::ArrayLiteral)
        {
            auto *arrLit = static_cast<ArrayLiteralNode *>(decl->initializer.get());
            size_t M = arrLit->elements.size();
            // store min(M, N) elements
            for (size_t i = 0; i < M && i < N; ++i)
            {
                llvm::Value *val = visitExpr(arrLit->elements[i].get());
                if (val->getType() != Builder.getInt32Ty() && val->getType()->isIntegerTy())
                {
                    val = Builder.CreateIntCast(val, Builder.getInt32Ty(), /*isSigned=*/true);
                }
                llvm::Value *gep = Builder.CreateGEP(
                    arrTy, arrAlloca,
                    {Builder.getInt32(0), Builder.getInt32(static_cast<int>(i))},
                    decl->name + "_elem");
                Builder.CreateStore(val, gep);
            }
            // zero-fill remaining elements
            for (size_t i = M; i < N; ++i)
            {
                llvm::Value *zero = llvm::ConstantInt::get(Builder.getInt32Ty(), 0);
                llvm::Value *gep = Builder.CreateGEP(
                    arrTy, arrAlloca,
                    {Builder.getInt32(0), Builder.getInt32(static_cast<int>(i))},
                    decl->name + "_elem");
                Builder.CreateStore(zero, gep);
            }
        }

        llvm::Value *storage = arrAlloca;
        if (EmittingTopLevel)
        {
            auto *global = new llvm::GlobalVariable(
                Module,
                arrTy,
                false,
                llvm::GlobalValue::InternalLinkage,
                llvm::ConstantAggregateZero::get(arrTy),
                decl->name);
            storage = global;
            if (decl->initializer && decl->initializer->type == ASTNodeType::ArrayLiteral)
            {
                auto *arrLit = static_cast<ArrayLiteralNode *>(decl->initializer.get());
                size_t M = arrLit->elements.size();
                for (size_t i = 0; i < M && i < N; ++i)
                {
                    llvm::Value *val = visitExpr(arrLit->elements[i].get());
                    if (val->getType() != Builder.getInt32Ty() && val->getType()->isIntegerTy())
                        val = Builder.CreateIntCast(val, Builder.getInt32Ty(), true);
                    llvm::Value *gep = Builder.CreateGEP(
                        arrTy, global,
                        {Builder.getInt32(0), Builder.getInt32(static_cast<int>(i))},
                        decl->name + "_global_elem");
                    Builder.CreateStore(val, gep);
                }
            }
        }

        NamedValues[decl->name] = storage;
        return storage;
    }

    // --- Scalar path ---
    // Prefer annotated type, fallback to string parsing
    llvm::Type *declTy;
    if (decl->resolvedType != TypeKind::Unknown)
    {
        declTy = getLLVMTypeFromTypeKind(decl->resolvedType);
    }
    else
    {
        // Fallback to string parsing for unannotated nodes
        declTy = getLLVMTypeForName(decl->typeName);
    }
    llvm::Value *scalarAlloca = createEntryBlockAlloca(currentFunction, decl->name, declTy);
    if (EmittingTopLevel)
    {
        auto *global = new llvm::GlobalVariable(
            Module,
            declTy,
            false,
            llvm::GlobalValue::InternalLinkage,
            llvm::Constant::getNullValue(declTy),
            decl->name);
        scalarAlloca = global;
    }

    if (decl->initializer)
    {
        llvm::Value *initVal = visitExpr(decl->initializer.get());
        if (initVal->getType() != declTy)
        {
            if (declTy->isDoubleTy() && initVal->getType()->isIntegerTy())
                initVal = Builder.CreateSIToFP(initVal, declTy);
            else if (declTy->isIntegerTy() && initVal->getType()->isIntegerTy())
                initVal = Builder.CreateIntCast(initVal, declTy, true);
            else if (declTy->isIntegerTy() && initVal->getType()->isDoubleTy())
                initVal = Builder.CreateFPToSI(initVal, declTy);
            else
                throw std::runtime_error("IRGenVisitor: unsupported initializer cast for " + decl->name);
        }
        Builder.CreateStore(initVal, scalarAlloca);
    }
    else
    {
        // Safety: if this is a Set type (pointer), create an empty bitmap at runtime
        // instead of storing null (which would crash on set operations)
        if (decl->resolvedType == TypeKind::Set)
        {
            auto *i8PtrTy = llvm::PointerType::get(Context, 0);
            auto *i64Ty = Builder.getInt64Ty();
            llvm::FunctionType *createFT = llvm::FunctionType::get(
                i8PtrTy, {i64Ty, i64Ty}, false);
            auto createFn = Module.getOrInsertFunction("roaring_bitmap_create", createFT);
            llvm::Value *emptyBm = Builder.CreateCall(createFn,
                                                      {llvm::ConstantInt::get(i64Ty, 256),
                                                       llvm::ConstantInt::get(i64Ty, 8)},
                                                      decl->name + ".empty");
            Builder.CreateStore(emptyBm, scalarAlloca);
        }
        else
        {
            Builder.CreateStore(llvm::Constant::getNullValue(declTy), scalarAlloca);
        }
    }

    NamedValues[decl->name] = scalarAlloca;
    return scalarAlloca;
}
void IRGenVisitor::visitWhile(WhileStmtNode *ws)
{
    llvm::BasicBlock *preheader = Builder.GetInsertBlock();
    llvm::Function *parent = preheader->getParent();
    bool isVerifiedFrontier = isFirstWinsFrontierLoop(ws);

    auto *condBB = llvm::BasicBlock::Create(Context, "loopcond", parent);
    auto *bodyBB = llvm::BasicBlock::Create(Context, "loopbody", parent);
    auto *mergeBB = llvm::BasicBlock::Create(Context, "loopmerge", parent);

    LoopStack.push_back({condBB, mergeBB});

    Builder.SetInsertPoint(preheader);
    llvm::Value *frontierBound = nullptr;
    if (isVerifiedFrontier)
    {
        auto *condition = static_cast<BinaryExprNode *>(ws->condition.get());
        frontierBound = visitExpr(condition->rhs.get());
    }
    Builder.CreateBr(condBB);

    Builder.SetInsertPoint(condBB);
    llvm::Value *condV = nullptr;
    if (isVerifiedFrontier)
    {
        auto *condition = static_cast<BinaryExprNode *>(ws->condition.get());
        llvm::Value *induction = visitExpr(condition->lhs.get());
        condV = Builder.CreateICmpSLT(induction, frontierBound, "frontier.cond");
    }
    else
    {
        condV = visitExpr(ws->condition.get());
    }
    llvm::Value *condBool = condV;
    if (condV->getType()->isIntegerTy(32))
        condBool = Builder.CreateICmpNE(condV, Builder.getInt32(0), "whilecond");
    else if (condV->getType()->isDoubleTy())
        condBool = Builder.CreateFCmpONE(condV, llvm::ConstantFP::get(Builder.getDoubleTy(), 0.0), "whilecond");
    Builder.CreateCondBr(condBool, bodyBB, mergeBB);

    Builder.SetInsertPoint(bodyBB);
    bool wasEmittingTopLevel = EmittingTopLevel;
    EmittingTopLevel = false;
    visitBlock(static_cast<BlockStmtNode *>(ws->body.get()));
    EmittingTopLevel = wasEmittingTopLevel;
    if (!Builder.GetInsertBlock()->getTerminator())
        Builder.CreateBr(condBB);

    if (isVerifiedFrontier)
    {
        condBB->getTerminator()->setMetadata(
            "sgpl.frontier.first_wins.candidate",
            llvm::MDNode::get(Context, llvm::MDString::get(Context, "verified")));
    }

    LoopStack.pop_back();

    Builder.SetInsertPoint(mergeBB);
}

// void IRGenVisitor::visitForEach(ForEachStmtNode *fs) {
//     llvm::Function *parent = Builder.GetInsertBlock()->getParent();

//     // Lookup the graph pointer value from the map
//     llvm::Value *graphPtr = GraphMap[fs->graphName];
//     if (!graphPtr)
//         throw std::runtime_error("Graph not allocated in IR: " + fs->graphName);

//     llvm::StructType *graphTy = GraphTy;
//     if (!graphTy)
//         throw std::runtime_error("GraphTy is not properly initialized");

//     // --- Load n, row_ptr, col_idx from the graph ---
//     llvm::Value *nPtr = Builder.CreateStructGEP(graphTy, graphPtr, 0, "g_n_ptr");
//     llvm::Value *nVal = Builder.CreateLoad(llvm::Type::getInt64Ty(Context), nPtr, "n_val");

//     llvm::Value *rpPtrGEP = Builder.CreateStructGEP(graphTy, graphPtr, 2, "g_rp_ptr");
//     llvm::Value *rowPtr = Builder.CreateLoad(
//         llvm::PointerType::getUnqual(llvm::Type::getInt64Ty(Context)),
//         rpPtrGEP, "row_ptr");

//     llvm::Value *ciPtrGEP = Builder.CreateStructGEP(graphTy, graphPtr, 3, "g_ci_ptr");
//     llvm::Value *colPtr = Builder.CreateLoad(
//         llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(Context)),
//         ciPtrGEP, "col_ptr");

//     // --- Allocate loop index ---
//     llvm::AllocaInst *iAlloca = createEntryBlockAlloca(parent, "i");
//     Builder.CreateStore(llvm::ConstantInt::get(Context, llvm::APInt(64, 0)), iAlloca);

//     // --- Create basic blocks ---
//     llvm::BasicBlock *condBB = llvm::BasicBlock::Create(Context, "foreach.cond", parent);
//     llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(Context, "foreach.body", parent);
//     llvm::BasicBlock *incBB = llvm::BasicBlock::Create(Context, "foreach.inc", parent);
//     llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(Context, "foreach.merge", parent);

//     Builder.CreateBr(condBB);

//     // --- Condition block ---
//     Builder.SetInsertPoint(condBB);
//     llvm::Value *iVal = Builder.CreateLoad(llvm::Type::getInt64Ty(Context), iAlloca, "i_val");
//     llvm::Value *cond = Builder.CreateICmpSLT(iVal, nVal, "loop_cond");
//     Builder.CreateCondBr(cond, bodyBB, mergeBB);

//     // --- Body block ---
//     Builder.SetInsertPoint(bodyBB);
//     NamedValues[fs->var1] = iAlloca;

//     // --- Load row_ptr[i] and col_idx[i] ---
//     llvm::Value *rpElemPtr = Builder.CreateGEP(
//         llvm::Type::getInt64Ty(Context), rowPtr, iVal, "rp_elem_ptr");
//     llvm::Value *rpElem = Builder.CreateLoad(llvm::Type::getInt64Ty(Context), rpElemPtr, "rp_val");

//     llvm::Value *ciElemPtr = Builder.CreateGEP(
//         llvm::Type::getInt32Ty(Context), colPtr, iVal, "ci_elem_ptr");
//     llvm::Value *ciElem = Builder.CreateLoad(llvm::Type::getInt32Ty(Context), ciElemPtr, "ci_val");

//     // --- Print the values using printf ---
//     llvm::FunctionCallee printfFn = Module.getOrInsertFunction(
//         "printf",
//         llvm::FunctionType::get(
//             Builder.getInt32Ty(),
//             {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context))},
//             true));

//     llvm::Value *fmtStr = Builder.CreateGlobalStringPtr("%llu %d\n");
//     Builder.CreateCall(printfFn, {fmtStr, rpElem, ciElem});

//     // --- Visit the loop body if any ---
//     visitBlock(static_cast<BlockStmtNode *>(fs->body.get()));

//     if (!Builder.GetInsertBlock()->getTerminator())
//         Builder.CreateBr(incBB);

//     // --- Increment block ---
//     Builder.SetInsertPoint(incBB);
//     llvm::Value *iNext = Builder.CreateAdd(iVal, llvm::ConstantInt::get(Context, llvm::APInt(64, 1)), "i_next");
//     Builder.CreateStore(iNext, iAlloca);
//     Builder.CreateBr(condBB);

//     // --- Merge block ---
//     Builder.SetInsertPoint(mergeBB);
// }

// void IRGenVisitor::visitForEach(ForEachStmtNode *fs) {
//     // Parent function (needed to create entry-block allocas and basic blocks)
//     llvm::Function *parent = Builder.GetInsertBlock()->getParent();

//     // Look up the graph pointer value produced by visitGraphDecl
//     llvm::Value *graphPtr = GraphMap[fs->graphName];
//     if (!graphPtr)
//         throw std::runtime_error("Graph not allocated in IR: " + fs->graphName);

//     // Graph struct type (must have been set up in visitGraphDecl)
//     if (!GraphTy)
//         throw std::runtime_error("GraphTy not initialized in IRGenVisitor");

//     // Common types
//     llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
//     llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);

//     // --- Load number of vertices: n (field 0 of graph struct) ---
//     llvm::Value *nPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
//     llvm::Value *nVal = Builder.CreateLoad(i64Ty, nPtr, "n_val");

//     // --- Prepare entry-block allocas (so they survive across blocks) ---
//     // Use a temporary IRBuilder positioned at the very start of the function entry.
//     llvm::IRBuilder<> tmpB(&parent->getEntryBlock(), parent->getEntryBlock().begin());

//     // Create an alloca for the loop index (64-bit because row_ptr uses i64 offsets)
//     llvm::AllocaInst *idxAlloca = tmpB.CreateAlloca(i64Ty, nullptr, fs->var1 + ".idx");
//     // Create a user-variable alloca (32-bit) so printf("%d") / normal integer ops work.
//     // If the loop variable is intended to be 64-bit, adapt accordingly and change print formatting.
//     llvm::AllocaInst *userVarAlloca = tmpB.CreateAlloca(i32Ty, nullptr, fs->var1);

//     // Save any previous mapping for the loop variable so we can restore it afterwards
//     llvm::AllocaInst *oldVarAlloca = nullptr;
//     auto itOld = NamedValues.find(fs->var1);
//     if (itOld != NamedValues.end())
//         oldVarAlloca = itOld->second;

//     // Register the user variable alloca for loads inside the loop body
//     NamedValues[fs->var1] = userVarAlloca;

//     // Initialize idx = 0
//     Builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);

//     // Create blocks: cond / body / inc / merge
//     llvm::BasicBlock *condBB  = llvm::BasicBlock::Create(Context, "foreach.cond", parent);
//     llvm::BasicBlock *bodyBB  = llvm::BasicBlock::Create(Context, "foreach.body", parent);
//     llvm::BasicBlock *incBB   = llvm::BasicBlock::Create(Context, "foreach.inc", parent);
//     llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(Context, "foreach.merge", parent);

//     // Jump to cond
//     Builder.CreateBr(condBB);

//     // --- condBB: check idx < n ---
//     Builder.SetInsertPoint(condBB);
//     llvm::Value *idxVal = Builder.CreateLoad(i64Ty, idxAlloca, "idx_val");
//     llvm::Value *cond = Builder.CreateICmpSLT(idxVal, nVal, "idx_lt_n");
//     Builder.CreateCondBr(cond, bodyBB, mergeBB);

//     // --- bodyBB: write current idx into the user variable (as i32) and execute body ---
//     Builder.SetInsertPoint(bodyBB);
//     // truncate or cast idx (i64) -> i32 to store into user variable
//     llvm::Value *idx_i32 = nullptr;
//     if (idxVal->getType()->isIntegerTy(64))
//         idx_i32 = Builder.CreateTrunc(idxVal, i32Ty, "idx_trunc_i32");
//     else
//         idx_i32 = Builder.CreateIntCast(idxVal, i32Ty, /*isSigned=*/true, "idx_cast_i32");

//     Builder.CreateStore(idx_i32, userVarAlloca);

//     // Visit body (the user's statements). If the body uses the variable name, NamedValues points to userVarAlloca.
//     visitBlock(static_cast<BlockStmtNode *>(fs->body.get()));

//     // After body: if body didn't emit a terminator, branch to inc
//     if (!Builder.GetInsertBlock()->getTerminator())
//         Builder.CreateBr(incBB);

//     // --- incBB: idx = idx + 1; branch back to cond ---
//     Builder.SetInsertPoint(incBB);
//     llvm::Value *one64 = llvm::ConstantInt::get(i64Ty, 1);
//     llvm::Value *nextIdx = Builder.CreateAdd(idxVal, one64, "idx_next");
//     Builder.CreateStore(nextIdx, idxAlloca);
//     Builder.CreateBr(condBB);

//     // --- mergeBB: restore NamedValues and continue ---
//     Builder.SetInsertPoint(mergeBB);
//     if (oldVarAlloca)
//         NamedValues[fs->var1] = oldVarAlloca;
//     else
//         NamedValues.erase(fs->var1);

//     // Done
// }

void IRGenVisitor::visitForEach(ForEachStmtNode *fs)
{
    llvm::Function *parent = Builder.GetInsertBlock()->getParent();
    llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);

    // --- Handle for each element v in setVar ---
    if (fs->targetType == ForEachTargetType::Element)
    {
        // Look up the set variable
        auto *BitmapPtrTy = getBitmapPtrTy(Context);
        llvm::Value *setStorage = lookupNamedStorage(fs->graphName);
        llvm::Value *bm = Builder.CreateLoad(BitmapPtrTy, setStorage, fs->graphName + ".bm");

        // Get cardinality
        llvm::FunctionType *cardFT = llvm::FunctionType::get(i64Ty, {BitmapPtrTy}, false);
        auto cardFn = Module.getOrInsertFunction("roaring_bitmap_get_cardinality", cardFT);
        llvm::Value *count = Builder.CreateCall(cardFn, {bm}, "set.card");

        // Allocate loop index and user variable
        llvm::IRBuilder<> TmpB(&parent->getEntryBlock(), parent->getEntryBlock().begin());
        auto *idxAlloca = TmpB.CreateAlloca(i64Ty, nullptr, fs->var1 + ".idx64");
        auto *userAlloca = TmpB.CreateAlloca(i32Ty, nullptr, fs->var1);
        NamedValues[fs->var1] = userAlloca;

        Builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);

        auto *condBB = llvm::BasicBlock::Create(Context, "foreach_set.cond", parent);
        auto *bodyBB = llvm::BasicBlock::Create(Context, "foreach_set.body", parent);
        auto *incBB = llvm::BasicBlock::Create(Context, "foreach_set.inc", parent);
        auto *mergeBB = llvm::BasicBlock::Create(Context, "foreach_set.merge", parent);
        LoopStack.push_back({incBB, mergeBB});
        Builder.CreateBr(condBB);

        // Condition: idx < count
        Builder.SetInsertPoint(condBB);
        llvm::Value *idxVal = Builder.CreateLoad(i64Ty, idxAlloca, "set.idx");
        llvm::Value *cond = Builder.CreateICmpULT(idxVal, count, "set.cond");
        Builder.CreateCondBr(cond, bodyBB, mergeBB);

        // Body: v = roaring_bitmap_get_at_index(bm, idx)
        Builder.SetInsertPoint(bodyBB);
        llvm::Value *idx32 = Builder.CreateTrunc(
            Builder.CreateLoad(i64Ty, idxAlloca), i32Ty, "set.idx32");
        llvm::FunctionType *atIdxFT = llvm::FunctionType::get(i32Ty, {BitmapPtrTy, i32Ty}, false);
        auto atIdxFn = Module.getOrInsertFunction("roaring_bitmap_get_at_index", atIdxFT);
        llvm::Value *elem = Builder.CreateCall(atIdxFn, {bm, idx32}, "set.elem");
        Builder.CreateStore(elem, userAlloca);

        visitBlock(static_cast<BlockStmtNode *>(fs->body.get()));
        if (!Builder.GetInsertBlock()->getTerminator())
            Builder.CreateBr(incBB);

        // Increment
        Builder.SetInsertPoint(incBB);
        llvm::Value *nextIdx = Builder.CreateAdd(
            Builder.CreateLoad(i64Ty, idxAlloca),
            llvm::ConstantInt::get(i64Ty, 1), "set.idx.next");
        Builder.CreateStore(nextIdx, idxAlloca);
        Builder.CreateBr(condBB);

        LoopStack.pop_back();
        Builder.SetInsertPoint(mergeBB);
        return;
    }

    // --- Graph-based foreach (vertex, edge, neighbor) ---
    llvm::Value *graphPtr = loadGraphValue(fs->graphName);

    if (!GraphTy)
        throw std::runtime_error("GraphTy not initialized");

    auto *i64PtrTy = llvm::PointerType::get(i64Ty, 0);
    auto *i32PtrTy = llvm::PointerType::get(i32Ty, 0);

    // ====== Neighbor iteration: for each neighbor u of v in G ======
    if (fs->targetType == ForEachTargetType::Neighbor)
    {
        // Evaluate the source vertex expression (e.g., variable v)
        llvm::Value *srcVertex = visitExpr(fs->adjNodeExpr.get());
        if (srcVertex->getType() != i64Ty)
            srcVertex = Builder.CreateZExt(srcVertex, i64Ty, "src_i64");

        // Allocate the user-visible neighbor variable and a neighbor buffer.
        llvm::Function *parent = Builder.GetInsertBlock()->getParent();
        llvm::IRBuilder<> TmpB(&parent->getEntryBlock(), parent->getEntryBlock().begin());
        auto *userAlloca = TmpB.CreateAlloca(i32Ty, nullptr, fs->var1);
        NamedValues[fs->var1] = userAlloca;

        // nbuf: heap buffer for autograph_get_neighbors output sized from
        // graph->m (max possible neighbors per vertex). Using n is unsafe
        // when duplicate edges inflate degree past n. Heap allocation avoids
        // repeated large stack growth when a neighbor-foreach appears inside
        // another loop.
        llvm::Value *nForBuf = Builder.CreateLoad(
            i64Ty,
            Builder.CreateStructGEP(GraphTy, graphPtr, 1, "g_m_for_buf_ptr"),
            "m_for_buf");
        auto *voidTy = llvm::Type::getVoidTy(Context);
        auto *opaquePtrTy = llvm::PointerType::get(Context, 0);
        auto *i32PtrTyLocal = llvm::PointerType::getUnqual(i32Ty);
        auto *i64PtrTyLocal = llvm::PointerType::getUnqual(i64Ty);
        auto acquireScratchFn = Module.getOrInsertFunction(
            "autograph_neighbor_scratch_acquire",
            llvm::FunctionType::get(i32PtrTyLocal, {i64Ty}, false));
        auto releaseScratchFn = Module.getOrInsertFunction(
            "autograph_neighbor_scratch_release",
            llvm::FunctionType::get(voidTy, {}, false));
        llvm::Value *nbufAlloca =
            Builder.CreateCall(acquireScratchFn, {nForBuf}, "nbr_nbuf");

        // cnt alloca
        auto *cntAlloca = TmpB.CreateAlloca(i64Ty, nullptr, "nbr_cnt");

        // Declare autograph_get_neighbors(void*, i64, i32*, i64*)
        llvm::FunctionType *getNbrFT = llvm::FunctionType::get(
            voidTy, {opaquePtrTy, i64Ty, i32PtrTyLocal, i64PtrTyLocal}, false);
        auto getNbrFn = Module.getOrInsertFunction("autograph_get_neighbors", getNbrFT);

        auto *condBB = llvm::BasicBlock::Create(Context, "foreach_nbr.cond", parent);
        auto *bodyBB = llvm::BasicBlock::Create(Context, "foreach_nbr.body", parent);
        auto *incBB = llvm::BasicBlock::Create(Context, "foreach_nbr.inc", parent);
        auto *mergeBB = llvm::BasicBlock::Create(Context, "foreach_nbr.merge", parent);
        LoopStack.push_back({incBB, mergeBB});

        // Preheader: call autograph_get_neighbors(G, v, nbuf, &cnt)
        llvm::BasicBlock *preheaderBB = Builder.GetInsertBlock();
        Builder.CreateCall(getNbrFn, {graphPtr, srcVertex, nbufAlloca, cntAlloca});
        llvm::Value *cntVal = Builder.CreateLoad(i64Ty, cntAlloca, "nbr_cnt_val");
        Builder.CreateBr(condBB);

        // Condition: idx < cnt
        Builder.SetInsertPoint(condBB);
        auto *idxPhi = Builder.CreatePHI(i64Ty, 2, fs->var1 + ".nbr_idx");
        idxPhi->addIncoming(llvm::ConstantInt::get(i64Ty, 0), preheaderBB);
        llvm::Value *cmp = Builder.CreateICmpSLT(idxPhi, cntVal, "nbr_cond");
        Builder.CreateCondBr(cmp, bodyBB, mergeBB);

        // Attach autotuner.traverse metadata to the loop header terminator
        condBB->getTerminator()->setMetadata("autotuner.traverse",
            llvm::MDNode::get(Context, llvm::MDString::get(Context, "neighbor")));

        // Body: u = nbuf[idx]
        Builder.SetInsertPoint(bodyBB);
        llvm::Value *neighborPtr = Builder.CreateGEP(i32Ty, nbufAlloca, {idxPhi}, "nbr_buf_ptr");
        llvm::Value *neighborVal = Builder.CreateLoad(i32Ty, neighborPtr, "neighbor_val");
        Builder.CreateStore(neighborVal, userAlloca);

        // Visit body
        visitBlock(static_cast<BlockStmtNode *>(fs->body.get()));
        if (!Builder.GetInsertBlock()->getTerminator())
            Builder.CreateBr(incBB);

        // Increment
        Builder.SetInsertPoint(incBB);
        llvm::Value *nextIdx = Builder.CreateAdd(
            idxPhi,
            llvm::ConstantInt::get(i64Ty, 1), "nbr_next");
        Builder.CreateBr(condBB);
        idxPhi->addIncoming(nextIdx, incBB);

        LoopStack.pop_back();
        Builder.SetInsertPoint(mergeBB);
        Builder.CreateCall(releaseScratchFn);
        return;
    }

    // ====== Edge iteration: for each edge u, v in G ======
    if (fs->targetType == ForEachTargetType::Edge)
    {
        llvm::Value *nPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
        llvm::Value *nVal = Builder.CreateLoad(i64Ty, nPtr, "n_val");

        // Allocate variables
        llvm::IRBuilder<> TmpB(&parent->getEntryBlock(), parent->getEntryBlock().begin());
        auto *uIdxAlloca = TmpB.CreateAlloca(i64Ty, nullptr, "edge_u_idx");
        auto *jIdxAlloca = TmpB.CreateAlloca(i64Ty, nullptr, "edge_j_idx");
        auto *var1Alloca = TmpB.CreateAlloca(i32Ty, nullptr, fs->var1); // u
        auto *var2Alloca = TmpB.CreateAlloca(i32Ty, nullptr, fs->var2); // v
        NamedValues[fs->var1] = var1Alloca;
        NamedValues[fs->var2] = var2Alloca;

        // nbuf: heap buffer for autograph_get_neighbors output sized from
        // graph->m (max possible neighbors per vertex). Using n is unsafe
        // when duplicate edges inflate degree past n. Heap allocation avoids
        // repeated large stack growth while walking all vertices' adjacency
        // lists.
        auto *nForBufEdge = Builder.CreateLoad(
            i64Ty,
            Builder.CreateStructGEP(GraphTy, graphPtr, 1, "g_m_edge_buf_ptr"),
            "m_edge_buf");
        auto *voidTy = llvm::Type::getVoidTy(Context);
        auto *opaquePtrTy = llvm::PointerType::get(Context, 0);
        auto *i32PtrTyLocal = llvm::PointerType::getUnqual(i32Ty);
        auto *i64PtrTyLocal = llvm::PointerType::getUnqual(i64Ty);
        llvm::FunctionType *mallocFT =
            llvm::FunctionType::get(opaquePtrTy, {i64Ty}, false);
        llvm::FunctionType *freeFT =
            llvm::FunctionType::get(voidTy, {opaquePtrTy}, false);
        auto mallocFn = Module.getOrInsertFunction("malloc", mallocFT);
        auto freeFn = Module.getOrInsertFunction("free", freeFT);
        llvm::Value *edgeBufBytes = Builder.CreateMul(
            nForBufEdge, llvm::ConstantInt::get(i64Ty, sizeof(int32_t)),
            "edge_nbuf_bytes");
        llvm::Value *edgeBufRaw =
            Builder.CreateCall(mallocFn, {edgeBufBytes}, "edge_nbuf_raw");
        auto *nbufAlloca =
            Builder.CreateBitCast(edgeBufRaw, i32PtrTyLocal, "edge_nbuf");
        auto *cntAlloca = Builder.CreateAlloca(i64Ty, nullptr, "edge_cnt");

        // Declare autograph_get_neighbors
        llvm::FunctionType *getNbrFT = llvm::FunctionType::get(
            voidTy, {opaquePtrTy, i64Ty, i32PtrTyLocal, i64PtrTyLocal}, false);
        auto getNbrFn = Module.getOrInsertFunction("autograph_get_neighbors", getNbrFT);
        if (auto *F = llvm::dyn_cast<llvm::Function>(getNbrFn.getCallee()))
            F->setMemoryEffects(llvm::MemoryEffects::argMemOnly());

        // u = 0
        Builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), uIdxAlloca);

        // Outer loop blocks
        auto *outerCondBB = llvm::BasicBlock::Create(Context, "edge.outer.cond", parent);
        auto *outerBodyBB = llvm::BasicBlock::Create(Context, "edge.outer.body", parent);
        auto *outerIncBB = llvm::BasicBlock::Create(Context, "edge.outer.inc", parent);
        auto *outerMergeBB = llvm::BasicBlock::Create(Context, "edge.outer.merge", parent);
        LoopStack.push_back({outerIncBB, outerMergeBB});

        Builder.CreateBr(outerCondBB);

        // Outer condition: u < n
        Builder.SetInsertPoint(outerCondBB);
        llvm::Value *uIdx = Builder.CreateLoad(i64Ty, uIdxAlloca, "u_idx");
        llvm::Value *outerCond = Builder.CreateICmpSLT(uIdx, nVal, "edge_outer_cond");
        Builder.CreateCondBr(outerCond, outerBodyBB, outerMergeBB);

        // Attach autotuner.traverse metadata to the outer loop header terminator
        outerCondBB->getTerminator()->setMetadata("autotuner.traverse",
            llvm::MDNode::get(Context, llvm::MDString::get(Context, "edge")));

        // Outer body: set u, call autograph_get_neighbors, start inner loop
        Builder.SetInsertPoint(outerBodyBB);
        llvm::Value *uVal32 = Builder.CreateTrunc(
            Builder.CreateLoad(i64Ty, uIdxAlloca, "u_idx2"), i32Ty, "u_val32");
        Builder.CreateStore(uVal32, var1Alloca);

        // Call autograph_get_neighbors(G, u, nbuf, &cnt)
        llvm::Value *uCur = Builder.CreateLoad(i64Ty, uIdxAlloca, "u_cur");
        Builder.CreateCall(getNbrFn, {graphPtr, uCur, nbufAlloca, cntAlloca});
        llvm::Value *cntVal = Builder.CreateLoad(i64Ty, cntAlloca, "edge_cnt_val");

        Builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), jIdxAlloca);

        // Inner loop blocks
        auto *innerCondBB = llvm::BasicBlock::Create(Context, "edge.inner.cond", parent);
        auto *innerBodyBB = llvm::BasicBlock::Create(Context, "edge.inner.body", parent);
        auto *innerIncBB = llvm::BasicBlock::Create(Context, "edge.inner.inc", parent);
        auto *innerMergeBB = llvm::BasicBlock::Create(Context, "edge.inner.merge", parent);

        Builder.CreateBr(innerCondBB);

        // Inner condition: j < cnt
        Builder.SetInsertPoint(innerCondBB);
        llvm::Value *jIdx = Builder.CreateLoad(i64Ty, jIdxAlloca, "j_idx");
        llvm::Value *innerCond = Builder.CreateICmpSLT(jIdx, cntVal, "edge_inner_cond");
        Builder.CreateCondBr(innerCond, innerBodyBB, innerMergeBB);

        // Inner body: v = nbuf[j], skip if u >= v (avoid undirected duplicates)
        Builder.SetInsertPoint(innerBodyBB);
        llvm::Value *jCur = Builder.CreateLoad(i64Ty, jIdxAlloca, "j_cur");
        llvm::Value *vPtr = Builder.CreateGEP(i32Ty, nbufAlloca, {jCur}, "nbuf_j");
        llvm::Value *vVal = Builder.CreateLoad(i32Ty, vPtr, "v_val");

        // Skip duplicate edges: only process u < v for undirected graphs
        llvm::Value *uForCmp = Builder.CreateLoad(i32Ty, var1Alloca, "u_cmp");
        llvm::Value *skipCond = Builder.CreateICmpSGE(uForCmp, vVal, "skip_dup");
        auto *userBodyBB = llvm::BasicBlock::Create(Context, "edge.user.body", parent);
        Builder.CreateCondBr(skipCond, innerIncBB, userBodyBB);

        // User body: store v, execute user loop body
        Builder.SetInsertPoint(userBodyBB);
        Builder.CreateStore(vVal, var2Alloca);

        visitBlock(static_cast<BlockStmtNode *>(fs->body.get()));
        if (!Builder.GetInsertBlock()->getTerminator())
            Builder.CreateBr(innerIncBB);

        // Inner increment
        Builder.SetInsertPoint(innerIncBB);
        llvm::Value *jNext = Builder.CreateAdd(
            Builder.CreateLoad(i64Ty, jIdxAlloca, "j_inc"),
            llvm::ConstantInt::get(i64Ty, 1), "j_next");
        Builder.CreateStore(jNext, jIdxAlloca);
        Builder.CreateBr(innerCondBB);

        // Inner merge → outer increment
        Builder.SetInsertPoint(innerMergeBB);
        Builder.CreateBr(outerIncBB);

        // Outer increment
        Builder.SetInsertPoint(outerIncBB);
        llvm::Value *uNext = Builder.CreateAdd(
            Builder.CreateLoad(i64Ty, uIdxAlloca, "u_inc"),
            llvm::ConstantInt::get(i64Ty, 1), "u_next");
        Builder.CreateStore(uNext, uIdxAlloca);
        Builder.CreateBr(outerCondBB);

        // Outer merge
        LoopStack.pop_back();
        Builder.SetInsertPoint(outerMergeBB);
        Builder.CreateCall(freeFn, {edgeBufRaw});
        return;
    }

    // ====== Vertex iteration: for each vertex v in G ======
    // --- Load n (number of vertices) ---
    llvm::Value *nPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
    llvm::Value *nVal = Builder.CreateLoad(i64Ty, nPtr, "n_val");

    // Allocate loop index (i64) and user variable (i32)
    llvm::IRBuilder<> TmpB(&parent->getEntryBlock(), parent->getEntryBlock().begin());
    auto *idxAlloca = TmpB.CreateAlloca(i64Ty, nullptr, fs->var1 + ".idx64");
    auto *userAlloca = TmpB.CreateAlloca(i32Ty, nullptr, fs->var1);

    NamedValues[fs->var1] = userAlloca;

    // idx = 0
    Builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);

    // Blocks
    auto *condBB = llvm::BasicBlock::Create(Context, "foreach.cond", parent);
    auto *bodyBB = llvm::BasicBlock::Create(Context, "foreach.body", parent);
    auto *incBB = llvm::BasicBlock::Create(Context, "foreach.inc", parent);
    auto *mergeBB = llvm::BasicBlock::Create(Context, "foreach.merge", parent);
    LoopStack.push_back({incBB, mergeBB});

    Builder.CreateBr(condBB);

    // --- Condition ---
    Builder.SetInsertPoint(condBB);
    llvm::Value *idxVal = Builder.CreateLoad(i64Ty, idxAlloca, "idx_val");
    llvm::Value *cond = Builder.CreateICmpSLT(idxVal, nVal, "loop_cond");
    Builder.CreateCondBr(cond, bodyBB, mergeBB);

    // Attach autotuner.traverse metadata to the loop header terminator
    condBB->getTerminator()->setMetadata("autotuner.traverse",
        llvm::MDNode::get(Context, llvm::MDString::get(Context, "vertex")));

    // --- Body ---
    Builder.SetInsertPoint(bodyBB);

    // Cast/truncate idxVal (i64) → i32 safely
    llvm::Value *idxVal32 = Builder.CreateTrunc(idxVal, i32Ty, "idx32");

    // Store into user variable
    Builder.CreateStore(idxVal32, userAlloca);

    // Visit loop body
    visitBlock(static_cast<BlockStmtNode *>(fs->body.get()));
    if (!Builder.GetInsertBlock()->getTerminator())
        Builder.CreateBr(incBB);

    // --- Increment ---
    Builder.SetInsertPoint(incBB);
    llvm::Value *nextIdx = Builder.CreateAdd(
        idxVal,
        llvm::ConstantInt::get(i64Ty, 1),
        "idx_next");
    Builder.CreateStore(nextIdx, idxAlloca);
    Builder.CreateBr(condBB);

    // --- Merge ---
    LoopStack.pop_back();
    Builder.SetInsertPoint(mergeBB);
}

llvm::Value *IRGenVisitor::visitExpr(ASTNode *expr)
{
    switch (expr->type)
    {
    case ASTNodeType::IntLiteral:
    {
        auto *intLit = static_cast<IntLiteralNode *>(expr);
        return llvm::ConstantInt::get(Builder.getInt32Ty(), intLit->value);
    }
    case ASTNodeType::BoolLiteral:
    {
        auto *bl = static_cast<BoolLiteralNode *>(expr);
        return llvm::ConstantInt::get(Builder.getInt1Ty(), bl->value ? 1 : 0);
    }
    case ASTNodeType::RealLiteral:
    {
        auto *rl = static_cast<RealLiteralNode *>(expr);
        return llvm::ConstantFP::get(Builder.getDoubleTy(), rl->value);
    }
    case ASTNodeType::Variable:
    {
        auto *varNode = static_cast<VariableNode *>(expr);
        llvm::Value *storage = lookupNamedStorage(varNode->name);
        return Builder.CreateLoad(getStorageValueType(storage), storage, varNode->name);
    }
    case ASTNodeType::BinaryExpr:
    {
        auto *bin = static_cast<BinaryExprNode *>(expr);
        llvm::Value *lhs = visitExpr(bin->lhs.get());
        llvm::Value *rhs = visitExpr(bin->rhs.get());
        bool isFP = lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy();
        if (isFP)
        {
            if (!lhs->getType()->isDoubleTy())
                lhs = Builder.CreateSIToFP(lhs, Builder.getDoubleTy());
            if (!rhs->getType()->isDoubleTy())
                rhs = Builder.CreateSIToFP(rhs, Builder.getDoubleTy());
        }

        if (bin->op == "+")
            return isFP ? Builder.CreateFAdd(lhs, rhs, "faddtmp")
                        : Builder.CreateAdd(lhs, rhs, "addtmp");
        if (bin->op == "-")
            return isFP ? Builder.CreateFSub(lhs, rhs, "fsubtmp")
                        : Builder.CreateSub(lhs, rhs, "subtmp");
        if (bin->op == "*")
            return isFP ? Builder.CreateFMul(lhs, rhs, "fmultmp")
                        : Builder.CreateMul(lhs, rhs, "multmp");
        if (bin->op == "/")
            return isFP ? Builder.CreateFDiv(lhs, rhs, "fdivtmp")
                        : Builder.CreateSDiv(lhs, rhs, "divtmp");
        if (bin->op == "%")
            return Builder.CreateSRem(lhs, rhs, "modtmp");
        if (bin->op == "&&")
            return Builder.CreateAnd(lhs, rhs, "andtmp");
        if (bin->op == "||")
            return Builder.CreateOr(lhs, rhs, "ortmp");

        if (bin->op == "==")
            return isFP ? Builder.CreateFCmpUEQ(lhs, rhs, "feqtmp")
                        : Builder.CreateICmpEQ(lhs, rhs, "eqtmp");
        if (bin->op == "!=")
            return isFP ? Builder.CreateFCmpUNE(lhs, rhs, "fnetmp")
                        : Builder.CreateICmpNE(lhs, rhs, "netmp");
        if (bin->op == "<")
            return isFP ? Builder.CreateFCmpULT(lhs, rhs, "flttmp")
                        : Builder.CreateICmpSLT(lhs, rhs, "lttmp");
        if (bin->op == "<=")
            return isFP ? Builder.CreateFCmpULE(lhs, rhs, "fletmp")
                        : Builder.CreateICmpSLE(lhs, rhs, "letmp");
        if (bin->op == ">")
            return isFP ? Builder.CreateFCmpUGT(lhs, rhs, "fgttmp")
                        : Builder.CreateICmpSGT(lhs, rhs, "gttmp");
        if (bin->op == ">=")
            return isFP ? Builder.CreateFCmpUGE(lhs, rhs, "fgetmp")
                        : Builder.CreateICmpSGE(lhs, rhs, "getmp");

        throw std::runtime_error("Unsupported binary operator: " + bin->op);
    }
    case ASTNodeType::FunctionCall:
    {
        auto *FC = static_cast<FunctionCallNode *>(expr);

        // Handle built-in timer() function
        if (FC->name == "timer")
        {
            // Declare: double timer_runtime(void);
            llvm::FunctionType *timerFT = llvm::FunctionType::get(
                Builder.getDoubleTy(), false);
            llvm::FunctionCallee timerDecl =
                Module.getOrInsertFunction("timer_runtime", timerFT);
            return Builder.CreateCall(timerDecl, {}, "timertmp");
        }

        // Built-in: numVertices(G) -> int
        if (FC->name == "numVertices")
        {
            auto *varArg = dynamic_cast<VariableNode *>(FC->arguments[0].get());
            if (!varArg)
                throw std::runtime_error("numVertices argument must be a graph name");
            llvm::Value *graphPtr = loadGraphValue(varArg->name);
            llvm::Value *nPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
            llvm::Value *nVal = Builder.CreateLoad(llvm::Type::getInt64Ty(Context), nPtr, "n_val");
            return Builder.CreateTrunc(nVal, Builder.getInt32Ty(), "numVertices");
        }

        // Built-in: numEdges(G) -> int (returns undirected edge count = m/2)
        if (FC->name == "numEdges")
        {
            auto *varArg = dynamic_cast<VariableNode *>(FC->arguments[0].get());
            if (!varArg)
                throw std::runtime_error("numEdges argument must be a graph name");
            llvm::Value *graphPtr = loadGraphValue(varArg->name);
            llvm::Value *mPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 1, "g_m_ptr");
            llvm::Value *mVal = Builder.CreateLoad(llvm::Type::getInt64Ty(Context), mPtr, "m_val");
            llvm::Value *mHalf = Builder.CreateUDiv(mVal,
                                                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), 2), "m_half");
            return Builder.CreateTrunc(mHalf, Builder.getInt32Ty(), "numEdges");
        }

        // Built-in: degree(G, v) -> int
        if (FC->name == "degree")
        {
            auto *varArg = dynamic_cast<VariableNode *>(FC->arguments[0].get());
            if (!varArg)
                throw std::runtime_error("degree first argument must be a graph name");
            llvm::Value *graphPtr = loadGraphValue(varArg->name);
            auto *i64Ty = llvm::Type::getInt64Ty(Context);
            auto *i64PtrTy = llvm::PointerType::get(i64Ty, 0);
            // Load row_ptr pointer (field 2)
            llvm::Value *rpPtrField = Builder.CreateStructGEP(GraphTy, graphPtr, 2, "g_rp_ptr_field");
            llvm::Value *rpBase = Builder.CreateLoad(i64PtrTy, rpPtrField, "rp_base");
            // Evaluate vertex v
            llvm::Value *v = visitExpr(FC->arguments[1].get());
            if (v->getType() != Builder.getInt32Ty())
                v = Builder.CreateIntCast(v, Builder.getInt32Ty(), true);
            llvm::Value *v64 = Builder.CreateZExt(v, i64Ty, "v64");
            llvm::Value *v1_64 = Builder.CreateAdd(v64, llvm::ConstantInt::get(i64Ty, 1), "v1_64");
            // row_ptr[v] and row_ptr[v+1]
            llvm::Value *ptr_v = Builder.CreateGEP(i64Ty, rpBase, v64, "rp_v");
            llvm::Value *ptr_v1 = Builder.CreateGEP(i64Ty, rpBase, v1_64, "rp_v1");
            llvm::Value *val_v = Builder.CreateLoad(i64Ty, ptr_v, "val_v");
            llvm::Value *val_v1 = Builder.CreateLoad(i64Ty, ptr_v1, "val_v1");
            llvm::Value *deg64 = Builder.CreateSub(val_v1, val_v, "deg64");
            return Builder.CreateTrunc(deg64, Builder.getInt32Ty(), "degree");
        }

        // Built-in: setSize(s) -> int (cardinality of a roaring bitmap)
        if (FC->name == "setSize")
        {
            auto *varArg = dynamic_cast<VariableNode *>(FC->arguments[0].get());
            if (!varArg)
                throw std::runtime_error("setSize argument must be a set variable name");
            llvm::Value *storage = lookupNamedStorage(varArg->name);
            auto *BitmapPtrTy = getBitmapPtrTy(Context);
            auto *i64Ty = llvm::Type::getInt64Ty(Context);
            llvm::Value *bm = Builder.CreateLoad(BitmapPtrTy, storage, varArg->name + ".bm");
            llvm::FunctionType *cardFT = llvm::FunctionType::get(i64Ty, {BitmapPtrTy}, false);
            auto cardFn = Module.getOrInsertFunction("roaring_bitmap_get_cardinality", cardFT);
            llvm::Value *card64 = Builder.CreateCall(cardFn, {bm}, "card64");
            return Builder.CreateTrunc(card64, Builder.getInt32Ty(), "setSize");
        }

        // Built-in: min(a, b) -> int
        if (FC->name == "min" || FC->name == "max")
        {
            llvm::Value *a = visitExpr(FC->arguments[0].get());
            llvm::Value *b = visitExpr(FC->arguments[1].get());
            bool isFloat = a->getType()->isDoubleTy() || b->getType()->isDoubleTy();
            if (isFloat)
            {
                if (!a->getType()->isDoubleTy())
                    a = Builder.CreateSIToFP(a, Builder.getDoubleTy());
                if (!b->getType()->isDoubleTy())
                    b = Builder.CreateSIToFP(b, Builder.getDoubleTy());
                llvm::Value *cmp = (FC->name == "min")
                                       ? Builder.CreateFCmpOLT(a, b, "min.cmp")
                                       : Builder.CreateFCmpOGT(a, b, "max.cmp");
                return Builder.CreateSelect(cmp, a, b, FC->name + ".val");
            }
            else
            {
                if (a->getType() != Builder.getInt32Ty())
                    a = Builder.CreateIntCast(a, Builder.getInt32Ty(), true);
                if (b->getType() != Builder.getInt32Ty())
                    b = Builder.CreateIntCast(b, Builder.getInt32Ty(), true);
                llvm::Value *cmp = (FC->name == "min")
                                       ? Builder.CreateICmpSLT(a, b, "min.cmp")
                                       : Builder.CreateICmpSGT(a, b, "max.cmp");
                return Builder.CreateSelect(cmp, a, b, FC->name + ".val");
            }
        }

        // Built-in: hasEdge(G, u, v) -> bool
        if (FC->name == "hasEdge")
        {
            auto *varArg = dynamic_cast<VariableNode *>(FC->arguments[0].get());
            if (!varArg)
                throw std::runtime_error("hasEdge first argument must be a graph name");
            llvm::Value *graphPtr = loadGraphValue(varArg->name);
            auto *i64Ty = llvm::Type::getInt64Ty(Context);
            auto *i32Ty = Builder.getInt32Ty();
            auto *i64PtrTy = llvm::PointerType::get(i64Ty, 0);
            auto *i32PtrTy = llvm::PointerType::get(i32Ty, 0);
            // Load row_ptr and col_idx
            llvm::Value *rpPtrField = Builder.CreateStructGEP(GraphTy, graphPtr, 2, "g_rp_ptr_field");
            llvm::Value *rpBase = Builder.CreateLoad(i64PtrTy, rpPtrField, "rp_base");
            llvm::Value *ciPtrField = Builder.CreateStructGEP(GraphTy, graphPtr, 3, "g_ci_ptr_field");
            llvm::Value *ciBase = Builder.CreateLoad(i32PtrTy, ciPtrField, "ci_base");
            // Evaluate u and v
            llvm::Value *u = visitExpr(FC->arguments[1].get());
            llvm::Value *v = visitExpr(FC->arguments[2].get());
            if (u->getType() != i32Ty)
                u = Builder.CreateIntCast(u, i32Ty, true);
            if (v->getType() != i32Ty)
                v = Builder.CreateIntCast(v, i32Ty, true);
            llvm::Value *u64 = Builder.CreateZExt(u, i64Ty);
            llvm::Value *u1_64 = Builder.CreateAdd(u64, llvm::ConstantInt::get(i64Ty, 1));
            // row_ptr[u] and row_ptr[u+1]
            llvm::Value *start64 = Builder.CreateLoad(i64Ty, Builder.CreateGEP(i64Ty, rpBase, u64));
            llvm::Value *end64 = Builder.CreateLoad(i64Ty, Builder.CreateGEP(i64Ty, rpBase, u1_64));
            // Loop through col_idx[start..end) looking for v
            llvm::Function *parent = Builder.GetInsertBlock()->getParent();
            auto *loopBB = llvm::BasicBlock::Create(Context, "hasedge.loop", parent);
            auto *foundBB = llvm::BasicBlock::Create(Context, "hasedge.found", parent);
            auto *notFoundBB = llvm::BasicBlock::Create(Context, "hasedge.notfound", parent);
            auto *mergeBB = llvm::BasicBlock::Create(Context, "hasedge.merge", parent);
            llvm::IRBuilder<> TmpB(&parent->getEntryBlock(), parent->getEntryBlock().begin());
            auto *idxAlloca = TmpB.CreateAlloca(i64Ty, nullptr, "hasedge.idx");
            Builder.CreateStore(start64, idxAlloca);
            Builder.CreateBr(loopBB);
            // Loop condition
            Builder.SetInsertPoint(loopBB);
            llvm::Value *idx = Builder.CreateLoad(i64Ty, idxAlloca, "he.idx");
            llvm::Value *cond = Builder.CreateICmpSLT(idx, end64, "he.cond");
            Builder.CreateCondBr(cond, /* loop body */ loopBB->getNextNode() ? loopBB : foundBB, notFoundBB);
            // Inline body: check col_idx[idx] == v
            auto *bodyBB = llvm::BasicBlock::Create(Context, "hasedge.body", parent);
            // Re-wire the conditional branch
            loopBB->getTerminator()->eraseFromParent();
            Builder.SetInsertPoint(loopBB);
            idx = Builder.CreateLoad(i64Ty, idxAlloca, "he.idx2");
            cond = Builder.CreateICmpSLT(idx, end64, "he.cond2");
            Builder.CreateCondBr(cond, bodyBB, notFoundBB);
            // Body
            Builder.SetInsertPoint(bodyBB);
            llvm::Value *colVal = Builder.CreateLoad(i32Ty,
                                                     Builder.CreateGEP(i32Ty, ciBase, Builder.CreateLoad(i64Ty, idxAlloca)));
            llvm::Value *match = Builder.CreateICmpEQ(colVal, v, "he.match");
            auto *incBB = llvm::BasicBlock::Create(Context, "hasedge.inc", parent);
            Builder.CreateCondBr(match, foundBB, incBB);
            // Increment
            Builder.SetInsertPoint(incBB);
            llvm::Value *nextIdx = Builder.CreateAdd(
                Builder.CreateLoad(i64Ty, idxAlloca),
                llvm::ConstantInt::get(i64Ty, 1));
            Builder.CreateStore(nextIdx, idxAlloca);
            Builder.CreateBr(loopBB);
            // Found
            Builder.SetInsertPoint(foundBB);
            Builder.CreateBr(mergeBB);
            // Not found
            Builder.SetInsertPoint(notFoundBB);
            Builder.CreateBr(mergeBB);
            // Merge with PHI
            Builder.SetInsertPoint(mergeBB);
            auto *phi = Builder.CreatePHI(Builder.getInt1Ty(), 2, "hasedge.result");
            phi->addIncoming(Builder.getTrue(), foundBB);
            phi->addIncoming(Builder.getFalse(), notFoundBB);
            return phi;
        }

        if (FC->name == "weight")
        {
            auto *varArg = dynamic_cast<VariableNode *>(FC->arguments[0].get());
            if (!varArg)
                throw std::runtime_error("weight first argument must be a graph name");

            llvm::Value *graphPtr = loadGraphValue(varArg->name);
            llvm::Value *u = visitExpr(FC->arguments[1].get());
            llvm::Value *v = visitExpr(FC->arguments[2].get());
            auto *i32TyLocal = Builder.getInt32Ty();
            if (u->getType() != i32TyLocal)
                u = Builder.CreateIntCast(u, i32TyLocal, true);
            if (v->getType() != i32TyLocal)
                v = Builder.CreateIntCast(v, i32TyLocal, true);

            llvm::FunctionType *weightFT =
                llvm::FunctionType::get(i32TyLocal, {GraphTy->getPointerTo(), i32TyLocal, i32TyLocal}, false);
            auto weightFn = Module.getOrInsertFunction("graph_get_edge_weight", weightFT);
            return Builder.CreateCall(weightFn, {graphPtr, u, v}, "weighttmp");
        }

        if (FC->name == "edgeWeight" || FC->name == "edgeSrc" || FC->name == "edgeDst")
        {
            auto *varArg = dynamic_cast<VariableNode *>(FC->arguments[0].get());
            if (!varArg)
                throw std::runtime_error(FC->name + " first argument must be a graph name");

            llvm::Value *graphPtr = loadGraphValue(varArg->name);
            llvm::Value *eid = visitExpr(FC->arguments[1].get());
            auto *i32TyLocal = Builder.getInt32Ty();
            if (eid->getType() != i32TyLocal)
                eid = Builder.CreateIntCast(eid, i32TyLocal, true);

            const char *runtimeName = "graph_get_edge_weight_by_id";
            if (FC->name == "edgeSrc")
                runtimeName = "graph_get_edge_src_by_id";
            else if (FC->name == "edgeDst")
                runtimeName = "graph_get_edge_dst_by_id";

            llvm::FunctionType *edgeFT =
                llvm::FunctionType::get(i32TyLocal, {GraphTy->getPointerTo(), i32TyLocal}, false);
            auto edgeFn = Module.getOrInsertFunction(runtimeName, edgeFT);
            return Builder.CreateCall(edgeFn, {graphPtr, eid}, FC->name + ".tmp");
        }

        // Lookup the prototype
        llvm::Function *callee = FunctionProtos[FC->name];
        if (!callee)
            throw std::runtime_error("Unknown function: " + FC->name);

        // Evaluate arguments
        std::vector<llvm::Value *> args;
        for (auto &argNode : FC->arguments)
        {
            args.push_back(visitExpr(argNode.get()));
        }

        if (callee->getReturnType()->isVoidTy())
            return Builder.CreateCall(callee, args);
        return Builder.CreateCall(callee, args, "calltmp");
    }

    case ASTNodeType::ArrayAccess:
    {
        auto *A = static_cast<ArrayAccessNode *>(expr);
        auto *baseVar = dynamic_cast<VariableNode *>(A->arrayExpr.get());
        if (!baseVar)
            throw std::runtime_error("Array base must be variable");

        llvm::Value *baseAlloca = lookupNamedStorage(baseVar->name);

        llvm::Value *idxVal = visitExpr(A->indexExpr.get());
        if (idxVal->getType() != Builder.getInt32Ty())
            idxVal = Builder.CreateIntCast(idxVal, Builder.getInt32Ty(), true);

        if (IndirectRealArrays.count(baseVar->name))
        {
            auto *f64Ty = Builder.getDoubleTy();
            llvm::Value *dataPtr = Builder.CreateLoad(Builder.getPtrTy(), baseAlloca, baseVar->name + ".ptr");
            llvm::Value *elemPtr = Builder.CreateGEP(
                f64Ty, dataPtr, {idxVal},
                baseVar->name + "_elemptr");
            return Builder.CreateLoad(f64Ty, elemPtr, baseVar->name + "_loadelem");
        }

        if (IndirectArrays.count(baseVar->name))
        {
            llvm::Value *dataPtr = Builder.CreateLoad(Builder.getPtrTy(), baseAlloca, baseVar->name + ".ptr");
            llvm::Value *elemPtr = Builder.CreateGEP(
                Builder.getInt32Ty(), dataPtr, {idxVal},
                baseVar->name + "_elemptr");
            return Builder.CreateLoad(Builder.getInt32Ty(), elemPtr, baseVar->name + "_loadelem");
        }

        auto *arrTy = llvm::dyn_cast<llvm::ArrayType>(getStorageValueType(baseAlloca));
        if (arrTy)
        {
            llvm::Value *elemPtr = Builder.CreateGEP(
                arrTy,
                baseAlloca,
                {Builder.getInt32(0), idxVal},
                baseVar->name + "_elemptr");
            llvm::Type *elemTy = arrTy->getElementType();
            return Builder.CreateLoad(elemTy, elemPtr, baseVar->name + "_loadelem");
        }
        else
        {
            llvm::Type *baseTy = getStorageValueType(baseAlloca);

            if (baseTy->isIntegerTy(32))
            {
                llvm::Value *elemPtr = Builder.CreateGEP(
                    baseTy, baseAlloca, {idxVal},
                    baseVar->name + "_elemptr");
                return Builder.CreateLoad(baseTy, elemPtr, baseVar->name + "_loadelem");
            }

            if (auto *ptrTy = llvm::dyn_cast<llvm::PointerType>(baseTy))
            {
                llvm::Type *elemTy = ptrTy->getContainedType(0);
                llvm::Value *elemPtr = Builder.CreateGEP(
                    elemTy, baseAlloca, {idxVal},
                    baseVar->name + "_elemptr");
                return Builder.CreateLoad(elemTy, elemPtr, baseVar->name + "_loadelem");
            }

            throw std::runtime_error("IRGenVisitor: unsupported array storage type for load " + baseVar->name);
        }
    }

    case ASTNodeType::SetContainsExpr:
    {
        auto *sc = static_cast<SetContainsExprNode *>(expr);
        return visitSetContainsExpr(sc);
    }

    case ASTNodeType::SetPopExpr:
    {
        auto *sp = static_cast<SetPopExprNode *>(expr);
        return visitSetPopExpr(sp);
    }

    case ASTNodeType::NotExpr:
    {
        auto *ne = static_cast<NotExprNode *>(expr);
        llvm::Value *operand = visitExpr(ne->operand.get());
        return Builder.CreateNot(operand, "nottmp");
    }

    case ASTNodeType::UnaryMinusExpr:
    {
        auto *um = static_cast<UnaryMinusExprNode *>(expr);
        llvm::Value *operand = visitExpr(um->operand.get());
        if (operand->getType()->isDoubleTy())
            return Builder.CreateFNeg(operand, "negtmp");
        return Builder.CreateNeg(operand, "negtmp");
    }

    case ASTNodeType::Array2DAccess:
    {
        auto *a2d = static_cast<Array2DAccessNode *>(expr);
        auto *baseVar = dynamic_cast<VariableNode *>(a2d->arrayExpr.get());
        if (!baseVar)
            throw std::runtime_error("2D array base must be variable");

        auto metaIt = Array2DMap.find(baseVar->name);
        if (metaIt == Array2DMap.end())
            throw std::runtime_error("2D array metadata not found: " + baseVar->name);

        llvm::Value *rowIdx = visitExpr(a2d->rowExpr.get());
        llvm::Value *colIdx = visitExpr(a2d->colExpr.get());
        if (rowIdx->getType() != Builder.getInt32Ty())
            rowIdx = Builder.CreateIntCast(rowIdx, Builder.getInt32Ty(), true);
        if (colIdx->getType() != Builder.getInt32Ty())
            colIdx = Builder.CreateIntCast(colIdx, Builder.getInt32Ty(), true);

        llvm::Value *cols = metaIt->second.colsVal;
        llvm::Value *offset = Builder.CreateMul(rowIdx, cols, "row_offset");
        llvm::Value *flatIdx = Builder.CreateAdd(offset, colIdx, "flat_idx");

        llvm::Value *baseAlloca = lookupNamedStorage(baseVar->name);
        auto *i32Ty = Builder.getInt32Ty();
        llvm::Value *elemPtr = Builder.CreateGEP(i32Ty, baseAlloca, {flatIdx}, baseVar->name + "_2d_ptr");
        return Builder.CreateLoad(i32Ty, elemPtr, baseVar->name + "_2d_val");
    }

    case ASTNodeType::SetLiteral:
    {
        // Inline set literal used as expression — create a new bitmap
        auto *setLit = static_cast<SetLiteralNode *>(expr);
        auto *BitmapPtrTy = getBitmapPtrTy(Context);
        auto *i32Ty = Builder.getInt32Ty();
        auto *i64Ty = Builder.getInt64Ty();

        // Call roaring_bitmap_create
        llvm::FunctionType *createFT = llvm::FunctionType::get(
            BitmapPtrTy, {i64Ty, i64Ty}, false);
        auto createFn = Module.getOrInsertFunction("roaring_bitmap_create", createFT);
        llvm::Value *bm = Builder.CreateCall(createFn,
                                             {llvm::ConstantInt::get(i64Ty, 64 * 1024),
                                              llvm::ConstantInt::get(i64Ty, 8)},
                                             "set.lit");

        // Add each element
        llvm::FunctionType *addFT = llvm::FunctionType::get(
            Builder.getVoidTy(), {BitmapPtrTy, i32Ty}, false);
        auto addFn = Module.getOrInsertFunction("roaring_bitmap_add", addFT);
        for (auto &elem : setLit->elements)
        {
            llvm::Value *val = visitExpr(elem.get());
            if (val->getType() != i32Ty)
                val = Builder.CreateIntCast(val, i32Ty, true);
            Builder.CreateCall(addFn, {bm, val});
        }
        return bm;
    }

    default:
        throw std::runtime_error("Unsupported expression type in IRGenVisitor");
    }
}

// llvm::Value *IRGenVisitor::visitGraphDecl(GraphDeclNode *G)
// {
//     auto *I64 = llvm::Type::getInt64Ty(Context);
//     auto *I32 = llvm::Type::getInt32Ty(Context);
//     auto *arrRP = llvm::ArrayType::get(I64, G->row_ptr.size());
//     auto *arrCI = llvm::ArrayType::get(I32, G->col_idx.size());

//     llvm::SmallVector<llvm::Constant *, 16> rpConsts;
//     for (auto x : G->row_ptr)
//     {
//         rpConsts.push_back(llvm::ConstantInt::get(I64, x));
//     }
//     auto *RPArray = llvm::ConstantArray::get(arrRP, rpConsts);

//     llvm::SmallVector<llvm::Constant *, 16> ciConsts;
//     for (auto x : G->col_idx)
//     {
//         ciConsts.push_back(llvm::ConstantInt::get(I32, x));
//     }
//     auto *CIArray = llvm::ConstantArray::get(arrCI, ciConsts);

//     auto *GV_RP = new llvm::GlobalVariable(
//         Module, arrRP, true, llvm::GlobalValue::InternalLinkage,
//         RPArray, G->name + "_csr_row");

//     auto *GV_CI = new llvm::GlobalVariable(
//         Module, arrCI, true, llvm::GlobalValue::InternalLinkage,
//         CIArray, G->name + "_csr_col");

//     auto *mallocTy = llvm::FunctionType::get(
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
//         {llvm::Type::getInt64Ty(Context)},
//         false);

//     llvm::FunctionCallee mallocCalle = Module.getOrInsertFunction("malloc", mallocTy);
//     llvm::Function *mallocFn = llvm::cast<llvm::Function>(mallocCalle.getCallee());

//     uint64_t rowBytes = G->row_ptr.size() * sizeof(int64_t);
//     uint64_t colBytes = G->col_idx.size() * sizeof(int32_t);

//     llvm::Value *rpRaw = Builder.CreateCall(
//         mallocFn, llvm::ConstantInt::get(I64, rowBytes), "rp_raw");
//     llvm::Value *rowPtr = Builder.CreateBitCast(
//         rpRaw, llvm::PointerType::getUnqual(I64), "row_ptr");

//     llvm::Value *ciRaw = Builder.CreateCall(
//         mallocFn, llvm::ConstantInt::get(I64, colBytes), "ci_raw");
//     llvm::Value *colPtr = Builder.CreateBitCast(
//         ciRaw, llvm::PointerType::getUnqual(I32), "col_ptr");

//     llvm::Value *rp_i8 = Builder.CreateBitCast(rowPtr,
//                                                llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     llvm::Value *ci_i8 = Builder.CreateBitCast(colPtr,
//                                                llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     llvm::Value *src_rp = Builder.CreateBitCast(GV_RP,
//                                                 llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     llvm::Value *src_ci = Builder.CreateBitCast(GV_CI,
//                                                 llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));

//     // Define the memcpy intrinsic type
//     auto *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
//     auto *i64Ty = llvm::Type::getInt64Ty(Context);
//     auto *memcpyTy = llvm::Intrinsic::getDeclaration(
//         &Module, llvm::Intrinsic::memcpy,
//         {i8PtrTy, i8PtrTy, i64Ty});

//     // Define the volatility flag as i1
//     auto *volatileFlag = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 0);

//     // Perform the memcpy calls with correct argument types
//     Builder.CreateCall(memcpyTy, {rp_i8, src_rp, llvm::ConstantInt::get(I64, rowBytes), volatileFlag}, "");
//     Builder.CreateCall(memcpyTy, {ci_i8, src_ci, llvm::ConstantInt::get(I64, colBytes), volatileFlag}, "");

//     // Build the malloc prototype for allocating the Graph struct
//     llvm::FunctionType *mallocFT_graph = llvm::FunctionType::get(
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)), // returns i8*
//         {Builder.getInt64Ty()},                                       // takes an i64 size
//         false);

//     llvm::FunctionCallee mallocCalleeG =
//         Module.getOrInsertFunction("malloc", mallocFT_graph);
//     llvm::Function *mallocFnG =
//         llvm::cast<llvm::Function>(mallocCalleeG.getCallee());

//     // Compute sizeof(struct.Graph)
//     uint64_t graphSize = Module.getDataLayout().getTypeAllocSize(GraphTy);
//     llvm::Value *graphSizeC =
//         llvm::ConstantInt::get(Builder.getInt64Ty(), graphSize);

//     // Call malloc(graphSize)
//     llvm::Value *rawGraph = Builder.CreateCall(
//         mallocFnG,
//         graphSizeC,
//         "graph_raw");

//     // Cast i8* → %struct.Graph*
//     llvm::Value *graphPtr = Builder.CreateBitCast(
//         rawGraph,
//         GraphTy->getPointerTo(),
//         "graph_ptr");

//     // GEP + store into field 0: n
//     {
//         // element 0 is the vertex count 'n'
//         llvm::Value *nPtr = Builder.CreateStructGEP(
//             GraphTy, graphPtr, 0, "g_n_ptr");
//         Builder.CreateStore(
//             llvm::ConstantInt::get(Builder.getInt64Ty(), G->n),
//             nPtr);
//     }

//     // GEP + store into field 1: m
//     {
//         llvm::Value *mPtr = Builder.CreateStructGEP(
//             GraphTy, graphPtr, 1, "g_m_ptr");
//         Builder.CreateStore(
//             llvm::ConstantInt::get(Builder.getInt64Ty(), G->m),
//             mPtr);
//     }

//     // GEP + store into field 2: row_ptr
//     {
//         llvm::Value *rpPtr = Builder.CreateStructGEP(
//             GraphTy, graphPtr, 2, "g_rp_ptr");
//         Builder.CreateStore(rowPtr, rpPtr);
//     }

//     // GEP + store into field 3: col_idx
//     {
//         llvm::Value *ciPtr = Builder.CreateStructGEP(
//             GraphTy, graphPtr, 3, "g_ci_ptr");
//         Builder.CreateStore(colPtr, ciPtr);
//     }

//     GraphMap[G->name] = graphPtr;

//     return graphPtr;
// }

// llvm::Value *zero64(llvm::IRBuilder<> &B, llvm::LLVMContext &C)
// {
//     return llvm::ConstantInt::get(llvm::Type::getInt64Ty(C), 0);
// }

// llvm::Value *IRGenVisitor::visitGraphDecl(GraphDeclNode *G) {
//     auto *I64 = llvm::Type::getInt64Ty(Context);
//     auto *I32 = llvm::Type::getInt32Ty(Context);

//     // ================= TIMING SETUP =================
//     // struct timespec { i64 tv_sec; i64 tv_nsec; }
//     auto *timeSpecTy = llvm::StructType::create(
//         Context, {I64, I64}, "struct.timespec");

//     // int clock_gettime(int, struct timespec*)
//     auto *clockGettimeTy = llvm::FunctionType::get(
//         llvm::Type::getInt32Ty(Context),
//         {llvm::Type::getInt32Ty(Context),
//          llvm::PointerType::getUnqual(timeSpecTy)},
//         false);
//     llvm::FunctionCallee clockGettimeFn =
//         Module.getOrInsertFunction("clock_gettime", clockGettimeTy);

//     // CLOCK_MONOTONIC = 1
//     auto *clockMonotonicConst = llvm::ConstantInt::get(
//         llvm::Type::getInt32Ty(Context), 1);

//     // Allocas for start/end times
//     llvm::Value *tStart = Builder.CreateAlloca(timeSpecTy, nullptr, "tStart");
//     llvm::Value *tEnd = Builder.CreateAlloca(timeSpecTy, nullptr, "tEnd");

//     // Call clock_gettime(CLOCK_MONOTONIC, &tStart)
//     Builder.CreateCall(clockGettimeFn, {clockMonotonicConst, tStart});
//     // =================================================

//     // ---- your existing graph construction code ----
//     auto *arrRP = llvm::ArrayType::get(I64, G->row_ptr.size());
//     auto *arrCI = llvm::ArrayType::get(I32, G->col_idx.size());

//     llvm::SmallVector<llvm::Constant *, 16> rpConsts;
//     for (auto x : G->row_ptr) {
//         rpConsts.push_back(llvm::ConstantInt::get(I64, x));
//     }
//     auto *RPArray = llvm::ConstantArray::get(arrRP, rpConsts);

//     llvm::SmallVector<llvm::Constant *, 16> ciConsts;
//     for (auto x : G->col_idx) {
//         ciConsts.push_back(llvm::ConstantInt::get(I32, x));
//     }
//     auto *CIArray = llvm::ConstantArray::get(arrCI, ciConsts);

//     auto *GV_RP = new llvm::GlobalVariable(
//         Module, arrRP, true, llvm::GlobalValue::InternalLinkage,
//         RPArray, G->name + "_csr_row");

//     auto *GV_CI = new llvm::GlobalVariable(
//         Module, arrCI, true, llvm::GlobalValue::InternalLinkage,
//         CIArray, G->name + "_csr_col");

//     auto *mallocTy = llvm::FunctionType::get(
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
//         {llvm::Type::getInt64Ty(Context)},
//         false);

//     llvm::FunctionCallee mallocCalle = Module.getOrInsertFunction("malloc", mallocTy);
//     llvm::Function *mallocFn = llvm::cast<llvm::Function>(mallocCalle.getCallee());

//     uint64_t rowBytes = G->row_ptr.size() * sizeof(int64_t);
//     uint64_t colBytes = G->col_idx.size() * sizeof(int32_t);

//     llvm::Value *rpRaw = Builder.CreateCall(
//         mallocFn, llvm::ConstantInt::get(I64, rowBytes), "rp_raw");
//     llvm::Value *rowPtr = Builder.CreateBitCast(
//         rpRaw, llvm::PointerType::getUnqual(I64), "row_ptr");

//     llvm::Value *ciRaw = Builder.CreateCall(
//         mallocFn, llvm::ConstantInt::get(I64, colBytes), "ci_raw");
//     llvm::Value *colPtr = Builder.CreateBitCast(
//         ciRaw, llvm::PointerType::getUnqual(I32), "col_ptr");

//     llvm::Value *rp_i8 = Builder.CreateBitCast(rowPtr,
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     llvm::Value *ci_i8 = Builder.CreateBitCast(colPtr,
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     llvm::Value *src_rp = Builder.CreateBitCast(GV_RP,
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     llvm::Value *src_ci = Builder.CreateBitCast(GV_CI,
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));

//     // memcpy intrinsic
//     auto *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
//     auto *i64Ty = llvm::Type::getInt64Ty(Context);
//     auto *memcpyTy = llvm::Intrinsic::getDeclaration(
//         &Module, llvm::Intrinsic::memcpy,
//         {i8PtrTy, i8PtrTy, i64Ty});

//     auto *volatileFlag = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 0);

//     Builder.CreateCall(memcpyTy, {rp_i8, src_rp, llvm::ConstantInt::get(I64, rowBytes), volatileFlag});
//     Builder.CreateCall(memcpyTy, {ci_i8, src_ci, llvm::ConstantInt::get(I64, colBytes), volatileFlag});

//     llvm::FunctionType *mallocFT_graph = llvm::FunctionType::get(
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
//         {Builder.getInt64Ty()}, false);

//     llvm::FunctionCallee mallocCalleeG =
//         Module.getOrInsertFunction("malloc", mallocFT_graph);
//     llvm::Function *mallocFnG =
//         llvm::cast<llvm::Function>(mallocCalleeG.getCallee());

//     uint64_t graphSize = Module.getDataLayout().getTypeAllocSize(GraphTy);
//     llvm::Value *graphSizeC =
//         llvm::ConstantInt::get(Builder.getInt64Ty(), graphSize);

//     llvm::Value *rawGraph = Builder.CreateCall(
//         mallocFnG, graphSizeC, "graph_raw");

//     llvm::Value *graphPtr = Builder.CreateBitCast(
//         rawGraph, GraphTy->getPointerTo(), "graph_ptr");

//     { // store n
//         llvm::Value *nPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
//         Builder.CreateStore(llvm::ConstantInt::get(Builder.getInt64Ty(), G->n), nPtr);
//     }

//     { // store m
//         llvm::Value *mPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 1, "g_m_ptr");
//         Builder.CreateStore(llvm::ConstantInt::get(Builder.getInt64Ty(), G->m), mPtr);
//     }

//     { // store row_ptr
//         llvm::Value *rpPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 2, "g_rp_ptr");
//         Builder.CreateStore(rowPtr, rpPtr);
//     }

//     { // store col_idx
//         llvm::Value *ciPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 3, "g_ci_ptr");
//         Builder.CreateStore(colPtr, ciPtr);
//     }

//     GraphMap[G->name] = graphPtr;

//     // ================= TIMING END =================
//     // Call clock_gettime(CLOCK_MONOTONIC, &tEnd)
//     Builder.CreateCall(clockGettimeFn, {clockMonotonicConst, tEnd});

//     // Load start/end
//     llvm::Value *startSec = Builder.CreateLoad(I64, Builder.CreateStructGEP(timeSpecTy, tStart, 0), "start_sec");
//     llvm::Value *startNSec = Builder.CreateLoad(I64, Builder.CreateStructGEP(timeSpecTy, tStart, 1), "start_nsec");
//     llvm::Value *endSec = Builder.CreateLoad(I64, Builder.CreateStructGEP(timeSpecTy, tEnd, 0), "end_sec");
//     llvm::Value *endNSec = Builder.CreateLoad(I64, Builder.CreateStructGEP(timeSpecTy, tEnd, 1), "end_nsec");

//     llvm::Value *secDiff = Builder.CreateSub(endSec, startSec, "secDiff");
//     llvm::Value *nsecDiff = Builder.CreateSub(endNSec, startNSec, "nsecDiff");
//     llvm::Value *secToNS = Builder.CreateMul(secDiff, llvm::ConstantInt::get(I64, 1000000000LL), "secToNS");
//     llvm::Value *elapsedNS = Builder.CreateAdd(secToNS, nsecDiff, "elapsedNS");

//     // printf("Elapsed: %lld ns\n", elapsedNS);
//     auto *printfTy = llvm::FunctionType::get(
//         llvm::Type::getInt32Ty(Context),
//         llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
//         true);
//     llvm::FunctionCallee printfFn =
//         Module.getOrInsertFunction("printf", printfTy);

//     llvm::Value *fmtStr = Builder.CreateGlobalStringPtr("Elapsed: %lld ms\n");
//     llvm::Value *elapsedMS = Builder.CreateSDiv(elapsedNS, llvm::ConstantInt::get(I64, 1000000LL), "elapsedMS");
//     Builder.CreateCall(printfFn, {fmtStr, elapsedMS});
//     // =================================================

//     return graphPtr;
// }

llvm::Value *IRGenVisitor::visitGraphDecl(GraphDeclNode *G)
{
    auto *I64 = llvm::Type::getInt64Ty(Context);
    auto *I32 = llvm::Type::getInt32Ty(Context);
    auto *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
    auto *i64Ty = llvm::Type::getInt64Ty(Context);
    auto *graphPtrTy = llvm::PointerType::get(GraphTy, 0);

    // ── File-based graph: load at runtime ──────────────────────────────
    if (G->isFileGraph)
    {
        llvm::Value *fnameStr = Builder.CreateGlobalStringPtr(
            G->edgeFileName, G->name + "_filename");

        llvm::FunctionType *loadFT = llvm::FunctionType::get(
            graphPtrTy, {i8PtrTy}, false);
        auto loadFn = Module.getOrInsertFunction("load_graph_from_file", loadFT);
        llvm::Value *graphPtr = Builder.CreateCall(loadFn, {fnameStr}, "graph_ptr");

        llvm::Value *graphStorage = nullptr;
        if (EmittingTopLevel)
        {
            graphStorage = new llvm::GlobalVariable(
                Module,
                graphPtrTy,
                false,
                llvm::GlobalValue::InternalLinkage,
                llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(graphPtrTy)),
                G->name);
        }
        else
        {
            graphStorage = createEntryBlockAlloca(Builder.GetInsertBlock()->getParent(), G->name, graphPtrTy);
        }
        Builder.CreateStore(graphPtr, graphStorage);
        GraphMap[G->name] = graphStorage;
        NamedValues[G->name] = graphStorage;
        GraphAstMap[G->name] = G;

        // G.nodes bitmap: call graph_get_node_bitmap(graphPtr)
        llvm::FunctionType *getBmFT = llvm::FunctionType::get(
            i8PtrTy, {graphPtrTy}, false);
        auto getNodeBmFn = Module.getOrInsertFunction("graph_get_node_bitmap", getBmFT);
        GraphNodesMap[G->name] = Builder.CreateCall(getNodeBmFn, {graphPtr},
                                                    G->name + "_nodes_bm");

        // G.edges bitmap: call graph_get_edge_bitmap(graphPtr)
        auto getEdgeBmFn = Module.getOrInsertFunction("graph_get_edge_bitmap", getBmFT);
        GraphEdgesMap[G->name] = Builder.CreateCall(getEdgeBmFn, {graphPtr},
                                                    G->name + "_edges_bm");

        // Edge pairs for print: call graph_get_edge_pairs / graph_get_num_edge_pairs
        auto *i32PtrTy = llvm::PointerType::getUnqual(I32);
        llvm::FunctionType *getPairsFT = llvm::FunctionType::get(
            i32PtrTy, {graphPtrTy}, false);
        auto getPairsFn = Module.getOrInsertFunction("graph_get_edge_pairs", getPairsFT);
        RuntimeEdgePairsPtr = Builder.CreateCall(getPairsFn, {graphPtr}, "edge_pairs_ptr");

        llvm::FunctionType *getCountFT = llvm::FunctionType::get(
            i64Ty, {graphPtrTy}, false);
        auto getCountFn = Module.getOrInsertFunction("graph_get_num_edge_pairs", getCountFT);
        RuntimeEdgePairsCount = Builder.CreateCall(getCountFn, {graphPtr}, "edge_pairs_count");

        // Emit autograph_init to register graph with autotuner runtime
        {
            auto *voidTy = llvm::Type::getVoidTy(Context);
            auto *opaquePtrTy = llvm::PointerType::get(Context, 0);
            llvm::Value *nValInit = Builder.CreateLoad(I64, Builder.CreateStructGEP(GraphTy, graphPtr, 0), "g.n_init");
            llvm::Value *mValInit = Builder.CreateLoad(I64, Builder.CreateStructGEP(GraphTy, graphPtr, 1), "g.m_init");
            std::optional<FileGraphEstimate> fileEstimate =
                estimateUnweightedGraphFile(SourceDir, G->edgeFileName);
            if (fileEstimate)
            {
                nValInit = llvm::ConstantInt::get(I64, static_cast<uint64_t>(fileEstimate->n));
                mValInit = llvm::ConstantInt::get(I64, static_cast<uint64_t>(2 * fileEstimate->logical_m));
                G->n = static_cast<size_t>(fileEstimate->n);
                G->m = static_cast<size_t>(2 * fileEstimate->logical_m);
                // llvm::errs() << "[IRGen] estimated file graph '" << G->edgeFileName
                //              << "': n=" << fileEstimate->n
                //              << " logical_m=" << fileEstimate->logical_m
                //              << " csr_m=" << (2 * fileEstimate->logical_m) << "\n";
            }
            llvm::FunctionType *initFT = llvm::FunctionType::get(
                voidTy, {opaquePtrTy, I64, I64, opaquePtrTy, opaquePtrTy, opaquePtrTy}, false);
            auto initFn = Module.getOrInsertFunction("autograph_init", initFT);
            auto *initCall = Builder.CreateCall(initFn, {graphPtr, nValInit, mValInit,
                                                         GraphNodesMap[G->name], GraphEdgesMap[G->name],
                                                         RuntimeEdgePairsPtr});
            if (fileEstimate)
            {
                llvm::Metadata *sizeOps[] = {
                    llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(I64, fileEstimate->n)),
                    llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(I64, fileEstimate->logical_m))};
                initCall->setMetadata("autotuner.graph_size",
                                      llvm::MDNode::get(Context, sizeOps));
            }
        }

        return graphPtr;
    }

    // ── Inline graph: embed CSR as constants (original path) ───────────
    unsigned rpLen = static_cast<unsigned>(G->n + 1);
    auto *arrRP = llvm::ArrayType::get(I64, rpLen);
    llvm::SmallVector<llvm::Constant *, 16> rpConsts;
    rpConsts.reserve(rpLen);
    for (size_t i = 0; i < rpLen; i++)
    {
        rpConsts.push_back(llvm::ConstantInt::get(I64, (uint64_t)G->row_ptr[i]));
    }
    auto *RPArray = llvm::ConstantArray::get(arrRP, rpConsts);

    unsigned ciLen = static_cast<unsigned>(G->m);
    auto *arrCI = llvm::ArrayType::get(I32, ciLen);
    llvm::SmallVector<llvm::Constant *, 16> ciConsts;
    ciConsts.reserve(ciLen);
    for (size_t i = 0; i < ciLen; i++)
    {
        ciConsts.push_back(llvm::ConstantInt::get(I32, (int32_t)G->col_idx[i]));
    }
    auto *CIArray = llvm::ConstantArray::get(arrCI, ciConsts);

    auto *GV_RP = new llvm::GlobalVariable(
        Module, arrRP, true, llvm::GlobalValue::InternalLinkage,
        RPArray, G->name + "_csr_row");

    auto *GV_CI = new llvm::GlobalVariable(
        Module, arrCI, true, llvm::GlobalValue::InternalLinkage,
        CIArray, G->name + "_csr_col");

    auto *mallocTy = llvm::FunctionType::get(i8PtrTy, {I64}, false);
    llvm::FunctionCallee mallocCalle = Module.getOrInsertFunction("malloc", mallocTy);
    llvm::Function *mallocFn = llvm::cast<llvm::Function>(mallocCalle.getCallee());

    uint64_t rowBytes = (rpLen) * sizeof(int64_t);
    uint64_t colBytes = ciLen * sizeof(int32_t);

    llvm::Value *rpRaw = Builder.CreateCall(
        mallocFn, llvm::ConstantInt::get(I64, rowBytes), "rp_raw");
    llvm::Value *rowPtr = Builder.CreateBitCast(
        rpRaw, llvm::PointerType::getUnqual(I64), "row_ptr");

    llvm::Value *ciRaw = Builder.CreateCall(
        mallocFn, llvm::ConstantInt::get(I64, colBytes), "ci_raw");
    llvm::Value *colPtr = Builder.CreateBitCast(
        ciRaw, llvm::PointerType::getUnqual(I32), "col_ptr");

    llvm::Value *rp_i8 = Builder.CreateBitCast(rowPtr, i8PtrTy);
    llvm::Value *ci_i8 = Builder.CreateBitCast(colPtr, i8PtrTy);
    llvm::Value *src_rp = Builder.CreateBitCast(GV_RP, i8PtrTy);
    llvm::Value *src_ci = Builder.CreateBitCast(GV_CI, i8PtrTy);

    auto *memcpyFn = llvm::Intrinsic::getOrInsertDeclaration(
        &Module, llvm::Intrinsic::memcpy, {i8PtrTy, i8PtrTy, i64Ty});
    auto *volatileFlag = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 0);

    Builder.CreateCall(memcpyFn, {rp_i8, src_rp, llvm::ConstantInt::get(I64, rowBytes), volatileFlag}, "");
    Builder.CreateCall(memcpyFn, {ci_i8, src_ci, llvm::ConstantInt::get(I64, colBytes), volatileFlag}, "");

    llvm::FunctionType *mallocFT_graph = llvm::FunctionType::get(i8PtrTy, {I64}, false);
    llvm::FunctionCallee mallocCalleeG = Module.getOrInsertFunction("malloc", mallocFT_graph);
    llvm::Function *mallocFnG = llvm::cast<llvm::Function>(mallocCalleeG.getCallee());

    uint64_t graphSize = Module.getDataLayout().getTypeAllocSize(GraphTy);
    llvm::Value *rawGraph = Builder.CreateCall(
        mallocFnG, llvm::ConstantInt::get(I64, graphSize), "graph_raw");
    llvm::Value *graphPtr = Builder.CreateBitCast(rawGraph, graphPtrTy, "graph_ptr");

    Builder.CreateStore(llvm::ConstantInt::get(I64, G->n),
                        Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr"));
    Builder.CreateStore(llvm::ConstantInt::get(I64, G->m),
                        Builder.CreateStructGEP(GraphTy, graphPtr, 1, "g_m_ptr"));
    Builder.CreateStore(rowPtr,
                        Builder.CreateStructGEP(GraphTy, graphPtr, 2, "g_rp_ptr"));
    Builder.CreateStore(colPtr,
                        Builder.CreateStructGEP(GraphTy, graphPtr, 3, "g_ci_ptr"));

    llvm::Value *graphStorage = nullptr;
    if (EmittingTopLevel)
    {
        graphStorage = new llvm::GlobalVariable(
            Module,
            graphPtrTy,
            false,
            llvm::GlobalValue::InternalLinkage,
            llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(graphPtrTy)),
            G->name);
    }
    else
    {
        graphStorage = createEntryBlockAlloca(Builder.GetInsertBlock()->getParent(), G->name, graphPtrTy);
    }
    Builder.CreateStore(graphPtr, graphStorage);
    GraphMap[G->name] = graphStorage;
    NamedValues[G->name] = graphStorage;
    GraphAstMap[G->name] = G;

    auto *i8Ty = llvm::Type::getInt8Ty(Context);
    llvm::FunctionType *deserializeFT =
        llvm::FunctionType::get(i8PtrTy, {i8PtrTy, i64Ty}, false);
    auto deserializeFn =
        Module.getOrInsertFunction("roaring_from_serialized", deserializeFT);

    auto emitBitmapFromBlob = [&](const std::vector<uint8_t> &blob,
                                  const std::string &name) -> llvm::Value *
    {
        llvm::ArrayType *blobTy = llvm::ArrayType::get(i8Ty, blob.size());
        llvm::SmallVector<llvm::Constant *, 128> bytes;
        bytes.reserve(blob.size());
        for (uint8_t b : blob)
            bytes.push_back(llvm::ConstantInt::get(i8Ty, b));

        llvm::Constant *blobConst = llvm::ConstantArray::get(blobTy, bytes);
        auto *blobGV = new llvm::GlobalVariable(
            Module, blobTy, true, llvm::GlobalValue::PrivateLinkage, blobConst, name);

        llvm::Value *zero = Builder.getInt32(0);
        llvm::Value *blobPtr = Builder.CreateInBoundsGEP(blobTy, blobGV, {zero, zero}, name + ".ptr");
        llvm::Value *sizeVal = llvm::ConstantInt::get(i64Ty, blob.size());
        return Builder.CreateCall(deserializeFn, {blobPtr, sizeVal}, name + ".bitmap");
    };

    auto edgesBlob = buildEdgeBlobForGraph(G);
    GraphNodesMap[G->name] = emitBitmapFromBlob(G->nodes_blob, G->name + "_nodes_blob");
    GraphEdgesMap[G->name] = emitBitmapFromBlob(edgesBlob, G->name + "_edges_blob");

    // Register CSR metadata and emit autograph_init for autotuner runtime
    {
        auto *voidTy = llvm::Type::getVoidTy(Context);
        auto *opaquePtrTy = llvm::PointerType::get(Context, 0);
        llvm::FunctionType *regFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, false);
        auto regFn = Module.getOrInsertFunction("graph_register_csr_metadata", regFT);
        Builder.CreateCall(regFn, {graphPtr});

        auto *i32PtrTy = llvm::PointerType::getUnqual(I32);
        llvm::FunctionType *getPairsFT = llvm::FunctionType::get(i32PtrTy, {graphPtrTy}, false);
        auto getPairsFn = Module.getOrInsertFunction("graph_get_edge_pairs", getPairsFT);
        llvm::Value *edgePairsInit = Builder.CreateCall(getPairsFn, {graphPtr}, "edge_pairs_init");

        llvm::FunctionType *initFT = llvm::FunctionType::get(
            voidTy, {opaquePtrTy, I64, I64, opaquePtrTy, opaquePtrTy, opaquePtrTy}, false);
        auto initFn = Module.getOrInsertFunction("autograph_init", initFT);
        auto *initCall = Builder.CreateCall(initFn, {graphPtr,
                                    llvm::ConstantInt::get(I64, G->n),
                                    llvm::ConstantInt::get(I64, G->m),
                                    GraphNodesMap[G->name], GraphEdgesMap[G->name],
                                    edgePairsInit});

        }

    return graphPtr;
}
llvm::Value *zero64(llvm::IRBuilder<> &B, llvm::LLVMContext &C)
{
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(C), 0);
}

llvm::Value *IRGenVisitor::visitWeightedGraphDecl(WeightedGraphDeclNode *G)
{
    auto *I64 = llvm::Type::getInt64Ty(Context);
    auto *I32 = llvm::Type::getInt32Ty(Context);
    auto *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
    auto *i64Ty = llvm::Type::getInt64Ty(Context);
    auto *graphPtrTy = llvm::PointerType::get(GraphTy, 0);

    if (G->isFileGraph)
    {
        llvm::Value *fnameStr = Builder.CreateGlobalStringPtr(
            G->edgeFileName, G->name + "_filename");

        llvm::FunctionType *loadFT = llvm::FunctionType::get(
            graphPtrTy, {i8PtrTy}, false);
        auto loadFn = Module.getOrInsertFunction("load_weighted_graph_from_file", loadFT);
        llvm::Value *graphPtr = Builder.CreateCall(loadFn, {fnameStr}, "weighted_graph_ptr");

        llvm::Value *graphStorage = nullptr;
        if (EmittingTopLevel)
        {
            graphStorage = new llvm::GlobalVariable(
                Module,
                graphPtrTy,
                false,
                llvm::GlobalValue::InternalLinkage,
                llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(graphPtrTy)),
                G->name);
        }
        else
        {
            graphStorage = createEntryBlockAlloca(Builder.GetInsertBlock()->getParent(), G->name, graphPtrTy);
        }
        Builder.CreateStore(graphPtr, graphStorage);
        GraphMap[G->name] = graphStorage;
        NamedValues[G->name] = graphStorage;

        llvm::FunctionType *getBmFT = llvm::FunctionType::get(i8PtrTy, {graphPtrTy}, false);
        auto getNodeBmFn = Module.getOrInsertFunction("graph_get_node_bitmap", getBmFT);
        auto getEdgeBmFn = Module.getOrInsertFunction("graph_get_edge_bitmap", getBmFT);
        GraphNodesMap[G->name] = Builder.CreateCall(getNodeBmFn, {graphPtr}, G->name + "_nodes_bm");
        GraphEdgesMap[G->name] = Builder.CreateCall(getEdgeBmFn, {graphPtr}, G->name + "_edges_bm");

        auto *i32PtrTy = llvm::PointerType::getUnqual(I32);
        llvm::FunctionType *getPairsFT = llvm::FunctionType::get(i32PtrTy, {graphPtrTy}, false);
        auto getPairsFn = Module.getOrInsertFunction("graph_get_edge_pairs", getPairsFT);
        RuntimeEdgePairsPtr = Builder.CreateCall(getPairsFn, {graphPtr}, "edge_pairs_ptr");

        llvm::FunctionType *getCountFT = llvm::FunctionType::get(i64Ty, {graphPtrTy}, false);
        auto getCountFn = Module.getOrInsertFunction("graph_get_num_edge_pairs", getCountFT);
        RuntimeEdgePairsCount = Builder.CreateCall(getCountFn, {graphPtr}, "edge_pairs_count");

        // Emit autograph_init to register graph with autotuner runtime
        {
            auto *voidTy = llvm::Type::getVoidTy(Context);
            auto *opaquePtrTy = llvm::PointerType::get(Context, 0);
            llvm::Value *nValInit = Builder.CreateLoad(I64, Builder.CreateStructGEP(GraphTy, graphPtr, 0), "g.n_init");
            llvm::Value *mValInit = Builder.CreateLoad(I64, Builder.CreateStructGEP(GraphTy, graphPtr, 1), "g.m_init");
            llvm::FunctionType *initFT = llvm::FunctionType::get(
                voidTy, {opaquePtrTy, I64, I64, opaquePtrTy, opaquePtrTy, opaquePtrTy}, false);
            auto initFn = Module.getOrInsertFunction("autograph_init", initFT);
            Builder.CreateCall(initFn, {graphPtr, nValInit, mValInit,
                                        GraphNodesMap[G->name], GraphEdgesMap[G->name],
                                        RuntimeEdgePairsPtr});
        }

        return graphPtr;
    }

    unsigned rpLen = static_cast<unsigned>(G->n + 1);
    auto *arrRP = llvm::ArrayType::get(I64, rpLen);
    llvm::SmallVector<llvm::Constant *, 16> rpConsts;
    rpConsts.reserve(rpLen);
    for (size_t i = 0; i < rpLen; i++)
    {
        rpConsts.push_back(llvm::ConstantInt::get(I64, (uint64_t)G->row_ptr[i]));
    }
    auto *RPArray = llvm::ConstantArray::get(arrRP, rpConsts);

    unsigned ciLen = static_cast<unsigned>(G->m);
    auto *arrCI = llvm::ArrayType::get(I32, ciLen);
    auto *arrW = llvm::ArrayType::get(I32, ciLen);
    llvm::SmallVector<llvm::Constant *, 16> ciConsts;
    llvm::SmallVector<llvm::Constant *, 16> wConsts;
    ciConsts.reserve(ciLen);
    wConsts.reserve(ciLen);
    for (size_t i = 0; i < ciLen; i++)
    {
        ciConsts.push_back(llvm::ConstantInt::get(I32, (int32_t)G->col_idx[i]));
        wConsts.push_back(llvm::ConstantInt::get(I32, (int32_t)G->weights[i]));
    }
    auto *CIArray = llvm::ConstantArray::get(arrCI, ciConsts);
    auto *WArray = llvm::ConstantArray::get(arrW, wConsts);
    auto *GV_RP = new llvm::GlobalVariable(
        Module, arrRP, true, llvm::GlobalValue::InternalLinkage,
        RPArray, G->name + "_csr_row");

    auto *GV_CI = new llvm::GlobalVariable(
        Module, arrCI, true, llvm::GlobalValue::InternalLinkage,
        CIArray, G->name + "_csr_col");

    auto *GV_W = new llvm::GlobalVariable(
        Module, arrW, true, llvm::GlobalValue::InternalLinkage,
        WArray, G->name + "_w_col");

    auto *mallocTy = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
        {llvm::Type::getInt64Ty(Context)},
        false);

    llvm::FunctionCallee mallocCalle = Module.getOrInsertFunction("malloc", mallocTy);
    llvm::Function *mallocFn = llvm::cast<llvm::Function>(mallocCalle.getCallee());

    uint64_t rowBytes = (rpLen) * sizeof(int64_t);
    uint64_t colBytes = ciLen * sizeof(int32_t);
    uint64_t wBytes = ciLen * sizeof(int32_t);

    llvm::Value *rpRaw = Builder.CreateCall(
        mallocFn, llvm::ConstantInt::get(I64, rowBytes), "rp_raw");
    llvm::Value *rowPtr = Builder.CreateBitCast(
        rpRaw, llvm::PointerType::getUnqual(I64), "row_ptr");

    llvm::Value *ciRaw = Builder.CreateCall(
        mallocFn, llvm::ConstantInt::get(I64, colBytes), "ci_raw");
    llvm::Value *colPtr = Builder.CreateBitCast(
        ciRaw, llvm::PointerType::getUnqual(I32), "col_ptr");

    llvm::Value *wRaw = Builder.CreateCall(
        mallocFn, llvm::ConstantInt::get(I64, wBytes), "w_raw");
    llvm::Value *wPtr = Builder.CreateBitCast(
        wRaw, llvm::PointerType::getUnqual(I32), "w_ptr");

    llvm::Value *rp_i8 = Builder.CreateBitCast(rowPtr,
                                               llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *ci_i8 = Builder.CreateBitCast(colPtr,
                                               llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *w_i8 = Builder.CreateBitCast(wPtr,
                                              llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *src_rp = Builder.CreateBitCast(GV_RP,
                                                llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *src_ci = Builder.CreateBitCast(GV_CI,
                                                llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *src_w = Builder.CreateBitCast(GV_W,
                                               llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));

    // Define the memcpy intrinsic type
    auto *memcpyTy = llvm::Intrinsic::getDeclaration(
        &Module, llvm::Intrinsic::memcpy,
        {i8PtrTy, i8PtrTy, i64Ty});

    // Define the volatility flag as i1
    auto *volatileFlag = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 0);

    // Perform the memcpy calls with correct argument types
    Builder.CreateCall(memcpyTy, {rp_i8, src_rp, llvm::ConstantInt::get(I64, rowBytes), volatileFlag}, "");
    Builder.CreateCall(memcpyTy, {ci_i8, src_ci, llvm::ConstantInt::get(I64, colBytes), volatileFlag}, "");
    Builder.CreateCall(memcpyTy, {w_i8, src_w, llvm::ConstantInt::get(I64, wBytes), volatileFlag}, "");

    // Build the malloc prototype for allocating the Graph struct
    llvm::FunctionType *mallocFT_graph = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)), // returns i8*
        {Builder.getInt64Ty()},                                       // takes an i64 size
        false);

    llvm::FunctionCallee mallocCalleeG =
        Module.getOrInsertFunction("malloc", mallocFT_graph);
    llvm::Function *mallocFnG =
        llvm::cast<llvm::Function>(mallocCalleeG.getCallee());

    // Compute sizeof(struct.Graph)
    uint64_t graphSize = Module.getDataLayout().getTypeAllocSize(GraphTy);
    llvm::Value *graphSizeC =
        llvm::ConstantInt::get(Builder.getInt64Ty(), graphSize);

    // Call malloc(graphSize)
    llvm::Value *rawGraph = Builder.CreateCall(
        mallocFnG,
        graphSizeC,
        "graph_raw");

    // Cast i8* → %struct.Graph*
    llvm::Value *graphPtr = Builder.CreateBitCast(
        rawGraph,
        GraphTy->getPointerTo(),
        "graph_ptr");

    // GEP + store into field 0: n
    {
        // element 0 is the vertex count 'n'
        llvm::Value *nPtr = Builder.CreateStructGEP(
            GraphTy, graphPtr, 0, "g_n_ptr");
        Builder.CreateStore(
            llvm::ConstantInt::get(Builder.getInt64Ty(), G->n),
            nPtr);
    }

    // GEP + store into field 1: m
    {
        llvm::Value *mPtr = Builder.CreateStructGEP(
            GraphTy, graphPtr, 1, "g_m_ptr");
        Builder.CreateStore(
            llvm::ConstantInt::get(Builder.getInt64Ty(), G->m),
            mPtr);
    }

    // GEP + store into field 2: row_ptr
    {
        llvm::Value *rpPtr = Builder.CreateStructGEP(
            GraphTy, graphPtr, 2, "g_rp_ptr");
        Builder.CreateStore(rowPtr, rpPtr);
    }

    // GEP + store into field 3: col_idx
    {
        llvm::Value *ciPtr = Builder.CreateStructGEP(
            GraphTy, graphPtr, 3, "g_ci_ptr");
        Builder.CreateStore(colPtr, ciPtr);
    }

    // GEP + store into field 4: weights
    {
        llvm::Value *wtPtr = Builder.CreateStructGEP(
            GraphTy, graphPtr, 4, "g_w_ptr");
        Builder.CreateStore(wPtr, wtPtr);
    }

    llvm::Value *graphStorage = nullptr;
    if (EmittingTopLevel)
    {
        graphStorage = new llvm::GlobalVariable(
            Module,
            GraphTy->getPointerTo(),
            false,
            llvm::GlobalValue::InternalLinkage,
            llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(GraphTy->getPointerTo())),
            G->name);
    }
    else
    {
        llvm::Function *F = Builder.GetInsertBlock()->getParent();
        graphStorage = createEntryBlockAlloca(F, G->name, GraphTy->getPointerTo());
    }
    Builder.CreateStore(graphPtr, graphStorage);
    GraphMap[G->name] = graphStorage;
    NamedValues[G->name] = graphStorage;
    // debug
    // llvm::errs() << "[visitWeightedGraphDecl] G->n (declared) = " << G->n << "\n";
    // llvm::errs() << "[visitWeightedGraphDecl] G->row_ptr entries = " << (G->n + 1) << "\n";
    if (G->n > 0)
        // llvm::errs() << "[visitWeightedGraphDecl] row_ptr[n] = " << G->row_ptr[G->n] << "\n";
    // llvm::errs() << "[visitWeightedGraphDecl] G->col_idx entries = " << G->m << "\n";

    // Emit roaring bitmaps for nodes and edges
    {
        auto *i8Ty = llvm::Type::getInt8Ty(Context);
        auto *bitmapPtrTy = llvm::PointerType::get(Context, 0);
        auto *i64BitmapTy = Builder.getInt64Ty();

        llvm::FunctionType *deserializeFT_w =
            llvm::FunctionType::get(bitmapPtrTy, {bitmapPtrTy, i64BitmapTy}, false);
        auto deserializeFn_w =
            Module.getOrInsertFunction("roaring_from_serialized", deserializeFT_w);

        auto emitBitmapFromBlob = [&](const std::vector<uint8_t> &blob,
                                      const std::string &name) -> llvm::Value *
        {
            llvm::ArrayType *blobTy = llvm::ArrayType::get(i8Ty, blob.size());
            llvm::SmallVector<llvm::Constant *, 128> bytes;
            bytes.reserve(blob.size());
            for (uint8_t b : blob)
                bytes.push_back(llvm::ConstantInt::get(i8Ty, b));

            llvm::Constant *blobConst = llvm::ConstantArray::get(blobTy, bytes);
            auto *blobGV = new llvm::GlobalVariable(
                Module, blobTy, true, llvm::GlobalValue::PrivateLinkage, blobConst, name);

            llvm::Value *zero = Builder.getInt32(0);
            llvm::Value *blobPtr = Builder.CreateInBoundsGEP(blobTy, blobGV, {zero, zero}, name + ".ptr");
            llvm::Value *sizeVal = llvm::ConstantInt::get(i64BitmapTy, blob.size());
            return Builder.CreateCall(deserializeFn_w, {blobPtr, sizeVal}, name + ".bitmap");
        };

        GraphNodesMap[G->name] = emitBitmapFromBlob(G->nodes_blob, G->name + "_nodes_blob");
        GraphEdgesMap[G->name] = emitBitmapFromBlob(G->edges_blob, G->name + "_edges_blob");
    }

    // Register CSR metadata and emit autograph_init for autotuner runtime
    {
        auto *voidTy = llvm::Type::getVoidTy(Context);
        auto *opaquePtrTy = llvm::PointerType::get(Context, 0);
        auto *graphPtrTyW = GraphTy->getPointerTo();
        llvm::FunctionType *regFT = llvm::FunctionType::get(voidTy, {graphPtrTyW}, false);
        auto regFn = Module.getOrInsertFunction("graph_register_csr_metadata", regFT);
        Builder.CreateCall(regFn, {graphPtr});

        auto *i32PtrTyW = llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(Context));
        llvm::FunctionType *getPairsFT = llvm::FunctionType::get(i32PtrTyW, {graphPtrTyW}, false);
        auto getPairsFn = Module.getOrInsertFunction("graph_get_edge_pairs", getPairsFT);
        llvm::Value *edgePairsInit = Builder.CreateCall(getPairsFn, {graphPtr}, "edge_pairs_init");

        llvm::FunctionType *initFT = llvm::FunctionType::get(
            voidTy, {opaquePtrTy, llvm::Type::getInt64Ty(Context), llvm::Type::getInt64Ty(Context),
                     opaquePtrTy, opaquePtrTy, opaquePtrTy}, false);
        auto initFn = Module.getOrInsertFunction("autograph_init", initFT);
        auto *initCallW = Builder.CreateCall(initFn, {graphPtr,
                                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), G->n),
                                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), G->m),
                                    GraphNodesMap[G->name], GraphEdgesMap[G->name],
                                    edgePairsInit});
    }

    return graphPtr;
}

// void IRGenVisitor::emitBFS(QueryNode *Q)
// {
//     // Grab the Graph* value
//     llvm::Value *graphPtr = loadGraphValue(Q->graphName);
//     assert(graphPtr && "Graph not found in IRGenVisitor::emitBFS");

//     // Declare the runtime function:
//     // extern "C" void bfs_runtime(struct.Graph* g);
//     llvm::Type *voidTy = llvm::Type::getVoidTy(Context);
//     llvm::Type *graphPtrTy = GraphTy->getPointerTo();
//     llvm::FunctionType *bfsFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, /*isVarArg=*/false);

//     // getOrInsertFunction will add a declaration if not present.
//     auto bfsDecl = Module.getOrInsertFunction("bfs_single", bfsFT);

//     // Call bfs_single(graphPtr)
//     Builder.CreateCall(bfsDecl, {graphPtr});
//     // done
//     return;
// }

// void IRGenVisitor::emitDFS(QueryNode *Q)
// {
//     // Grab the Graph* value
//     llvm::Value *graphPtr = loadGraphValue(Q->graphName);
//     assert(graphPtr && "Graph not found in IRGenVisitor::emitDFS");

//     // Declare the runtime function:
//     // extern "C" void bfs_runtime(struct.Graph* g);
//     llvm::Type *voidTy = llvm::Type::getVoidTy(Context);
//     llvm::Type *graphPtrTy = GraphTy->getPointerTo();
//     llvm::FunctionType *dfsFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, /*isVarArg=*/false);

//     // getOrInsertFunction will add a declaration if not present.
//     auto dfsDecl = Module.getOrInsertFunction("dfs_runtime", dfsFT);

//     // Call dfs_runtime(graphPtr)
//     Builder.CreateCall(dfsDecl, {graphPtr});

//     // done
//     return;
// }

void IRGenVisitor::emitBFS(QueryNode *Q)
{
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitBFS");

    // types
    llvm::Type *int32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *int32PtrTy = int32Ty->getPointerTo();
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();

    // BFS runtime: void bfs_runtime(Graph* g, int** out_array, int* out_size);
    llvm::FunctionType *bfsFT =
        llvm::FunctionType::get(int32Ty, {graphPtrTy, int32PtrTy->getPointerTo(), int32Ty->getPointerTo()}, false);
    auto bfsDecl = Module.getOrInsertFunction("bfs_runtime", bfsFT);

    // Allocate space for array pointer and size
    llvm::AllocaInst *arrAlloca = Builder.CreateAlloca(int32PtrTy, nullptr, Q->queryName + "_arr");
    llvm::AllocaInst *sizeAlloca = Builder.CreateAlloca(int32Ty, nullptr, Q->queryName + "_size");

    // Call BFS runtime
    llvm::Value *retVal = Builder.CreateCall(
        bfsDecl,
        {graphPtr, arrAlloca, sizeAlloca},
        Q->queryName + "_bfs_ret");

    // Store the alloca (pointer) and size in NamedValues
    NamedValues[Q->queryName + "_ptr"] = arrAlloca;
    NamedValues[Q->queryName + "_size"] = sizeAlloca;
}

void IRGenVisitor::emitDFS(QueryNode *Q)
{
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitDFS");

    llvm::Type *int32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *int32PtrTy = int32Ty->getPointerTo();
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();

    // Declare: extern "C" int dfs_runtime(Graph* g, int** out_order);
    llvm::FunctionType *dfsFT = llvm::FunctionType::get(
        int32Ty, {graphPtrTy, int32PtrTy->getPointerTo()}, false);

    auto dfsDecl = Module.getOrInsertFunction("dfs_runtime", dfsFT);

    // Allocate space for int* out_order
    llvm::Value *dfsArrayPtr = Builder.CreateAlloca(int32PtrTy, nullptr, Q->queryName + "_arr");

    // Call dfs_runtime(graphPtr, &dfsArrayPtr)
    llvm::Value *sizeVal = Builder.CreateCall(dfsDecl, {graphPtr, dfsArrayPtr}, Q->queryName + "_size");

    // Store array pointer and size in NamedValues
    NamedValues[Q->queryName + "_ptr"] = llvm::cast<llvm::AllocaInst>(dfsArrayPtr);
    // Allocate an alloca for the size and store the value
    llvm::AllocaInst *sizeAlloca = Builder.CreateAlloca(int32Ty, nullptr, Q->queryName + "_size");
    Builder.CreateStore(sizeVal, sizeAlloca);
    NamedValues[Q->queryName + "_size"] = sizeAlloca;
}

llvm::Function *IRGenVisitor::getPrintfFunction()
{
    llvm::FunctionType *printfType =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(Context),
            llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
            true);
    auto func = Module.getOrInsertFunction("printf", printfType);
    return llvm::cast<llvm::Function>(func.getCallee());
}

void IRGenVisitor::visitPrintArray(PrintArrayNode *node)
{
    auto arrIt = NamedValues.find(node->arrayName + "_ptr");
    auto sizeIt = NamedValues.find(node->arrayName + "_size");

    if (arrIt == NamedValues.end() || sizeIt == NamedValues.end())
    {
        llvm::errs() << "Array not found: " << node->arrayName << "\n";
        return;
    }

    llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *i32PtrTy = i32Ty->getPointerTo();

    // array pointer and size
    llvm::Value *arrPtr = Builder.CreateLoad(i32PtrTy, arrIt->second, node->arrayName + "_ptr_ld");
    llvm::Value *arrSize = Builder.CreateLoad(i32Ty, sizeIt->second, node->arrayName + "_size_ld");

    llvm::Function *printfFunc = getPrintfFunction();

    // index alloca in entry block for robustness
    llvm::Function *F = Builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
    llvm::AllocaInst *indexAlloca = tmpB.CreateAlloca(i32Ty, nullptr, node->arrayName + "_idx");
    Builder.CreateStore(llvm::ConstantInt::get(i32Ty, 0), indexAlloca);

    // Blocks
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(Context, "print_array.cond", F);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(Context, "print_array.body", F);
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(Context, "print_array.after", F);

    // jump to cond
    Builder.CreateBr(condBB);

    // cond: i < size ?
    Builder.SetInsertPoint(condBB);
    llvm::Value *iVal = Builder.CreateLoad(i32Ty, indexAlloca, node->arrayName + "_i");
    llvm::Value *cond = Builder.CreateICmpSLT(iVal, arrSize, node->arrayName + "_cond");
    Builder.CreateCondBr(cond, bodyBB, afterBB);

    // body: print element, i++
    Builder.SetInsertPoint(bodyBB);
    llvm::Value *elemPtr = Builder.CreateGEP(i32Ty, arrPtr, iVal, node->arrayName + "_elemptr");
    llvm::Value *elem = Builder.CreateLoad(i32Ty, elemPtr, node->arrayName + "_elem");
    llvm::Value *fmtElem = Builder.CreateGlobalStringPtr("%d ");
    Builder.CreateCall(printfFunc, {fmtElem, elem});

    llvm::Value *nextI = Builder.CreateAdd(iVal, llvm::ConstantInt::get(i32Ty, 1), node->arrayName + "_i_next");
    Builder.CreateStore(nextI, indexAlloca);
    Builder.CreateBr(condBB);

    // after: newline
    Builder.SetInsertPoint(afterBB);
    Builder.CreateCall(printfFunc, {Builder.CreateGlobalStringPtr("\n")});
}

void IRGenVisitor::emitFloydWarshall(QueryNode *Q)
{
    // Grab the Graph* value
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitFloydWarshall");

    // Declare the runtime function:
    // extern "C" void bfs_runtime(struct.Graph* g);
    llvm::Type *voidTy = llvm::Type::getVoidTy(Context);
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();
    llvm::FunctionType *fwFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, /*isVarArg=*/false);

    // getOrInsertFunction will add a declaration if not present.
    auto fwDecl = Module.getOrInsertFunction("floyd_runtime", fwFT);

    // Call floyd_runtime(graphPtr)
    Builder.CreateCall(fwDecl, {graphPtr});

    // done
    return;
}

void IRGenVisitor::emitBK(QueryNode *Q)
{
    // Grab the Graph* value
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitBK");

    // Declare the runtime function:
    // extern "C" void bfs_runtime(struct.Graph* g);
    llvm::Type *voidTy = llvm::Type::getVoidTy(Context);
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();
    llvm::FunctionType *BKFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, /*isVarArg=*/false);

    // getOrInsertFunction will add a declaration if not present.
    auto bkDecl = Module.getOrInsertFunction("bk_runtime", BKFT);

    // Call bk_runtime(graphPtr)
    Builder.CreateCall(bkDecl, {graphPtr});

    // done
    return;
}

void IRGenVisitor::emitDijkstra(QueryNode *Q)
{
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitDijkstra");

    llvm::Type *int32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *int32PtrTy = int32Ty->getPointerTo();       // int32_t*
    llvm::Type *int32PtrPtrTy = int32PtrTy->getPointerTo(); // int32_t**
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();

    // int dijkstra_runtime_src(Graph* g, int32_t src, int32_t** out_array, int32_t* out_size)
    llvm::FunctionType *djFT =
        llvm::FunctionType::get(int32Ty,
                                {graphPtrTy, int32Ty, int32PtrPtrTy, int32Ty->getPointerTo()},
                                false);

    auto djDecl = Module.getOrInsertFunction("dijkstra_runtime_src", djFT);

    llvm::AllocaInst *arrAlloca = Builder.CreateAlloca(int32PtrTy, nullptr, Q->queryName + "_arr");
    llvm::AllocaInst *sizeAlloca = Builder.CreateAlloca(int32Ty, nullptr, Q->queryName + "_size");

    int srcInt = Q->source.has_value() ? Q->source.value() : 0; // default source = 0
    llvm::Value *srcConst = llvm::ConstantInt::get(int32Ty, srcInt, /*isSigned=*/true);

    llvm::Value *retVal = Builder.CreateCall(
        djDecl,
        {graphPtr, srcConst, arrAlloca, sizeAlloca},
        Q->queryName + "_dijkstra_ret");

    // Expose to print routines: print k; will detect k_ptr/k_size and print the array
    NamedValues[Q->queryName + "_ptr"] = arrAlloca;
    NamedValues[Q->queryName + "_size"] = sizeAlloca;

    (void)retVal;
}

// void IRGenVisitor::emitChromacity(QueryNode *Q)
// {
//     // Grab the Graph* value
//     llvm::Value *graphPtr = loadGraphValue(Q->graphName);
//     assert(graphPtr && "Graph not found in IRGenVisitor::emitChromacity");

//     // Declare the runtime function:
//     // extern "C" void chromacity_runtime(struct.Graph* g);
//     llvm::Type *voidTy = llvm::Type::getVoidTy(Context);
//     llvm::Type *graphPtrTy = GraphTy->getPointerTo();
//     llvm::FunctionType *chromacityFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, /*isVarArg=*/false);

//     // getOrInsertFunction will add a declaration if not present.
//     auto chromacityDecl = Module.getOrInsertFunction("chromacity_runtime", chromacityFT);

//     // Call chromacity_runtime(graphPtr)
//     Builder.CreateCall(chromacityDecl, {graphPtr});

//     // done
//     return;
// }

void IRGenVisitor::emitChromacity(QueryNode *Q)
{
    // 1. Grab the Graph* value
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitChromacity");

    // 2. Declare the runtime function:
    // extern "C" int chromacity_runtime(struct Graph* g);
    llvm::Type *intTy = llvm::Type::getInt32Ty(Context);
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();
    llvm::FunctionType *chromFT = llvm::FunctionType::get(intTy, {graphPtrTy}, false);

    auto chromDecl = Module.getOrInsertFunction("chromacity_runtime", chromFT);

    // 3. Call the function and get return value
    llvm::Value *retVal = Builder.CreateCall(chromDecl, {graphPtr}, "chromacity_result");

    // 4. Allocate space for query result in NamedValues if not already present
    llvm::Value *varPtr = nullptr;
    auto it = NamedValues.find(Q->queryName);
    if (it == NamedValues.end())
    {
        varPtr = Builder.CreateAlloca(intTy, nullptr, Q->queryName);
        NamedValues[Q->queryName] = varPtr;
    }
    else
    {
        varPtr = it->second;
    }

    // 5. Store the returned value into the allocated variable
    Builder.CreateStore(retVal, varPtr);

    // Done
}

void IRGenVisitor::emitMinCut(QueryNode *Q)
{
    // 1. Grab the Graph* value
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitMinCut");

    // 2. Declare the runtime function:
    // extern "C" int min_cut_runtime(struct Graph* g);
    llvm::Type *intTy = llvm::Type::getInt32Ty(Context);
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();
    llvm::FunctionType *minCutFT = llvm::FunctionType::get(intTy, {graphPtrTy}, false);

    auto minCutDecl = Module.getOrInsertFunction("karger_runtime", minCutFT);

    // 3. Call the function and get return value
    llvm::Value *retVal = Builder.CreateCall(minCutDecl, {graphPtr}, "min_cut_result");

    // 4. Allocate space for query result in NamedValues if not already present
    llvm::Value *varPtr = nullptr;
    auto it = NamedValues.find(Q->queryName);
    if (it == NamedValues.end())
    {
        varPtr = Builder.CreateAlloca(intTy, nullptr, Q->queryName);
        NamedValues[Q->queryName] = varPtr;
    }
    else
    {
        varPtr = it->second;
    }

    // 5. Store the returned value into the allocated variable
    Builder.CreateStore(retVal, varPtr);

    // Done
}

// ...existing code...
void IRGenVisitor::emitDFSSrc(QueryNode *Q)
{
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    assert(graphPtr && "Graph not found in IRGenVisitor::emitDFSSrc");

    llvm::Type *int32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *int32PtrTy = int32Ty->getPointerTo();       // int32_t*
    llvm::Type *int32PtrPtrTy = int32PtrTy->getPointerTo(); // int32_t**
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();

    // int dfs_runtime_src(Graph* g, int32_t src, int32_t** out_array, int32_t* out_size)
    llvm::FunctionType *dfsFT_with_src =
        llvm::FunctionType::get(int32Ty,
                                {graphPtrTy, int32Ty, int32PtrPtrTy, int32Ty->getPointerTo()},
                                false);

    auto dfsWithSrcDecl = Module.getOrInsertFunction("dfs_runtime_src", dfsFT_with_src);

    llvm::AllocaInst *arrAlloca = Builder.CreateAlloca(int32PtrTy, nullptr, Q->queryName + "_arr");
    llvm::AllocaInst *sizeAlloca = Builder.CreateAlloca(int32Ty, nullptr, Q->queryName + "_size");

    int srcInt = Q->source.has_value() ? Q->source.value() : -1;
    llvm::Value *srcConst = llvm::ConstantInt::get(int32Ty, srcInt, true);

    llvm::Value *retVal = Builder.CreateCall(
        dfsWithSrcDecl,
        {graphPtr, srcConst, arrAlloca, sizeAlloca},
        Q->queryName + "_dfs_src_ret");

    // Make available to print routines
    NamedValues[Q->queryName + "_ptr"] = arrAlloca;
    NamedValues[Q->queryName + "_size"] = sizeAlloca;

    (void)retVal; // ignore status for now
}

void IRGenVisitor::emitBFSSrc(QueryNode *Q)
{
    llvm::Value *graphPtr = loadGraphValue(Q->graphName);
    // llvm::errs() << "Unsupported query type: " << Q->queryDesc;
    assert(graphPtr && "Graph not found in IRGenVisitor::emitBFSSrc");
    // llvm::errs() << "Unsupported query type: " << Q->queryDesc;
    // llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n\n";
    // types
    llvm::Type *int32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *int32PtrTy = int32Ty->getPointerTo();       // int32_t*
    llvm::Type *int32PtrPtrTy = int32PtrTy->getPointerTo(); // int32_t**
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();

    // int bfs_runtime_src_alloc(Graph* g, int32_t src, int32_t** out_array, int32_t* out_size)
    llvm::FunctionType *bfsFT_with_src =
        llvm::FunctionType::get(int32Ty,
                                {graphPtrTy, int32Ty, int32PtrPtrTy, int32Ty->getPointerTo()},
                                false);

    auto bfsWithSrcDecl = Module.getOrInsertFunction("bfs_runtime_src", bfsFT_with_src);

    llvm::AllocaInst *arrAlloca = Builder.CreateAlloca(int32PtrTy, nullptr, Q->queryName + "_arr");
    llvm::AllocaInst *sizeAlloca = Builder.CreateAlloca(int32Ty, nullptr, Q->queryName + "_size");

    int srcInt = Q->source.has_value() ? Q->source.value() : -1;
    llvm::Value *srcConst = llvm::ConstantInt::get(int32Ty, srcInt, /*isSigned=*/true);

    llvm::Value *retVal = Builder.CreateCall(
        bfsWithSrcDecl,
        {graphPtr, srcConst, arrAlloca, sizeAlloca},
        Q->queryName + "_bfs_src_ret");

    NamedValues[Q->queryName + "_ptr"] = arrAlloca;
    NamedValues[Q->queryName + "_size"] = sizeAlloca;
    // llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n\n";
}

void IRGenVisitor::visitQuery(QueryNode *Q)
{
    // Assuming QueryNode has a std::string field named 'queryDesc' for "bfs"/"dfs"
    if (Q->queryDesc == "bfs")
    {
        emitBFS(Q);
    }
    else if (Q->queryDesc == "dfs")
    {
        emitDFS(Q);
    }
    else if (Q->queryDesc == "bk")
    {
        emitBK(Q);
    }
    else if (Q->queryDesc == "transitive_closure")
    {
        emitFloydWarshall(Q);
    }
    else if (Q->queryDesc == "chromaticity")
    {
        emitChromacity(Q);
    }
    else if (Q->queryDesc == "dijkstra")
    {

        emitDijkstra(Q);
    }
    else if (Q->queryDesc == "bfs_src")
    {
        // llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n\n";
        emitBFSSrc(Q);
    }
    else if (Q->queryDesc == "min_cut")
    {
        emitMinCut(Q);
    }
    else if (Q->queryDesc == "dfs_src")
    {
        // llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n\n";
        emitDFSSrc(Q);
    }
    else
    {
        llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n";
        assert(false && "Unknown query type in QueryNode");
    }
}

void IRGenVisitor::visitSleepStmt(SleepStmtNode *SS)
{
    // Evaluate the duration expression
    llvm::Value *durationVal = visitExpr(SS->duration.get());

    // Ensure it's an integer (i32)
    if (!durationVal->getType()->isIntegerTy())
    {
        if (durationVal->getType()->isDoubleTy())
            durationVal = Builder.CreateFPToSI(durationVal, Builder.getInt32Ty());
        else
            throw std::runtime_error("sleep() requires integer argument");
    }
    else if (durationVal->getType() != Builder.getInt32Ty())
    {
        durationVal = Builder.CreateIntCast(durationVal, Builder.getInt32Ty(), true);
    }

    // Declare: void sleep_runtime(int32_t seconds);
    llvm::FunctionType *sleepFT = llvm::FunctionType::get(
        llvm::Type::getVoidTy(Context),
        {Builder.getInt32Ty()},
        false);
    llvm::FunctionCallee sleepDecl =
        Module.getOrInsertFunction("sleep_runtime", sleepFT);

    // Call sleep_runtime(duration)
    Builder.CreateCall(sleepDecl, {durationVal});
}

void IRGenVisitor::visitPrintStmt(PrintStmtNode *PS)
{
    // Ensure printf exists
    llvm::Function *printfFn = Module.getFunction("printf");
    if (!printfFn)
    {
        auto *i8p = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
        auto *printfTy = llvm::FunctionType::get(Builder.getInt32Ty(), {i8p}, true);
        printfFn = llvm::Function::Create(printfTy, llvm::Function::ExternalLinkage, "printf", &Module);
    }

    ASTNode *exprNode = PS->expr.get();

    // Handle string literal prints first
    if (auto *sln = dynamic_cast<StringLiteralNode *>(exprNode))
    {
        llvm::Value *fmt = Builder.CreateGlobalStringPtr("%s\n");
        llvm::Value *str = Builder.CreateGlobalStringPtr(sln->value);
        Builder.CreateCall(printfFn, {fmt, str});
        return;
    }

    // Handle variable-based prints (query arrays, sets, plain ints)
    if (exprNode->type == ASTNodeType::Variable)
    {
        auto *var = static_cast<VariableNode *>(exprNode);

        // 1) Check for query-array result first (e.g., k_ptr / k_size)
        auto ptrIt = NamedValues.find(var->name + "_ptr");
        auto sizeIt = NamedValues.find(var->name + "_size");
        if (ptrIt != NamedValues.end() && sizeIt != NamedValues.end())
        {
            PrintArrayNode pan(var->name, nullptr);
            visitPrintArray(&pan);
            return;
        }

        // 2) Must be a regular variable
        auto it = NamedValues.find(var->name);
        if (it == NamedValues.end())
        {
            throw std::runtime_error("Undefined variable in print: " + var->name);
        }

        llvm::Value *storage = it->second;
        llvm::Type *allocatedTy = getStorageValueType(storage);

        // 3) Set variable (allocated type is pointer → bitmap)
        if (llvm::isa<llvm::PointerType>(allocatedTy))
        {
            auto *BitmapPtrTy = getBitmapPtrTy(Context);
            llvm::Value *ptr = Builder.CreateLoad(BitmapPtrTy, storage, var->name + ".bitmap");

            auto kindIt = SetKinds.find(var->name);
            auto kind = (kindIt == SetKinds.end()) ? SetValueKind::Unknown : kindIt->second;

            if (kind == SetValueKind::Edges)
            {
                auto *i32Ty = Builder.getInt32Ty();
                auto *i64Ty = Builder.getInt64Ty();
                auto *i32PtrTy = llvm::PointerType::get(i32Ty, 0);

                llvm::Value *pairsPtr = nullptr;
                llvm::Value *countVal = nullptr;

                if (RuntimeEdgePairsPtr && RuntimeEdgePairsCount)
                {
                    pairsPtr = RuntimeEdgePairsPtr;
                    countVal = RuntimeEdgePairsCount;
                }
                else if (EdgePairsGV && EdgePairsCount > 0)
                {
                    llvm::Value *zero = Builder.getInt32(0);
                    pairsPtr = Builder.CreateInBoundsGEP(
                        EdgePairsGV->getValueType(),
                        EdgePairsGV,
                        {zero, zero},
                        "edge_pairs.ptr");
                    countVal = llvm::ConstantInt::get(i64Ty, EdgePairsCount);
                }

                if (pairsPtr && countVal)
                {
                    llvm::FunctionType *printEdgesFT = llvm::FunctionType::get(
                        Builder.getVoidTy(),
                        {BitmapPtrTy, i32PtrTy, i64Ty},
                        false);
                    auto printEdgesFn = Module.getOrInsertFunction("roaring_print_edges", printEdgesFT);
                    Builder.CreateCall(printEdgesFn, {ptr, pairsPtr, countVal});
                    return;
                }
            }

            llvm::FunctionType *printFT = llvm::FunctionType::get(
                Builder.getVoidTy(),
                {BitmapPtrTy},
                false);

            auto printFn = Module.getOrInsertFunction("roaring_print", printFT);
            Builder.CreateCall(printFn, {ptr});
            return;
        }

        // 4) Real (double) variable
        if (allocatedTy->isDoubleTy())
        {
            llvm::Value *val = Builder.CreateLoad(Builder.getDoubleTy(), storage, var->name);
            llvm::Value *strPtr = Builder.CreateGlobalStringPtr("%f\n");
            Builder.CreateCall(printfFn, {strPtr, val});
            return;
        }

        // 5) Plain integer variable
        llvm::Value *val = Builder.CreateLoad(Builder.getInt32Ty(), storage, var->name);
        llvm::Value *strPtr = Builder.CreateGlobalStringPtr("%d\n");
        Builder.CreateCall(printfFn, {strPtr, val});
        return;
    }

    // Generic expression printing: evaluate then print by type
    llvm::Value *val = visitExpr(PS->expr.get());
    llvm::Type *valTy = val->getType();

    if (valTy->isIntegerTy(1))
    {
        llvm::Value *val32 = Builder.CreateZExt(val, Builder.getInt32Ty(), "bool_to_i32");
        static llvm::GlobalVariable *fmtBool = nullptr;
        if (!fmtBool)
        {
            auto *fmtTy = llvm::ArrayType::get(Builder.getInt8Ty(), 4);
            fmtBool = new llvm::GlobalVariable(
                Module, fmtTy, /*isConstant=*/true,
                llvm::GlobalValue::PrivateLinkage,
                llvm::ConstantDataArray::getString(Context, "%d\n", true),
                ".fmt_bool");
        }
        llvm::Value *fmtPtr = Builder.CreateBitCast(fmtBool, llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
        Builder.CreateCall(printfFn, {fmtPtr, val32});
        return;
    }
    if (valTy->isDoubleTy())
    {
        static llvm::GlobalVariable *fmtReal = nullptr;
        if (!fmtReal)
        {
            auto *fmtTy = llvm::ArrayType::get(Builder.getInt8Ty(), 4);
            fmtReal = new llvm::GlobalVariable(
                Module, fmtTy, /*isConstant=*/true,
                llvm::GlobalValue::PrivateLinkage,
                llvm::ConstantDataArray::getString(Context, "%f\n", true),
                ".fmt_real");
        }
        llvm::Value *fmtPtr = Builder.CreateBitCast(fmtReal, llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
        Builder.CreateCall(printfFn, {fmtPtr, val});
        return;
    }
    if (valTy->isIntegerTy())
    {
        static llvm::GlobalVariable *fmtInt = nullptr;
        if (!fmtInt)
        {
            auto *fmtTy = llvm::ArrayType::get(Builder.getInt8Ty(), 4);
            fmtInt = new llvm::GlobalVariable(
                Module, fmtTy, /*isConstant=*/true,
                llvm::GlobalValue::PrivateLinkage,
                llvm::ConstantDataArray::getString(Context, "%d\n", true),
                ".fmt_int");
        }
        llvm::Value *fmtPtr = Builder.CreateBitCast(fmtInt, llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
        Builder.CreateCall(printfFn, {fmtPtr, val});
        return;
    }

    throw std::runtime_error("print: unsupported expression type");
}

void IRGenVisitor::visitSetDecl(SetDeclNode *setDecl)
{
    std::string baseName = setDecl->name;
    std::string blobName = baseName + "_blob";

    auto *i8Ty = llvm::Type::getInt8Ty(Context);
    auto *i8PtrTy = llvm::PointerType::get(Context, 0);
    auto *i64Ty = Builder.getInt64Ty();

    // Get current function for local allocation
    llvm::Function *fn = Builder.GetInsertBlock()->getParent();

    // Create LOCAL alloca for the bitmap pointer
    llvm::IRBuilder<> tmpB(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    llvm::AllocaInst *bitmapAlloca = tmpB.CreateAlloca(i8PtrTy, nullptr, baseName);
    NamedValues[baseName] = bitmapAlloca;
    SetKinds[setDecl->name] = inferSetKind(setDecl->initializer.get());

    // If no initializer at all (e.g., "set visited;"), create empty bitmap at runtime
    if (!setDecl->initializer)
    {
        llvm::FunctionType *createFT = llvm::FunctionType::get(
            i8PtrTy, {i64Ty, i64Ty}, false);
        auto createFn = Module.getOrInsertFunction("roaring_bitmap_create", createFT);
        llvm::Value *emptyBm = Builder.CreateCall(createFn,
                                                  {llvm::ConstantInt::get(i64Ty, 256),
                                                   llvm::ConstantInt::get(i64Ty, 8)},
                                                  baseName + ".empty");
        Builder.CreateStore(emptyBm, bitmapAlloca);
        return;
    }

    if (setDecl->initializer->type != ASTNodeType::SetLiteral)
    {
        llvm::Value *bitmapPtr = visitSetExpr(setDecl->initializer.get());
        Builder.CreateStore(bitmapPtr, bitmapAlloca);
        return;
    }

    // Check if initializer is a set expression (for cases like: set s3 = s1 union s2;)
    // if (setDecl->initializer && setDecl->initializer->type == ASTNodeType::SetBinaryExpr)
    // {
    //     // Evaluate the set expression at runtime
    //     llvm::Value *bitmapPtr = visitSetExpr(setDecl->initializer.get());

    //     // Store only into local alloca (no global!)
    //     Builder.CreateStore(bitmapPtr, bitmapAlloca);
    //     return;
    // }

    // Original logic for set literals
    RoaringBitmap *rb = roaring_bitmap_create(64 * 1024, 8);

    if (setDecl->initializer && setDecl->initializer->type == ASTNodeType::SetLiteral)
    {
        auto *lit = dynamic_cast<SetLiteralNode *>(setDecl->initializer.get());
        if (!lit)
            throw std::runtime_error("Set initializer must be a set literal");

        for (auto &e : lit->elements)
        {
            auto *intLit = dynamic_cast<IntLiteralNode *>(e.get());
            if (!intLit)
                throw std::runtime_error("Set literal elements must be integers");

            roaring_bitmap_add(rb, intLit->value);
        }
    }

    // SetKinds[setDecl->name] = inferSetKind(setDecl->initializer.get());

    size_t blobSize = roaring_bitmap_portable_size_in_bytes(rb);
    llvm::SmallVector<uint8_t> blob(blobSize);
    roaring_bitmap_portable_serialize(rb, blob.data());

    roaring_bitmap_free(rb);

    llvm::ArrayType *blobTy = llvm::ArrayType::get(i8Ty, blobSize);
    llvm::SmallVector<llvm::Constant *, 128> bytes;
    for (uint8_t b : blob)
        bytes.push_back(llvm::ConstantInt::get(i8Ty, b));

    llvm::Constant *blobConst = llvm::ConstantArray::get(blobTy, bytes);

    // Keep the blob as a private constant (this is fine - it's read-only data)
    llvm::GlobalVariable *blobGV =
        new llvm::GlobalVariable(
            Module,
            blobTy,
            /*isConstant=*/true,
            llvm::GlobalValue::PrivateLinkage,
            blobConst,
            blobName);

    llvm::Value *zero = Builder.getInt32(0);
    llvm::Value *blobPtr =
        Builder.CreateInBoundsGEP(blobTy, blobGV, {zero, zero}, blobName + ".ptr");

    llvm::FunctionType *deserializeFT =
        llvm::FunctionType::get(i8PtrTy, {i8PtrTy, i64Ty}, false);

    auto deserializeFn =
        Module.getOrInsertFunction("roaring_from_serialized", deserializeFT);

    llvm::Value *sizeVal =
        llvm::ConstantInt::get(i64Ty, blobSize);

    llvm::Value *bitmapPtr =
        Builder.CreateCall(deserializeFn, {blobPtr, sizeVal}, baseName + ".bitmap");

    // Store only into local alloca (no global variable for the bitmap pointer!)
    Builder.CreateStore(bitmapPtr, bitmapAlloca);
}

void IRGenVisitor::visitSetOperation(SetOperationNode *setOp)
{
    // Get the target set variable (must already exist)
    auto it = NamedValues.find(setOp->targetName);
    if (it == NamedValues.end())
    {
        throw std::runtime_error("IRGenVisitor: undefined set variable: " +
                                 setOp->targetName);
    }

    llvm::Value *targetAlloca = it->second;

    // Evaluate the set expression to get the resulting bitmap pointer
    llvm::Value *resultBitmap = visitSetExpr(setOp->expr.get());

    // Store the result ONLY into the local alloca (no global update!)
    Builder.CreateStore(resultBitmap, targetAlloca);
    SetKinds[setOp->targetName] = inferSetKind(setOp->expr.get());
}

llvm::Value *IRGenVisitor::visitSetExpr(ASTNode *expr)
{
    switch (expr->type)
    {
    case ASTNodeType::SetBinaryExpr:
    {
        return visitSetBinaryExpr(static_cast<SetBinaryExprNode *>(expr));
    }

    case ASTNodeType::GraphMemberSet:
    {
        auto *gm = static_cast<GraphMemberSetNode *>(expr);
        llvm::Value *graphPtr = loadGraphValue(gm->graphName);
        auto *voidTy = llvm::Type::getVoidTy(Context);
        auto *opaquePtrTy = llvm::PointerType::get(Context, 0);
        auto syncFT = llvm::FunctionType::get(voidTy, {opaquePtrTy}, false);
        auto syncFn = Module.getOrInsertFunction("autograph_sync_canonical_if_dirty", syncFT);
        Builder.CreateCall(syncFn, {graphPtr});
        if (gm->member == GraphMemberKind::Nodes)
            return GraphNodesMap.at(gm->graphName);
        return GraphEdgesMap.at(gm->graphName);
    }

    case ASTNodeType::Variable:
    {
        auto *varNode = static_cast<VariableNode *>(expr);
        auto it = NamedValues.find(varNode->name);
        if (it == NamedValues.end())
            throw std::runtime_error("Undefined set variable: " + varNode->name);

        auto *BitmapPtrTy = getBitmapPtrTy(Context);
        return Builder.CreateLoad(BitmapPtrTy, it->second, varNode->name + ".load");
    }

    case ASTNodeType::SetLiteral:
    {
        auto *lit = static_cast<SetLiteralNode *>(expr);

        auto *BitmapPtrTy = getBitmapPtrTy(Context);
        auto *i32Ty = Builder.getInt32Ty();
        auto *i64Ty = Builder.getInt64Ty();

        llvm::FunctionType *createFT = llvm::FunctionType::get(
            BitmapPtrTy,
            {i64Ty, i64Ty},
            false);

        auto createFn = Module.getOrInsertFunction("roaring_bitmap_create", createFT);

        llvm::Value *arenaSize = llvm::ConstantInt::get(i64Ty, 64 * 1024);
        llvm::Value *initialCap = llvm::ConstantInt::get(i64Ty, 8);
        llvm::Value *tempBitmap = Builder.CreateCall(
            createFn,
            {arenaSize, initialCap},
            "temp.bitmap");

        llvm::FunctionType *addFT = llvm::FunctionType::get(
            llvm::Type::getVoidTy(Context),
            {BitmapPtrTy, i32Ty},
            false);

        auto addFn = Module.getOrInsertFunction("roaring_bitmap_add", addFT);

        for (auto &e : lit->elements)
        {
            auto *intLit = dynamic_cast<IntLiteralNode *>(e.get());
            if (!intLit)
                throw std::runtime_error("Set literal elements must be integers");
            llvm::Value *value = llvm::ConstantInt::get(i32Ty, intLit->value);
            Builder.CreateCall(addFn, {tempBitmap, value});
        }

        return tempBitmap;
    }

    default:
        throw std::runtime_error("IRGenVisitor: unsupported set expression type");
    }
}

llvm::SmallVector<ASTNode *, 8> IRGenVisitor::flattenSetOperation(ASTNode *expr, const std::string &targetOp)
{
    llvm::SmallVector<ASTNode *, 8> operands;

    if (expr->type != ASTNodeType::SetBinaryExpr)
    {
        // Leaf node (Variable or SetLiteral)
        operands.push_back(expr);
        return operands;
    }

    auto *binExpr = static_cast<SetBinaryExprNode *>(expr);

    // Only flatten if this node has the same operation as targetOp
    if (binExpr->op != targetOp)
    {
        // Different operation - treat as atomic
        operands.push_back(expr);
        return operands;
    }

    // Same operation - recursively flatten both sides
    auto leftOps = flattenSetOperation(binExpr->lhs.get(), targetOp);
    auto rightOps = flattenSetOperation(binExpr->rhs.get(), targetOp);

    // Reserve space to avoid reallocations
    operands.reserve(leftOps.size() + rightOps.size());

    operands.insert(operands.end(), leftOps.begin(), leftOps.end());
    operands.insert(operands.end(), rightOps.begin(), rightOps.end());

    return operands;
}

llvm::Value *IRGenVisitor::visitSetBinaryExpr(SetBinaryExprNode *binExpr)
{
    auto *BitmapPtrTy = getBitmapPtrTy(Context);

    if (binExpr->op == "union")
    {
        llvm::SmallVector<ASTNode *, 8> operands = flattenSetOperation(binExpr, "union");

        // std::cerr << "[IRGen] Union expression with "
        //           << operands.size() << " operands\n";

        llvm::SmallVector<llvm::Value *, 8> bitmapPtrs;
        bitmapPtrs.reserve(operands.size());

        for (auto *op : operands)
        {
            bitmapPtrs.push_back(visitSetExpr(op));
        }

        auto *i64Ty = Builder.getInt64Ty();

        llvm::Value *arraySize = llvm::ConstantInt::get(Builder.getInt64Ty(), operands.size());
        llvm::AllocaInst *bitmapArray = Builder.CreateAlloca(
            BitmapPtrTy,
            arraySize,
            "bitmap_array");

        for (size_t i = 0; i < bitmapPtrs.size(); ++i)
        {
            llvm::Value *idx = llvm::ConstantInt::get(Builder.getInt32Ty(), i);
            llvm::Value *elemPtr = Builder.CreateGEP(
                BitmapPtrTy,
                bitmapArray,
                {idx},
                "array_elem_" + std::to_string(i));
            Builder.CreateStore(bitmapPtrs[i], elemPtr);
        }

        llvm::FunctionType *unionFT = llvm::FunctionType::get(
            BitmapPtrTy,
            {llvm::PointerType::get(BitmapPtrTy, 0), i64Ty},
            false);

        auto unionFn = Module.getOrInsertFunction("roaring_bitmap_union", unionFT);

        llvm::Value *count = llvm::ConstantInt::get(i64Ty, operands.size());
        llvm::Value *resultBitmap = Builder.CreateCall(
            unionFn,
            {bitmapArray, count},
            "set.union.result");

        llvm::FunctionType *freeFT = llvm::FunctionType::get(
            Builder.getVoidTy(),
            {BitmapPtrTy},
            false);
        auto freeFn = Module.getOrInsertFunction("roaring_bitmap_free", freeFT);

        for (size_t i = 0; i < bitmapPtrs.size(); ++i)
        {
            if (operands[i]->type != ASTNodeType::Variable && operands[i]->type != ASTNodeType::GraphMemberSet)
            {
                Builder.CreateCall(freeFn, {bitmapPtrs[i]});
            }
        }

        return resultBitmap;
    }
    else if (binExpr->op == "intersect")
    {
        llvm::Value *lhsBitmap = visitSetExpr(binExpr->lhs.get());
        llvm::Value *rhsBitmap = visitSetExpr(binExpr->rhs.get());

        llvm::FunctionType *intersectFT = llvm::FunctionType::get(
            BitmapPtrTy,
            {BitmapPtrTy, BitmapPtrTy},
            false);

        auto intersectFn = Module.getOrInsertFunction("roaring_bitmap_intersect", intersectFT);

        llvm::Value *resultBitmap = Builder.CreateCall(
            intersectFn,
            {lhsBitmap, rhsBitmap},
            "set.intersect.result");
        llvm::FunctionType *freeFT = llvm::FunctionType::get(
            Builder.getVoidTy(),
            {BitmapPtrTy},
            false);
        auto freeFn = Module.getOrInsertFunction("roaring_bitmap_free", freeFT);

        if (binExpr->lhs->type != ASTNodeType::Variable && binExpr->lhs->type != ASTNodeType::GraphMemberSet)
        {
            Builder.CreateCall(freeFn, {lhsBitmap});
        }

        if (binExpr->rhs->type != ASTNodeType::Variable && binExpr->rhs->type != ASTNodeType::GraphMemberSet)
        {
            Builder.CreateCall(freeFn, {rhsBitmap});
        }

        return resultBitmap;
    }
    else
    {
        throw std::runtime_error("Unsupported set operation: " + binExpr->op);
    }
}

void IRGenVisitor::visitSetMethodCall(SetMethodCallNode *node)
{
    auto *BitmapPtrTy = getBitmapPtrTy(Context);
    auto *i32Ty = Builder.getInt32Ty();
    auto *voidTy = Builder.getVoidTy();
    llvm::Value *bitmapPtr = nullptr;
    if (node->targetKind == SetTargetKind::Variable)
    {
        auto it = NamedValues.find(node->targetName);
        if (it == NamedValues.end())
            throw std::runtime_error("Undefined set variable: " + node->targetName);
        bitmapPtr = Builder.CreateLoad(BitmapPtrTy, it->second, node->targetName + ".load");
    }
    else if (node->targetKind == SetTargetKind::GraphNodes)
    {
        bitmapPtr = GraphNodesMap.at(node->targetName);
    }
    else
    {
        bitmapPtr = GraphEdgesMap.at(node->targetName);
    }

    // Get the set variable
    // auto it = NamedValues.find(node->setName);
    // if (it == NamedValues.end())
    // {
    //     throw std::runtime_error("Undefined set variable: " + node->setName);
    // }

    // llvm::AllocaInst *setAlloca = it->second;
    // llvm::Value *bitmapPtr = Builder.CreateLoad(BitmapPtrTy, setAlloca, node->setName + ".load");

    // Evaluate the argument
    llvm::Value *argValue = visitExpr(node->argument.get());
    if (argValue->getType() != i32Ty)
    {
        argValue = Builder.CreateIntCast(argValue, i32Ty, true);
    }

    if (node->methodName == "add")
    {
        llvm::FunctionType *addFT = llvm::FunctionType::get(
            voidTy, {BitmapPtrTy, i32Ty}, false);
        auto addFn = Module.getOrInsertFunction("roaring_bitmap_add", addFT);
        Builder.CreateCall(addFn, {bitmapPtr, argValue});
    }
    else if (node->methodName == "remove")
    {
        llvm::FunctionType *removeFT = llvm::FunctionType::get(
            voidTy, {BitmapPtrTy, i32Ty}, false);
        auto removeFn = Module.getOrInsertFunction("roaring_bitmap_remove", removeFT);
        Builder.CreateCall(removeFn, {bitmapPtr, argValue});
    }
}

llvm::Value *IRGenVisitor::visitSetContainsExpr(SetContainsExprNode *node)
{
    auto *BitmapPtrTy = getBitmapPtrTy(Context);
    auto *i32Ty = Builder.getInt32Ty();

    llvm::Value *bitmapPtr = nullptr;

    if (node->targetKind == SetTargetKind::Variable)
    {
        auto it = NamedValues.find(node->targetName);
        if (it == NamedValues.end())
        {
            throw std::runtime_error("Undefined set variable: " + node->targetName);
        }

        llvm::Value *setAlloca = it->second;
        bitmapPtr = Builder.CreateLoad(BitmapPtrTy, setAlloca, node->targetName + ".load");
    }
    else if (node->targetKind == SetTargetKind::GraphNodes)
    {
        bitmapPtr = GraphNodesMap.at(node->targetName);
    }
    else
    {
        bitmapPtr = GraphEdgesMap.at(node->targetName);
    }

    llvm::Value *argValue = visitExpr(node->argument.get());
    if (argValue->getType() != i32Ty)
    {
        argValue = Builder.CreateIntCast(argValue, i32Ty, true);
    }

    llvm::FunctionType *containsFT = llvm::FunctionType::get(
        i32Ty,
        {BitmapPtrTy, i32Ty},
        false);

    auto containsFn = Module.getOrInsertFunction("roaring_bitmap_contains", containsFT);
    llvm::Value *result = Builder.CreateCall(containsFn, {bitmapPtr, argValue}, "contains.result");

    return Builder.CreateICmpNE(result, llvm::ConstantInt::get(i32Ty, 0), "contains.bool");
}

llvm::Value *IRGenVisitor::visitSetPopExpr(SetPopExprNode *node)
{
    auto *BitmapPtrTy = getBitmapPtrTy(Context);
    auto *i32Ty = Builder.getInt32Ty();

    auto it = NamedValues.find(node->setName);
    if (it == NamedValues.end())
        throw std::runtime_error("Undefined set variable in pop: " + node->setName);

    llvm::Value *bitmapPtr = Builder.CreateLoad(BitmapPtrTy, it->second, node->setName + ".bm");

    llvm::FunctionType *popFT = llvm::FunctionType::get(i32Ty, {BitmapPtrTy}, false);
    auto popFn = Module.getOrInsertFunction("roaring_bitmap_pop", popFT);
    return Builder.CreateCall(popFn, {bitmapPtr}, "set.pop");
}

void IRGenVisitor::visitSwapStmt(SwapStmtNode *node)
{
    auto itA = NamedValues.find(node->name1);
    auto itB = NamedValues.find(node->name2);
    if (itA == NamedValues.end())
        throw std::runtime_error("swap: undefined array: " + node->name1);
    if (itB == NamedValues.end())
        throw std::runtime_error("swap: undefined array: " + node->name2);

    if (!IndirectArrays.count(node->name1) || !IndirectArrays.count(node->name2))
        throw std::runtime_error("swap() only works with dynamic arrays");

    auto *ptrTy = Builder.getPtrTy();
    llvm::Value *ptrA = Builder.CreateLoad(ptrTy, itA->second, node->name1 + ".swap");
    llvm::Value *ptrB = Builder.CreateLoad(ptrTy, itB->second, node->name2 + ".swap");
    Builder.CreateStore(ptrB, itA->second);
    Builder.CreateStore(ptrA, itB->second);
}
