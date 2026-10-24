/* General source-count, post-iteration liveness and source-map lifetime proofs.
 * This file is included after the existing private-allocation checks. */
static bool iterationBoundCallback(Function *F,Loop *Outer) {
  if(!F)return true;
  for(User *U:F->users()) {
    auto *C=dyn_cast<CallInst>(U);
    if(!C||!Outer->contains(C)||!C->getCalledFunction()){if(std::getenv("SGPL_LIFETIME_DEBUG"))errs()<<"unbound "<<F->getName()<<" user "<<*U<<"\n";return false;}
    auto N=C->getCalledFunction()->getName();
    if(N!="autograph_exec_op_create"&&N!="sgpl_fullgraph_certificate"&&N!="sgpl_schedule_certificate"){if(std::getenv("SGPL_LIFETIME_DEBUG"))errs()<<"unbound callback consumer "<<N<<"\n";return false;}
  }
  return true;
}
static bool iterationDeadArray(GlobalVariable *Slot,Loop *Outer,Function *Pair,Function *Gate) {
  // A syntactic last load of the global is insufficient: follow allocation
  // pointers and private stack aliases too, including preloaded pointers.
  if(!iterationBoundCallback(Pair,Outer)||!iterationBoundCallback(Gate,Outer)){if(std::getenv("SGPL_LIFETIME_DEBUG"))errs()<<"dead callback not bound "<<Slot->getName()<<"\n";return false;}
  SmallPtrSet<BasicBlock *,32> After;SmallVector<BasicBlock *,16> Blocks{Outer->getExitBlock()};
  while(!Blocks.empty()) {auto *B=Blocks.pop_back_val();if(!After.insert(B).second)continue;for(auto *N:successors(B))Blocks.push_back(N);}
  SmallPtrSet<Value *,32> Seen;SmallVector<Value *,32> Work;
  SmallPtrSet<Value *,4> Sites;if(!executionArrayAllocations(Slot,Sites))return false;
  for(auto *V:Sites)Work.push_back(V);
  for(User *U:Slot->users())if(auto *L=dyn_cast<LoadInst>(U))Work.push_back(L);
  auto Live=[&](Instruction *I) {
    if(I->getFunction()==Pair||I->getFunction()==Gate)return false;
    return I->getFunction()!=Outer->getHeader()->getParent()||After.count(I->getParent());
  };
  while(!Work.empty()) {
    Value *V=Work.pop_back_val();if(!Seen.insert(V).second)continue;
    for(User *U:V->users()) {
      if(isa<GetElementPtrInst>(U)||isa<BitCastInst>(U)){Work.push_back(U);continue;}
      if(auto *L=dyn_cast<LoadInst>(U)){if(L->getPointerOperand()!=V||Live(L)){if(std::getenv("SGPL_LIFETIME_DEBUG"))errs()<<"live "<<Slot->getName()<<": "<<*L<<"\n";return false;}continue;}
      if(auto *S=dyn_cast<StoreInst>(U)) {
        if(S->getValueOperand()!=V){if(Live(S))return false;continue;}
        if(S->getPointerOperand()==Slot)continue;
        auto *A=dyn_cast<AllocaInst>(S->getPointerOperand());if(!A)return false;
        for(User *Use:A->users())if(auto *L=dyn_cast<LoadInst>(Use))Work.push_back(L);
          else if(!isa<StoreInst>(Use))return false;
        continue;
      }
      if(isa<ICmpInst>(U))continue; // identities are always restored
      if(std::getenv("SGPL_LIFETIME_DEBUG"))errs()<<"unknown pointer use "<<Slot->getName()<<": "<<*U<<"\n";return false;
    }
  }
  return true;
}
static bool iterationMapLocal(Function *Map,GlobalVariable *Source) {
  if(!Source||Map->arg_size()!=7||Map->hasFnAttribute(Attribute::StrictFP))return false;
  PHINode *U=nullptr;
  for(BasicBlock &BB:*Map)for(auto &Phi:BB.phis())if(Phi.getType()->isIntegerTy(64)) {
    if(U)return false;U=&Phi;
  }
  if(!U)return false;
  SmallPtrSet<Value *,4> SourceSites;if(!executionArrayAllocations(Source,SourceSites))return false;
  unsigned Reads=0,Stores=0;
  for(BasicBlock &BB:*Map)for(Instruction &I:BB) {
    if(auto *S=dyn_cast<StoreInst>(&I)) {
      if(S->isAtomic()||S->isVolatile()||!S->getValueOperand()->getType()->isDoubleTy())return false;
      auto *G=dyn_cast<GetElementPtrInst>(S->getPointerOperand());
      if(!G||G->getPointerOperand()!=Map->getArg(5)||G->getNumIndices()!=1||G->getOperand(1)!=U)return false;
      ++Stores;continue;
    }
    if(auto *L=dyn_cast<LoadInst>(&I)) {
      if(L->isAtomic()||L->isVolatile())return false;
      if(auto *Slot=executionArraySlot(L->getPointerOperand())) {
        if(Slot==Source) {
          auto *G=dyn_cast<GetElementPtrInst>(L->getPointerOperand());
          Value *Index=G?G->getOperand(1):nullptr;
          if(auto *Ext=dyn_cast_or_null<SExtInst>(Index))Index=Ext->getOperand(0);
          bool Local=Index&&(iterationIndex(Index)==U||match(Index,m_AShr(m_Shl(m_Specific(U),m_SpecificInt(32)),m_SpecificInt(32))));
          // The checked invocation bounds U to [0, INT32_MAX), so LLVM's
          // canonical sign-extension of trunc(U) denotes exactly this cell.
          if(!G||!L->getType()->isDoubleTy()||G->getNumIndices()!=1||!Local)return false;
          ++Reads;
        } else {
          SmallPtrSet<Value *,4> Other;if(!executionArrayAllocations(Slot,Other))return false;
          for(Value *Site:Other)if(SourceSites.count(Site))return false;
        }
      } else if(!isa<GlobalVariable>(L->getPointerOperand())) {
        // Only row and gate reads have non-global provenance in generated maps.
        Value *Base=getUnderlyingObject(L->getPointerOperand());
        if(Base!=Map->getArg(3)&&Base!=Map->getArg(4))return false;
      }
    }
    if(isa<CallBase>(I)||isa<AtomicRMWInst>(I)||isa<AtomicCmpXchgInst>(I)||isa<FenceInst>(I))return false;
  }
  return Reads>0&&Stores==1;
}
static bool iterationKernelDisjoint(Function *Kernel,GlobalVariable *Source) {
  // Locality of the map alone is insufficient: a contribution can contain
  // both source-indexed reads (hoisted) and destination-indexed reads (left
  // in the kernel). Only the explicit map-value argument may alias old ranks.
  SmallPtrSet<Value *,4> Sites;if(!executionArrayAllocations(Source,Sites))return false;
  for(BasicBlock &BB:*Kernel)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I)) {
    if(L->isAtomic()||L->isVolatile())return false;
    if(L->getType()->isPointerTy())continue;
    if(auto *Slot=executionArraySlot(L->getPointerOperand())) {
      if(Slot==Source)return false;
      SmallPtrSet<Value *,4> Other;if(!executionArrayAllocations(Slot,Other))return false;
      for(Value *Site:Other)if(Sites.count(Site))return false;
    }
  }
  return true;
}
static void certifySourceCounts(Module &M) {
  if(!(executionSimplifyRules()&SGPL_EXEC_R16))return;
  SmallVector<CallInst *,8> Executes;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))
    if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="autograph_frontier_execute")Executes.push_back(C);
  for(auto *Execute:Executes) {
    auto Refuse=[&](StringRef Why){if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-lifetime] R16 refuse: "<<Why<<"\n";};
    auto *Ctx=dyn_cast<CallInst>(Execute->getArgOperand(1));
    if(!Ctx||!Ctx->getCalledFunction()||Ctx->getCalledFunction()->getName()!="autograph_exec_ctx_create"||Ctx->arg_size()!=13||
       Execute->getArgOperand(0)!=Ctx->getArgOperand(0)||!match(Ctx->getArgOperand(11),m_One())||!match(Ctx->getArgOperand(12),m_One()))continue;
    bool Safe=true;
    for(unsigned N:{1u,2u,6u,9u})Safe&=match(Ctx->getArgOperand(N),m_Zero());
    for(unsigned N:{3u,4u,5u,7u,8u})Safe&=isa<ConstantPointerNull>(Ctx->getArgOperand(N));
    unsigned Runs=0,Destroys=0;
    for(User *U:Ctx->users()) {
      auto *C=dyn_cast<CallInst>(U);auto *F=C?C->getCalledFunction():nullptr;if(!F){Safe=false;continue;}
      auto N=F->getName();
      if(N=="autograph_frontier_execute"){++Runs;Safe&=C==Execute;}
      else if(N=="autograph_exec_ctx_destroy")++Destroys;
      else if(N!="autograph_exec_ctx_set_dag_axes"&&N!="autograph_exec_ctx_prepare_spatial")Safe=false;
    }
    auto *Ops=dyn_cast<AllocaInst>(getUnderlyingObject(Ctx->getArgOperand(10)));CallInst *Op=nullptr;
    if(Ops)for(BasicBlock &BB:*Ctx->getFunction())for(Instruction &I:BB)if(auto *S=dyn_cast<StoreInst>(&I))
      if(getUnderlyingObject(S->getPointerOperand())==Ops){if(Op)Safe=false;Op=dyn_cast<CallInst>(S->getValueOperand());}
    if(!Safe||Runs!=1||Destroys!=1||!Op||!Op->getCalledFunction()||Op->getCalledFunction()->getName()!="autograph_exec_op_create"){Refuse("complete unobserved full-domain invocation unavailable");continue;}
    for(unsigned N:{1u,3u,4u,5u,6u,7u})Safe&=isa<ConstantPointerNull>(Op->getArgOperand(N));
    Safe&=match(Op->getArgOperand(0),m_One())&&match(Op->getArgOperand(8),m_Zero());
    auto *Pair=dyn_cast<Function>(Op->getArgOperand(2));if(!Safe||!Pair)continue;
    ValueToValueMapTy VM;auto *Proof=CloneFunction(Pair,VM);iterationCanonicalize(Proof);
    GlobalVariable *Slot=nullptr;StoreInst *Write=nullptr;LoadInst *Read=nullptr;
    Safe=executionCallbackSafe(Proof)&&Proof->getArg(3)->use_empty();
    for(BasicBlock &BB:*Proof)for(Instruction &I:BB)if(auto *S=dyn_cast<StoreInst>(&I)){if(Write)Safe=false;Write=S;}
    auto *Add=Write?dyn_cast<BinaryOperator>(Write->getValueOperand()):nullptr;
    if(Add&&Add->getOpcode()==Instruction::Add&&Add->getType()->isIntegerTy(32)&&!Add->hasNoSignedWrap()&&!Add->hasNoUnsignedWrap()) {
      Read=dyn_cast<LoadInst>(Add->getOperand(0));Safe&=match(Add->getOperand(1),m_One());
    } else Safe=false;
    auto *WG=Write?dyn_cast<GetElementPtrInst>(Write->getPointerOperand()):nullptr;
    auto *RG=Read?dyn_cast<GetElementPtrInst>(Read->getPointerOperand()):nullptr;
    auto SourceIndex=[&](Value *V){if(auto *Ext=dyn_cast<SExtInst>(V))V=Ext->getOperand(0);return V==Proof->getArg(2);};
    if(WG&&RG&&WG->getNumIndices()==1&&RG->getNumIndices()==1&&WG->getSourceElementType()->isIntegerTy(32)&&
       SourceIndex(WG->getOperand(1))&&SourceIndex(RG->getOperand(1))) {
      Slot=executionArraySlot(WG);Safe&=Slot&&executionArraySlot(RG)==Slot;
    } else Safe=false;
    for(BasicBlock &BB:*Proof)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))
      Safe&=L==Read||(L->getType()->isPointerTy()&&L->getPointerOperand()==Slot);
    Safe&=Slot&&iterationPrivateArray(Slot);Proof->eraseFromParent();
    if(!Safe){if(std::getenv("SGPL_LIFETIME_DEBUG")){errs()<<"count slot="<<(Slot?Slot->getName():"null")<<" private="<<(Slot&&iterationPrivateArray(Slot))<<"\n";Pair->print(errs());}Refuse("source-local modular increment and private tracking storage unavailable");continue;}
    auto Composed=sgpl_iteration_lifetimes(SGPL_FACT_COUNT_PURE,executionSimplifyRules());if(!(Composed.rules&SGPL_EXEC_R16))continue;
    IRBuilder<> B(Execute);auto *P=B.getPtrTy();auto *I=B.getInt32Ty();
    auto *Result=B.CreateCall(M.getOrInsertFunction("sgpl_try_source_count_v1",FunctionType::get(I,{P,P},false)),{Execute->getArgOperand(0),Slot});
    Result->setMetadata("sgpl.source.count.proof",MDNode::get(M.getContext(),{}));
    // Runtime refusal precedes all count effects. The original context and
    // callbacks remain intact; profile and status handling keep their positions.
    auto *Tail=Execute->getParent()->splitBasicBlock(Execute,"row.count.baseline");
    auto *Head=Tail->getSinglePredecessor();auto *After=Tail->splitBasicBlock(Execute->getNextNode(),"row.count.join");
    Head->getTerminator()->eraseFromParent();B.SetInsertPoint(Head);B.CreateCondBr(B.CreateICmpNE(Result,B.getInt32(0)),After,Tail);
    auto *Phi=PHINode::Create(I,2,"row.count.status",&*After->begin());Execute->replaceAllUsesWith(Phi);Phi->addIncoming(B.getInt32(0),Head);Phi->addIncoming(Execute,Tail);
    if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-lifetime] R16 certify: private modular source count; existing row lengths; baseline retained\n";
  }
  if(verifyModule(M,&errs()))report_fatal_error("invalid source count simplification");
}
