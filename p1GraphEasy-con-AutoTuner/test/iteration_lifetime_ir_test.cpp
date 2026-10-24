#define main previous_execution_ir_main
#include "execution_simplify_ir_test.cpp"
#undef main
#include <llvm/IRReader/IRReader.h>
#include "iteration_memory_lowering.inc"
static unsigned countCalls(Module &M,StringRef Name) {
  unsigned N=0;for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)
    if(auto *C=dyn_cast<CallInst>(&I))N+=C->getCalledFunction()&&C->getCalledFunction()->getName()==Name;
  return N;
}
int main(int argc,char **argv) {
  require(argc==2,"scalar fixture required");LLVMContext C;SMDiagnostic E;
  setenv("SGPL_EXEC_SIMPLIFY_STATS","1",1);
  for(unsigned Subset=0;Subset<8;++Subset) {
    auto M=parseIRFile(argv[1],E,C);require(!!M,"parse scalar fixture");
    std::string Rules="R1,R2,R3,R13";
    for(unsigned I=0;I<3;++I)if(Subset&(1u<<I))Rules+=",R"+std::to_string(I+16);
    setenv("SGPL_EXEC_SIMPLIFY_RULES",Rules.c_str(),1);
    certifySourceCounts(*M);require(countCalls(*M,"sgpl_try_source_count_v1")==!!(Subset&1),"R16 independent subset activation");
    certifyIterationMemory(*M);
    for(Function &F:*M)if(F.hasLocalLinkage()&&!F.isDeclaration())iterationCanonicalize(&F);
    lowerIterationMemory(*M);require(!verifyModule(*M,&errs()),"subset module verifies");
    unsigned Seen=0;
    for(auto &G:M->globals())if(G.getName().starts_with("sgpl.iteration.storage.v1")) {
      ++Seen;auto *S=cast<ConstantStruct>(G.getInitializer());auto Bits=cast<ConstantInt>(S->getOperand(2))->getZExtValue();
      unsigned Expected=(Subset&2?SGPL_EXEC_R17:0)|(Subset&4?SGPL_EXEC_R18:0);
      require(Bits==Expected,"storage descriptor has exactly justified subset");
      require(cast<ConstantInt>(S->getOperand(3))->getZExtValue()==(Subset&2?0u:3u),"timed fixture has no observed final arrays");
    }
    require(Seen==!!(Subset&6),"independent storage subset activation");
  }
  setenv("SGPL_EXEC_SIMPLIFY_RULES","all",1);
  for(unsigned Negative=0;Negative<10;++Negative) {
    auto M=parseIRFile(argv[1],E,C);auto *Pair=M->getFunction("sgpl_pair_work");require(Pair,"count pair landmark");
    StoreInst *Write=nullptr;LoadInst *Read=nullptr;
    for(BasicBlock &BB:*Pair)for(Instruction &I:BB)if(auto *S=dyn_cast<StoreInst>(&I))if(!isa<AllocaInst>(S->getPointerOperand()))Write=S;
    require(Write,"count update landmark");auto *Add=cast<BinaryOperator>(Write->getValueOperand());Read=cast<LoadInst>(Add->getOperand(0));
    IRBuilder<> B(Write);
    if(Negative==0)Add->setOperand(1,B.getInt32(2));
    if(Negative==1)cast<GetElementPtrInst>(Write->getPointerOperand())->setOperand(1,Pair->getArg(3));
    if(Negative==2)Write->setVolatile(true);
    if(Negative==3)Read->setAtomic(AtomicOrdering::Monotonic);
    if(Negative==4)B.CreateCall(M->getOrInsertFunction("opaque_count",FunctionType::get(B.getVoidTy(),false)));
    if(Negative==5)B.CreateStore(B.getInt32(0),B.CreateGEP(B.getInt32Ty(),cast<GetElementPtrInst>(Write->getPointerOperand())->getPointerOperand(),B.getInt32(0)));
    if(Negative==6)Add->setHasNoSignedWrap(true);
    if(Negative>=7)for(Function &F:*M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))
      if(Call->getCalledFunction()&&Call->getCalledFunction()->getName()=="autograph_exec_ctx_create"&&Call->getArgOperand(12)==B.getInt32(1)) {
        if(Negative==7)Call->setArgOperand(12,B.getInt32(0));
        if(Negative==8){IRBuilder<> X(Call->getNextNode());X.CreateCall(M->getOrInsertFunction("observe_context",FunctionType::get(X.getVoidTy(),{X.getPtrTy()},false)),{Call});}
        if(Negative==9){IRBuilder<> X(Call);X.CreateCall(M->getOrInsertFunction("publish_counts",FunctionType::get(X.getVoidTy(),{X.getPtrTy()},false)),{X.CreateLoad(X.getPtrTy(),M->getGlobalVariable("out_degree",true))});}
      }
    certifySourceCounts(*M);require(!countCalls(*M,"sgpl_try_source_count_v1"),"count premise violation refuses");
  }
  for(unsigned Negative=0;Negative<5;++Negative) {
    auto M=parseIRFile(argv[1],E,C);CallInst *Cert=nullptr;
    for(Function &F:*M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getCalledFunction()&&Call->getCalledFunction()->getName()=="sgpl_fullgraph_certificate")Cert=Call;
    auto *Pair=cast<Function>(Cert->getArgOperand(1));auto *Source=M->getGlobalVariable("cur_rank",true),*Dest=M->getGlobalVariable("next_rank",true);
    DominatorTree DT(*Cert->getFunction());LoopInfo LI(DT);auto *Outer=LI.getLoopFor(Cert->getParent());require(Outer,"iteration landmark");
    if(Negative<2) {
      // A direct observer and a pointer loaded before the loop both keep data live.
      StoreInst *Allocation=nullptr;for(User *U:Dest->users())if(auto *S=dyn_cast<StoreInst>(U))if(S->getPointerOperand()==Dest)Allocation=S;
      require(Allocation,"allocation dominates post-loop observer");IRBuilder<> Before(Allocation->getNextNode());
      Value *Ptr=Negative==1?Before.CreateLoad(Before.getPtrTy(),Dest):nullptr;
      IRBuilder<> After(&*Outer->getExitBlock()->getFirstInsertionPt());if(!Ptr)Ptr=After.CreateLoad(After.getPtrTy(),Dest);
      auto *Value=After.CreateLoad(After.getDoubleTy(),After.CreateGEP(After.getDoubleTy(),Ptr,After.getInt32(0)));
      After.CreateCall(M->getOrInsertFunction("observe_final",FunctionType::get(After.getVoidTy(),{After.getDoubleTy()},false)),{Value});
      auto *SourceValue=After.CreateLoad(After.getDoubleTy(),After.CreateLoad(After.getPtrTy(),Source));
      After.CreateCall(M->getOrInsertFunction("observe_final",FunctionType::get(After.getVoidTy(),{After.getDoubleTy()},false)),{SourceValue});
    } else if(Negative==2) {
      for(BasicBlock &BB:*Pair)for(Instruction &I:BB)if(auto *G=dyn_cast<GetElementPtrInst>(&I))if(executionArraySlot(G)==Source) {
        IRBuilder<> B(G);G->setOperand(1,B.CreateAdd(G->getOperand(1),ConstantInt::get(G->getOperand(1)->getType(),1)));break;
      }
    } else if(Negative==3) {
      for(Function &F:*M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))
        if(F.getName().contains("source_gate")&&L->getPointerOperand()==M->getGlobalVariable("out_degree",true))L->setOperand(0,Source);
    } else {
      setenv("SGPL_EXEC_SIMPLIFY_RULES","none",1);
    }
    certifyIterationMemory(*M);
    for(Function &F:*M)if(F.hasLocalLinkage()&&!F.isDeclaration())iterationCanonicalize(&F);
    lowerIterationMemory(*M);require(!verifyModule(*M,&errs()),"negative verifies");
    for(auto &G:M->globals())if(G.getName().starts_with("sgpl.iteration.storage.v1")) {
      auto Bits=cast<ConstantInt>(cast<ConstantStruct>(G.getInitializer())->getOperand(2))->getZExtValue();
      if(Negative<2)require(!(Bits&SGPL_EXEC_R17),"post-loop observations retain restoration");
      if(Negative==2||Negative==3)require(!(Bits&SGPL_EXEC_R18),"nonlocal reads or mutable gates retain separate scratch");
    }
    if(Negative==4)require(!countCalls(*M,"sgpl_try_iteration_storage_v1"),"none disables additions");
    setenv("SGPL_EXEC_SIMPLIFY_RULES","all",1);
  }
  std::cout<<"PASS iteration lifetime structural subsets=8 count_negatives=10 storage_negatives=5\n";
}
