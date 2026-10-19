
#include <cstdlib>
#include <iostream>
#include "autotuner_runtime.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include "../record_field_liveness.inc"
int main(int argc,char **argv){
 if(argc==2&&std::string(argv[1])=="offset"){std::cout<<offsetof(sgpl_exec_ctx,array_source_ordinal);return 0;}
 if(argc==2&&std::string(argv[1])=="gate_offset"){std::cout<<offsetof(sgpl_exec_ctx,spatial_source_gate);return 0;}
 if(argc==2&&std::string(argv[1])=="records_offset"){std::cout<<offsetof(sgpl_exec_ctx,gpu_records);return 0;}
 if(argc==2&&std::string(argv[1])=="sources_offset"){std::cout<<offsetof(sgpl_exec_ctx,gpu_record_sources);return 0;}
 if(argc==2&&std::string(argv[1])=="work_offset"){std::cout<<offsetof(sgpl_exec_ctx,gpu_record_work);return 0;}
 if(argc==2&&std::string(argv[1])=="ops_offset"){std::cout<<offsetof(sgpl_exec_ctx,ops);return 0;}
 if(argc==2&&std::string(argv[1])=="scratch_offset"){std::cout<<offsetof(sgpl_exec_ctx,execution_scratch);return 0;}
 if(argc==2&&std::string(argv[1])=="format_offset"){std::cout<<offsetof(sgpl_exec_ctx,array_compact_records);return 0;}
 if(argc==2&&std::string(argv[1])=="edges_offset"){std::cout<<offsetof(sgpl_exec_ctx,array_record_edges);return 0;}
 llvm::LLVMContext C;llvm::SMDiagnostic E;auto M=llvm::parseIRFile(argv[1],E,C);
 if(!M){E.print(argv[0],llvm::errs());return 2;}
 M->setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
 llvm::SmallPtrSet<llvm::Function *,16> Active;
 return sgplSourceOrdinalDead(M->getFunction("pair"),1,Active)?0:1;
}
