/* The frontend supplies the immutable full-domain ownership/alias certificate.
 * This second gate accepts only a transparent, single-block pair body. Generic
 * callbacks are retained verbatim. No FP flags or contribution order change. */
#include <llvm/IR/Verifier.h>
namespace {
using namespace llvm;
struct FullSourceInfo {bool safe=false,source=false;};
static FullSourceInfo fullSource(Value *V,Function *F,DenseMap<Value *,FullSourceInfo> &Memo) {
  auto It=Memo.find(V);if(It!=Memo.end())return It->second;
  if(isa<Constant>(V))return {true,false};
  if(auto *A=dyn_cast<Argument>(V))return {A!=F->getArg(3),A==F->getArg(2)};
  auto *I=dyn_cast<Instruction>(V);if(!I)return {};
  bool Allowed=isa<GetElementPtrInst>(I)||isa<CastInst>(I)||isa<BinaryOperator>(I)||
      isa<UnaryOperator>(I)||isa<CmpInst>(I)||isa<SelectInst>(I);
  if(auto *L=dyn_cast<LoadInst>(I))Allowed=!L->isVolatile()&&!L->isAtomic();
  if(!Allowed)return Memo[V]={};
  FullSourceInfo R{true,false};
  for(Value *O:I->operand_values()){auto S=fullSource(O,F,Memo);R.safe&=S.safe;R.source|=S.source;}
  return Memo[V]=R;
}
static Value *fullClone(Value *V,IRBuilder<> &B,ValueToValueMapTy &VM) {
  if(auto It=VM.find(V);It!=VM.end())return It->second;
  if(isa<Constant>(V))return V;
  auto *I=cast<Instruction>(V);Instruction *C=I->clone();
  for(unsigned k=0;k<C->getNumOperands();++k)C->setOperand(k,fullClone(I->getOperand(k),B,VM));
  B.Insert(C);VM[V]=C;return C;
}
static Value *fullField(IRBuilder<> &B,Value *Ctx,size_t Off,Type *T) {
  return B.CreateLoad(T,B.CreateGEP(B.getInt8Ty(),Ctx,B.getInt64(Off)));
}
static void fullOrderedLoop(BranchInst *Back) {
  LLVMContext &C=Back->getContext();
  auto *V=MDNode::get(C,{MDString::get(C,"llvm.loop.vectorize.enable"),ConstantAsMetadata::get(ConstantInt::getFalse(C))});
  auto *U=MDNode::get(C,{MDString::get(C,"llvm.loop.unroll.disable")});
  auto *M=MDNode::getDistinct(C,{nullptr,V,U});M->replaceOperandWith(0,M);Back->setMetadata(LLVMContext::MD_loop,M);
}
static std::pair<Function *,Function *> fullKernels(Module &M,Function *F,unsigned &Fields) {
  if(F->arg_size()!=4||F->size()!=1||!F->getArg(1)->use_empty())return {};
  for(Instruction &I:F->front()) {
    if(isa<ReturnInst>(I))continue;
    if(auto *L=dyn_cast<LoadInst>(&I)){if(L->isAtomic()||L->isVolatile())return {};continue;}
    if(auto *S=dyn_cast<StoreInst>(&I)){if(S->isAtomic()||S->isVolatile())return {};continue;}
    if(!(isa<GetElementPtrInst>(I)||isa<CastInst>(I)||isa<BinaryOperator>(I)||isa<UnaryOperator>(I)||isa<CmpInst>(I)||isa<SelectInst>(I)))return {};
  }
  DenseMap<Value *,FullSourceInfo> Memo;SmallVector<Value *,8> Hoist;
  for(Instruction &I:F->front())if(I.getType()->isDoubleTy()) {
    auto S=fullSource(&I,F,Memo);if(!S.safe||!S.source)continue;
    bool Boundary=false;
    for(User *U:I.users())if(!fullSource(U,F,Memo).safe)Boundary=true;
    if(Boundary)Hoist.push_back(&I);
  }
  if(Hoist.size()>16)return {};Fields=Hoist.size();
  LLVMContext &C=M.getContext();auto *P=PointerType::get(C,0);auto *I32=Type::getInt32Ty(C);auto *I64=Type::getInt64Ty(C);auto *D=Type::getDoubleTy(C);
  Function *Map=nullptr;
  if(Fields) {
    Map=Function::Create(FunctionType::get(Type::getVoidTy(C),{P,P,I64,P,P,P},false),GlobalValue::InternalLinkage,"sgpl_full_source_map",M);
    Map->addFnAttr(Attribute::NoInline);Map->setSection(".sgpl_full_kernels");
    auto *Entry=BasicBlock::Create(C,"entry",Map),*Loop=BasicBlock::Create(C,"source",Map),*Gate=BasicBlock::Create(C,"gate",Map),*GateLoad=BasicBlock::Create(C,"gate.load",Map),*Calc=BasicBlock::Create(C,"calculate",Map),*Next=BasicBlock::Create(C,"next",Map),*Done=BasicBlock::Create(C,"done",Map);
    IRBuilder<> B(Entry);B.CreateBr(Loop);B.SetInsertPoint(Loop);auto *U=B.CreatePHI(I64,2);U->addIncoming(B.getInt64(0),Entry);
    auto *Degree=BasicBlock::Create(C,"degree",Map);B.CreateCondBr(B.CreateICmpSLT(U,Map->getArg(2)),Degree,Done);
    B.SetInsertPoint(Degree);Value *First=B.CreateLoad(I64,B.CreateGEP(I64,Map->getArg(3),U));Value *Last=B.CreateLoad(I64,B.CreateGEP(I64,Map->getArg(3),B.CreateAdd(U,B.getInt64(1))));
    B.CreateCondBr(B.CreateICmpSGT(Last,First),Gate,Next);
    B.SetInsertPoint(Gate);B.CreateCondBr(B.CreateIsNull(Map->getArg(4)),Calc,GateLoad);
    B.SetInsertPoint(GateLoad);B.CreateCondBr(B.CreateICmpNE(B.CreateLoad(B.getInt8Ty(),B.CreateGEP(B.getInt8Ty(),Map->getArg(4),U)),B.getInt8(0)),Calc,Next);
    B.SetInsertPoint(Calc);ValueToValueMapTy VM;VM[F->getArg(0)]=Map->getArg(0);VM[F->getArg(1)]=Map->getArg(1);VM[F->getArg(2)]=B.CreateTrunc(U,I32);
    for(unsigned k=0;k<Fields;++k){Value *X=fullClone(Hoist[k],B,VM);B.CreateStore(X,B.CreateGEP(D,Map->getArg(5),B.CreateAdd(B.CreateMul(U,B.getInt64(Fields)),B.getInt64(k))));}
    B.CreateBr(Next);B.SetInsertPoint(Next);auto *UN=B.CreateAdd(U,B.getInt64(1));B.CreateBr(Loop);U->addIncoming(UN,Next);B.SetInsertPoint(Done);B.CreateRetVoid();
  }
  auto *K=Function::Create(FunctionType::get(Type::getVoidTy(C),{P,P,I32,I32,P,P,P},false),GlobalValue::InternalLinkage,"sgpl_full_incoming",M);
  K->addFnAttr(Attribute::NoInline);K->setSection(".sgpl_full_kernels");
  auto *Entry=BasicBlock::Create(C,"entry",K),*Outer=BasicBlock::Create(C,"destination",K),*Row=BasicBlock::Create(C,"row",K),*Inner=BasicBlock::Create(C,"incoming",K),*Visit=BasicBlock::Create(C,"visit",K),*Gate=BasicBlock::Create(C,"gate",K),*GateLoad=BasicBlock::Create(C,"gate.load",K),*Effect=BasicBlock::Create(C,"effect",K),*Mark=BasicBlock::Create(C,"mark",K),*Body=BasicBlock::Create(C,"body",K),*Next=BasicBlock::Create(C,"next.edge",K),*NextV=BasicBlock::Create(C,"next.destination",K),*Done=BasicBlock::Create(C,"done",K),*Err=BasicBlock::Create(C,"error",K);
  IRBuilder<> B(Entry);Value *GatePtr=fullField(B,K->getArg(1),offsetof(sgpl_exec_ctx,spatial_source_gate),P);Value *Seen=fullField(B,K->getArg(1),offsetof(sgpl_exec_ctx,spatial_groups_seen),P);Value *Error=fullField(B,K->getArg(1),offsetof(sgpl_exec_ctx,execution_error),P);B.CreateBr(Outer);
  B.SetInsertPoint(Outer);auto *V=B.CreatePHI(I32,2);V->addIncoming(K->getArg(2),Entry);B.CreateCondBr(B.CreateICmpSLT(V,K->getArg(3)),Row,Done);
  B.SetInsertPoint(Row);Value *V64=B.CreateSExt(V,I64);Value *First=B.CreateLoad(I64,B.CreateGEP(I64,K->getArg(4),V64));Value *Last=B.CreateLoad(I64,B.CreateGEP(I64,K->getArg(4),B.CreateAdd(V64,B.getInt64(1))));B.CreateBr(Inner);
  B.SetInsertPoint(Inner);auto *J=B.CreatePHI(I64,2);J->addIncoming(First,Row);B.CreateCondBr(B.CreateICmpSLT(J,Last),Visit,NextV);
  B.SetInsertPoint(Visit);Value *U=B.CreateLoad(I32,B.CreateGEP(I32,K->getArg(5),J));B.CreateCondBr(B.CreateIsNull(Error),Gate,Err);
  B.SetInsertPoint(Err);auto *Status=B.CreateLoad(I32,Error);Status->setAtomic(AtomicOrdering::Acquire);Status->setAlignment(Align(4));B.CreateCondBr(B.CreateICmpSLT(Status,B.getInt32(0)),Done,Gate);
  B.SetInsertPoint(Gate);B.CreateCondBr(B.CreateIsNull(GatePtr),Effect,GateLoad);
  B.SetInsertPoint(GateLoad);B.CreateCondBr(B.CreateICmpNE(B.CreateLoad(B.getInt8Ty(),B.CreateGEP(B.getInt8Ty(),GatePtr,B.CreateSExt(U,I64))),B.getInt8(0)),Effect,Next);
  B.SetInsertPoint(Effect);B.CreateCondBr(B.CreateIsNull(Seen),Body,Mark);
  B.SetInsertPoint(Mark);B.CreateStore(B.getInt8(1),B.CreateGEP(B.getInt8Ty(),Seen,V64));B.CreateBr(Body);
  B.SetInsertPoint(Body);ValueToValueMapTy VM;VM[F->getArg(0)]=K->getArg(0);VM[F->getArg(1)]=K->getArg(1);VM[F->getArg(2)]=U;VM[F->getArg(3)]=V;
  for(unsigned k=0;k<Fields;++k)VM[Hoist[k]]=B.CreateLoad(D,B.CreateGEP(D,K->getArg(6),B.CreateAdd(B.CreateMul(B.CreateSExt(U,I64),B.getInt64(Fields)),B.getInt64(k))));
  for(Instruction &I:F->front()) {
    if(isa<ReturnInst>(I)||VM.count(&I))continue;
    Instruction *Clone=I.clone();for(unsigned k=0;k<Clone->getNumOperands();++k){Value *O=I.getOperand(k);if(auto It=VM.find(O);It!=VM.end())Clone->setOperand(k,It->second);}
    B.Insert(Clone);VM[&I]=Clone;
  }
  B.CreateBr(Next);B.SetInsertPoint(Next);auto *JN=B.CreateAdd(J,B.getInt64(1));auto *Back=B.CreateBr(Inner);fullOrderedLoop(Back);J->addIncoming(JN,Next);
  B.SetInsertPoint(NextV);Value *VN=B.CreateAdd(V,B.getInt32(1));B.CreateBr(Outer);V->addIncoming(VN,NextV);B.SetInsertPoint(Done);B.CreateRetVoid();
  /* Remove dead cloned source calculations after replacing the maximal terms.
   * Optimize only the new kernels; leave generic IR and its rounding intact. */
  PassBuilder PB;LoopAnalysisManager LAM;FunctionAnalysisManager FAM;CGSCCAnalysisManager CGAM;ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);PB.registerCGSCCAnalyses(CGAM);PB.registerFunctionAnalyses(FAM);PB.registerLoopAnalyses(LAM);PB.crossRegisterProxies(LAM,FAM,CGAM,MAM);
  FunctionPassManager FPM;cantFail(PB.parsePassPipeline(FPM,"dce,simplifycfg,dce"));FPM.run(*K,FAM);if(Map)FPM.run(*Map,FAM);
  if(verifyFunction(*K,&errs())||(Map&&verifyFunction(*Map,&errs())))report_fatal_error("invalid certified full-graph kernel");
  return {K,Map};
}
static void lowerFullGraph(Module &M) {
  SmallVector<CallInst *,8> Markers;
  for(Function &F:M)for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *C=dyn_cast<CallInst>(&I))if(C->getCalledFunction()&&C->getCalledFunction()->getName()=="sgpl_fullgraph_certificate")Markers.push_back(C);
  unsigned Enabled=0,Refused=0;
  for(auto *C:Markers) {
    auto *F=dyn_cast<Function>(C->getArgOperand(1)->stripPointerCasts());unsigned Fields=0;std::pair<Function *,Function *> K{};
    if(F&&!std::getenv("SGPL_NO_FULLGRAPH_LOWERING"))K=fullKernels(M,F,Fields);
    if(K.first) {
      IRBuilder<> B(C);auto Bind=M.getOrInsertFunction("autograph_exec_ctx_bind_fullgraph_v1",FunctionType::get(B.getVoidTy(),{B.getPtrTy(),B.getPtrTy(),B.getPtrTy(),B.getInt64Ty(),B.getInt32Ty(),B.getInt64Ty(),B.getInt64Ty()},false));
      B.CreateCall(Bind,{C->getArgOperand(0),K.first,K.second?static_cast<Value *>(K.second):ConstantPointerNull::get(B.getPtrTy()),B.getInt64(Fields*8),B.getInt32(SGPL_EXECUTION_ABI_VERSION),B.getInt64(sizeof(sgpl_exec_ctx)),B.getInt64(sizeof(sgpl_runtime_op))});++Enabled;
      if(std::getenv("SGPL_FULLGRAPH_STATS"))errs()<<"[fullgraph] activate "<<F->getName()<<" source_fields="<<Fields<<" ordered incoming traversal\n";
    }else {++Refused;if(std::getenv("SGPL_FULLGRAPH_STATS"))errs()<<"[fullgraph] refuse certified callback: disabled, opaque/context-observing or unsupported control flow\n";}
    C->eraseFromParent();
  }
  if(auto *F=M.getFunction("sgpl_fullgraph_certificate"))if(F->use_empty())F->eraseFromParent();
  if(std::getenv("SGPL_FULLGRAPH_STATS"))errs()<<"[fullgraph] activated="<<Enabled<<" refused="<<Refused<<"\n";
}
}
