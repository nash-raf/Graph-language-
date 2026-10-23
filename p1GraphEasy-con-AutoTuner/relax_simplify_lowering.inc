#include "relax_simplify_plan.h"
namespace {
using namespace llvm;
struct RelaxRecipe {GlobalVariable *Array=nullptr;bool Weighted=false;GlobalVariable *WeightGraph=nullptr;unsigned SnapshotSlot=0;};
/* Prove an entire acyclic callback rather than recognize an algorithm name.
 * Only compiler-authored weight accessor instructions get the trusted leaf
 * summary. The original checked first-match accessor stays in every kernel. */
static RelaxRecipe relaxRecipe(Function *Pair,MDNode *Origins) {
  RelaxRecipe R;if(!Pair||!Origins)return R;
  ValueToValueMapTy VM;Function *F=CloneFunction(Pair,VM);
  auto Finish=[&](RelaxRecipe X){F->eraseFromParent();return X;};
  if(!frontierInlineLocal(F))return Finish({});
  DominatorTree ProofDT(*F);LoopInfo ProofLoops(ProofDT);if(!ProofLoops.empty())return Finish({});
  if(std::getenv("SGPL_RELAX_PROOF_DEBUG"))F->print(errs());
  SmallVector<PHINode *,2> Weights;
  for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *P=dyn_cast<PHINode>(&I))if(P->getType()->isIntegerTy(32)) {
    bool Fallback=false,Native=false,Only=true;
    for(Value *V:P->incoming_values()) {
      if(auto *C=dyn_cast<CallInst>(V)){Only&=C->getCalledFunction()&&C->getCalledFunction()->getName()=="autograph_exec_ctx_edge_weight"&&C->arg_size()==4&&C->getArgOperand(0)==F->getArg(1)&&C->getArgOperand(2)==F->getArg(2)&&C->getArgOperand(3)==F->getArg(3);Fallback|=Only;}
      else if(auto *L=dyn_cast<LoadInst>(V)){Native=true;Only&=L->getType()->isIntegerTy(32)&&!L->isAtomic()&&!L->isVolatile();}
      else {auto *K=dyn_cast<ConstantInt>(V);Only&=K&&K->equalsInt(1);}
    }
    if(Only&&Fallback&&Native)Weights.push_back(P);
  }
  /* The accessor PHI must originate in our own trusted inline accessor. */
  if(Weights.size()>1)return Finish({});
  PHINode *Weight=nullptr;
  if(!Weights.empty()) {
    auto *P=Weights[0];bool Tagged=false;
    for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))
      if(frontierCtxField(L->getPointerOperand(),F->getArg(1),offsetof(sgpl_exec_ctx,array_edge_index),F->getParent()->getDataLayout())) {
        auto *MD=L->getMetadata("sgpl.ctx.accessor");auto *S=MD&&MD->getNumOperands()==1?dyn_cast<MDString>(MD->getOperand(0)):nullptr;
        Tagged|=S&&S->getString()=="autograph_exec_ctx_edge_weight";
      }
    if(!Tagged)return Finish({});
    Weight=P;R.Weighted=true;
  }
  executionCleanup(F);
  /* Only the min guard, one candidate store and one conditional append may
   * have effects. No other array read or context observation passes. */
  StoreInst *Store=nullptr;CallInst *Append=nullptr;ICmpInst *Compare=nullptr;
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    if(auto *S=dyn_cast<StoreInst>(&I)){if(Store)return Finish({});Store=S;}
    if(auto *Call=dyn_cast<CallInst>(&I)) {
      auto *C=Call->getCalledFunction();if(!C)return Finish({});auto N=C->getName();
        if(N=="autograph_frontier_activate"){if(Append||Call->arg_size()!=2||Call->getArgOperand(0)!=F->getArg(1)||Call->getArgOperand(1)!=F->getArg(3))return Finish({});Append=Call;}
      else if(!(N=="autograph_exec_ctx_edge_weight"&&Call->getMetadata("sgpl.relax.weight.accessor"))&&!C->isIntrinsic())return Finish({});
    }
  }
  if(!Store||!Append||Store->getParent()!=Append->getParent()||!Store->comesBefore(Append)||Store->isAtomic()||Store->isVolatile())return Finish({});
  auto *G=dyn_cast<GetElementPtrInst>(Store->getPointerOperand());
  auto *Base=G?dyn_cast<LoadInst>(G->getPointerOperand()):nullptr;
  auto *Slot=Base?dyn_cast<GlobalVariable>(Base->getPointerOperand()):nullptr;
  if(!Slot||!Slot->hasLocalLinkage()||!Slot->getMetadata("sgpl.language.array")||G->getNumIndices()!=1||G->getSourceElementType()!=Type::getInt32Ty(F->getContext())||!frontierSourceIndex(G->idx_begin()->get(),F->getArg(3)))return Finish({});
  SmallPtrSet<Value *,4> AllocationSites;if(!executionArrayAllocations(Slot,AllocationSites))return Finish({});
  Value *Candidate=Store->getValueOperand();LoadInst *Source=nullptr;
  if(R.Weighted) {
    auto *Add=dyn_cast<BinaryOperator>(Candidate);if(!Add||Add->getOpcode()!=Instruction::Add||Add->hasNoSignedWrap()||Add->hasNoUnsignedWrap())return Finish({});
    for(unsigned I=0;I<2;++I)if(Add->getOperand(I)==Weight)Source=dyn_cast<LoadInst>(Add->getOperand(1-I));
  } else Source=dyn_cast<LoadInst>(Candidate);
  if(!Source||!Source->getType()->isIntegerTy(32)||!frontierSnapshotElement(Source,F->getArg(1),F->getArg(2),F->getParent()->getDataLayout()))return Finish({});
  auto *SourceG=cast<GetElementPtrInst>(Source->getPointerOperand());auto *Snapshot=cast<LoadInst>(SourceG->getPointerOperand());int64_t Off=0;
  GetPointerBaseWithConstantOffset(Snapshot->getPointerOperand(),Off,F->getParent()->getDataLayout());bool Origin=false;
  for(auto &O:Origins->operands())if(auto *T=dyn_cast<MDNode>(O))if(T->getNumOperands()==2) {
    auto *N=dyn_cast<ConstantAsMetadata>(T->getOperand(0));auto *V=dyn_cast<ValueAsMetadata>(T->getOperand(1));auto *K=N?dyn_cast<ConstantInt>(N->getValue()):nullptr;
    Origin|=K&&V&&K->getZExtValue()==(uint64_t)Off/8&&V->getValue()==Slot;
  }
  if(!Origin)return Finish({});
  R.SnapshotSlot=(unsigned)Off/8;
  if(R.Weighted)for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))
    if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="autograph_exec_ctx_edge_weight") {
      auto *L=dyn_cast<LoadInst>(C->getArgOperand(1));R.WeightGraph=L?dyn_cast<GlobalVariable>(L->getPointerOperand()):nullptr;
      if(!R.WeightGraph)return Finish({});
    }
  // The only conditional user-effect block must be the true arm of c < x[v].
  if(!Store->getParent()->hasNPredecessors(1))return Finish({});
  auto *Guard=dyn_cast<BranchInst>(Store->getParent()->getSinglePredecessor()->getTerminator());
  Compare=Guard&&Guard->isConditional()?dyn_cast<ICmpInst>(Guard->getCondition()):nullptr;
  auto *Old=Compare?dyn_cast<LoadInst>(Compare->getOperand(1)):nullptr;
  if(!Compare||Compare->getPredicate()!=ICmpInst::ICMP_SLT||Compare->getOperand(0)!=Candidate||!Old||Old->getPointerOperand()!=Store->getPointerOperand()||Guard->getSuccessor(0)!=Store->getParent())return Finish({});
  SmallPtrSet<Instruction *,32> Allowed;SmallVector<Value *,16> Work{Candidate,Old,Store->getPointerOperand()};
  while(!Work.empty()){Value *V=Work.pop_back_val();auto *I=dyn_cast<Instruction>(V);if(!I||!Allowed.insert(I).second)continue;for(Value *Op:I->operand_values())if(isa<Instruction>(Op))Work.push_back(Op);}
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    if(auto *L=dyn_cast<LoadInst>(&I))if(L->isAtomic()||L->isVolatile()||
        (!Allowed.count(L)&&!L->getMetadata("sgpl.relax.weight.accessor")&&
         !frontierCtxField(L->getPointerOperand(),F->getArg(1),offsetof(sgpl_exec_ctx,spatial_snapshot_count),F->getParent()->getDataLayout())))return Finish({});
    if(auto *B=dyn_cast<BranchInst>(&I))if(B->isConditional()&&B!=Guard&&!B->getMetadata("sgpl.relax.weight.accessor"))return Finish({});
    if(isa<SwitchInst>(I)||isa<IndirectBrInst>(I)||isa<InvokeInst>(I)||isa<AtomicRMWInst>(I)||isa<AtomicCmpXchgInst>(I)||isa<FenceInst>(I))return Finish({});
    if(auto *Op=dyn_cast<BinaryOperator>(&I))if(Op->getOpcode()==Instruction::SDiv||Op->getOpcode()==Instruction::UDiv||Op->getOpcode()==Instruction::SRem||Op->getOpcode()==Instruction::URem||Op->getType()->isFPOrFPVectorTy())return Finish({});
    if(auto *Call=dyn_cast<CallBase>(&I))if(Call->getCalledFunction()->isIntrinsic()&&Call->getIntrinsicID()!=Intrinsic::assume&&Call->getIntrinsicID()!=Intrinsic::lifetime_start&&Call->getIntrinsicID()!=Intrinsic::lifetime_end)return Finish({});
  }
  R.Array=Slot;return Finish(R);
}
static bool relaxPrivateContext(Value *Ctx) {
  auto *Create=dyn_cast<CallInst>(Ctx);
  if(!Create||!Create->getCalledFunction()||Create->getCalledFunction()->getName()!="autograph_exec_ctx_create")return false;
  unsigned Executes=0;
  for(User *U:Ctx->users()) {
    if(isa<ICmpInst>(U))continue;
    auto *C=dyn_cast<CallBase>(U);auto *F=C?C->getCalledFunction():nullptr;if(!F||C->getNumOperandBundles())return false;
    auto N=F->getName();bool Known=N=="autograph_exec_ctx_set_dag_axes"||N=="autograph_exec_ctx_bind_array_frontier"||
      N=="autograph_exec_ctx_prepare_spatial"||N=="autograph_exec_ctx_reserve_prepared_output"||N=="autograph_exec_ctx_bind_source_snapshot_range"||
      N=="autograph_exec_ctx_bind_snapshot_range"||N=="autograph_exec_ctx_destroy"||N=="sgpl_exec_bind_append_journal_v1"||N=="sgpl_exec_bind_simplified_frontier_v1"||N=="autograph_frontier_execute";
    unsigned At=N=="autograph_frontier_execute"?1:0;
    if(N=="autograph_frontier_execute")++Executes;
    if(!Known||C->arg_size()<=At||C->getArgOperand(At)!=Ctx)return false;
    for(unsigned I=0;I<C->arg_size();++I)if(I!=At&&C->getArgOperand(I)==Ctx)return false;
  }
  return Executes==1;
}
/* Preparation may read the live array only while it still equals the source
 * snapshot captured immediately afterward. Prove the complete instruction
 * window; unknown calls or writes refuse rather than assume quiescence. */
static bool relaxCaptureWindow(CallInst *Bind) {
  Value *Ctx=Bind->getArgOperand(0);bool Prepared=false,Captured=false;
  auto *Create=dyn_cast<CallInst>(Ctx);if(!Create||Create->getParent()!=Bind->getParent())return false;
  for(Instruction *I=Create->getNextNode();I;I=I->getNextNode()) {
    if(I->mayWriteToMemory()&&!isa<CallInst>(I))return false;
    auto *C=dyn_cast<CallInst>(I);if(!C)continue;
    auto *F=C->getCalledFunction();if(!F)return false;auto N=F->getName();
    if(N=="autograph_frontier_execute")return Prepared&&Captured&&C->getArgOperand(1)==Ctx;
    if(N=="autograph_exec_ctx_prepare_spatial"){if(Prepared||C->getArgOperand(0)!=Ctx)return false;Prepared=true;continue;}
    if(N=="autograph_exec_ctx_bind_source_snapshot_range"||N=="autograph_exec_ctx_bind_snapshot_range") {
      if(!Prepared||C->getArgOperand(0)!=Ctx)return false;Captured=true;continue;
    }
    if(N=="sgpl_exec_require_status"||N=="autograph_exec_ctx_reserve_prepared_output"||
        N=="autograph_exec_ctx_set_dag_axes"||N=="autograph_exec_ctx_bind_array_frontier"||
        N=="sgpl_exec_bind_append_journal_v1"||N=="sgpl_exec_bind_simplified_frontier_v1"||
        N=="autograph_profile_region_enter"||N=="llvm.assume")continue;
    return false;
  }
  return false;
}
/* Strong, intentionally narrow consumer proof. All circulating pointers and
 * counts have only traversal/swap/emptiness uses. Initial buffers must have a
 * complete nonescaping use chain; outlined/opaque initialization refuses. */
static bool relaxSetConsumer(CallInst *Bind) {
  auto *Input=dyn_cast<PHINode>(Bind->getArgOperand(1));auto *Size=dyn_cast<PHINode>(Bind->getArgOperand(2));auto *Slot=dyn_cast<GlobalVariable>(Bind->getArgOperand(3));
  if(!Input||!Size||!Slot||!Slot->hasLocalLinkage()||Input->getNumIncomingValues()!=2||Size->getNumIncomingValues()!=2)return false;
  DominatorTree DT(*Bind->getFunction());LoopInfo LI(DT);auto *Loop=LI.getLoopFor(Bind->getParent());if(!Loop||Loop->getNumBlocks()!=1)return false;
  Value *Ctx=Bind->getArgOperand(0);CallInst *Execute=nullptr;
  for(Instruction &I:*Bind->getParent())if(auto *C=dyn_cast<CallInst>(&I))if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="autograph_frontier_execute"&&C->getArgOperand(1)==Ctx){if(Execute)return false;Execute=C;}
  if(!Execute)return false;
  for(User *U:Execute->users()) {
    if(U==Size)continue;
    if(auto *C=dyn_cast<CallInst>(U)){if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="sgpl_exec_require_status")continue;}
    auto *C=dyn_cast<ICmpInst>(U);if(!C||C->getPredicate()!=ICmpInst::ICMP_SGT||C->getOperand(0)!=Execute||!isa<ConstantInt>(C->getOperand(1))||!cast<ConstantInt>(C->getOperand(1))->isZero()||!C->hasOneUse()||C->user_back()!=Loop->getHeader()->getTerminator())return false;
  }
  for(User *U:Size->users())if(U!=Bind)return false;
  for(User *U:Input->users())if(U!=Bind){auto *S=dyn_cast<StoreInst>(U);if(!S||S->getPointerOperand()!=Slot||S->getValueOperand()!=Input)return false;}
  auto Fresh=[&](Value *V)->bool {
    auto *C=dyn_cast<CallInst>(V);if(!C||!C->getCalledFunction()||C->getCalledFunction()->getName()!="calloc"||Loop->contains(C))return false;
    for(User *U:C->users()) {
      if(U==Input||isa<ICmpInst>(U))continue;
      if(auto *S=dyn_cast<StoreInst>(U)){if(S->getPointerOperand()==Slot)continue;return false;}
      auto *G=dyn_cast<GetElementPtrInst>(U);if(!G)return false;
      for(User *GU:G->users()){auto *S=dyn_cast<StoreInst>(GU);if(!S||S->getPointerOperand()!=G||Loop->contains(S))return false;}
    }
    return true;
  };
  for(User *U:Slot->users()) {
    if(U==Bind)continue;
    if(auto *S=dyn_cast<StoreInst>(U)){if(S->getValueOperand()!=Input&&!Fresh(S->getValueOperand()))return false;continue;}
    auto *L=dyn_cast<LoadInst>(U);if(!L)return false;
    for(User *LU:L->users()) {
      if(LU==Input)continue;
      auto *C=dyn_cast<CallInst>(LU);if(!C||C!=Ctx||C->getArgOperand(5)!=L)return false;
    }
  }
  bool Initial=false,Back=false;
  for(unsigned I=0;I<2;++I){Value *V=Input->getIncomingValue(I);if(Loop->contains(Input->getIncomingBlock(I))){auto *L=dyn_cast<LoadInst>(V);Back|=L&&L->getPointerOperand()==Slot&&Size->getIncomingValueForBlock(Input->getIncomingBlock(I))==Execute;}else Initial|=Fresh(V);}
  return Initial&&Back;
}
static void lowerRelaxationSimplifications(Module &M) {
  unsigned Requested=executionSimplifyRules()&480;bool Stats=std::getenv("SGPL_EXEC_SIMPLIFY_STATS");SmallVector<CallInst *,8> Binds;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))if(C->getMetadata("sgpl.relax.snapshot.bases"))Binds.push_back(C);
  for(auto *C:Binds) {
    auto *Origins=C->getMetadata("sgpl.relax.snapshot.bases"),*Witness=C->getMetadata("sgpl.exec.frontier");
    auto *V=Witness&&Witness->getNumOperands()==2?dyn_cast<ValueAsMetadata>(Witness->getOperand(0)):nullptr;
    auto *Pair=V?dyn_cast<Function>(V->getValue()):nullptr;auto *Gate=Witness&&Witness->getNumOperands()==2?dyn_cast<ConstantAsMetadata>(Witness->getOperand(1)):nullptr;
    SmallPtrSet<Function *,16> Active;RelaxRecipe Recipe;
    const char *Reason="disabled at compilation";
    if(Requested) {
      if(!Pair||!Gate)Reason="missing destination-ownership witness";
      else if(!Gate->getValue()->isNullValue())Reason="source gating is observable";
      else if(!Pair->hasFnAttribute("sgpl.record.source_ordinal.dead"))Reason="source occurrence ordinal is observable";
      else if(!frontierContextSafe(Pair,1,Active)||!frontierBodySafe(Pair,Active))Reason="callback has unresolved context uses or effects";
      else if(!relaxPrivateContext(C->getArgOperand(0)))Reason="context escapes or is not a single private invocation";
      else if(!relaxCaptureWindow(C))Reason="cannot prove live values unchanged through source snapshot capture";
      else {Recipe=relaxRecipe(Pair,Origins);Reason="callback is not one certified signed i32 minimum with immutable candidate";}
    }
    unsigned Rules=Recipe.Array?Requested:0;
    Value *Extent=nullptr;
    if(Rules) {
      DominatorTree DT(*C->getFunction());
      for(Instruction *I=C->getNextNode();I;I=I->getNextNode())if(auto *Capture=dyn_cast<CallInst>(I))
        if(auto *F=Capture->getCalledFunction())if(F->getName()=="autograph_exec_ctx_bind_source_snapshot_range"||F->getName()=="autograph_exec_ctx_bind_snapshot_range") {
          auto *Slot=dyn_cast<ConstantInt>(Capture->getArgOperand(1));auto *Base=dyn_cast<LoadInst>(Capture->getArgOperand(2));
          auto *Bytes=dyn_cast<ConstantInt>(Capture->getArgOperand(4));
          Value *Count=Capture->getArgOperand(3);auto *Def=dyn_cast<Instruction>(Count);
          if(Capture->getArgOperand(0)==C->getArgOperand(0)&&Slot&&Slot->getZExtValue()==Recipe.SnapshotSlot&&
              Base&&Base->getPointerOperand()==Recipe.Array&&Bytes&&Bytes->equalsInt(4)&&(!Def||DT.dominates(Def,C)))Extent=Count;
        }
      if(!Extent){Rules=0;Reason="snapshot extent is unavailable before preparation";}
    }
    if(Rules&&Recipe.Weighted) {
      auto *Create=cast<CallInst>(C->getArgOperand(0));auto *Graph=dyn_cast<LoadInst>(Create->getArgOperand(0));
      if(!Graph||Graph->getPointerOperand()!=Recipe.WeightGraph){Rules=0;Reason="weight query uses a different or unresolved graph";}
    }
    if((Rules&128)&&!relaxSetConsumer(C))Rules&=~128u;
    if(Recipe.Weighted)Rules&=~256u;
    if(Stats)for(unsigned I=5;I<9;++I)if(Requested&(1u<<I))errs()<<"[exec-simplify] R"<<I+1<<" "<<((Rules&(1u<<I))?"activate":"refuse")<<": "<<
      ((Rules&(1u<<I))?(I==7?"complete set consumer; one minimum/activation per destination":I==8?"exact notification stream under runtime nondecreasing-candidate guard":"strict minimum, immutable candidate, private no-error invocation"):
       (I==8&&Recipe.Weighted?"weighted contributions have no established total order":I==7&&Recipe.Array?"frontier count/order/multiplicity or initial-buffer escape is unresolved":Reason))<<"\n";
    C->setMetadata("sgpl.relax.snapshot.bases",nullptr);if(!Rules)continue;
    IRBuilder<> B(C->getNextNode());auto *I=B.getInt32Ty(),*L=B.getInt64Ty();auto *P=B.getPtrTy();auto *T=StructType::get(M.getContext(),{I,I,I,I,L,L,P});
    if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_relax_simplify_plan_v1))report_fatal_error("relax descriptor mismatch");
    auto *Plan=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,ConstantStruct::get(T,{B.getInt32(2),B.getInt32(sizeof(sgpl_relax_simplify_plan_v2)),B.getInt32(Rules),B.getInt32(Recipe.Weighted),B.getInt64(sizeof(sgpl_exec_ctx)),B.getInt64(sizeof(sgpl_runtime_op)),Pair}),"sgpl.relax.simplification.v2");
    B.CreateCall(M.getOrInsertFunction("sgpl_exec_bind_relax_simplification_v2",FunctionType::get(B.getVoidTy(),{P,P,P,L},false)),{C->getArgOperand(0),Plan,B.CreateLoad(P,Recipe.Array),Extent});
    if(Rules&256)B.CreateCall(M.getOrInsertFunction("sgpl_exec_bind_ordered_source_v1",FunctionType::get(B.getVoidTy(),{P,I},false)),{C->getArgOperand(0),B.getInt32(Recipe.SnapshotSlot)});
    if(verifyModule(M,&errs()))report_fatal_error("invalid minimum-relaxation simplification module");
  }
}
}
