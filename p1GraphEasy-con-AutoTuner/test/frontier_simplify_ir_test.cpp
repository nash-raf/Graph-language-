#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/SourceMgr.h>
#include "autotuner_runtime.h"
#include "fullgraph_lowering.inc"
#include "append_journal_lowering.inc"
#include "execution_simplify_lowering.inc"
#include "frontier_simplify_lowering.inc"
using namespace llvm;
static void require(bool V,const char *Why){if(!V){std::cerr<<"FAIL "<<Why<<"\n";std::exit(1);}}
static std::vector<std::pair<int64_t,int32_t>> appends;
static unsigned polls,cancelAt,reads;
extern "C" int32_t test_append(sgpl_exec_ctx *c,int32_t v){appends.emplace_back(c->array_work_ordinal*2+c->array_append_seq++,v);return 1;}
extern "C" void test_poll(int32_t *error){if(++polls==cancelAt)__atomic_store_n(error,-1,__ATOMIC_RELEASE);}
extern "C" void test_read(){++reads;}
static std::unique_ptr<Module> fixture(LLVMContext &C,unsigned Negative) {
  auto Field=[](size_t N){return std::to_string(N);};
  std::string IR="declare i32 @autograph_frontier_activate(ptr,i32)\ndeclare i32 @autograph_exec_ctx_bind_array_frontier(ptr,ptr,i32,ptr,ptr,i32)\ndeclare void @opaque()\n@tls=internal thread_local global i32 0\n"
    "define internal void @pair(ptr %state,ptr %ctx,i32 %u,i32 %v){\n";
  if(Negative==9)IR+="%skip=icmp eq i32 %v,2\nbr i1 %skip,label %done,label %read\nread:\n";
  IR+="%tablefield=getelementptr i8,ptr %ctx,i64 "+Field(offsetof(sgpl_exec_ctx,spatial_snapshots))+"\n%table=load ptr,ptr %tablefield\n%snapshot=load ptr,ptr %table\n%ui=sext i32 %u to i64\n%sp=getelementptr i32,ptr %snapshot,i64 %ui\n%source=load i32,ptr %sp\n";
  if(Negative==1)IR+="%observe=load i8,ptr %ctx\n";
  if(Negative==2)IR+="call void @opaque()\n";
  if(Negative==3)IR+="%atomic=load atomic i32,ptr %sp acquire,align 4\n";
  if(Negative==4)IR+="%volatile=load volatile i32,ptr %sp\n";
  if(Negative==5)IR+="%t=load i32,ptr @tls\n";
  if(Negative==6)IR+="store i32 42,ptr %sp\n";
  if(Negative==7)IR+="%t2=load ptr,ptr %tablefield\n%s2=load ptr,ptr %t2\nstore i32 42,ptr %s2\n";
  IR+="%dp=getelementptr i32,ptr %state,i32 %v\n%old=load i32,ptr %dp\n%candidate=add i32 %source,%u\n%less=icmp slt i32 %candidate,%old\nbr i1 %less,label %update,label %done\nupdate:\nstore i32 %candidate,ptr %dp\n%a=call i32 @autograph_frontier_activate(ptr %ctx,i32 %v)\n%b=call i32 @autograph_frontier_activate(ptr %ctx,i32 %u)\nbr label %done\ndone:\nret void}\n"
    "define void @main(ptr %ctx){%r=call i32 @autograph_exec_ctx_bind_array_frontier(ptr %ctx,ptr null,i32 0,ptr null,ptr null,i32 2)\nret void}\n";
  SMDiagnostic E;auto M=parseAssemblyString(IR,E,C);if(!M)E.print("frontier-test",errs());require(!!M,"parse fixture");M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
  auto *Pair=M->getFunction("pair");Pair->addFnAttr("sgpl.spatial.callback");Pair->addFnAttr("sgpl.record.source_ordinal.dead");
  for(BasicBlock &BB:*Pair)for(Instruction &I:BB)if(I.getName()=="source")I.setMetadata("sgpl.test.snapshot",MDNode::get(C,{}));
  auto *Bind=cast<CallInst>(&M->getFunction("main")->front().front());
  if(Negative!=8)Bind->setMetadata("sgpl.append.journal.certificate",MDNode::get(C,{ValueAsMetadata::get(Pair),ConstantAsMetadata::get(ConstantPointerNull::get(PointerType::get(C,0)))}));
  lowerAppendJournals(*M);return M;
}
static void structural(Module &M,bool R5) {
  unsigned Bodies=0,Cached=0;
  for(Function &F:M)if(F.getSection()==".sgpl_frontier_simplified") {
    if(F.getName().contains("plain."))++Bodies;
    for(BasicBlock &BB:F)for(Instruction &I:BB) {
      if(auto *Call=dyn_cast<CallBase>(&I)){require(Call->getCalledFunction(),"no indirect pair dispatch");require(Call->getCalledFunction()->getName()!="pair"&&Call->getCalledFunction()->getName()!="pair.append_journal_v3","pair body inlined");}
      if(BB.getName().starts_with("source.cache.miss"))if(auto *L=dyn_cast<LoadInst>(&I))if(!L->getType()->isPointerTy())++Cached;
      require(!isa<AllocaInst>(I),"cache promoted to SSA");
    }
  }
  require(Bodies==8,"four group modes, original and journal");require((Cached>0)==R5,"R5 removes repeated source loads on cache hits");
}
static unsigned scalarBitTests() {
  unsigned Comparisons=0;
  for(bool Double:{false,true})for(bool Cache:{false,true}) {
    LLVMContext C;SMDiagnostic E;std::string T=Double?"double":"float";
    std::string IR="define internal void @bits_pair(ptr %state,ptr %ctx,i32 %u,i32 %v){\n"
      "%tablefield=getelementptr i8,ptr %ctx,i64 "+std::to_string(offsetof(sgpl_exec_ctx,spatial_snapshots))+"\n"
      "%table=load ptr,ptr %tablefield\n%snapshot=load ptr,ptr %table\n%ui=sext i32 %u to i64\n"
      "%sp=getelementptr "+T+",ptr %snapshot,i64 %ui\n%value=load "+T+",ptr %sp\n"
      "%dp=getelementptr "+T+",ptr %state,i32 %v\nstore "+T+" %value,ptr %dp\nret void}\n";
    auto M=parseAssemblyString(IR,E,C);require(!!M,"scalar bits parse");M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
    auto *Pair=M->getFunction("bits_pair");Pair->setLinkage(GlobalValue::ExternalLinkage);
    auto Plain=frontierRecordKernel(*M,Pair,false,Cache),Weighted=frontierRecordKernel(*M,Pair,true,Cache);
    require((Plain.Cached>0)==Cache&&(Weighted.Cached>0)==Cache,"float/double cache activation");
    for(Function &F:*M)if(F.getSection()==".sgpl_frontier_simplified") {
      F.setLinkage(GlobalValue::ExternalLinkage);
      for(BasicBlock &BB:F)if(BB.getName()=="error.load") {
        auto *L=cast<LoadInst>(&BB.front());IRBuilder<> B(L);B.CreateCall(M->getOrInsertFunction("test_poll",FunctionType::get(B.getVoidTy(),{B.getPtrTy()},false)),{L->getPointerOperand()});
      }
      // Neither edge-index nor append-ordinal state is observed by this body.
      for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *S=dyn_cast<StoreInst>(&I)) {
        int64_t Off=0;auto *Base=dyn_cast<LoadInst>(GetPointerBaseWithConstantOffset(S->getPointerOperand(),Off,M->getDataLayout()));
        if(Base&&Base->getType()->isPointerTy()&&Off>0)
          require(Off!=(int64_t)offsetof(sgpl_exec_ctx,array_edge_index)&&Off!=(int64_t)offsetof(sgpl_exec_ctx,array_work_ordinal)&&Off!=(int64_t)offsetof(sgpl_exec_ctx,array_append_seq),"R4 removes unobserved context stores");
      }
    }
    std::string Error;std::unique_ptr<ExecutionEngine> EE(EngineBuilder(std::move(M)).setEngineKind(EngineKind::JIT).setErrorStr(&Error).create());require(!!EE,"scalar bits JIT");
    EE->addGlobalMapping("test_poll",(uint64_t)&test_poll);EE->finalizeObject();
    auto Base=(void(*)(void *,sgpl_exec_ctx *,int32_t,int32_t))EE->getFunctionAddress("bits_pair");
    const int32_t Us[]={0,0,0,2,2,1,1,0,3},Vs[]={2,2,1,0,0,3,2,2,1};
    const uint64_t D[2][4]={{0,UINT64_C(0x8000000000000000),UINT64_C(0x7ff8000000001234),UINT64_C(0x7ff0000000000000)},
      {UINT64_C(0xfff0000000000000),1,UINT64_C(0x7ff0000000000042),UINT64_C(0x3ff0000000000000)}};
    const uint32_t F[2][4]={{0,0x80000000,0x7fc00123,0x7f800000},{0xff800000,1,0x7f800042,0x3f800000}};
    for(bool Weight:{false,true})for(unsigned Data=0;Data<2;++Data)for(unsigned Count:{0u,1u,9u})for(unsigned Cancel=0;Cancel<12;++Cancel) {
      auto Kernel=(sgpl_frontier_simplify_kernel_v1)EE->getFunctionAddress(Weight?Weighted.Wrapper->getName().str():Plain.Wrapper->getName().str());require(!!Kernel,"bits kernel address");
      uint64_t A[4]={},B[4]={};int32_t Records[36]={};int64_t Edges[9]={};
      for(unsigned I=0;I<Count;++I){unsigned At=(Weight?4:3)*I;Records[At]=Us[I];Records[At+1]=Vs[I];Records[At+2]=I;if(Weight)Records[At+3]=I;}
      void *Table[]={const_cast<void *>(Double?static_cast<const void *>(D[Data]):static_cast<const void *>(F[Data]))};
      sgpl_exec_ctx CA{},CB{};CA.spatial_snapshots=(Count&&Cancel!=1)?Table:nullptr;CB=CA;int32_t EA=0,EB=0;
      polls=0;cancelAt=Cancel;bool Cancelled=false;
      for(unsigned I=0;I<Count;++I){test_poll(&EA);if(EA<0){Cancelled=true;break;}Base(A,&CA,Us[I],Vs[I]);}
      unsigned ExpectedPolls=polls;polls=0;
      sgpl_frontier_simplify_args_v1 Args{B,&CB,Records,Edges,0,Count,&EB,nullptr,nullptr,0,0};
      require(Kernel(&Args)==(Cancelled?-1:0)&&EA==EB&&polls==ExpectedPolls,"float/double cancellation boundaries");
      require(!std::memcmp(A,B,Double?32:16),"exact float/double NaN payload, infinity, signed zero and subnormal bits");++Comparisons;
    }
  }
  return Comparisons;
}
int main(int argc,char **argv) {
  InitializeNativeTarget();InitializeNativeTargetAsmPrinter();unsigned Comparisons=0;
  for(const char *Rules:{"none","R4","R4,R5","R5"}) {
    setenv("SGPL_EXEC_SIMPLIFY_RULES",Rules,1);
    for(unsigned Negative=0;Negative<10;++Negative) {
      LLVMContext C;auto M=fixture(C,Negative);lowerFrontierExecutionSimplifications(*M);require(!verifyModule(*M,&errs()),"verify module");
      auto *Plan=M->getNamedGlobal("sgpl.frontier.simplification.v1");bool Expected=(std::strstr(Rules,"R4")!=nullptr)&&(Negative==0||Negative==6||Negative==7||Negative==9);
      require(!!Plan==Expected,"activation and negative premises");if(!Plan)continue;
      unsigned Bits=cast<ConstantInt>(cast<ConstantStruct>(Plan->getInitializer())->getOperand(2))->getZExtValue();bool R5=std::strstr(Rules,"R5")&&(Negative==0||Negative==9);
      require(Bits==(8u|(R5?16u:0u)),"independent R5 refusal");
      if(Negative&&Negative!=9)continue;structural(*M,R5);
      if(argc>1){std::error_code EC;raw_fd_ostream Out(std::string(argv[1])+"/frontier-"+(R5?"R4R5":"R4")+".ll",EC);M->print(Out,nullptr);}
      M->getFunction("main")->eraseFromParent();
      std::vector<std::string> Names;for(Function &F:*M)if(F.getSection()==".sgpl_frontier_simplified"){F.setLinkage(GlobalValue::ExternalLinkage);Names.push_back(F.getName().str());}
      auto *Pair=M->getFunction("pair");Pair->setLinkage(GlobalValue::ExternalLinkage);
      // Instrument the retained baseline and generated loops at identical
      // edge boundaries; count actual snapshot loads without changing effects.
      for(Function &F:*M)if(F.getSection()==".sgpl_frontier_simplified")for(BasicBlock &BB:F)if(BB.getName()=="error.load") {
        auto *L=cast<LoadInst>(&BB.front());IRBuilder<> B(L);B.CreateCall(M->getOrInsertFunction("test_poll",FunctionType::get(B.getVoidTy(),{B.getPtrTy()},false)),{L->getPointerOperand()});
      }
      for(Function &F:*M)if(&F==Pair||F.getSection()==".sgpl_frontier_simplified") {
        SmallVector<LoadInst *,8> Loads;
        for(BasicBlock &BB:F)for(Instruction &I:BB)if(auto *L=dyn_cast<LoadInst>(&I))
          if(L->getMetadata("sgpl.test.snapshot"))Loads.push_back(L);
        for(auto *L:Loads){IRBuilder<> B(L);B.CreateCall(M->getOrInsertFunction("test_read",FunctionType::get(B.getVoidTy(),{},false)));}
      }
      std::string Error;std::unique_ptr<ExecutionEngine> EE(EngineBuilder(std::move(M)).setEngineKind(EngineKind::JIT).setErrorStr(&Error).create());require(!!EE,"create JIT");
      EE->addGlobalMapping("autograph_frontier_activate",(uint64_t)&test_append);EE->addGlobalMapping("sgpl_frontier_journal_activate_v1",(uint64_t)&test_append);EE->addGlobalMapping("test_poll",(uint64_t)&test_poll);EE->addGlobalMapping("test_read",(uint64_t)&test_read);EE->finalizeObject();
      auto Base=(void(*)(void *,sgpl_exec_ctx *,int32_t,int32_t))EE->getFunctionAddress("pair");require(!!Base,"baseline JIT");
      const int32_t Us[]={0,0,0,2,2,1,1,0,3},Vs[]={2,2,1,0,0,3,2,2,1};
      for(bool Weighted:{false,true})for(bool Journal:{false,true})for(unsigned Mode=0;Mode<4;++Mode)for(unsigned Cancel=0;Cancel<12;++Cancel)for(unsigned Count:{0u,1u,9u}) {
        auto Name=std::string(Journal?"pair.append_journal_v3":"pair")+".records."+(Weighted?"weighted":"plain");
        auto Kernel=(sgpl_frontier_simplify_kernel_v1)EE->getFunctionAddress(Name);require(!!Kernel,"generated JIT");
        int32_t A[4]={INT32_MAX,INT32_MAX,INT32_MAX,INT32_MAX},B[4];std::memcpy(B,A,sizeof A);
        int32_t Snapshot[]={9,-3,INT32_MIN,7};void *Table[]={Snapshot};int32_t ErrorA=0,ErrorB=0;
        uint8_t SeenA[4]={},SeenB[4]={};uint32_t TagsA[4]={},TagsB[4]={};int32_t Records[36]={};int64_t Edges[9]={};
        for(unsigned I=0;I<Count;++I){unsigned At=(Weighted?4:3)*I;Records[At]=Us[I];Records[At+1]=Vs[I];Records[At+2]=I;if(Weighted)Records[At+3]=I;Edges[I]=I;}
        sgpl_exec_ctx CA{},CB{};CA.spatial_snapshots=(Count&&Cancel!=1)?Table:nullptr;CA.spatial_snapshot_count=1;CA.array_append_sites=2;CA.execution_error=&ErrorA;CB=CA;CB.execution_error=&ErrorB;
        polls=0;reads=0;cancelAt=Cancel;appends.clear();int64_t Groups=0;bool Cancelled=false;
        for(unsigned I=0;I<Count;++I){test_poll(&ErrorA);if(ErrorA<0){Cancelled=true;break;}CA.array_work_ordinal=I;CA.array_append_seq=0;CA.array_edge_index=Weighted?I:Edges[I];
          unsigned V=Vs[I];if(Mode==1)SeenA[V]=1;else if(Mode==2){if(!SeenA[V]){SeenA[V]=1;++Groups;}}else if(Mode==3){if(TagsA[V]!=4){TagsA[V]=4;SeenA[V]=1;++Groups;}}Base(A,&CA,Us[I],Vs[I]);}
        auto ExpectedAppends=appends;unsigned ExpectedPolls=polls,ExpectedReads=reads;polls=0;reads=0;appends.clear();
        sgpl_frontier_simplify_args_v1 Args{B,&CB,Records,Edges,0,Count,&ErrorB,Mode?SeenB:nullptr,Mode==3?TagsB:nullptr,4,Mode>=2};
        auto Actual=Kernel(&Args);require(Actual==(Cancelled?-1:Groups),"groups and cancellation");require(ErrorA==ErrorB&&polls==ExpectedPolls,"exact acquire boundaries");
        require(R5?reads<=ExpectedReads:reads==ExpectedReads,"snapshot cache removes reads without speculative execution");
        if(R5&&Count==9&&!Cancel)require(reads<ExpectedReads,"R5 actually reduces executed snapshot loads");
        require(!std::memcmp(A,B,sizeof A)&&!std::memcmp(SeenA,SeenB,4)&&!std::memcmp(TagsA,TagsB,sizeof TagsA)&&appends==ExpectedAppends,"arrays, duplicates, group tracking, append order");++Comparisons;
      }
    }
  }
  Comparisons+=scalarBitTests();
  std::cout<<"PASS frontier IR/JIT comparisons="<<Comparisons<<"; R4/R5 activation/refusal and structural checks\n";
}
