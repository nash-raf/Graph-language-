/* Device lowering already consumed the accepted original work. These hooks
 * only synchronize host storage. They add no dependence or partition tests. */
static void instrumentGpuHostCoherence(llvm::Module &M) {
    using namespace llvm;
    if(!M.getNamedGlobal("gpu_embedded_ptx")) return;
    auto &C=M.getContext();auto *Ptr=PointerType::get(C,0);
    auto Access=M.getOrInsertFunction("sgpl_gpu_host_access",FunctionType::get(Type::getVoidTy(C),{Ptr,Type::getInt64Ty(C),Type::getInt32Ty(C)},false));
    auto Barrier=M.getOrInsertFunction("sgpl_gpu_host_barrier",FunctionType::get(Type::getVoidTy(C),false));
    auto Begin=M.getOrInsertFunction("sgpl_gpu_residency_begin",FunctionType::get(Type::getVoidTy(C),false));
    auto End=M.getOrInsertFunction("sgpl_gpu_residency_end",FunctionType::get(Type::getVoidTy(C),false));
    auto Forget=M.getOrInsertFunction("sgpl_gpu_unregister_buffer",FunctionType::get(Type::getVoidTy(C),{Ptr},false));
    auto Register=M.getOrInsertFunction("sgpl_gpu_register_resident_buffer",FunctionType::get(Type::getVoidTy(C),{Ptr,Type::getInt64Ty(C)},false));
    auto Refresh=M.getOrInsertFunction("sgpl_gpu_refresh_pointee",FunctionType::get(Type::getVoidTy(C),{Ptr,Ptr},false));
    auto RegisterStack=M.getOrInsertFunction("sgpl_gpu_register_stack_buffer",FunctionType::get(Type::getVoidTy(C),{Ptr,Type::getInt64Ty(C)},false));
    auto RestoreStack=M.getOrInsertFunction("sgpl_gpu_stack_restore",FunctionType::get(Type::getVoidTy(C),{Ptr},false));
    SmallVector<Instruction *,128> Work;
    for(Function &F:M) if(!F.isDeclaration()&&F.getCallingConv()!=CallingConv::PTX_Kernel&&
        !F.hasFnAttribute("sgpl.device.only")&&
        !F.getName().starts_with("sgpl.gpu."))
        for(BasicBlock &BB:F) for(Instruction &I:BB) Work.push_back(&I);
    auto Touch=[&](IRBuilder<> &B,Value *P,Value *Bytes,bool Write) {
        auto *A=dyn_cast<AllocaInst>(getUnderlyingObject(P));
        if((A&&!A->getMetadata("sgpl.language.array"))||isa<ConstantPointerNull>(P)) return;
        B.CreateCall(Access,{P,B.CreateZExtOrTrunc(Bytes,B.getInt64Ty()),B.getInt32(Write)});
    };
    for(Instruction *I:Work) {
        IRBuilder<> B(I);
        auto Sized=[&](Value *P,Type *T,bool W) {
            TypeSize S=M.getDataLayout().getTypeStoreSize(T);
            if(!S.isScalable()) Touch(B,P,B.getInt64(S.getFixedValue()),W);
        };
        if(auto *L=dyn_cast<LoadInst>(I)) Sized(L->getPointerOperand(),L->getType(),false);
        else if(auto *S=dyn_cast<StoreInst>(I)) {
            Sized(S->getPointerOperand(),S->getValueOperand()->getType(),true);
            if(auto *G=dyn_cast<GlobalVariable>(S->getPointerOperand()->stripPointerCasts()))
                if(G->getMetadata("sgpl.language.array")&&G->getValueType()->isPointerTy()) {
                    IRBuilder<> R(S->getNextNode());R.CreateCall(Refresh,{R.CreateGlobalStringPtr(G->getName()),S->getValueOperand()});
                }
        }
        else if(auto *A=dyn_cast<AtomicRMWInst>(I)) Sized(A->getPointerOperand(),A->getValOperand()->getType(),true);
        else if(auto *A=dyn_cast<AtomicCmpXchgInst>(I)) Sized(A->getPointerOperand(),A->getCompareOperand()->getType(),true);
        else if(auto *T=dyn_cast<MemTransferInst>(I)) {Touch(B,T->getSource(),T->getLength(),false);Touch(B,T->getDest(),T->getLength(),true);}
        else if(auto *S=dyn_cast<MemSetInst>(I)) Touch(B,S->getDest(),S->getLength(),true);
        else if(auto *Call=dyn_cast<CallBase>(I)) {
            Function *F=Call->getCalledFunction();
            if(F&&!F->isDeclaration()) continue;
            if(F&&F->isIntrinsic()) {
                if(F->getIntrinsicID()==Intrinsic::lifetime_end) {B.CreateCall(Forget,{Call->getArgOperand(1)});continue;}
                if(F->getIntrinsicID()==Intrinsic::stacksave) continue;
                if(F->getIntrinsicID()==Intrinsic::stackrestore) {B.CreateCall(RestoreStack,{Call->getArgOperand(0)});continue;}
                /* Masked and vector-predicated accesses can hide memory
                 * operations inside intrinsics. Unknown ones synchronize. */
                if(Call->mayReadOrWriteMemory()&&F->getIntrinsicID()!=Intrinsic::lifetime_start&&
                   F->getIntrinsicID()!=Intrinsic::lifetime_end) B.CreateCall(Barrier);
                continue;
            }
            StringRef N=F?F->getName():StringRef();
            if(N=="free") {B.CreateCall(Forget,{Call->getArgOperand(0)});continue;}
            if(N=="malloc"||N=="calloc"||N.starts_with("sgpl_gpu_register_")||
               N=="sgpl_exec_spatial_gpu"||N=="sgpl_exec_temporal_gpu_components"||N=="sgpl_exec_dag_round_spatial_gpu"||
               N.starts_with("autograph_exec_ctx_")||N.starts_with("autograph_op_")||
               N.starts_with("autograph_exec_op_")||N=="autograph_clean_cut_partition_count"||
               N=="sgpl_exec_require_status"||N=="autograph_profile_region_enter"||N=="autograph_profile_region_exit"||
               N=="autograph_build_clean_cut"||N=="autograph_gpu_clean_cut_partitions") continue;
            B.CreateCall(Barrier);
        }
    }
    if(Function *Main=M.getFunction("main")) if(!Main->isDeclaration()) {
        IRBuilder<> B(&*Main->getEntryBlock().getFirstInsertionPt());B.CreateCall(Begin);
        for(GlobalVariable &G:M.globals()) if(G.getMetadata("sgpl.language.array")&&G.getValueType()->isArrayTy())
            B.CreateCall(Register,{&G,B.getInt64(M.getDataLayout().getTypeAllocSize(G.getValueType()).getFixedValue())});
        for(Instruction *I:Work) if(auto *A=dyn_cast<AllocaInst>(I)) if(A->getFunction()==Main&&A->getMetadata("sgpl.language.array")) {
            IRBuilder<> R(A->getNextNode());
            Value *Bytes=R.CreateMul(R.CreateZExtOrTrunc(A->getArraySize(),R.getInt64Ty()),R.getInt64(M.getDataLayout().getTypeAllocSize(A->getAllocatedType()).getFixedValue()));
            R.CreateCall(RegisterStack,{A,Bytes});
        }
        for(BasicBlock &BB:*Main) if(auto *R=dyn_cast<ReturnInst>(BB.getTerminator())) {IRBuilder<> E(R);E.CreateCall(End);}
    }
}
