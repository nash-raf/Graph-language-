#include "IRGenVisitor.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>

void IRGenVisitor::visitProgram(ProgramNodePtr prog)
{

    for (auto &node : prog->topLevel) {
        if (node->type == ASTNodeType::FunctionDecl) {
            auto *FD = static_cast<FunctionDeclNode*>(node.get());

            // Collect parameter types (all i32 for now)
            std::vector<llvm::Type*> paramTys;
            for (size_t i = 0; i < FD->parameters.size(); ++i) {
                paramTys.push_back(Builder.getInt32Ty());
            }

            // Determine return type
            llvm::Type *retTy;
            if (FD->returnType == "void") {
                retTy = llvm::Type::getVoidTy(Context);
            } else {
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

    //Create: int main()
    auto *intTy = Builder.getInt32Ty();
    auto *mainFT = llvm::FunctionType::get(intTy, /*isVarArg=*/false);
    auto *mainF = llvm::Function::Create(
        mainFT,
        llvm::Function::ExternalLinkage,
        "main",
        &Module);

    //Entry block
    auto *BB = llvm::BasicBlock::Create(Context, "entry", mainF);
    Builder.SetInsertPoint(BB);

    //Lower every top‑level AST node
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
        case ASTNodeType::WhileStmt:
            visitWhile(static_cast<WhileStmtNode *>(node.get()));
            break;
        default:
            // ignore or handle other kinds
            break;
        }
    }

    //Return 0
    Builder.CreateRet(llvm::ConstantInt::get(intTy, 0));

    llvm::verifyFunction(*mainF, &llvm::errs());
    for (auto &node : prog->topLevel) {
        if (node->type == ASTNodeType::FunctionDecl) {
            visitFunctionDecl(static_cast<FunctionDeclNode*>(node.get()));
        }
    }
}

void IRGenVisitor::visitFunctionDecl(FunctionDeclNode *funcDecl) {
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
    for (auto &arg : function->args()) {
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
    auto *bodyBlock = static_cast<BlockStmtNode*>(funcDecl->body.get());
    for (auto &stmt : bodyBlock->statements) {
        visitStatement(stmt.get());
    }

    // 6) If no explicit return (and non-void), insert a default
    if (!Builder.GetInsertBlock()->getTerminator()) {
        if (funcDecl->returnType == "void") {
            Builder.CreateRetVoid();
        } else {
            Builder.CreateRet(
  llvm::ConstantInt::get(
    Builder.getInt32Ty(),  // <-- no Context here
    0
  )
);
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

llvm::AllocaInst *IRGenVisitor::visitVarDecl(VarDeclNode *decl) {
    llvm::Function *currentFunction = Builder.GetInsertBlock()->getParent();

    // --- 1) Handle array-initializers first ---
    if (decl->initializer
        && decl->initializer->type == ASTNodeType::ArrayLiteral)
    {
        auto *arrLit = static_cast<ArrayLiteralNode*>(decl->initializer.get());
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
        for (size_t i = 0; i < N; ++i) {
            llvm::Value *val = visitExpr(arrLit->elements[i].get());
            llvm::Value *gep = Builder.CreateGEP(
                arrTy, arrAlloca,
                {
                  Builder.getInt32(0),
                  Builder.getInt32(static_cast<int>(i))
                },
                decl->name + "_elem");
            Builder.CreateStore(val, gep);
        }

        NamedValues[decl->name] = arrAlloca;
        return arrAlloca;
    }

    // --- 2) Fallback scalar path ---
    llvm::AllocaInst *scalarAlloca =
        createEntryBlockAlloca(currentFunction, decl->name);

    if (decl->initializer) {
        llvm::Value *initVal = visitExpr(decl->initializer.get());
        Builder.CreateStore(initVal, scalarAlloca);
    } else {
        Builder.CreateStore(llvm::ConstantInt::get(
                                Builder.getInt32Ty(), 0),
                            scalarAlloca);
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
    case ASTNodeType::FunctionCall: {
        auto *FC = static_cast<FunctionCallNode*>(expr);
        // Lookup the prototype
        llvm::Function *callee = FunctionProtos[FC->name];
        if (!callee)
            throw std::runtime_error("Unknown function: " + FC->name);

        // Evaluate arguments
        std::vector<llvm::Value*> args;
        for (auto &argNode : FC->arguments) {
            args.push_back(visitExpr(argNode.get()));
        }

        return Builder.CreateCall(callee, args, "calltmp");
    }

    case ASTNodeType::ArrayAccess: {
        auto *A = static_cast<ArrayAccessNode*>(expr);
        // 1) find the alloca for the base array
        auto it = NamedValues.find( static_cast<VariableNode*>(A->arrayExpr.get())->name );
        if (it == NamedValues.end())
          throw std::runtime_error("Undefined array in access");
        llvm::AllocaInst *baseAlloca = it->second;

        // 2) compute the index
        llvm::Value *idxVal = visitExpr(A->indexExpr.get());

        // 3) build GEP: first index=0 (the array), then index=idxVal
        llvm::ArrayType *arrTy = llvm::cast<llvm::ArrayType>( baseAlloca->getAllocatedType() );
        llvm::Value *elemPtr = Builder.CreateGEP(
            arrTy,
            baseAlloca,
            { Builder.getInt32(0), idxVal },
            "elemptr"
        );

        // 4) load the i32 element
auto *elemTy = llvm::cast<llvm::GetElementPtrInst>(elemPtr)->getSourceElementType();
return Builder.CreateLoad(elemTy, elemPtr, "loadelem");      }
    default:
        throw std::runtime_error("Unsupported expression type in IRGenVisitor");
    }
}
