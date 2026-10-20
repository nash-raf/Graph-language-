#include <cstdlib>
#include <iostream>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SourceMgr.h>
#include "autotuner_runtime.h"
#include "static_schedule_lowering.inc"
using namespace llvm;
int main() {
  struct Fixture{const char *name,*body;bool enabled;};
  Fixture fixtures[]={
    {"ordinary","%x = load double, ptr @value\nstore double %x, ptr @output",true},
    {"thread_local","%x = load i32, ptr @tls",false},
    {"thread_local_gep","%x = load i32, ptr getelementptr ([2 x i32], ptr @tls_array, i64 0, i64 1)",false},
    {"opaque","call void @opaque()",false},
    {"context","%x = load i32, ptr %ctx",false},
    {"volatile","%x = load volatile double, ptr @value",false},
    {"atomic","%x = load atomic i32, ptr @shared acquire, align 4",false},
  };
  for(auto &T:fixtures)for(bool gate:{false,true})for(bool disabled:{false,true}) {
    LLVMContext C;SMDiagnostic E;
    std::string ir="@value = global double 0.0\n@output = global double 0.0\n@tls = thread_local global i32 0\n@tls_array = thread_local global [2 x i32] zeroinitializer\n@shared = global i32 0\n"
      "declare void @opaque()\ndeclare void @autograph_exec_ctx_set_source_gate(ptr, i32, i32)\n"
      "declare void @sgpl_schedule_certificate(ptr, ptr, ptr)\n"
      "define void @pair(ptr %state, ptr %ctx, i32 %u, i32 %v) {\n"+std::string(T.body)+"\nret void\n}\n"
      "define void @gate(ptr %op, ptr %ctx, i32 %u) {\ncall void @autograph_exec_ctx_set_source_gate(ptr %ctx, i32 %u, i32 1)\nret void\n}\n"
      "define void @main(ptr %ctx) {\ncall void @sgpl_schedule_certificate(ptr %ctx, ptr @pair, ptr "+std::string(gate?"@gate":"null")+")\nret void\n}\n";
    auto M=parseAssemblyString(ir,E,C);if(!M){E.print("schedule-test",errs());return 1;}
    M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
    if(disabled)setenv("SGPL_NO_STATIC_SCHEDULE_LOWERING","1",1);else unsetenv("SGPL_NO_STATIC_SCHEDULE_LOWERING");
    lowerStaticSchedules(*M);
    bool active=M->getFunction("autograph_exec_ctx_bind_static_schedule_v1")!=nullptr;
    if(active!=(T.enabled&&!disabled) || M->getFunction("sgpl_schedule_certificate") || verifyModule(*M,&errs())) {
      std::cerr<<"FAIL "<<T.name<<" gate="<<gate<<" disabled="<<disabled<<"\n";return 1;
    }
  }
  unsetenv("SGPL_NO_STATIC_SCHEDULE_LOWERING");
  std::cout<<"PASS static schedule activation/refusal: 28 compiler fixtures\n";
}
