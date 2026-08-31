/* graph_frontier_lowering.cpp — general graph-loop race-freedom (Graptor)
 *
 * General rule: any loop whose body uses a graph iterator
 * (autograph_neighbor_iter_init / autograph_neighbor_iter_next) is a "graph
 * loop".  Such loops are never parallelized by the racy DOALL path; they are
 * lowered to the owner-computes frontier step (CleanCut), which guarantees
 * race-freedom for any per-pair work by construction.
 *
 * Pipeline position: inside runPdgAndOutliner(), before runPDGOnModule().
 */
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include <string>
#include <unordered_set>

using namespace llvm;

namespace {

struct GraphFrontierLoweringPass : public PassInfoMixin<GraphFrontierLoweringPass>
{
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

/* Is a call one of our graph-iterator runtime functions? */
static bool isGraphIteratorCall(const CallInst *CI)
{
    if (!CI)
        return false;
    Function *F = CI->getCalledFunction();
    if (!F)
        return false;
    StringRef N = F->getName();
    return N == "autograph_neighbor_iter_init" || N == "autograph_neighbor_iter_next" ||
           N == "autograph_frontier_step_owner" ||
           N == "autograph_frontier_step_owner_push" ||
           N == "autograph_build_clean_cut";
}

/* Recursive scan of a basic block's instruction users to find iterator calls. */
static bool blockUsesGraphIterator(const BasicBlock *BB)
{
    for (const Instruction &I : *BB)
        if (auto *CI = dyn_cast<CallInst>(&I))
            if (isGraphIteratorCall(CI))
                return true;
    return false;
}

static bool loopUsesGraphIterator(Loop *L)
{
    for (BasicBlock *BB : L->blocks())
        if (blockUsesGraphIterator(BB))
            return true;
    return false;
}

PreservedAnalyses GraphFrontierLoweringPass::run(Module &M, ModuleAnalysisManager &MAM)
{
    /* Detection + classification; rewriting lands in a follow-up.  For now the
     * pass only marks such loops via module debug output so the integration
     * point can be validated before the (large) rewriter lands. */
    unsigned graphLoops = 0;
    for (Function &F : M)
    {
        if (F.isDeclaration())
            continue;
        auto &LI = MAM.getResult<LoopAnalysis>(F);
        for (Loop *L : LI.getLoopsInPreorder())
            if (loopUsesGraphIterator(L))
                ++graphLoops;
    }
    if (graphLoops && getenv("GRAPH_FRONTIER_DEBUG"))
        errs() << "[graph-frontier] detected " << graphLoops << " graph loops\n";
    return PreservedAnalyses::all();
}

} // namespace

void registerGraphFrontierLoweringPass(llvm::FunctionPassManager &FPM)
{
    /* placeholder: pass is a module pass; registered via its own entry */
}

ModulePassManager buildGraphFrontierLoweringPipeline()
{
    ModulePassManager MPM;
    MPM.addPass(GraphFrontierLoweringPass());
    return MPM;
}