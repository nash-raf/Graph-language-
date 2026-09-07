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
 *   - mixed dest+src writes that are incompatible (same array, carried
 *     dependence, or data flow across phases) -> sequential
 *   - independent W(A,U) ⊗ W(B,V) with membership-only coupling -> DualOwner
 *     (source step then dest step on the same pre-round F_t)
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
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
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

/* Region lattice for the *origin* of an index value:
 *   Bottom  constant / loop-invariant / neutral
 *   U       flows from the driver induction variable (source axis)
 *   V       flows from the iterator v-slot (destination / walked neighbor)
 *   D       flows from a graph data-array element load (nodes[i], perm[v], ...)
 *   G       scalar / global reduction slot (neither endpoint)
 *   Top     mixed / unknown → conservatively sequential */
enum class Region : uint8_t { Bottom = 0, U, V, D, G, Top };

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

enum class EffectKind : uint8_t
{
    R = 0,
    W,
    Uf,       /* U_f: write depends on old value; owner-serializable */
    Uop,      /* U_⊕: recognized algebraic combine; privatizable */
    Claim,    /* first-wins */
    Activate  /* frontier append / dest-owned side effect */
};

enum class Temporal : uint8_t
{
    Independent = 0,
    SameRoundRead,
    PreviousRoundRead,
    Carried
};

struct Effect
{
    EffectKind Kind = EffectKind::R;
    Region Reg = Region::Bottom;
    const Value *Base = nullptr;
    RedOp Op = RedOp::None;
    Temporal Temp = Temporal::Independent;
    Value *Index = nullptr;
    Instruction *Origin = nullptr;
};

static bool effectIsMutating(const Effect &E)
{
    return E.Kind != EffectKind::R;
}

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
    SmallVector<AtomicCmpXchgInst *, 4> DriverUClaims; /* first-wins claims */
    SmallVector<Effect, 8> Effects;   /* primitive effect set E */
    ICmpInst *DriverUGuard = nullptr; /* optional if (pred(u)) wrapping U-stores */
    bool MembershipGated = false;     /* driver is a frontier / F_t iteration */
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
 * Forward dataflow of Region over SSA.  mem2reg has already run before this
 * pass (main.cpp), so index chains are SSA except the escaping v-slot alloca
 * (its address is passed to autograph_neighbor_iter_next), which is handled
 * by an explicit load rule.  A `Top` origin in any store subscript forces
 * the loop sequential. */
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

static Value *primaryIndex(const GetElementPtrInst *GEP)
{
    Value *Last = nullptr;
    for (Value *IX : GEP->indices())
        if (!isa<Constant>(IX))
            Last = IX;
    return Last;
}

/* True if Stored's SSA uses a load of the same slot as Ptr (generic U_f). */
static bool storedDependsOnOldValue(Value *Stored, Value *Ptr)
{
    SmallPtrSet<Value *, 16> Seen;
    SmallVector<Value *, 8> Work;
    Work.push_back(Stored);
    while (!Work.empty())
    {
        Value *V = Work.pop_back_val();
        if (!V || !Seen.insert(V).second)
            continue;
        if (auto *LI = dyn_cast<LoadInst>(V))
        {
            if (sameArraySlot(LI->getPointerOperand(), Ptr))
                return true;
            continue;
        }
        if (auto *I = dyn_cast<Instruction>(V))
        {
            if (isa<PHINode>(I))
                continue;
            for (Value *Op : I->operands())
                Work.push_back(Op);
        }
    }
    return false;
}

static bool valueDependsOnBase(Value *V, const Value *Base)
{
    if (!V || !Base)
        return false;
    SmallPtrSet<Value *, 16> Seen;
    SmallVector<Value *, 8> Work;
    Work.push_back(V);
    while (!Work.empty())
    {
        Value *Cur = Work.pop_back_val();
        if (!Cur || !Seen.insert(Cur).second)
            continue;
        if (auto *LI = dyn_cast<LoadInst>(Cur))
        {
            Value *P = LI->getPointerOperand();
            if (auto *GEP = dyn_cast<GetElementPtrInst>(P))
            {
                if (canonicalArrayBase(GEP) == Base)
                    return true;
            }
            else if (getUnderlyingObject(P) == Base)
                return true;
            continue;
        }
        if (auto *I = dyn_cast<Instruction>(Cur))
        {
            if (isa<GetElementPtrInst>(I))
                continue;
            for (Value *Op : I->operands())
                Work.push_back(Op);
        }
    }
    return false;
}

static const char *effectKindName(EffectKind K)
{
    switch (K)
    {
    case EffectKind::R:
        return "R";
    case EffectKind::W:
        return "W";
    case EffectKind::Uf:
        return "U_f";
    case EffectKind::Uop:
        return "U";
    case EffectKind::Claim:
        return "Claim";
    case EffectKind::Activate:
        return "Activate";
    }
    return "?";
}

static const char *regionName(Region R)
{
    switch (R)
    {
    case Region::U:
        return "U";
    case Region::V:
        return "V";
    case Region::D:
        return "D";
    case Region::G:
        return "G";
    case Region::Top:
        return "Top";
    default:
        return "_";
    }
}

static int temporalRank(Temporal T)
{
    switch (T)
    {
    case Temporal::Carried:
        return 3;
    case Temporal::PreviousRoundRead:
        return 2;
    case Temporal::SameRoundRead:
        return 1;
    default:
        return 0;
    }
}

static const char *temporalName(Temporal T)
{
    switch (T)
    {
    case Temporal::SameRoundRead:
        return "SameRoundRead";
    case Temporal::PreviousRoundRead:
        return "PreviousRoundRead";
    case Temporal::Carried:
        return "Carried";
    default:
        return "Independent";
    }
}

static const char *redOpName(RedOp Op)
{
    switch (Op)
    {
    case RedOp::Add:
        return "+";
    case RedOp::Sub:
        return "-";
    case RedOp::Mul:
        return "*";
    case RedOp::Min:
        return "min";
    case RedOp::Max:
        return "max";
    case RedOp::And:
        return "&";
    case RedOp::Or:
        return "|";
    case RedOp::Xor:
        return "^";
    case RedOp::FirstWins:
        return "fw";
    default:
        return "";
    }
}

static ICmpInst *guardICmpForStore(StoreInst *SI)
{
    BasicBlock *BB = SI->getParent();
    if (!BB)
        return nullptr;
    BasicBlock *P = BB->getSinglePredecessor();
    if (!P)
        return nullptr;
    auto *Br = dyn_cast<BranchInst>(P->getTerminator());
    if (!Br || !Br->isConditional())
        return nullptr;
    if (Br->getSuccessor(0) != BB)
        return nullptr;
    return dyn_cast<ICmpInst>(Br->getCondition());
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
    Provenance ProvU(*F, Info.VAlloca, {IndVar, USrc, Info.UVal}, USlot);

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

    Info.MembershipGated = Info.FrontierSetPtr || Info.FrontierArrayPtr;

    auto classifyStore = [&](StoreInst *SI, Value *Ptr, Region R,
                             const Value *Base, Value *Index) -> Effect
    {
        Effect E;
        E.Reg = R;
        E.Base = Base;
        E.Index = Index;
        E.Origin = SI;
        if (detectFirstWinsStore(SI))
        {
            E.Kind = EffectKind::Claim;
            E.Op = RedOp::FirstWins;
            Info.HasFirstWins = true;
            Info.HasRecognizedOp = true;
            return E;
        }
        RedOp AOp = detectScalarRedOp(SI->getValueOperand(), Ptr);
        if (AOp == RedOp::None)
            AOp = detectConditionalMinMax(SI, Ptr);
        if (AOp != RedOp::None)
        {
            E.Kind = EffectKind::Uop;
            E.Op = AOp;
            Info.HasRecognizedOp = true;
            return E;
        }
        if (storedDependsOnOldValue(SI->getValueOperand(), Ptr))
        {
            E.Kind = EffectKind::Uf;
            return E;
        }
        E.Kind = EffectKind::W;
        return E;
    };

    bool HasV = false, HasU = false;
    std::unordered_set<const Value *> WrittenBases;
    std::unordered_set<const Value *> ReadBases;
    auto noteWritten = [&](Region R, const Value *Base)
    {
        if (Base)
            WrittenBases.insert(Base);
        if (R == Region::V)
            HasV = true;
        else if (R == Region::U)
            HasU = true;
        else if (R == Region::D)
            Info.HasDataWrite = true;
    };

    for (BasicBlock *BB : L->blocks())
        for (Instruction &I : *BB)
        {
            if (auto *SI = dyn_cast<StoreInst>(&I))
            {
                Value *Ptr = SI->getPointerOperand();
                if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr))
                {
                    const Value *Base = canonicalArrayBase(GEP);
                    Value *IX = primaryIndex(GEP);
                    /* next_frontier[next_size] = v is Activate(V), not a Top
                     * write through the append counter. */
                    bool AppendStore = false;
                    if (Info.AppendArrayPtr && Base == Info.AppendArrayPtr)
                        AppendStore = true;
                    if (IX)
                        if (auto *LI = dyn_cast<LoadInst>(IX))
                            if (IndexSlots.count(LI->getPointerOperand()) ||
                                LI->getPointerOperand() == Info.AppendCountPtr)
                                AppendStore = true;
                    if (AppendStore)
                        continue;
                    Region R = IX ? Prov.regionOf(IX) : Region::Bottom;
                    noteWritten(R, Base);
                    Info.Effects.push_back(classifyStore(SI, Ptr, R, Base, IX));
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
                        Effect E;
                        E.Reg = Region::G;
                        E.Base = Ptr;
                        E.Index = nullptr;
                        E.Origin = SI;
                        if (Info.ReduceOp != RedOp::None)
                        {
                            E.Kind = EffectKind::Uop;
                            E.Op = Info.ReduceOp;
                        }
                        else if (storedDependsOnOldValue(SI->getValueOperand(), Ptr))
                            E.Kind = EffectKind::Uf;
                        else
                            E.Kind = EffectKind::W;
                        Info.Effects.push_back(E);
                    }
                    else if (Info.ReduceOp == RedOp::None)
                    {
                        /* First store was not a recognizable update; a later
                         * guarded min/max store on the same slot still counts. */
                        if (Info.ReducePtr == Ptr)
                        {
                            Info.ReduceOp = detectConditionalMinMax(SI, Ptr);
                            if (Info.ReduceOp != RedOp::None)
                            {
                                Info.HasRecognizedOp = true;
                                for (Effect &E : Info.Effects)
                                    if (E.Origin == SI ||
                                        (E.Base == Ptr && E.Reg == Region::G))
                                    {
                                        E.Kind = EffectKind::Uop;
                                        E.Op = Info.ReduceOp;
                                    }
                            }
                        }
                    }
                }
            }
            if (auto *LI = dyn_cast<LoadInst>(&I))
            {
                if (auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand()))
                {
                    Value *IX = primaryIndex(GEP);
                    Region R = IX ? Prov.regionOf(IX) : Region::Bottom;
                    if (R == Region::V || R == Region::U)
                    {
                        const Value *Base = canonicalArrayBase(GEP);
                        ReadBases.insert(Base);
                        Effect E;
                        E.Kind = EffectKind::R;
                        E.Reg = R;
                        E.Base = Base;
                        E.Index = IX;
                        E.Origin = LI;
                        Info.Effects.push_back(E);
                    }
                }
            }
            if (auto *CI = dyn_cast<CallInst>(&I))
                if (Function *CF = CI->getCalledFunction())
                    if (CF->getName() == "roaring_bitmap_add" &&
                        CI->arg_size() >= 2)
                    {
                        Effect E;
                        E.Kind = EffectKind::Activate;
                        E.Reg = Region::V;
                        E.Base = Info.NextSetPtr;
                        E.Index = CI->getArgOperand(1);
                        E.Origin = CI;
                        Info.Effects.push_back(E);
                    }
        }

    if (Info.HasFrontierAppend && Info.AppendArrayPtr)
    {
        bool HaveAct = false;
        for (const Effect &E : Info.Effects)
            if (E.Kind == EffectKind::Activate)
                HaveAct = true;
        if (!HaveAct)
        {
            Effect E;
            E.Kind = EffectKind::Activate;
            E.Reg = Region::V;
            E.Base = Info.AppendArrayPtr;
            E.Origin = nullptr;
            Info.Effects.push_back(E);
        }
    }

    for (const Value *Base : WrittenBases)
        if (ReadBases.count(Base))
            Info.NeedsRoundSep = true;

    /* Per-source preamble in the driver (kcore `alive[u]=0`) is not in the
     * neighbor body; these are ordinary U-effects in E, not a graft list.
     * The preamble index is the driver's own u (the iterator init arg or the
     * frontier-set element) whose provenance may be Top (a set-iteration
     * call result), so accept UVal-derived indexes as U. */
    for (BasicBlock *BB : Info.DriverLoop->blocks())
    {
        if (getenv("GRAPH_FRONTIER_DIAG"))
        {
            unsigned SC = 0;
            for (Instruction &DI : *BB)
                if (isa<StoreInst>(&DI))
                    SC++;
            errs() << "[diag-driver] block=" << BB->getName()
                   << " stores=" << SC << "\n";
        }
        if (L->contains(BB))
            continue;
        for (Instruction &I : *BB)
        {
            if (auto *SI = dyn_cast<StoreInst>(&I))
                if (auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
                {
                    Value *IX = primaryIndex(GEP);
                    if (getenv("GRAPH_FRONTIER_DIAG"))
                        errs() << "[diag-driver] store " << (IX ? "IX" : "noIX")
                               << " base="
                               << (canonicalArrayBase(GEP) && canonicalArrayBase(GEP)->hasName()
                                       ? canonicalArrayBase(GEP)->getName()
                                       : "_")
                               << " reg="
                               << (IX ? (int)Prov.regionOf(IX) : -1)
                               << " uval=" << (Info.UVal ? 1 : 0)
                               << " uvalDeriv="
                               << (IX && Info.UVal
                                           ? ProvU.regionOf(IX) == Region::U
                                           : 0)
                               << "\n";
                    if (!IX)
                        continue;
                    if (Prov.regionOf(IX) != Region::U &&
                        ProvU.regionOf(IX) != Region::U)
                        continue;
                    const Value *Base = canonicalArrayBase(GEP);
                    noteWritten(Region::U, Base);
                    Info.DriverUStores.push_back(SI);
                    if (!Info.DriverUGuard)
                        Info.DriverUGuard = guardICmpForStore(SI);
                    Info.Effects.push_back(
                        classifyStore(SI, SI->getPointerOperand(), Region::U,
                                      Base, IX));
                }
            if (auto *CAS = dyn_cast<AtomicCmpXchgInst>(&I))
            {
                /* First-wins claim in the driver preamble (kcore
                 * `alive[u]=1 -> alive[u]=0` lowers to a CAS claim): a
                 * source-owned write on the claimed element. */
                if (!CAS->getMetadata("sgpl.first_wins.claim"))
                    continue;
                if (auto *GEP = dyn_cast<GetElementPtrInst>(CAS->getPointerOperand()))
                {
                    Value *IX = primaryIndex(GEP);
                    if (!IX)
                        continue;
                    if (Prov.regionOf(IX) != Region::U &&
                        ProvU.regionOf(IX) != Region::U)
                        continue;
                    const Value *Base = canonicalArrayBase(GEP);
                    noteWritten(Region::U, Base);
                    Effect E;
                    E.Kind = EffectKind::Claim;
                    E.Reg = Region::U;
                    E.Base = Base;
                    E.Index = IX;
                    E.Origin = &I;
                    Info.Effects.push_back(E);
                    Info.HasFirstWins = true;
                    Info.DriverUClaims.push_back(CAS);
                    if (getenv("GRAPH_FRONTIER_DIAG"))
                        errs() << "[diag-driver] CAS claim base="
                               << (Base && Base->hasName() ? Base->getName()
                                                           : "_")
                               << "\n";
                }
                continue;
            }
            if (auto *LI = dyn_cast<LoadInst>(&I))
                if (auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand()))
                {
                    Value *IX = primaryIndex(GEP);
                    if (!IX)
                        continue;
                    if (Prov.regionOf(IX) != Region::U &&
                        ProvU.regionOf(IX) != Region::U)
                        continue;
                    Effect E;
                    E.Kind = EffectKind::R;
                    E.Reg = Region::U;
                    E.Base = canonicalArrayBase(GEP);
                    E.Index = IX;
                    E.Origin = LI;
                    Info.Effects.push_back(E);
                }
        }
    }

    /* Temporal: same-location RW vs cross-endpoint carried dependence. */
    for (Effect &Rd : Info.Effects)
    {
        if (Rd.Kind != EffectKind::R || !Rd.Base)
            continue;
        Temporal Worst = Temporal::Independent;
        for (const Effect &M : Info.Effects)
        {
            if (!effectIsMutating(M) || M.Kind == EffectKind::Activate)
                continue;
            if (M.Base != Rd.Base)
                continue;
            if (M.Reg == Rd.Reg)
            {
                if (temporalRank(Worst) < temporalRank(Temporal::SameRoundRead))
                    Worst = Temporal::SameRoundRead;
            }
            else if ((Rd.Reg == Region::U && M.Reg == Region::V) ||
                     (Rd.Reg == Region::V && M.Reg == Region::U))
            {
                /* dest-owned write does not make R(A,U)+W(A,V) automatically
                 * safe.  A frontier F_t means R(A,U) saw a previous round's
                 * dest write.  W(A,U)+R(A,V) is staged DualOwner control
                 * (k-core alive) — not concurrent carried. */
                if (Rd.Reg == Region::U && M.Reg == Region::V)
                {
                    Temporal T = Info.MembershipGated ? Temporal::PreviousRoundRead
                                                      : Temporal::Carried;
                    if (temporalRank(T) > temporalRank(Worst))
                        Worst = T;
                }
                else if (temporalRank(Worst) < temporalRank(Temporal::SameRoundRead))
                    Worst = Temporal::SameRoundRead;
            }
        }
        Rd.Temp = Worst;
    }

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
    DualOwner,
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

static bool crossPhaseDataDep(const NeighborLoopInfo &Info,
                              const SmallPtrSet<const Value *, 4> &BaseU,
                              const SmallPtrSet<const Value *, 4> &BaseV)
{
    for (const Effect &E : Info.Effects)
    {
        if (!effectIsMutating(E) || E.Kind == EffectKind::Activate || !E.Origin)
            continue;
        auto *SI = dyn_cast<StoreInst>(E.Origin);
        if (!SI)
            continue;
        Value *Stored = SI->getValueOperand();
        if (E.Reg == Region::V)
            for (const Value *B : BaseU)
                if (valueDependsOnBase(Stored, B))
                    return true;
        if (E.Reg == Region::U)
            for (const Value *B : BaseV)
                if (valueDependsOnBase(Stored, B))
                    return true;
    }
    return false;
}

static Klass classify(const NeighborLoopInfo &Info)
{
    /* Semantic Mixed: incompatible ownership, carried same-array flow, or a
     * cross-phase data dependence.  DualOwner is E=EU∪EV with disjoint bases
     * and Dependence ⊆ Control/Membership. */
    if (Info.HasDataWrite)
        return Klass::Sequential;

    bool MutU = false, MutV = false, MutG = false, MutD = false, MutTop = false;
    bool HasUopG = false, HasCarriedOnMut = false;
    SmallPtrSet<const Value *, 4> BaseU, BaseV;
    for (const Effect &E : Info.Effects)
    {
        if (E.Kind == EffectKind::R)
        {
            if (E.Temp != Temporal::Carried || !E.Base)
                continue;
            for (const Effect &M : Info.Effects)
            {
                if (!effectIsMutating(M) || M.Kind == EffectKind::Activate)
                    continue;
                if (M.Base == E.Base && M.Reg != E.Reg)
                    HasCarriedOnMut = true;
            }
            continue;
        }
        if (E.Kind == EffectKind::Activate)
        {
            MutV = true;
            continue;
        }
        switch (E.Reg)
        {
        case Region::U:
            MutU = true;
            if (E.Base)
                BaseU.insert(E.Base);
            break;
        case Region::V:
            MutV = true;
            if (E.Base)
                BaseV.insert(E.Base);
            break;
        case Region::G:
            MutG = true;
            if (E.Kind == EffectKind::Uop)
                HasUopG = true;
            break;
        case Region::D:
            MutD = true;
            break;
        case Region::Bottom:
            break;
        default:
            MutTop = true;
            break;
        }
    }

    if (MutD || MutTop)
        return Klass::Sequential;
    if (HasCarriedOnMut)
        return Klass::Sequential;

    if (MutU && MutV && !MutG)
    {
        bool Disjoint = true;
        for (const Value *B : BaseU)
            if (BaseV.count(B))
                Disjoint = false;
        if (Disjoint && !BaseU.empty() && !BaseV.empty() &&
            !crossPhaseDataDep(Info, BaseU, BaseV))
        {
            if (Info.HasFrontierAppend && !envelopeWired(Info))
                return Klass::Sequential;
            return Klass::DualOwner;
        }
        return Klass::Sequential; /* same-array U+V or data dep */
    }

    if (Info.HasFrontierAppend)
    {
        /* Activate(V) needs the dest envelope. DualOwner already returned. */
        if (envelopeWired(Info) && MutV && !MutU && !MutG)
            return Klass::DestOwner;
        return Klass::Sequential;
    }
    if (MutG && HasUopG && !MutU && !MutV)
        return Klass::Reduction;
    if (MutV && !MutU && !MutG)
        return Klass::DestOwner;
    if (MutU && !MutV && !MutG)
        return Klass::SourceOwner;
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
    case Klass::DualOwner:
        return "dual-owner";
    default:
        return "sequential";
    }
}

static const char *compatName(Klass K)
{
    switch (K)
    {
    case Klass::DualOwner:
        return "dual";
    case Klass::Sequential:
        return "no";
    default:
        return "single";
    }
}

static void printEffects(const NeighborLoopInfo &Info, Klass K)
{
    bool First = true;
    Temporal Worst = Temporal::Independent;
    for (const Effect &E : Info.Effects)
    {
        if (E.Kind == EffectKind::R && E.Temp == Temporal::Independent)
            continue;
        if (!First)
            errs() << " ⊗ ";
        First = false;
        errs() << effectKindName(E.Kind);
        if (E.Kind == EffectKind::Uop && E.Op != RedOp::None)
            errs() << redOpName(E.Op);
        errs() << "(";
        if (E.Base && E.Base->hasName())
            errs() << E.Base->getName();
        else
            errs() << "_";
        errs() << "," << regionName(E.Reg) << ")";
        if (E.Kind == EffectKind::R && E.Temp != Temporal::Independent)
            errs() << ":" << temporalName(E.Temp);
        if (temporalRank(E.Temp) > temporalRank(Worst))
            Worst = E.Temp;
    }
    if (First)
        errs() << "(empty)";
    errs() << "  temporal=" << temporalName(Worst)
           << "  compat=" << compatName(K);
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

enum class PairPhase
{
    All,
    UOnly,
    VOnly
};

static bool mutatingOriginIs(const NeighborLoopInfo &Info, Instruction *I, Region R)
{
    for (const Effect &E : Info.Effects)
        if (E.Origin == I && E.Reg == R && effectIsMutating(E) &&
            E.Kind != EffectKind::Activate)
            return true;
    return false;
}

static bool neighborHasUMutating(const NeighborLoopInfo &Info)
{
    Loop *NL = Info.NeighborLoop;
    if (!NL)
        return false;
    for (const Effect &E : Info.Effects)
        if (effectIsMutating(E) && E.Reg == Region::U && E.Origin &&
            E.Kind != EffectKind::Activate && NL->contains(E.Origin->getParent()))
            return true;
    return false;
}

struct FrontierEnv
{
    Value *GraphArg = nullptr;
    Value *FrontArg = nullptr;
    Value *FrontSize = nullptr;
    Value *Membership = nullptr;
    Value *NextArg = nullptr;
    Value *SeenArg = nullptr;
    Value *WorkEnv = nullptr;
    bool HasEnvelope = false;
};

static void fillFrontierEnv(IRBuilder<> &EB, const NeighborLoopInfo &Info,
                            Module *Mod, Type *I8P, Type *I32, LLVMContext &Ctx,
                            Value *GraphArg, FrontierEnv &Env, bool WantEnvelope)
{
    Env.GraphArg = GraphArg;
    Env.FrontArg = ConstantPointerNull::get(cast<PointerType>(I8P));
    Env.FrontSize = ConstantInt::get(I32, 0);
    Env.WorkEnv = ConstantPointerNull::get(cast<PointerType>(I8P));
    Env.Membership = ConstantPointerNull::get(cast<PointerType>(I8P));
    Env.NextArg = ConstantPointerNull::get(cast<PointerType>(I8P));
    Env.SeenArg = ConstantPointerNull::get(cast<PointerType>(I8P));
    Env.HasEnvelope = false;
    const bool FillFt = WantEnvelope || Info.MembershipGated;
    if (!FillFt)
        return;
    FunctionCallee MemFn = Mod->getOrInsertFunction(
        "autograph_scratch_membership",
        FunctionType::get(PointerType::get(Type::getInt8Ty(Ctx), 0), {I8P}, false));
    Env.Membership = EB.CreateBitCast(EB.CreateCall(MemFn, {GraphArg}), I8P);
    if (WantEnvelope)
    {
        FunctionCallee SeenFn = Mod->getOrInsertFunction(
            "autograph_scratch_dest_seen",
            FunctionType::get(PointerType::get(I32, 0), {I8P}, false));
        Env.SeenArg = EB.CreateBitCast(EB.CreateCall(SeenFn, {GraphArg}), I8P);
        Env.WorkEnv = Env.SeenArg;
        Env.HasEnvelope = true;
        if (Info.FrontierSetPtr && Info.NextSetPtr)
        {
            FunctionCallee PrepBm = Mod->getOrInsertFunction(
                "autograph_prepare_frontier_bitmap",
                FunctionType::get(I32, {I8P, I8P}, false));
            FunctionCallee NextFn = Mod->getOrInsertFunction(
                "autograph_scratch_next_frontier",
                FunctionType::get(PointerType::get(I32, 0), {I8P}, false));
            Value *FBM = EB.CreateLoad(I8P, Info.FrontierSetPtr);
            Env.FrontSize = EB.CreateCall(PrepBm, {GraphArg, FBM});
            Env.NextArg = EB.CreateBitCast(EB.CreateCall(NextFn, {GraphArg}), I8P);
        }
        else if (Info.FrontierArrayPtr)
        {
            FunctionCallee PrepArr = Mod->getOrInsertFunction(
                "autograph_prepare_frontier_array",
                FunctionType::get(I32, {I8P, I8P, I32}, false));
            Value *FArr = ptrFromSlot(EB, Info.FrontierArrayPtr, I8P);
            Env.FrontArg = FArr;
            if (Info.FrontierSizePtr)
                Env.FrontSize = EB.CreateLoad(I32, Info.FrontierSizePtr);
            else
                Env.FrontSize = Info.FrontierSizeVal;
            Env.FrontSize = EB.CreateCall(PrepArr, {GraphArg, FArr, Env.FrontSize});
            if (Info.AppendArrayPtr)
                Env.NextArg = ptrFromSlot(EB, Info.AppendArrayPtr, I8P);
        }
    }
    else if (Info.FrontierSetPtr)
    {
        FunctionCallee PrepBm = Mod->getOrInsertFunction(
            "autograph_prepare_frontier_bitmap",
            FunctionType::get(I32, {I8P, I8P}, false));
        Value *FBM = EB.CreateLoad(I8P, Info.FrontierSetPtr);
        Env.FrontSize = EB.CreateCall(PrepBm, {GraphArg, FBM});
    }
    else if (Info.FrontierArrayPtr)
    {
        FunctionCallee PrepArr = Mod->getOrInsertFunction(
            "autograph_prepare_frontier_array",
            FunctionType::get(I32, {I8P, I8P, I32}, false));
        Value *FArr = ptrFromSlot(EB, Info.FrontierArrayPtr, I8P);
        Env.FrontArg = FArr;
        if (Info.FrontierSizePtr)
            Env.FrontSize = EB.CreateLoad(I32, Info.FrontierSizePtr);
        else
            Env.FrontSize = Info.FrontierSizeVal;
        Env.FrontSize = EB.CreateCall(PrepArr, {GraphArg, FArr, Env.FrontSize});
    }
}

static Value *callOwnerStep(IRBuilder<> &EB, Module *Mod, Type *I8P, Type *I32,
                            Function *WF, bool SourceOwner, const FrontierEnv &Env,
                            bool WithEnvelope)
{
    const char *StepName = SourceOwner ? "autograph_frontier_step_owner_source"
                                       : "autograph_frontier_step_owner_push";
    FunctionCallee Step = Mod->getOrInsertFunction(
        StepName, FunctionType::get(I32, {I8P, I8P, I32, I8P, I8P, I8P, I8P, I32,
                                          I8P}, false));
    Value *Null = ConstantPointerNull::get(cast<PointerType>(I8P));
    SmallVector<Value *, 9> Args = {
        Env.GraphArg, Env.FrontArg, Env.FrontSize,
        EB.CreateBitCast(WF, I8P),
        WithEnvelope ? Env.WorkEnv : Null,
        Env.Membership,
        WithEnvelope ? Env.NextArg : Null,
        ConstantInt::get(I32, 0),
        WithEnvelope ? Env.SeenArg : Null};
    return EB.CreateCall(Step, Args);
}

static void commitEnvelope(IRBuilder<> &EB, const NeighborLoopInfo &Info,
                           Module *Mod, Type *I8P, LLVMContext &Ctx,
                           Value *GraphArg, Value *NewSize)
{
    if (Info.FrontierSetPtr && Info.NextSetPtr)
    {
        FunctionCallee Commit = Mod->getOrInsertFunction(
            "autograph_commit_frontier_bitmap",
            FunctionType::get(Type::getVoidTy(Ctx),
                              {I8P, I8P, Type::getInt32Ty(Ctx)}, false));
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

static void deactivateDriver(const NeighborLoopInfo &Info)
{
    BasicBlock *Header = Info.DriverLoop->getHeader();
    if (Header)
        if (auto *HB = dyn_cast<BranchInst>(Header->getTerminator()))
            if (HB->isConditional())
                HB->setSuccessor(0, HB->getSuccessor(1));
}

static Function *emitPairWorkFn(const NeighborLoopInfo &Info, PairPhase Phase)
{
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    LLVMContext &Ctx = F->getContext();
    Module *Mod = F->getParent();

    /* Body = inner-loop blocks minus its header. */
    SmallVector<BasicBlock *> BodyBlocks;
    for (BasicBlock *BB : Info.NeighborLoop->getBlocks())
        if (BB != Info.NeighborLoop->getHeader())
            BodyBlocks.push_back(BB);
    const bool UPreambleOnly =
        Phase == PairPhase::UOnly && !neighborHasUMutating(Info);
    if (BodyBlocks.empty() && !UPreambleOnly)
        return nullptr;

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
    const bool UseEnvelope = Phase != PairPhase::UOnly &&
                             envelopeWired(Info) && Info.HasFrontierAppend &&
                             !Info.ReducePtr;
    Value *SeenBase = nullptr;
    if (Phase == PairPhase::All && Info.ReducePtr)
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
    if (!UPreambleOnly)
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
    if (Phase == PairPhase::All && Info.ReducePtr)
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

    auto isPhaseElide = [&](Instruction &I) -> bool
    {
        if (Phase == PairPhase::VOnly && mutatingOriginIs(Info, &I, Region::U))
            return true;
        if (Phase == PairPhase::UOnly)
        {
            if (mutatingOriginIs(Info, &I, Region::V))
                return true;
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
        }
        return false;
    };

    if (UPreambleOnly)
    {
        BasicBlock *DoU = BasicBlock::Create(Ctx, "u_effects", WF);
        Value *C = nullptr;
        if (Info.DriverUGuard)
            C = CloneValue(Info.DriverUGuard, CloneValue);
        WB.SetInsertPoint(EntryBB);
        if (C)
            WB.CreateCondBr(C, DoU, RetStub);
        else
            WB.CreateBr(DoU);
        IRBuilder<> UB(DoU);
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
            UB.Insert(Clone);
        }
        UB.CreateBr(RetStub);
        return WF;
    }

    /* Phase 1: clone non-terminator instructions into their cloned blocks so
     * Map is complete before operand remap (avoids hoisting icmps into entry). */
    for (BasicBlock *BB : BodyBlocks)
    {
        BasicBlock *CloneBB = BBMap[BB];
        for (Instruction &I : *BB)
        {
            if (I.isTerminator())
                break;
            if (isAppendElide(I) || isPhaseElide(I))
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
                    return nullptr;

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
                    return nullptr;
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
    /* Driver U-stores and first-wins claims belong on the source pair-fn
     * (DualOwner U-phase), not grafted into a dest-owned body. */
    if (Phase == PairPhase::UOnly)
    {
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
        for (AtomicCmpXchgInst *CAS : Info.DriverUClaims)
        {
            Instruction *Clone = CAS->clone();
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
    }
    WB.SetInsertPoint(EntryBB);
    WB.CreateBr(FirstClone);
    return WF;
}

static bool emitCleanCutCallbackAndStep(const NeighborLoopInfo &Info, bool SourceOwner)
{
    Function *WF = emitPairWorkFn(Info, PairPhase::All);
    if (!WF)
        return false;
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    LLVMContext &Ctx = F->getContext();
    Module *Mod = F->getParent();
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I64 = Type::getInt64Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);

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

    const bool UseEnvelope = envelopeWired(Info) && Info.HasFrontierAppend &&
                             !Info.ReducePtr;
    if (Info.ReducePtr)
    {
        Type *RedElemTy = Type::getDoubleTy(Ctx);
        for (User *U : Info.ReducePtr->users())
            if (auto *LI = dyn_cast<LoadInst>(U))
                RedElemTy = LI->getType();
        AllocaInst *Partials = EB.CreateAlloca(RedElemTy, PartCount, "red_partials");
        emitPartialInit(EB, Pre, Partials, PartCount, RedElemTy, Info.ReduceOp, Ctx);
        Function *Combiner = emitRedCombiner(Ctx, Mod, Info.ReduceOp, RedElemTy);
        FunctionCallee Step = Mod->getOrInsertFunction(
            "autograph_frontier_step_owner_red",
            FunctionType::get(I32,
                              {I8P, I8P, I32, I8P, I8P, I64, I8P, I8P, I8P, I8P, I32,
                               I8P}, false));
        SmallVector<Value *, 12> StepArgs = {
            GraphArg,
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
        EB.CreateCall(Step, StepArgs);
    }
    else
    {
        FrontierEnv Env;
        fillFrontierEnv(EB, Info, Mod, I8P, I32, Ctx, GraphArg, Env, UseEnvelope);
        Value *NewSize = callOwnerStep(EB, Mod, I8P, I32, WF, SourceOwner, Env,
                                       UseEnvelope);
        if (UseEnvelope)
            commitEnvelope(EB, Info, Mod, I8P, Ctx, GraphArg, NewSize);
    }
    deactivateDriver(Info);
    return true;
}

/* DualOwner: source step then dest step on the same pre-round F_t. */
static bool emitDualCleanCut(const NeighborLoopInfo &Info)
{
    Function *WFu = emitPairWorkFn(Info, PairPhase::UOnly);
    Function *WFv = emitPairWorkFn(Info, PairPhase::VOnly);
    if (!WFu || !WFv)
        return false;
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    LLVMContext &Ctx = F->getContext();
    Module *Mod = F->getParent();
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);

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
    EB.CreateCall(BuildCC, {GraphArg, ConstantInt::get(I32, 0)});

    const bool WantEnvelope = envelopeWired(Info) && Info.HasFrontierAppend;
    FrontierEnv Env;
    /* Membership(DestPhase)=Membership(SourcePhase)=F_t: prepare once. */
    fillFrontierEnv(EB, Info, Mod, I8P, I32, Ctx, GraphArg, Env, WantEnvelope);
    callOwnerStep(EB, Mod, I8P, I32, WFu, /*SourceOwner=*/true, Env,
                  /*WithEnvelope=*/false);
    Value *NewSize = callOwnerStep(EB, Mod, I8P, I32, WFv, /*SourceOwner=*/false,
                                   Env, WantEnvelope);
    if (WantEnvelope)
        commitEnvelope(EB, Info, Mod, I8P, Ctx, GraphArg, NewSize);
    deactivateDriver(Info);
    return true;
}

/* ── pass ──────────────────────────────────────────────────────── */

PreservedAnalyses GraphFrontierLoweringPass::run(Function &F,
                                                 FunctionAnalysisManager &FAM)
{
    if (F.isDeclaration() || F.begin() == F.end())
        return PreservedAnalyses::all();
    unsigned detected = 0;
    /* CleanCut lowering is ON by default: graph loops are always lowered to
     * the owner-computes step (or marked sequential).  GRAPH_FRONTIER_REWRITE_OFF=1
     * restores the old pipeline for benchmarking (graph loops then fall back to
     * the unconditional sequential marking). */
    const bool RewriteMode = getenv("GRAPH_FRONTIER_REWRITE_OFF") == nullptr;
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
                   << " class=" << klassName(K)
                   << "  ";
            if (IsIter)
                printEffects(Info, K);
            errs() << "\n";
        }
        if (!RewriteMode)
        {
            /* Safe by default: the CleanCut rewrite is env-gated, but a graph
             * loop must never be handed blindly to the racy DOALL path — mark
             * it sequential unconditionally. */
            markSequential(L);
            ++detected;
            continue;
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
                bool Emitted = (K == Klass::DualOwner)
                                   ? emitDualCleanCut(Info)
                                   : emitCleanCutCallbackAndStep(
                                         Info, K == Klass::SourceOwner);
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
