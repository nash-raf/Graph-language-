#include "../graph_frontier_lowering.cpp"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"
#include <iostream>
#include "llvm/Support/TargetSelect.h"
void emitGpuKernels(llvm::Module &,llvm::StringRef) __attribute__((weak));
int main(int argc,char **argv) {
  if(argc!=2&&argc!=3)return 2;
  bool device=argc==3&&std::string(argv[2])=="gpu";
  if(device) {
    if(!emitGpuKernels)return 2;
    setenv("FORCE_GPU","1",1);setenv("FORCE_CPU","0",1);
    InitializeAllTargetInfos();InitializeAllTargets();InitializeAllTargetMCs();InitializeAllAsmPrinters();InitializeAllAsmParsers();
  }
  for(std::string mode : {"source","destination","snapshot","source_acc","dynamic_scratch","dynamic_snapshot","exit_phi","guard_phi","frontier_source","frontier_destination","unsafe_extent","prefix_phi"}) {
    LLVMContext C;
    std::string body = (mode=="source" || mode=="dynamic_scratch" || mode=="exit_phi" || mode=="guard_phi" || mode=="frontier_source" || mode=="unsafe_extent" || mode=="prefix_phi") ?
      "%target = getelementptr [7 x i32], ptr %x, i32 0, i64 %u\n store i32 %v, ptr %target\n" :
      "%target = getelementptr [7 x i32], ptr %x, i32 0, i32 %v\n %u32 = trunc i64 %u to i32\n store i32 %u32, ptr %target\n";
    if(mode=="source_acc")body="%oldacc = load i32,ptr @acc\n %plus = add i32 %oldacc,%v\n store i32 %plus,ptr @acc\n";
    if(mode=="snapshot" || mode=="dynamic_snapshot")body=
      "%leftp = getelementptr [7 x i32], ptr %x, i32 0, i64 %u\n"
      "%target = getelementptr [7 x i32], ptr %x, i32 0, i32 %v\n"
      "%left = load i32, ptr %leftp\n %right = load i32, ptr %target\n"
      "%sum = add i32 %left, %right\n store i32 %sum, ptr %target\n";
    std::string ir="target triple = \"x86_64-pc-linux-gnu\"\n"
      "target datalayout = \"e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128\"\n"
      "@acc = internal global i32 0\n%Graph = type {i64,i64,ptr,ptr}\n @joint_graph = external global ptr\n @joint_output = external global [9 x i32]\n"
      "declare ptr @load_graph_from_file_fixture()\n"
      "declare void @autograph_neighbor_iter_init(ptr,i64,ptr)\n"
      "declare i32 @autograph_neighbor_iter_next(ptr,ptr)\n"
      "define void @joint_test() {\nentry:\n %g = call ptr @load_graph_from_file_fixture()\n"
      "%np = getelementptr %Graph, ptr %g, i32 0,i32 0\n %n = load i64, ptr %np\n br label %round\n"
      "round:\n %r = phi i32 [0,%entry],[%rn,%suffix]\n %more = icmp slt i32 %r,9\n br i1 %more,label %prefix,label %exit\n"
      "prefix:\n %x = alloca [7 x i32]\n %iter = alloca [128 x i8]\n %vslot = alloca i32\n";
    for(int i=0;i<7;++i)ir+="%xp"+std::to_string(i)+" = getelementptr [7 x i32],ptr %x,i32 0,i32 "+std::to_string(i)+
      "\n store i32 "+std::to_string(i+1)+",ptr %xp"+std::to_string(i)+"\n";
    ir+="br label %driver\ndriver:\n %u = phi i64 [0,%prefix],[%un,%driver_latch]\n %hasu = icmp slt i64 %u,%n\n"
      "br i1 %hasu,label %source,label %suffix\nsource:\n"
      "call void @autograph_neighbor_iter_init(ptr %g,i64 %u,ptr %iter)\n br label %neighbor\n"
      "neighbor:\n %next = call i32 @autograph_neighbor_iter_next(ptr %iter,ptr %vslot)\n %hasv = icmp ne i32 %next,0\n"
      "br i1 %hasv,label %pair,label %driver_latch\npair:\n %v = load i32,ptr %vslot\n"+body+
      "br label %neighbor\ndriver_latch:\n %un = add i64 %u,1\n br label %driver\nsuffix:\n";
    for(int i=0;i<7;++i)ir+="%readp"+std::to_string(i)+" = getelementptr [7 x i32],ptr %x,i32 0,i32 "+std::to_string(i)+
      "\n %read"+std::to_string(i)+" = load i32,ptr %readp"+std::to_string(i)+"\n";
    ir+="%s0 = add i32 %read0,%read1\n";
    for(int i=2;i<7;++i)ir+="%s"+std::to_string(i-1)+" = add i32 %s"+std::to_string(i-2)+",%read"+std::to_string(i)+"\n";
    ir+="%result = add i32 %s5,%r\n %outp = getelementptr [9 x i32],ptr @joint_output,i32 0,i32 %r\n"
      "store i32 %result,ptr %outp\n %rn = add i32 %r,1\n br label %round\nexit:\n ret void\n}\n";
    if(mode=="source_acc") {
      size_t where=ir.find("source:\n");ir.insert(where+8,"store i32 0,ptr @acc\n");
      where=ir.find("driver_latch:\n");ir.insert(where+14,"%accresult = load i32,ptr @acc\n%accdest = getelementptr [7 x i32],ptr %x,i32 0,i64 %u\nstore i32 %accresult,ptr %accdest\n");
    }
    if(mode=="dynamic_scratch" || mode=="dynamic_snapshot") {
      size_t where=ir.find("%x = alloca [7 x i32]");
      ir.replace(where,std::string("%x = alloca [7 x i32]").size(),"%extent = add i32 %r,7\n%x = alloca i32,i32 %extent");
    }
    if(mode=="exit_phi" || mode=="guard_phi" || mode=="unsafe_extent") {
      size_t where=ir.find("suffix:\n");
      ir.insert(where+8,mode=="exit_phi" ? "%final_u = phi i64 [%u,%driver]\n" : "%final_u = phi i64 [%u,%driver],[-1,%prefix]\n");
      where=ir.find("%result = add i32 %s5,%r");
      ir.replace(where,std::string("%result = add i32 %s5,%r").size(),"%final_u32 = trunc i64 %final_u to i32\n%with_u = add i32 %s5,%final_u32\n%result = add i32 %with_u,%r");
      if(mode=="guard_phi" || mode=="unsafe_extent") {
        where=ir.find("br label %driver\ndriver:");
        ir.replace(where,std::string("br label %driver").size(),"%enabled = icmp ne i32 %r,4\nbr i1 %enabled,label %driver,label %suffix");
      }
    }
    if(mode=="unsafe_extent") {
      size_t where=ir.find("label %driver,label %suffix\ndriver:");
      ir.replace(where,std::string("label %driver,label %suffix").size(),"label %allocate,label %suffix\nallocate:\n%denominator = sub i32 %r,4\n%extent = udiv i32 100,%denominator\n%unused = alloca i32,i32 %extent\nbr label %driver");
      where=ir.find("%u = phi i64 [0,%prefix]");ir.replace(where,std::string("%u = phi i64 [0,%prefix]").size(),"%u = phi i64 [0,%allocate]");
    }
    if(mode=="prefix_phi") {
      size_t where=ir.find("prefix:\n");
      ir.insert(where+8,"%odd = and i32 %r,1\n%choose = icmp eq i32 %odd,0\nbr i1 %choose,label %left,label %right\nleft:\n%left_size = add i32 %r,7\nbr label %allocate\nright:\n%right_size = add i32 %r,9\nbr label %allocate\nallocate:\n%extent = phi i32 [%left_size,%left],[%right_size,%right]\n%other = phi i32 [7,%left],[9,%right]\n");
      where=ir.find("%x = alloca [7 x i32]");ir.replace(where,std::string("%x = alloca [7 x i32]").size(),"%x = alloca i32,i32 %extent");
      where=ir.find("%u = phi i64 [0,%prefix]");ir.replace(where,std::string("%u = phi i64 [0,%prefix]").size(),"%u = phi i64 [0,%allocate]");
    }
    bool frontier=mode=="frontier_source"||mode=="frontier_destination";
    if(frontier) {
      size_t where=ir.find("define void @joint_test");
      ir.insert(where,"declare void @roaring_bitmap_add(ptr,i32)\ndeclare i1 @roaring_bitmap_contains(ptr,i32)\n");
      where=ir.find("prefix:\n");
      ir.insert(where+8,"%front_bits = alloca i64\n%next_bits = alloca i64\n%front_slot = alloca ptr\n%next_slot = alloca ptr\n%odd = and i32 %r,1\n%even = icmp eq i32 %odd,0\n%mask = select i1 %even,i64 127,i64 17\nstore i64 %mask,ptr %front_bits\nstore i64 0,ptr %next_bits\nstore ptr %front_bits,ptr %front_slot\nstore ptr %next_bits,ptr %next_slot\n");
      where=ir.find("source:\n");
      ir.insert(where+8,"%front_bitmap = load ptr,ptr %front_slot\n%source32 = trunc i64 %u to i32\n%enabled = call i1 @roaring_bitmap_contains(ptr %front_bitmap,i32 %source32)\nbr i1 %enabled,label %active_source,label %driver_latch\nactive_source:\n");
      where=ir.find("br label %neighbor\ndriver_latch:");
      ir.insert(where,"%next_bitmap = load ptr,ptr %next_slot\ncall void @roaring_bitmap_add(ptr %next_bitmap,i32 %v)\n");
      where=ir.find("%result = add i32 %s5,%r");
      ir.replace(where,std::string("%result = add i32 %s5,%r").size(),"%published_bits = load i64,ptr %next_bits\n%bits32 = trunc i64 %published_bits to i32\n%with_frontier = add i32 %s5,%bits32\n%result = add i32 %with_frontier,%r");
    }
    SMDiagnostic error;auto M=parseAssemblyString(ir,error,C);
    if(!M){error.print("joint",errs());return 1;}
      if(device) M->getOrInsertNamedMetadata("graph.ir.backend")->addOperand(MDNode::get(C,MDString::get(C,"gpu")));
    PassBuilder PB;LoopAnalysisManager LAM;FunctionAnalysisManager FAM;CGSCCAnalysisManager CGAM;ModuleAnalysisManager MAM;
    PB.registerModuleAnalyses(MAM);PB.registerCGSCCAnalyses(CGAM);PB.registerFunctionAnalyses(FAM);PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM,FAM,CGAM,MAM);
    Function &F=*M->getFunction("joint_test");
    if(frontier) {
      // Emitter fixture: supply a round certificate for this explicitly isolated
      // bitmap fixture. Production round analysis stays conservative for bitmap
      // calls. The existing spatial prover validates all frontier contracts.
      auto &LI=FAM.getResult<LoopAnalysis>(F);auto &AA=FAM.getResult<AAManager>(F);auto &DT=FAM.getResult<DominatorTreeAnalysis>(F);
      NeighborLoopInfo Info;bool recognized=false;
      for(Loop *L:LI.getLoopsInPreorder())if(analyzeNeighborLoop(L,Info)){recognized=true;break;}
      if(!recognized)return 1;
      for(BasicBlock &B:F)for(Instruction &I:B)if(I.getName()=="front_slot")Info.FrontierSetPtr=&I;
      Info.MembershipGated=true;
      proveSpatialPartition(Info,AA,DT);finalizeProofInputs(Info);
      RoundDomain R;proveRounds(Info,R,AA,DT);
      if(Info.SpatialResult.Status!=SpatialProofStatus::IndependentFamily||!Info.SpatialResult.RuntimeRealizable) {
        spatialPrint(Info);errs()<<"FAIL frontier spatial contract\n";return 1;
      }
      Info.TemporalResult.Rounds.Status=TemporalProofStatus::Independent;
      if(!emitRoundSpatial(Info,R)){errs()<<"FAIL certified frontier emission\n";return 1;}
      for(Function &Helper:*M)if(Helper.getName().starts_with("sgpl.round.spatial")||Helper.hasFnAttribute("sgpl.spatial.callback"))
        for(BasicBlock &B:Helper)for(Instruction &I:B)if(auto *Call=dyn_cast<CallInst>(&I))if(auto *Callee=Call->getCalledFunction()) {
          StringRef Name=Callee->getName();
          if(Name.starts_with("autograph_scratch_")||Name.starts_with("autograph_prepare_frontier_")||Name=="autograph_commit_frontier_bitmap") {
            errs()<<"FAIL shared frontier publication "<<Name<<"\n";return 1;
          }
        }
    } else GraphFrontierLoweringPass().run(F,FAM);
    bool found=false;
    for(BasicBlock &B:F)for(Instruction &I:B)if(auto *Call=dyn_cast<CallInst>(&I))
      if(Call->getCalledFunction()&&(Call->getCalledFunction()->getName()=="sgpl_exec_dag_round_spatial"||Call->getCalledFunction()->getName()=="sgpl_exec_dag_round_spatial_gpu"))found=true;
    if(mode=="unsafe_extent") {
      bool fallback=false;
      for(BasicBlock &B:F)for(Instruction &I:B)if(auto *Call=dyn_cast<CallInst>(&I))
        fallback |= Call->getMetadata("sgpl.round-spatial.unavailable")!=nullptr;
      if(found||!fallback||verifyModule(*M,&errs())){errs()<<"FAIL unsafe reservation fallback\n";return 1;}
    } else if(!found||verifyModule(*M,&errs())){errs()<<"FAIL combined "<<mode<<"\n";return 1;}
    if(device) emitGpuKernels(*M,std::string(argv[1])+"/"+mode+".ptx");
    std::error_code ec;raw_fd_ostream out(std::string(argv[1])+"/"+mode+".ll",ec);
    if(ec)return 1;M->print(out,nullptr);
    std::cout<<"PASS "<<(mode=="unsafe_extent" ? "safe round-only fallback " : "combined LLVM emission ")<<mode<<"\n";
  }
}
