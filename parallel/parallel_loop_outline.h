// parallel_loop_outline.h
#ifndef PARALLEL_LOOP_OUTLINE_H
#define PARALLEL_LOOP_OUTLINE_H

#include "llvm/IR/PassManager.h"

/// Register the LoopOutlinerPass on a FunctionPassManager.
/// Implemented in parallel_loop_outline.cpp (defined after the pass type).
void registerLoopOutlinerPass(llvm::FunctionPassManager &FPM);
void runLoopOutlinerOnModule(llvm::Module &M);
void registerLoopOutlinerPluginWithPassBuilder(llvm::PassBuilder &PB);

#endif // PARALLEL_LOOP_OUTLINE_H
