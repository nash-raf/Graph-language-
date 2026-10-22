/* Late bounded-append specialization. The frontend supplies destination
 * ownership/alias/lifecycle facts; this pass additionally proves the complete
 * context use chain and an acyclic append body. */
#include "append_journal_plan.h"
#include "execution_fusion_context.inc"
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Transforms/Utils/Cloning.h>
namespace {
static int journalContainsAppend(llvm::Function *F,llvm::SmallPtrSetImpl<llvm::Function *> &Active) {
  using namespace llvm;
  if(!F||F->isDeclaration())return 0;
  if(!Active.insert(F).second)return -1;
  int Result=0;
  for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallBase>(&I))if(auto *C=Call->getCalledFunction()) {
    if(C->getName()=="autograph_frontier_activate")Result=1;
    else if(C->hasLocalLinkage()) {
      int Child=journalContainsAppend(C,Active);if(Child<0){Active.erase(F);return -1;}Result|=Child;
    }
  }
  Active.erase(F);return Result;
}
static llvm::Function *journalClone(llvm::Function *F,unsigned Sites,bool GeneratedOwner) {
  using namespace llvm;
  ValueToValueMapTy VM;auto *K=CloneFunction(F,VM);K->setName(F->getName()+".append_journal_v3");
  for(unsigned Attempt=0;;++Attempt) {
    CallInst *Inline=nullptr;bool Refused=false;
    for(BasicBlock &BB:*K)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallBase>(&I))
      if(auto *C=Call->getCalledFunction())if(C->hasLocalLinkage()&&C!=K) {
        SmallPtrSet<Function *,16> Active;int Contains=journalContainsAppend(C,Active);
        if(Contains<0||(Contains&&!isa<CallInst>(Call)))Refused=true;
        else if(Contains&&!Inline)Inline=cast<CallInst>(Call);
      }
    if(Refused||Attempt>256){K->eraseFromParent();return nullptr;}
    if(!Inline)break;
    InlineFunctionInfo IFI;if(!InlineFunction(*Inline,IFI).isSuccess()){K->eraseFromParent();return nullptr;}
  }
  DominatorTree DT(*K);LoopInfo LI(DT);SmallVector<CallInst *,8> Appends;
  for(BasicBlock &BB:*K)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallBase>(&I))
    if(auto *C=Call->getCalledFunction())if(C->getName()=="autograph_frontier_activate") {
      if(!isa<CallInst>(Call)||Call->arg_size()!=2||Call->getArgOperand(0)->stripPointerCasts()!=K->getArg(1)||
          Call->getNumOperandBundles()||(!GeneratedOwner&&LI.getLoopFor(&BB))) {
        K->eraseFromParent();return nullptr;
      }
      Appends.push_back(cast<CallInst>(Call));
    }
  if(Appends.empty()||Appends.size()>Sites){K->eraseFromParent();return nullptr;}
  auto &M=*K->getParent();IRBuilder<> B(M.getContext());
  auto Append=M.getOrInsertFunction("sgpl_frontier_journal_activate_v1",FunctionType::get(B.getInt32Ty(),{B.getPtrTy(),B.getInt32Ty()},false));
  for(auto *Call:Appends)Call->setCalledFunction(Append);
  K->addFnAttr("sgpl.effect.append_journal","v3");K->setSection(".sgpl_journal_kernels");
  if(GeneratedOwner)K->removeFnAttr("sgpl.effect.frontier");
  if(verifyFunction(*K,&errs()))report_fatal_error("invalid bounded append clone");return K;
}
static void lowerAppendJournals(llvm::Module &M) {
  using namespace llvm;
  SmallVector<CallInst *,8> Marks;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))
    if(C->getMetadata("sgpl.append.journal.certificate"))Marks.push_back(C);
  for(CallInst *C:Marks) {
    auto *MD=C->getMetadata("sgpl.append.journal.certificate");auto *Fn=C->getCalledFunction();
    auto GetFn=[&](unsigned I)->Function * {
      auto *V=MD&&I<MD->getNumOperands()?dyn_cast<ValueAsMetadata>(MD->getOperand(I)):nullptr;
      return V?dyn_cast<Function>(V->getValue()->stripPointerCasts()):nullptr;
    };
    Function *Pair=GetFn(0),*Gate=GetFn(1),*Journal=nullptr,*Dense=nullptr,*Sparse=nullptr;
    auto *Sites=C->arg_size()==6?dyn_cast<ConstantInt>(C->getArgOperand(5)):nullptr;
    C->setMetadata("sgpl.append.journal.certificate",nullptr);
    SmallPtrSet<Function *,16> Active;
    const char *Reason="bounded private ordinal journals; certified producer and ordered bitmap merge";
    auto Require=[&](bool Condition,const char *Why){if(!Condition)Reason=Why;return Condition;};
    bool Eligible=Require(Fn&&Fn->getName()=="autograph_exec_ctx_bind_array_frontier"&&MD&&MD->getNumOperands()==2&&
      Pair&&Pair->arg_size()==4,"missing frontend ownership/lifecycle certificate")&&
      Require(Pair->hasFnAttribute("sgpl.spatial.callback"),"destination dependence certificate missing")&&
      Require(Sites&&Sites->getZExtValue()>0&&Sites->getZExtValue()<=INT32_MAX,"append bound missing or unsupported")&&
      Require(frontierContextSafe(Pair,1,Active),"context/storage observer or unknown context use")&&
      Require(frontierBodySafe(Pair,Active),"opaque, recursive, atomic, volatile or unsupported operation effects")&&
      Require(frontierGateSafe(Gate),"source lifecycle or gate observers")&&
      Require(!std::getenv("SGPL_NO_EFFECT_FUSION_LOWERING")&&!std::getenv("SGPL_NO_FUSION_C_LOWERING"),"lowering disabled");
    if(Eligible){Journal=journalClone(Pair,(unsigned)Sites->getZExtValue(),false);if(!Journal)Reason="append chain is cyclic, observed or exceeds certified bound";}
    if(Journal) {
      SmallVector<Function *,4> Owners;
      for(Function &F:M)if(F.hasFnAttribute("sgpl.effect.frontier")&&!F.hasFnAttribute("sgpl.effect.append_journal"))
        if(auto *Tag=F.getMetadata("sgpl.frontier.pair"))if(Tag->getNumOperands()==1)
          if(auto *V=dyn_cast<ValueAsMetadata>(Tag->getOperand(0)))if(V->getValue()==Pair)Owners.push_back(&F);
      bool OwnerRefusal=false;
      unsigned DenseCount=0,SparseCount=0;
      for(Function *F:Owners){DenseCount+=F->getName().starts_with("sgpl_frontier_owner_dense");SparseCount+=F->getName().starts_with("sgpl_frontier_owner_sparse");}
      if(!Owners.empty()&&(Owners.size()!=2||DenseCount!=1||SparseCount!=1)){OwnerRefusal=true;Owners.clear();}
      for(Function *F:Owners) {
        Function *Clone=journalClone(F,(unsigned)Sites->getZExtValue(),true);
        if(!Clone){OwnerRefusal=true;break;}
        if(F->getName().starts_with("sgpl_frontier_owner_sparse"))Sparse=Clone;
        else if(F->getName().starts_with("sgpl_frontier_owner_dense"))Dense=Clone;
        else{Clone->eraseFromParent();OwnerRefusal=true;break;}
      }
      if(OwnerRefusal||(!Owners.empty()&&(!Dense||!Sparse))) {
        if(Dense)Dense->eraseFromParent();if(Sparse)Sparse->eraseFromParent();Dense=Sparse=nullptr;
        /* Generic records may still use journals. Active direct owners refuse
         * at runtime unless both certified inlined variants exist. */
      }
      IRBuilder<> B(C->getNextNode());auto *I=B.getInt32Ty();auto *P=B.getPtrTy();
      auto *T=StructType::get(M.getContext(),{I,I,I,I,P,P,P,P,P});
      if(M.getDataLayout().getTypeAllocSize(T)!=sizeof(sgpl_append_journal_plan_v1))report_fatal_error("journal descriptor layout mismatch");
      auto Ptr=[&](Function *F)->Constant *{return F?static_cast<Constant *>(F):ConstantPointerNull::get(P);};
      auto *Plan=new GlobalVariable(M,T,true,GlobalValue::PrivateLinkage,
        ConstantStruct::get(T,{B.getInt32(SGPL_APPEND_JOURNAL_VERSION),B.getInt32(sizeof(sgpl_append_journal_plan_v1)),
          B.getInt32(5|SGPL_JOURNAL_PROVEN_APPEND),B.getInt32(Sites->getZExtValue()),Pair,Journal,Ptr(Gate),Ptr(Dense),Ptr(Sparse)}),"sgpl.append.journal.plan.v3");
      auto Bind=M.getOrInsertFunction("sgpl_exec_bind_append_journal_v1",FunctionType::get(B.getVoidTy(),{P,P},false));
      B.CreateCall(Bind,{C->getArgOperand(0),Plan});
      M.getOrInsertNamedMetadata("sgpl.effect.fusion.proofs")->addOperand(MDNode::get(M.getContext(),{
        MDString::get(M.getContext(),"bounded acyclic append; complete context use chain; native record induction gives valid owner and increasing work keys; per-record sequence reset and certified append bound rechecked before effects imply private capacity; no context or output-log observer; private bitmap before effects; duplicate values preserved in increasing ordinal order after join; no effect replay"),ValueAsMetadata::get(Journal)}));
    }
    if(std::getenv("SGPL_FUSION_STATS"))errs()<<"[append-journal] "<<(Journal?"activate":"refuse")<<": "<<Reason<<"; protocol="<<SGPL_APPEND_JOURNAL_VERSION<<"; runtime capacity and join checks remain required\n";
  }
}
}
