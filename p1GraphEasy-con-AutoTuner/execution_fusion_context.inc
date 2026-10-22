#ifndef SGPL_EXECUTION_FUSION_CONTEXT_INC
#define SGPL_EXECUTION_FUSION_CONTEXT_INC
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/IntrinsicInst.h>
namespace {
/* A representation-changing executor must prove more than a dead source
 * ordinal. Only the established snapshot/weight/append accessors may expose
 * context storage; unknown users and raw record/log observers refuse. */
static bool frontierContextSafe(llvm::Function *F,unsigned Arg,
    llvm::SmallPtrSetImpl<llvm::Function *> &Active) {
  using namespace llvm;
  if(!F||F->isDeclaration()||Arg>=F->arg_size()||!Active.insert(F).second)return false;
  auto Finish=[&](bool x){Active.erase(F);return x;};
  auto &DL=F->getParent()->getDataLayout();SmallVector<Value *,16> Pending{F->getArg(Arg)};
  SmallPtrSet<Value *,32> Seen;
  while(!Pending.empty()) {
    Value *V=Pending.pop_back_val();if(!Seen.insert(V).second)continue;
    for(User *U:V->users()) {
      if(isa<GetElementPtrInst>(U)||isa<BitCastInst>(U)) {
        int64_t Off=0;if(GetPointerBaseWithConstantOffset(cast<Value>(U),Off,DL)!=F->getArg(Arg))return Finish(false);
        Pending.push_back(cast<Value>(U));continue;
      }
      if(auto *L=dyn_cast<LoadInst>(U)) {
        int64_t Off=0;auto Size=DL.getTypeStoreSize(L->getType());
        if(L->getPointerOperand()!=V||L->isAtomic()||L->isVolatile()||Size.isScalable()||
            GetPointerBaseWithConstantOffset(V,Off,DL)!=F->getArg(Arg)||Off<0)return Finish(false);
        auto Exact=[&](size_t At,size_t N){return (uint64_t)Off==At&&Size.getFixedValue()==N;};
        bool Snapshot=Exact(offsetof(sgpl_exec_ctx,spatial_snapshots),sizeof(void *))||
            Exact(offsetof(sgpl_exec_ctx,spatial_snapshot_count),sizeof(int32_t));
        auto *Tag=L->getMetadata("sgpl.ctx.accessor");
        auto *Name=Tag&&Tag->getNumOperands()==1?dyn_cast<MDString>(Tag->getOperand(0)):nullptr;
        bool Weight=Name&&Name->getString()=="autograph_exec_ctx_edge_weight"&&
            (Exact(offsetof(sgpl_exec_ctx,graph),sizeof(void *))||
             Exact(offsetof(sgpl_exec_ctx,array_record_edges),sizeof(void *))||
             Exact(offsetof(sgpl_exec_ctx,array_compact_records),sizeof(int32_t))||
             Exact(offsetof(sgpl_exec_ctx,array_edge_index),sizeof(int64_t)));
        if(!Snapshot&&!Weight)return Finish(false);continue;
      }
      if(isa<ICmpInst>(U))continue;
      auto *Call=dyn_cast<CallBase>(U);auto *Callee=Call?Call->getCalledFunction():nullptr;
      if(!Callee||Call->getNumOperandBundles())return Finish(false);
      bool Matched=false;
      for(unsigned I=0;I<Call->arg_size();++I)if(Call->getArgOperand(I)==V) {
        Matched=true;int64_t Off=0;
        if(GetPointerBaseWithConstantOffset(V,Off,DL)!=F->getArg(Arg)||Off)return Finish(false);
        auto N=Callee->getName();
        if(I==0&&(N=="autograph_frontier_activate"||N=="autograph_exec_ctx_snapshot"||N=="autograph_exec_ctx_edge_weight"))continue;
        if(!Callee->hasLocalLinkage()||!frontierContextSafe(Callee,I,Active))return Finish(false);
      }
      if(!Matched||Call->getCalledOperand()==V)return Finish(false);
    }
  }
  return Finish(true);
}
static bool frontierGateSafe(llvm::Function *F) {
  using namespace llvm;
  if(!F)return true;
  if(F->isDeclaration()||F->arg_size()!=3)return false;
  for(User *U:F->getArg(1)->users()) {
    auto *Call=dyn_cast<CallInst>(U);
    if(!Call||!Call->getCalledFunction()||Call->getCalledFunction()->getName()!="autograph_exec_ctx_set_source_gate"||
        Call->arg_size()!=3||Call->getArgOperand(0)!=F->getArg(1)||Call->getArgOperand(1)!=F->getArg(2)||
        Call->getNumOperandBundles())return false;
  }
  /* These callbacks remain in the original ordered source-coverage stage.
   * The frontend certificate excludes user source lifecycle and claims. */
  return true;
}
static bool frontierBodySafe(llvm::Function *F,llvm::SmallPtrSetImpl<llvm::Function *> &Active) {
  using namespace llvm;
  if(!F||F->isDeclaration()||!Active.insert(F).second)return false;
  auto Finish=[&](bool Value){Active.erase(F);return Value;};
  for(BasicBlock &BB:*F)for(Instruction &I:BB) {
    for(Value *V:I.operand_values())if(V->getType()->isPointerTy())
      if(auto *G=dyn_cast<GlobalVariable>(getUnderlyingObject(V)))if(G->isThreadLocal())return Finish(false);
    if(auto *L=dyn_cast<LoadInst>(&I))if(L->isVolatile()||L->isAtomic())return Finish(false);
    if(auto *S=dyn_cast<StoreInst>(&I))if(S->isVolatile()||S->isAtomic())return Finish(false);
    if(isa<AtomicRMWInst>(I)||isa<AtomicCmpXchgInst>(I)||isa<FenceInst>(I))return Finish(false);
    if(auto *Call=dyn_cast<CallBase>(&I)) {
      auto *C=Call->getCalledFunction();if(!C||Call->getNumOperandBundles())return Finish(false);
      auto N=C->getName();
      if(N=="autograph_frontier_activate"||N=="autograph_exec_ctx_snapshot"||N=="autograph_exec_ctx_edge_weight"||N=="graph_get_edge_weight")continue;
      if(C->isIntrinsic()) {
        auto ID=C->getIntrinsicID();
        /* An unbundled assume is retained verbatim in the cloned body. Its
         * inaccessible-memory attribute is not an observable user effect.
         * Bundled assumptions have already been refused above. */
        if(C->doesNotAccessMemory()||ID==Intrinsic::assume||ID==Intrinsic::lifetime_start||ID==Intrinsic::lifetime_end||isa<DbgInfoIntrinsic>(Call))continue;
        return Finish(false);
      }
      if(C->hasLocalLinkage()){if(!frontierBodySafe(C,Active))return Finish(false);}
      else if(!C->doesNotAccessMemory()||!C->doesNotThrow()||!C->hasFnAttribute(Attribute::WillReturn))return Finish(false);
    }
  }
  return Finish(true);
}
}
#endif
