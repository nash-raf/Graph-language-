#include "IRGenVisitor.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include <chrono>
#include "GraphStorage.h"


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
        // case ASTNodeType::ForEachStmt:
        //     visitForEach(static_cast<ForEachStmtNode *>(node.get()));
        //     break;
        // case ASTNodeType::GraphDecl:
        //     // Lower graph h { … }
        //     visitGraphDecl(static_cast<GraphDeclNode *>(node.get()));
        //     break;
        case ASTNodeType::GraphDecl:
        {
            auto *GD = static_cast<GraphDeclNode *>(node.get());
            GraphDecls[GD->name] = GD;              
            visitGraphDecl(GD);
            break;
        }
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
    case ASTNodeType::GraphDecl:
        visitGraphDecl(static_cast<GraphDeclNode*>(node));
        return;
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

// void IRGenVisitor::visitForEach(ForEachStmtNode *fs) {
//     llvm::Function *parent = Builder.GetInsertBlock()->getParent();

//     llvm::Value *graphPtr = GraphMap[fs->graphName];
//     if (!graphPtr)
//         throw std::runtime_error("Graph not allocated: " + fs->graphName);

//     if (!GraphTy)
//         throw std::runtime_error("GraphTy not initialized");

//     llvm::errs() << "Graph field 0 type: " << *GraphTy->getElementType(0) << "\n";
//     llvm::errs() << "Graph field 1 type: " << *GraphTy->getElementType(1) << "\n";
//     llvm::errs() << "Graph field 2 type: " << *GraphTy->getElementType(2) << "\n";
//     llvm::errs() << "Graph field 3 type: " << *GraphTy->getElementType(3) << "\n";

//     llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
//     llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);

//     // --- Load n (number of vertices) ---
//     llvm::Value *nPtr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
//     llvm::Value *nVal = Builder.CreateLoad(i64Ty, nPtr, "n_val");

//     // Allocate loop index (i64) and user variable (i32)
//     llvm::IRBuilder<> TmpB(&parent->getEntryBlock(), parent->getEntryBlock().begin());
//     auto *idxAlloca  = TmpB.CreateAlloca(i64Ty, nullptr, fs->var1 + ".idx64");
//     auto *userAlloca = TmpB.CreateAlloca(i32Ty, nullptr, fs->var1);

//     NamedValues[fs->var1] = userAlloca;

//     // idx = 0
//     Builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), idxAlloca);

//     // Blocks
//     auto *condBB  = llvm::BasicBlock::Create(Context, "foreach.cond", parent);
//     auto *bodyBB  = llvm::BasicBlock::Create(Context, "foreach.body", parent);
//     auto *incBB   = llvm::BasicBlock::Create(Context, "foreach.inc", parent);
//     auto *mergeBB = llvm::BasicBlock::Create(Context, "foreach.merge", parent);

//     Builder.CreateBr(condBB);

//     // --- Condition ---
//     Builder.SetInsertPoint(condBB);
//     llvm::Value *idxVal = Builder.CreateLoad(i64Ty, idxAlloca, "idx_val");
//     llvm::Value *cond = Builder.CreateICmpSLT(idxVal, nVal, "loop_cond");
//     Builder.CreateCondBr(cond, bodyBB, mergeBB);

//     // --- Body ---
//     Builder.SetInsertPoint(bodyBB);

//     // Cast/truncate idxVal (i64) → i32 safely
//     llvm::Value *idxVal32 = Builder.CreateTrunc(idxVal, i32Ty, "idx32");

//     // Store into user variable
//     Builder.CreateStore(idxVal32, userAlloca);

//     // Visit loop body
//     visitBlock(static_cast<BlockStmtNode *>(fs->body.get()));
//     if (!Builder.GetInsertBlock()->getTerminator())
//         Builder.CreateBr(incBB);

//     // --- Increment ---
//     Builder.SetInsertPoint(incBB);
//     llvm::Value *nextIdx = Builder.CreateAdd(
//         idxVal,
//         llvm::ConstantInt::get(i64Ty, 1),
//         "idx_next"
//     );
//     Builder.CreateStore(nextIdx, idxAlloca);
//     Builder.CreateBr(condBB);

//     // --- Merge ---
//     Builder.SetInsertPoint(mergeBB);
// }



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



llvm::Value *IRGenVisitor::visitGraphDecl(GraphDeclNode *G)
{
    if (!G) return nullptr;

    // If already lowered/allocated, just return it.
    auto it = GraphMap.find(G->name);
    if (it != GraphMap.end())
        return it->second;

    // Allocate graph struct on stack (fast)
    llvm::AllocaInst *graphAlloca = Builder.CreateAlloca(GraphTy, nullptr, G->name + ".graph");

    // Detect file-backed edge list
    bool fromFile = false;
    std::string path;
    if (G->edgesSrc)
    {
        if (auto *fe = dynamic_cast<FileEdgeList*>(G->edgesSrc.get()))
        {
            fromFile = true;
            path = fe->path();
        }
    }

    // ---- Fast path for huge graphs: runtime mmap loader builds CSR + nodes_orig ----
    if (fromFile)
    {
        // extern "C" int load_edgelist_build_csr_mmap(const char* path, Graph* out, int directed);
        llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);
        llvm::Type *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
        llvm::Type *graphPtrTy = GraphTy->getPointerTo();

        auto *FT = llvm::FunctionType::get(i32Ty, {i8PtrTy, graphPtrTy, i32Ty}, false);
        auto loader = Module.getOrInsertFunction("load_edgelist_build_csr_mmap", FT);

        llvm::Value *pathStr = Builder.CreateGlobalStringPtr(path, G->name + ".path");
        llvm::Value *directedVal = llvm::ConstantInt::get(i32Ty, 1); // directed: grammar uses '->'

        llvm::Value *rc = Builder.CreateCall(loader, {pathStr, graphAlloca, directedVal}, "load_rc");

        // If rc != 0, trap (fast fail)
        llvm::Value *ok = Builder.CreateICmpEQ(rc, llvm::ConstantInt::get(i32Ty, 0), "load_ok");
        llvm::Function *parent = Builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *okBB = llvm::BasicBlock::Create(Context, "load.ok", parent);
        llvm::BasicBlock *badBB = llvm::BasicBlock::Create(Context, "load.bad", parent);
        Builder.CreateCondBr(ok, okBB, badBB);

        Builder.SetInsertPoint(badBB);
        Builder.CreateUnreachable();

        Builder.SetInsertPoint(okBB);

        GraphMap[G->name] = graphAlloca;
        GraphLayoutMap[G->name] = gpl::GraphLayoutKind::PackedCSR; // file-loader builds CSR-like layout
        return graphAlloca;
    }

    // ---- Non-file graphs: materialize in compiler, then malloc+memcpy small globals ----

    if (!G->materialized)
    {
        auto hints = Autotuner.choose(G->set, "", "");
        G->chosenKind = hints.preferred;
        G->mat.kind = hints.preferred;

        if (hints.preferred == gpl::GraphLayoutKind::PackedCSR)
            G->mat.packed = gpl::build_packed_csr(G->set);
        else if (hints.preferred == gpl::GraphLayoutKind::BlockedCSR)
            G->mat.blocked = gpl::build_blocked_csr(G->set, hints.blockedBlockSize);
        else
            throw std::runtime_error("visitGraphDecl: unsupported layout kind");

        G->materialized = true;
    }

    const bool isPacked = (G->chosenKind == gpl::GraphLayoutKind::PackedCSR);
    const auto &row_ptr = isPacked ? G->mat.packed.row_ptr : G->mat.blocked.row_ptr;
    const auto &col_idx = isPacked ? G->mat.packed.col_idx : G->mat.blocked.col_idx;
    const auto &nodes_orig = isPacked ? G->mat.packed.nodes : G->mat.blocked.nodes;

    const uint64_t n = uint64_t(row_ptr.size() ? (row_ptr.size() - 1) : 0);
    const uint64_t m = uint64_t(col_idx.size());

    llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
    llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *i8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));

    const uint64_t rowBytes = row_ptr.size() * sizeof(uint64_t);
    const uint64_t colBytes = col_idx.size() * sizeof(uint32_t);
    const uint64_t nodesBytes = nodes_orig.size() * sizeof(uint32_t);

    if (rowBytes + colBytes + nodesBytes > GlobalEmbedThresholdBytes)
    {
        throw std::runtime_error(
            "visitGraphDecl: graph too large to embed as LLVM globals. Use file edge-list input to enable mmap loader.");
    }

    // malloc
    auto *mallocTy = llvm::FunctionType::get(i8PtrTy, {i64Ty}, false);
    llvm::FunctionCallee mallocCallee = Module.getOrInsertFunction("malloc", mallocTy);

    // memcpy intrinsic
    auto *memcpyDecl = llvm::Intrinsic::getDeclaration(&Module, llvm::Intrinsic::memcpy, {i8PtrTy, i8PtrTy, i64Ty});
    llvm::Value *isVolatile = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 0);

    // Allocate heap buffers
    llvm::Value *rpRaw = Builder.CreateCall(mallocCallee, {llvm::ConstantInt::get(i64Ty, rowBytes)}, "rp_raw");
    llvm::Value *ciRaw = Builder.CreateCall(mallocCallee, {llvm::ConstantInt::get(i64Ty, colBytes)}, "ci_raw");
    llvm::Value *noRaw = Builder.CreateCall(mallocCallee, {llvm::ConstantInt::get(i64Ty, nodesBytes)}, "nodes_raw");

    llvm::Value *rowPtr = Builder.CreateBitCast(rpRaw, llvm::PointerType::getUnqual(i64Ty), "row_ptr");
    llvm::Value *colPtr = Builder.CreateBitCast(ciRaw, llvm::PointerType::getUnqual(i32Ty), "col_ptr");
    llvm::Value *nodesPtr = Builder.CreateBitCast(noRaw, llvm::PointerType::getUnqual(i32Ty), "nodes_orig");

    auto makeU64Global = [&](const std::vector<uint64_t>& vec, const std::string& nm) -> llvm::GlobalVariable* {
        std::vector<llvm::Constant*> cs;
        cs.reserve(vec.size());
        for (uint64_t x : vec) cs.push_back(llvm::ConstantInt::get(i64Ty, x));
        auto *arrTy = llvm::ArrayType::get(i64Ty, cs.size());
        auto *init = llvm::ConstantArray::get(arrTy, cs);
        return new llvm::GlobalVariable(Module, arrTy, true, llvm::GlobalValue::PrivateLinkage, init, nm);
    };
    auto makeU32Global = [&](const std::vector<uint32_t>& vec, const std::string& nm) -> llvm::GlobalVariable* {
        std::vector<llvm::Constant*> cs;
        cs.reserve(vec.size());
        for (uint32_t x : vec) cs.push_back(llvm::ConstantInt::get(i32Ty, x));
        auto *arrTy = llvm::ArrayType::get(i32Ty, cs.size());
        auto *init = llvm::ConstantArray::get(arrTy, cs);
        return new llvm::GlobalVariable(Module, arrTy, true, llvm::GlobalValue::PrivateLinkage, init, nm);
    };

    auto *rpGV = makeU64Global(row_ptr, G->name + ".rowptr.rodata");
    auto *ciGV = makeU32Global(col_idx, G->name + ".colidx.rodata");
    auto *noGV = makeU32Global(nodes_orig, G->name + ".nodesorig.rodata");

    Builder.CreateCall(memcpyDecl, {Builder.CreateBitCast(rowPtr, i8PtrTy), Builder.CreateBitCast(rpGV, i8PtrTy),
                                   llvm::ConstantInt::get(i64Ty, rowBytes), isVolatile});
    Builder.CreateCall(memcpyDecl, {Builder.CreateBitCast(colPtr, i8PtrTy), Builder.CreateBitCast(ciGV, i8PtrTy),
                                   llvm::ConstantInt::get(i64Ty, colBytes), isVolatile});
    Builder.CreateCall(memcpyDecl, {Builder.CreateBitCast(nodesPtr, i8PtrTy), Builder.CreateBitCast(noGV, i8PtrTy),
                                   llvm::ConstantInt::get(i64Ty, nodesBytes), isVolatile});

    // Store into struct { n, m, row_ptr, col_idx, nodes_orig }
    Builder.CreateStore(llvm::ConstantInt::get(i64Ty, n), Builder.CreateStructGEP(GraphTy, graphAlloca, 0));
    Builder.CreateStore(llvm::ConstantInt::get(i64Ty, m), Builder.CreateStructGEP(GraphTy, graphAlloca, 1));
    Builder.CreateStore(rowPtr, Builder.CreateStructGEP(GraphTy, graphAlloca, 2));
    Builder.CreateStore(colPtr, Builder.CreateStructGEP(GraphTy, graphAlloca, 3));
    Builder.CreateStore(nodesPtr, Builder.CreateStructGEP(GraphTy, graphAlloca, 4));

    GraphMap[G->name] = graphAlloca;
    GraphLayoutMap[G->name] = G->chosenKind;
    return graphAlloca;
}



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
    QueryResultGraphName[Q->queryName] = Q->graphName;
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
    QueryResultGraphName[Q->queryName] = Q->graphName;
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

// void IRGenVisitor::visitPrintArray(PrintArrayNode *node)
// {
//     auto arrIt = NamedValues.find(node->arrayName + "_ptr");
//     auto sizeIt = NamedValues.find(node->arrayName + "_size");

//     if (arrIt == NamedValues.end() || sizeIt == NamedValues.end())
//     {
//         llvm::errs() << "Array not found: " << node->arrayName << "\n";
//         return;
//     }

//     llvm::Value *arrPtr = Builder.CreateLoad(llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(Context)), arrIt->second);
//     llvm::Value *arrSize = Builder.CreateLoad(llvm::Type::getInt32Ty(Context), sizeIt->second);

//     llvm::Function *printfFunc = getPrintfFunction(); // helper function

//     // Create loop variables
//     llvm::Value *index = Builder.CreateAlloca(llvm::Type::getInt32Ty(Context));
//     Builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0), index);

//     llvm::Function *curFunc = Builder.GetInsertBlock()->getParent();
//     llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(Context, "print_loop", curFunc);
//     llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(Context, "after_print", curFunc);
//     Builder.CreateBr(loopBB);

//     Builder.SetInsertPoint(loopBB);
//     llvm::Value *iVal = Builder.CreateLoad(llvm::Type::getInt32Ty(Context), index);
//     llvm::Value *cond = Builder.CreateICmpSLT(iVal, arrSize);
//     Builder.CreateCondBr(cond, loopBB, afterBB);

//     // Load element and call printf
//     llvm::Value *elemPtr = Builder.CreateGEP(
//         llvm::Type::getInt32Ty(Context), arrPtr, {iVal});
//     llvm::Value *elem = Builder.CreateLoad(llvm::Type::getInt32Ty(Context), elemPtr);
//     Builder.CreateCall(printfFunc, {Builder.CreateGlobalStringPtr("%d "), elem});

//     // Increment index
//     llvm::Value *nextI = Builder.CreateAdd(iVal, llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 1));
//     Builder.CreateStore(nextI, index);

//     Builder.CreateBr(loopBB);
//     Builder.SetInsertPoint(afterBB);
//     Builder.CreateCall(printfFunc, {Builder.CreateGlobalStringPtr("\n")});
// }


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
    QueryResultGraphName[Q->queryName] = Q->graphName;

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
    QueryResultGraphName[Q->queryName] = Q->graphName;

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
    QueryResultGraphName[Q->queryName] = Q->graphName;

    // Done
}

void IRGenVisitor::emitDijkstra(QueryNode *Q)
{
    // Grab the Graph* value
    llvm::Value *graphPtr = GraphMap[Q->graphName];
    assert(graphPtr && "Graph not found in IRGenVisitor::emitDijkstra");

    // Declare the runtime function:
    // extern "C" void dijkstra_runtime(struct.Graph* g);
    llvm::Type *voidTy = llvm::Type::getVoidTy(Context);
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();
    llvm::FunctionType *dijkstraFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, /*isVarArg=*/false);

    // getOrInsertFunction will add a declaration if not present.
    auto dijkstraDecl = Module.getOrInsertFunction("dijkstra_runtime", dijkstraFT);

    // Call dijkstra_runtime(graphPtr)
    Builder.CreateCall(dijkstraDecl, {graphPtr});
    QueryResultGraphName[Q->queryName] = Q->graphName;

    // done
    return;
}

void IRGenVisitor::visitQuery(QueryNode *Q)
{
    // 1) Find AST graph decl for query-driven autotune
    GraphDeclNode *GD = nullptr;
    auto itG = GraphDecls.find(Q->graphName);
    if (itG != GraphDecls.end()) GD = itG->second;

    // 2) If we have the GraphDeclNode, autotune with query context and force materialization
    if (GD)
    {
        auto hints = Autotuner.choose(GD->set, /*queryName*/Q->queryName, /*queryDesc*/Q->queryDesc);

        // If layout differs from already materialized layout, rebuild
        const bool needRebuild =
            (!GD->materialized) || (GD->chosenKind != hints.preferred);

        if (needRebuild)
        {
            GD->chosenKind = hints.preferred;
            GD->mat.kind = hints.preferred;

            if (hints.preferred == gpl::GraphLayoutKind::PackedCSR)
            {
                GD->mat.packed = gpl::build_packed_csr(GD->set);
            }
            else if (hints.preferred == gpl::GraphLayoutKind::BlockedCSR)
            {
                GD->mat.blocked = gpl::build_blocked_csr(GD->set, hints.blockedBlockSize);
            }
            else
            {
                throw std::runtime_error("visitQuery: unsupported layout kind");
            }

            GD->materialized = true;

            // IMPORTANT: if we already emitted IR object for this graph, invalidate it so visitGraphDecl rebuilds buffers
            GraphMap.erase(GD->name);
        }

        // Ensure IR graph object exists with the chosen layout
        visitGraphDecl(GD);
    }
    else
    {
        // Fallback: ensure LLVM graph exists somehow
        // (This happens if the graph decl wasn't seen / wasn't registered.)
        auto itIR = GraphMap.find(Q->graphName);
        if (itIR == GraphMap.end())
            llvm::errs() << "[visitQuery] Warning: GraphDecl not found for graph '" << Q->graphName
                         << "'. Autotune disabled.\n";
    }

    // 3) Dispatch actual query runtime call
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
    else
    {
        llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n";
        assert(false && "Unknown query type in QueryNode");
    }
}

// void IRGenVisitor::visitQuery(QueryNode *Q)
// {
//     // Assuming QueryNode has a std::string field named 'queryDesc' for "bfs"/"dfs"
//     if (Q->queryDesc == "bfs")
//     {
//         emitBFS(Q);
//     }
//     else if (Q->queryDesc == "dfs")
//     {
//         emitDFS(Q);
//     }
//     else if (Q->queryDesc == "bk")
//     {
//         emitBK(Q);
//     }
//     else if (Q->queryDesc == "transitive_closure")
//     {
//         emitFloydWarshall(Q);
//     }
//     else if (Q->queryDesc == "chromaticity")
//     {
//         emitChromacity(Q);
//     }
//     else if(Q->queryDesc == "dijkstra")
//     {
//         emitDijkstra(Q);
//     }
//     else
//     {
//         llvm::errs() << "Unsupported query type: " << Q->queryDesc << "\n";
//         assert(false && "Unknown query type in QueryNode");
//     }
// }

void IRGenVisitor::printQueryArrayAsOrigIds(const std::string& queryName, const std::string& graphName)
{
    // Look up stored ptr/size allocas
    auto arrIt = NamedValues.find(queryName + "_ptr");
    auto sizeIt = NamedValues.find(queryName + "_size");
    if (arrIt == NamedValues.end() || sizeIt == NamedValues.end())
        throw std::runtime_error("print: query result not found for '" + queryName + "'");

    auto gIt = GraphMap.find(graphName);
    if (gIt == GraphMap.end() || !gIt->second)
        throw std::runtime_error("print: graph not found for '" + graphName + "'");

    llvm::Value *graphPtr = gIt->second;

    llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);
    llvm::Type *i32PtrTy = i32Ty->getPointerTo();
    llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);

    // Load int* arr from alloca that holds int*
    llvm::Value *arrPtr = Builder.CreateLoad(i32PtrTy, arrIt->second, queryName + ".arr");
    llvm::Value *szVal32 = Builder.CreateLoad(i32Ty, sizeIt->second, queryName + ".size");
    llvm::Value *szVal64 = Builder.CreateZExt(szVal32, i64Ty, queryName + ".size64");

    // Load nodes_orig pointer from graph struct field 4
    llvm::Value *nodesGEP = Builder.CreateStructGEP(GraphTy, graphPtr, 4, "g_nodes_ptr");
    llvm::Value *nodesOrigPtr = Builder.CreateLoad(i32PtrTy, nodesGEP, "nodes_orig");

    llvm::Function *printfFunc = getPrintfFunction();

    // for (i=0; i<size; ++i) print nodes_orig[arr[i]]
    llvm::Function *curFunc = Builder.GetInsertBlock()->getParent();

    llvm::AllocaInst *iAlloca = Builder.CreateAlloca(i64Ty, nullptr, queryName + ".i");
    Builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), iAlloca);

    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(Context, queryName + ".print.cond", curFunc);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(Context, queryName + ".print.body", curFunc);
    llvm::BasicBlock *incBB  = llvm::BasicBlock::Create(Context, queryName + ".print.inc", curFunc);
    llvm::BasicBlock *endBB  = llvm::BasicBlock::Create(Context, queryName + ".print.end", curFunc);

    Builder.CreateBr(condBB);

    // cond
    Builder.SetInsertPoint(condBB);
    llvm::Value *iVal = Builder.CreateLoad(i64Ty, iAlloca, "i");
    llvm::Value *cond = Builder.CreateICmpULT(iVal, szVal64, "i_lt_sz");
    Builder.CreateCondBr(cond, bodyBB, endBB);

    // body
    Builder.SetInsertPoint(bodyBB);

    // dense = arr[i]
    llvm::Value *densePtr = Builder.CreateGEP(i32Ty, arrPtr, {iVal}, "dense_ptr");
    llvm::Value *denseVal = Builder.CreateLoad(i32Ty, densePtr, "dense");

    // orig = nodes_orig[dense]
    llvm::Value *dense64 = Builder.CreateZExt(denseVal, i64Ty, "dense64");
    llvm::Value *origPtr = Builder.CreateGEP(i32Ty, nodesOrigPtr, {dense64}, "orig_ptr");
    llvm::Value *origVal = Builder.CreateLoad(i32Ty, origPtr, "orig");

    // printf("%d ", orig)
    Builder.CreateCall(printfFunc, {Builder.CreateGlobalStringPtr("%d "), origVal});

    Builder.CreateBr(incBB);

    // inc
    Builder.SetInsertPoint(incBB);
    llvm::Value *nextI = Builder.CreateAdd(iVal, llvm::ConstantInt::get(i64Ty, 1), "i_next");
    Builder.CreateStore(nextI, iAlloca);
    Builder.CreateBr(condBB);

    // end
    Builder.SetInsertPoint(endBB);
    Builder.CreateCall(printfFunc, {Builder.CreateGlobalStringPtr("\n")});
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
        const bool isQueryArr =
            (NamedValues.find(var->name + "_ptr") != NamedValues.end()) &&
            (NamedValues.find(var->name + "_size") != NamedValues.end());

        if (isQueryArr)
        {
            auto it = QueryResultGraphName.find(var->name);
            if (it == QueryResultGraphName.end())
                throw std::runtime_error("print: missing query->graph binding for '" + var->name + "'");

            printQueryArrayAsOrigIds(var->name, it->second);
            return;
        }

        // Otherwise print scalar int variable as before
        llvm::Value *alloca = NamedValues[var->name];
        val = Builder.CreateLoad(Builder.getInt32Ty(), alloca, var->name);

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