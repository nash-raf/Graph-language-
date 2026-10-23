#define main previous_execution_ir_main
#include "execution_simplify_ir_test.cpp"
#undef main
#include <llvm/IRReader/IRReader.h>
#include "iteration_memory_lowering.inc"
int main(int argc,char **argv) {
  require(argc==2,"provide scalar compiler fixture");
  LLVMContext C;SMDiagnostic E;auto M=parseIRFile(argv[1],E,C);require(!!M,"parse scalar fixture");
  setenv("SGPL_EXEC_SIMPLIFY_STATS","1",1);setenv("SGPL_EXEC_SIMPLIFY_RULES","all",1);
  certifyIterationMemory(*M);
  unsigned Plans=0;for(auto &G:M->globals())Plans+=!!G.getMetadata("sgpl.iteration.memory.proof");
  require(Plans==1,"one complete iteration certificate activates");
  // The normal pipeline canonicalizes the callback before late fullgraph
  // lowering. Apply only the same non-arithmetic cleanup for this fixture.
  for(Function &F:*M)if(F.hasLocalLinkage()&&!F.isDeclaration())iterationCanonicalize(&F);
  lowerIterationMemory(*M);require(!verifyModule(*M,&errs()),"generated seeded/range kernels verify");
  unsigned Ungated=0;
  for(Function &F:*M)if(F.getName().starts_with("sgpl.iteration.ungated_")) {
    ++Ungated;
    for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I)) {
      require(!L->getType()->isIntegerTy(8),"entailed gate clones have no bitmap element reads");
      require(executionPointerKind(L,&F,false)!=0,"entailed kernel has no context gate pointer load");
    }
  }
  require(Ungated==2,"immutable gate yields both ungated clones");
  for(auto &G:M->globals())if(G.getMetadata("sgpl.iteration.memory.proof")) {
    auto *S=cast<ConstantStruct>(G.getInitializer());require(!S->getOperand(6)->isNullValue()&&!S->getOperand(7)->isNullValue(),"seeded and source range kernels installed");
    auto *F=cast<Function>(S->getOperand(6));DominatorTree DT(*F);LoopInfo LI(DT);
    unsigned Stores=0,Loads=0;
    for(BasicBlock &BB:*F)for(Instruction &I:BB) {
      if(auto *Cmp=dyn_cast<ICmpInst>(&I))if(isa<ConstantPointerNull>(Cmp->getOperand(1)))require(executionPointerKind(Cmp->getOperand(0),F,false)<0,"private kernel retains R1 null-test elimination");
      if(auto *Load=dyn_cast<LoadInst>(&I))Loads+=executionArraySlot(Load->getPointerOperand())==S->getOperand(5);
      if(auto *Store=dyn_cast<StoreInst>(&I))if(executionArraySlot(Store->getPointerOperand())==S->getOperand(5)) {
        ++Stores;require(!LI.getLoopFor(&BB)||!LI.getLoopFor(&BB)->getParentLoop(),"one store outside incoming loop");
      }
    }
    require(Stores==1&&Loads==0,"seed fusion removes destination loads and repeated stores");
  }
  for(unsigned Negative=0;Negative<5;++Negative) {
    auto N=parseIRFile(argv[1],E,C);require(!!N,"reload scalar negative fixture");
    CallInst *Cert=nullptr;
    for(Function &F:*N)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getCalledFunction()&&Call->getCalledFunction()->getName()=="sgpl_fullgraph_certificate")Cert=Call;
    require(Cert,"negative certificate landmark");auto *Pair=cast<Function>(Cert->getArgOperand(1));
    auto *Source=N->getGlobalVariable("cur_rank",true),*Dest=N->getGlobalVariable("next_rank",true);
    require(Source&&Dest,"fixture allocation identities");
    IRBuilder<> B(Cert);
    if(Negative==0) {
      auto *Compare=new ICmpInst(ICmpInst::ICMP_ULT,B.CreateLoad(B.getPtrTy(),Source),B.CreateLoad(B.getPtrTy(),Dest));B.Insert(Compare);
    }
    if(Negative==1)B.CreateCall(N->getOrInsertFunction("unknown_publish",FunctionType::get(B.getVoidTy(),{B.getPtrTy()},false)),{B.CreateLoad(B.getPtrTy(),Source)});
    if(Negative==2)Pair->addFnAttr(Attribute::StrictFP);
    if(Negative==3)B.CreateStore(B.CreateLoad(B.getPtrTy(),Source),Dest);
    if(Negative==4) {
      B.SetInsertPoint(Pair->getEntryBlock().getTerminator());B.CreateStore(ConstantFP::get(B.getDoubleTy(),42.0),B.CreateLoad(B.getPtrTy(),Dest));
    }
    certifyIterationMemory(*N);unsigned Count=0;for(auto &G:N->globals())Count+=!!G.getMetadata("sgpl.iteration.memory.proof");
    require(Count==0,"observable identity/escape/strict FP/alias/extra store refuses iteration license");
  }
  for(unsigned Negative=0;Negative<4;++Negative) {
    auto N=parseIRFile(argv[1],E,C);require(!!N,"reload gate proof negative");
    Function *Gate=nullptr;GlobalVariable *Degree=N->getGlobalVariable("out_degree",true);
    for(Function &F:*N)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getCalledFunction()&&Call->getCalledFunction()->getName()=="autograph_exec_ctx_set_source_gate")Gate=&F;
    require(Gate&&Degree,"gate and allocation provenance landmarks");
    if(Negative==0) {
      // Same callback shape, but the predicate now reads the rotating source.
      for(BasicBlock &BB:*Gate)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))if(L->getPointerOperand()==Degree)L->setOperand(0,N->getGlobalVariable("cur_rank",true));
    }
    if(Negative==1)for(BasicBlock &BB:*Gate)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))if(L->getType()->isIntegerTy(32))L->setVolatile(true);
    if(Negative==2)setenv("SGPL_EXEC_SIMPLIFY_RULES","R1,R2,R3",1);
    if(Negative==3)setenv("SGPL_EXEC_SIMPLIFY_RULES","none",1);
    certifyIterationMemory(*N);
    for(auto &G:N->globals())require(!G.getMetadata("sgpl.iteration.gate.immutable"),"mutable/volatile/disabled conditions never assert gate immutability");
    setenv("SGPL_EXEC_SIMPLIFY_RULES","all",1);
  }
  std::cout<<"PASS iteration memory structural certificate and kernels\n";
}
