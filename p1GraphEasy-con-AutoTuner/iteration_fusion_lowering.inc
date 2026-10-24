/* Fuse a pure, source-local map into the producer of that source's next rank.
 * No algebraic substitution: clone the map's exact arithmetic and row guard. */
static Function *iterationFusedKernel(Module &M,Function *Kernel,Function *Map,
    GlobalVariable *Source,GlobalVariable *Destination) {
  // Caller established locality and disjointness before replacing the plan's
  // initializer (the old, unused LLVM constant can still appear in slot users).
  // These clones only remove the immutable gate; address provenance is intact.
  PHINode *U=nullptr;
  for(BasicBlock &BB:*Map)for(PHINode &Phi:BB.phis()) {
    if(U||!Phi.getType()->isIntegerTy(64))return nullptr;U=&Phi;
  }
  if(!U||U->getNumIncomingValues()!=2)return nullptr;
  BasicBlock *Header=U->getParent(),*Entry=&Map->getEntryBlock();
  int E=U->getBasicBlockIndex(Entry);if(E<0||U->getIncomingValue(E)!=Map->getArg(6))return nullptr;
  auto *Test=dyn_cast<BranchInst>(Header->getTerminator());
  auto *Cmp=Test&&Test->isConditional()?dyn_cast<ICmpInst>(Test->getCondition()):nullptr;
  if(!Cmp||Cmp->getPredicate()!=ICmpInst::ICMP_SLT||Cmp->getOperand(0)!=U||Cmp->getOperand(1)!=Map->getArg(2))return nullptr;
  BasicBlock *Latch=U->getIncomingBlock(1-E);auto *Back=dyn_cast<BranchInst>(Latch->getTerminator());
  if(!Back||!Back->isUnconditional()||Back->getSuccessor(0)!=Header)return nullptr;
  StoreInst *Output=nullptr;
  for(BasicBlock &BB:*Kernel)for(Instruction &I:BB)if(auto *S=dyn_cast<StoreInst>(&I)) {
    if(Output||S->isVolatile()||S->isAtomic()||executionArraySlot(S->getPointerOperand())!=Destination)return nullptr;
    Output=S;
  }
  auto *Address=Output?dyn_cast<GetElementPtrInst>(Output->getPointerOperand()):nullptr;
  if(!Address||Address->getNumIndices()!=1||!Output->getValueOperand()->getType()->isDoubleTy()||
     !Address->getOperand(1)->getType()->isIntegerTy(64))return nullptr;
  SmallVector<Type *,8> Types;for(Argument &A:Map->args())Types.push_back(A.getType());Types.push_back(Type::getDoubleTy(M.getContext()));
  auto *Cell=Function::Create(FunctionType::get(Type::getVoidTy(M.getContext()),Types,false),GlobalValue::InternalLinkage,"sgpl.iteration.contribution_cell",M);
  ValueToValueMapTy VM;unsigned J=0;for(Argument &A:Map->args())VM[&A]=Cell->getArg(J++);
  SmallVector<ReturnInst *,4> Returns;CloneFunctionInto(Cell,Map,VM,CloneFunctionChangeType::LocalChangesOnly,Returns);
  // The helper denotes exactly one existing map iteration. Preserve its row
  // predicate, including skipping zero-outdegree vertices without division.
  auto *CellU=cast<PHINode>(VM[U]);CellU->replaceAllUsesWith(Cell->getArg(6));CellU->eraseFromParent();
  auto *CellTest=cast<BranchInst>(VM[Test]);
  CellTest->getSuccessor(1)->removePredecessor(CellTest->getParent());
  BranchInst::Create(CellTest->getSuccessor(0),CellTest);CellTest->eraseFromParent();
  auto *CellBack=cast<BranchInst>(VM[Back]);ReturnInst::Create(M.getContext(),nullptr,CellBack);CellBack->eraseFromParent();
  for(BasicBlock &BB:*Cell)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))
    if(!L->getType()->isPointerTy()&&executionArraySlot(L->getPointerOperand())==Source)L->replaceAllUsesWith(Cell->getArg(7));
  if(verifyFunction(*Cell,&errs()))report_fatal_error("invalid singleton contribution map");
  iterationCanonicalize(Cell);
  Types.clear();for(Argument &A:Kernel->args())Types.push_back(A.getType());Types.push_back(PointerType::getUnqual(M.getContext()));
  auto *F=Function::Create(FunctionType::get(Type::getVoidTy(M.getContext()),Types,false),GlobalValue::InternalLinkage,"sgpl.iteration.fused_contributions",M);
  VM.clear();J=0;for(Argument &A:Kernel->args())VM[&A]=F->getArg(J++);Returns.clear();
  CloneFunctionInto(F,Kernel,VM,CloneFunctionChangeType::LocalChangesOnly,Returns);
  auto *Store=cast<StoreInst>(VM[Output]);auto *G=cast<GetElementPtrInst>(Store->getPointerOperand());
  IRBuilder<> B(Store);Value *Index=G->getOperand(1);
  auto *Call=B.CreateCall(Cell,{F->getArg(0),F->getArg(1),B.CreateAdd(Index,B.getInt64(1)),F->getArg(8),
    ConstantPointerNull::get(B.getPtrTy()),G->getPointerOperand(),Index,Store->getValueOperand()});
  Store->eraseFromParent();InlineFunctionInfo IFI;
  if(!InlineFunction(*Call,IFI).isSuccess()){F->eraseFromParent();Cell->eraseFromParent();return nullptr;}
  Cell->eraseFromParent();iterationCanonicalize(F);F->setSection(".sgpl_iteration_memory");
  if(verifyFunction(*F,&errs()))report_fatal_error("invalid fused contribution kernel");
  return F;
}
