void IRGenVisitor::visitQuery(QueryNode *Q)
{
    // 1) Grab the Graph* value
    llvm::Value *graphPtr = GraphMap[Q->graphName];
    assert(graphPtr && "Graph not found in IRGenVisitor::visitQuery");

    // 2) Extract n, row_ptr*, col_idx*
    llvm::Value *n_ptr = Builder.CreateStructGEP(GraphTy, graphPtr, 0, "g_n_ptr");
    llvm::Value *rp_ptr = Builder.CreateStructGEP(GraphTy, graphPtr, 2, "g_rp_ptr");
    llvm::Value *ci_ptr = Builder.CreateStructGEP(GraphTy, graphPtr, 3, "g_ci_ptr");

    llvm::Function *printfFn = Module.getFunction("printf");
    if (!printfFn)
    {
        // char*
        auto *i8p = llvm::PointerType::getUnqual(Builder.getInt8Ty());
        // int printf(char*, ...)
        auto *printfTy = llvm::FunctionType::get(
            Builder.getInt32Ty(), {i8p}, /*isVarArg=*/true);
        printfFn = llvm::Function::Create(
            printfTy,
            llvm::Function::ExternalLinkage,
            "printf",
            &Module);
    }

    static llvm::GlobalVariable *fmt64 = nullptr,
                                *fmtInt = nullptr,
                                *fmtNL = nullptr;
    if (!fmt64)
    {
        auto *arrTy = llvm::ArrayType::get(Builder.getInt8Ty(), 6); // "%lld\n\0"
        fmt64 = new llvm::GlobalVariable(
            Module, arrTy, /*isConstant=*/true,
            llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantDataArray::getString(Context, "%lld\n", true),
            ".fmt64");
    }
    if (!fmtInt)
    {
        auto *arrTy = llvm::ArrayType::get(Builder.getInt8Ty(), 4); // "%d \0"
        fmtInt = new llvm::GlobalVariable(
            Module, arrTy, /*isConstant=*/true,
            llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantDataArray::getString(Context, "%d ", true),
            ".fmt_int");
    }
    if (!fmtNL)
    {
        auto *arrTy = llvm::ArrayType::get(Builder.getInt8Ty(), 2); // "\n\0"
        fmtNL = new llvm::GlobalVariable(
            Module, arrTy, /*isConstant=*/true,
            llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantDataArray::getString(Context, "\n", true),
            ".fmt_nl");
    }

    // –– Compute the i8* bitcast helper once
    llvm::Type *i8Ty = llvm::Type::getInt8Ty(Context);
    llvm::Type *i64Ty = llvm::Type::getInt64Ty(Context);
    llvm::Type *i8PtrTy = llvm::PointerType::getUnqual(i8Ty);

    llvm::Value *n_val = Builder.CreateLoad(Builder.getInt64Ty(), n_ptr, "n_val");

    llvm::Value *fmt64Ptr = Builder.CreateBitCast(
        fmt64,   // the global [6 x i8] "%lld\n"
        i8PtrTy, // cast to i8*
        "fmt64Ptr");
    Builder.CreateCall(printfFn, {fmt64Ptr, n_val});

    llvm::Value *row_ptr = Builder.CreateLoad(
        llvm::PointerType::getUnqual(Builder.getInt64Ty()),
        rp_ptr, "row_ptr");
    llvm::Value *col_idx = Builder.CreateLoad(
        llvm::PointerType::getUnqual(Builder.getInt32Ty()),
        ci_ptr, "col_idx");

    // 3) Get malloc/free

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
    llvm::Value *fmtIntPtr = Builder.CreateBitCast(
        fmtInt,  // your GlobalVariable for "%d "
        i8PtrTy, // i8*
        "fmt_int_ptr");
    // call printf("%d ", u);
    Builder.CreateCall(printfFn, {fmtIntPtr, u});
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
