#ifndef PARALLEL_LOOP_OUTLINE_H
#define PARALLEL_LOOP_OUTLINE_H

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

void registerLoopOutlinerPluginWithPassBuilder(llvm::PassBuilder &PB);
void runLoopOutlinerOnModule(llvm::Module &M);
void registerLoopOutlinerPass(llvm::FunctionPassManager &FPM);

#endif // PARALLEL_LOOP_OUTLINE_H
