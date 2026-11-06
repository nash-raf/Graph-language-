#ifndef SGPL_PDG_BUFFER_LIFECYCLE_H
#define SGPL_PDG_BUFFER_LIFECYCLE_H

#include "pdg_memory_dependence.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IntrinsicInst.h"

namespace llvm {

// A state-version certificate, NOT a dependence-direction certificate. A
// suffix copy still has same-round WAR edges from the traversal to the copy.
// Untouched elements can retain initialization or older-round values. Neither
// full coverage nor an exact producer distance is asserted by this proof.
struct PDGBufferRoundProof {
    enum PublicationKind { Swap, Copy } Kind = Copy;
    const Value *Input = nullptr;
    const Value *Output = nullptr;
    SmallVector<const Instruction *, 4> Publications;
};

namespace pdg_buffer_detail {
struct Buffer {
    Value *Root = nullptr;
    Value *Slot = nullptr;
    Value *Initial = nullptr;
    StoreInst *Exchange = nullptr;
};

// Pointer slots must have one dominating initialization and no unaccounted
// accesses, including accesses from another function or an escaped address.
inline bool describe(Value *Root, Loop &Round, DominatorTree &DT, Buffer &B) {
    B.Root = Root;
    if (auto *L = dyn_cast<LoadInst>(Root)) {
        if (!L->isSimple() || !L->getType()->isPointerTy()) return false;
        B.Slot = L->getPointerOperand()->stripPointerCasts();
        auto *A = dyn_cast<AllocaInst>(B.Slot);
        auto *G = dyn_cast<GlobalVariable>(B.Slot);
        if ((!A || !A->getAllocatedType()->isPointerTy()) &&
            (!G || !G->hasLocalLinkage() || !G->getValueType()->isPointerTy()))
            return false;
        StoreInst *Init = nullptr;
        for (User *U : B.Slot->users()) {
            auto *I = dyn_cast<Instruction>(U);
            if (!I || I->getFunction() != L->getFunction()) return false;
            if (auto *R = dyn_cast<LoadInst>(I)) {
                if (!R->isSimple() || R->getPointerOperand() != B.Slot) return false;
            } else if (auto *S = dyn_cast<StoreInst>(I)) {
                if (!S->isSimple() || S->getPointerOperand() != B.Slot) return false;
                if (Round.contains(S)) {
                    if (B.Exchange) return false;
                    B.Exchange = S;
                } else {
                    if (Init) return false;
                    Init = S;
                }
            } else return false;
        }
        if (!Init || !DT.dominates(Init, Round.getHeader()->getTerminator()))
            return false;
        for (User *U : B.Slot->users())
            if (U != Init && !DT.dominates(Init, cast<Instruction>(U))) return false;
        B.Initial = getUnderlyingObject(Init->getValueOperand());
    } else {
        if (!Round.isLoopInvariant(Root)) return false;
        B.Initial = Root;
    }
    // Arbitrary loaded pointers are not allocation identities.
    return B.Initial && !isa<LoadInst>(B.Initial) && !isa<PHINode>(B.Initial) &&
           Round.isLoopInvariant(B.Initial);
}

inline bool matches(Value *Pointer, const Buffer &B) {
    Value *Root = getUnderlyingObject(Pointer);
    if (!B.Slot) return Root == B.Root;
    auto *L = dyn_cast<LoadInst>(Root);
    return L && L->isSimple() && L->getPointerOperand() == B.Slot;
}

inline bool dominatesLatches(BasicBlock *BB, Loop &L, DominatorTree &DT) {
    SmallVector<BasicBlock *, 4> Latches;
    L.getLoopLatches(Latches);
    if (Latches.empty()) return false;
    for (BasicBlock *Latch : Latches)
        if (!DT.dominates(BB, Latch)) return false;
    return true;
}

// A publication is after the complete traversal and on every continuing
// round path. A copy loop may have zero iterations; it then preserves state.
inline bool suffix(Instruction *I, Loop &Round, Loop &Traversal,
                   LoopInfo &LI, DominatorTree &DT) {
    BasicBlock *Exit = Traversal.getExitBlock();
    if (!Exit || !Round.contains(Exit) || Traversal.contains(I) ||
        !DT.dominates(Exit, I->getParent())) return false;
    Loop *Inner = LI.getLoopFor(I->getParent());
    if (Inner == &Round) return dominatesLatches(I->getParent(), Round, DT);
    if (!Inner || Inner->getParentLoop() != &Round) return false;
    return dominatesLatches(I->getParent(), *Inner, DT) &&
           dominatesLatches(Inner->getHeader(), Round, DT);
}

inline bool disjoint(Value *A, Value *B, AAResults &AA) {
    return AA.alias(MemoryLocation::getBeforeOrAfter(A),
                    MemoryLocation::getBeforeOrAfter(B)) == AliasResult::NoAlias;
}
} // namespace pdg_buffer_detail

// Prove that the input buffer remains unchanged from round entry through the
// traversal. Recognize reciprocal pointer-slot swaps, reciprocal header PHIs,
// and suffix copies (element stores or nonvolatile memory transfers). All
// other possible modifications are checked using whole-object AA/ModRef, as
// in the shared PDG proof. Unknown aliases/calls/lifetimes fail closed.
inline bool pdgProveBufferRoundRead(LoadInst &Read, Loop &Round, Loop &Traversal,
                                   LoopInfo &LI, AAResults &AA, DominatorTree &DT,
                                   PDGBufferRoundProof &Proof) {
    using namespace pdg_buffer_detail;
    Proof = PDGBufferRoundProof();
    if (!Read.isSimple() || !Traversal.contains(&Read) ||
        Traversal.getParentLoop() != &Round) return false;
    Buffer Input, Output;
    Value *Root = getUnderlyingObject(Read.getPointerOperand());
    Instruction *ExchangeStart = nullptr;
    bool Swap = false, PhiSwap = false;
    if (auto *P = dyn_cast<PHINode>(Root)) {
        BasicBlock *Pre = Round.getLoopPreheader(), *Latch = Round.getLoopLatch();
        if (!Pre || !Latch || P->getParent() != Round.getHeader() ||
            P->getNumIncomingValues() != 2) return false;
        auto *Q = dyn_cast<PHINode>(P->getIncomingValueForBlock(Latch));
        if (!Q || Q == P || Q->getParent() != P->getParent() ||
            Q->getNumIncomingValues() != 2 || Q->getIncomingValueForBlock(Latch) != P)
            return false;
        Input.Root = P; Output.Root = Q;
        Input.Initial = P->getIncomingValueForBlock(Pre);
        Output.Initial = Q->getIncomingValueForBlock(Pre);
        if (!Input.Initial || !Output.Initial ||
            !Round.isLoopInvariant(Input.Initial) || !Round.isLoopInvariant(Output.Initial))
            return false;
        Swap = PhiSwap = true;
    } else {
        if (!describe(Root, Round, DT, Input)) return false;
        if (Input.Exchange) {
            auto *PeerLoad = dyn_cast<LoadInst>(Input.Exchange->getValueOperand());
            if (!PeerLoad || !describe(PeerLoad, Round, DT, Output) ||
                !Output.Exchange || Input.Slot == Output.Slot) return false;
            auto *OldInput = dyn_cast<LoadInst>(Output.Exchange->getValueOperand());
            if (!OldInput || !matches(OldInput, Input)) return false;
            StoreInst *A = Input.Exchange, *B = Output.Exchange;
            if (A->getParent() != B->getParent() ||
                LI.getLoopFor(A->getParent()) != &Round ||
                !suffix(A, Round, Traversal, LI, DT)) return false;
            ExchangeStart = A->comesBefore(B) ? A : B;
            if (!DT.dominates(PeerLoad, ExchangeStart) ||
                !DT.dominates(OldInput, ExchangeStart)) return false;
            Proof.Publications.push_back(A); Proof.Publications.push_back(B);
            Swap = true;
        } else {
            // Find the second buffer from a suffix copy. All copies and all
            // other memory effects are validated below, not just this example.
            for (BasicBlock *BB : Round.blocks()) {
                for (Instruction &I : *BB) {
                    Value *Source = nullptr, *Dest = nullptr;
                    if (auto *S = dyn_cast<StoreInst>(&I)) {
                        auto *L = dyn_cast<LoadInst>(S->getValueOperand());
                        if (L && L->isSimple()) { Source = L->getPointerOperand(); Dest = S->getPointerOperand(); }
                    } else if (auto *M = dyn_cast<MemTransferInst>(&I)) {
                        Source = M->getSource(); Dest = M->getDest();
                    }
                    if (!Dest || !matches(Dest, Input) ||
                        !suffix(&I, Round, Traversal, LI, DT)) continue;
                    if (!describe(getUnderlyingObject(Source), Round, DT, Output) ||
                        Output.Exchange) return false;
                    break;
                }
                if (Output.Root) break;
            }
            if (!Output.Root) return false;
        }
    }
    if (!disjoint(Input.Initial, Output.Initial, AA)) return false;
    // For slot swaps every pointer load must refer to the pre-swap role. This
    // rejects cached pointers from earlier iterations and post-swap accesses.
    if (Swap && !PhiSwap) {
        for (const Buffer *B : {&Input, &Output})
            for (User *U : B->Slot->users())
                if (auto *L = dyn_cast<LoadInst>(U))
                    if (!Round.contains(L) || !DT.dominates(L, ExchangeStart)) {
                        // A load inside the traversal need not dominate its
                        // exit, but cannot occur after publication in this round.
                        if (!Round.contains(L) || !Traversal.contains(L)) return false;
                    }
    }
    // The validated slot/PHI lifecycle restricts both roles to these two
    // invariant objects. Query those objects, not opaque pointer reloads.
    SmallVector<Value *, 4> Protected = {Input.Initial, Output.Initial};
    if (Input.Slot) Protected.push_back(Input.Slot);
    if (Output.Slot) Protected.push_back(Output.Slot);
    bool OutputWritten = false;
    for (BasicBlock *BB : Round.blocks()) for (Instruction &I : *BB) {
        if (&I == Input.Exchange || &I == Output.Exchange) continue;
        if (auto *L = dyn_cast<LoadInst>(&I)) {
            if (!L->isSimple()) return false;
            continue;
        }
        Value *Dest = nullptr, *CopySource = nullptr;
        if (auto *S = dyn_cast<StoreInst>(&I)) {
            if (!S->isSimple()) return false;
            Dest = S->getPointerOperand();
            if (auto *L = dyn_cast<LoadInst>(S->getValueOperand()))
                if (L->isSimple()) CopySource = L->getPointerOperand();
        } else if (auto *M = dyn_cast<MemTransferInst>(&I)) {
            if (M->isVolatile()) return false;
            Dest = M->getDest(); CopySource = M->getSource();
        } else if (auto *M = dyn_cast<MemSetInst>(&I)) {
            if (M->isVolatile()) return false;
            Dest = M->getDest();
        }
        if (Dest) {
            if (matches(Dest, Input)) {
                if (Swap || !CopySource || !matches(CopySource, Output) ||
                    !suffix(&I, Round, Traversal, LI, DT)) return false;
                Proof.Publications.push_back(&I);
            } else if (matches(Dest, Output)) {
                if (ExchangeStart && !Traversal.contains(&I) && !DT.dominates(&I, ExchangeStart))
                    return false;
                OutputWritten = true;
            } else {
                for (Value *V : Protected) if (!disjoint(Dest, V, AA)) return false;
            }
            continue;
        }
        if (auto *C = dyn_cast<CallBase>(&I)) {
            if (C->isConvergent()) return false;
            if (auto *II = dyn_cast<IntrinsicInst>(C))
                if (II->getIntrinsicID() == Intrinsic::lifetime_start ||
                    II->getIntrinsicID() == Intrinsic::lifetime_end ||
                    II->getIntrinsicID() == Intrinsic::stackrestore) return false;
            for (Value *V : Protected)
                if (isModSet(AA.getModRefInfo(C, MemoryLocation::getBeforeOrAfter(V))))
                    return false;
        } else if (isa<FenceInst>(&I) || I.mayWriteToMemory()) return false;
    }
    if (!OutputWritten || (!Swap && Proof.Publications.empty())) return false;
    if (PhiSwap) {
        Proof.Publications.push_back(cast<Instruction>(Input.Root));
        Proof.Publications.push_back(cast<Instruction>(Output.Root));
    }
    Proof.Kind = Swap ? PDGBufferRoundProof::Swap : PDGBufferRoundProof::Copy;
    Proof.Input = Input.Root; Proof.Output = Output.Root;
    return true;
}

} // namespace llvm
#endif
