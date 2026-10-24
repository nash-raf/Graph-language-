/* Recognize scalar full-domain initialization -> certified traversal -> copy
 * before LLVM vectorizes the initialization. This proof covers the whole
 * circulating window; the later compiler stage only realizes its certificate.
 * There is no algorithm/function/array-name eligibility check. */
#include "iteration_memory_plan.h"
#include <llvm/IR/PatternMatch.h>
#include <llvm/Transforms/Utils/Local.h>
namespace {
using namespace llvm;
using namespace llvm::PatternMatch;
static void iterationCanonicalize(Function *F) {
  executionCleanup(F);
  PassBuilder PB;LoopAnalysisManager LAM;FunctionAnalysisManager FAM;CGSCCAnalysisManager CGAM;ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);PB.registerCGSCCAnalyses(CGAM);PB.registerFunctionAnalyses(FAM);PB.registerLoopAnalyses(LAM);PB.crossRegisterProxies(LAM,FAM,CGAM,MAM);
  FunctionPassManager PM;cantFail(PB.parsePassPipeline(PM,"instcombine,early-cse,simplifycfg,dce"));PM.run(*F,FAM);
}
static void iterationNonnullGate(Function *F,bool Map) {
  // Both generated functions are invoked only with the private, successfully
  // allocated gate buffer. Preserve element reads; specialize nullness only.
  for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *Cmp=dyn_cast<ICmpInst>(&I))
    if(Cmp->isEquality()&&isa<ConstantPointerNull>(Cmp->getOperand(1))) {
      if(executionPointerKind(Cmp->getOperand(0),F,Map)==0)Cmp->replaceAllUsesWith(ConstantInt::get(Cmp->getType(),Cmp->getPredicate()==ICmpInst::ICMP_NE));
      else if(isa<ConstantPointerNull>(Cmp->getOperand(0)))Cmp->replaceAllUsesWith(ConstantInt::get(Cmp->getType(),Cmp->getPredicate()==ICmpInst::ICMP_EQ));
    }
}
static Value *iterationIndex(Value *V) {
  if(auto *C=dyn_cast<TruncInst>(V))V=C->getOperand(0);
  return V;
}
static GlobalVariable *iterationSlot(Value *Address,Value *Index) {
  auto *G=dyn_cast<GetElementPtrInst>(Address);
  if(!G||G->getNumIndices()!=1||!G->getSourceElementType()->isDoubleTy()||iterationIndex(G->getOperand(1))!=Index)return nullptr;
  auto *L=dyn_cast<LoadInst>(G->getPointerOperand());
  return L?dyn_cast<GlobalVariable>(L->getPointerOperand()):nullptr;
}
struct IterationSweep {Loop *L=nullptr;GlobalVariable *To=nullptr,*From=nullptr,*Seed=nullptr;StoreInst *Store=nullptr;};
static GlobalVariable *iterationGraphExtent(Value *V) {
  auto *N=dyn_cast<LoadInst>(V);auto *P=N?dyn_cast<GetElementPtrInst>(N->getPointerOperand()):nullptr;
  if(!P||!P->hasAllZeroIndices())return nullptr;
  auto *G=dyn_cast<LoadInst>(P->getPointerOperand());return G?dyn_cast<GlobalVariable>(G->getPointerOperand()):nullptr;
}
static IterationSweep iterationSweep(Loop *L,GlobalVariable *Graph) {
  IterationSweep R;if(L->getNumBlocks()!=2||!L->getSubLoops().empty())return R;
  auto *H=L->getHeader();auto *Br=dyn_cast<BranchInst>(H->getTerminator());
  auto *Cmp=Br&&Br->isConditional()?dyn_cast<ICmpInst>(Br->getCondition()):nullptr;
  auto *Phi=Cmp?dyn_cast<PHINode>(Cmp->getOperand(0)):nullptr;
  if(!Phi||Phi->getParent()!=H||Phi->getNumIncomingValues()!=2||Cmp->getPredicate()!=ICmpInst::ICMP_SLT||iterationGraphExtent(Cmp->getOperand(1))!=Graph||!L->contains(Br->getSuccessor(0))||L->contains(Br->getSuccessor(1)))return R;
  auto *Body=Br->getSuccessor(0);auto *Back=dyn_cast<BranchInst>(Body->getTerminator());
  if(!Back||Back->isConditional()||Back->getSuccessor(0)!=H)return R;
  bool Zero=false,Step=false;
  for(unsigned K=0;K<2;++K)if(Phi->getIncomingBlock(K)==Body){auto *Add=dyn_cast<BinaryOperator>(Phi->getIncomingValue(K));Step=Add&&Add->getOpcode()==Instruction::Add&&Add->getOperand(0)==Phi&&match(Add->getOperand(1),m_One());}
  else Zero=match(Phi->getIncomingValue(K),m_Zero());
  if(!Zero||!Step)return R;
  StoreInst *S=nullptr;
  for(BasicBlock *B:L->blocks())for(Instruction &I:*B) {
    if(auto *St=dyn_cast<StoreInst>(&I)){if(S||St->isAtomic()||St->isVolatile())return R;S=St;}
    if(isa<CallBase>(I)||isa<AtomicRMWInst>(I)||isa<AtomicCmpXchgInst>(I)||isa<FenceInst>(I))return R;
    if(auto *Load=dyn_cast<LoadInst>(&I))if(Load->isAtomic()||Load->isVolatile())return R;
  }
  if(!S||!S->getValueOperand()->getType()->isDoubleTy())return R;
  R.To=iterationSlot(S->getPointerOperand(),Phi);if(!R.To)return {};
  auto *Read=dyn_cast<LoadInst>(S->getValueOperand());if(!Read)return {};
  R.From=iterationSlot(Read->getPointerOperand(),Phi);
  if(!R.From)R.Seed=dyn_cast<GlobalVariable>(Read->getPointerOperand());
  if(!R.From&&!R.Seed)return {};
  R.L=L;R.Store=S;return R;
}
static bool iterationStableSlot(GlobalVariable *G,Loop *Outer,DominatorTree &DT) {
  if(!G||!G->hasLocalLinkage()||G->isThreadLocal())return false;
  for(User *U:G->users()) {
    if(auto *C=dyn_cast<CallInst>(U))if(C->getMetadata("sgpl.source.count.proof")&&C->getCalledFunction()&&
        C->getCalledFunction()->getName()=="sgpl_try_source_count_v1"&&C->arg_size()==2&&C->getArgOperand(1)==G&&
        C->getFunction()==Outer->getHeader()->getParent()&&!Outer->contains(C)&&DT.dominates(C,Outer->getHeader()->getTerminator()))continue;
    if(auto *L=dyn_cast<LoadInst>(U)){if(L->getPointerOperand()!=G||L->isAtomic()||L->isVolatile())return false;continue;}
    auto *S=dyn_cast<StoreInst>(U);
    if(!S||S->getPointerOperand()!=G||S->getFunction()!=Outer->getHeader()->getParent()||Outer->contains(S)||!DT.dominates(S,Outer->getHeader()->getTerminator()))return false;
  }return true;
}
static bool iterationPrivatePointer(Value *V,GlobalVariable *Slot,SmallPtrSetImpl<Value *> &Seen) {
  if(!Seen.insert(V).second)return true;
  for(User *U:V->users()) {
    if(auto *G=dyn_cast<GetElementPtrInst>(U)){if(!iterationPrivatePointer(G,Slot,Seen))return false;continue;}
    if(auto *C=dyn_cast<BitCastInst>(U)){if(!iterationPrivatePointer(C,Slot,Seen))return false;continue;}
    if(auto *L=dyn_cast<LoadInst>(U)){if(L->getPointerOperand()!=V||L->getType()->isPointerTy()||L->isAtomic()||L->isVolatile())return false;continue;}
    if(auto *S=dyn_cast<StoreInst>(U)) {
      if(S->getValueOperand()==V&&S->getPointerOperand()==Slot)continue;
      if(S->getValueOperand()==V)if(auto *A=dyn_cast<AllocaInst>(S->getPointerOperand())) {
        bool Private=true;
        for(User *Use:A->users()) {
          if(auto *St=dyn_cast<StoreInst>(Use)){Private&=St->getPointerOperand()==A&&St->getValueOperand()==V;continue;}
          if(auto *L=dyn_cast<LoadInst>(Use)){Private&=L->getType()->isPointerTy()&&iterationPrivatePointer(L,Slot,Seen);continue;}
          Private=false;
        }
        if(Private)continue;
      }
      if(S->getPointerOperand()==V&&!S->getValueOperand()->getType()->isPointerTy()&&!S->isAtomic()&&!S->isVolatile())continue;
      return false;
    }
    if(auto *Cmp=dyn_cast<ICmpInst>(U)) {
      if(Cmp->isEquality()&&(isa<ConstantPointerNull>(Cmp->getOperand(0))||isa<ConstantPointerNull>(Cmp->getOperand(1))))continue;
      return false; // Pointer ordering/identity may distinguish buffer rotation.
    }
    return false;
  }return true;
}
static bool iterationPrivateArray(GlobalVariable *G) {
  SmallPtrSet<Value *,4> Sites;if(!executionArrayAllocations(G,Sites))return false;
  SmallPtrSet<Value *,32> Seen;
  for(Value *Site:Sites)if(!iterationPrivatePointer(Site,G,Seen))return false;
  for(User *U:G->users())if(auto *L=dyn_cast<LoadInst>(U))if(!iterationPrivatePointer(L,G,Seen))return false;
  return true;
}
static bool iterationGateSafe(Function *Gate,GlobalVariable *Destination,Loop *Outer,DominatorTree &DT) {
  if(!Gate)return true;if(!Gate->hasLocalLinkage()||Gate->arg_size()!=3){if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-memory] gate arity/linkage: "<<Gate->getName()<<" "<<Gate->arg_size()<<"\n";return false;}
  ValueToValueMapTy VM;auto *F=CloneFunction(Gate,VM);executionCleanup(F);bool Safe=!F->hasFnAttribute(Attribute::StrictFP);
  unsigned Sets=0;
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    bool Before=Safe;
    if(isa<StoreInst>(I)||isa<AtomicRMWInst>(I)||isa<FenceInst>(I))Safe=false;
    if(auto *L=dyn_cast<LoadInst>(&I)) {
      Safe&=!L->isAtomic()&&!L->isVolatile()&&L->getPointerOperand()!=Destination;
      if(auto *G=dyn_cast<GlobalVariable>(L->getPointerOperand()))Safe&=iterationStableSlot(G,Outer,DT);
      if(!L->getType()->isPointerTy())if(auto *Slot=executionArraySlot(L->getPointerOperand())) {
        SmallPtrSet<Value *,4> A,B;Safe&=executionArrayAllocations(Slot,A)&&executionArrayAllocations(Destination,B);
        for(Value *Site:A)if(B.count(Site))Safe=false;
      }
    }
    if(auto *C=dyn_cast<CallBase>(&I)){auto *G=C->getCalledFunction();Safe&=G&&G->getName()=="autograph_exec_ctx_set_source_gate"&&C->arg_size()==3&&C->getArgOperand(0)==F->getArg(1)&&C->getArgOperand(1)==F->getArg(2);++Sets;}
    if(auto *B=dyn_cast<BinaryOperator>(&I))if(B->getOpcode()==Instruction::SDiv||B->getOpcode()==Instruction::UDiv||B->getOpcode()==Instruction::SRem||B->getOpcode()==Instruction::URem)Safe=false;
    if(Before&&!Safe&&std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-memory] source gate refuses "<<I<<"\n";
  }
  // The context may only be passed to the checked source-gate setter.
  for(User *U:F->getArg(1)->users())if(!isa<CallBase>(U))Safe=false;
  DominatorTree GateDT(*F);LoopInfo GateLI(GateDT);Safe&=GateLI.empty();
  F->eraseFromParent();return Safe&&Sets>0;
}
static bool iterationGateImmutable(Function *Gate,GlobalVariable *Source,GlobalVariable *Destination,Loop *Outer,DominatorTree &DT) {
  if(!Gate)return true;
  SmallPtrSet<Value *,4> Mutable;
  if(!executionArrayAllocations(Source,Mutable)||!executionArrayAllocations(Destination,Mutable))return false;
  for(BasicBlock &BB:*Gate)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I)) {
    if(L->isAtomic()||L->isVolatile())return false;
    if(auto *G=dyn_cast<GlobalVariable>(L->getPointerOperand())) {
      if(!iterationStableSlot(G,Outer,DT))return false;
    } else {
      auto *Slot=executionArraySlot(L->getPointerOperand());SmallPtrSet<Value *,4> Sites;
      if(!Slot||!iterationStableSlot(Slot,Outer,DT)||!iterationPrivateArray(Slot)||!executionArrayAllocations(Slot,Sites))return false;
      for(Value *Site:Sites)if(Mutable.count(Site))return false;
      // The complete window already admits only the selected pair (one store
      // to Destination), seed/copy, the read-only gate, and checked runtime
      // infrastructure. No other private-array writer can run in that window.
      for(BasicBlock *B:Outer->blocks())for(Instruction &J:*B)if(auto *S=dyn_cast<StoreInst>(&J))
        if(executionArraySlot(S->getPointerOperand())==Slot)return false;
    }
  }
  return true;
}
#include "iteration_lifetime_lowering.inc"
#include "iteration_fusion_lowering.inc"
static void certifyIterationMemory(Module &M) {
  if((executionSimplifyRules()&7)!=7||std::getenv("SGPL_NO_ITERATION_MEMORY_LOWERING"))return;
  SmallVector<CallInst *,8> Certificates;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="sgpl_fullgraph_certificate")Certificates.push_back(C);
  for(auto *Cert:Certificates) {
    auto Refuse=[&](StringRef Why){if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-memory] refuse: "<<Why<<"\n";};
    Function *Main=Cert->getFunction();removeUnreachableBlocks(*Main);DominatorTree DT(*Main);LoopInfo LI(DT);
    Loop *Outer=LI.getLoopFor(Cert->getParent());if(!Outer){Refuse("surrounding iteration unavailable");continue;}
    auto *Create=dyn_cast<CallInst>(Cert->getArgOperand(0));auto *Pair=dyn_cast<Function>(Cert->getArgOperand(1));
    auto *GL=Create?dyn_cast<LoadInst>(Create->getArgOperand(0)):nullptr;auto *Graph=GL?dyn_cast<GlobalVariable>(GL->getPointerOperand()):nullptr;
    if(!Graph||!Pair||!iterationStableSlot(Graph,Outer,DT)){Refuse("stable graph identity unavailable");continue;}
    IterationSweep Init,Copy;
    for(Loop *Sub:Outer->getSubLoops()){auto S=iterationSweep(Sub,Graph);if(S.Seed&&!Init.L)Init=S;else if(S.From&&!Copy.L)Copy=S;}
    if(!Init.L||!Copy.L||Outer->getSubLoops().size()!=2||Copy.From!=Init.To||Copy.To==Init.To||!DT.dominates(Init.L->getHeader(),Cert->getParent())||!DT.dominates(Cert->getParent(),Copy.L->getHeader())){Refuse("ordered full-domain seed/traversal/copy unavailable");continue;}
    if(!iterationStableSlot(Init.Seed,Outer,DT)||!iterationStableSlot(Init.To,Outer,DT)||!iterationStableSlot(Copy.To,Outer,DT)){Refuse("array or seed pointer is mutable/published");continue;}
    if(!iterationPrivateArray(Init.To)||!iterationPrivateArray(Copy.To)){Refuse("array storage escapes or has atomic/volatile observers");continue;}
    // Use existing allocation-set and exact accumulator proofs on a temporary
    // canonical callback; retain the original callback as the late witness.
    ValueToValueMapTy VM;auto *Proof=CloneFunction(Pair,VM);iterationCanonicalize(Proof);
    unsigned Fields=0;auto K=executionCallbackSafe(Proof)?fullKernels(M,Proof,Fields):std::pair<Function *,Function *>{};
    auto Roles=K.first?executionRoles(K.first):std::map<std::string,BasicBlock *>{};
    auto *V=K.first?dyn_cast<PHINode>(&Roles["destination"]->front()):nullptr;
    auto A=V?executionAccumulator(K.first,Roles["body"],V):ExecutionAccumulator{};
    // Check the unhoisted callback as well: the source map must not conceal
    // reads of the evolving destination array (including at a source index).
    auto Original=executionAccumulator(Proof,&Proof->front(),Proof->getArg(3));
    bool Callback=!Proof->hasFnAttribute(Attribute::StrictFP)&&Original.Load&&A.Store&&executionArraySlot(A.Store->getPointerOperand())==Init.To&&Fields>0;
    bool ReadsSource=false;
    for(BasicBlock &BB:*Proof)for(Instruction &I:BB)if(auto *Load=dyn_cast<LoadInst>(&I)) {
      ReadsSource|=Load->getPointerOperand()==Copy.To;
      if(auto *G=dyn_cast<GlobalVariable>(Load->getPointerOperand()))Callback&=iterationStableSlot(G,Outer,DT);
    }
    Callback&=ReadsSource;
    if(K.first)K.first->eraseFromParent();if(K.second)K.second->eraseFromParent();Proof->eraseFromParent();
    if(!Callback){Refuse("ordered accumulator/source disjointness unavailable");continue;}
    Function *Gate=nullptr;unsigned Executes=0,Creates=0;bool Safe=true;StoreInst *CounterStore=nullptr;
    auto *Header=Outer->getHeader();auto *Br=dyn_cast<BranchInst>(Header->getTerminator());auto *Cmp=Br&&Br->isConditional()?dyn_cast<ICmpInst>(Br->getCondition()):nullptr;
    auto *CounterRead=Cmp?dyn_cast<LoadInst>(Cmp->getOperand(0)):nullptr,*BoundRead=Cmp?dyn_cast<LoadInst>(Cmp->getOperand(1)):nullptr;
    auto *Counter=CounterRead?dyn_cast<GlobalVariable>(CounterRead->getPointerOperand()):nullptr,*Bound=BoundRead?dyn_cast<GlobalVariable>(BoundRead->getPointerOperand()):nullptr;
    if(!Counter||!Bound||Cmp->getPredicate()!=ICmpInst::ICMP_SLT||Outer->contains(Br->getSuccessor(1))||!Outer->contains(Br->getSuccessor(0))||!iterationStableSlot(Bound,Outer,DT)){Refuse("bounded counter contract unavailable");continue;}
    ConstantInt *Limit=nullptr;bool Zero=false;
    for(User *U:Bound->users())if(auto *S=dyn_cast<StoreInst>(U)){auto *C=dyn_cast<ConstantInt>(S->getValueOperand());if(!C||!C->getType()->isIntegerTy(32)||C->getSExtValue()<=0||Limit)Safe=false;Limit=C;}
    for(User *U:Counter->users())if(auto *S=dyn_cast<StoreInst>(U)){if(Outer->contains(S)){if(CounterStore)Safe=false;CounterStore=S;}else {Zero=match(S->getValueOperand(),m_Zero())&&DT.dominates(S,Header->getTerminator());if(!Zero)Safe=false;}}else if(!isa<LoadInst>(U))Safe=false;
    auto *Add=CounterStore?dyn_cast<BinaryOperator>(CounterStore->getValueOperand()):nullptr;auto *Old=Add?dyn_cast<LoadInst>(Add->getOperand(0)):nullptr;
    Safe&=Limit&&Zero&&Add&&Add->getOpcode()==Instruction::Add&&match(Add->getOperand(1),m_One())&&Old&&Old->getPointerOperand()==Counter&&DT.dominates(CounterStore,Init.L->getHeader()->getTerminator());
    if(!Safe){Refuse("counter initialization/increment proof unavailable");continue;}
    for(BasicBlock *BB:Outer->blocks())for(Instruction &I:*BB) {
      bool Before=Safe;
      if(auto *Branch=dyn_cast<BranchInst>(&I))if(Branch->isConditional()&&BB!=Header&&BB!=Init.L->getHeader()&&BB!=Copy.L->getHeader()&&Branch->getSuccessor(0)!=Branch->getSuccessor(1))Safe=false;
      for(User *U:I.users())if(auto *Use=dyn_cast<Instruction>(U))Safe&=Outer->contains(Use);else Safe=false;
      if(auto *S=dyn_cast<StoreInst>(&I))Safe&=S==Init.Store||S==Copy.Store||S==CounterStore||isa<AllocaInst>(getUnderlyingObject(S->getPointerOperand()));
      if(auto *C=dyn_cast<CallInst>(&I)) {
        auto *F=C->getCalledFunction();if(!F){Safe=false;continue;}auto N=F->getName();
        if(N=="autograph_exec_op_create") {
          auto *P=dyn_cast<Function>(C->getArgOperand(2));auto *G=dyn_cast<Function>(C->getArgOperand(4));
          if(P&&P!=Pair)Safe=false;if(G){if(Gate)Safe=false;Gate=G;}
          for(unsigned J:{1u,3u,5u,6u,7u})Safe&=isa<ConstantPointerNull>(C->getArgOperand(J));
          continue;
        }
        if(N=="autograph_frontier_execute"){++Executes;Safe&=C->getArgOperand(1)==Create;continue;}
        if(N=="autograph_exec_ctx_create"){++Creates;Safe&=C==Create;for(unsigned J:{3u,4u,5u,7u,8u})Safe&=isa<ConstantPointerNull>(C->getArgOperand(J));continue;}
        if(N=="sgpl_fullgraph_certificate"||N=="sgpl_schedule_certificate"||N=="autograph_build_clean_cut"||N=="autograph_exec_ctx_set_dag_axes"||N=="autograph_exec_ctx_prepare_spatial"||N=="sgpl_exec_require_status"||N=="autograph_exec_ctx_destroy"||N=="autograph_profile_region_enter"||N=="autograph_profile_region_exit"||F->getIntrinsicID()==Intrinsic::stacksave||F->getIntrinsicID()==Intrinsic::stackrestore)continue;
        Safe=false;
      }
      if(auto *Load=dyn_cast<LoadInst>(&I))Safe&=!Load->isAtomic()&&!Load->isVolatile();
      if(isa<InvokeInst>(I)||isa<AtomicRMWInst>(I)||isa<AtomicCmpXchgInst>(I)||isa<FenceInst>(I))Safe=false;
      if(Before&&!Safe&&std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-memory] unobserved-window refusal at "<<I<<"\n";
    }
    bool GateSafe=iterationGateSafe(Gate,Init.To,Outer,DT);
    if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS")&&(!GateSafe||Executes!=1||Creates!=1))errs()<<"[iteration-memory] invocations="<<Executes<<" contexts="<<Creates<<" gate="<<GateSafe<<"\n";
    Safe&=Executes==1&&Creates==1&&GateSafe;
    if(!Safe||!Outer->getExitBlock()||!Outer->getExitBlock()->phis().empty()){Refuse("observable iteration effects or live state");continue;}
    bool StableGate=iterationGateImmutable(Gate,Copy.To,Init.To,Outer,DT);
    unsigned Observed=3;
    if(executionSimplifyRules()&SGPL_EXEC_R17) {
      if(iterationDeadArray(Copy.To,Outer,Pair,Gate))Observed&=~1u;
      if(iterationDeadArray(Init.To,Outer,Pair,Gate))Observed&=~2u;
    }
    // The original loop remains the complete pre-effect fallback. Only the
    // checked helper may rotate backing pointers, and restores both identities
    // and both final arrays before control reaches the original exit.
    SmallVector<BasicBlock *,4> Preds;for(BasicBlock *P:predecessors(Header))if(!Outer->contains(P))Preds.push_back(P);
    auto *Pre=SplitBlockPredecessors(Header,Preds,".iteration.preflight",&DT,&LI);
    IRBuilder<> B(Pre->getTerminator());auto *I=B.getInt32Ty();auto *P=B.getPtrTy();auto *T=StructType::get(M.getContext(),{I,I,P,P,P,P,P,P,B.getInt64Ty()});
    auto *Plan=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(1),B.getInt32(sizeof(sgpl_iteration_memory_plan_v1)),Pair,Gate?static_cast<Constant *>(Gate):ConstantPointerNull::get(P),Copy.To,Init.To,ConstantPointerNull::get(P),ConstantPointerNull::get(P),B.getInt64(0)}),"sgpl.iteration.memory.v1");
    Plan->setMetadata("sgpl.iteration.memory.proof",MDNode::get(M.getContext(),{MDString::get(M.getContext(),"scalar full-domain seed and copy; disjoint ordered accumulator; bounded private iteration; no user effects between phases")}));
    if((executionSimplifyRules()&SGPL_EXEC_R13)&&StableGate)
      Plan->setMetadata("sgpl.iteration.gate.immutable",MDNode::get(M.getContext(),{}));
    else if((executionSimplifyRules()&SGPL_EXEC_R13)&&std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[proof-removals] R13 refuse: gate reads mutable, aliased or unresolved data\n";
    if(StableGate)Plan->setMetadata("sgpl.iteration.map.gate.disjoint",MDNode::get(M.getContext(),{}));
    Plan->setMetadata("sgpl.iteration.final.uses",MDNode::get(M.getContext(),{ConstantAsMetadata::get(B.getInt32(Observed))}));
    auto *Result=B.CreateCall(M.getOrInsertFunction("sgpl_try_iteration_memory_v1",FunctionType::get(I,{P,P,B.getDoubleTy(),I},false)),{B.CreateLoad(P,Graph),Plan,B.CreateLoad(B.getDoubleTy(),Init.Seed),Limit});
    auto *Done=BasicBlock::Create(M.getContext(),"iteration.memory.handled",Main,Outer->getExitBlock());IRBuilder<> D(Done);D.CreateStore(Limit,Counter);D.CreateBr(Outer->getExitBlock());
    Pre->getTerminator()->eraseFromParent();B.SetInsertPoint(Pre);B.CreateCondBr(B.CreateICmpNE(Result,B.getInt32(0)),Done,Header);
    if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-memory] certify seed fusion, buffer rotation, parallel source map\n";
    if(verifyModule(M,&errs()))report_fatal_error("invalid iteration memory certificate");
  }
}
static void lowerIterationMemory(Module &M) {
  SmallVector<GlobalVariable *,4> Plans;for(auto &G:M.globals())if(G.getMetadata("sgpl.iteration.memory.proof"))Plans.push_back(&G);
  for(auto *Plan:Plans) {
    auto *S=cast<ConstantStruct>(Plan->getInitializer());auto *Pair=dyn_cast<Function>(S->getOperand(2));unsigned Fields=0;
    auto K=Pair&&!Pair->hasFnAttribute(Attribute::StrictFP)&&executionCallbackSafe(Pair)?fullKernels(M,Pair,Fields):std::pair<Function *,Function *>{};
    auto Fail=[&](){if(K.first)K.first->eraseFromParent();if(K.second)K.second->eraseFromParent();if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-memory] refuse late kernel proof\n";};
    if(!K.first||!K.second){Fail();continue;}
    auto Roles=executionRoles(K.first);auto *V=dyn_cast<PHINode>(&Roles["destination"]->front());auto A=executionAccumulator(K.first,Roles["body"],V);
    if(!A.Store||!executionAccumulator(Pair,&Pair->front(),Pair->getArg(3)).Load||executionArraySlot(A.Store->getPointerOperand())!=S->getOperand(5)){Fail();continue;}
    auto *Dest=cast<GlobalVariable>(S->getOperand(5));
    SmallVector<Type *,8> Args;for(auto &Arg:K.first->args())Args.push_back(Arg.getType());Args.push_back(Type::getDoubleTy(M.getContext()));
    auto *F=Function::Create(FunctionType::get(Type::getVoidTy(M.getContext()),Args,false),GlobalValue::InternalLinkage,"sgpl.iteration.seeded_kernel",M);
    ValueToValueMapTy VM;unsigned J=0;for(auto &Arg:K.first->args())VM[&Arg]=F->getArg(J++);SmallVector<ReturnInst *,4> Returns;CloneFunctionInto(F,K.first,VM,CloneFunctionChangeType::LocalChangesOnly,Returns);
    auto *Entry=cast<BasicBlock>(VM[&K.first->getEntryBlock()]),*Row=cast<BasicBlock>(VM[Roles["row"]]),*Next=cast<BasicBlock>(VM[Roles["next.destination"]]);
    IRBuilder<> B(&*Entry->getFirstInsertionPt());auto *Acc=B.CreateAlloca(B.getDoubleTy());auto *DestinationBase=B.CreateLoad(B.getPtrTy(),Dest);
    B.SetInsertPoint(Row->getTerminator());B.CreateStore(F->getArg(7),Acc);
    cast<LoadInst>(VM[A.Load])->setOperand(0,Acc);cast<StoreInst>(VM[A.Store])->setOperand(1,Acc);
    B.SetInsertPoint(Next->getTerminator());B.CreateStore(B.CreateLoad(B.getDoubleTy(),Acc),B.CreateGEP(B.getDoubleTy(),DestinationBase,B.CreateSExt(cast<Value>(VM[V]),B.getInt64Ty())));
    for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))if(executionPointerKind(L,F,false)==1||executionPointerKind(L,F,false)==2)L->replaceAllUsesWith(ConstantPointerNull::get(B.getPtrTy()));
    iterationNonnullGate(F,false);executionCleanup(F);F->setMetadata("sgpl.exec.fullgraph",nullptr);F->setSection(".sgpl_iteration_memory");
    Args.clear();for(auto &Arg:K.second->args())Args.push_back(Arg.getType());Args.push_back(B.getInt64Ty());
    auto *Map=Function::Create(FunctionType::get(B.getVoidTy(),Args,false),GlobalValue::InternalLinkage,"sgpl.iteration.source_range",M);
    VM.clear();J=0;for(auto &Arg:K.second->args())VM[&Arg]=Map->getArg(J++);Returns.clear();CloneFunctionInto(Map,K.second,VM,CloneFunctionChangeType::LocalChangesOnly,Returns);
    auto *MapEntry=cast<BasicBlock>(VM[&K.second->getEntryBlock()]);unsigned Starts=0;
    for(BasicBlock &BB:*Map)for(PHINode &Phi:BB.phis())if(Phi.getType()->isIntegerTy(64)) {int N=Phi.getBasicBlockIndex(MapEntry);if(N>=0&&match(Phi.getIncomingValue(N),m_Zero())){Phi.setIncomingValue(N,Map->getArg(6));++Starts;}}
    if(Starts!=1)report_fatal_error("invalid generated source range");iterationNonnullGate(Map,true);iterationCanonicalize(Map);Map->setSection(".sgpl_iteration_memory");
    // Check provenance before replacing the initializer: LLVM may retain its
    // now-unused constant until module cleanup, which is not an escape.
    auto *Source=cast<GlobalVariable>(S->getOperand(4));
    bool LocalMap=Fields==1&&iterationMapLocal(Map,Source);
    bool KernelDisjoint=iterationKernelDisjoint(F,Source);
    SmallVector<Constant *,9> Values;for(unsigned N=0;N<9;++N)Values.push_back(cast<Constant>(S->getOperand(N)));Values[6]=F;Values[7]=Map;Values[8]=ConstantInt::get(B.getInt64Ty(),Fields*8);
    Plan->setInitializer(ConstantStruct::get(cast<StructType>(S->getType()),Values));
    GlobalVariable *Extra=nullptr;
    if(Plan->getMetadata("sgpl.iteration.gate.immutable")&&(executionSimplifyRules()&SGPL_EXEC_R13)) {
      ValueToValueMapTy GateVM;auto *Ungated=CloneFunction(F,GateVM);Ungated->setName("sgpl.iteration.ungated_kernel");
      for(BasicBlock &BB:*Ungated)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))if(L->getType()->isIntegerTy(8))
        if(executionPointerKind(getUnderlyingObject(L->getPointerOperand()),Ungated,false)==0)L->replaceAllUsesWith(ConstantInt::get(L->getType(),1));
      iterationCanonicalize(Ungated);
      GateVM.clear();auto *UngatedMap=CloneFunction(Map,GateVM);UngatedMap->setName("sgpl.iteration.ungated_map");
      for(BasicBlock &BB:*UngatedMap)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))if(L->getType()->isIntegerTy(8)&&getUnderlyingObject(L->getPointerOperand())==UngatedMap->getArg(4))L->replaceAllUsesWith(ConstantInt::get(L->getType(),1));
      iterationCanonicalize(UngatedMap);
      auto *T=StructType::get(M.getContext(),{B.getInt32Ty(),B.getInt32Ty(),B.getInt32Ty(),B.getInt32Ty(),B.getPtrTy(),B.getPtrTy(),B.getPtrTy(),B.getPtrTy()});
      if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_iteration_gate_plan_v1))report_fatal_error("gate descriptor mismatch");
      Extra=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(1),B.getInt32(sizeof(sgpl_iteration_gate_plan_v1)),B.getInt32(SGPL_EXEC_R13),B.getInt32(0),Plan,Pair,Ungated,UngatedMap}),"sgpl.iteration.gate.v1");
      SmallVector<CallInst *,4> Calls;for(User *U:Plan->users())if(auto *C=dyn_cast<CallInst>(U))if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="sgpl_try_iteration_memory_v1")Calls.push_back(C);
      for(auto *C:Calls) {
        IRBuilder<> Call(C);auto *Result=Call.CreateCall(M.getOrInsertFunction("sgpl_try_iteration_gate_v1",FunctionType::get(B.getInt32Ty(),{B.getPtrTy(),B.getPtrTy(),B.getPtrTy(),B.getDoubleTy(),B.getInt32Ty()},false)),{C->getArgOperand(0),Plan,Extra,C->getArgOperand(2),C->getArgOperand(3)});
        C->replaceAllUsesWith(Result);C->eraseFromParent();
      }
      if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[proof-removals] R13 certify: immutable private gate; pre-effect edge implication witness; ungated clones\n";
    }
    unsigned Observed=3;uint64_t Facts=0;
    if(auto *Uses=Plan->getMetadata("sgpl.iteration.final.uses")) {
      Observed=mdconst::extract<ConstantInt>(Uses->getOperand(0))->getZExtValue();
      if(Observed!=3)Facts|=SGPL_FACT_FINAL_USES;
    }
    if(LocalMap)Facts|=SGPL_FACT_MAP_LOCAL;
    if(KernelDisjoint)Facts|=SGPL_FACT_KERNEL_DISJOINT;
    if(std::getenv("SGPL_LIFETIME_DEBUG")&&(executionSimplifyRules()&SGPL_EXEC_R18)&&!(Facts&SGPL_FACT_MAP_LOCAL)){errs()<<"map locality refuses fields="<<Fields<<"\n";Map->print(errs());}
    if(Plan->getMetadata("sgpl.iteration.map.gate.disjoint"))Facts|=SGPL_FACT_GATE_DISJOINT;
    auto Life=sgpl_iteration_lifetimes(Facts,executionSimplifyRules());unsigned Rules=Life.rules&(SGPL_EXEC_R17|SGPL_EXEC_R18);
    if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS")) {
      errs()<<"[iteration-lifetime] R17 "<<(Rules&SGPL_EXEC_R17?"certify":"refuse")<<": final live-array mask="<<Observed<<"\n";
      errs()<<"[iteration-lifetime] R18 "<<(Rules&SGPL_EXEC_R18?"certify":"refuse")<<": one local map field, disjoint gate and no remaining kernel rank reads required\n";
    }
    GlobalVariable *Storage=nullptr;
    if(Rules) {
      auto *T=StructType::get(M.getContext(),{B.getInt32Ty(),B.getInt32Ty(),B.getInt32Ty(),B.getInt32Ty(),B.getPtrTy(),B.getPtrTy(),B.getPtrTy()});
      if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_iteration_storage_plan_v1))report_fatal_error("storage descriptor mismatch");
      Storage=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(1),B.getInt32(sizeof(sgpl_iteration_storage_plan_v1)),B.getInt32(Rules),B.getInt32(Observed),Plan,Pair,Extra?static_cast<Constant *>(Extra):ConstantPointerNull::get(B.getPtrTy())}),"sgpl.iteration.storage.v1");
      SmallVector<CallInst *,4> Calls;
      for(User *U:Plan->users())if(auto *C=dyn_cast<CallInst>(U))if(C->getCalledFunction()) {
        auto N=C->getCalledFunction()->getName();if(N=="sgpl_try_iteration_gate_v1"||N=="sgpl_try_iteration_memory_v1")Calls.push_back(C);
      }
      for(auto *C:Calls) {
        bool GateCall=C->arg_size()==5;IRBuilder<> Call(C);
        auto *Result=Call.CreateCall(M.getOrInsertFunction("sgpl_try_iteration_storage_v1",FunctionType::get(B.getInt32Ty(),{B.getPtrTy(),B.getPtrTy(),B.getPtrTy(),B.getDoubleTy(),B.getInt32Ty()},false)),{C->getArgOperand(0),Plan,Storage,C->getArgOperand(GateCall?3:2),C->getArgOperand(GateCall?4:3)});
        C->replaceAllUsesWith(Result);C->eraseFromParent();
      }
    }
    if(executionSimplifyRules()&SGPL_EXEC_R19) {
      Function *Fused=nullptr;
      uint64_t FusionFacts=Facts;
      if(Extra&&LocalMap&&KernelDisjoint&&(Facts&SGPL_FACT_GATE_DISJOINT)) {
        auto *GateS=cast<ConstantStruct>(Extra->getInitializer());
        Fused=iterationFusedKernel(M,cast<Function>(GateS->getOperand(6)),cast<Function>(GateS->getOperand(7)),Source,Dest);
        if(Fused)FusionFacts|=SGPL_FACT_GATE_ENTAILED|SGPL_FACT_FUSION_SHAPE;
      }
      auto Fusion=sgpl_iteration_fusion(FusionFacts,executionSimplifyRules());
      if(std::getenv("SGPL_FUSION_DEBUG")) {
        errs()<<"fusion facts="<<FusionFacts<<" local="<<LocalMap<<" disjoint="<<KernelDisjoint<<" extra="<<!!Extra<<" generated="<<!!Fused<<"\n";
        if(Extra){auto *X=cast<ConstantStruct>(Extra->getInitializer());cast<Function>(X->getOperand(6))->print(errs());cast<Function>(X->getOperand(7))->print(errs());}
      }
      if(Fusion.rules&SGPL_EXEC_R19) {
        auto *T=StructType::get(M.getContext(),{B.getInt32Ty(),B.getInt32Ty(),B.getInt32Ty(),B.getInt32Ty(),B.getPtrTy(),B.getPtrTy(),B.getPtrTy(),B.getPtrTy(),B.getPtrTy()});
        if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_iteration_fusion_plan_v1))report_fatal_error("fusion descriptor mismatch");
        auto *Descriptor=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(1),B.getInt32(sizeof(sgpl_iteration_fusion_plan_v1)),B.getInt32(SGPL_EXEC_R19),B.getInt32(0),Plan,Pair,Extra,Storage?static_cast<Constant *>(Storage):ConstantPointerNull::get(B.getPtrTy()),Fused}),"sgpl.iteration.fusion.v1");
        SmallVector<CallInst *,4> Calls;
        for(User *U:Plan->users())if(auto *C=dyn_cast<CallInst>(U))if(C->getCalledFunction()) {
          auto N=C->getCalledFunction()->getName();if(N=="sgpl_try_iteration_storage_v1"||N=="sgpl_try_iteration_gate_v1"||N=="sgpl_try_iteration_memory_v1")Calls.push_back(C);
        }
        for(auto *C:Calls) {
          unsigned Tail=C->arg_size()==5?3:2;IRBuilder<> Call(C);
          auto *Result=Call.CreateCall(M.getOrInsertFunction("sgpl_try_iteration_fusion_v1",FunctionType::get(B.getInt32Ty(),{B.getPtrTy(),B.getPtrTy(),B.getPtrTy(),B.getDoubleTy(),B.getInt32Ty()},false)),{C->getArgOperand(0),Plan,Descriptor,C->getArgOperand(Tail),C->getArgOperand(Tail+1)});
          C->replaceAllUsesWith(Result);C->eraseFromParent();
        }
      } else if(Fused)Fused->eraseFromParent();
      if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-fusion] R19 "<<(Fusion.rules&SGPL_EXEC_R19?"certify":"refuse")<<": local pure map, immutable edge-entailed gate, rank-disjoint kernel and singleton map shape required\n";
    }
    K.first->eraseFromParent();K.second->eraseFromParent();
    if(std::getenv("SGPL_EXEC_SIMPLIFY_STATS"))errs()<<"[iteration-memory] activate seeded accumulator, empty destinations, rotating buffers, range source map\n";
  }
  if(verifyModule(M,&errs()))report_fatal_error("invalid proof-removal kernels");
}
}
