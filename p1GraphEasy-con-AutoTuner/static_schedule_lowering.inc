/* Lower a certified symbolic DAG, not a benchmark or algorithm name.
 * Source preparation -> independent owners -> join. The effect certificate
 * comes from the ordinary spatial analysis. Fail closed on context observers.
 * There is deliberately no new dependency discharge in this pass: it consumes
 * the dependencies remaining after ownership/snapshot/privatization proofs. */
namespace {
static bool scheduleCallbackContextFree(llvm::Function *F, bool Gate) {
  using namespace llvm;
  if(!F || F->isDeclaration() || F->arg_size()!=(Gate?3u:4u))return false;
  if(!Gate && (F->size()!=1 || !F->getArg(1)->use_empty()))return false;
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    for(Value *V:I.operand_values()) {
      auto *G=dyn_cast<GlobalVariable>(V->stripPointerCasts());
      if(G && G->isThreadLocal())return false;
    }
    if(auto *L=dyn_cast<LoadInst>(&I)) {
      if(L->isAtomic() || L->isVolatile())return false;
      continue;
    }
    if(auto *S=dyn_cast<StoreInst>(&I)) {
      if(Gate || S->isAtomic() || S->isVolatile())return false;
      continue;
    }
    if(auto *Call=dyn_cast<CallInst>(&I)) {
      auto *C=Call->getCalledFunction();
      if(!Gate || !C || C->getName()!="autograph_exec_ctx_set_source_gate" ||
         Call->arg_size()!=3 || Call->getArgOperand(0)!=F->getArg(1) ||
         Call->getArgOperand(1)!=F->getArg(2) || Call->getNumOperandBundles())return false;
      continue;
    }
    if(!(isa<ReturnInst>(I)||isa<BranchInst>(I)||isa<PHINode>(I)||
         isa<GetElementPtrInst>(I)||isa<CastInst>(I)||isa<BinaryOperator>(I)||
         isa<UnaryOperator>(I)||isa<CmpInst>(I)||isa<SelectInst>(I)))return false;
  }
  if(Gate) {
    if(!F->getArg(0)->use_empty())return false;
    for(User *U:F->getArg(1)->users()) {
      auto *C=dyn_cast<CallInst>(U);
      if(!C || !C->getCalledFunction() ||
         C->getCalledFunction()->getName()!="autograph_exec_ctx_set_source_gate" ||
         C->getArgOperand(0)!=F->getArg(1))return false;
    }
  }
  return true;
}
static void lowerStaticSchedules(llvm::Module &M) {
  using namespace llvm;
  SmallVector<CallInst *,8> Markers;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)
    if(auto *C=dyn_cast<CallInst>(&I))if(C->getCalledFunction() &&
      C->getCalledFunction()->getName()=="sgpl_schedule_certificate")Markers.push_back(C);
  for(CallInst *C:Markers) {
    auto *Pair=dyn_cast<Function>(C->getArgOperand(1)->stripPointerCasts());
    auto *Gate=dyn_cast<Function>(C->getArgOperand(2)->stripPointerCasts());
    bool NoGate=isa<ConstantPointerNull>(C->getArgOperand(2));
    bool Enable=!std::getenv("SGPL_NO_STATIC_SCHEDULE_LOWERING") &&
      scheduleCallbackContextFree(Pair,false) &&
      (NoGate || scheduleCallbackContextFree(Gate,true));
    if(Enable) {
      IRBuilder<> B(C);auto *P=B.getPtrTy();auto *I32=B.getInt32Ty();
      auto *T=StructType::get(M.getContext(),{I32,I32,I32,I32,P,P});
      if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_static_schedule_v1))
        report_fatal_error("static schedule descriptor layout mismatch");
      auto *Plan=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,
        ConstantStruct::get(T,{B.getInt32(SGPL_STATIC_SCHEDULE_VERSION),
          B.getInt32(SGPL_SCHEDULE_PREPARE_OWNERS_JOIN),
          B.getInt32(SGPL_SCHEDULE_EFFECT_PROOF|SGPL_SCHEDULE_CONTEXT_PROOF),
          B.getInt32(4),Pair,Gate?static_cast<Constant *>(Gate):ConstantPointerNull::get(P)}),
        "sgpl.schedule.prepare_owners_join");
      auto Bind=M.getOrInsertFunction("autograph_exec_ctx_bind_static_schedule_v1",
        FunctionType::get(B.getVoidTy(),{P,P,I32,B.getInt64Ty(),B.getInt64Ty()},false));
      B.CreateCall(Bind,{C->getArgOperand(0),Plan,B.getInt32(SGPL_EXECUTION_ABI_VERSION),
        B.getInt64(sizeof(sgpl_exec_ctx)),B.getInt64(sizeof(sgpl_runtime_op))});
      M.getOrInsertNamedMetadata("sgpl.static.schedule.proofs")->addOperand(MDNode::get(M.getContext(),{
        MDString::get(M.getContext(),"prepare-owners-join"),ValueAsMetadata::get(Pair),
        MDString::get(M.getContext(),"frontend: full-domain, immutable topology, destination ownership, separated source reads"),
        MDString::get(M.getContext(),"late: callbacks cannot observe worker/TLS/budget; source writes restricted to gate setter"),
        MDString::get(M.getContext(),"retained: source occurrence order; destination contribution order; prepare-before-owner visibility; joined completion")}));
    }
    if(std::getenv("SGPL_STATIC_SCHEDULE_STATS"))
      errs()<<"[static-schedule] "<<(Enable?"activate ":"refuse ")
        <<(Pair?Pair->getName():StringRef("unknown"))
        <<(Enable?": prepare -> independent owners -> join; source order, owner order, visibility, completion retained\n"
                 :": disabled or callback/context proof unavailable\n");
    C->eraseFromParent();
  }
  if(auto *F=M.getFunction("sgpl_schedule_certificate"))if(F->use_empty())F->eraseFromParent();
}
}
