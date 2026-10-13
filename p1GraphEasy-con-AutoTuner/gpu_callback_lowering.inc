/* Device capability and lowering only. Existing proof results are immutable. */
struct GpuCallback {
    Function *Kernel=nullptr;
    SmallVector<GlobalVariable *,16> Globals;
    SmallVector<GlobalVariable *,16> ReadOnlyGlobals;
    std::string Reason;
    bool ConcurrentPrivate=false;
};
static Value *gpuByteAddress(IRBuilder<> &B,Value *P,uint64_t Offset) {
    return B.CreateGEP(B.getInt8Ty(),P,B.getInt64(Offset));
}
static Function *gpuSpatialComponent(Module *M,Function *Pair,
    ArrayRef<std::pair<unsigned,Function *>> Begins,ArrayRef<std::pair<unsigned,Function *>> Ends,
    unsigned PairIndex,uint64_t ContextOffset) {
    auto &C=M->getContext();Type *Ptr=PointerType::get(C,0),*I32=Type::getInt32Ty(C),*I64=Type::getInt64Ty(C);
    Function *F=Function::Create(FunctionType::get(I32,{Ptr,I64},false),GlobalValue::InternalLinkage,"sgpl.gpu.spatial.group",M);
    IRBuilder<> B(BasicBlock::Create(C,"entry",F));
    Value *Shared=B.CreateLoad(Ptr,gpuByteAddress(B,F->getArg(0),ContextOffset));
    auto *Local=B.CreateAlloca(B.getInt8Ty(),B.getInt64(sizeof(sgpl_exec_ctx)));Local->setAlignment(Align(8));
    B.CreateMemCpy(Local,MaybeAlign(8),Shared,MaybeAlign(8),sizeof(sgpl_exec_ctx));
    auto Load=[&](Type *T,uint64_t Offset){return B.CreateLoad(T,gpuByteAddress(B,Local,Offset));};
    Value *SourceOwned=B.CreateICmpEQ(Load(I32,offsetof(sgpl_exec_ctx,traversal_kind)),B.getInt32(SGPL_TRAVERSE_OWNER_U));
    B.CreateStore(B.CreateSelect(SourceOwned,ConstantPointerNull::get(cast<PointerType>(Ptr)),Load(Ptr,offsetof(sgpl_exec_ctx,source_state))),gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,source_state)));
    Value *Partition=B.CreateAnd(F->getArg(1),B.getInt64(UINT32_MAX)),*P32=B.CreateTrunc(Partition,I32);
    B.CreateStore(P32,gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,spatial_partition)));
    Value *Part=Load(Ptr,offsetof(sgpl_exec_ctx,partition_base));
    Value *Stride=Load(I64,offsetof(sgpl_exec_ctx,partition_stride));
    Value *SharedPartial=B.CreateGEP(B.getInt8Ty(),Part,B.CreateMul(Partition,Stride));
    auto *PrivatePartial=B.CreateAlloca(B.getInt8Ty(),B.getInt64(512));PrivatePartial->setAlignment(Align(8));
    Value *OwnPartial=B.CreateICmpNE(Load(I32,offsetof(sgpl_exec_ctx,gpu_local_partials)),B.getInt32(0));
    B.CreateStore(B.CreateSelect(OwnPartial,PrivatePartial,SharedPartial),gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,partition_state)));
    BasicBlock *PartialEntry=B.GetInsertBlock(),*PartialCheck=BasicBlock::Create(C,"partial.init.check",F),
        *PartialCopy=BasicBlock::Create(C,"partial.init.copy",F),*PartialDone=BasicBlock::Create(C,"partial.init.done",F);
    B.CreateBr(PartialCheck);B.SetInsertPoint(PartialCheck);PHINode *Byte=B.CreatePHI(I64,2);Byte->addIncoming(B.getInt64(0),PartialEntry);
    B.CreateCondBr(B.CreateAnd(OwnPartial,B.CreateICmpULT(Byte,Stride)),PartialCopy,PartialDone);
    B.SetInsertPoint(PartialCopy);B.CreateStore(B.CreateLoad(B.getInt8Ty(),B.CreateGEP(B.getInt8Ty(),SharedPartial,Byte)),B.CreateGEP(B.getInt8Ty(),PrivatePartial,Byte));
    Byte->addIncoming(B.CreateAdd(Byte,B.getInt64(1)),PartialCopy);B.CreateBr(PartialCheck);B.SetInsertPoint(PartialDone);
    Value *LaneSeen=Load(Ptr,offsetof(sgpl_exec_ctx,spatial_lane_seen));
    Value *N=Load(I64,offsetof(sgpl_exec_ctx,gpu_vertex_count));
    Value *Seen=B.CreateSelect(B.CreateICmpNE(LaneSeen,ConstantPointerNull::get(cast<PointerType>(Ptr))),
        B.CreateGEP(I32,LaneSeen,B.CreateMul(Partition,N)),Load(Ptr,offsetof(sgpl_exec_ctx,dest_seen)));
    B.CreateStore(Seen,gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,dest_seen)));
    Value *Records=Load(Ptr,offsetof(sgpl_exec_ctx,gpu_records)),*Offsets=Load(Ptr,offsetof(sgpl_exec_ctx,gpu_record_offsets));
    Value *Parallel=B.CreateICmpNE(Load(I32,offsetof(sgpl_exec_ctx,gpu_parallel_groups)),B.getInt32(0));
    Value *GroupOffsets=Load(Ptr,offsetof(sgpl_exec_ctx,gpu_group_offsets));
    Value *RangeIndex=B.CreateSelect(Parallel,B.CreateLShr(F->getArg(1),B.getInt64(32)),Partition);
    Value *RangeOffsets=B.CreateSelect(Parallel,GroupOffsets,Offsets);
    Value *First=B.CreateLoad(I64,B.CreateGEP(I64,RangeOffsets,RangeIndex));
    Value *Last=B.CreateLoad(I64,B.CreateGEP(I64,RangeOffsets,B.CreateAdd(RangeIndex,B.getInt64(1))));
    Value *Ops=Load(Ptr,offsetof(sgpl_exec_ctx,ops));
    auto Op=[&](unsigned Index){return gpuByteAddress(B,Ops,Index*sizeof(sgpl_runtime_op));};
    BasicBlock *Entry=B.GetInsertBlock(),*Check=BasicBlock::Create(C,"record.check",F),*Work=BasicBlock::Create(C,"record",F),
        *Dispatch=BasicBlock::Create(C,"dispatch",F),*PairBlock=BasicBlock::Create(C,"pair",F),
        *BeginBlock=BasicBlock::Create(C,"source.begin",F),*EndBlock=BasicBlock::Create(C,"source.end",F),
        *Next=BasicBlock::Create(C,"next",F),*Done=BasicBlock::Create(C,"done",F);
    B.CreateBr(Check);B.SetInsertPoint(Check);PHINode *Index=B.CreatePHI(I64,2);Index->addIncoming(First,Entry);
    B.CreateCondBr(B.CreateAnd(B.CreateICmpSLT(Index,Last),B.CreateICmpEQ(Load(I32,offsetof(sgpl_exec_ctx,execution_status)),B.getInt32(0))),Work,Done);B.SetInsertPoint(Work);
    Value *At=B.CreateMul(Index,B.getInt64(3));Value *U=B.CreateLoad(I32,B.CreateGEP(I32,Records,At));
    Value *V=B.CreateLoad(I32,B.CreateGEP(I32,Records,B.CreateAdd(At,B.getInt64(1))));
    Value *Kind=B.CreateLoad(I32,B.CreateGEP(I32,Records,B.CreateAdd(At,B.getInt64(2))));
    BasicBlock *OrdinalRead=BasicBlock::Create(C,"array.occurrence",F),*OrdinalDone=BasicBlock::Create(C,"occurrence.done",F);
    B.CreateCondBr(B.CreateICmpNE(Load(I32,offsetof(sgpl_exec_ctx,array_append_sites)),B.getInt32(0)),OrdinalRead,OrdinalDone);
    B.SetInsertPoint(OrdinalRead);
    B.CreateStore(B.CreateLoad(I64,B.CreateGEP(I64,Load(Ptr,offsetof(sgpl_exec_ctx,gpu_record_sources)),Index)),gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,array_source_ordinal)));
    B.CreateStore(B.CreateLoad(I64,B.CreateGEP(I64,Load(Ptr,offsetof(sgpl_exec_ctx,gpu_record_work)),Index)),gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,array_work_ordinal)));
    B.CreateStore(B.getInt32(0),gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,array_append_seq)));
    B.CreateBr(OrdinalDone);B.SetInsertPoint(OrdinalDone);
    Value *Membership=Load(Ptr,offsetof(sgpl_exec_ctx,membership));
    BasicBlock *Gate=BasicBlock::Create(C,"membership",F);
    B.CreateCondBr(B.CreateICmpEQ(Membership,ConstantPointerNull::get(cast<PointerType>(Ptr))),Dispatch,Gate);
    B.SetInsertPoint(Gate);B.CreateCondBr(B.CreateICmpNE(B.CreateLoad(B.getInt8Ty(),B.CreateGEP(B.getInt8Ty(),Membership,U)),B.getInt8(0)),Dispatch,Next);
    B.SetInsertPoint(Dispatch);auto *Switch=B.CreateSwitch(Kind,PairBlock,2);Switch->addCase(B.getInt32(1),BeginBlock);Switch->addCase(B.getInt32(2),EndBlock);
    B.SetInsertPoint(BeginBlock);B.CreateStore(ConstantPointerNull::get(cast<PointerType>(Ptr)),gpuByteAddress(B,Local,offsetof(sgpl_exec_ctx,source_state)));
    for(auto Hook:Begins) B.CreateCall(Hook.second,{Op(Hook.first),Local,U});
    B.CreateBr(Next);B.SetInsertPoint(EndBlock);for(auto Hook:Ends) B.CreateCall(Hook.second,{Op(Hook.first),Local,U});B.CreateBr(Next);
    B.SetInsertPoint(PairBlock);
    Value *SourceGate=Load(Ptr,offsetof(sgpl_exec_ctx,spatial_source_gate));BasicBlock *GateRead=BasicBlock::Create(C,"source.gate",F),*Execute=BasicBlock::Create(C,"execute",F);
    B.CreateCondBr(B.CreateICmpEQ(SourceGate,ConstantPointerNull::get(cast<PointerType>(Ptr))),Execute,GateRead);
    B.SetInsertPoint(GateRead);B.CreateCondBr(B.CreateICmpNE(B.CreateLoad(B.getInt8Ty(),B.CreateGEP(B.getInt8Ty(),SourceGate,B.CreateSelect(B.CreateICmpNE(Load(I32,offsetof(sgpl_exec_ctx,array_append_sites)),B.getInt32(0)),Load(I64,offsetof(sgpl_exec_ctx,array_source_ordinal)),B.CreateZExt(U,I64)))),B.getInt8(0)),Execute,Next);
    B.SetInsertPoint(Execute);
    Value *Group=Load(I32,offsetof(sgpl_exec_ctx,spatial_group));
    BasicBlock *CountPair=BasicBlock::Create(C,"count.pair",F),*MarkGroup=BasicBlock::Create(C,"mark.group",F),*Invoke=BasicBlock::Create(C,"invoke",F);
    B.CreateCondBr(B.CreateICmpEQ(Group,B.getInt32(SGPL_SPATIAL_PAIR_OCCURRENCE)),CountPair,MarkGroup);
    B.SetInsertPoint(CountPair);B.CreateAtomicRMW(AtomicRMWInst::Add,gpuByteAddress(B,Shared,offsetof(sgpl_exec_ctx,spatial_occupied_groups)),B.getInt64(1),MaybeAlign(8),AtomicOrdering::Monotonic);B.CreateBr(Invoke);
    B.SetInsertPoint(MarkGroup);Value *Groups=Load(Ptr,offsetof(sgpl_exec_ctx,spatial_groups_seen));
    BasicBlock *Mark=BasicBlock::Create(C,"mark",F);B.CreateCondBr(B.CreateICmpNE(Groups,ConstantPointerNull::get(cast<PointerType>(Ptr))),Mark,Invoke);
    B.SetInsertPoint(Mark);Value *Key=B.CreateSelect(B.CreateICmpEQ(Group,B.getInt32(SGPL_SPATIAL_SOURCE)),U,V);
    B.CreateStore(B.getInt8(1),B.CreateGEP(B.getInt8Ty(),Groups,Key));B.CreateBr(Invoke);
    B.SetInsertPoint(Invoke);Value *State=B.CreateLoad(Ptr,gpuByteAddress(B,Op(PairIndex),offsetof(sgpl_runtime_op,state)));
    B.CreateCall(Pair,{State,Local,U,V});B.CreateBr(Next);
    B.SetInsertPoint(Next);Value *Advance=B.CreateAdd(Index,B.getInt64(1));Index->addIncoming(Advance,Next);B.CreateBr(Check);
    B.SetInsertPoint(Done);B.CreateRet(Load(I32,offsetof(sgpl_exec_ctx,execution_status)));
    Function *DispatchGroups=Function::Create(F->getFunctionType(),GlobalValue::InternalLinkage,"sgpl.gpu.spatial.component",M);
    Function *Trace=Function::Create(FunctionType::get(B.getVoidTy(),{Ptr,I64,I32,I64},false),GlobalValue::InternalLinkage,"sgpl.gpu.group.trace",M);
    IRBuilder<> TB(BasicBlock::Create(C,"entry",Trace));BasicBlock *WriteTrace=BasicBlock::Create(C,"write",Trace),*NoTrace=BasicBlock::Create(C,"done",Trace);
    TB.CreateCondBr(TB.CreateICmpNE(Trace->getArg(0),ConstantPointerNull::get(cast<PointerType>(Ptr))),WriteTrace,NoTrace);
    TB.SetInsertPoint(WriteTrace);FunctionType *TimerType=FunctionType::get(I64,false);
    Value *Timer=TB.CreateCall(TimerType,InlineAsm::get(TimerType,"mov.u64 $0, %globaltimer;","=l",true));
    Value *TraceValue=TB.CreateSelect(TB.CreateICmpEQ(Trace->getArg(2),TB.getInt32(2)),Trace->getArg(3),Timer);
    Value *TraceAt=TB.CreateAdd(TB.CreateMul(Trace->getArg(1),TB.getInt64(3)),TB.CreateZExt(Trace->getArg(2),I64));
    TB.CreateStore(TraceValue,TB.CreateGEP(I64,Trace->getArg(0),TraceAt));TB.CreateBr(NoTrace);TB.SetInsertPoint(NoTrace);TB.CreateRetVoid();
    IRBuilder<> G(BasicBlock::Create(C,"entry",DispatchGroups));
    Value *Context=G.CreateLoad(Ptr,gpuByteAddress(G,DispatchGroups->getArg(0),ContextOffset));
    Value *Enabled=G.CreateLoad(I32,gpuByteAddress(G,Context,offsetof(sgpl_exec_ctx,gpu_parallel_groups)));
    Value *Thread=G.CreateZExt(G.CreateAdd(G.CreateMul(gpuReadSreg(G,"ctaid.x"),gpuReadSreg(G,"ntid.x")),gpuReadSreg(G,"tid.x")),I64);
    Value *Width=G.CreateZExt(G.CreateMul(gpuReadSreg(G,"ntid.x"),gpuReadSreg(G,"nctaid.x")),I64);
    BasicBlock *Serial=BasicBlock::Create(C,"ordered.partition",DispatchGroups),*GroupBlock=BasicBlock::Create(C,"groups",DispatchGroups),
        *RunSerial=BasicBlock::Create(C,"lane.zero",DispatchGroups),*Exit=BasicBlock::Create(C,"exit",DispatchGroups);
    G.CreateCondBr(G.CreateICmpNE(Enabled,G.getInt32(0)),GroupBlock,Serial);
    G.SetInsertPoint(Serial);G.CreateCondBr(G.CreateICmpEQ(Thread,G.getInt64(0)),RunSerial,Exit);
    G.SetInsertPoint(RunSerial);G.CreateRet(G.CreateCall(F,{DispatchGroups->getArg(0),DispatchGroups->getArg(1)}));
    G.SetInsertPoint(GroupBlock);Value *Ranges=G.CreateLoad(Ptr,gpuByteAddress(G,Context,offsetof(sgpl_exec_ctx,gpu_partition_groups)));
    Value *P=DispatchGroups->getArg(1),*Start=G.CreateLoad(I64,G.CreateGEP(I64,Ranges,P));
    Value *Times=G.CreateLoad(Ptr,gpuByteAddress(G,Context,offsetof(sgpl_exec_ctx,gpu_group_times)));
    Value *End=G.CreateLoad(I64,G.CreateGEP(I64,Ranges,G.CreateAdd(P,G.getInt64(1))));
    BasicBlock *GroupEntry=G.GetInsertBlock(),*Test=BasicBlock::Create(C,"group.check",DispatchGroups),
        *Run=BasicBlock::Create(C,"group.run",DispatchGroups),*AdvanceGroup=BasicBlock::Create(C,"group.next",DispatchGroups),
        *Failed=BasicBlock::Create(C,"group.failed",DispatchGroups);
    Value *Initial=G.CreateAdd(Start,Thread);G.CreateBr(Test);G.SetInsertPoint(Test);PHINode *GI=G.CreatePHI(I64,2);GI->addIncoming(Initial,GroupEntry);
    G.CreateCondBr(G.CreateICmpULT(GI,End),Run,Exit);G.SetInsertPoint(Run);
    Value *Encoded=G.CreateOr(P,G.CreateShl(GI,G.getInt64(32)));
    G.CreateCall(Trace,{Times,GI,G.getInt32(0),Thread});
    Value *RC=G.CreateCall(F,{DispatchGroups->getArg(0),Encoded});
    G.CreateCall(Trace,{Times,GI,G.getInt32(1),Thread});G.CreateCall(Trace,{Times,GI,G.getInt32(2),Thread});
    G.CreateCondBr(G.CreateICmpEQ(RC,G.getInt32(0)),AdvanceGroup,Failed);
    G.SetInsertPoint(AdvanceGroup);GI->addIncoming(G.CreateAdd(GI,Width),AdvanceGroup);G.CreateBr(Test);
    G.SetInsertPoint(Failed);G.CreateRet(RC);G.SetInsertPoint(Exit);G.CreateRet(G.getInt32(0));return DispatchGroups;
}
static Function *gpuSpatialSpan(Function *Partition,uint64_t ContextOffset,uint64_t ActiveOffset) {
    Module *M=Partition->getParent();auto &C=M->getContext();Type *Ptr=PointerType::get(C,0),*I64=Type::getInt64Ty(C),*I32=Type::getInt32Ty(C);
    Function *F=Function::Create(FunctionType::get(I32,{Ptr,I64},false),GlobalValue::InternalLinkage,"sgpl.gpu.spatial.span",M);
    IRBuilder<> B(BasicBlock::Create(C,"entry",F));
    Value *Mini=B.CreateAlloca(Ptr);
    Value *Active=B.CreateLoad(I32,gpuByteAddress(B,F->getArg(0),ActiveOffset));
    BasicBlock *Enabled=BasicBlock::Create(C,"enabled",F),*Skipped=BasicBlock::Create(C,"skipped",F);
    B.CreateCondBr(B.CreateICmpNE(Active,B.getInt32(0)),Enabled,Skipped);IRBuilder<>(Skipped).CreateRet(ConstantInt::get(I32,0));
    B.SetInsertPoint(Enabled);
    B.CreateStore(B.CreateLoad(Ptr,gpuByteAddress(B,F->getArg(0),ContextOffset)),Mini);
    Value *First=B.CreateAnd(F->getArg(1),B.getInt64(UINT32_MAX));Value *Count=B.CreateLShr(F->getArg(1),B.getInt64(32));
    Count=B.CreateSelect(B.CreateICmpEQ(Count,B.getInt64(0)),B.getInt64(1),Count);Value *End=B.CreateAdd(First,Count);
    BasicBlock *Entry=B.GetInsertBlock(),*Check=BasicBlock::Create(C,"check",F),*Work=BasicBlock::Create(C,"partition",F),*Done=BasicBlock::Create(C,"done",F),*Fail=BasicBlock::Create(C,"failed",F);
    B.CreateBr(Check);B.SetInsertPoint(Check);PHINode *Index=B.CreatePHI(I64,2);Index->addIncoming(First,Entry);
    B.CreateCondBr(B.CreateICmpULT(Index,End),Work,Done);B.SetInsertPoint(Work);
    Value *Status=B.CreateCall(Partition,{Mini,Index});Value *Next=B.CreateAdd(Index,B.getInt64(1));Index->addIncoming(Next,Work);
    B.CreateCondBr(B.CreateICmpEQ(Status,B.getInt32(0)),Check,Fail);B.SetInsertPoint(Fail);B.CreateRet(Status);
    B.SetInsertPoint(Done);B.CreateRet(B.getInt32(0));return F;
}
static GpuCallback buildGpuCallback(Function *Root,uint64_t FrameBytes) {
    GpuCallback Result;Module *M=Root->getParent();auto &C=M->getContext();
    SmallVector<Function *,16> Functions;SmallPtrSet<Function *,16> Seen;
    SmallPtrSet<GlobalVariable *,16> Globals;
    std::function<void(Value *)> CollectGlobals=[&](Value *V) {
        if(auto *G=dyn_cast<GlobalVariable>(V)) {
            if(Globals.insert(G).second) Result.Globals.push_back(G);
            return;
        }
        if(auto *E=dyn_cast<ConstantExpr>(V)) for(Value *Operand:E->operands()) CollectGlobals(Operand);
    };
    std::function<void(Function *)> Visit=[&](Function *F) {
        if(!Seen.insert(F).second) return;
        Functions.push_back(F);
        for(BasicBlock &BB:*F) for(Instruction &I:BB) {
            if(auto *A=dyn_cast<AllocaInst>(&I)) if(!isa<ConstantInt>(A->getArraySize())) Result.Reason="dynamic device stack allocation";
            for(Value *V:I.operands()) CollectGlobals(V);
            if(auto *Call=dyn_cast<CallInst>(&I)) {
                if(Call->isInlineAsm()) continue;
                Function *F=Call->getCalledFunction();
                if(!F) {Result.Reason="indirect device call";continue;}
                if(F->isIntrinsic()) continue;
                if(!F->isDeclaration()) Visit(F);
                else if(F->getName()!="autograph_neighbor_iter_init"&&F->getName()!="autograph_neighbor_iter_next"&&
                        F->getName()!="sgpl_round_spatial_current_frame"&&F->getName()!="autograph_exec_partition_state"&&
                        F->getName()!="autograph_exec_op_state"&&F->getName()!="autograph_exec_ctx_snapshot"&&
                        F->getName()!="autograph_exec_ctx_claims"&&F->getName()!="autograph_exec_ctx_set_source_gate"&&
                        F->getName()!="autograph_frontier_activate"&&F->getName()!="autograph_exec_ctx_occurrence"&&F->getName()!="autograph_exec_ctx_last_source")
                    Result.Reason=("unsupported device call: "+F->getName()).str();
            }
            else if(isa<InvokeInst>(I)||isa<CallBrInst>(I)) Result.Reason="unsupported device call control flow";
        }
    };
    Visit(Root);if(!Result.Reason.empty()) return Result;
    /* Transfer directions only. Runtime alias merging retains every write
     * flag. Unknown captured storage remains writable in its descriptor. */
    SmallPtrSet<GlobalVariable *,16> Written;
    std::function<void(Value *,SmallPtrSetImpl<Value *> &)> Writes=[&](Value *V,SmallPtrSetImpl<Value *> &SeenValues) {
        if(!SeenValues.insert(V).second) return;
        if(auto *G=dyn_cast<GlobalVariable>(V)) {Written.insert(G);return;}
        if(auto *U=dyn_cast<User>(V)) for(Value *O:U->operands()) Writes(O,SeenValues);
    };
    for(Function *F:Functions) for(BasicBlock &BB:*F) for(Instruction &I:BB) {
        Value *P=nullptr;
        if(auto *S=dyn_cast<StoreInst>(&I)) P=S->getPointerOperand();
        else if(auto *A=dyn_cast<AtomicRMWInst>(&I)) P=A->getPointerOperand();
        else if(auto *A=dyn_cast<AtomicCmpXchgInst>(&I)) P=A->getPointerOperand();
        else if(auto *T=dyn_cast<MemIntrinsic>(&I)) P=T->getDest();
        if(P) {SmallPtrSet<Value *,32> SeenValues;Writes(P,SeenValues);}
        if(auto *Call=dyn_cast<CallBase>(&I)) if(!Call->isInlineAsm())
            for(Value *A:Call->args()) if(A->getType()->isPointerTy()) {SmallPtrSet<Value *,32> SeenValues;Writes(A,SeenValues);}
    }
    for(GlobalVariable *G:Result.Globals) if(!Written.count(G)) Result.ReadOnlyGlobals.push_back(G);
    DenseMap<Function *,Function *> Clones;
    Type *Ptr=PointerType::get(C,0);
    for(Function *F:Functions) {
        SmallVector<Type *,8> Args(F->getFunctionType()->params());Args.push_back(Ptr);
        Function *D=Function::Create(FunctionType::get(F->getReturnType(),Args,false),GlobalValue::InternalLinkage,
            "sgpl_device_fn_"+std::to_string(nextParallelSiteId()),M);Clones[F]=D;
        D->addFnAttr("sgpl.device.only");
    }
    for(Function *F:Functions) {
        Function *D=Clones[F];ValueToValueMapTy Map;auto A=D->arg_begin();
        for(Argument &Old:F->args()) Map[&Old]=&*A++;
        SmallVector<ReturnInst *,8> Returns;
        CloneFunctionInto(D,F,Map,CloneFunctionChangeType::LocalChangesOnly,Returns);
        D->addFnAttr("sgpl.device.only");
        Value *Env=D->getArg(D->arg_size()-1);
        IRBuilder<> Entry(&*D->getEntryBlock().getFirstInsertionPt());
        DenseMap<GlobalVariable *,Value *> Addresses;
        for(unsigned N=0;N<Result.Globals.size();++N)
            Addresses[Result.Globals[N]]=Entry.CreateLoad(Ptr,gpuByteAddress(Entry,Env,FrameBytes+N*8));
        std::function<Value *(Value *,IRBuilder<> &)> Materialize=[&](Value *V,IRBuilder<> &B)->Value* {
            if(auto *G=dyn_cast<GlobalVariable>(V)) return Addresses.lookup(G);
            if(auto *E=dyn_cast<ConstantExpr>(V)) {
                Instruction *Copy=E->getAsInstruction();
                for(unsigned N=0;N<Copy->getNumOperands();++N) Copy->setOperand(N,Materialize(Copy->getOperand(N),B));
                B.Insert(Copy);return Copy;
            }
            return V;
        };
        SmallVector<CallInst *,16> Calls;
        for(BasicBlock &BB:*D) for(Instruction &I:BB) {
            IRBuilder<> B(&I);
            for(unsigned N=0;N<I.getNumOperands();++N) {
                Value *V=I.getOperand(N);
                if(isa<GlobalVariable>(V)||isa<ConstantExpr>(V)) I.setOperand(N,Materialize(V,B));
            }
            if(auto *Call=dyn_cast<CallInst>(&I)) Calls.push_back(Call);
        }
        for(CallInst *Call:Calls) {
            Function *F=Call->getCalledFunction();IRBuilder<> B(Call);
            if(Clones.count(F)) {
                SmallVector<Value *,8> Args(Call->args());Args.push_back(Env);
                Value *New=B.CreateCall(Clones[F],Args);
                if(!Call->getType()->isVoidTy()) Call->replaceAllUsesWith(New);Call->eraseFromParent();
            } else if(F && F->getName()=="sgpl_round_spatial_current_frame") {
                Call->replaceAllUsesWith(Env);Call->eraseFromParent();
            } else if(F && (F->getName()=="autograph_exec_partition_state"||F->getName()=="autograph_exec_op_state"||F->getName()=="autograph_exec_ctx_claims")) {
                uint64_t Offset=F->getName()=="autograph_exec_partition_state"?offsetof(sgpl_exec_ctx,partition_state):
                    F->getName()=="autograph_exec_op_state"?offsetof(sgpl_runtime_op,state):offsetof(sgpl_exec_ctx,spatial_claims);
                Value *P=B.CreateLoad(Ptr,gpuByteAddress(B,Call->getArgOperand(0),Offset));Call->replaceAllUsesWith(P);Call->eraseFromParent();
            } else if(F && F->getName()=="autograph_exec_ctx_occurrence") {
                Value *Ordinal=B.CreateLoad(B.getInt64Ty(),gpuByteAddress(B,Call->getArgOperand(0),offsetof(sgpl_exec_ctx,array_source_ordinal)));
                Call->replaceAllUsesWith(Ordinal);Call->eraseFromParent();
            } else if(F && F->getName()=="autograph_exec_ctx_last_source") {
                Value *Ctx=Call->getArgOperand(0);
                Value *N=B.CreateLoad(B.getInt64Ty(),gpuByteAddress(B,Ctx,offsetof(sgpl_exec_ctx,gpu_vertex_count)));
                Value *Membership=B.CreateLoad(Ptr,gpuByteAddress(B,Ctx,offsetof(sgpl_exec_ctx,membership)));
                BasicBlock *Before=Call->getParent(),*Done=Before->splitBasicBlock(Call,"last.source.join");
                Before->getTerminator()->eraseFromParent();B.SetInsertPoint(Before);
                BasicBlock *Check=BasicBlock::Create(C,"last.source.check",D,Done),*Read=BasicBlock::Create(C,"last.source.read",D,Done),*Found=BasicBlock::Create(C,"last.source.found",D,Done),*Empty=BasicBlock::Create(C,"last.source.empty",D,Done);
                B.CreateBr(Check);B.SetInsertPoint(Check);PHINode *U=B.CreatePHI(B.getInt64Ty(),2);U->addIncoming(N,Before);
                B.CreateCondBr(B.CreateICmpSGT(U,B.getInt64(0)),Read,Empty);B.SetInsertPoint(Read);
                Value *Previous=B.CreateSub(U,B.getInt64(1));U->addIncoming(Previous,Read);
                BasicBlock *Member=BasicBlock::Create(C,"last.source.member",D,Done);
                B.CreateCondBr(B.CreateICmpEQ(Membership,ConstantPointerNull::get(cast<PointerType>(Ptr))),Found,Member);
                B.SetInsertPoint(Member);Value *Active=B.CreateICmpNE(B.CreateLoad(B.getInt8Ty(),B.CreateGEP(B.getInt8Ty(),Membership,Previous)),B.getInt8(0));
                B.CreateCondBr(Active,Found,Check);U->setIncomingBlock(1,Member);
                B.SetInsertPoint(Found);Value *Last=B.CreateTrunc(Previous,Call->getType());B.CreateBr(Done);
                B.SetInsertPoint(Empty);B.CreateBr(Done);
                PHINode *Result=PHINode::Create(Call->getType(),2,"last.source",&*Done->begin());Result->addIncoming(Last,Found);Result->addIncoming(ConstantInt::getSigned(Call->getType(),-1),Empty);
                Call->replaceAllUsesWith(Result);Call->eraseFromParent();
            } else if(F && F->getName()=="autograph_exec_ctx_snapshot") {
                Value *Table=B.CreateLoad(Ptr,gpuByteAddress(B,Call->getArgOperand(0),offsetof(sgpl_exec_ctx,spatial_snapshots)));
                Value *P=B.CreateLoad(Ptr,B.CreateGEP(Ptr,Table,Call->getArgOperand(1)));Call->replaceAllUsesWith(P);Call->eraseFromParent();
            } else if(F && F->getName()=="autograph_exec_ctx_set_source_gate") {
                Value *Gate=B.CreateLoad(Ptr,gpuByteAddress(B,Call->getArgOperand(0),offsetof(sgpl_exec_ctx,spatial_source_gate)));
                B.CreateStore(B.CreateZExt(B.CreateICmpNE(Call->getArgOperand(2),B.getInt32(0)),B.getInt8Ty()),B.CreateGEP(B.getInt8Ty(),Gate,B.CreateSelect(B.CreateICmpNE(B.CreateLoad(B.getInt32Ty(),gpuByteAddress(B,Call->getArgOperand(0),offsetof(sgpl_exec_ctx,array_append_sites))),B.getInt32(0)),B.CreateLoad(B.getInt64Ty(),gpuByteAddress(B,Call->getArgOperand(0),offsetof(sgpl_exec_ctx,array_source_ordinal))),B.CreateZExt(Call->getArgOperand(1),B.getInt64Ty()))));Call->eraseFromParent();
            } else if(F && F->getName()=="autograph_frontier_activate") {
                Value *Ctx=Call->getArgOperand(0),*V=Call->getArgOperand(1);
                BasicBlock *Before=Call->getParent(),*Done=Before->splitBasicBlock(Call,"activate.join");
                Before->getTerminator()->eraseFromParent();B.SetInsertPoint(Before);
                BasicBlock *Ordered=BasicBlock::Create(C,"array.append",D,Done),*Legacy=BasicBlock::Create(C,"set.append",D,Done);
                Value *Sites=B.CreateLoad(B.getInt32Ty(),gpuByteAddress(B,Ctx,offsetof(sgpl_exec_ctx,array_append_sites)));
                B.CreateCondBr(B.CreateICmpNE(Sites,B.getInt32(0)),Ordered,Legacy);
                IRBuilder<> A(Ordered);
                Value *SeqPtr=gpuByteAddress(A,Ctx,offsetof(sgpl_exec_ctx,array_append_seq));
                Value *Seq=A.CreateLoad(A.getInt32Ty(),SeqPtr),*Work=A.CreateLoad(A.getInt64Ty(),gpuByteAddress(A,Ctx,offsetof(sgpl_exec_ctx,array_work_ordinal)));
                Value *Count=A.CreateLoad(A.getInt64Ty(),gpuByteAddress(A,Ctx,offsetof(sgpl_exec_ctx,array_work_count)));
                BasicBlock *ArrayWrite=BasicBlock::Create(C,"array.write",D,Done),*ArrayError=BasicBlock::Create(C,"array.overflow",D,Done);
                A.CreateCondBr(A.CreateAnd(A.CreateICmpULT(Seq,Sites),A.CreateICmpULT(Work,Count)),ArrayWrite,ArrayError);
                IRBuilder<> AW(ArrayWrite);Value *Log=AW.CreateLoad(Ptr,gpuByteAddress(AW,Ctx,offsetof(sgpl_exec_ctx,array_append_log)));
                Value *At=AW.CreateMul(AW.CreateAdd(AW.CreateMul(Work,AW.CreateZExt(Sites,AW.getInt64Ty())),AW.CreateZExt(Seq,AW.getInt64Ty())),AW.getInt64(2));
                AW.CreateStore(V,AW.CreateGEP(AW.getInt32Ty(),Log,AW.CreateAdd(At,AW.getInt64(1))));
                AW.CreateStore(AW.getInt32(1),AW.CreateGEP(AW.getInt32Ty(),Log,At));AW.CreateStore(AW.CreateAdd(Seq,AW.getInt32(1)),SeqPtr);AW.CreateBr(Done);
                IRBuilder<> AE(ArrayError);Value *ArrayFailure=AE.CreateLoad(Ptr,gpuByteAddress(AE,Ctx,offsetof(sgpl_exec_ctx,execution_error)));
                auto *AS=AE.CreateStore(AE.getInt32(SGPL_DAG_ERR_INVALID),ArrayFailure);AS->setAtomic(AtomicOrdering::Monotonic);AS->setAlignment(Align(4));AE.CreateBr(Done);
                B.SetInsertPoint(Legacy);
                Value *Seen=B.CreateLoad(Ptr,gpuByteAddress(B,Ctx,offsetof(sgpl_exec_ctx,dest_seen)));
                Value *Cell=B.CreateGEP(B.getInt32Ty(),Seen,V);
                Value *Claim=B.CreateAtomicCmpXchg(Cell,B.getInt32(0),B.getInt32(1),MaybeAlign(4),AtomicOrdering::Monotonic,AtomicOrdering::Monotonic);
                Value *Old=B.CreateExtractValue(Claim,0);
                BasicBlock *Append=BasicBlock::Create(C,"activate.append",D,Done);B.CreateCondBr(B.CreateICmpEQ(Old,B.getInt32(0)),Append,Done);
                IRBuilder<> AB(Append);
                Value *Lanes=AB.CreateLoad(Ptr,gpuByteAddress(AB,Ctx,offsetof(sgpl_exec_ctx,spatial_frontier_lanes)));
                Value *P=AB.CreateLoad(AB.getInt32Ty(),gpuByteAddress(AB,Ctx,offsetof(sgpl_exec_ctx,spatial_partition)));
                Value *Lane=AB.CreateGEP(AB.getInt8Ty(),Lanes,AB.CreateMul(AB.CreateZExt(P,AB.getInt64Ty()),AB.getInt64(16)));
                Value *Data=AB.CreateLoad(Ptr,Lane),*Length=AB.CreateAtomicRMW(AtomicRMWInst::Add,gpuByteAddress(AB,Lane,8),AB.getInt32(1),MaybeAlign(4),AtomicOrdering::Monotonic);
                Value *Capacity=AB.CreateLoad(AB.getInt32Ty(),gpuByteAddress(AB,Lane,12));
                BasicBlock *Write=BasicBlock::Create(C,"activate.write",D,Done),*Overflow=BasicBlock::Create(C,"activate.overflow",D,Done);
                AB.CreateCondBr(AB.CreateICmpULT(Length,Capacity),Write,Overflow);
                IRBuilder<> WB(Write);WB.CreateStore(V,WB.CreateGEP(WB.getInt32Ty(),Data,Length));WB.CreateBr(Done);
                IRBuilder<> OB(Overflow);OB.CreateStore(OB.getInt32(SGPL_DAG_ERR_INVALID),gpuByteAddress(OB,Ctx,offsetof(sgpl_exec_ctx,execution_status)));
                Value *Error=OB.CreateLoad(Ptr,gpuByteAddress(OB,Ctx,offsetof(sgpl_exec_ctx,execution_error)));
                auto *Store=OB.CreateStore(OB.getInt32(SGPL_DAG_ERR_INVALID),Error);Store->setAtomic(AtomicOrdering::Monotonic);Store->setAlignment(Align(4));OB.CreateBr(Done);
                PHINode *Won=PHINode::Create(Call->getType(),5,"won",&*Done->begin());Won->addIncoming(ConstantInt::get(Call->getType(),0),Legacy);
                Won->addIncoming(ConstantInt::get(Call->getType(),1),ArrayWrite);Won->addIncoming(ConstantInt::get(Call->getType(),0),ArrayError);Won->addIncoming(ConstantInt::get(Call->getType(),1),Write);Won->addIncoming(ConstantInt::get(Call->getType(),0),Overflow);
                Call->replaceAllUsesWith(Won);Call->eraseFromParent();
            } else if(F && F->getName()=="autograph_neighbor_iter_init") {
                Value *Graph=Call->getArgOperand(0),*U=B.CreateSExtOrTrunc(Call->getArgOperand(1),B.getInt64Ty());
                Value *Iter=Call->getArgOperand(2);
                Value *Rows=B.CreateLoad(Ptr,gpuByteAddress(B,Graph,16));
                Value *Columns=B.CreateLoad(Ptr,gpuByteAddress(B,Graph,24));
                Value *Begin=B.CreateLoad(B.getInt64Ty(),B.CreateGEP(B.getInt64Ty(),Rows,U));
                Value *End=B.CreateLoad(B.getInt64Ty(),B.CreateGEP(B.getInt64Ty(),Rows,B.CreateAdd(U,B.getInt64(1))));
                B.CreateStore(Columns,Iter);B.CreateStore(Begin,gpuByteAddress(B,Iter,8));B.CreateStore(End,gpuByteAddress(B,Iter,16));Call->eraseFromParent();
            } else if(F && F->getName()=="autograph_neighbor_iter_next") {
                Value *Iter=Call->getArgOperand(0),*Out=Call->getArgOperand(1);
                Value *Pos=B.CreateLoad(B.getInt64Ty(),gpuByteAddress(B,Iter,8));
                Value *End=B.CreateLoad(B.getInt64Ty(),gpuByteAddress(B,Iter,16));
                BasicBlock *Before=Call->getParent();BasicBlock *Done=Before->splitBasicBlock(Call,"next.join");
                Before->getTerminator()->eraseFromParent();B.SetInsertPoint(Before);
                BasicBlock *Read=BasicBlock::Create(C,"next.read",D,Done);
                B.CreateCondBr(B.CreateICmpSLT(Pos,End),Read,Done);
                IRBuilder<> RB(Read);Value *Cols=RB.CreateLoad(Ptr,Iter);
                RB.CreateStore(RB.CreateLoad(RB.getInt32Ty(),RB.CreateGEP(RB.getInt32Ty(),Cols,Pos)),Out);
                RB.CreateStore(RB.CreateAdd(Pos,RB.getInt64(1)),gpuByteAddress(RB,Iter,8));RB.CreateBr(Done);
                PHINode *Has=PHINode::Create(Call->getType(),2,"next",&*Done->begin());
                Has->addIncoming(ConstantInt::get(Call->getType(),0),Before);Has->addIncoming(ConstantInt::get(Call->getType(),1),Read);
                Call->replaceAllUsesWith(Has);Call->eraseFromParent();
            }
        }
    }
    /* Concurrent groups reduce into their partition's private storage.
     * CAS implements the already accepted operator. Live arrays are unchanged.
     * Atomicity is a realization mechanism, never a partition premise. */
    for(auto &Entry:Clones) {
        Function *D=Entry.second;SmallVector<StoreInst *,8> Folds;
        for(BasicBlock &BB:*D) for(Instruction &I:BB) {
            if(auto *L=dyn_cast<LoadInst>(&I)) if(L->getMetadata("sgpl.gpu.private.load")) {
                unsigned Bits=L->getType()->getScalarSizeInBits();
                if(Bits!=32&&Bits!=64) {Result.Reason="private device fold requires a 32 or 64 bit cell";return Result;}
                L->setAtomic(AtomicOrdering::Monotonic);L->setAlignment(Align(Bits/8));
            }
            if(auto *S=dyn_cast<StoreInst>(&I)) if(S->getMetadata("sgpl.gpu.private.fold")) Folds.push_back(S);
        }
        for(StoreInst *S:Folds) {
            auto *MD=S->getMetadata("sgpl.gpu.private.fold");
            RedOp Op=(RedOp)mdconst::extract<ConstantInt>(MD->getOperand(0))->getZExtValue();
            Value *Stored=S->getValueOperand(),*Contribution=Stored;Type *T=Stored->getType();
            auto OldLoad=[&](Value *V){auto *L=dyn_cast<LoadInst>(V);return L&&sameArraySlot(L->getPointerOperand(),S->getPointerOperand());};
            if(auto *Binary=dyn_cast<BinaryOperator>(Stored)) {
                if(!OldLoad(Binary->getOperand(0))&&!OldLoad(Binary->getOperand(1))) {Result.Reason="private device fold contribution is unavailable";return Result;}
                Contribution=Binary->getOperand(OldLoad(Binary->getOperand(0))?1:0);
            } else if(auto *Call=dyn_cast<IntrinsicInst>(Stored)) Contribution=Call->getArgOperand(OldLoad(Call->getArgOperand(0))?1:0);
            else if(auto *Select=dyn_cast<SelectInst>(Stored)) Contribution=OldLoad(Select->getTrueValue())?Select->getFalseValue():Select->getTrueValue();
            unsigned Bits=T->getScalarSizeInBits();if(Bits!=32&&Bits!=64) {Result.Reason="unsupported private device fold width";return Result;}
            auto *Integer=IntegerType::get(C,Bits);Value *Cell=S->getPointerOperand();
            IRBuilder<> Before(S);auto *Initial=Before.CreateLoad(Integer,Cell);Initial->setAtomic(AtomicOrdering::Monotonic);Initial->setAlignment(Align(Bits/8));
            BasicBlock *Pre=S->getParent(),*Done=Pre->splitBasicBlock(S,"private.fold.done");Pre->getTerminator()->eraseFromParent();
            BasicBlock *Retry=BasicBlock::Create(C,"private.fold.retry",D,Done);IRBuilder<>(Pre).CreateBr(Retry);
            IRBuilder<> R(Retry);PHINode *Expected=R.CreatePHI(Integer,2);Expected->addIncoming(Initial,Pre);
            Value *Old=T==Integer?(Value*)Expected:R.CreateBitCast(Expected,T),*Desired=nullptr;
            bool FP=T->isFloatingPointTy();Intrinsic::ID Intr=Intrinsic::not_intrinsic;
            switch(Op) {
                case RedOp::Add: Desired=FP?R.CreateFAdd(Old,Contribution):R.CreateAdd(Old,Contribution);break;
                case RedOp::Sub: Desired=FP?R.CreateFSub(Old,Contribution):R.CreateSub(Old,Contribution);break;
                case RedOp::Mul: Desired=FP?R.CreateFMul(Old,Contribution):R.CreateMul(Old,Contribution);break;
                case RedOp::And: Desired=R.CreateAnd(Old,Contribution);break;
                case RedOp::Or: Desired=R.CreateOr(Old,Contribution);break;
                case RedOp::Xor: Desired=R.CreateXor(Old,Contribution);break;
                case RedOp::Min: Intr=Intrinsic::smin;break;case RedOp::Max: Intr=Intrinsic::smax;break;
                case RedOp::MinU: Intr=Intrinsic::umin;break;case RedOp::MaxU: Intr=Intrinsic::umax;break;
                case RedOp::FMinNum: Intr=Intrinsic::minnum;break;case RedOp::FMaxNum: Intr=Intrinsic::maxnum;break;
                case RedOp::FMinProp: Intr=Intrinsic::minimum;break;case RedOp::FMaxProp: Intr=Intrinsic::maximum;break;
                default:Result.Reason="unsupported private device fold operator";return Result;
            }
            if(!Desired) Desired=R.CreateCall(Intrinsic::getDeclaration(M,Intr,{T}),{Old,Contribution});
            Value *NewBits=T==Integer?Desired:R.CreateBitCast(Desired,Integer);
            auto *CAS=R.CreateAtomicCmpXchg(Cell,Expected,NewBits,MaybeAlign(Bits/8),AtomicOrdering::Monotonic,AtomicOrdering::Monotonic);
            CAS->copyMetadata(*S);
            Expected->addIncoming(R.CreateExtractValue(CAS,0),Retry);R.CreateCondBr(R.CreateExtractValue(CAS,1),Done,Retry);
            S->eraseFromParent();Result.ConcurrentPrivate=true;
        }
    }
    Function *K=Function::Create(FunctionType::get(Type::getVoidTy(C),{Ptr,Type::getInt64Ty(C),Ptr},false),
        GlobalValue::ExternalLinkage,(Root->getName().starts_with("sgpl.gpu.spatial.")?(Result.ConcurrentPrivate?"gpu_spatial_component_f_":"gpu_spatial_component_"):"gpu_component_")+std::to_string(nextParallelSiteId()),M);
    K->setCallingConv(CallingConv::PTX_Kernel);
    IRBuilder<> B(BasicBlock::Create(C,"entry",K));
    Value *Status=B.CreateCall(Clones[Root],{K->getArg(0),K->getArg(1),K->getArg(0)});
    BasicBlock *Error=BasicBlock::Create(C,"error",K),*Complete=BasicBlock::Create(C,"complete",K);
    B.CreateCondBr(B.CreateICmpNE(Status,B.getInt32(0)),Error,Complete);
    B.SetInsertPoint(Error);B.CreateAtomicRMW(AtomicRMWInst::Or,K->getArg(2),Status,MaybeAlign(4),AtomicOrdering::Monotonic);B.CreateBr(Complete);
    B.SetInsertPoint(Complete);B.CreateRetVoid();
    for(auto &Pair:Clones) if(verifyFunction(*Pair.second,&errs())) report_fatal_error("invalid device component");
    M->getOrInsertNamedMetadata("graph.gpu.kernels")->addOperand(MDNode::get(C,ValueAsMetadata::get(K)));
    Result.Kernel=K;return Result;
}

static CallInst *gpuTemporalDispatch(IRBuilder<> &B,Function *Callback,StructType *Frame,
    ArrayRef<Value *> Captures,Value *Graph,Value *State,Value *Count,Value *Budget,StringRef Scope) {
    Module *M=Callback->getParent();if(!gpuBackendSelected(M)) return nullptr;
    uint64_t Size=M->getDataLayout().getTypeAllocSize(Frame);
    Size=(Size+7)&~uint64_t(7);
    GpuCallback Device=buildGpuCallback(Callback,Size);
    if(!Device.Kernel) {
        if(getenv("SGPL_GPU_DEBUG")||getenv("GRAPH_FRONTIER_STATS")) errs()<<"[gpu-capability] "<<Scope<<": "<<Device.Reason<<"; CPU\n";
        return nullptr;
    }
    emitGpuStepPointeeRegs(M,B.getContext(),Callback);
    Type *Ptr=B.getPtrTy();uint64_t Bytes=Size+8*Device.Globals.size();
    auto *Extended=B.CreateAlloca(B.getInt8Ty(),B.getInt64(Bytes),"device.frame");Extended->setAlignment(Align(8));
    B.CreateMemCpy(Extended,MaybeAlign(8),State,MaybeAlign(8),M->getDataLayout().getTypeAllocSize(Frame));
    Type *FieldTy=StructType::get(B.getContext(),{B.getInt64Ty(),B.getInt64Ty(),B.getInt32Ty()});
    SmallVector<Constant *,16> Fields;
    const StructLayout *Layout=M->getDataLayout().getStructLayout(Frame);
    for(unsigned N=0;N<Captures.size();++N) if(Captures[N]->getType()->isPointerTy()) {
        Value *V=Captures[N];uint64_t Extent=0;unsigned Kind=0;
        if(V==Graph || V->stripPointerCasts()==Graph->stripPointerCasts()) Kind=1;
        else if(auto *A=dyn_cast<AllocaInst>(V->stripPointerCasts())) {
            if(auto *C=dyn_cast<ConstantInt>(A->getArraySize())) Extent=M->getDataLayout().getTypeAllocSize(A->getAllocatedType())*C->getZExtValue();
            if(A->getAllocatedType()->isPointerTy()) Kind=2;
        } else if(auto *G=dyn_cast<GlobalVariable>(V->stripPointerCasts())) {
            Extent=M->getDataLayout().getTypeAllocSize(G->getValueType());Kind=G->getValueType()->isPointerTy()?2:0;
        }
        Fields.push_back(ConstantStruct::get(cast<StructType>(FieldTy),{B.getInt64(Layout->getElementOffset(N)),B.getInt64(Extent),B.getInt32(Kind)}));
    }
    for(unsigned N=0;N<Device.Globals.size();++N) {
        GlobalVariable *G=Device.Globals[N];B.CreateStore(G,gpuByteAddress(B,Extended,Size+8*N));
        uint64_t Extent=M->getDataLayout().getTypeAllocSize(G->getValueType());
        bool GraphHolder=false;
        if(auto *L=dyn_cast<LoadInst>(Graph)) GraphHolder=L->getPointerOperand()->stripPointerCasts()==G;
        unsigned Kind=GraphHolder?4:G->getValueType()->isPointerTy()?2:0;
        if(is_contained(Device.ReadOnlyGlobals,G)) Kind|=SGPL_GPU_FIELD_READ_ONLY;
        Fields.push_back(ConstantStruct::get(cast<StructType>(FieldTy),{B.getInt64(Size+8*N),B.getInt64(Extent),B.getInt32(Kind)}));
    }
    ArrayType *Array=ArrayType::get(FieldTy,Fields.size());
    auto *Descriptors=new GlobalVariable(*M,Array,true,GlobalValue::PrivateLinkage,ConstantArray::get(Array,Fields),"sgpl.gpu.fields");
    auto Fn=M->getOrInsertFunction("sgpl_exec_temporal_gpu_components",FunctionType::get(B.getInt32Ty(),
        {Ptr,Ptr,B.getInt32Ty(),B.getInt32Ty(),Ptr,B.getInt64Ty(),Ptr,B.getInt32Ty(),B.getInt32Ty()},false));
    auto *Call=B.CreateCall(Fn,{Callback,Extended,Count,Budget,gpuCString(M,Device.Kernel->getName(),"gpu.component.name"),B.getInt64(Bytes),Descriptors,B.getInt32(Fields.size()),B.getInt32(Scope=="Round"?1:2)});
    B.CreateMemCpy(State,MaybeAlign(8),Extended,MaybeAlign(8),M->getDataLayout().getTypeAllocSize(Frame));
    Call->setMetadata("sgpl.temporal.scope",MDNode::get(B.getContext(),MDString::get(B.getContext(),Scope)));
    Call->setMetadata("sgpl.execution.backend",MDNode::get(B.getContext(),MDString::get(B.getContext(),"gpu-or-cpu")));
    return Call;
}
