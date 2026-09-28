#ifndef PARALLEL_LOOP_OUTLINE_H
#define PARALLEL_LOOP_OUTLINE_H

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

// One profile-id space, shared by the loop outliner and the graph-frontier
// lowering: the runtime indexes its per-site TDG/profile state (and the level
// pool entries) by this id, so every parallel site -- an outlined loop or a
// lowered engine step -- must be allocated a distinct id.  Ids are allocated,
// never guessed from names or shapes.
inline unsigned nextParallelSiteId()
{
    static unsigned Next = 1;
    return Next++;
}

void registerLoopOutlinerPluginWithPassBuilder(llvm::PassBuilder &PB);
void runLoopOutlinerOnModule(llvm::Module &M);
void registerLoopOutlinerPass(llvm::FunctionPassManager &FPM);

// If GPU backend is enabled, clones the marked DOALL kernels (see
// "graph.gpu.kernels" named metadata) into a dedicated device module, emits
// NVPTX PTX to PtxPath, and removes the temporary kernel placeholders from M.
void emitGpuKernels(llvm::Module &M, llvm::StringRef PtxPath);

#endif // PARALLEL_LOOP_OUTLINE_H