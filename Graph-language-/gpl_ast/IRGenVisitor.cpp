#include "IRGenVisitor.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include <chrono>

void IRGenVisitor::visitProgram(ProgramNodePtr prog)
{

    for (auto &node : prog->topLevel)
    {
        if (node->type == ASTNodeType::FunctionDecl)
        {
            auto *FD = static_cast<FunctionDeclNode *>(node.get());

            // Collect parameter types (all i32 for now)
            std::vector<llvm::Type *> paramTys;
            for (size_t i = 0; i < FD->parameters.size(); ++i)
            {
                paramTys.push_back(Builder.getInt32Ty());
            }

            // Determine return type
            llvm::Type *retTy;
            if (FD->returnType == "void")
            {
                retTy = llvm::Type::getVoidTy(Context);
            }
            else
            {
                retTy = Builder.getInt32Ty();
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

    // Entry block
    auto *BB = llvm::BasicBlock::Create(Context, "entry", mainF);
    Builder.SetInsertPoint(BB);

    // Lower every top‑level AST node
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
        case ASTNodeType::WhileStmt:
            visitWhile(static_cast<WhileStmtNode *>(node.get()));
            break;
        case ASTNodeType::GraphDecl:
            // Lower graph h { … }
            visitGraphDecl(static_cast<GraphDeclNode *>(node.get()));
            break;
        case ASTNodeType::QueryNode:
            visitQuery(static_cast<QueryNode *>(node.get()));
            break;
        default:
            // ignore or handle other kinds
            break;
        }
    }

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

void IRGenVisitor::visitFunctionDecl(FunctionDeclNode *funcDecl)
{
    // 1) Grab the LLVM Function* from your prototype map
    llvm::Function *function = FunctionProtos[funcDecl->name];
    assert(function && "No prototype for function");

    // 2) Create entry block and set builder
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(Context, "entry", function);
    Builder.SetInsertPoint(BB);

    // 3) New scope for locals/params
    NamedValues.clear();

    // 4) Allocate space for each parameter and store the incoming arg
    unsigned idx = 0;
    for (auto &arg : function->args())
    {
        // Name it
        arg.setName(funcDecl->parameters[idx]->paramName);

        // Create an alloca in entry
        llvm::AllocaInst *alloca =
            createEntryBlockAlloca(function, arg.getName().str());

        // Store the initial argument value into our alloca
        Builder.CreateStore(&arg, alloca);

        // Remember it for lookups in the body
        NamedValues[arg.getName().str()] = alloca;
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
        if (funcDecl->returnType == "void")
        {
            Builder.CreateRetVoid();
        }
        else
        {
            Builder.CreateRet(
                llvm::ConstantInt::get(
                    Builder.getInt32Ty(), // <-- no Context here
                    0));
        }
    }

    // 7) Verify this function
    llvm::verifyFunction(*function, &llvm::errs());
}

void IRGenVisitor::visitConditional(ConditionalNode *ifs)
{
    llvm::Value *condVal = visitExpr(ifs->condition.get());
    llvm::Value *condBool = condVal;

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

    for (auto &stmt : thenBlockNode->statements)
    {
        visitStatement(stmt.get());
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

    case ASTNodeType::QueryNode:
        std::cerr << "Entered queryNODe\n";
        visitQuery(static_cast<QueryNode *>(node));
        break;

    default:
        std::cerr << "Unsupported statement type.\n";
        break;
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
    // 1) Evaluate the RHS into an llvm::Value*
    llvm::Value *rhsVal = visitExpr(assign->rhs.get());

    // 2) Find the alloca for the LHS variable
    auto it = NamedValues.find(assign->lhsName);
    if (it == NamedValues.end())
    {
        throw std::runtime_error("IRGenVisitor: undefined variable in assignment: " + assign->lhsName);
    }
    llvm::AllocaInst *alloca = it->second;

    // 3) Emit the store instruction
    Builder.CreateStore(rhsVal, alloca);
}

llvm::AllocaInst *IRGenVisitor::visitVarDecl(VarDeclNode *decl)
{
    llvm::Function *currentFunction = Builder.GetInsertBlock()->getParent();

    // --- 1) Handle array-initializers first ---
    if (decl->initializer && decl->initializer->type == ASTNodeType::ArrayLiteral)
    {
        auto *arrLit = static_cast<ArrayLiteralNode *>(decl->initializer.get());
        size_t N = arrLit->elements.size();

        // Build the LLVM array type [N x i32]
        llvm::ArrayType *arrTy = llvm::ArrayType::get(Builder.getInt32Ty(), N);

        // Create alloca in the entry block manually with arrTy
        // (we need a small IRBuilder positioned at entry)
        llvm::IRBuilder<> tmpB(&currentFunction->getEntryBlock(),
                               currentFunction->getEntryBlock().begin());
        llvm::AllocaInst *arrAlloca =
            tmpB.CreateAlloca(arrTy, nullptr, decl->name);

        // Store each initializer element
        for (size_t i = 0; i < N; ++i)
        {
            llvm::Value *val = visitExpr(arrLit->elements[i].get());
            llvm::Value *gep = Builder.CreateGEP(
                arrTy, arrAlloca,
                {Builder.getInt32(0),
                 Builder.getInt32(static_cast<int>(i))},
                decl->name + "_elem");
            Builder.CreateStore(val, gep);
        }

        NamedValues[decl->name] = arrAlloca;
        return arrAlloca;
    }

    // --- 2) Fallback scalar path ---
    llvm::AllocaInst *scalarAlloca =
        createEntryBlockAlloca(currentFunction, decl->name);

    if (decl->initializer)
    {
        llvm::Value *initVal = visitExpr(decl->initializer.get());
        Builder.CreateStore(initVal, scalarAlloca);
    }
    else
    {
        Builder.CreateStore(llvm::ConstantInt::get(
                                Builder.getInt32Ty(), 0),
                            scalarAlloca);
    }

    NamedValues[decl->name] = scalarAlloca;
    return scalarAlloca;
}

// void IRGenVisitor::visitWhile(WhileStmtNode *ws)
// {
//     // Save the current insertion point = preheader
//     llvm::BasicBlock *preheader = Builder.GetInsertBlock();
//     llvm::Function *parent = preheader->getParent();

//     // ---- NEW: create per-loop temporaries in the function entry and initialize them
//     // Save original NamedValues so we can restore after the loop
//     auto savedNamed = NamedValues;

//     // We'll keep a map of original -> temp allocas
//     std::unordered_map<std::string, llvm::AllocaInst *> tempAllocaMap;

//     // Create entry-block builder for entry-allocas
//     llvm::IRBuilder<> entryB(&parent->getEntryBlock(), parent->getEntryBlock().begin());

//     for (auto &kv : savedNamed) {
//         const std::string &varName = kv.first;
//         llvm::AllocaInst *origAlloca = kv.second;

//         // Create a temp alloca in entry block named varName.loop
//         llvm::AllocaInst *tempAlloca = entryB.CreateAlloca(
//             origAlloca->getAllocatedType(), nullptr, varName + ".loop");
//         // Load current value from original and store into temp (this executes at runtime before loop)
//         llvm::Value *origVal = Builder.CreateLoad(origAlloca->getAllocatedType(), origAlloca, varName + ".load_pre");
//         Builder.CreateStore(origVal, tempAlloca);

//         // Replace NamedValues mapping for the duration of lowering the loop
//         NamedValues[varName] = tempAlloca;
//         tempAllocaMap[varName] = tempAlloca;
//     }

//     // ---- Now create canonical loop structure
//     auto *condBB = llvm::BasicBlock::Create(Context, "loop.cond", parent);
//     auto *bodyBB = llvm::BasicBlock::Create(Context, "loop.body", parent);
//     auto *mergeBB = llvm::BasicBlock::Create(Context, "loop.merge", parent);

//     // Branch from preheader into loop condition
//     Builder.SetInsertPoint(preheader);
//     Builder.CreateBr(condBB);

//     // cond block: evaluate condition (using remapped NamedValues -> temps)
//     Builder.SetInsertPoint(condBB);
//     llvm::Value *condV = visitExpr(ws->condition.get());
//     Builder.CreateCondBr(condV, bodyBB, mergeBB);

//     // body: lowered with NamedValues pointing at the temps
//     Builder.SetInsertPoint(bodyBB);
//     visitBlock(static_cast<BlockStmtNode *>(ws->body.get()));
//     if (!Builder.GetInsertBlock()->getTerminator())
//         Builder.CreateBr(condBB);

//     // After loop: copy temps back into original allocas
//     Builder.SetInsertPoint(mergeBB);
//     for (auto &kv : tempAllocaMap) {
//         const std::string &name = kv.first;
//         llvm::AllocaInst *tempA = kv.second;
//         llvm::AllocaInst *origA = savedNamed[name]; // original stored earlier

//         // Load from temp and store into original (commits final loop value)
//         llvm::Value *finalVal = Builder.CreateLoad(tempA->getAllocatedType(), tempA, name + ".final");
//         Builder.CreateStore(finalVal, origA);
//     }

//     // Restore NamedValues to original mapping
//     NamedValues = savedNamed;
// }

void IRGenVisitor::visitWhile(WhileStmtNode *ws)
{
    llvm::BasicBlock *preheader = Builder.GetInsertBlock();
    llvm::Function *parent = preheader->getParent();

    auto *condBB = llvm::BasicBlock::Create(Context, "loopcond", parent);
    auto *bodyBB = llvm::BasicBlock::Create(Context, "loopbody", parent);
    auto *mergeBB = llvm::BasicBlock::Create(Context, "loopmerge", parent);

    Builder.SetInsertPoint(preheader);
    Builder.CreateBr(condBB);

    Builder.SetInsertPoint(condBB);
    llvm::Value *condV = visitExpr(ws->condition.get());
    Builder.CreateCondBr(condV, bodyBB, mergeBB);

    Builder.SetInsertPoint(bodyBB);
    visitBlock(static_cast<BlockStmtNode *>(ws->body.get()));
    // if the body didn’t already end in a terminator, loop back
    if (!Builder.GetInsertBlock()->getTerminator())
        Builder.CreateBr(condBB);

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
    case ASTNodeType::Variable:
    {
        auto *varNode = static_cast<VariableNode *>(expr);
        auto it = NamedValues.find(varNode->name);
        if (it == NamedValues.end())
        {
            throw std::runtime_error("Undefined variable: " + varNode->name);
        }
        return Builder.CreateLoad(it->second->getAllocatedType(), it->second, varNode->name);
    }
    case ASTNodeType::BinaryExpr:
    {
        auto *bin = static_cast<BinaryExprNode *>(expr);
        llvm::Value *lhs = visitExpr(bin->lhs.get());
        llvm::Value *rhs = visitExpr(bin->rhs.get());
        if (bin->op == "+")
        {
            return Builder.CreateAdd(lhs, rhs, "addtmp");
        }
        else if (bin->op == "-")
        {
            return Builder.CreateSub(lhs, rhs, "subtmp");
        }
        else if (bin->op == "*")
        {
            return Builder.CreateMul(lhs, rhs, "multmp");
        }
        else if (bin->op == "/")
        {
            return Builder.CreateSDiv(lhs, rhs, "divtmp");
        }
        else if (bin->op == "%")
        {
            return Builder.CreateSRem(lhs, rhs, "modtmp");
        }
        else if (bin->op == "==")
        {
            return Builder.CreateICmpEQ(lhs, rhs, "eqtmp");
        }
        else if (bin->op == "!=")
        {
            return Builder.CreateICmpNE(lhs, rhs, "netmp");
        }
        else if (bin->op == "<")
        {
            return Builder.CreateICmpSLT(lhs, rhs, "lttmp");
        }
        else if (bin->op == "<=")
        {
            return Builder.CreateICmpSLE(lhs, rhs, "letmp");
        }
        else if (bin->op == ">")
        {
            return Builder.CreateICmpSGT(lhs, rhs, "gttmp");
        }
        else if (bin->op == ">=")
        {
            return Builder.CreateICmpSGE(lhs, rhs, "getmp");
        }
        else
        {
            throw std::runtime_error("Unsupported binary operator: " + bin->op);
        }
    }
    case ASTNodeType::FunctionCall:
    {
        auto *FC = static_cast<FunctionCallNode *>(expr);
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

        return Builder.CreateCall(callee, args, "calltmp");
    }

    case ASTNodeType::ArrayAccess:
    {
        auto *A = static_cast<ArrayAccessNode *>(expr);
        // 1) find the alloca for the base array
        auto it = NamedValues.find(static_cast<VariableNode *>(A->arrayExpr.get())->name);
        if (it == NamedValues.end())
            throw std::runtime_error("Undefined array in access");
        llvm::AllocaInst *baseAlloca = it->second;

        // 2) compute the index
        llvm::Value *idxVal = visitExpr(A->indexExpr.get());

        // 3) build GEP: first index=0 (the array), then index=idxVal
        llvm::ArrayType *arrTy = llvm::cast<llvm::ArrayType>(baseAlloca->getAllocatedType());
        llvm::Value *elemPtr = Builder.CreateGEP(
            arrTy,
            baseAlloca,
            {Builder.getInt32(0), idxVal},
            "elemptr");

        // 4) load the i32 element
        auto *elemTy = llvm::cast<llvm::GetElementPtrInst>(elemPtr)->getSourceElementType();
        return Builder.CreateLoad(elemTy, elemPtr, "loadelem");
    }
    default:
        throw std::runtime_error("Unsupported expression type in IRGenVisitor");
    }
}

llvm::Value *IRGenVisitor::visitGraphDecl(GraphDeclNode *G)
{
    auto *I64 = llvm::Type::getInt64Ty(Context);
    auto *I32 = llvm::Type::getInt32Ty(Context);
    auto *arrRP = llvm::ArrayType::get(I64, G->row_ptr.size());
    auto *arrCI = llvm::ArrayType::get(I32, G->col_idx.size());

    llvm::SmallVector<llvm::Constant *, 16> rpConsts;
    for (auto x : G->row_ptr)
    {
        rpConsts.push_back(llvm::ConstantInt::get(I64, x));
    }
    auto *RPArray = llvm::ConstantArray::get(arrRP, rpConsts);

    llvm::SmallVector<llvm::Constant *, 16> ciConsts;
    for (auto x : G->col_idx)
    {
        ciConsts.push_back(llvm::ConstantInt::get(I32, x));
    }
    auto *CIArray = llvm::ConstantArray::get(arrCI, ciConsts);

    auto *GV_RP = new llvm::GlobalVariable(
        Module, arrRP, true, llvm::GlobalValue::InternalLinkage,
        RPArray, G->name + "_csr_row");

    auto *GV_CI = new llvm::GlobalVariable(
        Module, arrCI, true, llvm::GlobalValue::InternalLinkage,
        CIArray, G->name + "_csr_col");

    auto *mallocTy = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
        {llvm::Type::getInt64Ty(Context)},
        false);

    llvm::FunctionCallee mallocCalle = Module.getOrInsertFunction("malloc", mallocTy);
    llvm::Function *mallocFn = llvm::cast<llvm::Function>(mallocCalle.getCallee());

    uint64_t rowBytes = G->row_ptr.size() * sizeof(int64_t);
    uint64_t colBytes = G->col_idx.size() * sizeof(int32_t);

    llvm::Value *rpRaw = Builder.CreateCall(
        mallocFn, llvm::ConstantInt::get(I64, rowBytes), "rp_raw");
    llvm::Value *rowPtr = Builder.CreateBitCast(
        rpRaw, llvm::PointerType::getUnqual(I64), "row_ptr");

    llvm::Value *ciRaw = Builder.CreateCall(
        mallocFn, llvm::ConstantInt::get(I64, colBytes), "ci_raw");
    llvm::Value *colPtr = Builder.CreateBitCast(
        ciRaw, llvm::PointerType::getUnqual(I32), "col_ptr");

    llvm::Value *rp_i8 = Builder.CreateBitCast(rowPtr,
                                               llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *ci_i8 = Builder.CreateBitCast(colPtr,
                                               llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *src_rp = Builder.CreateBitCast(GV_RP,
                                                llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    llvm::Value *src_ci = Builder.CreateBitCast(GV_CI,
                                                llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));

    // Define the memcpy intrinsic type
    auto *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
    auto *i64Ty = llvm::Type::getInt64Ty(Context);
    auto *memcpyTy = llvm::Intrinsic::getDeclaration(
        &Module, llvm::Intrinsic::memcpy,
        {i8PtrTy, i8PtrTy, i64Ty});

    // Define the volatility flag as i1
    auto *volatileFlag = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 0);

    // Perform the memcpy calls with correct argument types
    Builder.CreateCall(memcpyTy, {rp_i8, src_rp, llvm::ConstantInt::get(I64, rowBytes), volatileFlag}, "");
    Builder.CreateCall(memcpyTy, {ci_i8, src_ci, llvm::ConstantInt::get(I64, colBytes), volatileFlag}, "");

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

    GraphMap[G->name] = graphPtr;

    return graphPtr;
}


void IRGenVisitor::visitQuery(QueryNode *Q)
{
    // 1) Grab the Graph* value
    llvm::Value *graphPtr = GraphMap[Q->graphName];
    assert(graphPtr && "Graph not found in IRGenVisitor::visitQuery");

    // 2) Extract n, row_ptr*, col_idx*
    llvm::Value *n_ptr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
    llvm::Value *rp_ptr = Builder.CreateStructGEP(GraphTy, graphPtr, 2, "g_rp_ptr");
    llvm::Value *ci_ptr = Builder.CreateStructGEP(GraphTy, graphPtr, 3, "g_ci_ptr");

    llvm::Value *n_val = Builder.CreateLoad(Builder.getInt64Ty(), n_ptr, "n_val");
    llvm::Value *row_ptr = Builder.CreateLoad(
        llvm::PointerType::getUnqual(Builder.getInt64Ty()),
        rp_ptr, "row_ptr");
    llvm::Value *col_idx = Builder.CreateLoad(
        llvm::PointerType::getUnqual(Builder.getInt32Ty()),
        ci_ptr, "col_idx");

    // 3) Get malloc/free
    llvm::Type *i8Ty = llvm::Type::getInt8Ty(Context);
    llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
    llvm::Type *i8PtrTy = llvm::PointerType::getUnqual(i8Ty);
    auto mallocFn = Module.getOrInsertFunction(
        "malloc",
        llvm::FunctionType::get(i8PtrTy, {i64Ty}, false));
    auto freeFn = Module.getOrInsertFunction(
        "free",
        llvm::FunctionType::get(Builder.getVoidTy(), {i8PtrTy}, false));

    llvm::Type *i1Ty = llvm::Type::getInt1Ty(Context);
    llvm::Type *i1PtrTy = llvm::PointerType::getUnqual(i1Ty);

    // 4) malloc + zero-init visited (i1*)
    llvm::Value *bytesVisited = Builder.CreateMul(
        Builder.CreateSExt(n_val, i64Ty),
        llvm::ConstantInt::get(i64Ty, 1));
    llvm::Value *rawVisited = Builder.CreateCall(mallocFn, bytesVisited, "rawVisited");
    // memset(rawVisited, 0, bytesVisited, align=1)
    llvm::Value *raw8Visited = Builder.CreateBitCast(rawVisited, i8PtrTy, "raw8Visited");
    Builder.CreateMemSet(raw8Visited, Builder.getInt8(0), bytesVisited, llvm::MaybeAlign(1));
    llvm::Value *visited = Builder.CreateBitCast(
        rawVisited,
        i1PtrTy,
        "visited");

    // 5) malloc queue (i32*)
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *i32PtrTy = llvm::PointerType::getUnqual(i32Ty);
    llvm::Value *bytesQueue = Builder.CreateMul(
        Builder.CreateSExt(n_val, i64Ty),
        llvm::ConstantInt::get(i64Ty, 4));
    llvm::Value *rawQueue = Builder.CreateCall(mallocFn, bytesQueue, "rawQueue");
    llvm::Value *queue = Builder.CreateBitCast(rawQueue, i32PtrTy, "queue");

    // 6) alloca head, tail in entry
    llvm::Function *F = Builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> entryB(&F->getEntryBlock(), F->getEntryBlock().begin());
    auto *headPtr = entryB.CreateAlloca(i32Ty, nullptr, "head");
    auto *tailPtr = entryB.CreateAlloca(i32Ty, nullptr, "tail");
    Builder.CreateStore(Builder.getInt32(0), headPtr);
    Builder.CreateStore(Builder.getInt32(0), tailPtr);

    // 7) enqueue start = 0
    Builder.CreateStore(Builder.getInt32(0),
                        Builder.CreateGEP(i32Ty, queue, Builder.getInt32(0)));
    Builder.CreateStore(Builder.getInt1(true),
                        Builder.CreateGEP(llvm::PointerType::getUnqual(llvm::Type::getInt1Ty(Context)),
                                          visited, Builder.getInt32(0)));
    Builder.CreateStore(Builder.getInt32(1), tailPtr);

    // 8) BFS outer loop
    auto *condBB = llvm::BasicBlock::Create(Context, "bfs.cond", F);
    auto *bodyBB = llvm::BasicBlock::Create(Context, "bfs.body", F);
    auto *exitBB = llvm::BasicBlock::Create(Context, "bfs.exit", F);
    Builder.CreateBr(condBB);

    Builder.SetInsertPoint(condBB);
    auto *h = Builder.CreateLoad(i32Ty, headPtr, "h");
    auto *t = Builder.CreateLoad(i32Ty, tailPtr, "t");
    auto *cmp = Builder.CreateICmpSLT(h, t, "cond");
    Builder.CreateCondBr(cmp, bodyBB, exitBB);

    Builder.SetInsertPoint(bodyBB);
    // dequeue u = queue[h]
    auto *uqptr = Builder.CreateGEP(i32Ty, queue, h, "uqptr");
    auto *u = Builder.CreateLoad(i32Ty, uqptr, "u");
    Builder.CreateStore(Builder.CreateAdd(h, Builder.getInt32(1)), headPtr);

    // rp_u = row_ptr[u], rp_u1 = row_ptr[u+1]
    auto *u64 = Builder.CreateSExt(u, i64Ty);
    auto *rp_u = Builder.CreateLoad(i64Ty,
                                    Builder.CreateGEP(llvm::PointerType::getUnqual(i64Ty), row_ptr, {u64}), "rp_u");
    auto *rp_u1 = Builder.CreateLoad(i64Ty,
                                     Builder.CreateGEP(llvm::PointerType::getUnqual(i64Ty), row_ptr,
                                                       {Builder.CreateAdd(u64, Builder.getInt64(1))}),
                                     "rp_u1");

    // inner loop init/body/cont
    auto *forInitBB = llvm::BasicBlock::Create(Context, "bfs.for.init", F);
    auto *forBodyBB = llvm::BasicBlock::Create(Context, "bfs.for.body", F);
    auto *enqueueBB = llvm::BasicBlock::Create(Context, "bfs.enqueue", F);
    auto *doEnqBB = llvm::BasicBlock::Create(Context, "bfs.doEnqueue", F);
    auto *contBB = llvm::BasicBlock::Create(Context, "bfs.for.cont", F);
    auto *exitBB_inner = llvm::BasicBlock::Create(Context, "bfs.for.exit", F);

    Builder.CreateBr(forInitBB);

    Builder.SetInsertPoint(forInitBB);
    llvm::AllocaInst *iPtr = entryB.CreateAlloca(i64Ty, nullptr, "i");
    Builder.CreateStore(rp_u, iPtr);
    Builder.CreateBr(forBodyBB);

    Builder.SetInsertPoint(forBodyBB);

    llvm::Value *iVal = Builder.CreateLoad(i64Ty, iPtr, "iVal");
    llvm::Value *inner = Builder.CreateICmpSLT(iVal, rp_u1, "innerCond");
    Builder.CreateCondBr(inner, enqueueBB, exitBB_inner);

    Builder.SetInsertPoint(enqueueBB);

    llvm::Value *nbr = Builder.CreateLoad(i32Ty,
                                          Builder.CreateGEP(i32PtrTy, col_idx,
                                                            Builder.CreateTrunc(iVal, i32Ty)),
                                          "nbr");
    llvm::Value *vptr = Builder.CreateGEP(i1PtrTy, visited, nbr, "vptr");
    llvm::Value *was = Builder.CreateLoad(i1Ty, vptr, "wasVisited");
    Builder.CreateCondBr(
        Builder.CreateICmpEQ(was, Builder.getInt1(false)),
        doEnqBB,
        contBB);

    Builder.SetInsertPoint(doEnqBB);
    llvm::Value *tailVal = Builder.CreateLoad(i32Ty, tailPtr, "tailVal");
    Builder.CreateStore(nbr,
                        Builder.CreateGEP(i32Ty, queue, tailVal));
    Builder.CreateStore(Builder.getInt1(true), vptr);
    Builder.CreateStore(Builder.CreateAdd(tailVal, Builder.getInt32(1)), tailPtr);
    Builder.CreateBr(contBB);

    Builder.SetInsertPoint(contBB);
    Builder.CreateStore(
        Builder.CreateAdd(iVal, Builder.getInt64(1)),
        iPtr);
    Builder.CreateBr(forBodyBB);

    // 9) exit: free buffers
    Builder.SetInsertPoint(exitBB_inner);
    Builder.CreateBr(condBB);

    Builder.SetInsertPoint(exitBB);
    Builder.CreateCall(freeFn, Builder.CreateBitCast(rawVisited, i8PtrTy));
    Builder.CreateCall(freeFn, Builder.CreateBitCast(rawQueue, i8PtrTy));
    // void return
}

void IRGenVisitor::visitPrintStmt(PrintStmtNode *PS)
{
    // 1) Ensure printf is declared
    llvm::Function *printfFn = Module.getFunction("printf");
    if (!printfFn)
    {
        // int printf(char*, ...)
        auto *i8p = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
        auto *printfTy = llvm::FunctionType::get(
            /*Ret=*/Builder.getInt32Ty(),
            /*Params=*/{i8p},
            /*isVarArg=*/true);
        printfFn = llvm::Function::Create(
            printfTy,
            llvm::Function::ExternalLinkage,
            "printf",
            &Module);
    }

    // 2) Prepare format string
    llvm::Value *fmtPtr = nullptr;
    llvm::Value *val = nullptr;

    if (auto *lit = dynamic_cast<IntLiteralNode *>(PS->expr.get()))
    {
        // integer literal: we’ll just embed the value
        val = llvm::ConstantInt::get(Builder.getInt32Ty(), lit->value);
        // constant global: "%d\n\0"
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
        fmtPtr = Builder.CreateBitCast(fmtInt,
                                       llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    }
    else if (auto *var = dynamic_cast<VariableNode *>(PS->expr.get()))
    {
        // assume it's an int variable
        llvm::Value *alloca = NamedValues[var->name];
        val = Builder.CreateLoad(Builder.getInt32Ty(), alloca, var->name);
        // same fmtInt as above
        static llvm::GlobalVariable *fmtInt = nullptr;
        if (!fmtInt)
        {
            auto *fmtTy = llvm::ArrayType::get(Builder.getInt8Ty(), 4);
            fmtInt = new llvm::GlobalVariable(
                Module, fmtTy, true,
                llvm::GlobalValue::PrivateLinkage,
                llvm::ConstantDataArray::getString(Context, "%d\n", true),
                ".fmt_int");
        }
        fmtPtr = Builder.CreateBitCast(fmtInt,
                                       llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
    }
    else
    {
        // for string literals, extend similarly:
        // extract the const char* from your AST’s StringLiteralNode...
        // use "%s\n" format.
        throw std::runtime_error("print: only int literals & vars supported so far");
    }

    // 3) Call printf(fmtPtr, val)
    Builder.CreateCall(printfFn, {fmtPtr, val});
}
