/* Generic storage preparation. Algorithm operations and their access sites
 * remain in the IR for the effect system. This does not emit motif engines. */
static Value *frontierCapacitySlot(Value *Slot) {
    MDNode *M = nullptr;
    if (auto *G = dyn_cast_or_null<GlobalObject>(Slot)) M=G->getMetadata("sgpl.array.capacity");
    else if (auto *I = dyn_cast_or_null<Instruction>(Slot)) M=I->getMetadata("sgpl.array.capacity");
    auto *V=M&&M->getNumOperands()?dyn_cast<ValueAsMetadata>(M->getOperand(0)):nullptr;
    return V?V->getValue():nullptr;
}
static bool frontierInfrastructure(const Instruction &I) {
    return I.getMetadata("sgpl.frontier.infrastructure") != nullptr;
}
static bool frontierMovableSlot(Value *Slot) {
    if(!isa<GlobalVariable>(Slot)||!frontierCapacitySlot(Slot))return false;
    bool Fresh=false;
    for(User *U:Slot->users()) {
        if(auto *Load=dyn_cast<LoadInst>(U)) {
            std::function<bool(Value *,unsigned)> Uses=[&](Value *P,unsigned Depth) {
                if(Depth>16)return false;
                for(User *Use:P->users()) {
                    if(auto *Read=dyn_cast<LoadInst>(Use)) {if(Read->getPointerOperand()!=P)return false;continue;}
                    if(auto *G=dyn_cast<GetElementPtrInst>(Use)) {if(!Uses(G,Depth+1))return false;continue;}
                    if(auto *Cast=dyn_cast<CastInst>(Use)) {if(!Cast->getType()->isPointerTy()||!Uses(Cast,Depth+1))return false;continue;}
                    if(auto *Store=dyn_cast<StoreInst>(Use)) {
                        if(Store->getPointerOperand()==P)continue;
                        bool Swap=false;
                        for(Instruction *I:{Store->getPrevNode(),Store->getNextNode()})if(auto *Other=dyn_cast_or_null<StoreInst>(I)) {
                            auto *Back=dyn_cast<LoadInst>(Other->getValueOperand());
                            Swap|=P==Load&&Back&&Other->getPointerOperand()==Slot&&
                                Back->getPointerOperand()==Store->getPointerOperand();
                        }
                        if(!Swap)return false;continue;
                    }
                    auto *Call=dyn_cast<CallInst>(Use);Function *F=Call?Call->getCalledFunction():nullptr;
                    if(Call&&frontierInfrastructure(*Call)&&F&&F->getName()=="sgpl_frontier_reserve_i32")continue;
                    return false;
                }
                return true;
            };
            if(!Uses(Load,0))return false;
            continue;
        }
        if(auto *Call=dyn_cast<CallInst>(U)) {
            Function *F=Call->getCalledFunction();
            if(frontierInfrastructure(*Call)&&F&&(F->getName()=="sgpl_frontier_ensure_i32"||
                F->getName()=="sgpl_frontier_reserve_i32"))continue;
            return false;
        }
        auto *S=dyn_cast<StoreInst>(U);
        if(!S||S->getPointerOperand()!=Slot)return false;
        Value *V=S->getValueOperand()->stripPointerCasts();
        if(auto *A=dyn_cast<AllocaInst>(V)) {
            for(User *AU:A->users())if(AU!=S&&!isa<MemIntrinsic>(AU)) {
                auto *DeadStore=dyn_cast<StoreInst>(AU);
                auto *DeadSlot=DeadStore?dyn_cast<AllocaInst>(DeadStore->getPointerOperand()):nullptr;
                if(!DeadSlot||!DeadSlot->hasOneUse())return false;
            }
            Fresh=true;continue;
        }
        auto *L=dyn_cast<LoadInst>(V);
        if(!L)return false;
        bool Reciprocal=false;
        for(Instruction *I:{S->getPrevNode(),S->getNextNode()})if(auto *T=dyn_cast_or_null<StoreInst>(I)) {
            auto *Back=dyn_cast<LoadInst>(T->getValueOperand());
            Instruction *First=S->comesBefore(T)?S:T;
            Reciprocal|=Back&&T->getPointerOperand()==L->getPointerOperand()&&Back->getPointerOperand()==Slot&&
                L->getParent()==First->getParent()&&Back->getParent()==First->getParent()&&
                L->comesBefore(First)&&Back->comesBefore(First);
        }
        if(!Reciprocal)return false;
    }
    return Fresh;
}
static void frontierAppendGuard(StoreInst *S,Value *Slot) {
    if(S->getMetadata("sgpl.frontier.checked"))return;
    auto *G=cast<GetElementPtrInst>(S->getPointerOperand());Value *Index=primaryIndex(G);
    Module *M=S->getModule();LLVMContext &C=M->getContext();IRBuilder<> W(S);
    Type *Ptr=PointerType::get(C,0),*I32=W.getInt32Ty(),*I64=W.getInt64Ty();
    auto Ensure=M->getOrInsertFunction("sgpl_frontier_ensure_i32",FunctionType::get(I32,{Ptr,Ptr,I64},false));
    auto Require=M->getOrInsertFunction("sgpl_exec_require_status",FunctionType::get(W.getVoidTy(),{I32},false));
    auto *E=W.CreateCall(Ensure,{Slot,frontierCapacitySlot(Slot),W.CreateSExtOrTrunc(Index,I64)});
    auto *Status=W.CreateCall(Require,{E});
    for(CallInst *I:{E,Status}) {
        I->setMetadata("sgpl.frontier.infrastructure",MDNode::get(C,{}));
        I->setMetadata("sgpl.frontier.append.guard",MDNode::get(C,{}));
    }
    S->setOperand(1,W.CreateGEP(I32,W.CreateLoad(Ptr,Slot),Index));
    if(G->use_empty())G->eraseFromParent();
    S->setMetadata("sgpl.frontier.checked",MDNode::get(C,{}));
}
static Value *frontierInteger(Value *V) {
    while(auto *C=dyn_cast_or_null<CastInst>(V)) {
        if(!C->getType()->isIntegerTy())break;
        V=C->getOperand(0);
    }
    return V;
}
static bool arraySourceDomain(const NeighborLoopInfo &Info) {
    auto *Phi=dyn_cast_or_null<PHINode>(driverIndVar(Info.DriverLoop));
    auto *Br=dyn_cast<BranchInst>(Info.DriverLoop->getHeader()->getTerminator());
    auto *Cmp=Br&&Br->isConditional()?dyn_cast<ICmpInst>(Br->getCondition()):nullptr;
    if(!Phi||Phi->getNumIncomingValues()!=2||!Cmp||
       (Cmp->getPredicate()!=ICmpInst::ICMP_SLT&&Cmp->getPredicate()!=ICmpInst::ICMP_ULT)||
       frontierInteger(Cmp->getOperand(0))!=Phi)return false;
    auto *Size=dyn_cast_or_null<LoadInst>(frontierInteger(Cmp->getOperand(1)));
    if(!Size||Size->getPointerOperand()!=Info.FrontierSizePtr)return false;
    bool Zero=false,Step=false;
    for(unsigned I=0;I<2;++I) {
        Value *V=Phi->getIncomingValue(I);
        if(auto *K=dyn_cast<ConstantInt>(V))Zero|=K->isZero()&&!Info.DriverLoop->contains(Phi->getIncomingBlock(I));
        if(auto *A=dyn_cast<BinaryOperator>(V))if(A->getOpcode()==Instruction::Add&&A->getOperand(0)==Phi)
            if(auto *K=dyn_cast<ConstantInt>(A->getOperand(1)))Step|=K->isOne()&&Info.DriverLoop->contains(Phi->getIncomingBlock(I));
    }
    auto IsSource=[&](Value *V) {
        auto *L=dyn_cast_or_null<LoadInst>(frontierInteger(V));
        auto *G=L?dyn_cast<GetElementPtrInst>(L->getPointerOperand()):nullptr;
        return G&&canonicalArrayBase(G)==Info.FrontierArrayPtr&&frontierInteger(primaryIndex(G))==Phi;
    };
    bool Source=IsSource(Info.UVal);
    if(Info.UAlloca)for(User *U:Info.UAlloca->users())if(auto *S=dyn_cast<StoreInst>(U))
        if(Info.DriverLoop->contains(S)) {if(!IsSource(S->getValueOperand()))return false;Source=true;}
    for(BasicBlock *BB:Info.DriverLoop->blocks())for(Instruction &I:*BB)if(auto *S=dyn_cast<StoreInst>(&I))
        if(S->getPointerOperand()==Info.FrontierSizePtr||S->getPointerOperand()==Info.FrontierArrayPtr)return false;
    return Zero&&Step&&Source;
}
static bool checkedArrayAppend(const NeighborLoopInfo &Info,
                                SmallVectorImpl<StoreInst *> *Sites=nullptr) {
    if(!Info.HasFrontierAppend||!Info.FrontierArrayPtr||!Info.AppendArrayPtr||
       !Info.AppendCountPtr||!Info.FrontierSizePtr||
       !frontierCapacitySlot(Info.AppendArrayPtr)||!Info.NeighborLoop->getSubLoops().empty()) return false;
    if(!arraySourceDomain(Info))return false;
    unsigned Count=0;
    SmallPtrSet<const Instruction *,16> AllowedCountUses;
    for(BasicBlock *BB:Info.NeighborLoop->blocks()) for(Instruction &I:*BB) {
        auto *S=dyn_cast<StoreInst>(&I);if(!S)continue;
        auto *G=dyn_cast<GetElementPtrInst>(S->getPointerOperand());
        if(!G||canonicalArrayBase(G)!=Info.AppendArrayPtr)continue;
        auto *Index=dyn_cast_or_null<LoadInst>(frontierInteger(primaryIndex(G)));
        if(!Index||Index->getPointerOperand()!=Info.AppendCountPtr||
           !S->getValueOperand()->getType()->isIntegerTy(32)) return false;
        Value *Payload=S->getValueOperand();
        if(!isOwnedEndpointIndex(Payload,Region::V,Info)) return false;
        for(User *U:G->users())if(U!=S)return false;
        if(Index->getParent()!=S->getParent()||!Index->comesBefore(S))return false;
        for(Instruction *I=Index->getNextNode();I&&I!=S;I=I->getNextNode())
            if(auto *Other=dyn_cast<StoreInst>(I))if(Other->getPointerOperand()==Info.AppendCountPtr)return false;
        ++Count;AllowedCountUses.insert(G);
        if(Sites)Sites->push_back(S);
    }
    if(!Count)return false;
    unsigned Increments=0;
    for(BasicBlock *BB:Info.DriverLoop->blocks()) for(Instruction &I:*BB) {
        if(frontierInfrastructure(I))continue;
        if(auto *S=dyn_cast<StoreInst>(&I))if(auto *G=dyn_cast<GetElementPtrInst>(S->getPointerOperand()))
            if(canonicalArrayBase(G)==Info.FrontierArrayPtr)return false;
        if(auto *L=dyn_cast<LoadInst>(&I))if(auto *G=dyn_cast<GetElementPtrInst>(L->getPointerOperand()))
            if(canonicalArrayBase(G)==Info.AppendArrayPtr)return false;
        if(auto *S=dyn_cast<StoreInst>(&I)) if(S->getPointerOperand()==Info.AppendCountPtr) {
            auto *Add=dyn_cast<BinaryOperator>(S->getValueOperand());
            if(!Info.NeighborLoop->contains(S)||!Add||Add->getOpcode()!=Instruction::Add)return false;
            auto *L=dyn_cast<LoadInst>(Add->getOperand(0));auto *One=dyn_cast<ConstantInt>(Add->getOperand(1));
            if(!L||L->getPointerOperand()!=Info.AppendCountPtr||!One||!One->isOne())return false;
            if(!Add->hasOneUse())return false;
            StoreInst *Append=nullptr;
            for(Instruction &Earlier:*S->getParent()) {
                if(&Earlier==S)break;
                auto *AS=dyn_cast<StoreInst>(&Earlier);
                auto *G=AS?dyn_cast<GetElementPtrInst>(AS->getPointerOperand()):nullptr;
                if(G&&canonicalArrayBase(G)==Info.AppendArrayPtr)Append=AS;
                if(AS&&AS->getPointerOperand()==Info.AppendCountPtr)Append=nullptr;
            }
            if(!Append)return false;
            AllowedCountUses.insert(Add);++Increments;
        }
        if(auto *L=dyn_cast<LoadInst>(&I)) if(L->getPointerOperand()==Info.AppendCountPtr)
            for(User *U:L->users()) {
                auto *UseI=dyn_cast<Instruction>(U);
                if(UseI&&isa<CastInst>(UseI)&&llvm::all_of(UseI->users(),[](const User *U){
                    auto *I=dyn_cast<Instruction>(U);return I&&frontierInfrastructure(*I);}))continue;
                if(!UseI||(!isa<GetElementPtrInst>(UseI)&&
                    !(isa<BinaryOperator>(UseI)&&cast<BinaryOperator>(UseI)->getOpcode()==Instruction::Add))) return false;
            }
    }
    for(BasicBlock *BB:Info.DriverLoop->blocks())for(Instruction &I:*BB)
        if(auto *L=dyn_cast<LoadInst>(&I))if(L->getPointerOperand()==Info.AppendCountPtr)
            for(User *U:L->users()) {
                auto *UseI=dyn_cast<Instruction>(U);
                if(UseI&&frontierInfrastructure(*UseI))continue;
                if(UseI&&isa<CastInst>(UseI)&&llvm::all_of(UseI->users(),[](const User *U){
                    auto *I=dyn_cast<Instruction>(U);return I&&frontierInfrastructure(*I);}))continue;
                if(!UseI||!AllowedCountUses.count(UseI))return false;
            }
    return Count==Increments;
}

PreservedAnalyses ArrayFrontierStoragePass::run(Function &F,FunctionAnalysisManager &FAM) {
    if(F.isDeclaration()||F.hasFnAttribute("sgpl.spatial.callback")||F.hasFnAttribute("sgpl.temporal.callback"))
        return PreservedAnalyses::all();
    auto &LI=FAM.getResult<LoopAnalysis>(F);auto &DT=FAM.getResult<DominatorTreeAnalysis>(F);
    SmallPtrSet<Value *,8> OwnedSlots;bool Changed=false;
    for(Loop *L:LI.getLoopsInPreorder()) {
        NeighborLoopInfo Info;
        if(!analyzeNeighborLoop(L,Info)||!Info.FrontierArrayPtr||!Info.HasFrontierAppend)continue;
        SmallVector<StoreInst *,4> Sites;
        if(!checkedArrayAppend(Info,&Sites))continue;
        if(Sites.front()->getMetadata("sgpl.frontier.bound"))continue;
        BasicBlock *Pre=Info.DriverLoop->getLoopPreheader();
        if(!Pre)continue;
        Value *Cap=frontierCapacitySlot(Info.AppendArrayPtr);
        // The supported slots have function lifetime. Reject a pointer escape
        // rather than silently relocating storage that an observer can retain.
        if(!frontierMovableSlot(Info.AppendArrayPtr)||!frontierMovableSlot(Info.FrontierArrayPtr))continue;
        Module *M=F.getParent();LLVMContext &C=M->getContext();
        Type *Ptr=PointerType::get(C,0),*I32=Type::getInt32Ty(C),*I64=Type::getInt64Ty(C);
        IRBuilder<> B(Pre->getTerminator());
        auto Require=M->getOrInsertFunction("sgpl_exec_require_status",FunctionType::get(Type::getVoidTy(C),{I32},false));
        auto Reserve=M->getOrInsertFunction("sgpl_frontier_reserve_i32",FunctionType::get(I32,{Ptr,Ptr,I32,Ptr,Ptr,I32,I32},false));
        auto Ensure=M->getOrInsertFunction("sgpl_frontier_ensure_i32",FunctionType::get(I32,{Ptr,Ptr,I64},false));
        Value *Graph=Info.GraphPtr;
        if(auto *Load=dyn_cast<LoadInst>(Graph)) if(Info.DriverLoop->contains(Load))
            Graph=B.CreateLoad(Ptr,Load->getPointerOperand());
        if(auto *I=dyn_cast<Instruction>(Graph)) if(!DT.dominates(I,Pre->getTerminator()))continue;
        Value *Input=B.CreateLoad(Ptr,Info.FrontierArrayPtr);
        Value *Count=B.CreateLoad(I32,Info.FrontierSizePtr);
        auto *R=B.CreateCall(Reserve,{Graph,Input,Count,Info.AppendArrayPtr,Cap,B.getInt32(Sites.size()),B.CreateLoad(I32,Info.AppendCountPtr)});
        R->setMetadata("sgpl.frontier.infrastructure",MDNode::get(C,{}));
        auto *Check=B.CreateCall(Require,{R});Check->setMetadata("sgpl.frontier.infrastructure",MDNode::get(C,{}));
        for(StoreInst *S:Sites) {
            frontierAppendGuard(S,Info.AppendArrayPtr);
            S->setMetadata("sgpl.frontier.bound",MDNode::get(C,{}));
        }
        OwnedSlots.insert(Info.AppendArrayPtr);OwnedSlots.insert(Info.FrontierArrayPtr);Changed=true;
        if(getenv("GRAPH_FRONTIER_STATS"))errs()<<"[frontier-storage] bound=sum(degree(frontier[i]))*"<<Sites.size()
            <<" fallback=checked-copy-grow occurrences=preserved driver="<<Info.DriverLoop->getHeader()->getName()<<"\n";
    }
    /* Install guards before O3 can unroll a source loop. A bound requires a
     * recognized driver; storage safety does not require a parallel proof. */
    for(Loop *L:LI.getLoopsInPreorder()) {
        Value *Destination=nullptr;
        for(BasicBlock *BB:L->blocks())for(Instruction &I:*BB)
            if(auto *Call=dyn_cast<CallInst>(&I))if(isIteratorNext(Call)&&Call->arg_size()==2)
                Destination=Call->getArgOperand(1);
        if(!Destination)continue;
        SmallVector<std::pair<StoreInst *,Value *>,8> Appends;
        for(BasicBlock *BB:L->blocks())for(Instruction &I:*BB) {
            auto *S=dyn_cast<StoreInst>(&I);auto *G=S?dyn_cast<GetElementPtrInst>(S->getPointerOperand()):nullptr;
            auto *Payload=S?dyn_cast<LoadInst>(S->getValueOperand()):nullptr;
            auto *Index=G?dyn_cast_or_null<LoadInst>(frontierInteger(primaryIndex(G))):nullptr;
            Value *Slot=G?const_cast<Value *>(canonicalArrayBase(G)):nullptr;
            if(!G||!Payload||Payload->getPointerOperand()!=Destination||!Index||
                !S->getValueOperand()->getType()->isIntegerTy(32)||S->getMetadata("sgpl.frontier.checked"))continue;
            bool Increment=false;
            for(Instruction *J=S->getNextNode();J&&!J->isTerminator();J=J->getNextNode())
                if(auto *Count=dyn_cast<StoreInst>(J))if(Count->getPointerOperand()==Index->getPointerOperand()) {
                    auto *Add=dyn_cast<BinaryOperator>(Count->getValueOperand());
                    auto *Read=Add?dyn_cast<LoadInst>(Add->getOperand(0)):nullptr;
                    auto *One=Add?dyn_cast<ConstantInt>(Add->getOperand(1)):nullptr;
                    Increment=Add&&Add->getOpcode()==Instruction::Add&&Read&&
                        Read->getPointerOperand()==Index->getPointerOperand()&&One&&One->isOne();break;
                }
            if(Increment&&frontierMovableSlot(Slot))Appends.push_back({S,Slot});
        }
        for(auto Site:Appends){frontierAppendGuard(Site.first,Site.second);Changed=true;}
    }
    if(Changed&&F.getName()=="main") {
        bool Present=false;
        for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))
            if(Function *Fn=Call->getCalledFunction())Present|=Fn->getName()=="sgpl_frontier_release_buffers";
        if(Present)return PreservedAnalyses::none();
        auto Release=F.getParent()->getOrInsertFunction("sgpl_frontier_release_buffers",
            FunctionType::get(Type::getVoidTy(F.getContext()),{},false));
        for(BasicBlock &BB:F) if(auto *Ret=dyn_cast<ReturnInst>(BB.getTerminator())) {
            IRBuilder<> B(Ret);B.CreateCall(Release);
        }
    }
    return Changed?PreservedAnalyses::none():PreservedAnalyses::all();
}
