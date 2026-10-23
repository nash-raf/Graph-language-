/* Separate certificates: callback locality (R10), ordered private accumulator
 * (R11), and complete unobserved convergence with a finite-lattice operation
 * (R12). No algebraic property is inferred from a function/algorithm name. */
#include "execution_simplification_worklist.h"
namespace {
using namespace llvm;
struct SemanticRecipe {
  Function *Pair=nullptr;GlobalVariable *Array=nullptr,*WeightGraph=nullptr;
  unsigned Bytes=0,Slot=0,Join=0;bool Weighted=false,Minimum=false;
};
static bool semanticAddress(Value *V,GlobalVariable *Array,Value *Index) {
  auto *G=dyn_cast<GetElementPtrInst>(V);auto *L=G?dyn_cast<LoadInst>(G->getPointerOperand()):nullptr;
  return L&&L->getPointerOperand()==Array&&G->getNumIndices()==1&&frontierSourceIndex(G->idx_begin()->get(),Index);
}
static SemanticRecipe semanticRecipe(Function *Pair,MDNode *Origins) {
  SemanticRecipe R;if(!Pair||!Origins||Pair->hasFnAttribute(Attribute::StrictFP))return R;
  ValueToValueMapTy VM;Function *F=CloneFunction(Pair,VM);
  auto Fail=[&](){F->eraseFromParent();return SemanticRecipe{};};
  if(!frontierInlineLocal(F))return Fail();executionCleanup(F);
  DominatorTree DT(*F);LoopInfo LI(DT);if(!LI.empty())return Fail();
  StoreInst *Store=nullptr;CallInst *Append=nullptr;
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    if(auto *S=dyn_cast<StoreInst>(&I)){if(Store||S->isAtomic()||S->isVolatile())return Fail();Store=S;}
    if(auto *C=dyn_cast<CallInst>(&I))if(auto *G=C->getCalledFunction())if(G->getName()=="autograph_frontier_activate") {
      if(Append||C->arg_size()!=2||C->getArgOperand(0)!=F->getArg(1)||C->getArgOperand(1)!=F->getArg(3)||!C->use_empty())return Fail();Append=C;
    }
  }
  if(!Store||!Append)return Fail();
  auto *G=dyn_cast<GetElementPtrInst>(Store->getPointerOperand());auto *Base=G?dyn_cast<LoadInst>(G->getPointerOperand()):nullptr;
  auto *Array=Base?dyn_cast<GlobalVariable>(Base->getPointerOperand()):nullptr;
  if(!Array||!Array->hasLocalLinkage()||!Array->getMetadata("sgpl.language.array")||!semanticAddress(Store->getPointerOperand(),Array,F->getArg(3)))return Fail();
  Type *T=Store->getValueOperand()->getType();if(!T->isIntegerTy(32)&&!T->isDoubleTy())return Fail();
  SmallPtrSet<Value *,4> Alloc;if(!executionArrayAllocations(Array,Alloc))return Fail();
  R.Array=Array;R.Bytes=T->isDoubleTy()?8:4;
  auto KnownWeighted=relaxRecipe(Pair,Origins);
  LoadInst *Source=nullptr,*Old=nullptr;
  const DataLayout &DL=F->getParent()->getDataLayout();
  bool NeedWeighted=false;
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    if(isa<InvokeInst>(I)||isa<IndirectBrInst>(I)||isa<SwitchInst>(I)||I.isEHPad()||
        isa<AtomicRMWInst>(I)||isa<AtomicCmpXchgInst>(I)||isa<FenceInst>(I))return Fail();
    if(auto *L=dyn_cast<LoadInst>(&I)) {
      if(L->isAtomic()||L->isVolatile())return Fail();
      if(L->getPointerOperand()==Array&&L->getType()->isPointerTy())continue;
      if(KnownWeighted.Array==Array&&KnownWeighted.Weighted&&L->getPointerOperand()==KnownWeighted.WeightGraph&&L->getType()->isPointerTy()){NeedWeighted=true;continue;}
      if(semanticAddress(L->getPointerOperand(),Array,F->getArg(3))&&L->getType()==T){if(Old)return Fail();Old=L;continue;}
      if(frontierSnapshotElement(L,F->getArg(1),F->getArg(2),DL)&&L->getType()==T) {if(Source)return Fail();Source=L;continue;}
      if(frontierCtxField(L->getPointerOperand(),F->getArg(1),offsetof(sgpl_exec_ctx,spatial_snapshots),DL)||
          frontierCtxField(L->getPointerOperand(),F->getArg(1),offsetof(sgpl_exec_ctx,spatial_snapshot_count),DL))continue;
      // The snapshot pointer's slot load is covered by its exact use-chain.
      auto *B=Source?cast<GetElementPtrInst>(Source->getPointerOperand()):nullptr;
      (void)B;
      if(L->getType()->isPointerTy()&&frontierReadOnlyPointers(L))continue;
      if(L->getMetadata("sgpl.relax.weight.accessor")){NeedWeighted=true;continue;}
      return Fail();
    }
    if(auto *C=dyn_cast<CallBase>(&I)) {
      auto *Fn=C->getCalledFunction();if(!Fn)return Fail();
      if(Fn->getName()=="autograph_frontier_activate")continue;
      if(Fn->getName()=="autograph_exec_ctx_edge_weight"&&C->getMetadata("sgpl.relax.weight.accessor")){NeedWeighted=true;continue;}
      if(!Fn->isIntrinsic()||(Fn->getIntrinsicID()!=Intrinsic::assume&&Fn->getIntrinsicID()!=Intrinsic::lifetime_start&&Fn->getIntrinsicID()!=Intrinsic::lifetime_end))return Fail();
    }
    if(auto *Op=dyn_cast<BinaryOperator>(&I))if(Op->getOpcode()==Instruction::SDiv||Op->getOpcode()==Instruction::UDiv||Op->getOpcode()==Instruction::SRem||Op->getOpcode()==Instruction::URem||
        Op->getOpcode()==Instruction::FDiv||Op->getOpcode()==Instruction::FRem||Op->isShift())return Fail();
  }
  if(!Source||!Old)return Fail();
  auto *SG=cast<GetElementPtrInst>(Source->getPointerOperand());auto *Snapshot=cast<LoadInst>(SG->getPointerOperand());int64_t Off=0;
  GetPointerBaseWithConstantOffset(Snapshot->getPointerOperand(),Off,DL);bool Origin=false;
  for(auto &O:Origins->operands())if(auto *N=dyn_cast<MDNode>(O))if(N->getNumOperands()==2) {
    auto *K=mdconst::dyn_extract<ConstantInt>(N->getOperand(0));auto *A=dyn_cast<ValueAsMetadata>(N->getOperand(1));
    Origin|=K&&A&&K->getZExtValue()==(uint64_t)Off/8&&A->getValue()==Array;
  }
  if(!Origin)return Fail();R.Slot=(unsigned)Off/8;
  if(NeedWeighted) {
    if(!KnownWeighted.Array||!KnownWeighted.Weighted||KnownWeighted.Array!=Array)return Fail();R.Weighted=true;R.WeightGraph=KnownWeighted.WeightGraph;
  }
  R.Minimum=KnownWeighted.Array==R.Array&&KnownWeighted.SnapshotSlot==R.Slot&&
      KnownWeighted.Weighted==R.Weighted&&KnownWeighted.WeightGraph==R.WeightGraph;
  // Only an exact strict-extremum transfer earns the finite-lattice license.
  BranchInst *EffectGuard=nullptr;
  if(!R.Weighted&&T->isIntegerTy(32)&&Store->getParent()==Append->getParent()&&Store->comesBefore(Append)&&Store->getParent()->hasNPredecessors(1)) {
    auto *Br=dyn_cast<BranchInst>(Store->getParent()->getSinglePredecessor()->getTerminator());
    auto *Cmp=Br&&Br->isConditional()?dyn_cast<ICmpInst>(Br->getCondition()):nullptr;
    if(Cmp&&Br->getSuccessor(0)==Store->getParent()) {
      EffectGuard=Br;
      if(Store->getValueOperand()==Source&&Cmp->getOperand(0)==Source&&Cmp->getOperand(1)==Old) {
        if(Cmp->getPredicate()==ICmpInst::ICMP_SLT)R.Join=SGPL_JOIN_SMIN;
        if(Cmp->getPredicate()==ICmpInst::ICMP_SGT)R.Join=SGPL_JOIN_SMAX;
      }
      if(auto *Op=dyn_cast<BinaryOperator>(Store->getValueOperand()))if(Cmp->getPredicate()==ICmpInst::ICMP_NE&&Cmp->getOperand(0)==Op&&Cmp->getOperand(1)==Old&&
          ((Op->getOperand(0)==Old&&Op->getOperand(1)==Source)||(Op->getOperand(1)==Old&&Op->getOperand(0)==Source))) {
        if(Op->getOpcode()==Instruction::Or)R.Join=SGPL_JOIN_OR;
        if(Op->getOpcode()==Instruction::And)R.Join=SGPL_JOIN_AND;
      }
    }
  }
  // A join certificate also requires the entire effect to be exactly that
  // guarded store/notification, with no additional conditional user behavior.
  if(R.Join)for(BasicBlock &BB:*F)if(auto *Br=dyn_cast<BranchInst>(BB.getTerminator()))if(Br->isConditional()) {
    if(Br!=EffectGuard)R.Join=0;
  }
  for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="autograph_frontier_activate")
    C->setCalledFunction(F->getParent()->getOrInsertFunction("sgpl_semantic_activate_v1",C->getFunctionType()));
  F->setName(Pair->getName()+".semantic.ordered_pair");F->setSection(".sgpl_semantic_stage");R.Pair=F;return R;
}

/* A record-free row kernel is allowed only when the runtime proves that the
 * frontier is exactly 0..n-1. CSC then has the original per-destination order.
 * Notifications retain original edge keys and are compacted in that order. */
static Function *semanticRow(Module &M,const SemanticRecipe &R) {
  IRBuilder<> Ty(M.getContext());auto *P=Ty.getPtrTy();auto *I=Ty.getInt32Ty(),*L=Ty.getInt64Ty();
  auto *F=Function::Create(FunctionType::get(Ty.getVoidTy(),{P,I,P,P,L,L},false),GlobalValue::InternalLinkage,"sgpl.semantic.ordered_row",M);
  F->setSection(".sgpl_semantic_stage");
  auto *Entry=BasicBlock::Create(M.getContext(),"entry",F),*Init=BasicBlock::Create(M.getContext(),"first.contribution",F),
    *Loop=BasicBlock::Create(M.getContext(),"contributions",F),*Body=BasicBlock::Create(M.getContext(),"pair",F),
    *Flush=BasicBlock::Create(M.getContext(),"flush.check",F),*Write=BasicBlock::Create(M.getContext(),"flush",F),*Done=BasicBlock::Create(M.getContext(),"done",F);
  IRBuilder<> B(Entry);Type *T=R.Bytes==8?B.getDoubleTy():static_cast<Type *>(I);
  auto *Acc=B.CreateAlloca(T,nullptr,"ordered.accumulator"),*Dirty=B.CreateAlloca(B.getInt1Ty(),nullptr,"accumulator.written");
  B.CreateStore(B.getFalse(),Dirty);B.CreateCondBr(B.CreateICmpSLT(F->getArg(4),F->getArg(5)),Init,Done);
  B.SetInsertPoint(Init);Value *Address=B.CreateGEP(T,B.CreateLoad(P,R.Array),B.CreateSExt(F->getArg(1),L));auto *Initial=B.CreateLoad(T,Address);B.CreateStore(Initial,Acc);B.CreateBr(Loop);
  B.SetInsertPoint(Loop);auto *At=B.CreatePHI(L,2,"incoming.index");At->addIncoming(F->getArg(4),Init);B.CreateCondBr(B.CreateICmpSLT(At,F->getArg(5)),Body,Flush);
  B.SetInsertPoint(Body);auto *U=B.CreateLoad(I,B.CreateGEP(I,F->getArg(2),At));auto *Key=B.CreateLoad(L,B.CreateGEP(L,F->getArg(3),At));
  B.CreateStore(Key,B.CreateGEP(B.getInt8Ty(),F->getArg(0),B.getInt64(offsetof(sgpl_exec_ctx,array_work_ordinal))));
  if(R.Weighted) {
    auto *First=B.CreateLoad(P,B.CreateGEP(B.getInt8Ty(),F->getArg(0),B.getInt64(offsetof(sgpl_exec_ctx,array_record_edges))));
    auto *Index=B.CreateLoad(L,B.CreateGEP(L,First,Key));
    B.CreateStore(Index,B.CreateGEP(B.getInt8Ty(),F->getArg(0),B.getInt64(offsetof(sgpl_exec_ctx,array_edge_index))));
  }
  auto *Call=B.CreateCall(R.Pair,{ConstantPointerNull::get(P),F->getArg(0),U,F->getArg(1)});auto *Next=B.CreateAdd(At,B.getInt64(1));B.CreateBr(Loop);At->addIncoming(Next,Body);
  B.SetInsertPoint(Flush);B.CreateCondBr(B.CreateLoad(B.getInt1Ty(),Dirty),Write,Done);
  B.SetInsertPoint(Write);B.CreateStore(B.CreateLoad(T,Acc),Address);B.CreateBr(Done);B.SetInsertPoint(Done);B.CreateRetVoid();
  InlineFunctionInfo IFI;if(!InlineFunction(*Call,IFI).isSuccess())report_fatal_error("cannot inline semantic row callback");
  SmallVector<Instruction *,4> Accesses;
  for(BasicBlock &BB:*F)for(Instruction &X:BB) {
    Value *Ptr=isa<LoadInst>(X)?cast<LoadInst>(X).getPointerOperand():isa<StoreInst>(X)?cast<StoreInst>(X).getPointerOperand():nullptr;
    if(Ptr&&semanticAddress(Ptr,R.Array,F->getArg(1))&& &X!=Initial&&X.getParent()!=Write)Accesses.push_back(&X);
  }
  for(auto *X:Accesses)if(auto *Load=dyn_cast<LoadInst>(X))Load->setOperand(0,Acc);else {auto *Store=cast<StoreInst>(X);IRBuilder<>(Store).CreateStore(ConstantInt::getTrue(M.getContext()),Dirty);Store->setOperand(1,Acc);}
  executionCleanup(F);return F;
}

/* Initialization summaries concern pointer escape only. They do not assert
 * any algebra, initial values or scheduling property. Compiler task wrappers
 * are inspected transitively; an unknown call or pointer publication refuses. */
static bool semanticInitClosure(Function *F,SmallPtrSetImpl<Function *> &Seen) {
  if(!F||F->isDeclaration()||!F->hasLocalLinkage())return false;
  if(!Seen.insert(F).second)return true;
  auto PointerDerived=[](Value *Root) {
    SmallVector<Value *,16> Work{Root};SmallPtrSet<Value *,32> Visited;
    while(!Work.empty()){Value *V=Work.pop_back_val();if(!Visited.insert(V).second)continue;
      if(isa<PtrToIntInst>(V)||isa<IntToPtrInst>(V))return true;
      if(auto *I=dyn_cast<Instruction>(V))if(!isa<LoadInst>(I))for(Value *Op:I->operand_values())Work.push_back(Op);
    }return false;
  };
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    if(auto *S=dyn_cast<StoreInst>(&I))if((S->getValueOperand()->getType()->isPointerTy()||PointerDerived(S->getValueOperand()))&&!isa<AllocaInst>(getUnderlyingObject(S->getPointerOperand())))return false;
    if(auto *Ret=dyn_cast<ReturnInst>(&I))if(Ret->getReturnValue()&&(Ret->getReturnValue()->getType()->isPointerTy()||PointerDerived(Ret->getReturnValue()))&&!isa<ConstantPointerNull>(Ret->getReturnValue()))return false;
    if(auto *C=dyn_cast<CallBase>(&I)) {
      auto *G=C->getCalledFunction();if(!G)return false;
      if(G->isIntrinsic()) {
        auto ID=G->getIntrinsicID();
        if(C->doesNotAccessMemory()||ID==Intrinsic::assume||ID==Intrinsic::lifetime_start||ID==Intrinsic::lifetime_end||ID==Intrinsic::trap)continue;
        return false; // A bulk memory intrinsic can publish pointer bytes.
      }
      if(G->hasLocalLinkage()){if(!semanticInitClosure(G,Seen))return false;continue;}
      if(G->getName()=="parallel_for_runtime"||G->getName()=="parallel_for_runtime_ex"||G->getName()=="sgpl_run_tdg_level") {
        for(Value *V:C->args())if(auto *Fn=dyn_cast<Function>(V->stripPointerCasts()))if(!semanticInitClosure(Fn,Seen))return false;
        continue;
      }
      if(G->doesNotReturn())continue;
      if(G->getName()=="printf"||G->getName()=="puts") {
        for(Value *V:C->args())if(V->getType()->isPointerTy()){auto *Global=dyn_cast<GlobalVariable>(getUnderlyingObject(V));if(!Global||!Global->isConstant())return false;}continue;
      }
      return false;
    }
  }
  return true;
}
static bool semanticFresh(Value *V,Loop *L,DominatorTree &DT,GlobalVariable *Output,SmallPtrSetImpl<Value *> &Seen) {
  if(!Seen.insert(V).second)return true;
  for(User *U:V->users()) {
    auto *I=dyn_cast<Instruction>(U);if(!I)return false;
    if(L->contains(I))continue;
    if(!DT.dominates(I,L->getHeader()->getTerminator()))return false;
    if(isa<ICmpInst>(I))continue;
    if(auto *G=dyn_cast<GetElementPtrInst>(I)){if(!semanticFresh(G,L,DT,Output,Seen))return false;continue;}
    if(auto *S=dyn_cast<StoreInst>(I)) {
      if(S->getPointerOperand()==V&&!S->getValueOperand()->getType()->isPointerTy())continue;
      if(S->getValueOperand()==V) {
        if(S->getPointerOperand()==Output)continue;
        auto *A=dyn_cast<AllocaInst>(getUnderlyingObject(S->getPointerOperand()));
        if(!A||!semanticFresh(A,L,DT,Output,Seen))return false;continue;
      }
      if(isa<AllocaInst>(getUnderlyingObject(S->getPointerOperand()))) {
        if(auto *Fn=dyn_cast<Function>(S->getValueOperand()->stripPointerCasts())){SmallPtrSet<Function *,16> Fns;if(!semanticInitClosure(Fn,Fns))return false;}
        else if(S->getValueOperand()->getType()->isPointerTy()&&!isa<ConstantPointerNull>(S->getValueOperand())&&!Seen.count(S->getValueOperand())&&!isa<AllocaInst>(getUnderlyingObject(S->getValueOperand())))return false;
        continue;
      }
    }
    if(auto *C=dyn_cast<CallInst>(I))if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="sgpl_run_tdg_level"&&C->getArgOperand(0)==V)continue;
    return false;
  }
  return true;
}
static bool semanticInfrastructure(CallInst *C) {
  auto *F=C->getCalledFunction();if(!F)return false;StringRef N=F->getName();
  if(F->isIntrinsic())return F->getIntrinsicID()==Intrinsic::stacksave||F->getIntrinsicID()==Intrinsic::stackrestore||F->getIntrinsicID()==Intrinsic::assume;
  return N=="autograph_build_array_cut"||N=="sgpl_exec_require_status"||N=="autograph_exec_op_create"||N=="autograph_exec_ctx_create"||
    N=="autograph_exec_ctx_set_dag_axes"||N=="autograph_exec_ctx_bind_array_frontier"||N=="autograph_exec_ctx_prepare_spatial"||N=="autograph_exec_ctx_reserve_prepared_output"||
    N=="autograph_exec_ctx_bind_source_snapshot_range"||N=="autograph_exec_ctx_bind_snapshot_range"||N=="autograph_frontier_execute"||
    N=="autograph_exec_ctx_destroy"||N=="autograph_profile_region_enter"||N=="autograph_profile_region_exit"||N=="sgpl_exec_bind_append_journal_v1";
}
static bool semanticConvergence(CallInst *Bind,Loop *L,CallInst *Execute,GlobalVariable *Capacity,GlobalVariable *Array,Function *Pair,Function *Clone) {
  if(!L||L->getNumBlocks()!=1||!Capacity||!Capacity->hasLocalLinkage())return false;
  auto *Input=dyn_cast<PHINode>(Bind->getArgOperand(1));auto *Size=dyn_cast<PHINode>(Bind->getArgOperand(2));auto *Out=dyn_cast<GlobalVariable>(Bind->getArgOperand(3));
  if(!Input||!Size||!Out||!Out->hasLocalLinkage()||Input->getNumIncomingValues()!=2||Size->getNumIncomingValues()!=2)return false;
  bool Backedge=false;
  for(unsigned K=0;K<2;++K)if(L->contains(Input->getIncomingBlock(K))) {
    auto *Back=dyn_cast<LoadInst>(Input->getIncomingValue(K));
    if(!Back||Back->getPointerOperand()!=Out||Size->getIncomingValueForBlock(Input->getIncomingBlock(K))!=Execute)return false;
    Backedge=true;
  }
  if(!Backedge)return false;
  for(User *U:Input->users())if(U!=Bind){auto *S=dyn_cast<StoreInst>(U);if(!S||S->getPointerOperand()!=Out)return false;}
  for(User *U:Size->users())if(U!=Bind)return false;
  DominatorTree DT(*Bind->getFunction());SmallPtrSet<Value *,32> Seen;
  SmallPtrSet<Function *,16> Callbacks;SmallVector<Function *,8> Pending{Pair,Clone};
  // The existing C3 descriptor certifies its callback clone as the same pair
  // with only notification realization changed. Include that exact identity,
  // not arbitrary functions sharing an attribute or a naming convention.
  for(Instruction &I:*L->getHeader())if(auto *C=dyn_cast<CallInst>(&I))if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="sgpl_exec_bind_append_journal_v1"&&C->getArgOperand(0)==Bind->getArgOperand(0))
    if(auto *G=dyn_cast<GlobalVariable>(C->getArgOperand(1)->stripPointerCasts()))if(auto *S=dyn_cast<ConstantStruct>(G->getInitializer()))if(S->getNumOperands()==9&&S->getOperand(4)==Pair)
      if(auto *Journal=dyn_cast<Function>(S->getOperand(5)))Pending.push_back(Journal);
  while(!Pending.empty()){auto *F=Pending.pop_back_val();if(!Callbacks.insert(F).second)continue;for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallBase>(&I))if(auto *G=C->getCalledFunction())if(G->hasLocalLinkage()&&!G->isDeclaration())Pending.push_back(G);}
  for(User *U:Array->users()) {
    auto *I=dyn_cast<Instruction>(U);if(!I)return false;
    if(auto *S=dyn_cast<StoreInst>(I)) {
      auto *A=dyn_cast<CallInst>(S->getValueOperand());
      if(S->getPointerOperand()!=Array||S->getFunction()!=Bind->getFunction()||!A||!A->getCalledFunction()||A->getCalledFunction()->getName()!="calloc"||
          L->contains(S)||!semanticFresh(A,L,DT,Array,Seen))return false;
    } else if(auto *Load=dyn_cast<LoadInst>(I)) {
      if(Load->getFunction()==Bind->getFunction()) {
        if(L->contains(Load))continue;
        // No pointer escape before the loop. After-loop scalar observations
        // are permitted through the original private global, not its address.
        if(DT.dominates(Load,L->getHeader()->getTerminator())&&!semanticFresh(Load,L,DT,Array,Seen))return false;
      } else if(!Callbacks.count(Load->getFunction())) {SmallPtrSet<Function *,16> Functions;if(!semanticInitClosure(Load->getFunction(),Functions))return false;}
    } else return false;
  }
  for(unsigned K=0;K<2;++K)if(!L->contains(Input->getIncomingBlock(K))) {
    auto *Alloc=dyn_cast<CallInst>(Input->getIncomingValue(K));
    if(!Alloc||!Alloc->getCalledFunction()||Alloc->getCalledFunction()->getName()!="calloc"||!semanticFresh(Alloc,L,DT,Out,Seen))return false;
  }
  for(auto *Slot:{Out,Capacity})for(User *U:Slot->users()) {
    auto *I=dyn_cast<Instruction>(U);if(!I||I->getFunction()!=Bind->getFunction())return false;
    if(L->contains(I))continue;
    auto *S=dyn_cast<StoreInst>(I);if(!S||!DT.dominates(S,L->getHeader()->getTerminator()))return false;
    if(Slot==Out) {
      auto *A=dyn_cast<CallInst>(S->getValueOperand());if(!A||!A->getCalledFunction()||A->getCalledFunction()->getName()!="calloc"||!semanticFresh(A,L,DT,Out,Seen))return false;
    }
  }
  for(Instruction &I:*L->getHeader()) {
    for(User *U:I.users())if(auto *UserI=dyn_cast<Instruction>(U)){if(!L->contains(UserI))return false;}else return false;
    if(auto *P=dyn_cast<PHINode>(&I))if(P!=Input&&P!=Size&&!P->getType()->isIntegerTy(64))return false;
    if(auto *S=dyn_cast<StoreInst>(&I))if(S->getPointerOperand()!=Out&&S->getPointerOperand()!=Capacity) {
      auto *A=dyn_cast<AllocaInst>(getUnderlyingObject(S->getPointerOperand()));if(!A||!L->contains(A))return false;
    }
    if(auto *C=dyn_cast<CallInst>(&I)) {
      auto *F=C->getCalledFunction();if(!F)return false;StringRef N=F->getName();
      if(F->isIntrinsic()&&(F->getIntrinsicID()==Intrinsic::stacksave||F->getIntrinsicID()==Intrinsic::stackrestore||F->getIntrinsicID()==Intrinsic::assume))continue;
      if(N=="autograph_build_array_cut"||N=="sgpl_exec_require_status"||N=="autograph_exec_op_create"||N=="autograph_exec_ctx_create"||
          N=="autograph_exec_ctx_set_dag_axes"||N=="autograph_exec_ctx_bind_array_frontier"||N=="autograph_exec_ctx_prepare_spatial"||N=="autograph_exec_ctx_reserve_prepared_output"||
          N=="autograph_exec_ctx_bind_source_snapshot_range"||N=="autograph_exec_ctx_bind_snapshot_range"||N=="autograph_frontier_execute"||
          N=="autograph_exec_ctx_destroy"||N=="autograph_profile_region_enter"||N=="autograph_profile_region_exit"||N=="sgpl_exec_bind_append_journal_v1")continue;
      return false;
    }
  }
  auto *B=dyn_cast<BranchInst>(L->getHeader()->getTerminator());auto *Cmp=B&&B->isConditional()?dyn_cast<ICmpInst>(B->getCondition()):nullptr;
  auto *Zero=Cmp?dyn_cast<ConstantInt>(Cmp->getOperand(1)):nullptr;
  return Cmp&&Cmp->getPredicate()==ICmpInst::ICMP_SGT&&Cmp->getOperand(0)==Execute&&Zero&&Zero->isZero();
}

static void lowerSemanticStages(Module &M) {
  unsigned Requested=executionSimplifyRules();if(!(Requested&(512|1024|2048)))return;
  SmallVector<CallInst *,8> Binds;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))if(C->getMetadata("sgpl.relax.snapshot.bases"))Binds.push_back(C);
  for(auto *Bind:Binds) {
    auto Refuse=[&](StringRef Why){if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[exec-simplify] R10/R11/R12 refuse: "<<Why<<"\n";};
    auto *Witness=Bind->getMetadata("sgpl.exec.frontier");auto *P=Witness&&Witness->getNumOperands()==2?dyn_cast<ValueAsMetadata>(Witness->getOperand(0)):nullptr;
    auto *Pair=P?dyn_cast<Function>(P->getValue()):nullptr;auto *Gate=Witness&&Witness->getNumOperands()==2?dyn_cast<ConstantAsMetadata>(Witness->getOperand(1)):nullptr;
    SmallPtrSet<Function *,16> Active;
    if(!Pair||Pair->arg_size()!=4||!Pair->getReturnType()->isVoidTy()||!Gate||!Gate->getValue()->isNullValue()||!Pair->hasFnAttribute("sgpl.record.source_ordinal.dead")||
        !frontierContextSafe(Pair,1,Active)||!frontierBodySafe(Pair,Active)||
        !relaxPrivateContext(Bind->getArgOperand(0))||!relaxCaptureWindow(Bind)){Refuse("private bounded invocation, gate, or capture window unavailable");continue;}
    DominatorTree DT(*Bind->getFunction());LoopInfo LI(DT);auto *L=LI.getLoopFor(Bind->getParent());
    if(!L||L->getNumBlocks()!=1||L->getHeader()!=Bind->getParent()){Refuse("single-block frontier recurrence unavailable");continue;}
    auto *Header=L->getHeader();CallInst *Execute=nullptr,*Capture=nullptr;Value *Extent=nullptr;
    bool LiveOut=false;
    for(Instruction &I:*Header)if(!isa<PHINode>(I))for(User *U:I.users())if(auto *UseI=dyn_cast<Instruction>(U))LiveOut|=UseI->getParent()!=Header;
    if(LiveOut){Refuse("unmodeled live value escapes the recurrence");continue;}
    for(Instruction &I:*Header)if(auto *C=dyn_cast<CallInst>(&I))if(auto *F=C->getCalledFunction()) {
      if(F->getName()=="autograph_frontier_execute")Execute=C;
      if(F->getName()=="autograph_exec_ctx_bind_source_snapshot_range"||F->getName()=="autograph_exec_ctx_bind_snapshot_range")Capture=C;
    }
    auto *Input=dyn_cast<PHINode>(Bind->getArgOperand(1));auto *Size=dyn_cast<PHINode>(Bind->getArgOperand(2));
    auto *Create=dyn_cast<CallInst>(Bind->getArgOperand(0));auto *Output=dyn_cast<GlobalVariable>(Bind->getArgOperand(3));auto *Capacity=dyn_cast<GlobalVariable>(Bind->getArgOperand(4));
    if(!Execute||!Capture||!Input||!Size||!Create||!Output||!Capacity){Refuse("recurrence shape unavailable");continue;}
    auto *Initial=dyn_cast<ConstantInt>(Create->getArgOperand(6));
    auto *Traversal=dyn_cast<ConstantInt>(Create->getArgOperand(1));
    auto *Domain=dyn_cast<ConstantInt>(Create->getArgOperand(2));
    if(!Initial||!Initial->isZero()||!Traversal||Traversal->getZExtValue()!=SGPL_TRAVERSE_OWNER_V||!Domain||Domain->getZExtValue()!=SGPL_DOMAIN_FRONTIER||!isa<ConstantPointerNull>(Create->getArgOperand(3))){Refuse("nonempty output prefix, membership or traversal contract");continue;}
    auto *OpCount=dyn_cast<ConstantInt>(Create->getArgOperand(11));CallInst *Op=nullptr;unsigned OpStores=0;bool PrivateOps=true;
    auto *Ops=dyn_cast<AllocaInst>(Create->getArgOperand(10));
    if(Ops)for(User *U:Ops->users())if(auto *S=dyn_cast<StoreInst>(U)) {if(S->getPointerOperand()==Ops){++OpStores;Op=dyn_cast<CallInst>(S->getValueOperand());PrivateOps&=DT.dominates(S,Create);}else PrivateOps=false;}else if(U!=Create)PrivateOps=false;
    bool Operation=PrivateOps&&OpStores==1&&OpCount&&OpCount->equalsInt(1)&&Op&&Op->getCalledFunction()&&Op->getCalledFunction()->getName()=="autograph_exec_op_create"&&Op->arg_size()==9&&Op->getArgOperand(2)==Pair;
    if(Operation) {
      auto *Caps=dyn_cast<ConstantInt>(Op->getArgOperand(0));Operation=Caps&&(Caps->getZExtValue()&SGPL_OP_PAIR)&&!(Caps->getZExtValue()&~(uint64_t)(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL));
      for(unsigned N:{1u,3u,4u,5u,6u})Operation&=isa<ConstantPointerNull>(Op->getArgOperand(N));
    }
    if(!Operation){Refuse("baseline pair identity, lifecycle or operation contract unavailable");continue;}
    bool PrefixSafe=true;
    for(Instruction &I:*Header) {
      if(&I==Execute)break;
      if(auto *C=dyn_cast<CallInst>(&I))PrefixSafe&=semanticInfrastructure(C);
      if(auto *S=dyn_cast<StoreInst>(&I)) {auto *A=dyn_cast<AllocaInst>(getUnderlyingObject(S->getPointerOperand()));PrefixSafe&=A&&A->getParent()==Header&&!S->isAtomic()&&!S->isVolatile();}
      if(auto *Load=dyn_cast<LoadInst>(&I))PrefixSafe&=!Load->isAtomic()&&!Load->isVolatile();
      if(isa<InvokeInst>(I)||isa<AtomicRMWInst>(I)||isa<AtomicCmpXchgInst>(I)||isa<FenceInst>(I))PrefixSafe=false;
    }
    if(!PrefixSafe){Refuse("observable effects outside the pair callback");continue;}
    auto *Sites=dyn_cast<ConstantInt>(Bind->getArgOperand(5));
    if(!Sites||!Sites->equalsInt(1)){Refuse("one bounded notification site required");continue;}
    Extent=Capture->getArgOperand(3);auto *ED=dyn_cast<Instruction>(Extent);if(ED&&L->contains(ED)){Refuse("extent is not loop invariant");continue;}
    SemanticRecipe R=semanticRecipe(Pair,Bind->getMetadata("sgpl.relax.snapshot.bases"));if(!R.Pair){Refuse("callback memory/address/use proof unavailable");continue;}
    if(R.Weighted) {auto *Graph=dyn_cast<LoadInst>(Create->getArgOperand(0));if(!Graph||Graph->getPointerOperand()!=R.WeightGraph){R.Pair->eraseFromParent();Refuse("weight reads are not from the traversed immutable graph");continue;}}
    auto *CaptureBase=dyn_cast<LoadInst>(Capture->getArgOperand(2));auto *Slot=dyn_cast<ConstantInt>(Capture->getArgOperand(1));auto *Bytes=dyn_cast<ConstantInt>(Capture->getArgOperand(4));
    if(!CaptureBase||CaptureBase->getPointerOperand()!=R.Array||!Slot||Slot->getZExtValue()!=R.Slot||!Bytes||Bytes->getZExtValue()!=R.Bytes){R.Pair->eraseFromParent();Refuse("snapshot identity unavailable");continue;}
    bool Whole=R.Join&&semanticConvergence(Bind,L,Execute,Capacity,R.Array,Pair,R.Pair);
    uint64_t Facts=sgpl_execution::LocalCallback|sgpl_execution::StableSnapshot|sgpl_execution::DestinationOwner;
    if(R.Minimum)Facts|=sgpl_execution::ImmutableMinimumCandidate;
    if(Whole)Facts|=sgpl_execution::WholeClosure;
    auto Composed=sgpl_execution::simplify(Facts,Requested,sgpl_execution::semanticRules());
    unsigned Rules=Composed.rules;
    if(!Rules){R.Pair->eraseFromParent();Refuse("whole-loop observation proof unavailable");continue;}
    // Only the recognized buffer-swap tail is duplicated into the handled arm.
    SmallVector<Instruction *,8> Tail;bool After=false,Valid=true;
    for(Instruction &I:*Header) {
      if(&I==Execute){After=true;continue;}if(!After||I.isTerminator())continue;
      if(auto *C=dyn_cast<CallInst>(&I)) {auto *F=C->getCalledFunction();if(!F){Valid=false;break;}auto N=F->getName();if(N=="autograph_profile_region_exit"||N=="sgpl_exec_require_status"||N=="autograph_exec_ctx_destroy"||F->getIntrinsicID()==Intrinsic::stackrestore)continue;Valid=false;break;}
      if(isa<LoadInst>(I)||isa<StoreInst>(I)||isa<ICmpInst>(I))Tail.push_back(&I);else Valid=false;
    }
    if(!Valid){R.Pair->eraseFromParent();Refuse("unrecognized observable post-execution tail");continue;}
    SmallPtrSet<Value *,32> Available;Available.insert(Execute);for(PHINode &Phi:Header->phis())Available.insert(&Phi);
    for(auto *I:Tail) {
      for(Value *V:I->operand_values())if(auto *Def=dyn_cast<Instruction>(V))if(Def->getParent()==Header&&!Available.count(Def))Valid=false;
      Available.insert(I);
    }
    if(!Valid){R.Pair->eraseFromParent();Refuse("tail depends on skipped infrastructure or user state");continue;}
    for(PHINode &Phi:Header->phis())for(unsigned K=0;K<Phi.getNumIncomingValues();++K)if(Phi.getIncomingBlock(K)==Header) {
      auto *V=dyn_cast<Instruction>(Phi.getIncomingValue(K));
      if(V&&V->getParent()==Header&&V!=Execute&&!isa<PHINode>(V)&&!llvm::is_contained(Tail,V))Valid=false;
    }
    if(!Valid){R.Pair->eraseFromParent();Refuse("unmodeled recurrence state");continue;}
    Function *Row=(Rules&1024)?semanticRow(M,R):nullptr;
    auto *First=Header->getFirstNonPHI();Value *Graph=Create->getArgOperand(0);auto *GD=dyn_cast<Instruction>(Graph);
    GlobalVariable *GraphSlot=nullptr;
    if(GD&&L->contains(GD)) {
      auto *Load=dyn_cast<LoadInst>(GD);GraphSlot=Load?dyn_cast<GlobalVariable>(Load->getPointerOperand()):nullptr;
      bool Stable=GraphSlot&&GraphSlot->hasLocalLinkage()&&!Load->isAtomic()&&!Load->isVolatile();
      if(Stable)for(User *U:GraphSlot->users())if(!isa<LoadInst>(U)) {
        auto *S=dyn_cast<StoreInst>(U);Stable&=S&&S->getPointerOperand()==GraphSlot&&S->getFunction()==Bind->getFunction()&&!L->contains(S)&&DT.dominates(S,First);
      }
      if(!Stable){if(Row)Row->eraseFromParent();R.Pair->eraseFromParent();Refuse("graph pointer is mutable or escapes");continue;}
    }
    IRBuilder<> B(First);auto *I=B.getInt32Ty(),*I64=B.getInt64Ty();auto *Ptr=B.getPtrTy();
    if(GraphSlot)Graph=B.CreateLoad(Ptr,GraphSlot,"semantic.graph");
    auto *T=StructType::get(M.getContext(),{I,I,I,I,I64,Ptr,Ptr,I,I,I,I});
    if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_semantic_stage_plan_v1))report_fatal_error("semantic descriptor mismatch");
    auto *Plan=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(SGPL_SEMANTIC_STAGE_VERSION),B.getInt32(sizeof(sgpl_semantic_stage_plan_v1)),B.getInt32(Rules),B.getInt32(R.Join),B.getInt64(sizeof(sgpl_exec_ctx)),R.Pair,Row?static_cast<Constant *>(Row):ConstantPointerNull::get(Ptr),B.getInt32(R.Bytes),B.getInt32(R.Slot),B.getInt32(R.Weighted),B.getInt32(0)}),"sgpl.semantic.stage.v1");
    M.getOrInsertNamedMetadata("sgpl.execution.compositions")->addOperand(MDNode::get(M.getContext(),{
      ValueAsMetadata::get(Pair),ConstantAsMetadata::get(B.getInt64(Composed.facts)),
      ConstantAsMetadata::get(B.getInt32(Composed.rules)),ConstantAsMetadata::get(B.getInt32(Composed.rewrites)),
      MDString::get(M.getContext(),"immutable callback/snapshot/ownership witnesses; monotone worklist before one lowering; R6/R7 require exact immutable signed-minimum candidate") }));
    if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS")) {
      errs()<<"[exec-compose] fixed-point rules="<<Rules<<" rewrites="<<Composed.rewrites<<" visits="<<Composed.visits<<"\n";
      for(unsigned N:{5u,6u})if(Requested&(1u<<N))errs()<<"[exec-compose] R"<<N+1<<" "<<((Rules&(1u<<N))?"activate":"refuse")<<": immutable minimum candidate and selected traversal required\n";
    }
    auto *Result=B.CreateCall(M.getOrInsertFunction("sgpl_try_semantic_stage_v1",FunctionType::get(I,{Ptr,Ptr,Ptr,I,Ptr,Ptr,Ptr,I64},false)),{Graph,Plan,Input,Size,Output,Capacity,B.CreateLoad(Ptr,R.Array),Extent},"semantic.result");
    auto *Handled=BasicBlock::Create(M.getContext(),"semantic.handled",Header->getParent());
    auto *Baseline=Header->splitBasicBlock(First,"semantic.baseline");Header->getTerminator()->eraseFromParent();B.SetInsertPoint(Header);B.CreateCondBr(B.CreateICmpSGE(Result,B.getInt32(0)),Handled,Baseline);
    ValueToValueMapTy VM;VM[Execute]=Result;B.SetInsertPoint(Handled);
    for(auto *Old:Tail){auto *New=Old->clone();RemapInstruction(New,VM,RF_IgnoreMissingLocals);B.Insert(New);VM[Old]=New;}
    auto *Term=Baseline->getTerminator()->clone();RemapInstruction(Term,VM,RF_IgnoreMissingLocals);B.Insert(Term);
    for(BasicBlock *Succ:successors(Handled))for(PHINode &Phi:Succ->phis()) {
      int K=Phi.getBasicBlockIndex(Baseline);if(K>=0){Value *V=Phi.getIncomingValue(K);Phi.addIncoming(VM.count(V)?static_cast<Value *>(VM[V]):V,Handled);}
    }
    if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))for(unsigned N=9;N<12;++N)if(Requested&(1u<<N))errs()<<"[exec-simplify] R"<<N+1<<" "<<((Rules&(1u<<N))?"activate":"refuse")<<": "<<(N==11?"whole-loop finite-lattice/observation certificate":N==10?"exact ordered destination state; full-order runtime guard":"generic bounded callback; original contribution and notification order")<<"\n";
    if(verifyModule(M,&errs()))report_fatal_error("invalid semantic stage module");
  }
}
}
