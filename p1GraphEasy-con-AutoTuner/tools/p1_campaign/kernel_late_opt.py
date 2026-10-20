"""Causal diagnostic: optimize only the two late-generated full-domain kernels.

No fast math, reassociation, changed algorithm, graph rules, or promotion.
Separate fixed-address kernel sections prevent changed sizes from relocating
the rest of the executable. Alias metadata is an explicit diagnostic hypothesis;
its general compiler proof is outside this investigation.
"""
import kernel_gap as K
R=K.R
from pathlib import Path
import subprocess,sys,re,statistics
W=Path('/home/kazi/p1_kernel_20261020/late_opt');W.mkdir(parents=True,exist_ok=True)
CPP=r'''
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
using namespace llvm;
int main(int argc,char **argv){LLVMContext C;SMDiagnostic E;auto M=parseIRFile(argv[1],E,C);if(!M){E.print("lateopt",errs());return 1;}std::string mode=argv[3];
 PassBuilder PB;LoopAnalysisManager LAM;FunctionAnalysisManager FAM;CGSCCAnalysisManager CGAM;ModuleAnalysisManager MAM;PB.registerModuleAnalyses(MAM);PB.registerCGSCCAnalyses(CGAM);PB.registerFunctionAnalyses(FAM);PB.registerLoopAnalyses(LAM);PB.crossRegisterProxies(LAM,FAM,CGAM,MAM);
 for(auto name:{"sgpl_full_incoming","sgpl_full_source_map"}){auto *F=M->getFunction(name);if(!F)return 2;F->setSection(name==std::string("sgpl_full_incoming")?".gap_incoming":".gap_map");
 if(mode=="alias"){
  MDBuilder B(C);auto *D=B.createAnonymousAliasScopeDomain("private full-domain storage certificate");auto *Dest=B.createAnonymousAliasScope(D,"destination property");auto *Global=B.createAnonymousAliasScope(D,"global pointer storage");
  auto list=[&](MDNode *N){return MDNode::get(C,{N});};
  for(auto &BB:*F)for(auto &I:BB){Value *Ptr=nullptr;Type *T=nullptr;if(auto *L=dyn_cast<LoadInst>(&I)){Ptr=L->getPointerOperand();T=L->getType();}if(auto *S=dyn_cast<StoreInst>(&I)){Ptr=S->getPointerOperand();T=S->getValueOperand()->getType();}if(!Ptr)continue;
   auto *Root=getUnderlyingObject(Ptr);bool Glob=isa<GlobalVariable>(Root);bool IsDest=false;
   if(name==std::string("sgpl_full_incoming")&&T->isDoubleTy())if(auto *L=dyn_cast<LoadInst>(Root))IsDest=isa<GlobalVariable>(getUnderlyingObject(L->getPointerOperand()));
   if(IsDest){I.setMetadata(LLVMContext::MD_alias_scope,list(Dest));I.setMetadata(LLVMContext::MD_noalias,list(Global));}
   else if(Glob){I.setMetadata(LLVMContext::MD_alias_scope,list(Global));I.setMetadata(LLVMContext::MD_noalias,list(Dest));}
   else I.setMetadata(LLVMContext::MD_noalias,MDNode::get(C,{Dest,Global}));
  }
 }
 if(mode!="control"){FunctionPassManager FPM;cantFail(PB.parsePassPipeline(FPM,"sroa,early-cse,instcombine,simplifycfg,loop-simplify,lcssa,loop-mssa(licm),gvn,instcombine,simplifycfg"));FPM.run(*F,FAM);}
 }
 if(verifyModule(*M,&errs()))return 3;std::error_code EC;raw_fd_ostream O(argv[2],EC);M->print(O,nullptr);return EC?4:0;
}
'''
def build():
 p=W/'lateopt.cpp';p.write_text(CPP);exe=W/'lateopt'
 if not exe.exists():R.checked(['g++',*R.cfg('--cxxflags'),'-O2',p,*R.cfg('--ldflags'),*R.cfg('--libs','core','irreader','passes','analysis'),*R.cfg('--system-libs'),'-o',exe],W,W/'tool.build.log')
 helper=W/'exact_bits.o'
 if not helper.exists():R.checked(['gcc','-O2','-c',R.HERE/'exact_bits.c','-o',helper],W,W/'bits.build.log')
 return exe,helper
def run():
 tool,helper=build();d=R.ROOT/'versions'/K.V;records=[];bits=[]
 for x in R.read(R.OUT/'selected_cases.json'):
  if x['algorithm']!='pagerank':continue
  row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=W/'cases'/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
  for ll in (d/'src').glob('*.ll'):
   if not (folder/ll.name).exists():(folder/ll.name).symlink_to(ll)
  for mode in ['timed','validate']:
   raw=folder/(mode+'.raw.ll')
   if not raw.exists():
    env=R.M.env();env['FORCE_CPU']='1'
    R.checked([d/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(raw),R.case(K.V,'pagerank',row)/(mode+'.graph')],folder,folder/(mode+'.compiler.log'),env)
    R.save(folder/(mode+'.regenerated.json'),dict(original=R.sha(R.case(K.V,'pagerank',row)/(mode+'.o')),regenerated=R.sha(folder/'program.o')))
   for variant in ['control','opt','alias']:
    exe=folder/(mode+'.'+variant)
    if exe.exists():continue
    ir=folder/(mode+'.'+variant+'.ll');obj=folder/(mode+'.'+variant+'.o')
    R.checked([tool,raw,ir,variant],folder,folder/(mode+'.'+variant+'.transform.log'))
    R.checked(['llc-20','-O2','-filetype=obj','-mcpu=native',ir,'-o',obj],folder,folder/(mode+'.'+variant+'.codegen.log'))
    R.checked(['g++','-O3','-fopenmp','-no-pie',obj,*[d/(n+'.o') for n in R.OBJECTS],R.ROOT/'monotonic_timer.o',*([helper] if mode=='validate' else []),'-Wl,--wrap=timer_runtime',*(['-Wl,--wrap=printf','-Wl,--wrap=__printf_chk','-Wl,--wrap=puts'] if mode=='validate' else []),'-Wl,--section-start=.gap_incoming=0x10000000','-Wl,--section-start=.gap_map=0x10010000','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/(mode+'.'+variant+'.link.log'))
  for w in [1,2,6]:
   patterns=[]
   for variant in ['control','opt','alias']:
    path=folder/(variant+f'.w{w}.bits')
    z=R.invoke(K.ATTEMPT,'validate_late_opt',K.V,'pagerank',row,0,label=variant,workers=w,extra={'SGPL_BITS_OUTPUT':str(path)},executable_override=folder/('validate.'+variant));assert not z.get('error'),z;patterns.append(path.read_bytes())
   assert len(patterns[0])==row['vertices']*8 and all(p==patterns[0] for p in patterns),'late optimization changed FP output'
   bits.append(dict(graph=row['graph'],workers=w,doubles=row['vertices'],sha256=R.hashlib.sha256(patterns[0]).hexdigest()))
  for rep in range(20):
   specs=['control','opt','alias','AA'];specs=specs if rep%2==0 else list(reversed(specs))
   for variant in specs:
    z=R.invoke(K.ATTEMPT,'late_opt',K.V,'pagerank',row,rep,label=variant,executable_override=folder/('timed.'+('control' if variant=='AA' else variant)));assert not z.get('error'),z;records.append(z)
  R.save(R.OUT/'late_opt.json',records);R.save(R.OUT/'late_opt_bits.json',bits);R.progress(stage='late_opt',algorithm='pagerank',graph=row['graph'])
if __name__=='__main__':
 with R.locked('kernel_late_opt'):
  R.verify_inputs();R.verify_fingerprint(K.V);run()
