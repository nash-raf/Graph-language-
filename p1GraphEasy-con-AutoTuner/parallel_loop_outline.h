#ifndef PARALLEL_LOOP_OUTLINE_H
#define PARALLEL_LOOP_OUTLINE_H

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

void registerLoopOutlinerPluginWithPassBuilder(llvm::PassBuilder &PB);
void runLoopOutlinerOnModule(llvm::Module &M);
void registerLoopOutlinerPass(llvm::FunctionPassManager &FPM);

// If GPU backend is enabled, clones the marked DOALL kernels (see
// "graph.gpu.kernels" named metadata) into a dedicated device module, emits
// NVPTX PTX to PtxPath, and removes the temporary kernel placeholders from M.
void emitGpuKernels(llvm::Module &M, llvm::StringRef PtxPath);

#endif // PARALLEL_LOOP_OUTLINE_H