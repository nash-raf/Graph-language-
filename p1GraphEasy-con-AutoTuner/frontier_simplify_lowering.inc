/* Late specialization of the existing ordered record executor. The frontend
 * witness is necessary; complete context/effect checks remain a separate gate.
 * Records, their order, ownership, journal protocol and scheduling are retained. */
#include "frontier_simplify_plan.h"
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
namespace {
using namespace llvm;
static bool frontierInlineLocal(Function *F) {
  for(unsigned N=0;N<256;++N) {
    CallInst *Next=nullptr;
    for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallBase>(&I))
      if(auto *G=C->getCalledFunction())if(G->hasLocalLinkage()&&!G->isDeclaration()) {
        if(!isa<CallInst>(C)||G==F)return false;
        if(!Next)Next=cast<CallInst>(C);
      }
    if(!Next)return true;
    InlineFunctionInfo IFI;if(!InlineFunction(*Next,IFI).isSuccess())return false;
  }
  return false;
}
static bool frontierCtxField(Value *V,Value *Ctx,size_t Offset,const DataLayout &DL) {
  int64_t At=0;return GetPointerBaseWithConstantOffset(V,At,DL)==Ctx&&At==(int64_t)Offset;
}
/* Snapshot storage is private to the invocation and frozen before owner work.
 * Reject writes, pointer escapes, merges and unknown uses of its complete
 * derived pointer chain, even if the frontend already supplied ownership. */
static bool frontierReadOnlyPointers(Value *Root) {
  SmallVector<Value *,16> Work{Root};SmallPtrSet<Value *,32> Seen;
  while(!Work.empty()) {
    Value *V=Work.pop_back_val();if(!Seen.insert(V).second)continue;
    for(User *U:V->users()) {
      if(auto *G=dyn_cast<GetElementPtrInst>(U)){if(G->getPointerOperand()!=V)return false;Work.push_back(G);continue;}
      if(isa<BitCastInst>(U)){Work.push_back(cast<Value>(U));continue;}
      if(auto *L=dyn_cast<LoadInst>(U)) {
        if(L->getPointerOperand()!=V||L->isAtomic()||L->isVolatile())return false;
        if(L->getType()->isPointerTy())Work.push_back(L);continue;
      }
      if(isa<ICmpInst>(U))continue;
      return false;
    }
  }
  return true;
}
static bool frontierSourceIndex(Value *V,Value *Source) {
  if(V==Source)return true;
  if(auto *C=dyn_cast<CastInst>(V))if(isa<SExtInst>(C)||isa<ZExtInst>(C))return C->getOperand(0)==Source;
  return false;
}
static bool frontierSnapshotElement(LoadInst *L,Value *Ctx,Value *Source,const DataLayout &DL) {
  if(L->isAtomic()||L->isVolatile()||!(L->getType()->isIntegerTy()||L->getType()->isFloatTy()||L->getType()->isDoubleTy()))return false;
  auto *G=dyn_cast<GetElementPtrInst>(L->getPointerOperand());
  if(!G||G->getNumIndices()!=1||G->getSourceElementType()!=L->getType()||!frontierSourceIndex(G->idx_begin()->get(),Source))return false;
  auto *Snapshot=dyn_cast<LoadInst>(G->getPointerOperand());if(!Snapshot||!Snapshot->getType()->isPointerTy())return false;
  int64_t SlotOffset=0;auto *Table=dyn_cast<LoadInst>(GetPointerBaseWithConstantOffset(Snapshot->getPointerOperand(),SlotOffset,DL));
  return Table&&SlotOffset>=0&&SlotOffset%8==0&&Table->getType()->isPointerTy()&&
      frontierCtxField(Table->getPointerOperand(),Ctx,offsetof(sgpl_exec_ctx,spatial_snapshots),DL)&&
      frontierReadOnlyPointers(Table);
}
/* Cache each eligible source load at its original dynamic evaluation point.
 * A miss executes the original instruction. Conditional/empty/cancelled work
 * therefore introduces no speculative snapshot access. Independent caches
 * handle different conditional sites. mem2reg makes their state loop PHIs. */
static void frontierCacheSource(LoadInst *L,Value *Source,Function *K) {
  IRBuilder<> E(&*K->getEntryBlock().getFirstInsertionPt());
  auto *Valid=E.CreateAlloca(E.getInt1Ty(),nullptr,"source.cache.valid");
  auto *Key=E.CreateAlloca(Source->getType(),nullptr,"source.cache.key");
  auto *Cache=E.CreateAlloca(L->getType(),nullptr,"source.cache.value");
  E.CreateStore(E.getFalse(),Valid);E.CreateStore(Constant::getNullValue(Source->getType()),Key);
  E.CreateStore(Constant::getNullValue(L->getType()),Cache);
  BasicBlock *Before=L->getParent(),*After=Before->splitBasicBlock(L,"source.cache.join");Before->getTerminator()->eraseFromParent();
  auto *Hit=BasicBlock::Create(K->getContext(),"source.cache.hit",K,After);
  auto *Miss=BasicBlock::Create(K->getContext(),"source.cache.miss",K,After);
  IRBuilder<> B(Before);auto *Matches=B.CreateAnd(B.CreateLoad(B.getInt1Ty(),Valid),B.CreateICmpEQ(B.CreateLoad(Source->getType(),Key),Source));B.CreateCondBr(Matches,Hit,Miss);
  B.SetInsertPoint(Hit);auto *Saved=B.CreateLoad(L->getType(),Cache);B.CreateBr(After);
  L->removeFromParent();B.SetInsertPoint(Miss);B.Insert(L);auto *Save=B.CreateStore(L,Cache);B.CreateStore(Source,Key);B.CreateStore(B.getTrue(),Valid);B.CreateBr(After);
  auto *Phi=PHINode::Create(L->getType(),2,"source.value",After->begin());Phi->addIncoming(Saved,Hit);Phi->addIncoming(L,Miss);
  L->replaceUsesWithIf(Phi,[&](Use &U){return U.getUser()!=Phi&&U.getUser()!=Save;});
}
struct FrontierGenerated {Function *Wrapper=nullptr;unsigned Cached=0;};
static FrontierGenerated frontierRecordKernel(Module &M,Function *Pair,bool Weighted,bool CacheSources) {
  auto &C=M.getContext();IRBuilder<> Types(C);auto *P=Types.getPtrTy();auto *I64=Types.getInt64Ty();
  SmallVector<Function *,4> Modes;unsigned Cached=0;
  for(unsigned Mode=0;Mode<4;++Mode) {
    auto *K=Function::Create(FunctionType::get(I64,{P},false),GlobalValue::InternalLinkage,
        Pair->getName()+".records."+(Weighted?"weighted.":"plain.")+Twine(Mode),M);
    Modes.push_back(K);K->setSection(".sgpl_frontier_simplified");K->addFnAttr(Attribute::NoInline);
    auto *Entry=BasicBlock::Create(C,"entry",K),*Loop=BasicBlock::Create(C,"records",K),
      *Poll=BasicBlock::Create(C,"poll",K),*Check=BasicBlock::Create(C,"error.load",K),
      *Visit=BasicBlock::Create(C,"visit",K),*Mark=BasicBlock::Create(C,"mark",K),
      *PairBB=BasicBlock::Create(C,"pair",K),*Next=BasicBlock::Create(C,"next",K),
      *Done=BasicBlock::Create(C,"done",K),*Error=BasicBlock::Create(C,"cancelled",K);
    IRBuilder<> B(Entry);
    auto Arg=[&](size_t Off,Type *T){return B.CreateLoad(T,B.CreateGEP(B.getInt8Ty(),K->getArg(0),B.getInt64(Off)));};
    Value *State=Arg(offsetof(sgpl_frontier_simplify_args_v1,state),P),*Ctx=Arg(offsetof(sgpl_frontier_simplify_args_v1,context),P),
      *Records=Arg(offsetof(sgpl_frontier_simplify_args_v1,records),P),*Edges=Arg(offsetof(sgpl_frontier_simplify_args_v1,edges),P),
      *First=Arg(offsetof(sgpl_frontier_simplify_args_v1,first),I64),*Last=Arg(offsetof(sgpl_frontier_simplify_args_v1,last),I64),
      *Err=Arg(offsetof(sgpl_frontier_simplify_args_v1,error),P),*Seen=Arg(offsetof(sgpl_frontier_simplify_args_v1,seen),P),
      *Tags=Arg(offsetof(sgpl_frontier_simplify_args_v1,tags),P),*Epoch=Arg(offsetof(sgpl_frontier_simplify_args_v1,epoch),B.getInt32Ty());
    B.CreateAssumption(B.CreateIsNotNull(Ctx));B.CreateBr(Loop);
    B.SetInsertPoint(Loop);auto *I=B.CreatePHI(I64,2,"record.index"),*Groups=B.CreatePHI(I64,2,"groups");
    I->addIncoming(First,Entry);Groups->addIncoming(B.getInt64(0),Entry);B.CreateCondBr(B.CreateICmpSLT(I,Last),Poll,Done);
    B.SetInsertPoint(Poll);B.CreateCondBr(B.CreateIsNull(Err),Visit,Check);
    B.SetInsertPoint(Check);auto *Status=B.CreateLoad(B.getInt32Ty(),Err);Status->setAtomic(AtomicOrdering::Acquire);Status->setAlignment(Align(4));B.CreateCondBr(B.CreateICmpSLT(Status,B.getInt32(0)),Error,Visit);
    B.SetInsertPoint(Visit);Value *At=B.CreateMul(I,B.getInt64(Weighted?4:3));
    Value *U=B.CreateLoad(B.getInt32Ty(),B.CreateGEP(B.getInt32Ty(),Records,At),"source"),*V=B.CreateLoad(B.getInt32Ty(),B.CreateGEP(B.getInt32Ty(),Records,B.CreateAdd(At,B.getInt64(1))),"destination");
    auto Put=[&](size_t Off,Value *X){B.CreateStore(X,B.CreateGEP(B.getInt8Ty(),Ctx,B.getInt64(Off)));};
    Value *Ordinal=B.CreateSExt(B.CreateLoad(B.getInt32Ty(),B.CreateGEP(B.getInt32Ty(),Records,B.CreateAdd(At,B.getInt64(2)))),I64);
    Put(offsetof(sgpl_exec_ctx,array_work_ordinal),Ordinal);Put(offsetof(sgpl_exec_ctx,array_append_seq),B.getInt32(0));
    if(Weighted)Put(offsetof(sgpl_exec_ctx,array_edge_index),B.CreateSExt(B.CreateLoad(B.getInt32Ty(),B.CreateGEP(B.getInt32Ty(),Records,B.CreateAdd(At,B.getInt64(3)))),I64));
    else {
      auto *HasEdges=BasicBlock::Create(C,"edge.index",K),*SetEdge=BasicBlock::Create(C,"edge.ready",K);
      B.CreateCondBr(B.CreateIsNull(Edges),SetEdge,HasEdges);B.SetInsertPoint(HasEdges);auto *Edge=B.CreateLoad(I64,B.CreateGEP(I64,Edges,I));B.CreateBr(SetEdge);
      B.SetInsertPoint(SetEdge);auto *Index=B.CreatePHI(I64,2);Index->addIncoming(B.getInt64(-1),Visit);Index->addIncoming(Edge,HasEdges);Put(offsetof(sgpl_exec_ctx,array_edge_index),Index);
    }
    auto *BeforeMark=B.GetInsertBlock();Value *SeenPtr=B.CreateGEP(B.getInt8Ty(),Seen,B.CreateSExt(V,I64));Value *TagPtr=nullptr;
    if(Mode==3){TagPtr=B.CreateGEP(B.getInt32Ty(),Tags,B.CreateSExt(V,I64));B.CreateCondBr(B.CreateICmpNE(B.CreateLoad(B.getInt32Ty(),TagPtr),Epoch),Mark,PairBB);}
    else if(Mode==2)B.CreateCondBr(B.CreateICmpEQ(B.CreateLoad(B.getInt8Ty(),SeenPtr),B.getInt8(0)),Mark,PairBB);
    else B.CreateBr(Mode?Mark:PairBB);
    B.SetInsertPoint(Mark);if(Mode==3)B.CreateStore(Epoch,TagPtr);if(Mode)B.CreateStore(B.getInt8(1),SeenPtr);
    auto *Marked=B.CreateAdd(Groups,B.getInt64(Mode>=2?1:0));B.CreateBr(PairBB);
    B.SetInsertPoint(PairBB);auto *AfterGroups=B.CreatePHI(I64,Mode>=2?2:1);AfterGroups->addIncoming(Marked,Mark);
    if(Mode>=2||!Mode)AfterGroups->addIncoming(Groups,BeforeMark);
    auto *Call=B.CreateCall(Pair,{State,Ctx,U,V});B.CreateBr(Next);
    B.SetInsertPoint(Next);auto *NI=B.CreateAdd(I,B.getInt64(1));auto *Back=B.CreateBr(Loop);fullOrderedLoop(Back);I->addIncoming(NI,Next);Groups->addIncoming(AfterGroups,Next);
    B.SetInsertPoint(Done);B.CreateRet(Groups);B.SetInsertPoint(Error);B.CreateRet(B.getInt64(-1));
    InlineFunctionInfo IFI;if(!InlineFunction(*Call,IFI).isSuccess()||!frontierInlineLocal(K))report_fatal_error("cannot inline certified frontier callback");
    SmallVector<LoadInst *,16> SourceLoads,DeadFields;bool SnapshotsReadOnly=true;
    bool Appends=false,WeightUse=false;
    for(BasicBlock &BB:*K)for(Instruction &X:BB) {
      if(auto *L=dyn_cast<LoadInst>(&X)) {
        if(frontierCtxField(L->getPointerOperand(),Ctx,offsetof(sgpl_exec_ctx,spatial_snapshots),M.getDataLayout())&&!frontierReadOnlyPointers(L))SnapshotsReadOnly=false;
        if(frontierCtxField(L->getPointerOperand(),Ctx,offsetof(sgpl_exec_ctx,array_compact_records),M.getDataLayout()))DeadFields.push_back(L);
        if(frontierCtxField(L->getPointerOperand(),Ctx,offsetof(sgpl_exec_ctx,array_edge_index),M.getDataLayout()))WeightUse=true;
        if(CacheSources&&frontierSnapshotElement(L,Ctx,U,M.getDataLayout()))SourceLoads.push_back(L);
      }
      if(auto *CB=dyn_cast<CallBase>(&X))if(auto *F=CB->getCalledFunction()) {
        auto N=F->getName();Appends|=N=="autograph_frontier_activate"||N=="sgpl_frontier_journal_activate_v1";
        WeightUse|=N=="autograph_exec_ctx_edge_weight";
        if(N=="autograph_exec_ctx_snapshot")SnapshotsReadOnly=false;
      }
    }
    for(auto *L:DeadFields){L->replaceAllUsesWith(ConstantInt::get(L->getType(),Weighted?4:3));L->eraseFromParent();}
    SmallVector<StoreInst *,8> DeadStores;
    for(BasicBlock &BB:*K)for(Instruction &X:BB)if(auto *S=dyn_cast<StoreInst>(&X)) {
      bool AppendField=frontierCtxField(S->getPointerOperand(),Ctx,offsetof(sgpl_exec_ctx,array_work_ordinal),M.getDataLayout())||frontierCtxField(S->getPointerOperand(),Ctx,offsetof(sgpl_exec_ctx,array_append_seq),M.getDataLayout());
      bool EdgeField=frontierCtxField(S->getPointerOperand(),Ctx,offsetof(sgpl_exec_ctx,array_edge_index),M.getDataLayout());
      if((AppendField&&!Appends)||(EdgeField&&!WeightUse))DeadStores.push_back(S);
    }
    for(auto *S:DeadStores)S->eraseFromParent();
    if(!SnapshotsReadOnly)SourceLoads.clear();
    for(auto *L:SourceLoads)frontierCacheSource(L,U,K);
    Cached=std::max(Cached,(unsigned)SourceLoads.size());executionCleanup(K);
  }
  auto *W=Function::Create(FunctionType::get(I64,{P},false),GlobalValue::InternalLinkage,
      Pair->getName()+".records."+(Weighted?"weighted":"plain"),M);W->setSection(".sgpl_frontier_simplified");
  auto *Entry=BasicBlock::Create(C,"select.tracking",W);IRBuilder<> B(Entry);
  auto Arg=[&](size_t Off,Type *T){return B.CreateLoad(T,B.CreateGEP(B.getInt8Ty(),W->getArg(0),B.getInt64(Off)));};
  auto *Seen=Arg(offsetof(sgpl_frontier_simplify_args_v1,seen),P),*Tags=Arg(offsetof(sgpl_frontier_simplify_args_v1,tags),P),*Sparse=Arg(offsetof(sgpl_frontier_simplify_args_v1,sparse),B.getInt32Ty());
  Value *Mode=B.CreateSelect(B.CreateIsNull(Seen),B.getInt32(0),B.CreateSelect(B.CreateIsNotNull(Tags),B.getInt32(3),B.CreateSelect(B.CreateICmpNE(Sparse,B.getInt32(0)),B.getInt32(2),B.getInt32(1))));
  auto *Default=BasicBlock::Create(C,"tracking.0",W);auto *Switch=B.CreateSwitch(Mode,Default,3);
  for(unsigned N=0;N<4;++N){auto *BB=N?BasicBlock::Create(C,"tracking."+Twine(N),W):Default;if(N)Switch->addCase(B.getInt32(N),BB);B.SetInsertPoint(BB);B.CreateRet(B.CreateCall(Modes[N],{W->getArg(0)}));}
  executionCleanup(W);return {W,Cached};
}
static void lowerFrontierExecutionSimplifications(Module &M) {
  unsigned Requested=executionSimplifyRules();bool Stats=std::getenv("SGPL_EXEC_SIMPLIFY_STATS");SmallVector<CallInst *,8> Marks;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))if(C->getMetadata("sgpl.exec.frontier"))Marks.push_back(C);
  for(auto *C:Marks) {
    auto *MD=C->getMetadata("sgpl.exec.frontier");C->setMetadata("sgpl.exec.frontier",nullptr);
    auto GetFn=[&](unsigned N)->Function *{auto *V=N<MD->getNumOperands()?dyn_cast<ValueAsMetadata>(MD->getOperand(N)):nullptr;return V?dyn_cast<Function>(V->getValue()->stripPointerCasts()):nullptr;};
    Function *Pair=GetFn(0),*Gate=GetFn(1);SmallPtrSet<Function *,16> Active;
    const char *Reason="certified ordered record loop";
    bool Eligible=(Requested&8)&&C->getCalledFunction()&&C->getCalledFunction()->getName()=="autograph_exec_ctx_bind_array_frontier"&&MD->getNumOperands()==2&&
        Pair&&Pair->arg_size()==4&&Pair->hasFnAttribute("sgpl.spatial.callback")&&Pair->hasFnAttribute("sgpl.record.source_ordinal.dead")&&
        frontierContextSafe(Pair,1,Active)&&frontierBodySafe(Pair,Active)&&frontierGateSafe(Gate);
    if(!Eligible){if(Stats)errs()<<"[exec-simplify] R4 refuse: disabled or missing ownership/lifecycle/context/effect proof\n";continue;}
    /* Reject cycles and exceptional control flow in the full inlined body.
     * These bodies are bounded ordinary pair operations, not arbitrary loops. */
    ValueToValueMapTy VM;auto *Probe=CloneFunction(Pair,VM);bool Flat=frontierInlineLocal(Probe);
    if(Flat){DominatorTree DT(*Probe);LoopInfo LI(DT);Flat=LI.empty();for(BasicBlock &BB:*Probe)for(Instruction &I:BB)if(isa<InvokeInst>(I)||I.isEHPad())Flat=false;}
    Probe->eraseFromParent();if(!Flat){if(Stats)errs()<<"[exec-simplify] R4 refuse: recursive, cyclic, exceptional or unbounded pair body\n";continue;}
    Function *Journal=nullptr;
    for(Instruction &I:*C->getParent())if(auto *Bind=dyn_cast<CallInst>(&I))if(Bind->getCalledFunction()&&Bind->getCalledFunction()->getName()=="sgpl_exec_bind_append_journal_v1"&&Bind->getArgOperand(0)==C->getArgOperand(0))
      if(auto *G=dyn_cast<GlobalVariable>(Bind->getArgOperand(1)->stripPointerCasts()))if(auto *S=dyn_cast<ConstantStruct>(G->getInitializer()))if(S->getNumOperands()==9&&S->getOperand(4)==Pair)Journal=dyn_cast<Function>(S->getOperand(5));
    auto Plain=frontierRecordKernel(M,Pair,false,Requested&16),Weighted=frontierRecordKernel(M,Pair,true,Requested&16);
    FrontierGenerated JP,JW;if(Journal){JP=frontierRecordKernel(M,Journal,false,Requested&16);JW=frontierRecordKernel(M,Journal,true,Requested&16);}
    unsigned Cached=std::max({Plain.Cached,Weighted.Cached,JP.Cached,JW.Cached}),Rules=8|((Requested&16)&&Cached?16:0);
    IRBuilder<> B(C->getNextNode());auto *I=B.getInt32Ty(),*L=B.getInt64Ty();auto *P=B.getPtrTy();
    auto *T=StructType::get(M.getContext(),{I,I,I,I,L,L,L,P,P,P,P,P,P});
    if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_frontier_simplify_plan_v1))report_fatal_error("frontier simplification descriptor layout mismatch");
    auto Ptr=[&](Function *F)->Constant *{return F?static_cast<Constant *>(F):ConstantPointerNull::get(P);};
    auto *Plan=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(1),B.getInt32(sizeof(sgpl_frontier_simplify_plan_v1)),B.getInt32(Rules),B.getInt32(0),B.getInt64(sizeof(sgpl_exec_ctx)),B.getInt64(sizeof(sgpl_runtime_op)),B.getInt64(sizeof(sgpl_frontier_simplify_args_v1)),Pair,Ptr(Journal),Plain.Wrapper,Weighted.Wrapper,Ptr(JP.Wrapper),Ptr(JW.Wrapper)}),"sgpl.frontier.simplification.v1");
    B.CreateCall(M.getOrInsertFunction("sgpl_exec_bind_simplified_frontier_v1",FunctionType::get(B.getVoidTy(),{P,P},false)),{C->getArgOperand(0),Plan});
    M.getOrInsertNamedMetadata("sgpl.execution.simplification.proofs")->addOperand(MDNode::get(M.getContext(),{ValueAsMetadata::get(Pair),MDString::get(M.getContext(),"R4: destination ownership, bounded pair-only records, complete context effects; R5: read-only private round snapshots, exact source index, lazy last-source cache; original occurrence/order/error/journal semantics retained")}));
    if(Stats){errs()<<"[exec-simplify] R4 activate "<<Pair->getName()<<": "<<Reason<<"\n";if(Requested&16)errs()<<"[exec-simplify] R5 "<<(Cached?"activate ":"refuse ")<<Pair->getName()<<": immutable source snapshot loads="<<Cached<<"\n";}
    if(verifyModule(M,&errs()))report_fatal_error("invalid frontier execution simplification module");
  }
}
}
