#ifndef SGPL_PDG_MEMORY_DEPENDENCE_H
#define SGPL_PDG_MEMORY_DEPENDENCE_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

// Shared with the PDG builder. Same-iteration NoAlias must not remove a
// possible conflict between different iterations of a common loop.
inline bool pdgMemoryPairCandidate(Instruction *From, Instruction *To,
                                   LoopInfo &LI, AAResults &AA,
                                   DominatorTree &DT) {
    if (From == To || (isa<LoadInst>(From) && isa<LoadInst>(To)) ||
        !isPotentiallyReachable(From, To, nullptr, &DT, &LI))
        return false;
    for (Loop *L = LI.getLoopFor(From->getParent()); L; L = L->getParentLoop())
        if (L->contains(To->getParent()))
            return true;
    return AA.alias(MemoryLocation::get(From), MemoryLocation::get(To)) !=
           AliasResult::NoAlias;
}

// An unknown SCEV containing a load/call inside the loop is not a stable
// symbolic index across iterations. The PDG uses this guard too.
inline bool pdgIsLoopMemoryDerived(const Value *V, const Loop *L,
                                   SmallPtrSetImpl<const Value *> &Seen) {
    if (!V || !Seen.insert(V).second)
        return false;
    const auto *I = dyn_cast<Instruction>(V);
    if (!I || !L->contains(I->getParent()))
        return false;
    if (isa<LoadInst>(I) || isa<CallBase>(I))
        return true;
    for (const Use &U : I->operands())
        if (pdgIsLoopMemoryDerived(U.get(), L, Seen))
            return true;
    return false;
}

// Direction proof only. There is no inferred constant distance. Equality of
// all enclosing components is required before attributing the direction to L.
inline bool pdgStrictlyEarlierAtLoop(const Dependence &D, const Loop &L) {
    if (D.isConfused() || D.isLoopIndependent() || !D.isFlow() ||
        D.getLevels() < L.getLoopDepth())
        return false;
    for (unsigned Level = 1; Level < L.getLoopDepth(); ++Level)
        if (D.getDirection(Level) != Dependence::DVEntry::EQ)
            return false;
    return D.getDirection(L.getLoopDepth()) == Dependence::DVEntry::LT;
}

// Prove a conservative relation, not a unique reaching definition: every
// possible write->read dependence in Round must come from an earlier Round
// iteration. Initial state and the last writer's exact age are not inferred.
// Scan the whole round, including preamble, epilogue, aliases and call effects.
// Empty evidence, unknown directions and opaque effects cannot certify a read.
inline bool pdgProveEarlierRoundRead(LoadInst &Read, Loop &Round,
                                     DependenceInfo &DI, LoopInfo &LI,
                                     AAResults &AA, DominatorTree &DT,
                                     SmallVectorImpl<const Instruction *> &Writers) {
    Writers.clear();
    if (!Read.isSimple() || !Round.contains(&Read))
        return false;
    SmallPtrSet<const Value *, 16> Seen;
    if (pdgIsLoopMemoryDerived(Read.getPointerOperand(), &Round, Seen))
        return false;
    SmallVector<const Instruction *, 4> Proven;
    for (BasicBlock *BB : Round.blocks())
        for (Instruction &I : *BB) {
            if (auto *Store = dyn_cast<StoreInst>(&I)) {
                if (!Store->isSimple())
                    return false;
                if (!pdgMemoryPairCandidate(Store, &Read, LI, AA, DT))
                    continue;
                auto D = DI.depends(Store, &Read, /*PossiblyLoopIndependent=*/true);
                if (!D)
                    continue;
                Seen.clear();
                if (pdgIsLoopMemoryDerived(Store->getPointerOperand(), &Round, Seen) ||
                    !pdgStrictlyEarlierAtLoop(*D, Round))
                    return false;
                Proven.push_back(Store);
                continue;
            }
            if (auto *Load = dyn_cast<LoadInst>(&I)) {
                if (!Load->isSimple())
                    return false;
                continue;
            }
            if (auto *Call = dyn_cast<CallBase>(&I)) {
                // Declared effects/AA may prove that a call cannot change this
                // object. A query about just this iteration's element could
                // miss a call's writes in other iterations. No runtime name
                // is trusted as evidence.
                const Value *Object = getUnderlyingObject(Read.getPointerOperand());
                if (Call->isConvergent() ||
                    !Round.isLoopInvariant(Object) ||
                    isModSet(AA.getModRefInfo(Call, MemoryLocation::getBeforeOrAfter(Object))))
                    return false;
                continue;
            }
            if (isa<FenceInst>(&I) || isa<AtomicRMWInst>(&I) ||
                isa<AtomicCmpXchgInst>(&I) || I.mayWriteToMemory())
                return false;
        }
    if (Proven.empty())
        return false;
    Writers.append(Proven.begin(), Proven.end());
    return true;
}

} // namespace llvm
#endif
