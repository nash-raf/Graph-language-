#include <cstdlib>
#include <cstring>
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
#include "execution_fusion_context.inc"
#include "append_journal_lowering.inc"
using namespace llvm;
static std::vector<std::pair<int64_t,int32_t>> entries;
extern "C" int32_t journal_ir_append(sgpl_exec_ctx *c,int32_t value) {
  if(!c||value<0)return 0;
  if(c->array_append_seq>=c->array_append_sites)return 0;
  entries.emplace_back(c->array_work_ordinal*c->array_append_sites+c->array_append_seq++,value);return 1;
}
static bool execute(Module &Source) {
  auto M=CloneModule(Source);M->getFunction("main")->eraseFromParent();
  M->getFunction("pair")->setLinkage(GlobalValue::ExternalLinkage);
  M->getFunction("pair.append_journal_v3")->setLinkage(GlobalValue::ExternalLinkage);
  std::string error;std::unique_ptr<ExecutionEngine> EE(EngineBuilder(std::move(M)).setEngineKind(EngineKind::JIT).setErrorStr(&error).create());
  if(!EE){errs()<<error;return false;}
  for(const char *name:{"autograph_frontier_activate","sgpl_frontier_journal_activate_v1"})
    EE->addGlobalMapping(name,reinterpret_cast<uint64_t>(&journal_ir_append));
  EE->finalizeObject();using Pair=void(*)(void *,sgpl_exec_ctx *,int32_t,int32_t);
  auto Base=(Pair)EE->getFunctionAddress("pair"),Journal=(Pair)EE->getFunctionAddress("pair.append_journal_v3");
  if(!Base||!Journal)return false;
  std::vector<std::pair<int64_t,int32_t>> expected;int32_t a[]={99,99,99,99},b[4];std::memcpy(b,a,sizeof a);
  const int32_t sources[]={0,0,2,1,1,0,3,2},destinations[]={2,2,2,0,0,2,1,2};
  for(int mode=0;mode<2;++mode) {
    entries.clear();sgpl_exec_ctx c{};c.array_append_sites=2;
    for(unsigned i=0;i<8;++i){c.array_work_ordinal=i;c.array_append_seq=0;(mode?Journal:Base)(mode?b:a,&c,sources[i],destinations[i]);}
    if(!mode)expected=entries;else if(expected!=entries||std::memcmp(a,b,sizeof a))return false;
  }
  return true;
}
int main() {
  InitializeNativeTarget();InitializeNativeTargetAsmPrinter();
  for(int fixture=0;fixture<9;++fixture)for(bool disabled:{false,true}) {
    bool hasInvoke=fixture==4||fixture==5;
    LLVMContext C;SMDiagnostic E;
    std::string ir="declare i32 @autograph_frontier_activate(ptr,i32)\n"
      "declare i32 @autograph_exec_ctx_bind_array_frontier(ptr,ptr,i32,ptr,ptr,i32)\n";
    if(hasInvoke)ir+="declare i32 @__gxx_personality_v0(...)\n";
    if(fixture==5)ir+="define internal void @append_helper(ptr %ctx,i32 %v){%r=call i32 @autograph_frontier_activate(ptr %ctx,i32 %v)\nret void}\n";
    ir+="define internal void @pair(ptr %state,ptr %ctx,i32 %u,i32 %v)";
    if(hasInvoke)ir+=" personality ptr @__gxx_personality_v0";
    ir+="{\n"
      "%p=getelementptr i32,ptr %state,i32 %v\n%old=load i32,ptr %p\n%less=icmp slt i32 %u,%old\n"
      "br i1 %less,label %update,label %done\nupdate:\nstore i32 %u,ptr %p\n";
    if(fixture==1)ir+="%raw=getelementptr i8,ptr %ctx,i64 "+std::to_string(offsetof(sgpl_exec_ctx,array_append_log))+"\n%log=load ptr,ptr %raw\n%observed=load i32,ptr %log\n";
    if(fixture==6)ir+="%atomic=atomicrmw add ptr %p,i32 1 seq_cst\n";
    if(fixture==7)ir+="%ordinalptr=getelementptr i8,ptr %ctx,i64 "+std::to_string(offsetof(sgpl_exec_ctx,array_work_ordinal))+"\n%observedord=load volatile i64,ptr %ordinalptr\n";
    if(fixture==8)ir+="%boundptr=getelementptr i8,ptr %ctx,i64 "+std::to_string(offsetof(sgpl_exec_ctx,array_append_sites))+"\nstore i32 1,ptr %boundptr\n";
    if(fixture==2)ir+="br label %cycle\ncycle:\n%i=phi i32 [0,%update],[%next,%cycle]\n";
    ir+="%a=call i32 @autograph_frontier_activate(ptr %ctx,i32 %v)\n";
    if(fixture==4)ir+="%b=invoke i32 @autograph_frontier_activate(ptr %ctx,i32 %v) to label %done unwind label %lpad\n";
    else if(fixture==5)ir+="invoke void @append_helper(ptr %ctx,i32 %v) to label %done unwind label %lpad\n";
    else ir+="%b=call i32 @autograph_frontier_activate(ptr %ctx,i32 %v)\n";
    if(fixture==2)ir+="%next=add i32 %i,1\n%again=icmp slt i32 %next,%u\nbr i1 %again,label %cycle,label %done\n";
    else if(!hasInvoke)ir+="br label %done\n";
    if(hasInvoke)ir+="lpad:\n%exception=landingpad {ptr,i32} cleanup\nret void\n";
    ir+="done:\nret void\n}\ndefine void @main(ptr %ctx){%r=call i32 @autograph_exec_ctx_bind_array_frontier(ptr %ctx,ptr null,i32 0,ptr null,ptr null,i32 "+std::to_string(fixture==3?1:2)+")\nret void}\n";
    auto M=parseAssemblyString(ir,E,C);if(!M){E.print("journal-ir",errs());return 1;}
    M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
    auto *Pair=M->getFunction("pair");Pair->addFnAttr("sgpl.spatial.callback");
    auto *Bind=cast<CallInst>(&*M->getFunction("main")->getEntryBlock().begin());
    Bind->setMetadata("sgpl.append.journal.certificate",MDNode::get(C,{ValueAsMetadata::get(Pair),ConstantAsMetadata::get(ConstantPointerNull::get(PointerType::get(C,0)))}));
    if(disabled)setenv("SGPL_NO_FUSION_C_LOWERING","1",1);else unsetenv("SGPL_NO_FUSION_C_LOWERING");
    lowerAppendJournals(*M);if(verifyModule(*M,&errs()))return 2;
    bool active=M->getFunction("pair.append_journal_v3")!=nullptr;if(active!=(fixture==0&&!disabled))return 3;
    if(active) {
      auto *Plan=M->getNamedGlobal("sgpl.append.journal.plan.v3");
      auto *Fields=Plan?dyn_cast<ConstantStruct>(Plan->getInitializer()):nullptr;
      auto *Bits=Fields?dyn_cast<ConstantInt>(Fields->getOperand(2)):nullptr;
      if(!Bits||Bits->getZExtValue()!=(5|SGPL_JOURNAL_PROVEN_APPEND))return 5;
      auto *Bound=dyn_cast<ConstantInt>(Fields->getOperand(3));
      if(!Bound||Bound->getZExtValue()!=2)return 6;
    }
    if(active&&!execute(*M))return 4;
  }
  unsetenv("SGPL_NO_FUSION_C_LOWERING");outs()<<"PASS append clone JIT: exact effects, duplicate append sequence, bounds, observers, cycles, invoke refusal and disable\n";
}
