/* graph_frontier_lowering.cpp — make CleanCut the only path for graph loops.
 *
 * General rule: any loop whose body uses a graph iterator
 * (autograph_neighbor_iter_init / autograph_neighbor_iter_next) is a "graph
 * loop".  Such loops are never handed to the racy DOALL/outliner path; the
 * frontier/motif engine at IRGen time gets first chance; everything else that
 * lowered to a neighbor-iterator nest is converted here into the
 * owner-computes CleanCut step, which is race-free for any per-pair work by
 * construction (destination home partitions; one worker per partition).
 *
 * Route A: this pass rewrites only the *inner* neighbor-iterator loops into
 * CleanCut step calls.  Outer round loops and array-copy loops stay as-is.
 *
 * Detected shape (verified on the real pre-outline IR):
 *   driver loop over u:
 *      ... per-u prefix computing values from u (e.g. contrib) ...
 *      init(graph, u, &iter)
 *   inner loop:
 *      %has = next(&iter, &v)
 *      %cmp = icmp ne %has, 0; br cond body/merge
 *      body: per-pair statements using u and v
 *
 * The rewrite (pair callback + build/step emission) is the next increment;
 * the detection and shape analysis below are complete and validated.
 */
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/Local.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "graph_frontier_lowering.h"

using namespace llvm;

/* ── iterator-call recognition ─────────────────────────────────── */

static bool isIteratorInit(const CallInst *CI)
{
    Function *F = CI ? CI->getCalledFunction() : nullptr;
    return F && F->getName() == "autograph_neighbor_iter_init";
}

static bool isIteratorNext(const CallInst *CI)
{
    Function *F = CI ? CI->getCalledFunction() : nullptr;
    return F && F->getName() == "autograph_neighbor_iter_next";
}

/* ── canonical-shape analysis ───────────────────────────────────── */

struct NeighborLoopInfo
{
    Loop *NeighborLoop = nullptr;
    Loop *DriverLoop = nullptr;
    CallInst *InitCall = nullptr;   /* autograph_neighbor_iter_init(...) */
    CallInst *NextCall = nullptr;   /* in the inner-loop exit compare */
    Value *VAlloca = nullptr;       /* loop-var v written by next */
    Value *GraphPtr = nullptr;      /* init(graph, ...) */
    Value *UVal = nullptr;          /* init(..., u) — the driver value */
};

static bool analyzeNeighborLoop(Loop *L, NeighborLoopInfo &Info)
{
    BasicBlock *Header = L->getHeader();
    if (!Header)
        return false;
    auto *BI = dyn_cast_or_null<BranchInst>(Header->getTerminator());
    if (!BI || !BI->isConditional())
        return false;
    auto *Cmp = dyn_cast<ICmpInst>(BI->getCondition());
    if (!Cmp)
        return false;
    CallInst *next = nullptr;
    for (Value *Op : Cmp->operands())
        if (auto *CI = dyn_cast<CallInst>(Op))
            if (isIteratorNext(CI))
            {
                next = CI;
                break;
            }
    if (!next || next->getNumOperands() < 2)
        return false;

    Info.NeighborLoop = L;
    Info.NextCall = next;
    Info.VAlloca = next->getArgOperand(1);
    Info.DriverLoop = L->getParentLoop();
    if (!Info.DriverLoop)
        return false; /* no driver: outside the canonical shape */

    /* Find the init call that feeds this iterator (same iterator pointer). */
    Function *F = L->getHeader()->getParent();
    for (BasicBlock &BB : *F)
        for (Instruction &I : BB)
            if (auto *CI = dyn_cast<CallInst>(&I))
                if (isIteratorInit(CI) &&
                    CI->getArgOperand(2) == next->getArgOperand(0))
                {
                    Info.InitCall = CI;
                    Info.GraphPtr = CI->getArgOperand(0);
                    Info.UVal = CI->getArgOperand(1);
                    return true;
                }
    return false;
}

/* ── pass ──────────────────────────────────────────────────────── */

PreservedAnalyses GraphFrontierLoweringPass::run(Function &F,
                                                 FunctionAnalysisManager &FAM)
{
    if (F.isDeclaration() || F.begin() == F.end())
        return PreservedAnalyses::all();
    unsigned detected = 0;
    auto &LI = FAM.getResult<LoopAnalysis>(F);
    SmallVector<Loop *, 16> Work;
    for (Loop *L : LI.getLoopsInPreorder())
        Work.push_back(L);
    for (Loop *L : Work)
    {
        NeighborLoopInfo Info;
        if (!analyzeNeighborLoop(L, Info))
            continue;
        if (getenv("GRAPH_FRONTIER_STATS"))
            errs() << "[graph-frontier] candidate: " << F.getName()
                   << " driver=" << Info.DriverLoop->getHeader()->getName()
                   << " inner=" << L->getHeader()->getName() << "\n";
        ++detected;
    }
    if (getenv("GRAPH_FRONTIER_STATS"))
        errs() << "[graph-frontier] detected=" << detected << "\n";
    return PreservedAnalyses::none();
}