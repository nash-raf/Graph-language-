#ifndef GRAPH_FRONTIER_LOWERING_H
#define GRAPH_FRONTIER_LOWERING_H

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>

/* Graph-loop race-freedom pass: converts loops that use graph iterators
 * (autograph_neighbor_iter_*) into the owner-computes CleanCut frontier step.
 * Runs inside runPdgAndOutliner() before the PDG, so graph loops never reach
 * the racy DOALL path. */
struct GraphFrontierLoweringPass
    : public llvm::PassInfoMixin<GraphFrontierLoweringPass>
{
    llvm::PreservedAnalyses run(llvm::Function &F,
                                llvm::FunctionAnalysisManager &FAM);
};

#endif // GRAPH_FRONTIER_LOWERING_H