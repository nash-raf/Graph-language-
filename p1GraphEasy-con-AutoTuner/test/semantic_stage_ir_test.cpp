#define main relaxation_fixture_main
#include "relax_simplify_ir_test.cpp"
#undef main
#include "semantic_stage_lowering.inc"
static unsigned mask(Module &M) {
  unsigned N=0;for(auto &G:M.globals())if(G.getName().starts_with("sgpl.semantic.stage.v1"))N=cast<ConstantInt>(cast<ConstantStruct>(G.getInitializer())->getOperand(2))->getZExtValue();return N;
}
int main(int argc,char **argv) {
  unsigned Checks=0;
  {
    LLVMContext C;Module M("initialization-memory-control",C);IRBuilder<> B(C);
    auto *F=Function::Create(FunctionType::get(B.getVoidTy(),{B.getPtrTy(),B.getPtrTy()},false),GlobalValue::InternalLinkage,"publish",M);
    B.SetInsertPoint(BasicBlock::Create(C,"entry",F));B.CreateMemCpy(F->getArg(0),MaybeAlign(8),F->getArg(1),MaybeAlign(8),8);B.CreateRetVoid();
    SmallPtrSet<Function *,16> Seen;require(!semanticInitClosure(F,Seen),"unknown bulk memory publication refuses initialization summary");++Checks;
  }
  for(unsigned Negative=0;Negative<6;++Negative) {
    LLVMContext C;auto M=fixture(C,24);auto *Pair=M->getFunction("pair");CallInst *Bind=nullptr,*Weight=nullptr;
    for(BasicBlock &BB:*M->getFunction("main"))for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getName()=="bind")Bind=Call;
    StoreInst *Store=nullptr;LoadInst *Old=nullptr;
    for(BasicBlock &BB:*Pair)for(Instruction &I:BB) {
      if(auto *S=dyn_cast<StoreInst>(&I))Store=S;
      if(auto *L=dyn_cast<LoadInst>(&I))if(L->getName()=="old")Old=L;
      if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getCalledFunction()->getName()=="autograph_exec_ctx_edge_weight")Weight=Call;
    }
    require(Store&&Old&&Bind&&Weight,"general weighted fixture landmarks");
    IRBuilder<> B(Store);Store->setOperand(0,B.CreateSub(Old,Store->getValueOperand()));
    if(Negative==1)Weight->setArgOperand(2,Pair->getArg(3));
    if(Negative==2)Weight->setMetadata("sgpl.relax.weight.accessor",nullptr);
    if(Negative==3)M->getGlobalVariable("graph",true)->setLinkage(GlobalValue::ExternalLinkage);
    if(Negative==4)for(BasicBlock &BB:*Pair)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))if(L->getName()=="nativeweight")L->setVolatile(true);
    if(Negative==5)for(BasicBlock &BB:*Pair)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))if(L->getName()=="nativeweight")L->setMetadata("sgpl.relax.weight.accessor",nullptr);
    auto R=semanticRecipe(Pair,Bind->getMetadata("sgpl.relax.snapshot.bases"));
    require(!!R.Pair==(Negative==0),"general weight provenance requires exact args, compiler leaf, private graph and ordinary proven reads");
    if(R.Pair){require(R.Weighted&&!R.Minimum&&!R.Join,"nonfoldable weighted code earns no minimum/set/fixed-point license");R.Pair->eraseFromParent();}
    ++Checks;
  }
  for(unsigned Extra=0;Extra<2;++Extra)for(unsigned Prep=0;Prep<4;++Prep)for(unsigned Subset=0;Subset<8;++Subset)for(unsigned Negative:{0,1,2,3,4,5,6,7,8,9,11,12,13,14,16,17,18,19,20,22,23,24,25,26,28,30,31,32,33,34,35,36,37,38,39,40,43,44,45,46,47,48,49,50,51}) {
    if(Extra&&(Prep||Subset!=1||(Negative!=0&&Negative!=30&&Negative!=31&&Negative!=32&&Negative!=49)))continue;
    if(Prep&&Negative!=0&&Negative!=30&&Negative!=35&&Negative!=37&&Negative!=38&&Negative!=39&&Negative!=50)continue;
    LLVMContext C;auto M=fixture(C,Negative>=30?0:Negative);
    auto *F=M->getFunction("main");auto *Pair=M->getFunction("pair");
    auto *Capacity=new GlobalVariable(*M,Type::getInt64Ty(C),false,GlobalValue::InternalLinkage,ConstantInt::get(Type::getInt64Ty(C),4),"cap");
    CallInst *Bind=nullptr,*Execute=nullptr;LoadInst *Graph=nullptr;
    for(BasicBlock &BB:*F)for(Instruction &I:BB) {
      if(auto *Load=dyn_cast<LoadInst>(&I))if(Load->getPointerOperand()==M->getGlobalVariable("graph",true))Graph=Load;
      if(auto *Call=dyn_cast<CallInst>(&I)) {
        if(Call->getName()=="bind")Bind=Call;
        if(Call->getName()=="result")Execute=Call;
      }
    }
    require(Bind&&Graph&&Execute,"fixture landmarks");
    Graph->moveBefore(F->getEntryBlock().getTerminator()->getIterator());Bind->setArgOperand(4,Capacity);
    {
      auto *Create=cast<CallInst>(Bind->getArgOperand(0));IRBuilder<> B(Create);auto *P=B.getPtrTy();auto *I=B.getInt32Ty();auto *L=B.getInt64Ty();
      auto *Slots=B.CreateAlloca(P);auto *Null=ConstantPointerNull::get(P);
      auto Op=M->getOrInsertFunction("autograph_exec_op_create",FunctionType::get(P,{L,P,P,P,P,P,P,P,I},false));
      auto *O=B.CreateCall(Op,{B.getInt64(SGPL_OP_PAIR|SGPL_OP_NO_SOURCE_ORDINAL),Null,Pair,Null,Null,Null,Null,Null,B.getInt32(0)});B.CreateStore(O,Slots);
      Create->setArgOperand(10,Slots);Create->setArgOperand(11,B.getInt32(1));
    }
    unsigned Expected=Subset<<9;
    if(Negative&&Negative!=5&&Negative!=6&&Negative!=7&&Negative!=8&&Negative!=16&&Negative!=17&&Negative!=20&&Negative<30)Expected=0;
    if(Negative==5||Negative==6||Negative==7||Negative==8||Negative==16||Negative==17||Negative==20)Expected&=~2048u;
    if(Negative==16||Negative==17)Expected=0;
    if(Negative==24)Expected=(Subset<<9)&~2048u;
    // Generic arithmetic is legal without any fold/reorder/fixed-point license.
    if(Negative==30) {
      StoreInst *S=nullptr;LoadInst *Old=nullptr,*Source=nullptr;
      for(BasicBlock &BB:*Pair)for(Instruction &I:BB){if(auto *X=dyn_cast<StoreInst>(&I))S=X;if(auto *X=dyn_cast<LoadInst>(&I)){if(X->getName()=="old")Old=X;if(X->getName()=="source")Source=X;}}
      IRBuilder<> B(S);S->setOperand(0,B.CreateSub(Old,Source));Expected&=~2048u;
    }
    if(Negative==31) {IRBuilder<> B(&*Execute->getParent()->getFirstInsertionPt());B.CreateStore(B.getInt32(1),M->getGlobalVariable("side",true));Expected=0;}
    if(Negative==32) {IRBuilder<> B(F->back().getTerminator());B.CreateLoad(B.getPtrTy(),M->getGlobalVariable("out",true));Expected&=~2048u;}
    if(Negative==33) {auto *Alloc=dyn_cast<CallInst>(M->getGlobalVariable("array",true)->user_back());(void)Alloc;Bind->setArgOperand(5,ConstantInt::get(Type::getInt32Ty(C),2));Expected=0;}
    if(Negative==34) {Graph->moveBefore(cast<CallInst>(Bind->getArgOperand(0))->getIterator());IRBuilder<> B(F->getEntryBlock().getTerminator());B.CreateCall(M->getFunction("opaque"),{M->getGlobalVariable("graph",true)});Expected=0;}
    if(Negative==35) {
      for(BasicBlock &BB:*Pair)for(Instruction &I:BB)if(auto *Cmp=dyn_cast<ICmpInst>(&I))if(Cmp->getName()=="less")Cmp->setPredicate(ICmpInst::ICMP_SGT);
    }
    if(Negative==36) {
      IRBuilder<> B(F->back().getTerminator());B.CreateCall(M->getFunction("observe"),{Execute});Expected=0;
    }
    if(Negative>=37&&Negative<=40) {
      StoreInst *S=nullptr;ICmpInst *Cmp=nullptr;LoadInst *Source=nullptr,*Old=nullptr;
      for(BasicBlock &BB:*Pair)for(Instruction &I:BB){if(auto *X=dyn_cast<StoreInst>(&I))S=X;if(auto *X=dyn_cast<ICmpInst>(&I))if(X->getName()=="less")Cmp=X;if(auto *X=dyn_cast<LoadInst>(&I)){if(X->getName()=="source")Source=X;if(X->getName()=="old")Old=X;}}
      IRBuilder<> B(Cmp);Value *V=Negative==37?B.CreateOr(Old,Source):Negative==38?B.CreateAnd(Old,Source):Negative==39?B.CreateAdd(Old,Source):B.CreateShl(Old,Source);
      Cmp->setPredicate(ICmpInst::ICMP_NE);Cmp->setOperand(0,V);Cmp->setOperand(1,Old);S->setOperand(0,V);
      if(Negative==39)Expected&=~2048u;if(Negative==40)Expected=0;
    }
    if(Negative==43) {for(BasicBlock &BB:*Pair)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getCalledFunction()->getName()=="autograph_frontier_activate")Call->setArgOperand(1,Pair->getArg(2));Expected=0;}
    if(Negative==44){cast<CallInst>(Bind->getArgOperand(0))->setArgOperand(6,ConstantInt::get(Type::getInt32Ty(C),1));Expected=0;}
    if(Negative==45||Negative==46){for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getCalledFunction()->getName()=="autograph_exec_op_create")Call->setArgOperand(Negative==45?2:3,M->getFunction("opaque"));Expected=0;}
    if(Negative==47||Negative==48){for(BasicBlock &BB:*F)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getName()=="snap")Call->setArgOperand(Negative==47?1:4,ConstantInt::get(Negative==47?Type::getInt32Ty(C):Type::getInt64Ty(C),Negative==47?1:8));Expected=0;}
    if(Negative==49){CallInst *A=nullptr;for(Instruction &I:F->getEntryBlock())if(auto *Call=dyn_cast<CallInst>(&I))if(Call->getName()=="values")A=Call;IRBuilder<> B(F->getEntryBlock().getTerminator());B.CreateCall(M->getFunction("opaque"),{A});Expected&=~2048u;}
    if(Negative==50) {
      LoadInst *Source=nullptr;ICmpInst *Cmp=nullptr;
      for(BasicBlock &BB:*Pair)for(Instruction &I:BB){if(auto *X=dyn_cast<LoadInst>(&I))if(X->getName()=="source")Source=X;if(auto *X=dyn_cast<ICmpInst>(&I))if(X->getName()=="less")Cmp=X;}
      auto *Before=Cmp->getParent();auto *Inner=Before->splitBasicBlock(Cmp,"conditional.minimum");Before->getTerminator()->eraseFromParent();IRBuilder<> B(Before);
      BasicBlock *Done=nullptr;for(BasicBlock &BB:*Pair)if(BB.getTerminator()&&isa<ReturnInst>(BB.getTerminator()))Done=&BB;
      B.CreateCondBr(B.CreateICmpSLT(Source,B.getInt32(0)),Inner,Done);Expected&=~2048u;
    }
    if(Negative==51){IRBuilder<> B(cast<CallInst>(Bind->getArgOperand(0)));auto *Saved=B.CreateLoad(B.getInt32Ty(),M->getGlobalVariable("side",true));B.SetInsertPoint(Execute->getParent()->getTerminator());B.CreateStore(Saved,M->getGlobalVariable("side",true));Expected=0;}
    std::string Rules;
    for(unsigned I=0;I<3;++I)if(Subset&(1u<<I)){if(!Rules.empty())Rules+=",";Rules+="R"+std::to_string(I+10);}
    for(unsigned I=0;I<2;++I)if(Prep&(1u<<I)){if(!Rules.empty())Rules+=",";Rules+="R"+std::to_string(I+6);}
    if(Negative==0&&(Expected&1536u))Expected|=Prep<<5;
    if(Extra){Rules+=",R8,R14,R15";if(Negative==0)Expected|=128|SGPL_EXEC_R14;}
    setenv("SGPL_EXEC_SIMPLIFY_RULES",Rules.empty()?"none":Rules.c_str(),1);
    require(!verifyModule(*M,&errs()),"adapted fixture verifies");lowerSemanticStages(*M);
    require(!verifyModule(*M,&errs()),"semantic transformation verifies");
    unsigned Actual=mask(*M);
    if(Actual!=Expected){errs()<<"subset="<<Subset<<" negative="<<Negative<<" expected="<<Expected<<" actual="<<Actual<<"\n";require(false,"separate rule applicability");}
    require(M->getFunction("pair")&&!M->getFunction("pair")->empty(),"baseline callback retained");
    if(Actual)require(M->getNamedMetadata("sgpl.execution.compositions")&&M->getNamedMetadata("sgpl.execution.compositions")->getNumOperands()==1,"one proof-carrying fixed-point plan, one lowering");
    if(Actual&SGPL_EXEC_R14) {
      unsigned Calls=0,Resets=0;for(Function &Fn:*M)for(BasicBlock &BB:Fn)for(Instruction &I:BB)if(auto *Call=dyn_cast<CallInst>(&I))if(auto *Callee=Call->getCalledFunction()) {
        Calls+=Callee->getName()=="sgpl_try_semantic_stage_facts_v1";Resets+=Callee->getName()=="sgpl_semantic_frontier_reset_v1";
      }
      require(Calls==1&&Resets==1,"private frontier induction has one scoped reset and one composed dispatch");
    }
    if(Actual&1024) {
      Function *Row=nullptr;for(Function &Fn:*M)if(Fn.getName().starts_with("sgpl.semantic.ordered_row"))Row=&Fn;
      require(Row&&!Row->empty(),"ordered row realization exists");
      unsigned Stores=0;for(BasicBlock &BB:*Row)for(Instruction &I:BB)if(auto *S=dyn_cast<StoreInst>(&I))if(BB.getName().starts_with("flush"))++Stores;
      require(Stores==1,"one dirty-gated destination flush outside steady contribution loop");
    }
    if((Actual&2048)&&(Negative==35||Negative==37||Negative==38))for(auto &G:M->globals())if(G.getName().starts_with("sgpl.semantic.stage.v1"))require(cast<ConstantInt>(cast<ConstantStruct>(G.getInitializer())->getOperand(3))->getZExtValue()==(Negative==35?SGPL_JOIN_SMAX:Negative==37?SGPL_JOIN_OR:SGPL_JOIN_AND),"operation-specific fixed-point license");
    if(argc>1&&Subset==7){std::error_code E;raw_fd_ostream O(std::string(argv[1])+"/semantic_fixture_"+std::to_string(Negative)+".ll",E);M->print(O,nullptr);}
    ++Checks;
  }
  std::cout<<"PASS semantic IR checks="<<Checks<<" subsets=8\n";
}
