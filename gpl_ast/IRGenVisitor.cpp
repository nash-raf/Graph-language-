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
    // std::cerr << "[IRGen] visitAssignment called for LHS type = " << static_cast<int>(assign->lhs->type) << "\n";

    // Evaluate RHS first
    llvm::Value *rhsVal = visitExpr(assign->rhs.get());

    // Case 1: LHS is a scalar variable
    if (assign->lhs->type == ASTNodeType::Variable)
    {
        auto *varNode = static_cast<VariableNode *>(assign->lhs.get());
        auto it = NamedValues.find(varNode->name);
        if (it == NamedValues.end())
            throw std::runtime_error("IRGenVisitor: undefined variable in assignment: " + varNode->name);

        llvm::AllocaInst *alloca = it->second;
        llvm::Type *allocTy = alloca->getAllocatedType();

        // Cast RHS if needed
        if (rhsVal->getType() != allocTy)
        {
            if (rhsVal->getType()->isIntegerTy() && allocTy->isIntegerTy())
                rhsVal = Builder.CreateIntCast(rhsVal, llvm::cast<llvm::IntegerType>(allocTy), true);
            else
                throw std::runtime_error("IRGenVisitor: type mismatch in scalar assignment");
        }

        Builder.CreateStore(rhsVal, alloca);
        return;
    }

    // Case 2: LHS is an array element access
    else if (assign->lhs->type == ASTNodeType::ArrayAccess)
    {
        auto *A = static_cast<ArrayAccessNode *>(assign->lhs.get());
        auto *baseVar = dynamic_cast<VariableNode *>(A->arrayExpr.get());
        if (!baseVar)
            throw std::runtime_error("IRGenVisitor: unsupported array base expression");

        auto it = NamedValues.find(baseVar->name);
        if (it == NamedValues.end())
            throw std::runtime_error("IRGenVisitor: undefined array in assignment: " + baseVar->name);

        llvm::AllocaInst *baseAlloca = it->second;

        // Evaluate the index
        llvm::Value *idxVal = visitExpr(A->indexExpr.get());
        if (idxVal->getType() != Builder.getInt32Ty())
            idxVal = Builder.CreateIntCast(idxVal, Builder.getInt32Ty(), true);

        // If it's a static array allocated on stack
        if (auto *arrTy = llvm::dyn_cast<llvm::ArrayType>(baseAlloca->getAllocatedType()))
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

        // If it's a pointer to dynamically allocated array
        else if (auto *ptrTy = llvm::dyn_cast<llvm::PointerType>(baseAlloca->getAllocatedType()))
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

    // Unsupported LHS
    else
    {
        throw std::runtime_error("IRGenVisitor: Unsupported LHS in assignment");
    }
}

llvm::AllocaInst *IRGenVisitor::visitVarDecl(VarDeclNode *decl)
{
    llvm::Function *currentFunction = Builder.GetInsertBlock()->getParent();

    // --- Array path (explicit array or array-initializer) ---
    if (decl->isArray)
    {
        size_t N = decl->arraySize;

        // If initializer present and is ArrayLiteral, we can verify/adjust
        if (decl->initializer && decl->initializer->type == ASTNodeType::ArrayLiteral)
        {
            auto *arrLit = static_cast<ArrayLiteralNode *>(decl->initializer.get());
            // If explicit size is 0 (shouldn't happen with our builder), infer from initializer
            if (N == 0)
                N = arrLit->elements.size();
            // If initializer is larger than declared size, you may want to error or truncate
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

        NamedValues[decl->name] = arrAlloca;
        return arrAlloca;
    }

    // --- Scalar path ---
    llvm::AllocaInst *scalarAlloca =
        createEntryBlockAlloca(currentFunction, decl->name);

    if (decl->initializer)
    {
        llvm::Value *initVal = visitExpr(decl->initializer.get());
        if (initVal->getType() != Builder.getInt32Ty() && initVal->getType()->isIntegerTy())
        {
            initVal = Builder.CreateIntCast(initVal, Builder.getInt32Ty(), true);
        }
        Builder.CreateStore(initVal, scalarAlloca);
    }
    else
    {
        Builder.CreateStore(llvm::ConstantInt::get(Builder.getInt32Ty(), 0), scalarAlloca);
    }

    NamedValues[decl->name] = scalarAlloca;
    return scalarAlloca;
}
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

    llvm::Value *graphPtr = GraphMap[fs->graphName];
    if (!graphPtr)
        throw std::runtime_error("Graph not allocated: " + fs->graphName);

    if (!GraphTy)
        throw std::runtime_error("GraphTy not initialized");

    llvm::errs() << "Graph field 0 type: " << *GraphTy->getElementType(0) << "\n";
    llvm::errs() << "Graph field 1 type: " << *GraphTy->getElementType(1) << "\n";
    llvm::errs() << "Graph field 2 type: " << *GraphTy->getElementType(2) << "\n";
    llvm::errs() << "Graph field 3 type: " << *GraphTy->getElementType(3) << "\n";

    llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);

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

    Builder.CreateBr(condBB);

    // --- Condition ---
    Builder.SetInsertPoint(condBB);
    llvm::Value *idxVal = Builder.CreateLoad(i64Ty, idxAlloca, "idx_val");
    llvm::Value *cond = Builder.CreateICmpSLT(idxVal, nVal, "loop_cond");
    Builder.CreateCondBr(cond, bodyBB, mergeBB);

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
        auto *baseVar = dynamic_cast<VariableNode *>(A->arrayExpr.get());
        if (!baseVar)
            throw std::runtime_error("Array base must be variable");

        auto it = NamedValues.find(baseVar->name);
        if (it == NamedValues.end())
            throw std::runtime_error("Undefined array in access: " + baseVar->name);
        llvm::AllocaInst *baseAlloca = it->second;

        llvm::Value *idxVal = visitExpr(A->indexExpr.get());
        if (idxVal->getType() != Builder.getInt32Ty())
            idxVal = Builder.CreateIntCast(idxVal, Builder.getInt32Ty(), true);

        auto *arrTy = llvm::dyn_cast<llvm::ArrayType>(baseAlloca->getAllocatedType());
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
            // pointer case: baseAlloca holds a pointer to elements
            llvm::Type *baseTy = baseAlloca->getAllocatedType();
            if (auto *ptrTy = llvm::dyn_cast<llvm::PointerType>(baseTy))
            {
                llvm::Type *elemTy = ptrTy->getContainedType(0);

                // Compute element pointer with GEP
                llvm::Value *elemPtr = Builder.CreateGEP(
                    elemTy,
                    baseAlloca,
                    {idxVal},
                    baseVar->name + "_elemptr");

                // Load element and return
                return Builder.CreateLoad(elemTy, elemPtr, baseVar->name + "_loadelem");
            }

            throw std::runtime_error("IRGenVisitor: unsupported array storage type for load " + baseVar->name);
        }
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

    auto *mallocTy = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)),
        {llvm::Type::getInt64Ty(Context)},
        false);

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

    llvm::Function *F = Builder.GetInsertBlock()->getParent();
    llvm::AllocaInst *graphAlloca = createEntryBlockAlloca(F, G->name);
    Builder.CreateStore(graphPtr, graphAlloca);
    NamedValues[G->name] = graphAlloca;
    // debug
    llvm::errs() << "[visitGraphDecl] G->n (declared) = " << G->n << "\n";
    llvm::errs() << "[visitGraphDecl] G->row_ptr entries = " << (G->n + 1) << "\n";
    if (G->n > 0)
        llvm::errs() << "[visitGraphDecl] row_ptr[n] = " << G->row_ptr[G->n] << "\n";
    llvm::errs() << "[visitGraphDecl] G->col_idx entries = " << G->m << "\n";

    return graphPtr;
}

llvm::Value *IRGenVisitor::visitWeightedGraphDecl(WeightedGraphDeclNode *G)
{
    auto *I64 = llvm::Type::getInt64Ty(Context);
    auto *I32 = llvm::Type::getInt32Ty(Context);

    GraphTy = llvm::StructType::create(
        Context,
        {
            llvm::Type::getInt64Ty(Context),                               // n
            llvm::Type::getInt64Ty(Context),                               // m
            llvm::PointerType::getUnqual(llvm::Type::getInt64Ty(Context)), // row_ptr
            llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(Context)), // col_idx
            llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(Context))  // weights
        },
        "struct.Graph");

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

    GraphMap[G->name] = graphPtr;

    llvm::Function *F = Builder.GetInsertBlock()->getParent();
    llvm::AllocaInst *graphAlloca = createEntryBlockAlloca(F, G->name);
    Builder.CreateStore(graphPtr, graphAlloca);
    NamedValues[G->name] = graphAlloca;
    // debug
    llvm::errs() << "[visitGraphDecl] G->n (declared) = " << G->n << "\n";
    llvm::errs() << "[visitGraphDecl] G->row_ptr entries = " << (G->n + 1) << "\n";
    if (G->n > 0)
        llvm::errs() << "[visitGraphDecl] row_ptr[n] = " << G->row_ptr[G->n] << "\n";
    llvm::errs() << "[visitGraphDecl] G->col_idx entries = " << G->m << "\n";
    // llvm::errs() << "[visitGraphDecl] G->w_idx entries = " << G->weights << "\n";

    return graphPtr;
}

// void IRGenVisitor::emitBFS(QueryNode *Q)
// {
//     // Grab the Graph* value
//     llvm::Value *graphPtr = GraphMap[Q->graphName];
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
//     llvm::Value *graphPtr = GraphMap[Q->graphName];
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
    llvm::Value *graphPtr = GraphMap[Q->graphName];
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
    llvm::Value *graphPtr = GraphMap[Q->graphName];
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

    llvm::Value *arrPtr = Builder.CreateLoad(llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(Context)), arrIt->second);
    llvm::Value *arrSize = Builder.CreateLoad(llvm::Type::getInt32Ty(Context), sizeIt->second);

    llvm::Function *printfFunc = getPrintfFunction(); // helper function

    // Create loop variables
    llvm::Value *index = Builder.CreateAlloca(llvm::Type::getInt32Ty(Context));
    Builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0), index);

    llvm::Function *curFunc = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(Context, "print_loop", curFunc);
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(Context, "after_print", curFunc);
    Builder.CreateBr(loopBB);

    Builder.SetInsertPoint(loopBB);
    llvm::Value *iVal = Builder.CreateLoad(llvm::Type::getInt32Ty(Context), index);
    llvm::Value *cond = Builder.CreateICmpSLT(iVal, arrSize);
    Builder.CreateCondBr(cond, loopBB, afterBB);

    // Load element and call printf
    llvm::Value *elemPtr = Builder.CreateGEP(
        llvm::Type::getInt32Ty(Context), arrPtr, {iVal});
    llvm::Value *elem = Builder.CreateLoad(llvm::Type::getInt32Ty(Context), elemPtr);
    Builder.CreateCall(printfFunc, {Builder.CreateGlobalStringPtr("%d "), elem});

    // Increment index
    llvm::Value *nextI = Builder.CreateAdd(iVal, llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 1));
    Builder.CreateStore(nextI, index);

    Builder.CreateBr(loopBB);
    Builder.SetInsertPoint(afterBB);
    Builder.CreateCall(printfFunc, {Builder.CreateGlobalStringPtr("\n")});
}

void IRGenVisitor::emitFloydWarshall(QueryNode *Q)
{
    // Grab the Graph* value
    llvm::Value *graphPtr = GraphMap[Q->graphName];
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
    llvm::Value *graphPtr = GraphMap[Q->graphName];
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

// void IRGenVisitor::emitChromacity(QueryNode *Q)
// {
//     // Grab the Graph* value
//     llvm::Value *graphPtr = GraphMap[Q->graphName];
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
    llvm::Value *graphPtr = GraphMap[Q->graphName];
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
    llvm::AllocaInst *varPtr = nullptr;
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
    else
    {
        llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n";
        assert(false && "Unknown query type in QueryNode");
    }
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