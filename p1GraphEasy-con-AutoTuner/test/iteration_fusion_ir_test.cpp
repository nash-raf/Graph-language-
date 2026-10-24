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
  for(unsigned subset=0;subset<32;++subset) {
    auto M=parseIRFile(argv[1],E,C);require(!!M,"parse fusion fixture");
    std::string rules="R1,R2,R3";const unsigned ids[]={13,16,17,18,19};
    for(unsigned i=0;i<5;++i)if(subset&(1u<<i))rules+=",R"+std::to_string(ids[i]);
    setenv("SGPL_EXEC_SIMPLIFY_RULES",rules.c_str(),1);certifySourceCounts(*M);certifyIterationMemory(*M);
    for(Function &F:*M)if(F.hasLocalLinkage()&&!F.isDeclaration())iterationCanonicalize(&F);
    lowerIterationMemory(*M);require(!verifyModule(*M,&errs()),"fusion subset verifies");
    bool expected=(subset&17)==17;
    if(countCalls(*M,"sgpl_try_iteration_fusion_v1")!=expected)errs()<<"failing subset="<<subset<<"\n";
    require(countCalls(*M,"sgpl_try_iteration_fusion_v1")==expected,"R19 requires R13 but composes independently with R16/R17/R18");
    Function *F=M->getFunction("sgpl.iteration.fused_contributions");
    require(!!F==expected,"fused kernel emitted only when justified");
    if(F) {
      unsigned stores=0,adds=0,divs=0;
      for(BasicBlock &BB:*F)for(Instruction &I:BB) {
        require(!isa<CallBase>(I),"singleton map fully inlined");
        if(auto *S=dyn_cast<StoreInst>(&I)){++stores;require(isa<BinaryOperator>(S->getValueOperand()),"contribution arithmetic directly feeds final store");}
        if(auto *L=dyn_cast<LoadInst>(&I))if(!L->getType()->isPointerTy())require(executionArraySlot(L->getPointerOperand())!=M->getGlobalVariable("cur_rank",true),"no old-rank read remains");
        adds+=I.getOpcode()==Instruction::FAdd;divs+=I.getOpcode()==Instruction::FDiv;
      }
      require(stores==1&&adds==1&&divs==1,"one ordered addition, original division and one contribution store");
    }
  }
  setenv("SGPL_EXEC_SIMPLIFY_RULES","all",1);
  for(unsigned negative=0;negative<7;++negative) {
    auto M=parseIRFile(argv[1],E,C);certifyIterationMemory(*M);
    for(Function &F:*M)if(F.hasLocalLinkage()&&!F.isDeclaration())iterationCanonicalize(&F);
    // Certificate removal refuses fusion even though the accepted iteration remains.
    for(GlobalVariable &G:M->globals())if(G.getMetadata("sgpl.iteration.memory.proof")) {
      if(negative==0)G.setMetadata("sgpl.iteration.gate.immutable",nullptr);
      if(negative==1)G.setMetadata("sgpl.iteration.map.gate.disjoint",nullptr);
      auto *P=cast<ConstantStruct>(G.getInitializer());auto *Pair=cast<Function>(P->getOperand(2));
      auto *Source=M->getGlobalVariable("cur_rank",true);auto *Dest=M->getGlobalVariable("next_rank",true);
      for(BasicBlock &BB:*Pair)for(Instruction &I:BB) {
        if(auto *L=dyn_cast<LoadInst>(&I))if(executionArraySlot(L->getPointerOperand())==Source) {
          if(negative==2)L->setVolatile(true);
          if(negative==3){auto *GEP=cast<GetElementPtrInst>(L->getPointerOperand());IRBuilder<> B(GEP);GEP->setOperand(1,B.CreateAdd(GEP->getOperand(1),ConstantInt::get(GEP->getOperand(1)->getType(),1)));}
        }
        if(negative==4)if(auto *S=dyn_cast<StoreInst>(&I))if(executionArraySlot(S->getPointerOperand())==Dest) {
          auto *Add=cast<BinaryOperator>(S->getValueOperand());IRBuilder<> B(Add);
          auto *Read=B.CreateLoad(B.getDoubleTy(),B.CreateGEP(B.getDoubleTy(),B.CreateLoad(B.getPtrTy(),Source),Pair->getArg(3)));
          Add->setOperand(1,B.CreateFAdd(Add->getOperand(1),Read));
        }
      }
      if(negative==5)Pair->addFnAttr(Attribute::StrictFP);
      if(negative==6){IRBuilder<> B(Pair->getEntryBlock().getTerminator());B.CreateCall(M->getOrInsertFunction("opaque_fusion",FunctionType::get(B.getVoidTy(),false)));}
    }
    lowerIterationMemory(*M);require(!verifyModule(*M,&errs()),"fusion refusal verifies");
    require(!countCalls(*M,"sgpl_try_iteration_fusion_v1"),"missing proof refuses fusion");
  }
  for(unsigned subset=0;subset<32;++subset) {
    const uint64_t premises[]={SGPL_FACT_GATE_ENTAILED,SGPL_FACT_MAP_LOCAL,SGPL_FACT_GATE_DISJOINT,SGPL_FACT_KERNEL_DISJOINT,SGPL_FACT_FUSION_SHAPE};uint64_t facts=0;
    for(unsigned i=0;i<5;++i)if(subset&(1u<<i))facts|=premises[i];
    require(!!(sgpl_iteration_fusion(facts,SGPL_EXEC_R19).rules&SGPL_EXEC_R19)==(subset==31),"all fusion premises necessary");
    require(!sgpl_iteration_fusion(facts,0).rules,"disabled rule cannot fire");
    uint64_t candidate=(facts&~SGPL_FACT_GATE_ENTAILED)|((subset&1)?SGPL_FACT_GATE_STABLE:0);
    require(!!sgpl_iteration_fusion_candidate(candidate,SGPL_EXEC_R19).rules==(subset==31),"compiler requires all static candidate premises");
    require(!sgpl_iteration_fusion(candidate,SGPL_EXEC_R19).rules,"static gate stability never supplies runtime entailment");
  }
  std::cout<<"PASS fusion structural subsets=32 negative_premises=7 worklist_subsets=64\n";
  return 0;
}
