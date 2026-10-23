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
  for(auto &G:M->globals())if(G.getMetadata("sgpl.iteration.memory.proof")) {
    auto *S=cast<ConstantStruct>(G.getInitializer());require(!S->getOperand(6)->isNullValue()&&!S->getOperand(7)->isNullValue(),"seeded and source range kernels installed");
    auto *F=cast<Function>(S->getOperand(6));DominatorTree DT(*F);LoopInfo LI(DT);
    unsigned Stores=0,Loads=0;
    for(BasicBlock &BB:*F)for(Instruction &I:BB) {
      if(auto *Load=dyn_cast<LoadInst>(&I))Loads+=executionArraySlot(Load->getPointerOperand())==S->getOperand(5);
      if(auto *Store=dyn_cast<StoreInst>(&I))if(executionArraySlot(Store->getPointerOperand())==S->getOperand(5)) {
        ++Stores;require(!LI.getLoopFor(&BB)||!LI.getLoopFor(&BB)->getParentLoop(),"one store outside incoming loop");
      }
    }
    require(Stores==1&&Loads==0,"seed fusion removes destination loads and repeated stores");
  }
  std::cout<<"PASS iteration memory structural certificate and kernels\n";
}
