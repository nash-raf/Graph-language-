#include "IRGenVisitor.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>
#include "roaring_bitmap.h"
#include <chrono>

llvm::Type *getBitmapPtrTy(llvm::LLVMContext &Context)
{
    return llvm::PointerType::get(Context, 0);
}

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

    auto *BitmapPtrTy = llvm::PointerType::get(Context, 0);
    auto *BitmapPtrPtrTy = BitmapPtrTy->getPointerTo();
    auto *i64Ty = Builder.getInt64Ty();
    auto *i32Ty = Builder.getInt32Ty();
    auto *voidTy = Builder.getVoidTy();

    // Declare all Roaring functions early with consistent typed i8* signatures
    auto createFT = llvm::FunctionType::get(BitmapPtrTy, {i64Ty, i64Ty}, false);
    Module.getOrInsertFunction("roaring_bitmap_create", createFT);

    auto addFT = llvm::FunctionType::get(voidTy, {BitmapPtrTy, i32Ty}, false);
    Module.getOrInsertFunction("roaring_bitmap_add", addFT);

    auto unionFT = llvm::FunctionType::get(BitmapPtrTy, {BitmapPtrPtrTy, i64Ty}, false);
    Module.getOrInsertFunction("roaring_bitmap_union", unionFT);

    // Intersect: takes two bitmaps
    auto intersectFT = llvm::FunctionType::get(BitmapPtrTy, {BitmapPtrTy, BitmapPtrTy}, false);
    Module.getOrInsertFunction("roaring_bitmap_intersect", intersectFT);

    auto printFT = llvm::FunctionType::get(voidTy, {BitmapPtrTy}, false);
    Module.getOrInsertFunction("roaring_print", printFT);

    auto removeFT = llvm::FunctionType::get(voidTy, {BitmapPtrTy, i32Ty}, false);
    Module.getOrInsertFunction("roaring_bitmap_remove", removeFT);

    auto containsFT = llvm::FunctionType::get(i32Ty, {BitmapPtrTy, i32Ty}, false);
    Module.getOrInsertFunction("roaring_bitmap_contains", containsFT);

    auto getAtIndexFT = llvm::FunctionType::get(i32Ty, {BitmapPtrTy, i32Ty}, false);
    Module.getOrInsertFunction("roaring_bitmap_get_at_index", getAtIndexFT);

    auto getCardinalityFT = llvm::FunctionType::get(i64Ty, {BitmapPtrTy}, false);
    Module.getOrInsertFunction("roaring_bitmap_get_cardinality", getCardinalityFT);

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
        case ASTNodeType::SetDecl:
            visitSetDecl(static_cast<SetDeclNode *>(node.get()));
            break;
        case ASTNodeType::SetMethodCall:
            visitSetMethodCall(static_cast<SetMethodCallNode *>(node.get()));
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
        // std::cerr << "Entered queryNODe\n";
        visitQuery(static_cast<QueryNode *>(node));
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
    default:
        // std::cerr << "Unsupported statement type.\n";
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

    case ASTNodeType::SetContainsExpr:
    {
        auto *containsNode = static_cast<SetContainsExprNode *>(expr);
        return visitSetContainsExpr(containsNode);
    }

    case ASTNodeType::ArrayAccess:
    {
        auto *A = static_cast<ArrayAccessNode *>(expr);
        auto *baseVar = dynamic_cast<VariableNode *>(A->arrayExpr.get());
        if (!baseVar)
            throw std::runtime_error("Array/Set base must be variable");

        auto it = NamedValues.find(baseVar->name);
        if (it == NamedValues.end())
            throw std::runtime_error("Undefined array/set in access: " + baseVar->name);
        llvm::AllocaInst *baseAlloca = it->second;

        // Check if this is a set (pointer type to i8/bitmap)
        llvm::Type *allocatedType = baseAlloca->getAllocatedType();
        if (allocatedType->isPointerTy())
        {
            // This might be a set - check if it's a bitmap pointer
            auto *BitmapPtrTy = getBitmapPtrTy(Context);
            if (allocatedType == BitmapPtrTy)
            {
                // This is set access: s[index]
                llvm::Value *bitmapPtr = Builder.CreateLoad(BitmapPtrTy, baseAlloca, baseVar->name + ".load");

                llvm::Value *idxVal = visitExpr(A->indexExpr.get());
                if (idxVal->getType() != Builder.getInt32Ty())
                    idxVal = Builder.CreateIntCast(idxVal, Builder.getInt32Ty(), true);

                // Call roaring_bitmap_get_at_index(bitmap*, i32) -> i32
                llvm::FunctionType *getAtIndexFT = llvm::FunctionType::get(
                    Builder.getInt32Ty(),
                    {BitmapPtrTy, Builder.getInt32Ty()},
                    false);

                auto getAtIndexFn = Module.getOrInsertFunction("roaring_bitmap_get_at_index", getAtIndexFT);
                return Builder.CreateCall(getAtIndexFn, {bitmapPtr, idxVal}, "set.get");
            }
        }

        // Otherwise, handle as regular array access (existing code)
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
            llvm::Type *baseTy = baseAlloca->getAllocatedType();
            if (auto *ptrTy = llvm::dyn_cast<llvm::PointerType>(baseTy))
            {
                llvm::Type *elemTy = ptrTy->getContainedType(0);
                llvm::Value *elemPtr = Builder.CreateGEP(
                    elemTy,
                    baseAlloca,
                    {idxVal},
                    baseVar->name + "_elemptr");
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

    return graphPtr;
}

llvm::Value *zero64(llvm::IRBuilder<> &B, llvm::LLVMContext &C)
{
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(C), 0);
}

void IRGenVisitor::visitQuery(QueryNode *Q)
{
    // Grab the Graph* value
    llvm::Value *graphPtr = GraphMap[Q->graphName];
    assert(graphPtr && "Graph not found in IRGenVisitor::visitQuery");

    // Declare the runtime function:
    // extern "C" void bfs_runtime(struct.Graph* g);
    llvm::Type *voidTy = llvm::Type::getVoidTy(Context);
    llvm::Type *graphPtrTy = GraphTy->getPointerTo();
    llvm::FunctionType *bfsFT = llvm::FunctionType::get(voidTy, {graphPtrTy}, /*isVarArg=*/false);

    // getOrInsertFunction will add a declaration if not present.
    auto bfsDecl = Module.getOrInsertFunction("bfs_runtime", bfsFT);

    // Call bfs_runtime(graphPtr)
    Builder.CreateCall(bfsDecl, {graphPtr});

    return;
}

// void IRGenVisitor::visitPrintStmt(PrintStmtNode *PS)
// {
//     // 1) Ensure printf is declared
//     auto *i8Ty = llvm::Type::getInt8Ty(Context);
//     auto *i8PtrTy = llvm::PointerType::getUnqual(i8Ty);

//     llvm::Function *printfFn = Module.getFunction("printf");
//     if (!printfFn)
//     {
//         // int printf(char*, ...)
//         auto *i8p = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context));
//         auto *printfTy = llvm::FunctionType::get(
//             /*Ret=*/Builder.getInt32Ty(),
//             /*Params=*/{i8p},
//             /*isVarArg=*/true);
//         printfFn = llvm::Function::Create(
//             printfTy,
//             llvm::Function::ExternalLinkage,
//             "printf",
//             &Module);
//     }

//     // 2) Prepare format string
//     llvm::Value *fmtPtr = nullptr;
//     llvm::Value *val = nullptr;

//     if (auto *lit = dynamic_cast<IntLiteralNode *>(PS->expr.get()))
//     {
//         // integer literal: we’ll just embed the value
//         val = llvm::ConstantInt::get(Builder.getInt32Ty(), lit->value);
//         // constant global: "%d\n\0"
//         static llvm::GlobalVariable *fmtInt = nullptr;
//         if (!fmtInt)
//         {
//             auto *fmtTy = llvm::ArrayType::get(Builder.getInt8Ty(), 4);
//             fmtInt = new llvm::GlobalVariable(
//                 Module, fmtTy, /*isConstant=*/true,
//                 llvm::GlobalValue::PrivateLinkage,
//                 llvm::ConstantDataArray::getString(Context, "%d\n", true),
//                 ".fmt_int");
//         }
//         fmtPtr = Builder.CreateBitCast(fmtInt,
//                                        llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     }
//     else if (auto *var = dynamic_cast<VariableNode *>(PS->expr.get()))
//     {
//         // assume it's an int variable
//         llvm::AllocaInst *alloca = NamedValues[var->name];
//         llvm::Type *allocTy = alloca->getAllocatedType();

//         if (auto *ptrTy = llvm::dyn_cast<llvm::PointerType>(allocTy))
//         {
//             llvm::Type *elemTy = ptrTy->getContainedType(0);
//             if (elemTy->isIntegerTy(8))
//             {
//                 // This is a set variable (i8*)
//                 llvm::Value *ptr = Builder.CreateLoad(i8PtrTy, alloca, var->name + ".ptr");
//                 llvm::FunctionType *printFT = llvm::FunctionType::get(Builder.getVoidTy(), {i8PtrTy}, false);
//                 auto printFn = Module.getOrInsertFunction("roaring_print", printFT);
//                 Builder.CreateCall(printFn, {ptr});
//                 return;
//             }
//         }
//         val = Builder.CreateLoad(Builder.getInt32Ty(), alloca, var->name);
//         // same fmtInt as above
//         static llvm::GlobalVariable *fmtInt = nullptr;
//         if (!fmtInt)
//         {
//             auto *fmtTy = llvm::ArrayType::get(Builder.getInt8Ty(), 4);
//             fmtInt = new llvm::GlobalVariable(
//                 Module, fmtTy, true,
//                 llvm::GlobalValue::PrivateLinkage,
//                 llvm::ConstantDataArray::getString(Context, "%d\n", true),
//                 ".fmt_int");
//         }

//         fmtPtr = Builder.CreateBitCast(fmtInt,
//                                        llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Context)));
//     }
//     else
//     {
//         // for string literals, extend similarly:
//         // extract the const char* from your AST’s StringLiteralNode...
//         // use "%s\n" format.
//         throw std::runtime_error("print: only int literals & vars supported so far");
//     }

//     // 3) Call printf(fmtPtr, val)
//     Builder.CreateCall(printfFn, {fmtPtr, val});
// }

void IRGenVisitor::visitPrintStmt(PrintStmtNode *node)
{
    ASTNode *exprNode = node->expr.get();

    // Handle variable print (int or set)
    if (exprNode->type == ASTNodeType::Variable)
    {
        auto *var = static_cast<VariableNode *>(exprNode);
        auto it = NamedValues.find(var->name);
        if (it == NamedValues.end())
        {
            throw std::runtime_error("Undefined variable in print: " + var->name);
        }

        llvm::AllocaInst *alloca = it->second;
        llvm::Type *allocatedTy = alloca->getAllocatedType();

        if (llvm::isa<llvm::PointerType>(allocatedTy))
        {
            // Assume this is a set (bitmap pointer)
            auto *BitmapPtrTy = getBitmapPtrTy(Context);
            llvm::Value *ptr = Builder.CreateLoad(BitmapPtrTy, alloca, var->name + ".bitmap");

            llvm::FunctionType *printFT = llvm::FunctionType::get(
                Builder.getVoidTy(),
                {BitmapPtrTy},
                false);

            auto printFn = Module.getOrInsertFunction("roaring_print", printFT);
            Builder.CreateCall(printFn, {ptr});
            return;
        }

        // Fall through to integer print if not a set
        llvm::Value *val = Builder.CreateLoad(Builder.getInt32Ty(), alloca, var->name);
        llvm::Function *printfFn = Module.getFunction("printf");
        if (!printfFn)
        {
            llvm::FunctionType *printfTy = llvm::FunctionType::get(
                Builder.getInt32Ty(),
                {llvm::PointerType::get(Context, 0)},
                true);
            printfFn = llvm::Function::Create(printfTy, llvm::Function::ExternalLinkage, "printf", Module);
        }

        llvm::Value *strPtr = Builder.CreateGlobalStringPtr("%d\n");
        Builder.CreateCall(printfFn, {strPtr, val});
        return;
    }

    // Add other print cases here if needed (e.g., string literals, expressions)
    // For now, only variable print is supported
    throw std::runtime_error("print only supports variables (int or set) for now");
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
    // CRITICAL: Must allocate ptr type, not i32!
    llvm::IRBuilder<> tmpB(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    llvm::AllocaInst *bitmapAlloca = tmpB.CreateAlloca(i8PtrTy, nullptr, baseName);
    NamedValues[baseName] = bitmapAlloca;

    // Check if initializer is a set expression (for cases like: set s3 = s1 union s2;)
    if (setDecl->initializer && setDecl->initializer->type == ASTNodeType::SetBinaryExpr)
    {
        // Evaluate the set expression at runtime
        llvm::Value *bitmapPtr = visitSetExpr(setDecl->initializer.get());

        // Store only into local alloca (no global!)
        Builder.CreateStore(bitmapPtr, bitmapAlloca);
        return;
    }

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

    llvm::AllocaInst *targetAlloca = it->second;

    // Evaluate the set expression to get the resulting bitmap pointer
    llvm::Value *resultBitmap = visitSetExpr(setOp->expr.get());

    // Store the result ONLY into the local alloca (no global update!)
    Builder.CreateStore(resultBitmap, targetAlloca);
}

llvm::Value *IRGenVisitor::visitSetExpr(ASTNode *expr)
{
    switch (expr->type)
    {
    case ASTNodeType::SetBinaryExpr:
    {
        return visitSetBinaryExpr(static_cast<SetBinaryExprNode *>(expr));
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

        std::cerr << "[IRGen] Union expression with "
                  << operands.size() << " operands\n";

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
            {BitmapPtrTy->getPointerTo(), i64Ty},
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
            if (operands[i]->type != ASTNodeType::Variable)
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

        if (binExpr->lhs->type != ASTNodeType::Variable)
        {
            Builder.CreateCall(freeFn, {lhsBitmap});
        }

        if (binExpr->rhs->type != ASTNodeType::Variable)
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

    // Get the set variable
    auto it = NamedValues.find(node->setName);
    if (it == NamedValues.end())
    {
        throw std::runtime_error("Undefined set variable: " + node->setName);
    }

    llvm::AllocaInst *setAlloca = it->second;
    llvm::Value *bitmapPtr = Builder.CreateLoad(BitmapPtrTy, setAlloca, node->setName + ".load");

    // Evaluate the argument
    llvm::Value *argValue = visitExpr(node->argument.get());
    if (argValue->getType() != i32Ty)
    {
        argValue = Builder.CreateIntCast(argValue, i32Ty, true);
    }

    // ADD DEBUG: Print what we're doing
    llvm::Function *printfFn = Module.getFunction("printf");
    if (!printfFn)
    {
        llvm::FunctionType *printfTy = llvm::FunctionType::get(
            Builder.getInt32Ty(),
            {llvm::PointerType::get(Context, 0)},
            true);
        printfFn = llvm::Function::Create(printfTy, llvm::Function::ExternalLinkage, "printf", Module);
    }

    if (node->methodName == "add")
    {
        // Debug output
        llvm::Value *debugStr = Builder.CreateGlobalStringPtr("[DEBUG] Calling roaring_bitmap_add(%p, %d)\n");
        Builder.CreateCall(printfFn, {debugStr, bitmapPtr, argValue});

        // Call roaring_bitmap_add(bitmap*, i32)
        llvm::FunctionType *addFT = llvm::FunctionType::get(
            voidTy,
            {BitmapPtrTy, i32Ty},
            false);

        auto addFn = Module.getOrInsertFunction("roaring_bitmap_add", addFT);
        Builder.CreateCall(addFn, {bitmapPtr, argValue});

        // Debug: print after add
        llvm::Value *afterStr = Builder.CreateGlobalStringPtr("[DEBUG] After add, printing bitmap:\n");
        Builder.CreateCall(printfFn, {afterStr});

        llvm::FunctionType *printFT = llvm::FunctionType::get(
            Builder.getVoidTy(),
            {BitmapPtrTy},
            false);
        auto printFn = Module.getOrInsertFunction("roaring_print", printFT);
        Builder.CreateCall(printFn, {bitmapPtr});
    }
    else if (node->methodName == "remove")
    {
        // Debug output
        llvm::Value *debugStr = Builder.CreateGlobalStringPtr("[DEBUG] Calling roaring_bitmap_remove(%p, %d)\n");
        Builder.CreateCall(printfFn, {debugStr, bitmapPtr, argValue});

        // Call roaring_bitmap_remove(bitmap*, i32)
        llvm::FunctionType *removeFT = llvm::FunctionType::get(
            voidTy,
            {BitmapPtrTy, i32Ty},
            false);

        auto removeFn = Module.getOrInsertFunction("roaring_bitmap_remove", removeFT);
        Builder.CreateCall(removeFn, {bitmapPtr, argValue});

        // Debug: print after remove
        llvm::Value *afterStr = Builder.CreateGlobalStringPtr("[DEBUG] After remove, printing bitmap:\n");
        Builder.CreateCall(printfFn, {afterStr});

        llvm::FunctionType *printFT = llvm::FunctionType::get(
            Builder.getVoidTy(),
            {BitmapPtrTy},
            false);
        auto printFn = Module.getOrInsertFunction("roaring_print", printFT);
        Builder.CreateCall(printFn, {bitmapPtr});
    }
}

llvm::Value *IRGenVisitor::visitSetContainsExpr(SetContainsExprNode *node)
{
    auto *BitmapPtrTy = getBitmapPtrTy(Context);
    auto *i32Ty = Builder.getInt32Ty();

    // Get the set variable
    auto it = NamedValues.find(node->setName);
    if (it == NamedValues.end())
    {
        throw std::runtime_error("Undefined set variable: " + node->setName);
    }

    llvm::AllocaInst *setAlloca = it->second;
    llvm::Value *bitmapPtr = Builder.CreateLoad(BitmapPtrTy, setAlloca, node->setName + ".load");

    // Evaluate the argument
    llvm::Value *argValue = visitExpr(node->argument.get());
    if (argValue->getType() != i32Ty)
    {
        argValue = Builder.CreateIntCast(argValue, i32Ty, true);
    }

    // Call roaring_bitmap_contains(bitmap*, i32) -> i32
    llvm::FunctionType *containsFT = llvm::FunctionType::get(
        i32Ty,
        {BitmapPtrTy, i32Ty},
        false);

    auto containsFn = Module.getOrInsertFunction("roaring_bitmap_contains", containsFT);
    llvm::Value *result = Builder.CreateCall(containsFn, {bitmapPtr, argValue}, "contains.result");

    // Convert to i1 (boolean) for conditional use
    return Builder.CreateICmpNE(result, llvm::ConstantInt::get(i32Ty, 0), "contains.bool");
}
