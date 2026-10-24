/* Specialize certified executor implementations, never arbitrary user loops.
 * Each rule checks its premises; the original kernel remains executable. */
#include "execution_simplify_plan.h"
#include "frontier_simplify_plan.h"
#include "relax_simplify_plan.h"
#include "semantic_stage_plan.h"
#include <llvm/Transforms/Utils/ValueMapper.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>
#include <map>
namespace {
using namespace llvm;
static unsigned executionSimplifyRules() {
  const char *E=std::getenv("SGPL_EXEC_SIMPLIFY_RULES");
  if(!E||StringRef(E)=="all")return 524287;
  if(StringRef(E)=="none")return 0;
  SmallVector<StringRef,4> Parts;StringRef(E).split(Parts,',');unsigned Mask=0;
  for(auto P:Parts) {
    if(P=="R1")Mask|=1;else if(P=="R2")Mask|=2;else if(P=="R3")Mask|=4;
    else if(P=="R4")Mask|=8;else if(P=="R5")Mask|=16;
    else if(P=="R6")Mask|=32;else if(P=="R7")Mask|=64;else if(P=="R8")Mask|=128;
    else if(P=="R9")Mask|=256;
    else if(P=="R10")Mask|=512;else if(P=="R11")Mask|=1024;else if(P=="R12")Mask|=2048;
    else if(P=="R13")Mask|=4096;else if(P=="R14")Mask|=8192;else if(P=="R15")Mask|=16384;
    else if(P=="R16")Mask|=32768;else if(P=="R17")Mask|=65536;else if(P=="R18")Mask|=131072;
    else if(P=="R19")Mask|=262144;
    else report_fatal_error("SGPL_EXEC_SIMPLIFY_RULES expects all, none, or R1 through R19");
  }
  return Mask;
}
static std::map<std::string,BasicBlock *> executionRoles(Function *F) {
  std::map<std::string,BasicBlock *> R;
  for(BasicBlock &B:*F)if(auto *M=B.getTerminator()->getMetadata("sgpl.exec.role"))
    if(M->getNumOperands()==1)if(auto *S=dyn_cast<MDString>(M->getOperand(0)))R[S->getString().str()]=&B;
  return R;
}
static bool executionCallbackSafe(Function *F) {
  if(!F||!F->hasLocalLinkage()||F->arg_size()!=4||F->size()!=1||!F->getArg(1)->use_empty())return false;
  for(Instruction &I:F->front()) {
    if(auto *L=dyn_cast<LoadInst>(&I)){if(L->isAtomic()||L->isVolatile())return false;}
    else if(auto *S=dyn_cast<StoreInst>(&I)){if(S->isAtomic()||S->isVolatile())return false;}
    else if(!(isa<ReturnInst>(I)||isa<GetElementPtrInst>(I)||isa<CastInst>(I)||
      isa<BinaryOperator>(I)||isa<UnaryOperator>(I)||isa<CmpInst>(I)||isa<SelectInst>(I)))return false;
    for(Value *V:I.operand_values())if(V->getType()->isPointerTy())
      if(auto *G=dyn_cast<GlobalVariable>(getUnderlyingObject(V)))if(G->isThreadLocal())return false;
  }
  return true;
}
/* Resolve a language array slot to its complete set of allocation sites.
 * Distinct slot names alone never imply disjoint buffers. */
static bool executionArrayAllocations(GlobalVariable *G,SmallPtrSetImpl<Value *> &Sites) {
  if(!G||!G->hasLocalLinkage()||!G->getMetadata("sgpl.language.array")||G->isThreadLocal())return false;
  for(User *U:G->users()) {
    // R16's compiler-certified count helper consumes the private slot without
    // retaining/publishing it. It writes the same source-local count locations.
    if(auto *C=dyn_cast<CallInst>(U))if(C->getMetadata("sgpl.source.count.proof")&&C->getCalledFunction()&&
        C->getCalledFunction()->getName()=="sgpl_try_source_count_v1"&&C->arg_size()==2&&C->getArgOperand(1)==G)continue;
    // The private iteration certificate restores both identities on return and
    // permits only exchanging these two independently allocated buffers while
    // its owned map/kernel phases run. Their mutual disjointness is invariant;
    // no pointer is published to arbitrary code through this descriptor.
    if(auto *S=dyn_cast<ConstantStruct>(U)) {
      bool Private=S->getNumOperands()==9&&(S->getOperand(4)==G||S->getOperand(5)==G)&&!S->use_empty();
      for(User *Use:S->users()){auto *Plan=dyn_cast<GlobalVariable>(Use);Private&=Plan&&Plan->hasPrivateLinkage()&&Plan->isConstant()&&Plan->getMetadata("sgpl.iteration.memory.proof");}
      if(Private)continue;
    }
    if(auto *L=dyn_cast<LoadInst>(U)){if(L->getPointerOperand()!=G||!L->getType()->isPointerTy()||L->isAtomic()||L->isVolatile())return false;continue;}
    auto *S=dyn_cast<StoreInst>(U);
    if(!S||S->getPointerOperand()!=G||S->isAtomic()||S->isVolatile())return false;
    auto *A=dyn_cast<CallBase>(S->getValueOperand()->stripPointerCasts());
    if(!A||!A->getCalledFunction()||A->getCalledFunction()->getName()!="calloc"||!A->getMetadata("sgpl.language.array"))return false;
    Sites.insert(A);
  }
  return !Sites.empty();
}
static GlobalVariable *executionArraySlot(Value *P) {
  auto *L=dyn_cast<LoadInst>(getUnderlyingObject(P));
  return L&&L->getType()->isPointerTy()?dyn_cast<GlobalVariable>(L->getPointerOperand()->stripPointerCasts()):nullptr;
}
struct ExecutionAccumulator {LoadInst *Load=nullptr;StoreInst *Store=nullptr;};
static ExecutionAccumulator executionAccumulator(Function *K,BasicBlock *Body,Value *Destination) {
  ExecutionAccumulator A;StoreInst *Store=nullptr;
  for(Instruction &I:*Body)if(auto *S=dyn_cast<StoreInst>(&I)){if(Store)return {};Store=S;}
  if(!Store||Store->isAtomic()||Store->isVolatile())return {};
  auto *Op=dyn_cast<BinaryOperator>(Store->getValueOperand());
  if(!Op||!((Op->getOpcode()==Instruction::FAdd&&Op->getType()->isDoubleTy()&&!Op->getFastMathFlags().any())||
    (Op->getOpcode()==Instruction::Add&&Op->getType()->isIntegerTy()&&!Op->hasNoSignedWrap()&&!Op->hasNoUnsignedWrap())))return {};
  auto *G=dyn_cast<GetElementPtrInst>(Store->getPointerOperand());
  if(!G||G->getNumIndices()!=1)return {};
  Value *Index=G->getOperand(1);if(auto *Cast=dyn_cast<SExtInst>(Index))Index=Cast->getOperand(0);
  if(Index!=Destination)return {};
  auto *Slot=executionArraySlot(G);SmallPtrSet<Value *,4> OutputSites;
  if(!executionArrayAllocations(Slot,OutputSites))return {};
  LoadInst *Old=nullptr;
  for(Value *V:Op->operands())if(auto *L=dyn_cast<LoadInst>(V))if(L->getPointerOperand()==G){if(Old)return {};Old=L;}
  if(!Old||!Old->hasOneUse())return {};
  for(Instruction &I:*Body) {
    if(auto *B=dyn_cast<BinaryOperator>(&I))if(B->getOpcode()==Instruction::SDiv||B->getOpcode()==Instruction::UDiv||
      B->getOpcode()==Instruction::SRem||B->getOpcode()==Instruction::URem)return {};
    if(auto *L=dyn_cast<LoadInst>(&I)) {
      if(L==Old)continue;
      if(L->isAtomic()||L->isVolatile())return {};
      if(L->getType()->isPointerTy()&&L->getPointerOperand()==Slot)continue;
      Value *Base=getUnderlyingObject(L->getPointerOperand());
      /* Source-map storage is private runtime allocation, disjoint from all
       * language arrays. Its identity/lifetime is checked by fullgraph bind. */
      if(K->arg_size()==7&&Base==K->getArg(6))continue;
      auto *Other=executionArraySlot(L->getPointerOperand());
      if(!Other) {
        if(auto *Global=dyn_cast<GlobalVariable>(L->getPointerOperand())) {
          if(Global->hasLocalLinkage()&&!Global->isThreadLocal()&&Global!=Slot)continue;
        }
        return {};
      }
      if(Other==Slot)return {};
      SmallPtrSet<Value *,4> ReadSites;if(!executionArrayAllocations(Other,ReadSites))return {};
      for(Value *Site:ReadSites)if(OutputSites.count(Site))return {};
    }
  }
  A.Load=Old;A.Store=Store;return A;
}
static void executionCleanup(Function *F) {
  DominatorTree DT(*F);SmallVector<AllocaInst *,8> Slots;
  for(Instruction &I:F->getEntryBlock())if(auto *A=dyn_cast<AllocaInst>(&I))if(isAllocaPromotable(A))Slots.push_back(A);
  PromoteMemToReg(Slots,DT);
  PassBuilder PB;LoopAnalysisManager LAM;FunctionAnalysisManager FAM;CGSCCAnalysisManager CGAM;ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);PB.registerCGSCCAnalyses(CGAM);PB.registerFunctionAnalyses(FAM);PB.registerLoopAnalyses(LAM);PB.crossRegisterProxies(LAM,FAM,CGAM,MAM);
  FunctionPassManager PM;cantFail(PB.parsePassPipeline(PM,"simplifycfg,dce"));PM.run(*F,FAM);
  if(verifyFunction(*F,&errs()))report_fatal_error("invalid execution simplification");
}
/* Peel the initial search for an executed edge. Only that search performs the
 * first tracking write / accumulator load. The steady loop has no first flag. */
static unsigned executionPeel(Function *K,unsigned Requested,Function *Callback) {
  auto R=executionRoles(K);
  for(const char *Name:{"row","incoming","visit","gate","gate.load","effect","mark","body","next.edge","error","next.destination","done","destination"})if(!R.count(Name))return 0;
  auto *V=dyn_cast<PHINode>(&R["destination"]->front());if(!V)return 0;
  auto A=executionAccumulator(K,R["body"],V);unsigned Applied=Requested&2;
  /* Also inspect the original expression: source-map substitution must not
   * conceal an observation of the evolving accumulator. */
  if(!executionAccumulator(Callback,&Callback->front(),Callback->getArg(3)).Load)A={};
  if((Requested&4)&&A.Load)Applied|=4;
  if(!Applied)return 0;
  LLVMContext &C=K->getContext();ValueToValueMapTy VM;SmallVector<BasicBlock *,12> Original;
  for(const char *Name:{"incoming","visit","gate","gate.load","effect","mark","body","next.edge","error"})Original.push_back(R[Name]);
  for(auto *B:Original){auto *Clone=CloneBasicBlock(B,VM,".steady",K);VM[B]=Clone;}
  for(auto *B:Original)for(Instruction &I:*cast<BasicBlock>(VM[B]))RemapInstruction(&I,VM,RF_IgnoreMissingLocals|RF_NoModuleLevelChanges);
  auto *Enter=BasicBlock::Create(C,"steady.enter",K);IRBuilder<> EB(Enter);
  auto *J=cast<PHINode>(&R["incoming"]->front());auto *SJ=cast<PHINode>(VM[J]);
  auto *Next=EB.CreateAdd(J,EB.getInt64(1));EB.CreateBr(cast<BasicBlock>(VM[R["incoming"]]));
  SJ->removeIncomingValue(R["row"],false);SJ->addIncoming(Next,Enter);
  R["body"]->getTerminator()->eraseFromParent();BranchInst::Create(Enter,R["body"]);
  if(Applied&2) {
    auto *Effect=cast<BasicBlock>(VM[R["effect"]]);Effect->getTerminator()->eraseFromParent();
    BranchInst::Create(cast<BasicBlock>(VM[R["body"]]),Effect);
  }
  if(Applied&4) {
    IRBuilder<> Entry(&*K->getEntryBlock().getFirstInsertionPt());auto *T=A.Load->getType();
    auto *Acc=Entry.CreateAlloca(T,nullptr,"ordered.accumulator");auto *Address=Entry.CreateAlloca(Entry.getPtrTy(),nullptr,"accumulator.address");
    auto *Active=Entry.CreateAlloca(Entry.getInt1Ty(),nullptr,"accumulator.active");
    IRBuilder<> Row(R["row"]->getTerminator());Row.CreateStore(Row.getFalse(),Active);
    auto *SteadyStore=cast<StoreInst>(VM[A.Store]);
    IRBuilder<> First(A.Store);First.CreateStore(A.Store->getPointerOperand(),Address);First.CreateStore(A.Store->getValueOperand(),Acc);First.CreateStore(First.getTrue(),Active);A.Store->eraseFromParent();
    auto *SteadyLoad=cast<LoadInst>(VM[A.Load]);SteadyLoad->setOperand(0,Acc);
    SteadyStore->setOperand(1,Acc);
    auto Flush=[&](BasicBlock *From,BasicBlock *To,const char *Name) {
      auto *Check=BasicBlock::Create(C,std::string(Name)+".check",K),*Write=BasicBlock::Create(C,std::string(Name)+".write",K);
      IRBuilder<> B(Check);B.CreateCondBr(B.CreateLoad(B.getInt1Ty(),Active),Write,To);
      B.SetInsertPoint(Write);auto *S=B.CreateStore(B.CreateLoad(T,Acc),B.CreateLoad(B.getPtrTy(),Address));S->setAlignment(A.Load->getAlign());B.CreateBr(To);
      auto *Term=From->getTerminator();for(unsigned I=0;I<Term->getNumSuccessors();++I)if(Term->getSuccessor(I)==To)Term->setSuccessor(I,Check);
    };
    Flush(R["incoming"],R["next.destination"],"initial.flush");
    Flush(cast<BasicBlock>(VM[R["incoming"]]),R["next.destination"],"steady.flush");
    Flush(R["error"],R["done"],"initial.error.flush");
    Flush(cast<BasicBlock>(VM[R["error"]]),R["done"],"steady.error.flush");
  }
  return Applied;
}
static int executionPointerKind(Value *V,Function *F,bool Map) {
  if(Map)return V==F->getArg(4)?0:-1;
  auto *L=dyn_cast<LoadInst>(V);if(!L||!L->getType()->isPointerTy())return -1;
  int64_t Off=0;if(GetPointerBaseWithConstantOffset(L->getPointerOperand(),Off,F->getParent()->getDataLayout())!=F->getArg(1))return -1;
  if(Off==offsetof(sgpl_exec_ctx,spatial_source_gate))return 0;
  if(Off==offsetof(sgpl_exec_ctx,spatial_groups_seen))return 1;
  if(Off==offsetof(sgpl_exec_ctx,execution_error))return 2;
  return -1;
}
static Function *executionUnswitch(Function *F,bool Map) {
  unsigned Count=Map?2:8;auto &M=*F->getParent();auto &C=M.getContext();
  auto *Wrapper=Function::Create(F->getFunctionType(),GlobalValue::InternalLinkage,F->getName()+".null_dispatch",M);
  Wrapper->addFnAttr(Attribute::NoInline);Wrapper->setSection(".sgpl_simplified_kernels");
  IRBuilder<> B(BasicBlock::Create(C,"entry",Wrapper));Value *Mask=B.getInt32(0);
  for(unsigned I=0;I<(Map?1u:3u);++I) {
    size_t Off=I==0?offsetof(sgpl_exec_ctx,spatial_source_gate):I==1?offsetof(sgpl_exec_ctx,spatial_groups_seen):offsetof(sgpl_exec_ctx,execution_error);
    Value *P=Map?Wrapper->getArg(4):fullField(B,Wrapper->getArg(1),Off,B.getPtrTy());
    Mask=B.CreateOr(Mask,B.CreateShl(B.CreateZExt(B.CreateIsNull(P),B.getInt32Ty()),B.getInt32(I)));
  }
  auto *Bad=BasicBlock::Create(C,"impossible",Wrapper);IRBuilder<>(Bad).CreateUnreachable();auto *Switch=B.CreateSwitch(Mask,Bad,Count);
  for(unsigned Bits=0;Bits<Count;++Bits) {
    ValueToValueMapTy VM;Function *Variant=CloneFunction(F,VM);Variant->setName(F->getName()+".null_"+Twine(Bits));Variant->setSection(".sgpl_simplified_kernels");
    SmallVector<ICmpInst *,16> Checks;
    for(BasicBlock &BB:*Variant)for(Instruction &I:BB)if(auto *Cmp=dyn_cast<ICmpInst>(&I))
      if((Cmp->getPredicate()==ICmpInst::ICMP_EQ||Cmp->getPredicate()==ICmpInst::ICMP_NE)&&isa<ConstantPointerNull>(Cmp->getOperand(1))&&executionPointerKind(Cmp->getOperand(0),Variant,Map)>=0)Checks.push_back(Cmp);
    for(auto *Cmp:Checks){int Kind=executionPointerKind(Cmp->getOperand(0),Variant,Map);bool Null=Bits&(1u<<Kind);Cmp->replaceAllUsesWith(ConstantInt::get(B.getInt1Ty(),Cmp->getPredicate()==ICmpInst::ICMP_EQ?Null:!Null));}
    executionCleanup(Variant);
    auto *Call=BasicBlock::Create(C,"case_"+Twine(Bits),Wrapper);Switch->addCase(B.getInt32(Bits),Call);B.SetInsertPoint(Call);
    SmallVector<Value *,8> Args;for(auto &A:Wrapper->args())Args.push_back(&A);B.CreateCall(Variant,Args);B.CreateRetVoid();
  }
  return Wrapper;
}
static void lowerExecutionSimplifications(Module &M) {
  unsigned Requested=executionSimplifyRules()&7;bool Stats=std::getenv("SGPL_EXEC_SIMPLIFY_STATS");
  SmallVector<CallInst *,8> Binds;
  for(Function &F:M)for(BasicBlock &B:F)for(Instruction &I:B)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getCalledFunction()&&Call->getCalledFunction()->getName()=="autograph_exec_ctx_bind_fullgraph_v1")Binds.push_back(Call);
  for(auto *Bind:Binds) {
    auto *Base=dyn_cast<Function>(Bind->getArgOperand(1)->stripPointerCasts());
    auto *Map=dyn_cast<Function>(Bind->getArgOperand(2)->stripPointerCasts());
    auto *Cert=Base?Base->getMetadata("sgpl.exec.fullgraph"):nullptr;
    auto *Value=Cert&&Cert->getNumOperands()==1?dyn_cast<ValueAsMetadata>(Cert->getOperand(0)):nullptr;
    auto *Callback=Value?dyn_cast<Function>(Value->getValue()):nullptr;
    if(!Requested||!executionCallbackSafe(Callback)) {
      if(Stats)errs()<<"[exec-simplify] refuse: disabled or missing callback/context certificate\n";continue;
    }
    ValueToValueMapTy VM;Function *K=CloneFunction(Base,VM);K->setName(Base->getName()+".simplified");K->setMetadata("sgpl.exec.fullgraph",nullptr);K->setSection(".sgpl_simplified_kernels");
    unsigned Applied=executionPeel(K,Requested,Callback);executionCleanup(K);
    Function *Optimized=K,*OptimizedMap=Map;
    if(Requested&1){Optimized=executionUnswitch(K,false);if(Map)OptimizedMap=executionUnswitch(Map,true);K->eraseFromParent();Applied|=1;}
    if(Stats)for(unsigned Rule=0;Rule<3;++Rule)if(Requested&(1u<<Rule))errs()<<"[exec-simplify] R"<<Rule+1<<" "<<((Applied&(1u<<Rule))?"activate":"refuse")<<" "<<Base->getName()<<((Rule==2&&!(Applied&4))?": accumulator shape/alias/effect proof unavailable":"")<<"\n";
    if(!Applied){K->eraseFromParent();continue;}
    IRBuilder<> B(Bind->getNextNode());auto *I=B.getInt32Ty();auto *P=B.getPtrTy();auto *T=StructType::get(M.getContext(),{I,I,I,I,P,P,P,P});
    if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_execution_simplify_plan_v1))report_fatal_error("execution simplification descriptor layout mismatch");
    auto Ptr=[&](Function *F)->Constant *{return F?static_cast<Constant *>(F):ConstantPointerNull::get(P);};
    auto *Plan=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(1),B.getInt32(sizeof(sgpl_execution_simplify_plan_v1)),B.getInt32(Applied),B.getInt32(0),Base,Ptr(Map),Optimized,Ptr(OptimizedMap)}),"sgpl.execution.simplification.v1");
    B.CreateCall(M.getOrInsertFunction("autograph_exec_ctx_bind_simplified_fullgraph_v1",FunctionType::get(B.getVoidTy(),{P,P},false)),{Bind->getArgOperand(0),Plan});
    M.getOrInsertNamedMetadata("sgpl.execution.simplification.proofs")->addOperand(MDNode::get(M.getContext(),{
      ValueAsMetadata::get(Base),ValueAsMetadata::get(Callback),ConstantAsMetadata::get(B.getInt32(Applied)),
      MDString::get(M.getContext(),"full-domain destination owners; transparent context-free callback; immutable context pointer spans"),
      MDString::get(M.getContext(),"R1: pointer nullness only, live gate/error loads retained; R2: first executed occurrence retains mark; R3: disjoint allocation sites, ordered original operations, lazy load and error-exit flush")}));
    if(verifyModule(M,&errs()))report_fatal_error("invalid execution simplification module");
  }
}
}
