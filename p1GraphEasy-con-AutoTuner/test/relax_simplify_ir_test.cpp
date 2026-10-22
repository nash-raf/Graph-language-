#include <cstdlib>
#include <iostream>
#include <fstream>
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/SourceMgr.h>
#include "autotuner_runtime.h"
#include "fullgraph_lowering.inc"
#include "append_journal_lowering.inc"
#include "execution_simplify_lowering.inc"
#include "frontier_simplify_lowering.inc"
#include "relax_simplify_lowering.inc"
using namespace llvm;
static void require(bool X,const char *Why){if(!X){errs()<<"FAIL "<<Why<<"\n";std::exit(1);}}
static std::unique_ptr<Module> fixture(LLVMContext &C,unsigned Negative) {
  std::string IR=R"IR(
@array=internal global ptr null, !sgpl.language.array !0
@other=internal global ptr null
@graph=internal global ptr null
@out=internal global ptr null
@side=internal global i32 0
declare ptr @calloc(i64,i64)
declare ptr @autograph_exec_ctx_create(ptr,i32,i32,ptr,ptr,ptr,i32,ptr,ptr,i64,ptr,i32,i32)
declare void @autograph_exec_ctx_set_dag_axes(ptr,i32,i32)
declare i32 @autograph_exec_ctx_bind_array_frontier(ptr,ptr,i32,ptr,ptr,i32)
declare i32 @autograph_exec_ctx_prepare_spatial(ptr,i32,i32,i32)
declare i32 @autograph_exec_ctx_bind_source_snapshot_range(ptr,i32,ptr,i64,i64)
declare void @autograph_exec_ctx_destroy(ptr)
declare i32 @autograph_frontier_execute(ptr,ptr)
declare i32 @autograph_frontier_activate(ptr,i32)
declare i32 @autograph_exec_ctx_edge_weight(ptr,ptr,i32,i32)
declare void @sgpl_exec_require_status(i32)
declare void @opaque(ptr)
declare void @observe(i32)
define internal void @pair(ptr %state,ptr %ctx,i32 %u,i32 %v){
)IR";
  if(Negative==22)IR+="br label %again\nagain: br i1 true,label %again,label %read\nread:\n";
  IR+="%tablefield=getelementptr i8,ptr %ctx,i64 "+std::to_string(offsetof(sgpl_exec_ctx,spatial_snapshots))+"\n";
  IR+="%table=load ptr,ptr %tablefield\n%snapshot=load ptr,ptr %table\n%ui=sext i32 "+std::string(Negative==4?"%v":"%u")+" to i64\n%sp=getelementptr i32,ptr %snapshot,i64 %ui\n";
  IR+=Negative==2?"%source=load atomic i32,ptr %sp acquire,align 4\n":Negative==3?"%source=load volatile i32,ptr %sp\n":"%source=load i32,ptr %sp\n";
  if(Negative==1)IR+="call void @opaque(ptr null)\n";
  if(Negative==9)IR+="store i32 1,ptr @side\n";
  if(Negative==12)IR+="store i32 0,ptr %sp\n";
  if(Negative==23)IR+="%ordinalfield=getelementptr i8,ptr %ctx,i64 "+std::to_string(offsetof(sgpl_exec_ctx,array_work_ordinal))+"\n%ordinal=load i64,ptr %ordinalfield\n";
  if(Negative>=24) {
    IR+="%wg=load ptr,ptr "+std::string(Negative==25?"@other":"@graph")+"\n";
    IR+="%weightfield=getelementptr i8,ptr %ctx,i64 "+std::to_string(offsetof(sgpl_exec_ctx,array_edge_index))+"\n%weightindex=load i64,ptr %weightfield,!sgpl.ctx.accessor !1,!sgpl.relax.weight.accessor !0\n";
    IR+="%choose=icmp eq i32 %u,0\nbr i1 %choose,label %native,label %fallback,!sgpl.relax.weight.accessor !0\nnative:\n%nativeweight=load i32,ptr @side,!sgpl.relax.weight.accessor !0\nbr label %weightjoin\nfallback:\n%lookup=call i32 @autograph_exec_ctx_edge_weight(ptr %ctx,ptr %wg,i32 %u,i32 %v),!sgpl.relax.weight.accessor !0\nbr label %weightjoin\nweightjoin:\n%weight=phi i32 [%nativeweight,%native],[%lookup,%fallback]\n%candidate=add "+std::string(Negative==26?"nsw ":"")+"i32 %source,%weight\n";
  }
  IR+="%base=load ptr,ptr @array\n%vi=sext i32 %v to i64\n%dp=getelementptr i32,ptr %base,i64 %vi\n%old=load i32,ptr %dp\n";
  if(Negative==5)IR+="%candidate=add i32 %source,%u\n";
  std::string Candidate=Negative==5||Negative>=24?"%candidate":Negative==7?"%old":"%source";
  IR+="%less=icmp "+std::string(Negative==6?"ult":"slt")+" i32 "+(Negative>=24?"%candidate":"%source")+",%old\nbr i1 %less,label %update,label %done\nupdate:\nstore i32 "+Candidate+",ptr %dp\n";
  if(Negative!=8)IR+="%app=call i32 @autograph_frontier_activate(ptr %ctx,i32 %v)\n";
  IR+="br label %done\ndone:\n";
  if(Negative==8)IR+="%app=call i32 @autograph_frontier_activate(ptr %ctx,i32 %v)\n";
  IR+="ret void}\ndefine i32 @main(){\nentry:\n%values=call ptr @calloc(i64 4,i64 4),!sgpl.language.array !0\nstore ptr %values,ptr @array\n%fresh=call ptr @calloc(i64 4,i64 4)\n%second=call ptr @calloc(i64 4,i64 4)\nstore ptr %second,ptr @out\n%init=getelementptr i32,ptr %fresh,i32 0\nstore i32 0,ptr %init\n";
  if(Negative==20)IR+="call void @opaque(ptr %fresh)\n";
  IR+=R"IR(br label %loop
loop:
%input=phi ptr [%fresh,%entry],[%next,%loop]
%size=phi i32 [1,%entry],[%result,%loop]
%g=load ptr,ptr @graph
%output=load ptr,ptr @out
%ctx=call ptr @autograph_exec_ctx_create(ptr %g,i32 1,i32 1,ptr null,ptr null,ptr %output,i32 0,ptr null,ptr null,i64 0,ptr null,i32 0,i32 -1)
call void @autograph_exec_ctx_set_dag_axes(ptr %ctx,i32 1,i32 -1)
%bind=call i32 @autograph_exec_ctx_bind_array_frontier(ptr %ctx,ptr %input,i32 %size,ptr @out,ptr null,i32 1)
)IR";
  if(Negative==13)IR+="call void @opaque(ptr %ctx)\n";
  if(Negative==14)IR+="store i32 3,ptr @side\n";
  IR+=R"IR(%prep=call i32 @autograph_exec_ctx_prepare_spatial(ptr %ctx,i32 2,i32 2,i32 0)
%a=load ptr,ptr @array
%snap=call i32 @autograph_exec_ctx_bind_source_snapshot_range(ptr %ctx,i32 0,ptr %a,i64 4,i64 4)
%result=call i32 @autograph_frontier_execute(ptr %g,ptr %ctx)
)IR";
  if(Negative==16)IR+="call void @observe(i32 %result)\n";
  if(Negative==17)IR+="%peek=load i32,ptr %input\ncall void @observe(i32 %peek)\n";
  IR+=R"IR(call void @sgpl_exec_require_status(i32 %result)
call void @autograph_exec_ctx_destroy(ptr %ctx)
%next=load ptr,ptr @out
store ptr %input,ptr @out
%more=icmp sgt i32 %result,0
br i1 %more,label %loop,label %exit
exit: ret i32 0
}
!0=!{}
!1=!{!"autograph_exec_ctx_edge_weight"}
)IR";
  SMDiagnostic E;auto M=parseAssemblyString(IR,E,C);if(!M)E.print("relax-test",errs());require(!!M,"parse fixture");
  M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
  auto *Pair=M->getFunction("pair");Pair->addFnAttr("sgpl.spatial.callback");Pair->addFnAttr("sgpl.record.source_ordinal.dead");
  for(BasicBlock &BB:*M->getFunction("main"))for(Instruction &I:BB)if(auto *B=dyn_cast<CallInst>(&I))if(B->getName()=="bind") {
    if(Negative!=18)B->setMetadata("sgpl.exec.frontier",MDNode::get(C,{ValueAsMetadata::get(Pair),ConstantAsMetadata::get(Negative==19?cast<Constant>(Pair):ConstantPointerNull::get(PointerType::get(C,0)))}));
    B->setMetadata("sgpl.relax.snapshot.bases",MDNode::get(C,{MDNode::get(C,{ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C),Negative==11?1:0)),ValueAsMetadata::get(M->getGlobalVariable("array",true))})}));
  }
  require(!verifyModule(*M,&errs()),"fixture verification");return M;
}
int main(int argc,char **argv) {
  unsigned Checks=0;
  for(unsigned Subset=0;Subset<8;++Subset)for(unsigned Negative:{0,1,2,3,4,5,6,7,8,9,11,12,13,14,16,17,18,19,20,22,23,24,25,26}) {
    std::string Rules;
    for(unsigned I=0;I<3;++I)if(Subset&(1u<<I)){if(!Rules.empty())Rules+=",";Rules+="R"+std::to_string(I+6);}
    setenv("SGPL_EXEC_SIMPLIFY_RULES",Rules.empty()?"none":Rules.c_str(),1);
    LLVMContext C;auto M=fixture(C,Negative);lowerRelaxationSimplifications(*M);
    require(!verifyModule(*M,&errs()),"transformed module verifies");
    unsigned Expected=Subset<<5;
    if(Negative&&Negative!=16&&Negative!=17&&Negative!=20&&Negative!=24)Expected=0;
    if(Negative==16||Negative==17||Negative==20)Expected&=~128u;
    unsigned Actual=0;
    for(auto &G:M->globals())if(G.getName().starts_with("sgpl.relax.simplification.v1"))Actual=cast<ConstantInt>(cast<ConstantStruct>(G.getInitializer())->getOperand(2))->getZExtValue();
    if(Actual!=Expected){errs()<<"subset="<<Subset<<" negative="<<Negative<<" expected="<<Expected<<" actual="<<Actual<<"\n";require(false,"rule applicability and independent refusal");}
    require(M->getFunction("pair")&&!M->getFunction("pair")->empty(),"baseline callback retained");
    if(argc>1&&Subset==7){std::error_code E;raw_fd_ostream O(std::string(argv[1])+"/fixture_"+std::to_string(Negative)+".ll",E);M->print(O,nullptr);}
    ++Checks;
  }
  std::cout<<"PASS relaxation IR checks="<<Checks<<" rule_subsets=8\n";
}
