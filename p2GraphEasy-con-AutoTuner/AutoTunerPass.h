/*
 * AutoTunerPass.h — Phase-Aware Graph Layout AutoTuner LLVM Pass
 *
 * Registers a ModulePass that:
 *   1. Detects operation-dominant regions in main()
 *   2. Computes optimal layouts via a graph-size-aware cost model
 *   3. Solves the global layout scheduling DP
 *   4. Injects guarded autograph_ensure_layout() calls at boundaries
 */

#ifndef AUTOTUNERPASS_H
#define AUTOTUNERPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"

/// The AutoTuner module pass — ties all 4 stages together.
struct AutoTunerModulePass : public llvm::PassInfoMixin<AutoTunerModulePass> {
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &MAM);
};

/// Register the AutoTuner pass with a PassBuilder instance.
void registerAutoTunerPass(llvm::PassBuilder &PB);

/// Standalone entry for running AutoTuner on a module.
void runAutoTunerOnModule(llvm::Module &M);

#endif /* AUTOTUNERPASS_H */
