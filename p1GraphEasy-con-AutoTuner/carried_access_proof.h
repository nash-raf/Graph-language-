#ifndef SGPL_CARRIED_ACCESS_PROOF_H
#define SGPL_CARRIED_ACCESS_PROOF_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include <string>

namespace llvm {
struct SourceAddressProof {
    const Value *Base = nullptr;
    Type *Element = nullptr;
    std::string IndexMap;
};
struct CarriedAccessProof {
    DenseMap<const Instruction *, SourceAddressProof> SourceAddresses;
    DenseMap<const Instruction *, SmallVector<const Instruction *, 4>> Disjoint;
};

inline Value *carriedPointer(Instruction *I) {
    if (auto *L = dyn_cast_or_null<LoadInst>(I)) return L->getPointerOperand();
    if (auto *S = dyn_cast_or_null<StoreInst>(I)) return S->getPointerOperand();
    if (auto *A = dyn_cast_or_null<AtomicRMWInst>(I)) return A->getPointerOperand();
    if (auto *A = dyn_cast_or_null<AtomicCmpXchgInst>(I)) return A->getPointerOperand();
    return nullptr;
}

// Only lossless integer extensions preserve the source identity here.
inline Value *carriedSourceIndex(Value *V, std::string *Map = nullptr) {
    if (!V || !V->getType()->isIntegerTy()) return nullptr;
    if (Map) *Map = std::to_string(V->getType()->getIntegerBitWidth());
    while (auto *C = dyn_cast<CastInst>(V)) {
        if (C->getOpcode() != Instruction::SExt && C->getOpcode() != Instruction::ZExt)
            return V; // do not look through a lossy transformation of the binding
        if (Map) *Map += ":" + std::string(C->getOpcodeName()) + ":" +
                         std::to_string(C->getSrcTy()->getIntegerBitWidth());
        V = C->getOperand(0);
    }
    return V;
}

// Return a pointer value that is stable throughout this traversal. A reload
// from a private pointer slot is stable only when every possible modification
// is excluded. This is independent of the array's index provenance.
inline Value *carriedStableBase(Value *Root, Loop &Driver, AAResults &AA) {
    Root = Root->stripPointerCasts();
    if (Driver.isLoopInvariant(Root)) return Root;
    auto *L = dyn_cast<LoadInst>(Root);
    if (!L || !L->isSimple() || !L->getType()->isPointerTy()) return nullptr;
    Value *Slot = L->getPointerOperand();
    if (!Driver.isLoopInvariant(Slot)) return nullptr;
    auto *A = dyn_cast<AllocaInst>(Slot);
    auto *G = dyn_cast<GlobalVariable>(Slot);
    if (!A && (!G || !G->hasLocalLinkage())) return nullptr;
    for (User *U : Slot->users()) {
        if (auto *R = dyn_cast<LoadInst>(U)) {
            if (!R->isSimple() || R->getPointerOperand() != Slot) return nullptr;
        } else if (auto *S = dyn_cast<StoreInst>(U)) {
            if (!S->isSimple() || S->getPointerOperand() != Slot || Driver.contains(S)) return nullptr;
        } else return nullptr; // includes an escaped slot address
    }
    if (G) for (BasicBlock *BB : Driver.blocks()) for (Instruction &I : *BB)
        if (auto *C = dyn_cast<CallBase>(&I))
            if (isModSet(AA.getModRefInfo(C, MemoryLocation::getBeforeOrAfter(Slot)))) return nullptr;
    return Slot;
}

inline bool carriedExactSourceAddress(Instruction *I, Value *Source, Loop &Driver,
                                     AAResults &AA, SourceAddressProof &Proof) {
    if (!Source) return false;
    Value *Pointer = carriedPointer(I);
    auto *G = dyn_cast_or_null<GetElementPtrInst>(Pointer);
    if (!G) return false;
    Type *Element = G->getResultElementType();
    if (!Element->isIntegerTy() && !Element->isFloatingPointTy()) return false;
    // Accept A[u] and a leading zero for a statically sized array. Reject all
    // other dimensions, shifts, remainders and narrowed source expressions.
    if (G->getNumIndices() == 2) {
        auto *Zero = dyn_cast<ConstantInt>(G->getOperand(1));
        if (!G->getSourceElementType()->isArrayTy() || !Zero || !Zero->isZero()) return false;
    } else if (G->getNumIndices() != 1 || G->getSourceElementType() != Element) return false;
    if (carriedSourceIndex(G->getOperand(G->getNumOperands()-1), &Proof.IndexMap) != Source) return false;
    Type *Access = nullptr;
    if (auto *L = dyn_cast<LoadInst>(I)) Access = L->getType();
    if (auto *S = dyn_cast<StoreInst>(I)) Access = S->getValueOperand()->getType();
    if (Access != Element) return false; // no overlapping wide/mixed-width accesses
    const DataLayout &DL = I->getModule()->getDataLayout();
    auto Bytes = DL.getTypeAllocSize(Element);
    if (Bytes.isScalable()) return false;
    unsigned Bits = Source->getType()->getIntegerBitWidth();
    unsigned PointerBits = DL.getIndexTypeSizeInBits(Pointer->getType());
    // Without inbounds, exclude modular pointer wrap. This bound covers both
    // signed and unsigned extensions, including their full source ranges.
    unsigned ScaleBits = 0;
    for (uint64_t N = Bytes.getFixedValue(); N > 1; N = (N + 1) / 2) ++ScaleBits;
    if (Bits > PointerBits || (!G->isInBounds() && Bits + ScaleBits >= PointerBits)) return false;
    Proof.Base = carriedStableBase(G->getPointerOperand(), Driver, AA);
    Proof.Element = Element;
    return Proof.Base != nullptr;
}

inline bool carriedSourceSeparated(const CarriedAccessProof &Proof,
                                   const Instruction *A, const Instruction *B) {
    auto X = Proof.SourceAddresses.find(A), Y = Proof.SourceAddresses.find(B);
    return X != Proof.SourceAddresses.end() && Y != Proof.SourceAddresses.end() &&
           X->second.Base == Y->second.Base && X->second.Element == Y->second.Element &&
           X->second.IndexMap == Y->second.IndexMap;
}
inline bool carriedDisjoint(const CarriedAccessProof &Proof,
                            const Instruction *A, const Instruction *B) {
    auto It = Proof.Disjoint.find(A);
    return It != Proof.Disjoint.end() && llvm::is_contained(It->second, B);
}
} // namespace llvm
#endif
