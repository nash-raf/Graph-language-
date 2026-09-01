/* graph_frontier_lowering.cpp — make CleanCut the only path for graph loops.
 *
 * General rule: any loop whose body uses a graph iterator
 * (autograph_neighbor_iter_init / autograph_neighbor_iter_next) is a "graph
 * loop".  Such loops are never blindly handed to the racy DOALL/outliner
 * path: the compiler classifies the loop by its *write pattern against the
 * walked edge's endpoints* and lowers it to an owner-computes CleanCut step
 * (destination home partitions; one worker per partition — race-free for any
 * per-pair work by construction), or conservatively marks it sequential.
 *
 * Shapes handled (all currently use the forward out-edge iterator, i.e.
 * push: src = driver axis, dst = walked neighbor):
 *   - write indexed by dst  (prop[v] += ..)       -> dest-owned step
 *   - write indexed by src  (out_degree[u]++)     -> source-owned step
 *   - scalar/third-array accumulation (sum += ..) -> red step (per-partition
 *     partials + combine)
 *   - read/write overlap on one array (in-place dist[v] = min(dist[v], ..))
 *     -> round separation (buffer swap) when constructible, else sequential
 *   - mixed dst+src writes or unclassifiable -> sequential (never DOALL)
 *
 * The per-pair body is cloned by hand into the sgpl_frontier_pair_fn ABI
 * (no CodeExtractor: single-block bodies were out of reach and its
 * leave-behind loop reshaping only broke later passes).
 */
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

/* ── canonical-shape analysis ──────────────────────────────────── */

/* Recognized associative-commutative update operators for the reduction
 * (owner_red partials+combine) path.  Float vs int is chosen from the element
 * type at combine time, so one op covers both (e.g. Add == add/fadd). */
enum class RedOp
{
    None = 0,
    Add,
    Sub,
    Mul,
    Min,
    Max,
    And,
    Or,
    Xor,
    FirstWins
};

struct NeighborLoopInfo
{
    Loop *NeighborLoop = nullptr;
    Loop *DriverLoop = nullptr;
    CallInst *InitCall = nullptr;   /* autograph_neighbor_iter_init(...) */
    CallInst *NextCall = nullptr;   /* in the inner-loop exit compare */
    Value *VAlloca = nullptr;       /* loop-var v written by next */
    Value *GraphPtr = nullptr;      /* init(graph, ...) */
    Value *UVal = nullptr;          /* init(..., u) — the driver value */
    Value *UAlloca = nullptr;       /* scalar slot holding u (foreach-set) */
    enum WriteKind_e
    {
        WriteUnknown = 0, /* no vertex-indexed write found */
        WriteV = 1,       /* writes array indexed by v (dest-owned) */
        WriteU = 2,       /* writes array indexed by u (source-owned) */
        WriteMixed = 3,   /* writes both u- and v-indexed arrays — sequential */
        WriteData = 4     /* writes through a graph-data-derived index — sequential */
    } WriteKind = WriteUnknown;

    /* Any store in the body whose index is load-derived from a graph data array
     * element (nodes[i], perm[v], ...) — forces conservative sequential. */
    bool HasDataWrite = false;

    /* Reduction: the scalar the body accumulates into (alloca/global/array
     * element whose index is neither u nor v) and the update operator. */
    Value *ReducePtr = nullptr;
    RedOp ReduceOp = RedOp::None;

    /* In-place: an array both read and written on an endpoint region — a
     * round-separation (in-place) overlap.  Informational; for single-endpoint
     * (owned) writes the owner-computes step is race-free regardless, so this
     * no longer forces sequential (see classify()). */
    bool NeedsRoundSep = false;

    /* Frontier append (BFS-style): the body pushes newly-claimed vertices into
     * an array via a scalar counter, `next_frontier[next_size++] = v`.  The
     * counter is not a reduction; the append is chained through the runtime
     * frontier envelope (dest_seen/next_frontier) — see emit's frontier path. */
    bool HasFrontierAppend = false;
    Value *AppendArrayPtr = nullptr;  /* base slot of next_frontier */
    Value *AppendCountPtr = nullptr;  /* scalar slot of next_size */
    Value *FrontierArrayPtr = nullptr;/* base slot of frontier (driver source) */
    Value *FrontierSizePtr = nullptr; /* scalar slot of frontier_size */
    Value *FrontierSizeVal = nullptr; /* SSA size (when mem2reg'd, no slot) */
    Value *FrontierSetPtr = nullptr;  /* alloca of roaring frontier set */
    Value *NextSetPtr = nullptr;      /* alloca of roaring next set */
    bool HasFirstWins = false;        /* dest-owned CAS-style claim */
    bool HasRecognizedOp = false;     /* body has a known Update operator */
    SmallVector<StoreInst *, 4> DriverUStores; /* per-source preamble (alive[u]=0) */
};

/* Does this load read one element out of a data array (base = a loaded pointer
 * or a global array), as opposed to a scalar slot?  A graph data array element
 * lowers to `load elem, ptr <GEP base, idx>`; the iterator's scalar v-slot
 * lowers to `load i32, ptr %v_alloca` — no element GEP — and is excluded. */
static bool isArrayElementLoad(const LoadInst *LI)
{
    return LI && isa<GetElementPtrInst>(LI->getPointerOperand());
}

/* ── provenance domain ──────────────────────────────────────────
 * Region lattice for the *origin* of an index value:
 *   Bottom  constant / loop-invariant / neutral
 *   U       flows from the driver induction variable (source axis)
 *   V       flows from the iterator v-slot (destination / walked neighbor)
 *   D       flows from a graph data-array element load (nodes[i], perm[v], ...)
 *   Top     mixed / unknown → conservatively sequential
 *
 * This replaces the old depth-bounded `derivedFrom` back-walk with a sound
 * forward dataflow.  mem2reg has already run before this pass (main.cpp), so
 * index chains are SSA except the escaping v-slot alloca (its address is passed
 * to autograph_neighbor_iter_next), which is handled by an explicit load rule.
 * A `Top` origin in any store subscript forces the loop sequential, so the
 * domain is always at least as conservative as the code it replaces. */
enum class Region : uint8_t { Bottom = 0, U, V, D, Top };

static Region joinRegion(Region A, Region B)
{
    if (A == B)
        return A;
    if (A == Region::Bottom)
        return B;
    if (B == Region::Bottom)
        return A;
    return Region::Top; /* two distinct non-bottom origins */
}

class Provenance
{
    DenseMap<Value *, Region> M;
    Value *VSlot;                   /* iterator v-slot alloca (loads → V) */
    Value *USlot;                   /* scalar u slot (foreach-set element) */
    SmallPtrSet<Value *, 4> USeeds; /* driver induction phi and u SSA (→ U) */

    Region lookup(Value *V) const
    {
        if (USeeds.count(V))
            return Region::U;
        if (isa<Constant>(V) || isa<Argument>(V))
            return Region::Bottom;
        auto It = M.find(V);
        return It == M.end() ? Region::Bottom : It->second;
    }

    Region transfer(Instruction *I) const
    {
        if (USeeds.count(I))
            return Region::U;
        if (auto *LI = dyn_cast<LoadInst>(I))
        {
            if (LI->getPointerOperand() == VSlot)
                return Region::V;
            if (USlot && LI->getPointerOperand() == USlot)
                return Region::U;
            if (isArrayElementLoad(LI))
                return Region::D;
            return Region::Top;
        }
        if (isa<PHINode>(I) || isa<BinaryOperator>(I) || isa<CastInst>(I) ||
            isa<GetElementPtrInst>(I) || isa<SelectInst>(I))
        {
            Region R = Region::Bottom;
            for (Value *Op : I->operands())
                R = joinRegion(R, lookup(Op));
            return R;
        }
        return Region::Top;
    }

public:
    Provenance(Function &F, Value *VSlotIn, ArrayRef<Value *> USeedsIn,
               Value *USlotIn = nullptr)
        : VSlot(VSlotIn), USlot(USlotIn)
    {
        for (Value *S : USeedsIn)
            if (S)
                USeeds.insert(S);
        bool Changed = true;
        while (Changed)
        {
            Changed = false;
            for (BasicBlock &BB : F)
                for (Instruction &I : BB)
                {
                    Region R = transfer(&I);
                    Region &Slot = M[&I];
                    if (R != Slot)
                    {
                        Slot = R;
                        Changed = true;
                    }
                }
        }
    }

    Region regionOf(Value *V) const { return lookup(V); }
};

/* Canonical identity of the array a GEP indexes, robust to CSE.  When the base
 * pointer is `load X` (the common `load %arr_slot` then index shape), key on the
 * loaded-from pointer `X` so the read and the write of the same logical array
 * map to the same identity even when their base loads are distinct SSA values.
 * Otherwise fall back to the underlying object of the base. */
static const Value *canonicalArrayBase(const GetElementPtrInst *GEP)
{
    const Value *Base = GEP->getPointerOperand();
    if (auto *BL = dyn_cast<LoadInst>(Base))
        return getUnderlyingObject(BL->getPointerOperand());
    return getUnderlyingObject(Base);
}

/* True if A and B address the same array element, even when they are distinct
 * GEP instructions (the common non-CSE'd `load slot; gep; load/store` shape). */
static bool sameIndexVal(Value *A, Value *B)
{
    if (A == B)
        return true;
    auto *LA = dyn_cast<LoadInst>(A);
    auto *LB = dyn_cast<LoadInst>(B);
    return LA && LB && LA->getPointerOperand() == LB->getPointerOperand();
}

static bool sameArraySlot(Value *A, Value *B)
{
    if (A == B)
        return true;
    auto *GA = dyn_cast<GetElementPtrInst>(A);
    auto *GB = dyn_cast<GetElementPtrInst>(B);
    if (!GA || !GB)
        return false;
    if (canonicalArrayBase(GA) != canonicalArrayBase(GB))
        return false;
    if (GA->getNumIndices() != GB->getNumIndices())
        return false;
    auto Ia = GA->idx_begin(), Ib = GB->idx_begin();
    for (; Ia != GA->idx_end(); ++Ia, ++Ib)
        if (!sameIndexVal(*Ia, *Ib))
            return false;
    return true;
}

/* Recognize the associative-commutative update operator of a reduction store
 * `*RedPtr = f(load RedPtr, x)` — arithmetic/bitwise via BinaryOperator,
 * min/max via the llvm min/max intrinsics or an `icmp`+`select` idiom.  Returns
 * RedOp::None if the stored value is not a self-update through a known op. */
static RedOp detectScalarRedOp(Value *Stored, Value *RedPtr)
{
    auto loadsRed = [&](Value *V) -> bool
    {
        auto *LI = dyn_cast<LoadInst>(V);
        return LI && sameArraySlot(LI->getPointerOperand(), RedPtr);
    };

    if (auto *BO = dyn_cast<BinaryOperator>(Stored))
    {
        if (!loadsRed(BO->getOperand(0)) && !loadsRed(BO->getOperand(1)))
            return RedOp::None;
        switch (BO->getOpcode())
        {
        case Instruction::Add:
        case Instruction::FAdd:
            return RedOp::Add;
        case Instruction::Sub:
        case Instruction::FSub:
            return RedOp::Sub;
        case Instruction::Mul:
        case Instruction::FMul:
            return RedOp::Mul;
        case Instruction::And:
            return RedOp::And;
        case Instruction::Or:
            return RedOp::Or;
        case Instruction::Xor:
            return RedOp::Xor;
        default:
            return RedOp::None;
        }
    }

    if (auto *II = dyn_cast<IntrinsicInst>(Stored))
    {
        bool involvesRed = false;
        for (Value *A : II->args())
            if (loadsRed(A))
                involvesRed = true;
        if (!involvesRed)
            return RedOp::None;
        switch (II->getIntrinsicID())
        {
        case Intrinsic::smin:
        case Intrinsic::umin:
        case Intrinsic::minnum:
        case Intrinsic::minimum:
            return RedOp::Min;
        case Intrinsic::smax:
        case Intrinsic::umax:
        case Intrinsic::maxnum:
        case Intrinsic::maximum:
            return RedOp::Max;
        default:
            return RedOp::None;
        }
    }

    /* min/max as `select(icmp pred a, b, a, b)`. */
    if (auto *Sel = dyn_cast<SelectInst>(Stored))
    {
        auto *Cmp = dyn_cast<ICmpInst>(Sel->getCondition());
        if (!Cmp)
            return RedOp::None;
        Value *A = Cmp->getOperand(0), *B = Cmp->getOperand(1);
        Value *T = Sel->getTrueValue(), *Fv = Sel->getFalseValue();
        if (!(loadsRed(A) || loadsRed(B) || loadsRed(T) || loadsRed(Fv)))
            return RedOp::None;
        bool tIsA = (T == A && Fv == B);
        bool tIsB = (T == B && Fv == A);
        if (!tIsA && !tIsB)
            return RedOp::None;
        ICmpInst::Predicate P = Cmp->getPredicate();
        bool isLT = ICmpInst::isLT(P) || ICmpInst::isLE(P);
        bool isGT = ICmpInst::isGT(P) || ICmpInst::isGE(P);
        if (!isLT && !isGT)
            return RedOp::None;
        if (tIsA)
            return isLT ? RedOp::Min : RedOp::Max;
        return isLT ? RedOp::Max : RedOp::Min;
    }

    return RedOp::None;
}

/* Recognize a min/max reduction expressed as a guarded store rather than a
 * select: `if (X <cmp> *RedPtr) *RedPtr = X;`.  `SI` is the guarded store; its
 * block must have a single conditional predecessor whose icmp compares the
 * stored value against the current `*RedPtr`.  Predicate + branch direction +
 * operand side decide Min vs Max. */
static RedOp detectConditionalMinMax(StoreInst *SI, Value *RedPtr)
{
    BasicBlock *BT = SI->getParent();
    BasicBlock *BP = BT->getSinglePredecessor();
    if (!BP)
        return RedOp::None;
    auto *Br = dyn_cast<BranchInst>(BP->getTerminator());
    if (!Br || !Br->isConditional())
        return RedOp::None;
    auto *Cmp = dyn_cast<ICmpInst>(Br->getCondition());
    if (!Cmp)
        return RedOp::None;
    bool takenTrue;
    if (Br->getSuccessor(0) == BT)
        takenTrue = true;
    else if (Br->getSuccessor(1) == BT)
        takenTrue = false;
    else
        return RedOp::None;

    auto isLoadOf = [&](Value *V, Value *P) -> bool
    {
        auto *L = dyn_cast<LoadInst>(V);
        return L && sameArraySlot(L->getPointerOperand(), P);
    };
    Value *A = Cmp->getOperand(0), *B = Cmp->getOperand(1);
    Value *X = nullptr;
    bool xIsOp0 = false;
    if (isLoadOf(A, RedPtr)) /* current value is op0 -> X is op1 */
    {
        X = B;
        xIsOp0 = false;
    }
    else if (isLoadOf(B, RedPtr)) /* current value is op1 -> X is op0 */
    {
        X = A;
        xIsOp0 = true;
    }
    else
        return RedOp::None;

    auto equalish = [&](Value *a, Value *b) -> bool
    {
        if (a == b)
            return true;
        auto *la = dyn_cast<LoadInst>(a);
        auto *lb = dyn_cast<LoadInst>(b);
        return la && lb && la->getPointerOperand() == lb->getPointerOperand();
    };
    if (!equalish(SI->getValueOperand(), X))
        return RedOp::None;

    /* Predicate of "X <cmp> current". */
    ICmpInst::Predicate P = Cmp->getPredicate();
    if (!xIsOp0)
        P = ICmpInst::getSwappedPredicate(P);
    if (!takenTrue)
        P = ICmpInst::getInversePredicate(P);
    if (ICmpInst::isLT(P) || ICmpInst::isLE(P))
        return RedOp::Min; /* store X when X < current */
    if (ICmpInst::isGT(P) || ICmpInst::isGE(P))
        return RedOp::Max; /* store X when X > current */
    return RedOp::None;
}

/* First-wins claim: `if (A[i] == expected) A[i] = desired;` with expected !=
 * desired.  Dest-owned, so the claim is race-free under owner-computes (CSR
 * source order decides the winner). */
static bool detectFirstWinsStore(StoreInst *SI)
{
    Value *Ptr = SI->getPointerOperand();
    Value *Desired = SI->getValueOperand();
    BasicBlock *BT = SI->getParent();
    BasicBlock *BP = BT->getSinglePredecessor();
    if (!BP)
        return false;
    auto *Br = dyn_cast<BranchInst>(BP->getTerminator());
    if (!Br || !Br->isConditional() || Br->getSuccessor(0) != BT)
        return false;
    auto *Cmp = dyn_cast<ICmpInst>(Br->getCondition());
    if (!Cmp || Cmp->getPredicate() != ICmpInst::ICMP_EQ)
        return false;
    auto isLoadOfPtr = [&](Value *V) -> bool
    {
        auto *L = dyn_cast<LoadInst>(V);
        return L && sameArraySlot(L->getPointerOperand(), Ptr);
    };
    if (isLoadOfPtr(Cmp->getOperand(0)))
        return Cmp->getOperand(1) != Desired;
    if (isLoadOfPtr(Cmp->getOperand(1)))
        return Cmp->getOperand(0) != Desired;
    return false;
}

/* The driver loop's induction variable (its header phi), when present. */
static Value *driverIndVar(Loop *DriverLoop)
{
    if (!DriverLoop || !DriverLoop->getHeader())
        return nullptr;
    for (Instruction &I : *DriverLoop->getHeader())
        if (auto *Phi = dyn_cast<PHINode>(&I))
            return Phi;
    return nullptr;
}

/* Conservative sequential: the PDG classifier reads this on the loop header
 * terminator and hard-classifies the loop SEQUENTIAL (never DOALL/DOACROSS).
 * The motif engine is untouched — it already had its chance before PDG.
 * The (rewritten) loop is at the top of a round nest: mark the loop, all its
 * ancestors and every subloop (beta-init / swap / driver nests) sequential so
 * the reconstruction cannot GPU-ify or DOALL any part of the round. */
static void markSequential(Loop *L)
{
    if (!L)
        return;
    Loop *Top = L;
    while (Loop *P = Top->getParentLoop())
        Top = P;
    std::vector<Loop *> Work{Top};
    while (!Work.empty())
    {
        Loop *Cur = Work.back();
        Work.pop_back();
        if (Instruction *T = Cur->getHeader()->getTerminator())
            T->setMetadata(
                "sgpl.frontier.nested.sequential",
                MDNode::get(T->getContext(),
                            MDString::get(T->getContext(), "sequential")));
        for (Loop *Sub : Cur->getSubLoops())
            Work.push_back(Sub);
    }
}

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
                    break;
                }

    /* Build the loop's effect set through the provenance domain.  Seeds: the
     * v-slot alloca (loads → V) and the driver induction phi (→ U).  Every
     * store subscript is classified by the origin of its index; scalar stores
     * are reduction candidates; a written array that is also read endpoint-
     * indexed is a round-separation (in-place) overlap. */
    Value *IndVar = driverIndVar(Info.DriverLoop);
    Value *USrc = Info.UVal;
    while (USrc && isa<CastInst>(USrc))
        USrc = cast<CastInst>(USrc)->getOperand(0);
    Value *USlot = nullptr;
    if (auto *UL = dyn_cast_or_null<LoadInst>(USrc))
        if (!isa<GetElementPtrInst>(UL->getPointerOperand()))
            USlot = UL->getPointerOperand();
    Info.UAlloca = USlot;
    Provenance Prov(*F, Info.VAlloca, {IndVar, USrc}, USlot);

    /* Pre-pass: find scalar slots used as non-endpoint array-write subscripts
     * (frontier-append counters, `next_frontier[next_size] = v`).  These are
     * not reductions; record the append array + counter for the frontier
     * envelope, and exclude the counter from reduction detection below. */
    std::unordered_set<Value *> IndexSlots;
    for (BasicBlock *BB : L->blocks())
        for (Instruction &I : *BB)
            if (auto *SI = dyn_cast<StoreInst>(&I))
                if (auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
                    for (Value *IX : GEP->indices())
                    {
                        if (isa<Constant>(IX))
                            continue;
                        Region R = Prov.regionOf(IX);
                        if (R == Region::U || R == Region::V || R == Region::D)
                            continue;
                        if (auto *LI = dyn_cast<LoadInst>(IX))
                        {
                            IndexSlots.insert(LI->getPointerOperand());
                            if (!Info.AppendCountPtr)
                            {
                                Info.AppendCountPtr = LI->getPointerOperand();
                                Value *GBase = GEP->getPointerOperand();
                                if (auto *BL = dyn_cast<LoadInst>(GBase))
                                    Info.AppendArrayPtr = BL->getPointerOperand();
                                else
                                    Info.AppendArrayPtr = GBase;
                            }
                        }
                    }
    if (Info.AppendCountPtr)
        for (BasicBlock *BB : L->blocks())
            for (Instruction &I : *BB)
                if (auto *SI = dyn_cast<StoreInst>(&I))
                    if (SI->getPointerOperand() == Info.AppendCountPtr)
                        if (auto *BO = dyn_cast<BinaryOperator>(SI->getValueOperand()))
                            if (BO->getOpcode() == Instruction::Add)
                                Info.HasFrontierAppend = true;
    /* Set-based frontier: `next.add(v)` is roaring_bitmap_add. */
    for (BasicBlock *BB : L->blocks())
        for (Instruction &I : *BB)
            if (auto *CI = dyn_cast<CallInst>(&I))
                if (Function *CF = CI->getCalledFunction())
                    if (CF->getName() == "roaring_bitmap_add" &&
                        CI->arg_size() >= 2)
                    {
                        Info.HasFrontierAppend = true;
                        if (auto *BL = dyn_cast<LoadInst>(CI->getArgOperand(0)))
                            Info.NextSetPtr = BL->getPointerOperand();
                    }
    /* Frontier source arrays for the envelope: the driver reads
     * `v = frontier[i]` (init's u operand) and loops while `i < frontier_size`. */
    if (auto *ULoad = dyn_cast<LoadInst>(USrc))
        if (auto *UGEP = dyn_cast<GetElementPtrInst>(ULoad->getPointerOperand()))
        {
            Value *Base = UGEP->getPointerOperand();
            if (auto *FBL = dyn_cast<LoadInst>(Base))
                Info.FrontierArrayPtr = FBL->getPointerOperand();
            else
                Info.FrontierArrayPtr = Base;
        }
    /* Fallback: UVal is a load of a scalar copy (`int v = frontier[i]`). */
    if (!Info.FrontierArrayPtr)
        if (auto *UL = dyn_cast<LoadInst>(USrc))
            for (User *U : UL->getPointerOperand()->users())
                if (auto *SI = dyn_cast<StoreInst>(U))
                    if (auto *SrcL = dyn_cast<LoadInst>(SI->getValueOperand()))
                        if (auto *UGEP =
                                dyn_cast<GetElementPtrInst>(SrcL->getPointerOperand()))
                        {
                            Value *Base = UGEP->getPointerOperand();
                            if (auto *FBL = dyn_cast<LoadInst>(Base))
                                Info.FrontierArrayPtr = FBL->getPointerOperand();
                            else
                                Info.FrontierArrayPtr = Base;
                        }
    if (BasicBlock *DH = Info.DriverLoop->getHeader())
        if (auto *DBI = dyn_cast<BranchInst>(DH->getTerminator()))
            if (DBI->isConditional())
                if (auto *DCmp = dyn_cast<ICmpInst>(DBI->getCondition()))
                {
                    Value *Ind = driverIndVar(Info.DriverLoop);
                    for (Value *Op : DCmp->operands())
                    {
                        if (Op == Ind)
                            continue;
                        Info.FrontierSizeVal = Op;
                        if (auto *SL = dyn_cast<LoadInst>(Op))
                            Info.FrontierSizePtr = SL->getPointerOperand();
                    }
                }
    if (Info.DriverLoop)
        for (BasicBlock *BB : Info.DriverLoop->blocks())
            for (Instruction &I : *BB)
                if (auto *CI = dyn_cast<CallInst>(&I))
                    if (Function *CF = CI->getCalledFunction())
                        if (CF->getName() == "roaring_bitmap_get_at_index" &&
                            CI->arg_size() >= 1)
                            if (auto *BL = dyn_cast<LoadInst>(CI->getArgOperand(0)))
                                Info.FrontierSetPtr = BL->getPointerOperand();
    /* Preheader of the driver often holds the bitmap load used by get_at_index. */
    if (!Info.FrontierSetPtr && Info.DriverLoop)
        if (BasicBlock *PH = Info.DriverLoop->getLoopPreheader())
            for (Instruction &I : *PH)
                if (auto *CI = dyn_cast<CallInst>(&I))
                    if (Function *CF = CI->getCalledFunction())
                        if (CF->getName() == "roaring_bitmap_get_cardinality" &&
                            CI->arg_size() >= 1)
                            if (auto *BL = dyn_cast<LoadInst>(CI->getArgOperand(0)))
                                Info.FrontierSetPtr = BL->getPointerOperand();

    bool HasV = false, HasU = false;
    std::unordered_set<const Value *> WrittenBases;
    std::unordered_set<const Value *> ReadBases;
    for (BasicBlock *BB : L->blocks())
        for (Instruction &I : *BB)
        {
            if (auto *SI = dyn_cast<StoreInst>(&I))
            {
                Value *Ptr = SI->getPointerOperand();
                if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr))
                {
                    /* Canonical base (CSE-robust) so an in-place read+write of
                     * the same logical array is detected regardless of whether
                     * the base loads were merged. */
                    WrittenBases.insert(canonicalArrayBase(GEP));
                    for (Value *IX : GEP->indices())
                    {
                        if (isa<Constant>(IX))
                            continue;
                        switch (Prov.regionOf(IX))
                        {
                        case Region::V:
                            HasV = true;
                            break;
                        case Region::U:
                            HasU = true;
                            break;
                        case Region::D:
                            Info.HasDataWrite = true;
                            break;
                        default:
                            break;
                        }
                    }
                    RedOp AOp = detectScalarRedOp(SI->getValueOperand(), Ptr);
                    if (AOp == RedOp::None)
                        AOp = detectConditionalMinMax(SI, Ptr);
                    if (AOp != RedOp::None)
                        Info.HasRecognizedOp = true;
                    if (detectFirstWinsStore(SI))
                    {
                        Info.HasFirstWins = true;
                        Info.HasRecognizedOp = true;
                    }
                }
                else
                {
                    /* scalar / non-vertex-indexed write: reduction candidate.
                     * Recognize the update operator (+ - * min max & | ^) so
                     * every associative-commutative reduction parallelizes. */
                    if (IndexSlots.count(Ptr))
                    {
                        /* frontier-append counter (next_size) — not a reduction */
                    }
                    else if (!Info.ReducePtr)
                    {
                        Info.ReducePtr = Ptr;
                        Info.ReduceOp = detectScalarRedOp(SI->getValueOperand(), Ptr);
                        if (Info.ReduceOp == RedOp::None)
                            Info.ReduceOp = detectConditionalMinMax(SI, Ptr);
                        if (Info.ReduceOp != RedOp::None)
                            Info.HasRecognizedOp = true;
                    }
                    else if (Info.ReduceOp == RedOp::None)
                    {
                        /* First store was not a recognizable update; a later
                         * guarded min/max store on the same slot still counts. */
                        if (Info.ReducePtr == Ptr)
                        {
                            Info.ReduceOp = detectConditionalMinMax(SI, Ptr);
                            if (Info.ReduceOp != RedOp::None)
                                Info.HasRecognizedOp = true;
                        }
                    }
                }
            }
            if (auto *LI = dyn_cast<LoadInst>(&I))
            {
                if (auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand()))
                {
                    for (Value *IX : GEP->indices())
                    {
                        Region R = Prov.regionOf(IX);
                        if (R == Region::V || R == Region::U)
                        {
                            ReadBases.insert(canonicalArrayBase(GEP));
                            break;
                        }
                    }
                }
            }
        }

    for (const Value *Base : WrittenBases)
        if (ReadBases.count(Base))
            Info.NeedsRoundSep = true;

    if (Info.HasDataWrite)
        Info.WriteKind = NeighborLoopInfo::WriteData; /* data-index → sequential */
    else if (HasV && HasU)
        Info.WriteKind = NeighborLoopInfo::WriteMixed;
    else if (HasV)
        Info.WriteKind = NeighborLoopInfo::WriteV;
    else if (HasU)
        Info.WriteKind = NeighborLoopInfo::WriteU;
    else
        Info.WriteKind = NeighborLoopInfo::WriteUnknown; /* red handled below */

    /* Per-source preamble in the driver (kcore `alive[u]=0`) is not in the
     * neighbor body; record U-indexed stores so emit can run them per pair. */
    for (BasicBlock *BB : Info.DriverLoop->blocks())
    {
        if (L->contains(BB))
            continue;
        for (Instruction &I : *BB)
            if (auto *SI = dyn_cast<StoreInst>(&I))
                if (auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
                    for (Value *IX : GEP->indices())
                        if (!isa<Constant>(IX) && Prov.regionOf(IX) == Region::U)
                        {
                            Info.DriverUStores.push_back(SI);
                            break;
                        }
    }
    return true;
}

/* Iterator-less detection: does any store in the loop body write through a
 * subscript that is load-derived from a graph data array element?  This catches
 * hand-rolled loops (e.g. `while (i<n) next_rank[nodes[i]] += c;`) that never
 * use the neighbor iterator and would otherwise reach the racy DOALL path. */
static bool loopHasGraphDataWrite(Loop *L)
{
    Function *F = L->getHeader()->getParent();
    Provenance Prov(*F, nullptr, {}); /* no v/u seeds: only data-array origins */
    for (BasicBlock *BB : L->blocks())
        for (Instruction &I : *BB)
            if (auto *SI = dyn_cast<StoreInst>(&I))
                if (auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
                    for (Value *IX : GEP->indices())
                        if (!isa<Constant>(IX) && Prov.regionOf(IX) == Region::D)
                            return true;
    return false;
}

/* ── classifier: effect set → execution class ──────────────────────
 * Derives the same decisions the pass has always made, expressed as one
 * function over the loop's effect summary.  Sequential covers WriteData,
 * round-separation overlap, mixed ownership, and unclassifiable loops. */
enum class Klass
{
    SourceOwner,
    DestOwner,
    Reduction,
    Sequential
};

static bool envelopeWired(const NeighborLoopInfo &Info)
{
    bool ArrayEnv = Info.AppendArrayPtr && Info.AppendCountPtr &&
                    Info.FrontierArrayPtr &&
                    (Info.FrontierSizePtr || Info.FrontierSizeVal);
    bool SetEnv = Info.FrontierSetPtr && Info.NextSetPtr;
    return ArrayEnv || SetEnv;
}

static Klass classify(const NeighborLoopInfo &Info)
{
    /* Unified Update(operator, region) rule.  Graph-data-index and mixed-
     * ownership writes are never safe to DOALL.  Otherwise the single write
     * endpoint selects the owner-computes handler.  An in-place read+write
     * overlap on the owned region (NeedsRoundSep) with a recognized operator
     * (incl. first-wins) is race-free under owner-computes -- one partition
     * owns the index -- so it stays parallel. */
    if (Info.HasDataWrite)
        return Klass::Sequential; /* WriteData */
    if (Info.WriteKind == NeighborLoopInfo::WriteMixed)
        return Klass::Sequential; /* two independently-owned write regions */
    if (Info.HasFrontierAppend)
    {
        /* BFS/SSSP/cc/kcore-style frontier loop: dest-owned updates plus a
         * chained frontier append.  Parallelizable via the envelope when the
         * append arrays/sets were recovered. */
        if (envelopeWired(Info) &&
            Info.WriteKind == NeighborLoopInfo::WriteV)
            return Klass::DestOwner;
        return Klass::Sequential;
    }
    if (Info.ReducePtr && Info.ReduceOp != RedOp::None)
        return Klass::Reduction; /* global-scalar Update(op) -> partials+combine */
    if (Info.WriteKind == NeighborLoopInfo::WriteV)
    {
        if (Info.NeedsRoundSep && !Info.HasRecognizedOp && !Info.HasFirstWins)
            return Klass::Sequential;
        return Klass::DestOwner;
    }
    if (Info.WriteKind == NeighborLoopInfo::WriteU)
    {
        if (Info.NeedsRoundSep && !Info.HasRecognizedOp)
            return Klass::Sequential;
        return Klass::SourceOwner;
    }
    return Klass::Sequential;
}

static const char *klassName(Klass K)
{
    switch (K)
    {
    case Klass::SourceOwner:
        return "source-owner";
    case Klass::DestOwner:
        return "dest-owner";
    case Klass::Reduction:
        return "reduction";
    default:
        return "sequential";
    }
}

/* ── per-pair wrapper via manual clone ────────────────────────── */

/* Combine one partition partial into the running total: `*out = *out (op) *partial`.
 * Float vs int chosen from the element type; ascending partition-order calls in
 * the runtime keep the fold deterministic. */
static Function *emitRedCombiner(LLVMContext &Ctx, Module *Mod, RedOp Op,
                                 Type *ElemTy)
{
    FunctionType *FT = FunctionType::get(Type::getVoidTy(Ctx),
                                         {PointerType::get(Ctx, 0),
                                          PointerType::get(Ctx, 0)}, false);
    Function *FN = Function::Create(FT, GlobalValue::InternalLinkage,
                                    "sgpl_combine_red", Mod);
    BasicBlock *BB = BasicBlock::Create(Ctx, "entry", FN);
    IRBuilder<> B(BB);
    Value *P = B.CreateLoad(ElemTy, FN->getArg(0)); /* partial */
    Value *O = B.CreateLoad(ElemTy, FN->getArg(1)); /* out (running total) */
    bool isFP = ElemTy->isFloatingPointTy();
    Value *R = nullptr;
    switch (Op)
    {
    case RedOp::Add:
        R = isFP ? B.CreateFAdd(O, P) : B.CreateAdd(O, P);
        break;
    case RedOp::Sub:
        /* The body computed `acc = acc - x`, so each partition partial already
         * holds the negated partition sum (identity 0); fold partials into the
         * running total with addition so the result equals the serial
         * `init - sum(x)`. */
        R = isFP ? B.CreateFAdd(O, P) : B.CreateAdd(O, P);
        break;
    case RedOp::Mul:
        R = isFP ? B.CreateFMul(O, P) : B.CreateMul(O, P);
        break;
    case RedOp::Min:
        R = B.CreateBinaryIntrinsic(isFP ? Intrinsic::minnum : Intrinsic::smin, O, P);
        break;
    case RedOp::Max:
        R = B.CreateBinaryIntrinsic(isFP ? Intrinsic::maxnum : Intrinsic::smax, O, P);
        break;
    case RedOp::And:
        R = B.CreateAnd(O, P);
        break;
    case RedOp::Or:
        R = B.CreateOr(O, P);
        break;
    case RedOp::Xor:
        R = B.CreateXor(O, P);
        break;
    default:
        R = isFP ? B.CreateFAdd(O, P) : B.CreateAdd(O, P);
        break;
    }
    B.CreateStore(R, FN->getArg(1));
    B.CreateRetVoid();
    return FN;
}

/* Identity element for a reduction operator, used to initialize per-partition
 * partials so the ordered combine yields the serial result. */
static Constant *identityFor(RedOp Op, Type *ElemTy)
{
    bool isFP = ElemTy->isFloatingPointTy();
    switch (Op)
    {
    case RedOp::Mul:
        return isFP ? ConstantFP::get(ElemTy, 1.0)
                    : ConstantInt::get(ElemTy, 1);
    case RedOp::And:
        return ConstantInt::getAllOnesValue(ElemTy);
    case RedOp::Min:
        return isFP ? ConstantFP::getInfinity(ElemTy, /*Negative=*/false)
                    : ConstantInt::get(ElemTy,
                                       APInt::getSignedMaxValue(
                                           ElemTy->getIntegerBitWidth()));
    case RedOp::Max:
        return isFP ? ConstantFP::getInfinity(ElemTy, /*Negative=*/true)
                    : ConstantInt::get(ElemTy,
                                       APInt::getSignedMinValue(
                                           ElemTy->getIntegerBitWidth()));
    default: /* Add, Sub, Or, Xor */
        return Constant::getNullValue(ElemTy);
    }
}

/* Fill `PartCount` partial slots at `Partials` with the operator identity by
 * splitting `Pre` and emitting a small count loop.  `EB` is left positioned in
 * the continuation block (before its terminator) so the caller keeps emitting
 * the build/step sequence there. */
static void emitPartialInit(IRBuilder<> &EB, BasicBlock *Pre, Value *Partials,
                            Value *PartCount, Type *ElemTy, RedOp Op,
                            LLVMContext &Ctx)
{
    Constant *Identity = identityFor(Op, ElemTy);
    Type *I32 = Type::getInt32Ty(Ctx);
    Instruction *Term = &*EB.GetInsertPoint();
    BasicBlock *Cont = Pre->splitBasicBlock(Term, "red_fill_cont");
    Function *Fn = Pre->getParent();
    BasicBlock *Cond = BasicBlock::Create(Ctx, "red_fill_cond", Fn, Cont);
    BasicBlock *Body = BasicBlock::Create(Ctx, "red_fill_body", Fn, Cont);
    Pre->getTerminator()->eraseFromParent();
    IRBuilder<>(Pre).CreateBr(Cond);

    IRBuilder<> CB(Cond);
    PHINode *IV = CB.CreatePHI(I32, 2, "fi");
    IV->addIncoming(ConstantInt::get(I32, 0), Pre);
    Value *Cmp = CB.CreateICmpSLT(IV, PartCount);
    CB.CreateCondBr(Cmp, Body, Cont);

    IRBuilder<> BB(Body);
    Value *EP = BB.CreateGEP(ElemTy, Partials, IV);
    BB.CreateStore(Identity, EP);
    Value *Nxt = BB.CreateAdd(IV, ConstantInt::get(I32, 1));
    BB.CreateBr(Cond);
    IV->addIncoming(Nxt, Body);

    EB.SetInsertPoint(Cont->getTerminator());
}

static Value *ptrFromSlot(IRBuilder<> &B, Value *Slot, Type *I8P)
{
    if (auto *AI = dyn_cast<AllocaInst>(Slot))
        if (!AI->getAllocatedType()->isPointerTy())
            return B.CreateBitCast(Slot, I8P);
    return B.CreateLoad(I8P, Slot);
}

static bool emitCleanCutCallbackAndStep(const NeighborLoopInfo &Info, bool SourceOwner)
{
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    LLVMContext &Ctx = F->getContext();
    Module *Mod = F->getParent();

    /* Body = inner-loop blocks minus its header. */
    SmallVector<BasicBlock *> BodyBlocks;
    for (BasicBlock *BB : Info.NeighborLoop->getBlocks())
        if (BB != Info.NeighborLoop->getHeader())
            BodyBlocks.push_back(BB);
    if (BodyBlocks.empty())
        return false;

    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I64 = Type::getInt64Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);

    /* Pair wrapper. */
    FunctionType *FT = FunctionType::get(Type::getVoidTy(Ctx),
                                         {I32, I32, I64, I8P}, false);
    Function *WF = Function::Create(FT, GlobalValue::InternalLinkage,
                                    "sgpl_pair_work", Mod);
    BasicBlock *EntryBB = BasicBlock::Create(Ctx, "entry", WF);
    IRBuilder<> WB(EntryBB);
    Argument *ArgU = WF->getArg(0);
    Argument *ArgV = WF->getArg(1);
    Argument *ArgEnv = WF->getArg(3);

    AllocaInst *USlot = WB.CreateAlloca(I32, nullptr, "uslot");
    WB.CreateStore(ArgU, USlot);
    AllocaInst *VSlot = WB.CreateAlloca(I32, nullptr, "vslot");
    WB.CreateStore(ArgV, VSlot);

    /* u value terminals: U64 = init's i64 u; U32 = its i32 source. */
    Value *U64 = Info.UVal;
    Value *U32 = Info.UVal;
    if (auto *CAS = dyn_cast<CastInst>(U64))
        U32 = CAS->getOperand(0);
    Value *U64Src = WB.CreateZExt(ArgU, I64, "u64src");

    /* Reduction: accumulate directly into this partition's partial (ArgEnv).
     * The runtime pre-initializes each partition partial to the operator
     * identity and folds them into the target in partition order.  Mapping the
     * body's ReducePtr to the partial pointer means every edge's load+op+store
     * of the reduction target accumulates into the running partial.
     * (Previously a per-call zero-init slot overwrote the partial each edge,
     * which zeroed multiplicative reductions and only summed correctly when
     * every partition held exactly one edge.) */
    Value *RedPartial = nullptr;
    Type *RedElemTy = nullptr;
    const bool UseEnvelope = envelopeWired(Info) && Info.HasFrontierAppend &&
                             !Info.ReducePtr;
    Value *SeenBase = nullptr;
    if (Info.ReducePtr)
    {
        RedElemTy = Type::getDoubleTy(Ctx);
        for (User *U : Info.ReducePtr->users())
            if (auto *LI = dyn_cast<LoadInst>(U))
                RedElemTy = LI->getType();
        RedPartial = WB.CreateBitCast(ArgEnv, PointerType::get(RedElemTy, 0));
    }
    else if (UseEnvelope)
        SeenBase = WB.CreateBitCast(ArgEnv, PointerType::get(I32, 0));

    std::unordered_map<BasicBlock *, BasicBlock *> BBMap;
    for (BasicBlock *BB : BodyBlocks)
        BBMap[BB] = BasicBlock::Create(Ctx, BB->getName(), WF);
    BasicBlock *RetStub = BasicBlock::Create(Ctx, "pair_ret", WF);
    IRBuilder<> RB(RetStub);
    RB.CreateRetVoid();

    /* Recursive value cloner. */
    ValueToValueMapTy Map;
    Map[U32] = ArgU;
    Map[U64] = U64Src;
    Map[Info.VAlloca] = VSlot;
    if (Info.UAlloca)
        Map[Info.UAlloca] = USlot;
    if (Value *IndVar = driverIndVar(Info.DriverLoop))
        Map[IndVar] = U64Src;
    if (Info.ReducePtr)
        Map[Info.ReducePtr] = RedPartial;

    auto CloneValue = [&](Value *V, auto &&CloneValueRef) -> Value *
    {
        if (Map.count(V))
            return Map[V];
        if (isa<Constant>(V) || isa<Argument>(V))
            return V;
        if (auto *I = dyn_cast<Instruction>(V))
        {
            if (isa<PHINode>(I))
            {
                Map[V] = USlot;
                return USlot;
            }
            Instruction *Clone = I->clone();
            for (unsigned oi = 0; oi < Clone->getNumOperands(); ++oi)
            {
                Value *M = CloneValueRef(Clone->getOperand(oi), CloneValueRef);
                if (!M)
                {
                    Clone->deleteValue();
                    return nullptr;
                }
                Clone->setOperand(oi, M);
            }
            /* Insert in the cloned parent block when possible so loads/icmps
             * keep their original order relative to stores (kcore deg-- then
             * deg<k).  Fall back to the entry block for pre-loop values. */
            BasicBlock *Dest = EntryBB;
            auto It = BBMap.find(I->getParent());
            if (It != BBMap.end())
                Dest = It->second;
            if (Instruction *T = Dest->getTerminator())
                Clone->insertBefore(T);
            else
                Clone->insertInto(Dest, Dest->end());
            Map[V] = Clone;
            return Clone;
        }
        return nullptr;
    };

    auto isAppendElide = [&](Instruction &I) -> bool
    {
        if (!UseEnvelope)
            return false;
        if (auto *CI = dyn_cast<CallInst>(&I))
            if (Function *CF = CI->getCalledFunction())
                if (CF->getName() == "roaring_bitmap_add")
                    return true;
        if (auto *SI = dyn_cast<StoreInst>(&I))
        {
            if (SI->getPointerOperand() == Info.AppendCountPtr)
                return true;
            if (auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
            {
                Value *GBase = GEP->getPointerOperand();
                Value *BaseSlot = GBase;
                if (auto *BL = dyn_cast<LoadInst>(GBase))
                    BaseSlot = BL->getPointerOperand();
                if (BaseSlot == Info.AppendArrayPtr)
                    return true;
            }
        }
        return false;
    };

    /* Phase 1: clone non-terminator instructions into their cloned blocks so
     * Map is complete before operand remap (avoids hoisting icmps into entry). */
    for (BasicBlock *BB : BodyBlocks)
    {
        BasicBlock *CloneBB = BBMap[BB];
        for (Instruction &I : *BB)
        {
            if (I.isTerminator())
                break;
            if (isAppendElide(I))
                continue;
            Instruction *Clone = I.clone();
            Map[&I] = Clone;
            Clone->insertInto(CloneBB, CloneBB->end());
        }
    }

    auto remapInst = [&](Instruction *Clone) -> bool
    {
        for (unsigned oi = 0; oi < Clone->getNumOperands(); ++oi)
        {
            Value *M = CloneValue(Clone->getOperand(oi), CloneValue);
            if (!M)
                return false;
            Clone->setOperand(oi, M);
        }
        return true;
    };
    for (BasicBlock *BB : BodyBlocks)
        for (Instruction &I : *BB)
            if (Map.count(&I))
                if (!remapInst(cast<Instruction>(Map[&I])))
                    return false;

    /* Rewrite elided frontier appends into dest_seen[v] = 1 in-place. */
    BasicBlock *FirstClone = nullptr;
    for (BasicBlock *BB : BodyBlocks)
    {
        BasicBlock *CloneBB = BBMap[BB];
        if (!FirstClone)
            FirstClone = CloneBB;
        if (UseEnvelope && SeenBase)
            for (Instruction &I : *BB)
            {
                if (I.isTerminator())
                    break;
                if (!isAppendElide(I))
                    continue;
                Value *Vtx = nullptr;
                if (auto *CI = dyn_cast<CallInst>(&I))
                    Vtx = CloneValue(CI->getArgOperand(1), CloneValue);
                else if (auto *SI = dyn_cast<StoreInst>(&I))
                {
                    if (SI->getPointerOperand() == Info.AppendCountPtr)
                        continue;
                    Vtx = CloneValue(SI->getValueOperand(), CloneValue);
                }
                if (!Vtx)
                    continue;
                if (Vtx->getType() != I32)
                    Vtx = IRBuilder<>(CloneBB).CreateIntCast(Vtx, I32, true);
                IRBuilder<> B3(CloneBB);
                Value *Slot = B3.CreateGEP(I32, SeenBase, Vtx);
                B3.CreateStore(ConstantInt::get(I32, 1), Slot);
            }
        Instruction *HostTerm = BB->getTerminator();
        if (auto *HB = dyn_cast<BranchInst>(HostTerm))
        {
            if (HB->isUnconditional())
            {
                BasicBlock *T = HB->getSuccessor(0);
                BasicBlock *Dst = BBMap.count(T) ? BBMap[T] : RetStub;
                IRBuilder<> B2(CloneBB);
                B2.CreateBr(Dst);
            }
            else
            {
                Value *MCond = CloneValue(HB->getCondition(), CloneValue);
                if (!MCond)
                    return false;
                BasicBlock *T0 = HB->getSuccessor(0);
                BasicBlock *T1 = HB->getSuccessor(1);
                BasicBlock *D0 = BBMap.count(T0) ? BBMap[T0] : RetStub;
                BasicBlock *D1 = BBMap.count(T1) ? BBMap[T1] : RetStub;
                IRBuilder<> B2(CloneBB, CloneBB->end());
                B2.CreateCondBr(MCond, D0, D1);
            }
        }
        else
        {
            IRBuilder<> B2(CloneBB);
            B2.CreateBr(RetStub);
        }
        if (!FirstClone)
            FirstClone = CloneBB;
    }
    for (StoreInst *SI : Info.DriverUStores)
    {
        Instruction *Clone = SI->clone();
        bool Ok = true;
        for (unsigned oi = 0; oi < Clone->getNumOperands(); ++oi)
        {
            Value *M = CloneValue(Clone->getOperand(oi), CloneValue);
            if (!M)
            {
                Ok = false;
                break;
            }
            Clone->setOperand(oi, M);
        }
        if (!Ok)
        {
            Clone->deleteValue();
            continue;
        }
        WB.Insert(Clone);
    }
    WB.CreateBr(FirstClone);

    /* Build+step at the driver preheader end; fresh graph load (the init()
     * graph load lives in the soon-dead driver body). */
    BasicBlock *Pre = Info.DriverLoop->getLoopPreheader();
    if (!Pre || !Pre->getTerminator())
        return false;
    IRBuilder<> EB(Pre, Pre->getFirstInsertionPt());
    EB.SetInsertPoint(Pre->getTerminator());
    Value *GraphArg = Info.GraphPtr;
    if (auto *GL = dyn_cast<LoadInst>(Info.GraphPtr))
        if (GL->getPointerOperand())
            GraphArg = EB.CreateLoad(GL->getType(), GL->getPointerOperand());
    FunctionCallee BuildCC = Mod->getOrInsertFunction(
        "autograph_build_clean_cut", FunctionType::get(I32, {I8P, I32}, false));
    Value *PartCount = EB.CreateCall(BuildCC, {GraphArg, ConstantInt::get(I32, 0)});

    FunctionCallee Step;
    SmallVector<Value *, 12> StepArgs;
    if (Info.ReducePtr)
    {
        AllocaInst *Partials = EB.CreateAlloca(RedElemTy, PartCount, "red_partials");
        /* Initialize every partial to the operator identity (0 / 1 / all-ones /
         * +inf / -inf) so the ordered combine reproduces the serial result. */
        emitPartialInit(EB, Pre, Partials, PartCount, RedElemTy, Info.ReduceOp, Ctx);
        Function *Combiner = emitRedCombiner(Ctx, Mod, Info.ReduceOp, RedElemTy);
        Step = Mod->getOrInsertFunction(
            "autograph_frontier_step_owner_red",
            FunctionType::get(I32,
                              {I8P, I8P, I32, I8P, I8P, I64, I8P, I8P, I8P, I8P, I32,
                               I8P}, false));
        StepArgs = {GraphArg,
                    ConstantPointerNull::get(cast<PointerType>(I8P)),
                    ConstantInt::get(I32, 0),
                    EB.CreateBitCast(WF, I8P),
                    EB.CreateBitCast(Partials, I8P),
                    ConstantInt::get(I64, (RedElemTy->getPrimitiveSizeInBits() + 7) / 8),
                    EB.CreateBitCast(Combiner, I8P),
                    EB.CreateBitCast(Info.ReducePtr, I8P),
                    ConstantPointerNull::get(cast<PointerType>(I8P)),
                    ConstantPointerNull::get(cast<PointerType>(I8P)),
                    ConstantInt::get(I32, 0),
                    ConstantPointerNull::get(cast<PointerType>(I8P))};
    }
    else
    {
        const char *StepName = SourceOwner ? "autograph_frontier_step_owner_source"
                                           : "autograph_frontier_step_owner_push";
        Step = Mod->getOrInsertFunction(
            StepName, FunctionType::get(I32, {I8P, I8P, I32, I8P, I8P, I8P, I8P, I32,
                                              I8P}, false));
        Value *FrontArg = ConstantPointerNull::get(cast<PointerType>(I8P));
        Value *FrontSize = ConstantInt::get(I32, 0);
        Value *WorkEnv = ConstantPointerNull::get(cast<PointerType>(I8P));
        Value *Membership = ConstantPointerNull::get(cast<PointerType>(I8P));
        Value *NextArg = ConstantPointerNull::get(cast<PointerType>(I8P));
        Value *SeenArg = ConstantPointerNull::get(cast<PointerType>(I8P));
        Value *NewSize = nullptr;
        if (UseEnvelope)
        {
            FunctionCallee SeenFn = Mod->getOrInsertFunction(
                "autograph_scratch_dest_seen",
                FunctionType::get(PointerType::get(I32, 0), {I8P}, false));
            FunctionCallee MemFn = Mod->getOrInsertFunction(
                "autograph_scratch_membership",
                FunctionType::get(PointerType::get(Type::getInt8Ty(Ctx), 0), {I8P}, false));
            SeenArg = EB.CreateBitCast(EB.CreateCall(SeenFn, {GraphArg}), I8P);
            WorkEnv = SeenArg;
            Membership = EB.CreateBitCast(EB.CreateCall(MemFn, {GraphArg}), I8P);
            if (Info.FrontierSetPtr && Info.NextSetPtr)
            {
                FunctionCallee PrepBm = Mod->getOrInsertFunction(
                    "autograph_prepare_frontier_bitmap",
                    FunctionType::get(I32, {I8P, I8P}, false));
                FunctionCallee NextFn = Mod->getOrInsertFunction(
                    "autograph_scratch_next_frontier",
                    FunctionType::get(PointerType::get(I32, 0), {I8P}, false));
                Value *FBM = EB.CreateLoad(I8P, Info.FrontierSetPtr);
                FrontSize = EB.CreateCall(PrepBm, {GraphArg, FBM});
                NextArg = EB.CreateBitCast(EB.CreateCall(NextFn, {GraphArg}), I8P);
            }
            else
            {
                FunctionCallee PrepArr = Mod->getOrInsertFunction(
                    "autograph_prepare_frontier_array",
                    FunctionType::get(I32, {I8P, I8P, I32}, false));
                Value *FArr = ptrFromSlot(EB, Info.FrontierArrayPtr, I8P);
                FrontArg = FArr;
                if (Info.FrontierSizePtr)
                    FrontSize = EB.CreateLoad(I32, Info.FrontierSizePtr);
                else
                    FrontSize = Info.FrontierSizeVal;
                FrontSize = EB.CreateCall(PrepArr, {GraphArg, FArr, FrontSize});
                NextArg = ptrFromSlot(EB, Info.AppendArrayPtr, I8P);
            }
        }
        StepArgs = {GraphArg, FrontArg, FrontSize,
                    EB.CreateBitCast(WF, I8P), WorkEnv, Membership, NextArg,
                    ConstantInt::get(I32, 0), SeenArg};
        NewSize = EB.CreateCall(Step, StepArgs);
        if (UseEnvelope)
        {
            if (Info.FrontierSetPtr && Info.NextSetPtr)
            {
                FunctionCallee Commit = Mod->getOrInsertFunction(
                    "autograph_commit_frontier_bitmap",
                    FunctionType::get(Type::getVoidTy(Ctx), {I8P, I8P, I32}, false));
                Value *NBM = EB.CreateLoad(I8P, Info.NextSetPtr);
                EB.CreateCall(Commit, {GraphArg, NBM, NewSize});
            }
            else if (Info.AppendCountPtr)
                EB.CreateStore(NewSize, Info.AppendCountPtr);
            if (Instruction *T = Info.DriverLoop->getHeader()->getTerminator())
                T->setMetadata(
                    "sgpl.frontier.first_wins.doall",
                    MDNode::get(Ctx, MDString::get(Ctx, "requires-int-append-priv")));
        }
    }
    if (Info.ReducePtr)
        EB.CreateCall(Step, StepArgs);

    /* Deactivate the old driver nest: route the driver header's loop-entry
     * (true) edge to its exit so the whole original nest becomes dead code
     * (the CleanCut step below the preheader is the only live path). */
    BasicBlock *Header = Info.DriverLoop->getHeader();
    if (Header)
        if (auto *HB = dyn_cast<BranchInst>(Header->getTerminator()))
            if (HB->isConditional())
                HB->setSuccessor(0, HB->getSuccessor(1));
    return true;
}

/* ── pass ──────────────────────────────────────────────────────── */

PreservedAnalyses GraphFrontierLoweringPass::run(Function &F,
                                                 FunctionAnalysisManager &FAM)
{
    if (F.isDeclaration() || F.begin() == F.end())
        return PreservedAnalyses::all();
    unsigned detected = 0;
    const bool RewriteMode = getenv("GRAPH_FRONTIER_REWRITE") != nullptr;
    auto &LI = FAM.getResult<LoopAnalysis>(F);
    for (Loop *L : LI.getLoopsInPreorder())
    {
        NeighborLoopInfo Info;
        /* A loop is a graph-construct loop if it has the iterator shape OR any
         * write whose subscript is load-derived from a graph data array. */
        bool IsIter = analyzeNeighborLoop(L, Info);
        bool HasData = IsIter ? Info.HasDataWrite : loopHasGraphDataWrite(L);
        if (!IsIter && !HasData)
            continue;
        /* Derive the execution class from the loop's effect set. Iterator-less
         * hand-rolled loops have no per-pair structure to rewrite → sequential. */
        Klass K = IsIter ? classify(Info) : Klass::Sequential;
        if (getenv("GRAPH_FRONTIER_STATS"))
        {
            errs() << "[graph-frontier] candidate: " << F.getName();
            if (IsIter)
                errs() << " driver=" << Info.DriverLoop->getHeader()->getName();
            else
                errs() << " driver=<none>";
            errs() << " inner=" << L->getHeader()->getName()
                   << " kind=" << (int)Info.WriteKind
                   << " red=" << (Info.ReducePtr ? 1 : 0)
                   << " sep=" << (Info.NeedsRoundSep ? 1 : 0)
                   << " data=" << (HasData ? 1 : 0)
                   << " fw=" << (Info.HasFirstWins ? 1 : 0)
                   << " env=" << (Info.HasFrontierAppend ? 1 : 0)
                   << " class=" << klassName(K) << "\n";
        }
        if (RewriteMode)
        {
            /* Iterator-less graph-data write loop (hand-rolled shape): no
             * per-pair structure to rewrite — conservatively sequential. */
            if (!IsIter)
            {
                markSequential(L);
                ++detected;
                continue;
            }
            bool Rewritable = (K != Klass::Sequential);
            if (Rewritable)
            {
                bool SourceOwner = (K == Klass::SourceOwner);
                bool Emitted = emitCleanCutCallbackAndStep(Info, SourceOwner);
                ++detected;
                if (Emitted)
                {
                    /* Keep every part of the round nest serial (the step is
                     * already parallel via CleanCut; the residual beta-init /
                     * swap loops must not be DOALL/GPU-ified by the
                     * reconstruction). */
                    markSequential(L);
                    if (getenv("GRAPH_FRONTIER_DUMP") && F.getParent())
                    {
                        std::error_code EC;
                        raw_fd_ostream Out("/tmp/post_emit_main.ll", EC);
                        if (!EC)
                        {
                            F.print(Out);
                            Out.flush();
                        }
                    }
                    if (getenv("GRAPH_FRONTIER_DUMP"))
                    {
                        std::error_code EC2;
                        raw_fd_ostream Out2("/tmp/post_emit_module.ll", EC2);
                        if (!EC2)
                        {
                            F.getParent()->print(Out2, nullptr);
                            Out2.flush();
                        }
                    }
                    if (getenv("GRAPH_FRONTIER_VERIFY"))
                    {
                        std::string Err;
                        raw_string_ostream SS(Err);
                        if (verifyFunction(F, &SS))
                            errs() << "[graph-frontier] VERIFY FAIL on "
                                   << F.getName() << ": " << SS.str() << "\n";
                        else
                            errs() << "[graph-frontier] verify OK on "
                                   << F.getName() << "\n";
                    }
                    continue;
                }
            }
            /* Anything not rewritten is conservatively sequential. */
            markSequential(L);
        }
        ++detected;
    }
    if (getenv("GRAPH_FRONTIER_STATS") && getenv("GRAPH_FRONTIER_VERBOSE"))
        errs() << "[graph-frontier] detected=" << detected << "\n";
    return PreservedAnalyses::none();
}
