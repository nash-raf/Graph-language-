#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include "autotuner_runtime.h"
#include "fullgraph_lowering.inc"
#include "execution_simplify_lowering.inc"
using namespace llvm;
static void require(bool V,const char *Why){if(!V){std::cerr<<"FAIL "<<Why<<"\n";std::exit(1);}}
static void structuralChecks(Module &M,unsigned Mask) {
  for(Function &F:M)if(F.getSection()==".sgpl_simplified_kernels") {
    DominatorTree DT(F);LoopInfo LI(DT);
    for(BasicBlock &BB:F) {
      Loop *L=LI.getLoopFor(&BB);if(!L)continue;
      bool Steady=L->getHeader()->getName().contains("steady");
      for(Instruction &I:BB) {
        if((Mask&1))if(auto *Cmp=dyn_cast<ICmpInst>(&I))if(isa<ConstantPointerNull>(Cmp->getOperand(1))) {
          bool Map=F.arg_size()==6;
          require(executionPointerKind(Cmp->getOperand(0),&F,Map)<0,"R1 removes invariant loop null tests");
        }
        if((Mask&2)&&Steady)if(auto *S=dyn_cast<StoreInst>(&I)) {
          auto *Base=getUnderlyingObject(S->getPointerOperand());require(executionPointerKind(Base,&F,false)!=1,"R2 removes steady tracking store");
        }
        if((Mask&4)&&Steady) {
          if(auto *Load=dyn_cast<LoadInst>(&I))if(!Load->getType()->isPointerTy())require(executionArraySlot(Load->getPointerOperand())!=M.getNamedGlobal("y"),"R3 removes steady accumulator load");
          if(auto *Store=dyn_cast<StoreInst>(&I))require(executionArraySlot(Store->getPointerOperand())!=M.getNamedGlobal("y"),"R3 removes steady accumulator store");
        }
      }
    }
  }
}
static unsigned pollCount,pollAt;
static std::vector<std::pair<int32_t,int32_t>> trace;
extern "C" void simplify_test_trace(int32_t u,int32_t v){trace.emplace_back(u,v);}
extern "C" void simplify_test_poll(int32_t *error) {
  if(++pollCount==pollAt)__atomic_store_n(error,-1,__ATOMIC_RELEASE);
}
static void negativeTests() {
  struct Test{const char *name,*source,*update,*extra;unsigned mask;bool baseline;};
  Test Tests[]={
    {"source_alias","@y","fadd double %b,%a","",3,true},
    {"observed_old","@x","fadd double %b,%b","",3,true},
    {"second_store","@x","fadd double %b,%a","store double %a,ptr %sp\n",3,true},
    {"unsupported_update","@x","fmul double %b,%a","",3,true},
    {"fast_math","@x","fadd fast double %b,%a","",3,true},
    {"atomic","@x","fadd double %b,%a","%atomic=load atomic i32,ptr @flag acquire,align 4\n",0,false},
    {"volatile","@x","fadd double %b,%a","%volatile=load volatile i32,ptr @flag\n",0,false},
    {"opaque","@x","fadd double %b,%a","call void @opaque()\n",0,false},
    {"context","@x","fadd double %b,%a","%ctxread=load i8,ptr %ctx\n",0,false},
    {"tls","@x","fadd double %b,%a","%tlsread=load i32,ptr @tls\n",0,true},
    {"unknown_source_allocation","@unknown","fadd double %b,%a","",3,true},
  };
  for(auto &Test:Tests) {
    LLVMContext C;SMDiagnostic Error;
    std::string IR="@x=internal global ptr null,!sgpl.language.array !0\n@y=internal global ptr null,!sgpl.language.array !0\n@unknown=internal global ptr null\n@flag=internal global i32 0\n@tls=internal thread_local global i32 0\ndeclare ptr @calloc(i64,i64)\ndeclare void @opaque()\n"
      "define void @setup(){%a=call ptr @calloc(i64 8,i64 8),!sgpl.language.array !0\nstore ptr %a,ptr @x\n%b=call ptr @calloc(i64 8,i64 8),!sgpl.language.array !0\nstore ptr %b,ptr @y\nret void}\n"
      "define internal void @pair(ptr %state,ptr %ctx,i32 %u,i32 %v){%xp=load ptr,ptr "+std::string(Test.source)+"\n%yp=load ptr,ptr @y\n%ui=sext i32 %u to i64\n%vi=sext i32 %v to i64\n%sp=getelementptr double,ptr %xp,i64 %ui\n%dp=getelementptr double,ptr %yp,i64 %vi\n%a=load double,ptr %sp\n%b=load double,ptr %dp\n%c="+Test.update+"\nstore double %c,ptr %dp\n"+Test.extra+"ret void}\n!0=!{}\n";
    auto M=parseAssemblyString(IR,Error,C);require(!!M,"negative parse");M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");unsigned Fields=0;
    auto K=fullKernels(*M,M->getFunction("pair"),Fields);require(!!K.first==Test.baseline,"baseline refusal");
    if(!K.first){std::cout<<"PASS refusal "<<Test.name<<"\n";continue;}
    auto *Driver=Function::Create(FunctionType::get(Type::getVoidTy(C),{PointerType::get(C,0)},false),GlobalValue::ExternalLinkage,"driver",*M);IRBuilder<> B(BasicBlock::Create(C,"entry",Driver));auto *P=B.getPtrTy();
    B.CreateCall(M->getOrInsertFunction("autograph_exec_ctx_bind_fullgraph_v1",FunctionType::get(B.getVoidTy(),{P,P,P,B.getInt64Ty(),B.getInt32Ty(),B.getInt64Ty(),B.getInt64Ty()},false)),{Driver->getArg(0),K.first,K.second?static_cast<Value *>(K.second):ConstantPointerNull::get(P),B.getInt64(Fields*8),B.getInt32(1),B.getInt64(sizeof(sgpl_exec_ctx)),B.getInt64(sizeof(sgpl_runtime_op))});B.CreateRetVoid();
    setenv("SGPL_EXEC_SIMPLIFY_RULES","all",1);lowerExecutionSimplifications(*M);
    auto *Plan=M->getNamedGlobal("sgpl.execution.simplification.v1");unsigned Mask=Plan?cast<ConstantInt>(cast<ConstantStruct>(Plan->getInitializer())->getOperand(2))->getZExtValue():0;
    require(Mask==Test.mask,Test.name);std::cout<<"PASS refusal "<<Test.name<<" rules="<<Mask<<"\n";
    if(Plan){Plan->replaceAllUsesWith(ConstantPointerNull::get(cast<PointerType>(Plan->getType())));Plan->eraseFromParent();}
    K.first->setMetadata("sgpl.exec.fullgraph",nullptr);lowerExecutionSimplifications(*M);require(!M->getNamedGlobal("sgpl.execution.simplification.v1"),"missing certificate refuses");
  }
}
int main(int argc,char **argv) {
  InitializeNativeTarget();InitializeNativeTargetAsmPrinter();
  for(unsigned Mask=0;Mask<8;++Mask)for(bool integer:{false,true}) {
    LLVMContext C;SMDiagnostic Error;
    std::string T=integer?"i32":"double",Op=integer?"add":"fadd";
    std::string IR="@x = internal global ptr null, !sgpl.language.array !0\n@y = internal global ptr null, !sgpl.language.array !0\n"
      "declare ptr @calloc(i64,i64)\n"
      "define void @setup(){ %a=call ptr @calloc(i64 8,i64 8), !sgpl.language.array !0\nstore ptr %a,ptr @x\n%b=call ptr @calloc(i64 8,i64 8), !sgpl.language.array !0\nstore ptr %b,ptr @y\nret void }\n"
      "define internal void @pair(ptr %state,ptr %ctx,i32 %u,i32 %v){\n"
      "%xp=load ptr,ptr @x\n%yp=load ptr,ptr @y\n%ui=sext i32 %u to i64\n%vi=sext i32 %v to i64\n"
      "%sp=getelementptr "+T+",ptr %xp,i64 %ui\n%dp=getelementptr "+T+",ptr %yp,i64 %vi\n"
      "%a=load "+T+",ptr %sp\n%b=load "+T+",ptr %dp\n%c="+Op+" "+T+" %b,%a\nstore "+T+" %c,ptr %dp\nret void }\n!0=!{}\n";
    auto M=parseAssemblyString(IR,Error,C);if(!M){Error.print("execution-test",errs());return 1;}
    M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
    auto *Pair=M->getFunction("pair");unsigned Fields=0;auto Kernels=fullKernels(*M,Pair,Fields);require(Kernels.first,"baseline generation");
    auto *Driver=Function::Create(FunctionType::get(Type::getVoidTy(C),{PointerType::get(C,0)},false),GlobalValue::ExternalLinkage,"driver",*M);
    IRBuilder<> B(BasicBlock::Create(C,"entry",Driver));auto *P=B.getPtrTy();
    B.CreateCall(M->getOrInsertFunction("autograph_exec_ctx_bind_fullgraph_v1",FunctionType::get(B.getVoidTy(),{P,P,P,B.getInt64Ty(),B.getInt32Ty(),B.getInt64Ty(),B.getInt64Ty()},false)),{Driver->getArg(0),Kernels.first,Kernels.second?static_cast<Value *>(Kernels.second):ConstantPointerNull::get(P),B.getInt64(Fields*8),B.getInt32(SGPL_EXECUTION_ABI_VERSION),B.getInt64(sizeof(sgpl_exec_ctx)),B.getInt64(sizeof(sgpl_runtime_op))});B.CreateRetVoid();
    /* Deterministic external cancellation before an edge, including after a
     * previous contribution. It is test instrumentation, outside pair effects. */
    auto Roles=executionRoles(Kernels.first);IRBuilder<> Poll(&*Roles["error"]->getFirstInsertionPt());
    auto *Status=cast<LoadInst>(&Roles["error"]->front());
    Poll.CreateCall(M->getOrInsertFunction("simplify_test_poll",FunctionType::get(Poll.getVoidTy(),{P},false)),{Status->getPointerOperand()});
    IRBuilder<> Trace(&*Roles["body"]->getFirstInsertionPt());
    auto *Source=cast<LoadInst>(&*Roles["visit"]->getFirstInsertionPt()->getNextNode());
    auto *Destination=cast<PHINode>(&Roles["destination"]->front());
    Trace.CreateCall(M->getOrInsertFunction("simplify_test_trace",FunctionType::get(Trace.getVoidTy(),{Trace.getInt32Ty(),Trace.getInt32Ty()},false)),{Source,Destination});
    std::string Rules;for(unsigned I=0;I<3;++I)if(Mask&(1u<<I)){if(!Rules.empty())Rules+=",";Rules+="R"+std::to_string(I+1);}if(!Mask)Rules="none";
    setenv("SGPL_EXEC_SIMPLIFY_RULES",Rules.c_str(),1);setenv("SGPL_EXEC_SIMPLIFY_STATS","1",1);lowerExecutionSimplifications(*M);
    require(!verifyModule(*M,&errs()),"module verification");
    structuralChecks(*M,Mask);
    require((M->getFunction("autograph_exec_ctx_bind_simplified_fullgraph_v1")!=nullptr)==(Mask!=0),"activation");
    if(argc>1){std::error_code EC;raw_fd_ostream Out(std::string(argv[1])+"/mask"+std::to_string(Mask)+(integer?"_int.ll":"_double.ll"),EC);M->print(Out,nullptr);}
    Function *Optimized=Kernels.first;
    if(Mask){auto *Plan=M->getNamedGlobal("sgpl.execution.simplification.v1");auto *D=cast<ConstantStruct>(Plan->getInitializer());require(cast<ConstantInt>(D->getOperand(2))->getZExtValue()==Mask,"all requested rules activated");Optimized=cast<Function>(D->getOperand(6));}
    auto Copy=CloneModule(*M);Copy->getFunction("driver")->eraseFromParent();
    for(auto Name:{Kernels.first->getName().str(),Optimized->getName().str()})Copy->getFunction(Name)->setLinkage(GlobalValue::ExternalLinkage);
    std::string EngineError;std::unique_ptr<ExecutionEngine> EE(EngineBuilder(std::move(Copy)).setEngineKind(EngineKind::JIT).setErrorStr(&EngineError).create());require(!!EE,EngineError.c_str());
    EE->addGlobalMapping("simplify_test_poll",reinterpret_cast<uint64_t>(&simplify_test_poll));
    EE->addGlobalMapping("simplify_test_trace",reinterpret_cast<uint64_t>(&simplify_test_trace));EE->finalizeObject();
    using Kernel=void(*)(void *,sgpl_exec_ctx *,int32_t,int32_t,const int64_t *,const int32_t *,const void *);
    auto Baseline=(Kernel)EE->getFunctionAddress(Kernels.first->getName().str());auto Candidate=(Kernel)EE->getFunctionAddress(Optimized->getName().str());require(Baseline&&Candidate,"JIT kernels");
    double xd[]={1e16,1.,-1e16,-0.,INFINITY,NAN,2.,3.};int32_t xi[]={INT32_MAX,1,-7,0,5,8,2,3};
    int64_t rows[]={0,3,3,5,6,6,7,7,7};int32_t sources[]={0,1,2,3,3,4,5};
    for(unsigned pointers=0;pointers<8;++pointers)for(unsigned gateMode=0;gateMode<3;++gateMode)for(int error:{0,-1})for(unsigned cancel:{0,1,2,3,5,7}) {
      alignas(8) unsigned char expected[64]={},actual[64]={};uint8_t ga[8]={},gb[8]={},gate[8];for(unsigned u=0;u<8;++u)gate[u]=gateMode==0?1:gateMode==1?0:u%2;
      int32_t ea=error,eb=error;sgpl_exec_ctx ca{},cb{};
      ca.spatial_source_gate=cb.spatial_source_gate=(pointers&1)?nullptr:gate;
      ca.spatial_groups_seen=(pointers&2)?nullptr:ga;cb.spatial_groups_seen=(pointers&2)?nullptr:gb;
      ca.execution_error=(pointers&4)?nullptr:&ea;cb.execution_error=(pointers&4)?nullptr:&eb;
      *(void **)EE->getGlobalValueAddress("x")=integer?(void *)xi:(void *)xd;
      double map[8];for(unsigned i=0;i<8;++i)map[i]=xd[i];
      if(!integer){((double *)expected)[3]=-0.;((double *)actual)[3]=-0.;}
      pollAt=cancel;pollCount=0;trace.clear();*(void **)EE->getGlobalValueAddress("y")=expected;Baseline(nullptr,&ca,0,8,rows,sources,map);unsigned expectedPolls=pollCount;auto expectedTrace=trace;
      pollCount=0;trace.clear();*(void **)EE->getGlobalValueAddress("y")=actual;Candidate(nullptr,&cb,0,8,rows,sources,map);
      require(!std::memcmp(expected,actual,integer?32:64),"exact ordered result");require(!std::memcmp(ga,gb,8),"group tracking");
      require(ea==eb&&expectedPolls==pollCount,"error visibility and polling positions");
      require(trace==expectedTrace,"duplicate occurrence order");
    }
    std::cout<<"PASS mask="<<Mask<<" type="<<T<<"\n";
  }
  negativeTests();
}
