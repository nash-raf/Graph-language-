
/* Pair-body demand proof; runtime separately checks every lifecycle/gate
 * consumer. Opaque uses, captures, writes, variable offsets and wide context
 * accesses refuse. Runtime leaf summaries are part of the execution ABI. */
static bool sgplSourceOrdinalDead(llvm::Function *F,unsigned ArgNo,
    llvm::SmallPtrSetImpl<llvm::Function *> &Active) {
  if(!F || F->isDeclaration() || ArgNo>=F->arg_size() || !Active.insert(F).second)return false;
  auto Finish=[&](bool value){Active.erase(F);return value;};
  auto &DL=F->getParent()->getDataLayout();
  llvm::SmallVector<llvm::Value *,16> Pending{F->getArg(ArgNo)};
  llvm::SmallPtrSet<llvm::Value *,32> Seen;
  while(!Pending.empty()) {
    llvm::Value *V=Pending.pop_back_val();if(!Seen.insert(V).second)continue;
    for(llvm::User *U:V->users()) {
      if(auto *G=llvm::dyn_cast<llvm::GetElementPtrInst>(U)) {
        int64_t Offset=0;auto *Base=llvm::GetPointerBaseWithConstantOffset(G,Offset,DL);
        if(Base!=F->getArg(ArgNo))return Finish(false);
        Pending.push_back(G);continue;
      }
      if(auto *Cast=llvm::dyn_cast<llvm::BitCastInst>(U)){Pending.push_back(Cast);continue;}
      if(auto *L=llvm::dyn_cast<llvm::LoadInst>(U)) {
        if(L->getPointerOperand()!=V)return Finish(false);
        int64_t Offset=0;auto *Base=llvm::GetPointerBaseWithConstantOffset(V,Offset,DL);
        if(Base!=F->getArg(ArgNo) || Offset<0 || L->isVolatile() || L->isAtomic())return Finish(false);
        auto Size=DL.getTypeStoreSize(L->getType());if(Size.isScalable())return Finish(false);
        uint64_t At=(uint64_t)Offset,N=Size.getFixedValue(),Field=offsetof(sgpl_exec_ctx,array_source_ordinal);
        uint64_t Gate=offsetof(sgpl_exec_ctx,spatial_source_gate);
        if(At<Gate+sizeof(void *) && Gate<At+N)return Finish(false);
        if(At>sizeof(sgpl_exec_ctx) || N>sizeof(sgpl_exec_ctx)-At ||
           (At<Field+sizeof(int64_t) && Field<At+N))return Finish(false);

        /* Reject representation observers: a dead context ordinal alone
         * does not prove that raw record payloads/capabilities are unread.
         * The format load introduced by our known weight accessor has
         * explicitly versioned semantics, and carries trusted provenance. */
        const uint64_t PointerFields[]={
          offsetof(sgpl_exec_ctx,gpu_records),offsetof(sgpl_exec_ctx,gpu_record_sources),
          offsetof(sgpl_exec_ctx,gpu_record_work),offsetof(sgpl_exec_ctx,execution_scratch),
          offsetof(sgpl_exec_ctx,ops)};
        for(auto Field:PointerFields)
          if(At<Field+sizeof(void *) && Field<At+N)return Finish(false);
        uint64_t Format=offsetof(sgpl_exec_ctx,array_compact_records);
        uint64_t Edges=offsetof(sgpl_exec_ctx,array_record_edges);
        if((At<Format+sizeof(int32_t) && Format<At+N) ||
           (At<Edges+sizeof(void *) && Edges<At+N)) {
          auto *Tag=L->getMetadata("sgpl.ctx.accessor");
          auto *Name=Tag&&Tag->getNumOperands()==1?llvm::dyn_cast<llvm::MDString>(Tag->getOperand(0)):nullptr;
          if(!Name||Name->getString()!="autograph_exec_ctx_edge_weight")return Finish(false);
        }
        continue;
      }
      if(llvm::isa<llvm::ICmpInst>(U))continue;
      auto *Call=llvm::dyn_cast<llvm::CallBase>(U);
      auto *Callee=Call?Call->getCalledFunction():nullptr;
      if(!Callee || Call->getNumOperandBundles())return Finish(false);
      bool Matched=false;
      for(unsigned I=0;I<Call->arg_size();++I)if(Call->getArgOperand(I)==V) {
        Matched=true;
        int64_t Offset=0;auto *Base=llvm::GetPointerBaseWithConstantOffset(V,Offset,DL);
        if(Base!=F->getArg(ArgNo) || Offset!=0)return Finish(false);
        auto N=Callee->getName();
        if(I==0 && (N=="autograph_frontier_activate" ||
                    N=="autograph_exec_ctx_snapshot" ||
                    N=="autograph_exec_ctx_edge_weight"))continue;
        if(!Callee->hasLocalLinkage() || !sgplSourceOrdinalDead(Callee,I,Active))return Finish(false);
      }
      if(!Matched || Call->getCalledOperand()==V)return Finish(false);
    }
  }
  return Finish(true);
}
static void certifyRecordFieldLiveness(llvm::Module &M) {
  if(std::getenv("SGPL_NO_RECORD_LIVENESS"))return;
  unsigned Certified=0,Refused=0;
  for(auto &F:M)for(auto &BB:F)for(auto &I:BB) {
    auto *Call=llvm::dyn_cast<llvm::CallBase>(&I);
    auto *Callee=Call?Call->getCalledFunction():nullptr;
    if(!Callee || Callee->getName()!="autograph_exec_op_create" || Call->arg_size()<3)continue;
    auto *Caps=llvm::dyn_cast<llvm::ConstantInt>(Call->getArgOperand(0));
    auto *Pair=llvm::dyn_cast<llvm::Function>(Call->getArgOperand(2)->stripPointerCasts());
    if(!Caps || Caps->getZExtValue()!=SGPL_OP_PAIR || !Pair ||
       Pair->arg_size()!=4 || !Pair->hasFnAttribute("sgpl.spatial.callback"))continue;
    llvm::SmallPtrSet<llvm::Function *,16> Active;
    if(!sgplSourceOrdinalDead(Pair,1,Active)){
      ++Refused;
      if(std::getenv("SGPL_RECORD_LIVENESS_STATS"))llvm::errs()<<"[record-liveness] refuse "<<Pair->getName()<<": observable context/ordinal/representation use\n";
      continue;
    }
    Call->setArgOperand(0,llvm::ConstantInt::get(Caps->getType(),Caps->getZExtValue()|SGPL_OP_NO_SOURCE_ORDINAL));
    Pair->addFnAttr("sgpl.record.source_ordinal.dead");++Certified;
    if(std::getenv("SGPL_RECORD_LIVENESS_STATS"))llvm::errs()<<"[record-liveness] activate "<<Pair->getName()<<": source ordinal unobservable\n";
  }
  if(std::getenv("SGPL_RECORD_LIVENESS_STATS"))
    llvm::errs()<<"[record-liveness] certified="<<Certified<<" refused="<<Refused<<"\n";
  if(Certified) {
    auto *Entry=M.getFunction("main");
    if(!Entry||Entry->empty())llvm::report_fatal_error("execution ABI check requires main");
    llvm::IRBuilder<> B(&*Entry->getEntryBlock().getFirstInsertionPt());
    auto Check=M.getOrInsertFunction("autograph_execution_abi_check_v1",
      llvm::FunctionType::get(B.getVoidTy(),{B.getInt32Ty(),B.getInt64Ty(),B.getInt64Ty()},false));
    B.CreateCall(Check,{B.getInt32(SGPL_EXECUTION_ABI_VERSION),B.getInt64(sizeof(sgpl_exec_ctx)),B.getInt64(sizeof(sgpl_runtime_op))});
  }
}
