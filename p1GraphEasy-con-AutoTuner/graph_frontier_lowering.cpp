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
#include "llvm/IR/Dominators.h"
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
#include <cassert>
#include <cstring>

#include "autotuner_runtime.h"

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
 * (owner_red partials+combine) path.
 *
 * The combine must reproduce the *body's* operation exactly, so the flavours
 * are kept apart instead of being collapsed into "min"/"max": an unsigned
 * minimum combined with a signed one, or a NaN-propagating minimum combined
 * with minnum, is a wrong answer that only shows on the edge values, and the
 * identity element differs too (UINT_MAX vs INT_MAX). */
enum class RedOp
{
    None = 0,
    Add,
    Sub,
    Mul,
    Min,      /* signed integer min: llvm.smin, select(icmp slt/sle) */
    Max,      /* signed integer max: llvm.smax, select(icmp sgt/sge) */
    MinU,     /* unsigned integer min: llvm.umin, select(icmp ult/ule) */
    MaxU,     /* unsigned integer max: llvm.umax, select(icmp ugt/uge) */
    FMinNum,  /* float minnum (ignores NaN) */
    FMaxNum,  /* float maxnum */
    FMinProp, /* float minimum (propagates NaN) */
    FMaxProp, /* float maximum */
    And,
    Or,
    Xor,
    FirstWins
};

/* Self-describing reduction operator ω = ⟨f_body, N_ω, f_fold, 1_ω, L_ω⟩.
 *   f_body (BodyOp)     the update operation recognized in the loop body;
 *   N_ω    (Norm)       normalizes each body contribution before folding:
 *                       Identity for most operators, Negated for subtraction
 *                       (acc := acc - x contributes -x, so the fold is an
 *                       addition over the negated partial sums);
 *   f_fold (FoldOp)     combines partition partials;
 *   L_ω    (Assoc/Comm/Idempotent) laws of f_fold in exact semantics.
 * The minimal law for ordered partition folding is associativity; commutativity
 * is recorded but only needed when contributions may be permuted.  The
 * fold-homomorphism theorem
 *     fold_ω(concat(S_1..S_P)) == fold_ω(fold_ω(S_1)..fold_ω(S_P))
 * is gated by FoldSound(ω,T) below (element-type- and machine-dependent, e.g.
 * float +/* fail it under bit-exact semantics).
 * The table MUST stay in RedOp declaration order. */
enum class RedNorm : uint8_t
{
    Identity = 0,
    Negated
};

struct RedOpInfo
{
    RedOp BodyOp = RedOp::None;
    RedNorm Norm = RedNorm::Identity;
    RedOp FoldOp = RedOp::None;
    bool Assoc = false;
    bool Comm = false;
    bool Idempotent = false;
};

static const RedOpInfo &redOpInfo(RedOp Op)
{
    static const RedOpInfo Table[] = {
        /* BodyOp           Norm                 FoldOp           Assoc  Comm   Idem */
        {RedOp::None,       RedNorm::Identity,   RedOp::None,     false, false, false},
        {RedOp::Add,        RedNorm::Identity,   RedOp::Add,      true,  true,  false},
        {RedOp::Sub,        RedNorm::Negated,    RedOp::Add,      true,  true,  false},
        {RedOp::Mul,        RedNorm::Identity,   RedOp::Mul,      true,  true,  false},
        {RedOp::Min,        RedNorm::Identity,   RedOp::Min,      true,  true,  true},
        {RedOp::Max,        RedNorm::Identity,   RedOp::Max,      true,  true,  true},
        {RedOp::MinU,       RedNorm::Identity,   RedOp::MinU,     true,  true,  true},
        {RedOp::MaxU,       RedNorm::Identity,   RedOp::MaxU,     true,  true,  true},
        {RedOp::FMinNum,    RedNorm::Identity,   RedOp::FMinNum,  true,  true,  true},
        {RedOp::FMaxNum,    RedNorm::Identity,   RedOp::FMaxNum,  true,  true,  true},
        {RedOp::FMinProp,   RedNorm::Identity,   RedOp::FMinProp, true,  true,  true},
        {RedOp::FMaxProp,   RedNorm::Identity,   RedOp::FMaxProp, true,  true,  true},
        {RedOp::And,        RedNorm::Identity,   RedOp::And,      true,  true,  true},
        {RedOp::Or,         RedNorm::Identity,   RedOp::Or,       true,  true,  true},
        {RedOp::Xor,        RedNorm::Identity,   RedOp::Xor,      true,  true,  false},
        {RedOp::FirstWins,  RedNorm::Identity,   RedOp::FirstWins,false, false, true},
    };
    unsigned I = (unsigned)Op;
    if (I >= sizeof(Table) / sizeof(Table[0]))
        return Table[0];
    return Table[I];
}

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

/* Occurrence scope κ: the iteration domain over which an effect is
 * instantiated.  A rewrite must preserve the scope (a per-source claim may not
 * become a per-pair claim), which is the algebraic form of the driver-preamble
 * refusals.  κ is a diagnostic projection; its semantic source is the binder
 * path of the effect in the expression.  `PerRound` is retired: round-snapshot
 * lifetime is a value-source property (σ), not an occurrence scope. */
enum class OccurrenceScope : uint8_t
{
    Once = 0,
    PerSource,
    PerPair
};

/* Value source σ: where a read observes its value.  Applies to read effects
 * only (`Kind != R ⇒ σ = None`).  `Snapshot` marks a read resolved by a
 * round-separation shadow snapshot (the implementation witness is membership
 * in a RoundSepBase::CrossReads set); `Live` is every other read.  σ is
 * assigned after round separation has been established and is never an input
 * to the temporal relation τ. */
enum class ValueSource : uint8_t
{
    None = 0, /* non-read effects */
    Live,
    Snapshot
};

/* Region-boundary invariant: provenance's codomain is {Bottom,U,V,D,Top} —
 * provenance never produces G.  G is introduced only while constructing
 * effects for scalar/global accumulator locations (no vertex index).  This
 * helper makes the boundary explicit at provenance use sites. */
static bool isProvRegion(Region R)
{
    return R != Region::G;
}

struct Effect
{
    EffectKind Kind = EffectKind::R;
    Region Reg = Region::Bottom;
    const Value *Base = nullptr;
    RedOp Op = RedOp::None;
    Temporal Temp = Temporal::Independent;
    OccurrenceScope Scope = OccurrenceScope::PerPair;
    ValueSource VSource = ValueSource::None;
    /* Ordered claim primitive C(A,r,γ,δ,≺): γ (guard) is the expected value the
     * branch compares against the location; δ (transition) is the stored
     * desired value.  ≺ is the serial priority relation (the CSR scan order
     * the owner-computes runtime discharges); it is an obligation, not a
     * global algebraic law. */
    Value *ClaimGuard = nullptr;
    Value *ClaimTransition = nullptr;
    Value *Index = nullptr;
    Instruction *Origin = nullptr;
};

static bool effectIsMutating(const Effect &E)
{
    return E.Kind != EffectKind::R;
}

/* ── effect expressions ─────────────────────────────────────────
 * The effect expression is a structural tree (arena of EffNode) whose shape is
 * the original computation of one round:
 *
 *     E_t = SeqDomain_{u∈F_t}( P(u) ; SeqDomain_{v∈N(u)} B(u,v) ; Q(u) )
 *
 * with P the per-source preamble, B the per-pair neighbour body and Q the
 * per-source finish.  `Par` is structural concurrent composition (the two
 * children are concurrent and their syntactic order carries no temporal
 * meaning); primitives within a phase are composed with Par, and the phase
 * sequence P ; B ; Q is the only source of intra-round order.  Domain binders
 * carry their semantic domain: AllVertices, Frontier(t) or Neighbors(u).
 *
 * `ParDomain` and `Star` are grammar-complete constructors that the compiler
 * never generates (parallelization is selected by the interpretation, and
 * rounds are explicit sequences); paths that meet them assert in debug builds.
 *
 * The flat EffectExpr segments below are a derived projection of the tree
 * (flatten modulo Par permutation) kept for diagnostics and for the emission
 * consumers that read whole segments; the tree is the semantic source. */
enum class EffNodeKind : uint8_t
{
    Empty = 0, /* ε */
    Prim,      /* one primitive effect */
    Seq,       /* structural sequential composition (binary) */
    Par,       /* structural concurrent composition (binary) */
    SeqDomain, /* sequential iteration domain */
    ParDomain, /* concurrent iteration domain (grammar-only) */
    Star       /* finite iteration (grammar-only) */
};

enum class DomainKind : uint8_t
{
    AllVertices = 0, /* u ∈ V */
    Frontier,        /* u ∈ F_t */
    Neighbors        /* v ∈ N(u) */
};

enum class PhaseSegment : uint8_t
{
    None = 0,
    Preamble,
    Pair,
    Epilogue
};

struct EffNode
{
    EffNodeKind Kind = EffNodeKind::Empty;
    DomainKind Domain = DomainKind::AllVertices; /* Domain nodes */
    const Effect *Prim = nullptr;                /* Prim nodes */
    PhaseSegment Seg = PhaseSegment::None;       /* Prim nodes */
    unsigned L = 0, R = 0;                       /* children (arena indices) */
};

struct EffectExpr
{
    /* Structural tree (arena; node 0 is ε).  Root is the round expression. */
    SmallVector<EffNode, 32> Arena;
    unsigned Root = 0;
    DomainKind RoundDomain = DomainKind::AllVertices; /* outer binder kind */
    bool MembershipGated = false; /* derived: RoundDomain == Frontier */
    /* Flat derived projection (segment multisets). */
    SmallVector<const Effect *, 8> Preamble;
    SmallVector<const Effect *, 16> Pair;
    SmallVector<const Effect *, 4> Epilogue;
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
    /* Composition R3 (privatization): a scalar slot beyond the first, and the
     * arrays whose in-loop stores are all recognized U_⊕ updates on a base
     * that has no single owner region.  Filled by privLayout(). */
    struct SlotInfo
    {
        const Value *Ptr = nullptr;
        RedOp Op = RedOp::None;
        Type *ElemTy = nullptr;
    };
    struct ArrayInfo
    {
        const Value *Base = nullptr; /* canonical base: the GEP redirection key */
        Value *Ptr = nullptr;         /* the array itself (alloca / global) */
        RedOp Op = RedOp::None;
        Type *ElemTy = nullptr;
        Value *Count = nullptr; /* element count (constant or SSA) */
    };
    SmallVector<SlotInfo, 2> Slots;
    SmallVector<ArrayInfo, 2> PrivArrays;
    bool UsePrivLayout = false; /* verdict came from privLayout(): emit must
                                 * use the per-partition copies */
    /* Composition I (per-source gather): the driver preamble consumes the
     * scalar accumulator once per source (`arr[u] = acc`) after resetting it,
     * so the reduction is per source, not per loop. */
    StoreInst *AccConsumeStore = nullptr; /* the `arr[u] = acc` store */
    bool AccResetSeen = false;            /* `acc = <invariant>` in the preamble */

    /* In-place: an array both read and written on an endpoint region — a
     * round-separation (in-place) overlap.  Informational; for single-endpoint
     * (owned) writes the owner-computes step is race-free regardless, so this
     * no longer forces sequential (see classify()). */
    bool NeedsRoundSep = false;

    /* Round-separation shadow bases (composition A): an in-place array read on
     * one endpoint region and written on the other (same base, cross-endpoint
     * R x W).  Such loops emit a shadow snapshot (per-round memcpy into a
     * runtime scratch) and redirect every read on the base to the shadow so
     * the round reads a frozen round-start snapshot — the owner-computes step
     * itself stays untouched.  WritesV selects DestOwner vs SourceOwner. */
    struct RoundSepBase
    {
        const Value *Base = nullptr; /* canonical base (global slot) */
        Type *ElemTy = nullptr;      /* i32 or double */
        bool WritesV = true;         /* writes indexed by V (dest-owned) */
        /* Loads on the base that read the OPPOSITE endpoint region
         * (cross-endpoint R x W) — these are redirected to the shadow
         * snapshot.  Same-region RMW reads stay LIVE: they are the running
         * accumulator on the owner's own slot, and freezing them would turn
         * min/max/+=-style relaxes into last-write-wins. */
        SmallPtrSet<const LoadInst *, 8> CrossReads;
    };
    SmallVector<RoundSepBase, 2> RoundSepBases;

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
    bool HasDriverClaim = false;      /* first-wins claim in the driver preamble
                                       * (store form of `alive[u]=1 -> 0`) */
    bool HasRecognizedOp = false;     /* body has a known Update operator */
    SmallVector<StoreInst *, 4> DriverUStores; /* per-source preamble (alive[u]=0) */
    SmallVector<AtomicCmpXchgInst *, 4> DriverUClaims; /* first-wins claims */
    SmallVector<Effect, 8> Effects;   /* primitive effect set E */
    EffectExpr Expr;                  /* E_t as a segmented expression */
    ICmpInst *DriverUGuard = nullptr; /* optional if (pred(u)) wrapping U-stores */
    bool MembershipGated = false;     /* driver is a frontier / F_t iteration */
};

static void buildEffectExpr(NeighborLoopInfo &Info);
static void deriveAllTemporal(NeighborLoopInfo &Info);

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
                    assert(isProvRegion(R) &&
                           "provenance codomain is {Bottom,U,V,D,Top}; G is "
                           "effect-level only");
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

/* True when this GEP is the root of an address computation on its array, i.e.
 * it indexes the base itself (or a load/cast of the base slot) rather than the
 * result of another GEP.  Re-basing an address onto a private copy is only
 * valid at the root: doing it again on a chained GEP would apply the leading
 * offset twice. */
static bool gepIsArrayRoot(const GetElementPtrInst *GEP)
{
    return !isa<GetElementPtrInst>(GEP->getPointerOperand()->stripPointerCasts());
}

/* True when A and B are the same address computation.
 *
 * The front end does not CSE, so one source-level slot routinely appears as
 * several distinct SSA values: `load @cnt; gep; load elem` is emitted again for
 * the store that follows, and again for the index it uses.  Identity, or one
 * level of `load ... same pointer`, therefore misses genuine read-modify-writes
 * (`cnt[deg[u]] = cnt[deg[u]] + 1` lowers to two GEPs whose indices are two
 * loads of the same element).  The relation is the standard structural one,
 * applied recursively: same value; two loads from the same address; two GEPs
 * with the same base and equal indices; two casts of the same kind over equal
 * operands.  Anything else is not the same location. */
static bool sameAddressValue(Value *A, Value *B, unsigned Depth)
{
    if (A == B)
        return true;
    if (!A || !B || Depth > 6 || A->getType() != B->getType())
        return false;
    if (auto *LA = dyn_cast<LoadInst>(A))
        if (auto *LB = dyn_cast<LoadInst>(B))
            return sameAddressValue(LA->getPointerOperand(), LB->getPointerOperand(),
                                    Depth + 1);
    if (auto *GA = dyn_cast<GetElementPtrInst>(A))
        if (auto *GB = dyn_cast<GetElementPtrInst>(B))
        {
            if (GA->getNumIndices() != GB->getNumIndices())
                return false;
            if (!sameAddressValue(GA->getPointerOperand(), GB->getPointerOperand(),
                                  Depth + 1))
                return false;
            auto Ia = GA->idx_begin(), Ib = GB->idx_begin();
            for (; Ia != GA->idx_end(); ++Ia, ++Ib)
                if (!sameAddressValue(*Ia, *Ib, Depth + 1))
                    return false;
            return true;
        }
    if (auto *CA = dyn_cast<CastInst>(A))
        if (auto *CB = dyn_cast<CastInst>(B))
            if (CA->getOpcode() == CB->getOpcode())
                return sameAddressValue(CA->getOperand(0), CB->getOperand(0),
                                        Depth + 1);
    return false;
}

/* True if A and B address the same array element, even when they are distinct
 * GEP instructions (the common non-CSE'd `load slot; gep; load/store` shape). */
static bool sameIndexVal(Value *A, Value *B)
{
    return sameAddressValue(A, B, 0);
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
            return RedOp::Min;
        case Intrinsic::umin:
            return RedOp::MinU;
        case Intrinsic::minnum:
            return RedOp::FMinNum;
        case Intrinsic::minimum:
            return RedOp::FMinProp;
        case Intrinsic::smax:
            return RedOp::Max;
        case Intrinsic::umax:
            return RedOp::MaxU;
        case Intrinsic::maxnum:
            return RedOp::FMaxNum;
        case Intrinsic::maximum:
            return RedOp::FMaxProp;
        default:
            return RedOp::None;
        }
    }

    /* min/max as `select(icmp pred a, b, a, b)`.  The predicate decides the
     * flavour: slt/sle is a signed minimum, ult/ule an unsigned one, and the
     * combine has to use the matching intrinsic (the identity differs too).
     *
     * The float form `select(fcmp olt a, b, a, b)` is deliberately NOT
     * recognized.  Its value is not reorder-invariant once a NaN or a signed
     * zero is in the stream (the select returns its false operand when the
     * comparison is unordered), so folding partition partials with it cannot
     * reproduce the serial left-to-right fold.  llvm.minnum/maxnum (what
     * InstCombine usually produces for this shape) *are* order-invariant and
     * are recognized above; the raw select shape stays sequential. */
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
        bool IsMin = tIsA ? isLT : isGT;
        bool Unsigned = ICmpInst::isUnsigned(P);
        if (IsMin)
            return Unsigned ? RedOp::MinU : RedOp::Min;
        return Unsigned ? RedOp::MaxU : RedOp::Max;
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
    bool IsMin = ICmpInst::isLT(P) || ICmpInst::isLE(P);
    bool IsMax = ICmpInst::isGT(P) || ICmpInst::isGE(P);
    if (!IsMin && !IsMax)
        return RedOp::None;
    bool Unsigned = ICmpInst::isUnsigned(P);
    if (IsMin)
        return Unsigned ? RedOp::MinU : RedOp::Min;
    return Unsigned ? RedOp::MaxU : RedOp::Max;
}

/* First-wins claim: `if (A[i] == expected) A[i] = desired;` with expected !=
 * desired.  Dest-owned, so the claim is race-free under owner-computes (CSR
 * source order decides the winner).  This is the ordered conditional primitive
 * of the claim algebra: `Expected`/`Desired` are its guard operands, and the
 * priority is the serial scan order the runtime discharges. */
static bool detectFirstWinsClaim(StoreInst *SI, Value *&Expected, Value *&Desired)
{
    Expected = nullptr;
    Desired = nullptr;
    Value *Ptr = SI->getPointerOperand();
    Value *DesiredV = SI->getValueOperand();
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
    Value *Other = nullptr;
    if (isLoadOfPtr(Cmp->getOperand(0)))
        Other = Cmp->getOperand(1);
    else if (isLoadOfPtr(Cmp->getOperand(1)))
        Other = Cmp->getOperand(0);
    else
        return false;
    if (Other == DesiredV)
        return false;
    Expected = Other;
    Desired = DesiredV;
    return true;
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
    case RedOp::MinU:
        return "minu";
    case RedOp::MaxU:
        return "maxu";
    case RedOp::FMinNum:
        return "fminnum";
    case RedOp::FMaxNum:
        return "fmaxnum";
    case RedOp::FMinProp:
        return "fminprop";
    case RedOp::FMaxProp:
        return "fmaxprop";
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

    /* The parent must be the graph-iteration loop itself (the DSL
     * `for each vertex` / `for each edge` nest), not an arbitrary loop between
     * it and the neighbour loop.  In `for each vertex u { ... while (k < C) {
     * for each neighbor v ... } }` the parent is the while: treating it as the
     * frontier driver deletes the while's trip count from the emitted nest and
     * runs one whole-graph engine step per outer iteration -- wrong result and
     * (20k vertices x 320k edges) an effective hang.  Graph-iteration loops are
     * emitted as canonical counted loops with an SSA induction phi; DSL `while`
     * loops keep their counter in memory, so a missing phi is the signal that
     * this parent is not a graph-iteration loop.  Canonicalisation runs after
     * this pass, so the distinction is still visible here. */
    if (!driverIndVar(Info.DriverLoop))
        return false;

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
        if (detectFirstWinsClaim(SI, E.ClaimGuard, E.ClaimTransition))
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
        if (getenv("GRAPH_FRONTIER_DIAG") && AOp == RedOp::None && R != Region::G)
        {
            errs() << "[diag-store] reg=" << (int)R << " value: "
                   << *SI->getValueOperand() << "  ptr: " << *Ptr << "\n";
            if (auto *BO = dyn_cast<BinaryOperator>(SI->getValueOperand()))
                for (Value *Op : BO->operands())
                    if (auto *LI = dyn_cast<LoadInst>(Op))
                        errs() << "[diag-load] " << *LI << "\n"
                               << "[diag-load-ptr] "
                               << *LI->getPointerOperand() << "\n";
        }
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
                    else if (Info.ReducePtr == Ptr)
                    {
                        /* Same slot, already recognized: a *second*, different
                         * operator on it (`s = s + x; s = s * y;`) would be
                         * folded by the combine with one operator only, so the
                         * partition partials would be combined with the wrong
                         * op.  Record the slot as an unrecognized global effect
                         * and let classify() refuse the reduction class. */
                        RedOp Op2 = detectScalarRedOp(SI->getValueOperand(), Ptr);
                        if (Op2 == RedOp::None)
                            Op2 = detectConditionalMinMax(SI, Ptr);
                        if (Op2 != RedOp::None && Op2 != Info.ReduceOp)
                        {
                            Effect E2;
                            E2.Kind = EffectKind::Uf;
                            E2.Reg = Region::G;
                            E2.Base = Ptr;
                            E2.Index = nullptr;
                            E2.Origin = SI;
                            Info.Effects.push_back(E2);
                        }
                    }
                    else
                    {
                        /* A second, distinct scalar slot in the same body.
                         * Composition R3 gives *every* recognized accumulator
                         * its own per-partition partial, so a recognized
                         * operator here is no longer a refusal; only an
                         * unrecognized update stays one (nothing would combine
                         * it, and every partition would write the same slot). */
                        RedOp Op2 = detectScalarRedOp(SI->getValueOperand(), Ptr);
                        if (Op2 == RedOp::None)
                            Op2 = detectConditionalMinMax(SI, Ptr);
                        Effect E2;
                        E2.Reg = Region::G;
                        E2.Base = Ptr;
                        E2.Index = nullptr;
                        E2.Origin = SI;
                        if (Op2 != RedOp::None)
                        {
                            E2.Kind = EffectKind::Uop;
                            E2.Op = Op2;
                        }
                        else
                        {
                            E2.Kind = EffectKind::Uf;
                        }
                        Info.Effects.push_back(E2);
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
            /* Composition I / P10: the driver preamble's per-source epilogue
             * consumes the scalar accumulator (`arr[u] = acc`) and resets it
             * (`acc = 0`) before the neighbour loop.  Recorded here, but the
             * consume store keeps its ordinary U effect: that is what makes the
             * loop refuse the plain reduction class (a per-loop partial would
             * fold every source together and never run the epilogue). */
            if (auto *SI = dyn_cast<StoreInst>(&I))
            {
                if (Info.ReducePtr &&
                    storedDependsOnOldValue(SI->getValueOperand(), Info.ReducePtr) &&
                    dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
                {
                    if (auto *CGEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand()))
                    {
                        Value *CIX = primaryIndex(CGEP);
                        if (CIX && (Prov.regionOf(CIX) == Region::U ||
                                    ProvU.regionOf(CIX) == Region::U))
                            Info.AccConsumeStore = SI;
                    }
                }
                else if (Info.ReducePtr &&
                         SI->getPointerOperand()->stripPointerCasts() ==
                             Info.ReducePtr &&
                         !storedDependsOnOldValue(SI->getValueOperand(),
                                                  Info.ReducePtr))
                {
                    Info.AccResetSeen = true;
                }
            }
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
                    Effect DE = classifyStore(SI, SI->getPointerOperand(),
                                              Region::U, Base, IX);
                    DE.Scope = OccurrenceScope::PerSource;
                    if (DE.Kind == EffectKind::Claim)
                        Info.HasDriverClaim = true;
                    Info.Effects.push_back(DE);
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
                    E.Scope = OccurrenceScope::PerSource;
                    E.ClaimGuard = CAS->getCompareOperand();
                    E.ClaimTransition = CAS->getNewValOperand();
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
                    E.Scope = OccurrenceScope::PerSource;
                    Info.Effects.push_back(E);
                }
        }
    }

    /* Temporal: derived from the expression tree (segment identity + round
     * domain), independent of σ and RoundSepBases. */
    buildEffectExpr(Info);
    deriveAllTemporal(Info);

    /* Round-separation bases (composition A): a base read on one endpoint
     * region and written on the other.  These need a shadow snapshot so the
     * pair work fn reads a frozen round-start value instead of a value a
     * sibling partition is concurrently writing.  Eligibility:
     *   - same base has both an R effect and a mutating effect;
     *   - the mutating effects on the base are single-region (all U or all V,
     *     none G/D) — mixed U+V writes on one base is composition C;
     *   - the reads include the opposite region (cross-endpoint);
     *   - the element type is uniform and i32/double. */
    if (Info.RoundSepBases.empty())
    {
        SmallPtrSet<const Value *, 4> MutBases, ReadBases;
        for (const Effect &E : Info.Effects)
        {
            if (!E.Base)
                continue;
            if (E.Kind == EffectKind::R)
                ReadBases.insert(E.Base);
            else if (E.Kind != EffectKind::Activate)
                MutBases.insert(E.Base);
        }
        for (const Value *Base : MutBases)
        {
            if (!ReadBases.count(Base))
                continue;
            /* Write region + uniform type on this base. */
            Region WReg = Region::Bottom;
            bool MixedWrites = false;
            Type *ElemTy = nullptr;
            for (const Effect &M : Info.Effects)
            {
                if (M.Base != Base || M.Kind == EffectKind::R ||
                    M.Kind == EffectKind::Activate)
                    continue;
                if (M.Reg != Region::U && M.Reg != Region::V)
                {
                    MixedWrites = true;
                    break;
                }
                if (WReg == Region::Bottom)
                    WReg = M.Reg;
                else if (WReg != M.Reg)
                {
                    MixedWrites = true;
                    break;
                }
                if (auto *SI = dyn_cast<StoreInst>(M.Origin))
                {
                    Type *T = SI->getValueOperand()->getType();
                    if (!ElemTy)
                        ElemTy = T;
                    else if (ElemTy != T)
                        MixedWrites = true;
                }
            }
            if (MixedWrites || WReg == Region::Bottom)
                continue;
            /* Reads: need the opposite region, uniform i32/double type. */
            bool HasCross = false;
            Type *ReadTy = nullptr;
            bool BadRead = false;
            for (const Effect &Rd : Info.Effects)
            {
                if (Rd.Kind != EffectKind::R || Rd.Base != Base)
                    continue;
                if (Rd.Reg != WReg)
                    HasCross = true;
                auto *LI = dyn_cast<LoadInst>(Rd.Origin);
                Type *T = LI ? LI->getType() : nullptr;
                if (!T)
                {
                    BadRead = true;
                    break;
                }
                if (!ReadTy)
                    ReadTy = T;
                else if (ReadTy != T)
                {
                    BadRead = true;
                    break;
                }
            }
            if (BadRead || !HasCross)
                continue;
            if (!ReadTy || (!ReadTy->isIntegerTy(32) && !ReadTy->isDoubleTy()))
                continue;
            NeighborLoopInfo::RoundSepBase RS;
            RS.Base = Base;
            RS.ElemTy = ReadTy;
            RS.WritesV = (WReg == Region::V);
            /* Cross-endpoint reads on the base: every load whose index origin
             * differs from the write region.  Same-region RMW reads (the
             * accumulator) stay live. */
            for (BasicBlock *BB : L->blocks())
                for (Instruction &I : *BB)
                    if (auto *LI = dyn_cast<LoadInst>(&I))
                        if (auto *GEP =
                                dyn_cast<GetElementPtrInst>(LI->getPointerOperand()))
                            if (canonicalArrayBase(GEP) == Base)
                            {
                                Value *IX = primaryIndex(GEP);
                                Region R = IX ? Prov.regionOf(IX) : Region::Bottom;
                                if (R != WReg)
                                    RS.CrossReads.insert(LI);
                            }
            Info.RoundSepBases.push_back(RS);
        }
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

    /* Value source σ (assigned after round separation, never an input to τ):
     * a read resolved by a round-separation shadow snapshot is Snapshot(t);
     * every other read is Live.  The CrossReads sets are the witnesses. */
    for (Effect &E : Info.Effects)
    {
        if (E.Kind != EffectKind::R)
        {
            E.VSource = ValueSource::None;
            continue;
        }
        bool Snapshot = false;
        if (auto *LI = dyn_cast_or_null<LoadInst>(E.Origin))
            for (const auto &RS : Info.RoundSepBases)
                if (RS.Base == E.Base && RS.CrossReads.count(LI))
                    Snapshot = true;
        E.VSource = Snapshot ? ValueSource::Snapshot : ValueSource::Live;
    }
    return true;
}

/* ── expression construction and temporal derivation ─────────────
 * buildEffectExpr partitions the primitive set into the preamble/pair/
 * epilogue segments by occurrence scope (the driver preamble carries
 * PerSource effects; the consume store is the per-source epilogue).  The
 * temporal relation of every read is then derived from segment and round
 * structure:
 *
 *   R,W in the same segment (same round, concurrent)   → SameRoundRead
 *   W precedes R across segments (preamble→pair/consume) → SameRoundRead
 *   R(A,U) in the pair with a V write to A             → PreviousRoundRead
 *                                                         (membership-gated
 *                                                          round boundary) or
 *                                                         Carried (ungated)
 *
 * The pairwise scan was retired at the Phase-3 cutover (the differential
 * harness reported zero disagreements); deriveTemporalWorst is the sole τ. */
/* ── expression construction ─────────────────────────────────────
 * The tree is built from the (already classified) primitive effects:
 *
 *     Root = SeqDomain(RoundDomain,
 *              Seq(ParTree(P prims),
 *                  Seq(SeqDomain(Neighbors, ParTree(B prims)),
 *                      ParTree(Q prims))))
 *
 * Par trees are right-nested for deterministic flattening; their child order
 * carries no temporal meaning.  The flat segments are filled by flattening the
 * tree, which also checks that every primitive appears exactly once. */
static unsigned newEffNode(EffectExpr &X, const EffNode &N)
{
    X.Arena.push_back(N);
    return (unsigned)X.Arena.size() - 1;
}

static unsigned buildParTree(EffectExpr &X, ArrayRef<const Effect *> Prims,
                             PhaseSegment Seg)
{
    if (Prims.empty())
        return 0; /* ε */
    unsigned Cur = newEffNode(X, EffNode{EffNodeKind::Prim, DomainKind::AllVertices,
                                         Prims[0], Seg, 0, 0});
    for (unsigned i = 1; i < Prims.size(); ++i)
    {
        unsigned P = newEffNode(X, EffNode{EffNodeKind::Prim, DomainKind::AllVertices,
                                           Prims[i], Seg, 0, 0});
        Cur = newEffNode(X, EffNode{EffNodeKind::Par, DomainKind::AllVertices,
                                    nullptr, PhaseSegment::None, Cur, P});
    }
    return Cur;
}

/* Flatten the tree, asserting that the only unreachable constructors
 * (ParDomain, Star) are never met.  The Seg tag on Prim nodes is the phase
 * segment identity; it is assigned by the builder, not inferred from Par
 * child order. */
static void flattenTree(const EffectExpr &X, unsigned Idx,
                        SmallVectorImpl<std::pair<const Effect *, PhaseSegment>> &Out)
{
    const EffNode &N = X.Arena[Idx];
    switch (N.Kind)
    {
    case EffNodeKind::Empty:
        return;
    case EffNodeKind::Prim:
        Out.push_back({N.Prim, N.Seg});
        return;
    case EffNodeKind::Seq:
    case EffNodeKind::Par:
    case EffNodeKind::SeqDomain:
    case EffNodeKind::ParDomain:
    case EffNodeKind::Star:
        assert(N.Kind != EffNodeKind::ParDomain && N.Kind != EffNodeKind::Star &&
               "ParDomain/Star are grammar-only: never generated by the compiler");
        flattenTree(X, N.L, Out);
        flattenTree(X, N.R, Out);
        return;
    }
}

/* Collect the primitive occurrences of the expression in tree order. */
static void collectTreePrims(const EffectExpr &X,
                             SmallVectorImpl<std::pair<const Effect *, PhaseSegment>> &Out)
{
    if (X.Arena.empty())
        return;
    flattenTree(X, X.Root, Out);
}

static void buildEffectExpr(NeighborLoopInfo &Info)
{
    EffectExpr &X = Info.Expr;
    X.Arena.clear();
    X.Preamble.clear();
    X.Pair.clear();
    X.Epilogue.clear();
    X.MembershipGated = Info.MembershipGated;
    X.RoundDomain = Info.MembershipGated ? DomainKind::Frontier
                                         : DomainKind::AllVertices;
    /* Construction is defined only for modelable programs (Modelable(P) in the
     * design note); this function does not carry a totality flag.  Scope has no
     * producer other than PerSource/PerPair, so every effect lands in exactly
     * one segment. */
    for (const Effect &E : Info.Effects)
    {
        if (E.Scope == OccurrenceScope::PerSource)
        {
            if (Info.AccConsumeStore && E.Origin == Info.AccConsumeStore)
                X.Epilogue.push_back(&E);
            else
                X.Preamble.push_back(&E);
        }
        else
            X.Pair.push_back(&E);
    }
    X.Arena.push_back(EffNode{}); /* node 0: ε */
    unsigned P = buildParTree(X, X.Preamble, PhaseSegment::Preamble);
    unsigned B = buildParTree(X, X.Pair, PhaseSegment::Pair);
    unsigned Q = buildParTree(X, X.Epilogue, PhaseSegment::Epilogue);
    unsigned Inner = newEffNode(X, EffNode{EffNodeKind::SeqDomain,
                                           DomainKind::Neighbors, nullptr,
                                           PhaseSegment::None, B, 0});
    unsigned Tail = newEffNode(X, EffNode{EffNodeKind::Seq, DomainKind::AllVertices,
                                          nullptr, PhaseSegment::None, Inner, Q});
    unsigned Body = newEffNode(X, EffNode{EffNodeKind::Seq, DomainKind::AllVertices,
                                          nullptr, PhaseSegment::None, P, Tail});
    X.Root = newEffNode(X, EffNode{EffNodeKind::SeqDomain, X.RoundDomain,
                                   nullptr, PhaseSegment::None, Body, 0});
    /* flatten(E) ≡ Info.Effects modulo Par-induced permutation: multiset
     * equality, not ordered equality. */
    SmallVector<std::pair<const Effect *, PhaseSegment>, 32> Flat;
    collectTreePrims(X, Flat);
    assert(Flat.size() == Info.Effects.size() &&
           "flatten(E) must cover every primitive exactly once");
    (void)Flat;
}

/* Structural temporal relation τ of one read against one mutating primitive,
 * with the oracle-exact five-case table:
 *   same base, same region                      → SameRoundRead
 *   R(A,U)×W(A,V), W in the pair phase, Frontier→ PreviousRoundRead
 *   R(A,U)×W(A,V), W in the pair phase, AllVerts→ Carried
 *   R(A,U)×W(A,V), W not in the pair phase      → SameRoundRead
 *   R(A,V)×W(A,U)                               → SameRoundRead
 *   all remaining cross-region pairs            → Independent
 *
 * The decisive condition is the WRITE's phase: a pair-phase (round-concurrent)
 * dest write is exactly what a frontier-gated U read observes from the previous
 * round.  The read's own segment never weakens the frontier rule: P(u);B(u,·)
 * ;Q(u) is a per-source sequence, not a per-round barrier, so other sources'
 * pair work is concurrent with every phase and sssp's driver read R(dist,U) in
 * the preamble is PreviousRoundRead.  (Errata: the segment-equality variant
 * `Seg(e_r) ≠ Seg(e_w) → SameRoundRead` is incorrect — it was a misreading of
 * the pre-v7 write-side SameSegment boolean.  See proof/EFFECT_ALGEBRA_DESIGN.md
 * §4.) */
static Temporal relateReadWrite(const Effect &Rd, const Effect &M,
                                PhaseSegment MSeg, DomainKind RoundDomain)
{
    if (!effectIsMutating(M) || M.Kind == EffectKind::Activate)
        return Temporal::Independent;
    if (M.Base != Rd.Base)
        return Temporal::Independent;
    if (M.Reg == Rd.Reg)
        return Temporal::SameRoundRead;
    if (Rd.Reg == Region::U && M.Reg == Region::V)
    {
        if (MSeg != PhaseSegment::Pair)
            return Temporal::SameRoundRead; /* per-source write: sequenced */
        return RoundDomain == DomainKind::Frontier ? Temporal::PreviousRoundRead
                                                   : Temporal::Carried;
    }
    if (Rd.Reg == Region::V && M.Reg == Region::U)
        return Temporal::SameRoundRead; /* staged owner control */
    return Temporal::Independent; /* other cross-region pairs are unrelated */
}

/* Derive τ for every read from the tree: each read is related against every
 * mutating primitive with the write's phase-segment identity and the
 * round-domain kind.  No σ or RoundSepBases information is consulted. */
static void deriveAllTemporal(NeighborLoopInfo &Info)
{
    SmallVector<std::pair<const Effect *, PhaseSegment>, 32> Prims;
    collectTreePrims(Info.Expr, Prims);
    for (Effect &Rd : Info.Effects)
    {
        if (Rd.Kind != EffectKind::R || !Rd.Base)
            continue;
        Temporal Worst = Temporal::Independent;
        for (const auto &P : Prims)
        {
            Temporal T = relateReadWrite(Rd, *P.first, P.second,
                                         Info.Expr.RoundDomain);
            if (temporalRank(T) > temporalRank(Worst))
                Worst = T;
        }
        Rd.Temp = Worst;
    }
}

/* Occurrence-preservation predicate over binder paths: does the expression
 * contain a per-source claim (Preamble/Epilogue segment)?  A κ=PerSource
 * effect may not become κ=PerPair under the rewrite. */
static bool hasPerSourceClaim(const NeighborLoopInfo &Info)
{
    SmallVector<std::pair<const Effect *, PhaseSegment>, 32> Prims;
    collectTreePrims(Info.Expr, Prims);
    for (const auto &P : Prims)
        if (P.first->Kind == EffectKind::Claim && P.second != PhaseSegment::Pair)
            return true;
    return false;
}

/* All per-source first-wins claim effects (store form) when every one has a
 * constant guard/transition and a module-reachable base (pointer slot or a
 * statically sized array global).  Empty when any claim needs unsupported
 * staging, so callers fail closed. */
static SmallVector<const Effect *, 2>
perSourceClaims(const NeighborLoopInfo &Info)
{
    SmallVector<const Effect *, 2> Found;
    for (const Effect &E : Info.Effects)
        if (E.Kind == EffectKind::Claim && E.Scope != OccurrenceScope::PerPair)
        {
            if (!E.Base || !isa<Constant>(E.ClaimGuard) ||
                !isa<Constant>(E.ClaimTransition) || isa<AllocaInst>(E.Base) ||
                !E.ClaimGuard->getType()->isIntegerTy())
            {
                Found.clear();
                return Found;
            }
            Found.push_back(&E);
        }
    return Found;
}

/* Does the claim's guard dominate the neighbour loop?  Only then does the
 * claim gate the pair work; a preamble claim that does not wrap the loop still
 * performs its transition once per source but must not gate the pairs. */
static bool claimGatesLoop(const NeighborLoopInfo &Info, const Effect *Claim)
{
    if (!Claim || !Claim->Origin)
        return false;
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    DominatorTree DT(*F);
    return DT.dominates(Claim->Origin->getParent(),
                        Info.NeighborLoop->getHeader());
}

/* Module-global name publishing the per-source claim state array S[u][j]. */
static std::string claimGlobalName(const NeighborLoopInfo &Info)
{
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    return F->getName().str() + ".claim";
}

/* Count mutating primitives in the Preamble segment (binder-path query); the
 * per-source consume store lives in the Epilogue, so this is the occurrence
 * check the source-reduction gate needs. */
static unsigned preambleMutationCount(const NeighborLoopInfo &Info)
{
    SmallVector<std::pair<const Effect *, PhaseSegment>, 32> Prims;
    collectTreePrims(Info.Expr, Prims);
    unsigned N = 0;
    for (const auto &P : Prims)
        if (P.second == PhaseSegment::Preamble &&
            P.first->Kind != EffectKind::R)
            ++N;
    return N;
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

/* ── composition R3: privatization ───────────────────────────────
 *
 * A loop whose every mutating effect is a recognized U_⊕ update is a pure
 * accumulation: the final value of each written location is the operator
 * applied over the updates, independent of the order in which the partitions
 * apply them (that is what makes the operator family an *algebraic* combine in
 * the first place).  Such a loop needs no owner: give every partition a private
 * copy of each written base, let it accumulate there, and fold the copies with
 * the operator's own combine.  That is the only mechanism that covers bases an
 * owner table cannot key:
 *
 *   - a data-valued subscript (`cnt[deg[u]] += 1`): the destination house is a
 *     data value, not a vertex id, so `CC_PART_OF` has nothing to key on;
 *   - one base written through both endpoint regions (`arr[u] += 1;
 *     arr[v] += 1`): no single region owns it, so no owner rewrite is sound;
 *   - several scalar accumulators: the legacy engine maps exactly one slot.
 *
 * The proof obligations below are what keep this derived rather than a shape
 * whitelist.  The loop is privatizable only when:
 *   1. every mutating effect is Update with a recognized operator (an
 *      unrecognized update has no combine to fold with);
 *   2. no claim (first-wins), no frontier append/activate, no driver-preamble
 *      store — those have per-source semantics a per-pair step cannot carry;
 *   3. one operator per base (two operators on one base would fold with the
 *      wrong one);
 *   4. every load from a privatized base is a link in the *old-value* chain of
 *      an update to that same base, and reaches at least one such update: a
 *      load used as an index, a call argument, or a branch condition would
 *      observe the real array, not this partition's copy, and the copy would
 *      answer a different question;
 *   5. the operator is order-independent for the element type — integer
 *      arithmetic is exact modulo the type width, min/max are idempotent and
 *      commutative, but float + and * re-associate, so they stay sequential
 *      (the fold would not reproduce the serial rounding). */

/* Element type of a scalar slot: the type its own loads use. */
static Type *slotElemType(const Value *Ptr)
{
    Type *Ty = nullptr;
    for (const User *U : Ptr->users())
        if (const auto *LI = dyn_cast<LoadInst>(U))
            Ty = LI->getType();
    return Ty;
}

/* Element type, element count and the array value itself for a base.
 *
 * The front end lowers a declared array to an alloca whose *address* is kept
 * in a ptr-typed global slot (`store ptr %cnt.data, ptr @cnt`) and re-loaded at
 * every use, so the canonical base can be that slot rather than the array.
 * Resolve it through the single pointer stored into it; two different stored
 * arrays mean the base is not one array and the caller must refuse. */
static bool arrayBaseInfo(const Value *Base, Type *&ElemTy, Value *&Count,
                          Value *&ArrPtr, unsigned Depth = 0)
{
    LLVMContext &Ctx = Base->getContext();
    if (Depth > 3)
        return false;
    if (const auto *AI = dyn_cast<AllocaInst>(Base))
    {
        ArrPtr = const_cast<AllocaInst *>(AI);
        Type *T = AI->getAllocatedType();
        if (auto *AT = dyn_cast<ArrayType>(T))
        {
            ElemTy = AT->getElementType();
            Count = ConstantInt::get(Type::getInt64Ty(Ctx), AT->getNumElements());
        }
        else
        {
            ElemTy = T;
            Count = const_cast<Value *>(AI->getArraySize());
        }
        return ElemTy->isIntegerTy() || ElemTy->isFloatingPointTy();
    }
    if (const auto *GV = dyn_cast<GlobalVariable>(Base))
    {
        if (auto *AT = dyn_cast<ArrayType>(GV->getValueType()))
        {
            ElemTy = AT->getElementType();
            Count = ConstantInt::get(Type::getInt64Ty(Ctx), AT->getNumElements());
            ArrPtr = const_cast<GlobalVariable *>(GV);
            return ElemTy->isIntegerTy() || ElemTy->isFloatingPointTy();
        }
        if (GV->getValueType()->isPointerTy())
        {
            const Value *Stored = nullptr;
            for (const User *U : GV->users())
                if (const auto *SI = dyn_cast<StoreInst>(U))
                {
                    if (SI->getPointerOperand()->stripPointerCasts() != GV)
                        continue;
                    const Value *V = SI->getValueOperand();
                    if (Stored && Stored != V)
                        return false; /* two different arrays in one slot */
                    Stored = V;
                }
            if (Stored && Stored != Base)
                return arrayBaseInfo(Stored, ElemTy, Count, ArrPtr, Depth + 1);
        }
    }
    return false;
}

/* FoldSound(ω,T): does folding partition partials with ω.f_fold reproduce the
 * serial left-to-right fold for element type T under exact machine semantics?
 * Integer arithmetic is exact modulo the width for every recognized operator,
 * so associativity (recorded in L_ω) is the whole obligation.  On floats only
 * the idempotent, commutative min/max flavours satisfy it — float +/* and the
 * bitwise ops re-associate, so they are refused.  Exactness is a property of
 * this predicate, not an intrinsic field of the operator. */
static bool foldSound(RedOp Op, Type *ElemTy)
{
    if (!ElemTy)
        return false;
    if (ElemTy->isIntegerTy())
        return true; /* exact modulo the width for every recognized op */
    if (!ElemTy->isFloatingPointTy())
        return false;
    switch (Op)
    {
    case RedOp::FMinNum:
    case RedOp::FMaxNum:
    case RedOp::FMinProp:
    case RedOp::FMaxProp:
    case RedOp::Min:
    case RedOp::Max:
        return true; /* idempotent and commutative: fold == serial */
    default:
        return false; /* float +/* : re-associates, so refuse */
    }
}

/* Every use of a load from `Base` must be a link in the update chain that ends
 * in a store to `Base` (the old-value operand of an update), and at least one
 * such store must be reached.  Anything else — an index, a call argument, a
 * branch condition — reads the real array, which a private copy would not
 * reproduce.
 *
 * Feeding an update is necessary but not sufficient: the load must be *that
 * update's own old value*, i.e. read the element the update writes.  A load at
 * a different element of the same base (`arr[u] = arr[u] + arr[u+1]`) also
 * feeds an update, but a private copy holds the operator identity there, not
 * the array's value, so the accumulation theorem does not cover it.  The one
 * exception is a cross-endpoint read that round separation resolves: the pair
 * body reads it from the frozen round-start snapshot, which is the real value
 * independently of the copies (`ShadowResolved`). */
static bool loadFeedsOnlyUpdates(LoadInst *LI, const Value *Base, Loop *Nest,
                                 bool ShadowResolved)
{
    if (LI->use_empty())
        return false;
    SmallPtrSet<const Value *, 8> Seen;
    SmallVector<Instruction *, 8> Work;
    Work.push_back(LI);
    Seen.insert(LI);
    bool ReachedStore = false;
    while (!Work.empty())
    {
        Instruction *I = Work.pop_back_val();
        for (const User *U : I->users())
        {
            const auto *UI = dyn_cast<Instruction>(U);
            if (!UI || !Nest->contains(UI))
                return false;
            if (const auto *SI = dyn_cast<StoreInst>(UI))
            {
                const auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
                if (!GEP || canonicalArrayBase(GEP) != Base)
                    return false;
                if (!ShadowResolved &&
                    !sameArraySlot(const_cast<Value *>(SI->getPointerOperand()),
                                   LI->getPointerOperand()))
                    return false;
                ReachedStore = true;
                continue;
            }
            if (isa<CallInst>(UI) || isa<LoadInst>(UI) || isa<GetElementPtrInst>(UI) ||
                UI->isTerminator())
                return false;
            if (Seen.insert(UI).second)
                Work.push_back(const_cast<Instruction *>(UI));
        }
    }
    return ReachedStore;
}

/* All stores to `Base` inside the nest (the neighbour loop *and* the driver
 * body around it -- a per-source preamble write such as `w[u] += 1` is one of
 * them) are recognized U_⊕ updates with `Op`, and every load is part of such an
 * update.  `Info` supplies the round-separation witnesses: a load listed as a
 * cross-endpoint read of this base is served from the shadow snapshot by the
 * emitted body, so it is exempt from the own-old-value requirement. */
static bool baseIsPrivatizable(const Value *Base, RedOp Op, Loop *Nest,
                               const NeighborLoopInfo &Info)
{
    auto shadowResolved = [&](const LoadInst *LI) -> bool
    {
        for (const auto &RS : Info.RoundSepBases)
            if (RS.Base == Base && RS.CrossReads.count(LI))
                return true;
        return false;
    };

    for (BasicBlock *BB : Nest->blocks())
        for (Instruction &I : *BB)
        {
            if (auto *SI = dyn_cast<StoreInst>(&I))
            {
                const auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
                if (!GEP || canonicalArrayBase(GEP) != Base)
                    continue;
                RedOp AOp = detectScalarRedOp(SI->getValueOperand(), SI->getPointerOperand());
                if (AOp == RedOp::None)
                    AOp = detectConditionalMinMax(SI, SI->getPointerOperand());
                if (AOp != Op)
                    return false;
            }
            else if (auto *LI = dyn_cast<LoadInst>(&I))
            {
                const auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
                if (!GEP || canonicalArrayBase(GEP) != Base)
                    continue;
                if (!loadFeedsOnlyUpdates(LI, Base, Nest, shadowResolved(LI)))
                    return false;
            }
        }
    return true;
}

/* Fill Info.Slots / Info.PrivArrays when the whole effect set is privatizable.
 * Returns true when the proof holds (the caller decides whether the layout is
 * *needed*: a single scalar slot with no array is the legacy Reduction path).
 *
 * Algebraic premises (Composition R3) checked here:
 *   (0) no claim / activation / non-preserved occurrence in the expression;
 *   (1) every mutating primitive is an algebraic update U(A,r,ω) with ω known;
 *   (2) every written base has a single compatible operator ω;
 *   (3) every load from a written base is part of that base's old-value chain;
 *   (4) the fold law holds for ω on the element type (foldSound). */
static const char *PrivReason = "";

/* Premise 0 at the expression level: a claim anywhere in the segmented effect
 * expression makes the pure-accumulation theorem inapplicable (claims have
 * per-source/ordered semantics on the live array).  Activation is orthogonal:
 * it touches the dest envelope, never the privatized bases. */

static bool privLayout(NeighborLoopInfo &Info)
{
    Loop *L = Info.NeighborLoop;
    PrivReason = "";
    if (!L)
    {
        PrivReason = "no loop";
        return false;
    }
    /* The proof covers the whole nest: a driver-preamble update runs once per
     * source and is carried by the step's preamble phase (see the R3+D step).
     * Activation is orthogonal to the accumulation theorem: it touches the
     * dest envelope (dest_seen/next_frontier), never the privatized bases, so
     * it composes with privatization (the A+ primitive reads the context). */
    Loop *Nest = Info.DriverLoop ? Info.DriverLoop : L;
    if (Info.HasFirstWins || hasPerSourceClaim(Info))
    {
        PrivReason = "claim in the loop nest";
        return false;
    }

    Info.Slots.clear();
    Info.PrivArrays.clear();

    /* 1/2. Effect set: updates only (reads and activation are orthogonal). */
    for (const Effect &E : Info.Effects)
    {
        if (E.Kind == EffectKind::R || E.Kind == EffectKind::Activate)
            continue;
        if (E.Kind != EffectKind::Uop || E.Op == RedOp::None || !E.Base)
        {
            PrivReason = "mutating effect is not a recognized update";
            return false;
        }
        if (E.Reg != Region::G && E.Reg != Region::U && E.Reg != Region::V &&
            E.Reg != Region::D)
        {
            PrivReason = "unknown index provenance";
            return false;
        }
    }

    /* 3. One operator per location; scalars become slots. */
    for (const Effect &E : Info.Effects)
    {
        if (E.Kind != EffectKind::Uop || E.Reg != Region::G || !E.Base)
            continue;
        bool Found = false;
        for (auto &S : Info.Slots)
            if (S.Ptr == E.Base)
            {
                if (S.Op != E.Op)
                    return false;
                Found = true;
            }
        if (!Found)
        {
            Type *Ty = slotElemType(E.Base);
            if (!foldSound(E.Op, Ty))
            {
                PrivReason = "operator re-associates for the element type";
                return false;
            }
            NeighborLoopInfo::SlotInfo S;
            S.Ptr = E.Base;
            S.Op = E.Op;
            S.ElemTy = Ty;
            Info.Slots.push_back(S);
        }
    }

    /* 4/5. Arrays: one operator per base, update-only uses, order-independent
     * operator for the element type. */
    SmallVector<const Value *, 4> Bases;
    for (const Effect &E : Info.Effects)
        if (E.Kind == EffectKind::Uop && E.Reg != Region::G && E.Base)
        {
            bool Have = false;
            for (const Value *B : Bases)
                if (B == E.Base)
                    Have = true;
            if (!Have)
                Bases.push_back(E.Base);
        }
    for (const Value *B : Bases)
    {
        RedOp Op = RedOp::None;
        for (const Effect &E : Info.Effects)
            if (E.Kind == EffectKind::Uop && E.Base == B && E.Reg != Region::G)
            {
                if (Op == RedOp::None)
                    Op = E.Op;
                else if (Op != E.Op)
                    return false;
            }
        Type *ElemTy = nullptr;
        Value *Count = nullptr;
        Value *ArrPtr = nullptr;
        if (!arrayBaseInfo(B, ElemTy, Count, ArrPtr))
        {
            PrivReason = "array base is not a sized alloca/global";
            return false;
        }
        if (!foldSound(Op, ElemTy))
        {
            PrivReason = "operator re-associates for the element type";
            return false;
        }
        if (!baseIsPrivatizable(B, Op, Nest, Info))
        {
            PrivReason = "a load from the base is not an update's own old value";
            return false;
        }
        NeighborLoopInfo::ArrayInfo A;
        A.Base = B;
        A.Ptr = ArrPtr;
        A.Op = Op;
        A.ElemTy = ElemTy;
        A.Count = Count;
        Info.PrivArrays.push_back(A);
    }
    return true;
}

/* True when ownership already localizes every mutation of every privatizable
 * array to one endpoint, so the private per-partition copies are unnecessary. */
static bool ownedLocalizedArray(const NeighborLoopInfo &Info);

/* The layout is *needed* only when no cheaper legal realization exists: more
 * than one scalar accumulator (the record is the sole carrier of multiple
 * scalar partials), or an array whose mutations ownership does not already
 * localize (dual endpoints, cross-endpoint pair reads, round-separation
 * bases, derived/data-derived indices, ...).  `privatizable` means the
 * transformation is legal; `needed` must mean "required". */
static bool privLayoutNeeded(const NeighborLoopInfo &Info)
{
    if (Info.Slots.size() > 1)
        return true;
    if (Info.PrivArrays.empty())
        return false;
    return !ownedLocalizedArray(Info);
}

/* ── algebraic interpretation ⟦E⟧_par ────────────────────────────
 * EffectSummary is the fold of the segmented expression into the predicates
 * the parallelization laws are stated over.  summarizeEffects() computes it
 * from the expression segments (Preamble ; Pair ; Epilogue), so the
 * interpretation consumes the algebraic object rather than the raw list.
 * interpretPar() is the interpretation function: a monotone map from the
 * expression's algebraic properties to the execution class, with Sequential
 * as the terminal refusal.  It is the sole classification path since the
 * Phase-3 cutover; the legacy ladder was removed after the differential
 * harness (algebra-only vs legacy-only over every fixture) reported zero
 * disagreements, and validate_algebra.sh now guards the verdicts. */
struct EffectSummary
{
    bool MutU = false, MutV = false, MutG = false, MutD = false, MutTop = false;
    bool HasUopG = false, HasUnrecognizedG = false, HasCarriedOnMut = false;
    SmallPtrSet<const Value *, 4> BaseU, BaseV;
};

/* Visit every primitive of the expression in tree order (binder-path walk). */
template <typename Fn>
static void forEachExprEffect(const NeighborLoopInfo &Info, Fn &&F)
{
    SmallVector<std::pair<const Effect *, PhaseSegment>, 32> Prims;
    collectTreePrims(Info.Expr, Prims);
    for (const auto &P : Prims)
        F(*P.first, P.second);
}

/* True when `Index` is exactly the endpoint value the owner traversal hands to
 * the pair callback -- the provenance seeds themselves, modulo same-kind casts
 * and the u-slot / v-slot load rules the analysis already admits -- not merely
 * a value whose provenance joins to that region (deg[u+1], deg[perm[u]], ...). */
static bool isOwnedEndpointIndex(Value *Index, Region Owned,
                                 const NeighborLoopInfo &Info)
{
    if (!Index)
        return false;
    auto Peel = [](Value *V)
    {
        while (auto *C = dyn_cast<CastInst>(V))
            V = C->getOperand(0);
        return V;
    };
    Value *I = Peel(Index);
    if (Owned == Region::V)
    {
        /* Provenance::transfer's v-slot rule, exact. */
        if (auto *LI = dyn_cast<LoadInst>(I))
            return LI->getPointerOperand() == Info.VAlloca;
        return false;
    }
    /* U: the same seeds the analysis uses (driver induction phi, the init u
     * operand and its cast-peeled form). */
    Value *USrc = Info.UVal;
    while (USrc && isa<CastInst>(USrc))
        USrc = cast<CastInst>(USrc)->getOperand(0);
    Value *Seeds[3] = {driverIndVar(Info.DriverLoop), USrc, Info.UVal};
    for (Value *S : Seeds)
        if (S && sameAddressValue(I, Peel(S), 0))
            return true;
    if (Info.UAlloca)
        if (auto *LI = dyn_cast<LoadInst>(I))
            return LI->getPointerOperand() == Info.UAlloca;
    return false;
}

/* True when a direct owner-computes write is already race-free for every
 * privatizable array in the loop, so the per-partition private copies and the
 * serial element-wise combine are unnecessary (privatization is legal but not
 * required).  Requires: no scalar accumulator; no claim / shadow base; every
 * array update accepted by privLayout(); every mutation indexed by exactly one
 * owned endpoint; every pair-phase read of those bases at that same endpoint.
 * Under OWNER_U each source and its row belong to exactly one partition and
 * are traversed in serial order; OWNER_V is the destination mirror.  NOTE:
 * requiring all arrays to share one endpoint is a conservative restriction of
 * this patch (it keeps the single-traversal realization); it is not a
 * fundamental correctness requirement. */
static bool ownedLocalizedArray(const NeighborLoopInfo &Info)
{
    if (Info.PrivArrays.empty())
        return false;
    if (Info.ReducePtr || !Info.Slots.empty())
        return false;
    if (Info.HasFirstWins || hasPerSourceClaim(Info) ||
        !Info.RoundSepBases.empty())
        return false;

    /* Every array update must be one the privatization proof accepted: a base
     * rejected by privLayout() (fold-unsound type, load not in the old-value
     * chain, unsized alloca, ...) keeps the existing behaviour. */
    for (const Effect &E : Info.Effects)
    {
        if (E.Kind != EffectKind::Uop || E.Reg == Region::G || !E.Base)
            continue;
        bool Known = false;
        for (const auto &A : Info.PrivArrays)
            if (A.Base == E.Base)
                Known = true;
        if (!Known)
            return false;
    }

    Region Owned = Region::Bottom;
    for (const auto &A : Info.PrivArrays)
        for (const Effect &E : Info.Effects)
        {
            if (E.Base != A.Base)
                continue;
            if (E.Kind == EffectKind::R || E.Kind == EffectKind::Activate)
                continue;
            if (E.Kind != EffectKind::Uop ||
                (E.Reg != Region::U && E.Reg != Region::V))
                return false;
            if (!isOwnedEndpointIndex(E.Index, E.Reg, Info))
                return false;
            if (Owned == Region::Bottom)
                Owned = E.Reg;
            else if (Owned != E.Reg)
                return false;
        }
    if (Owned == Region::Bottom)
        return false;

    bool CrossRead = false;
    forEachExprEffect(Info, [&](const Effect &E, PhaseSegment Seg)
    {
        if (CrossRead || Seg != PhaseSegment::Pair ||
            E.Kind != EffectKind::R || !E.Base)
            return;
        bool OnPriv = false;
        for (const auto &A : Info.PrivArrays)
            if (A.Base == E.Base)
                OnPriv = true;
        if (OnPriv && !isOwnedEndpointIndex(E.Index, Owned, Info))
            CrossRead = true;
    });
    return !CrossRead;
}

/* Φ: the recursive summary homomorphism E → S.  Primitive facts are joined
 * with ⊙_seq/⊙_par/⊙_dom (idempotent boolean joins and set unions, so the fold
 * order is immaterial); the phase-segment tag is used only to keep the tree
 * walk honest.  HasCarriedOnMut is a projection of the order-sensitive temporal
 * layer (τ is already derived), not reconstructed from Φ alone: a carried read
 * that is not resolved by a shadow snapshot (σ=Snapshot) and shares its base
 * with a cross-region mutation forces Sequential. */
static void summarizeEffects(const NeighborLoopInfo &Info, EffectSummary &S)
{
    auto Scan = [&](const Effect &E, PhaseSegment Seg)
    {
        (void)Seg;
        if (E.Kind == EffectKind::R)
        {
            if (E.Temp != Temporal::Carried || !E.Base)
                return;
            /* A carried read resolved by the round-separation shadow snapshot
             * is not a sequential trigger (σ=Snapshot). */
            if (E.VSource == ValueSource::Snapshot)
                return;
            forEachExprEffect(Info, [&](const Effect &M, PhaseSegment MSeg)
            {
                (void)MSeg;
                if (!effectIsMutating(M) || M.Kind == EffectKind::Activate)
                    return;
                if (M.Base == E.Base && M.Reg != E.Reg)
                    S.HasCarriedOnMut = true;
            });
            return;
        }
        if (E.Kind == EffectKind::Activate)
        {
            S.MutV = true;
            return;
        }
        switch (E.Reg)
        {
        case Region::U:
            S.MutU = true;
            if (E.Base)
                S.BaseU.insert(E.Base);
            break;
        case Region::V:
            S.MutV = true;
            if (E.Base)
                S.BaseV.insert(E.Base);
            break;
        case Region::G:
            S.MutG = true;
            if (E.Kind == EffectKind::Uop)
                S.HasUopG = true;
            else
                S.HasUnrecognizedG = true;
            break;
        case Region::D:
            S.MutD = true;
            break;
        case Region::Bottom:
            break;
        default:
            S.MutTop = true;
            break;
        }
    };
    if (Info.Expr.Arena.empty())
    {
        /* Defensive: the expression is built before classification; fall back
         * to the raw set if a future path forgets, so the summary is never
         * silently empty. */
        for (const Effect &E : Info.Effects)
            Scan(E, PhaseSegment::None);
        return;
    }
    forEachExprEffect(Info, Scan);
}

static void printEffects(const NeighborLoopInfo &Info)
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
    errs() << "  temporal=" << temporalName(Worst);
}

/* Canonical atom key for the executable-atom census: identity (kind, op,
 * base, region, occurrence scope, phase segment) with occurrences preserved by
 * the multiset comparison. */
static std::string atomKey(const Effect &E, PhaseSegment Seg)
{
    std::string K = effectKindName(E.Kind);
    if (E.Kind == EffectKind::Uop && E.Op != RedOp::None)
        K += std::string("[") + redOpName(E.Op) + "]";
    K += "(";
    K += (E.Base && E.Base->hasName()) ? E.Base->getName().str() : "_";
    K += ",";
    K += regionName(E.Reg);
    K += "):seg=" + std::to_string((int)Seg) +
         ":scope=" + std::to_string((int)E.Scope);
    return K;
}

/* Executable-atom census (SGPL_EXEC_DUMP=1): the flattened expression's atoms
 * and the atom set each executable operation implements.  Fusion keeps the
 * multiset invariant atoms(ExecOps) == flatten(E) modulo Par permutation:
 * the claim ops realize the per-source claims, the preamble op the remaining
 * preamble mutations, the pair op the pair segment, the finish op the
 * epilogue, and snapshot/combine are mechanisms that realize no atom. */
static void dumpExecAtoms(const NeighborLoopInfo &Info)
{
    for (const Effect &E : Info.Effects)
    {
        PhaseSegment Seg = PhaseSegment::None;
        for (const auto &P : Info.Expr.Preamble)
            if (P == &E)
                Seg = PhaseSegment::Preamble;
        for (const auto &P : Info.Expr.Pair)
            if (P == &E)
                Seg = PhaseSegment::Pair;
        for (const auto &P : Info.Expr.Epilogue)
            if (P == &E)
                Seg = PhaseSegment::Epilogue;
        errs() << "[atom] " << atomKey(E, Seg) << "\n";
    }
    auto Emit = [&](const char *Op, const SmallVectorImpl<const Effect *> &Seg,
                    bool Claims)
    {
        errs() << "[exec] op=" << Op << " atoms=";
        bool First = true;
        for (const Effect *E : Seg)
        {
            bool IsClaim = E->Kind == EffectKind::Claim;
            if (IsClaim != Claims)
                continue;
            if (!First)
                errs() << "|";
            First = false;
            errs() << atomKey(*E, PhaseSegment::Preamble);
        }
        errs() << "\n";
    };
    Emit("claim", Info.Expr.Preamble, true);
    Emit("preamble", Info.Expr.Preamble, false);
    errs() << "[exec] op=pair atoms=";
    {
        bool First = true;
        for (const Effect *E : Info.Expr.Pair)
        {
            if (!First)
                errs() << "|";
            First = false;
            errs() << atomKey(*E, PhaseSegment::Pair);
        }
    }
    errs() << "\n[exec] op=finish atoms=";
    {
        bool First = true;
        for (const Effect *E : Info.Expr.Epilogue)
        {
            if (!First)
                errs() << "|";
            First = false;
            errs() << atomKey(*E, PhaseSegment::Epilogue);
        }
    }
    errs() << "\n[exec] op=snapshot atoms=\n[exec] op=combine atoms=\n";
}

/* Segmented form of the same set: `preamble ; pair ; epilogue` with each
 * segment's primitives joined by `⊕` and per-primitive occurrence scope. */
static void printEffectExpr(const NeighborLoopInfo &Info)
{
    const EffectExpr &X = Info.Expr;
    auto PrintSeg = [&](const char *Name, const auto &Seg)
    {
        errs() << "  " << Name << "=";
        if (Seg.empty())
        {
            errs() << "ε";
            return;
        }
        bool First = true;
        for (const Effect *E : Seg)
        {
            if (!First)
                errs() << "⊕";
            First = false;
            errs() << effectKindName(E->Kind);
            if (E->Kind == EffectKind::Uop && E->Op != RedOp::None)
                errs() << redOpName(E->Op);
            errs() << "(";
            if (E->Base && E->Base->hasName())
                errs() << E->Base->getName();
            else
                errs() << "_";
            errs() << "," << regionName(E->Reg) << ")";
        }
    };
    errs() << "  expr:";
    PrintSeg("pre", X.Preamble);
    PrintSeg("pair", X.Pair);
    PrintSeg("epi", X.Epilogue);
    errs() << "  gated=" << (X.MembershipGated ? 1 : 0);
}

/* ── witness dump (Stage G) ──────────────────────────────────────
 * Normalized, stable dump of every emission-relevant NeighborLoopInfo field
 * plus the per-effect semantic facts (kind, base, region, occurrence scope,
 * temporal).  This is the compatibility oracle for the algebra refactor:
 * SGPL_WITNESS_DUMP=1 prints it; proof/refactor_golden_* captures it.
 * Format is deliberately line-per-fact and sorted so it is diff-stable. */
static void printWitness(const NeighborLoopInfo &Info, bool IsIter)
{
    errs() << "[witness] iter=" << (IsIter ? 1 : 0)
           << " driver="
           << (Info.DriverLoop ? Info.DriverLoop->getHeader()->getName() : "<none>")
           << " inner=" << Info.NeighborLoop->getHeader()->getName()
           << "\n";
    errs() << "[witness] writekind=" << (int)Info.WriteKind
           << " hasdata=" << (Info.HasDataWrite ? 1 : 0)
           << " redptr=" << (Info.ReducePtr && Info.ReducePtr->hasName()
                                 ? Info.ReducePtr->getName()
                                 : "_")
           << " redop=" << redOpName(Info.ReduceOp)
           << " firstwins=" << (Info.HasFirstWins ? 1 : 0)
           << " driverclaim=" << (Info.HasDriverClaim ? 1 : 0)
           << " append=" << (Info.HasFrontierAppend ? 1 : 0)
           << " gated=" << (Info.MembershipGated ? 1 : 0)
           << " needsroundsep=" << (Info.NeedsRoundSep ? 1 : 0)
           << " priv=" << (Info.UsePrivLayout ? 1 : 0) << "\n";
    errs() << "[witness] slots=" << Info.Slots.size()
           << " privarrays=" << Info.PrivArrays.size()
           << " roundsep=" << Info.RoundSepBases.size()
           << " driverustores=" << Info.DriverUStores.size()
           << " driveruclaims=" << Info.DriverUClaims.size()
           << " accconsume=" << (Info.AccConsumeStore ? 1 : 0)
           << " accreset=" << (Info.AccResetSeen ? 1 : 0) << "\n";
    for (const auto &RS : Info.RoundSepBases)
    {
        errs() << "[witness] rs base="
               << (RS.Base && RS.Base->hasName() ? RS.Base->getName() : "_")
               << " writesv=" << (RS.WritesV ? 1 : 0)
               << " crossreads=" << RS.CrossReads.size() << "\n";
    }
    /* Effect facts, in construction order. */
    for (const Effect &E : Info.Effects)
    {
        errs() << "[witness] eff kind=" << effectKindName(E.Kind)
               << " op=" << redOpName(E.Op)
               << " base=" << (E.Base && E.Base->hasName() ? E.Base->getName() : "_")
               << " reg=" << regionName(E.Reg)
               << " scope=" << (int)E.Scope
               << " temp=" << temporalName(E.Temp);
        if (E.Kind == EffectKind::R)
            errs() << " vsrc=" << (E.VSource == ValueSource::Snapshot ? "snapshot"
                                                                       : "live");
        errs() << "\n";
    }
}

/* ── per-pair wrapper via manual clone ────────────────────────── */

/* Combine one partition partial into the running total: `*out = *out (op) *partial`
 * where `op` is the operator record's *fold* operation.  Float vs int chosen from
 * the element type; ascending partition-order calls in the runtime keep the fold
 * deterministic.  The switch is over the fold op, so subtraction (whose partials
 * hold the negated partition sums) folds through the Add arm. */
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
    switch (redOpInfo(Op).FoldOp)
    {
    case RedOp::Add:
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
    case RedOp::MinU:
        R = B.CreateBinaryIntrinsic(Intrinsic::umin, O, P);
        break;
    case RedOp::MaxU:
        R = B.CreateBinaryIntrinsic(Intrinsic::umax, O, P);
        break;
    case RedOp::FMinNum:
        R = B.CreateBinaryIntrinsic(Intrinsic::minnum, O, P);
        break;
    case RedOp::FMaxNum:
        R = B.CreateBinaryIntrinsic(Intrinsic::maxnum, O, P);
        break;
    case RedOp::FMinProp:
        R = B.CreateBinaryIntrinsic(Intrinsic::minimum, O, P);
        break;
    case RedOp::FMaxProp:
        R = B.CreateBinaryIntrinsic(Intrinsic::maximum, O, P);
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

/* Identity element for a reduction operator's *fold* operation, used to
 * initialize per-partition partials so the ordered combine yields the serial
 * result.  Subtraction folds through Add, so its identity is 0. */
static Constant *identityFor(RedOp Op, Type *ElemTy)
{
    bool isFP = ElemTy->isFloatingPointTy();
    switch (redOpInfo(Op).FoldOp)
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
    case RedOp::MinU:
        return ConstantInt::getAllOnesValue(ElemTy);
    case RedOp::MaxU:
        return ConstantInt::get(ElemTy, 0);
    case RedOp::FMinNum:
    case RedOp::FMinProp:
        return ConstantFP::getInfinity(ElemTy, /*Negative=*/false);
    case RedOp::FMaxNum:
    case RedOp::FMaxProp:
        return ConstantFP::getInfinity(ElemTy, /*Negative=*/true);
    default: /* Add, Or, Xor (Sub folds through Add) */
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
    /* A statically sized array global (`int dist[8]` -> @dist = [8 x i32]) is
     * the storage itself, not a pointer slot: take its address. */
    if (auto *GV = dyn_cast<GlobalVariable>(Slot))
        if (GV->getValueType()->isArrayTy())
            return B.CreateBitCast(GV, I8P);
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

/* Deterministic, per-loop unique name of the pass-created global slot that
 * publishes a round-separation shadow pointer.  Both the pair work fn (reads)
 * and the round preheader (writes) compute the same name. */
static std::string shadowGlobalName(const NeighborLoopInfo &Info, unsigned Index)
{
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    const auto &RS = Info.RoundSepBases[Index];
    std::string BaseName =
        RS.Base && RS.Base->hasName() ? RS.Base->getName().str() : "arr";
    return F->getName().str() + "." + BaseName + ".shadow." + std::to_string(Index);
}

/* Composition A: emit the round-separation shadow snapshot for every in-place
 * base.  Runs in the round preheader — once per round for the gated frontier
 * while, once for the ungated driver.  For each base: grab a per-graph scratch
 * buffer, publish its pointer through the global the pair fn reads, then
 * memcpy the live array into it so every read in this round sees the frozen
 * round-start snapshot.  R7: the copy is produced by a Snapshot operation
 * (SGPL_OP_SNAPSHOT) whose callback calls the runtime primitive
 * autograph_snapshot_publish; the preheader only publishes the live base
 * pointer and the operation runs under the round owner. */
static void emitRoundSepShadow(
    IRBuilder<> &EB, const NeighborLoopInfo &Info, Module *Mod, Type *I8P,
    Type *I64, LLVMContext &Ctx, Value *GraphArg,
    SmallVectorImpl<std::pair<Function *, GlobalVariable *>> &SnapOps)
{
    if (Info.RoundSepBases.empty() || !GraphArg)
        return;
    (void)GraphArg; /* the operation's callback reads ctx->graph */
    Type *I32 = Type::getInt32Ty(Ctx);
    FunctionCallee GraphFn = Mod->getOrInsertFunction(
        "autograph_exec_ctx_graph", FunctionType::get(I8P, {I8P}, false));
    FunctionCallee Publish = Mod->getOrInsertFunction(
        "autograph_snapshot_publish",
        FunctionType::get(I8P, {I8P, I8P, I64, I32}, false));
    for (unsigned si = 0; si < Info.RoundSepBases.size(); ++si)
    {
        const auto &RS = Info.RoundSepBases[si];
        if (!RS.Base)
            continue;
        Type *ElemTy = RS.ElemTy;
        if (!ElemTy || (!ElemTy->isIntegerTy(32) && !ElemTy->isDoubleTy()))
            continue;
        unsigned ElemBytes = (unsigned)(ElemTy->getPrimitiveSizeInBits() / 8);
        GlobalVariable *SlotG = cast<GlobalVariable>(
            Mod->getOrInsertGlobal(shadowGlobalName(Info, si), I8P));
        SlotG->setLinkage(GlobalValue::InternalLinkage);
        SlotG->setInitializer(ConstantPointerNull::get(cast<PointerType>(I8P)));
        GlobalVariable *BaseG = cast<GlobalVariable>(Mod->getOrInsertGlobal(
            shadowGlobalName(Info, si) + ".base", I8P));
        BaseG->setLinkage(GlobalValue::InternalLinkage);
        BaseG->setInitializer(ConstantPointerNull::get(cast<PointerType>(I8P)));
        /* Publish the live base for the operation's callback (works for both
         * pointer slots and statically sized array globals/allocas). */
        Value *DistDp = ptrFromSlot(EB, const_cast<Value *>(RS.Base), I8P);
        EB.CreateStore(DistDp, BaseG);

        /* Snapshot(A) callback: void(i8* op, i8* ctx).  The runtime owns the
         * buffer lifecycle; the callback publishes it through the shadow
         * global the pair work functions read. */
        FunctionType *SnapTy = FunctionType::get(Type::getVoidTy(Ctx),
                                                 {I8P, I8P}, false);
        Function *CB = Function::Create(SnapTy, GlobalValue::InternalLinkage,
                                        "sgpl_snapshot", Mod);
        IRBuilder<> SB(BasicBlock::Create(Ctx, "entry", CB));
        Value *Graph = SB.CreateCall(GraphFn, {CB->getArg(1)});
        Value *Base = SB.CreateLoad(I8P, BaseG);
        Value *Pub = SB.CreateCall(Publish,
                                   {Graph, Base, ConstantInt::get(I64, ElemBytes),
                                    ConstantInt::get(I32, (int32_t)si)});
        SB.CreateStore(Pub, SlotG);
        SB.CreateRetVoid();
        SnapOps.push_back({CB, SlotG});
    }
}

/* Per-source claim staging (R7): allocate the per-source state array
 * S[u][j] (K claims), zero it, publish its base through the module global the
 * pair callbacks read, and emit one SOURCE_BEGIN callback per claim.  Each
 * callback performs its guarded transition on the live base and records the
 * outcome in S[u][j].  Exactly-once per source is discharged by the executor's
 * source lifecycle (OWNER_V coverage pre-pass / OWNER_U inline source change),
 * so the callbacks need no synchronization. */
static SmallVector<Function *, 2>
buildClaimState(IRBuilder<> &EB, const NeighborLoopInfo &Info, Module *Mod,
                LLVMContext &Ctx, Type *I8P, Type *I32, Type *I64,
                Value *GraphArg)
{
    SmallVector<Function *, 2> CBs;
    SmallVector<const Effect *, 2> Claims = perSourceClaims(Info);
    if (Claims.empty())
        return CBs;
    Value *N = EB.CreateLoad(I64, GraphArg, "graph_n");
    const unsigned K = (unsigned)Claims.size();
    Value *NElems = EB.CreateMul(N, ConstantInt::get(I64, (int64_t)K));
    AllocaInst *ClaimRes = EB.CreateAlloca(I32, NElems, "claim_result");
    EB.CreateMemSet(ClaimRes, ConstantInt::get(Type::getInt8Ty(Ctx), 0),
                    EB.CreateMul(NElems, ConstantInt::get(I64, 4)),
                    MaybeAlign(4));
    GlobalVariable *ClaimG = cast<GlobalVariable>(
        Mod->getOrInsertGlobal(claimGlobalName(Info), I8P));
    ClaimG->setLinkage(GlobalValue::InternalLinkage);
    ClaimG->setInitializer(ConstantPointerNull::get(cast<PointerType>(I8P)));
    EB.CreateStore(EB.CreateBitCast(ClaimRes, I8P), ClaimG);

    FunctionType *CTy = FunctionType::get(Type::getVoidTy(Ctx),
                                          {I8P, I8P, I32}, false);
    for (unsigned j = 0; j < K; ++j)
    {
        const Effect *Claim = Claims[j];
        Type *ClaimET = Claim->ClaimGuard->getType();
        Function *CB = Function::Create(CTy, GlobalValue::InternalLinkage,
                                        "sgpl_claim", Mod);
        BasicBlock *Ent = BasicBlock::Create(Ctx, "entry", CB);
        IRBuilder<> CBI(Ent);
        Value *U = CB->getArg(2);
        Value *Arr = CBI.CreateLoad(I8P, ClaimG, "claim_arr");
        Value *ArrI32 = CBI.CreateBitCast(Arr, PointerType::get(I32, 0));
        Value *BasePtr = ptrFromSlot(CBI, const_cast<Value *>(Claim->Base), I8P);
        Value *BaseT = CBI.CreateBitCast(BasePtr, PointerType::get(ClaimET, 0));
        Value *EP = CBI.CreateGEP(ClaimET, BaseT, U);
        Value *Old = CBI.CreateLoad(ClaimET, EP);
        Value *Won = CBI.CreateICmpEQ(Old,
                                      const_cast<Value *>(Claim->ClaimGuard));
        BasicBlock *Then = BasicBlock::Create(Ctx, "claim_then", CB);
        BasicBlock *Cont = BasicBlock::Create(Ctx, "claim_cont", CB);
        CBI.CreateCondBr(Won, Then, Cont);
        IRBuilder<> TBI(Then);
        TBI.CreateStore(const_cast<Value *>(Claim->ClaimTransition), EP);
        TBI.CreateBr(Cont);
        IRBuilder<> KBI(Cont);
        Value *Idx = KBI.CreateAdd(
            KBI.CreateMul(U, ConstantInt::get(I32, (int32_t)K)),
            ConstantInt::get(I32, (int32_t)j), "claim_slot");
        KBI.CreateStore(KBI.CreateZExt(Won, I32),
                        KBI.CreateGEP(I32, ArrI32, Idx));
        KBI.CreateRetVoid();
        CBs.push_back(CB);
    }
    return CBs;
}

/* Where the cloned pair body's environment comes from.  The runtime pair
 * callback ABI is (state, ctx, u, v); deriving the environment inside the work
 * function removes an emitted adapter and its per-pair accessor call. */
enum class PairEnvKind
{
    State,     /* op state (simple loops: null) */
    Partition, /* this partition's partial / privatized record */
    Ctx        /* the runtime context (activation: the A* primitive) */
};

/* Emit-time postcondition for composition R3: the pure-accumulation theorem is
 * only *realized* if the emitted body never touches the shared array, so no
 * element of a privatized base may be read or written in the pair work
 * function -- every such access must chain from the private copy published in
 * the partition's record.  Returns the offending base, or null when the
 * postcondition holds.
 *
 * A pointer-typed access at the base itself is not an element access: the front
 * end keeps a declared array's address in a ptr-typed global slot and re-loads
 * it at every use, and the redirect leaves those loads behind as dead address
 * computations.  Both the canonical base and the resolved array pointer are
 * checked, so a form the redirect does not recognize fails closed instead of
 * silently writing through the shared array. */
static const Value *privBaseStillShared(const Function *WF,
                                        const NeighborLoopInfo &Info)
{
    SmallPtrSet<const Value *, 8> Bases;
    for (const auto &A : Info.PrivArrays)
    {
        if (A.Base)
            Bases.insert(A.Base);
        if (A.Ptr)
            Bases.insert(A.Ptr);
    }
    if (Bases.empty())
        return nullptr;

    for (const BasicBlock &BB : *WF)
        for (const Instruction &I : BB)
        {
            const Value *Addr = nullptr;
            Type *AccessTy = nullptr;
            if (const auto *LI = dyn_cast<LoadInst>(&I))
            {
                Addr = LI->getPointerOperand();
                AccessTy = LI->getType();
            }
            else if (const auto *SI = dyn_cast<StoreInst>(&I))
            {
                Addr = SI->getPointerOperand();
                AccessTy = SI->getValueOperand()->getType();
            }
            else if (const auto *RMW = dyn_cast<AtomicRMWInst>(&I))
            {
                Addr = RMW->getPointerOperand();
                AccessTy = RMW->getValOperand()->getType();
            }
            else if (const auto *CAS = dyn_cast<AtomicCmpXchgInst>(&I))
            {
                Addr = CAS->getPointerOperand();
                AccessTy = CAS->getNewValOperand()->getType();
            }
            if (!Addr)
                continue;
            Addr = Addr->stripPointerCasts();
            if (const auto *GEP = dyn_cast<GetElementPtrInst>(Addr))
            {
                const Value *B = canonicalArrayBase(GEP);
                if (Bases.count(B))
                    return B;
                continue;
            }
            if (AccessTy && AccessTy->isPointerTy())
                continue; /* the array slot, not one of its elements */
            const Value *B = getUnderlyingObject(Addr);
            if (Bases.count(B))
                return B;
        }
    return nullptr;
}

static Function *emitPairWorkFn(const NeighborLoopInfo &Info, PairPhase Phase,
                                bool RuntimeActivate = false,
                                PairEnvKind EnvKind = PairEnvKind::State)
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

    /* Runtime pair callback: void(i8* state, i8* ctx, i32 u, i32 v). */
    FunctionType *FT = FunctionType::get(Type::getVoidTy(Ctx),
                                         {I8P, I8P, I32, I32}, false);
    Function *WF = Function::Create(FT, GlobalValue::InternalLinkage,
                                    "sgpl_pair_work", Mod);
    BasicBlock *EntryBB = BasicBlock::Create(Ctx, "entry", WF);
    IRBuilder<> WB(EntryBB);
    Argument *ArgU = WF->getArg(2);
    Argument *ArgV = WF->getArg(3);
    Value *ArgEnv = nullptr;
    if (EnvKind == PairEnvKind::State)
        ArgEnv = WF->getArg(0);
    else if (EnvKind == PairEnvKind::Ctx)
        ArgEnv = WF->getArg(1);
    else
    {
        FunctionCallee PartStateFn = Mod->getOrInsertFunction(
            "autograph_exec_partition_state",
            FunctionType::get(I8P, {I8P}, false));
        ArgEnv = WB.CreateCall(PartStateFn, {WF->getArg(1)});
    }

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
    /* Activation is orthogonal to the body environment: A+ always reads the
     * runtime context, so a reduction/privatization body keeps its partition
     * environment and the append still elides into the activation primitive. */
    const bool UseEnvelope = Phase != PairPhase::UOnly &&
                             envelopeWired(Info) && Info.HasFrontierAppend;
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

    /* Composition A: round-separation shadow reads.  Load the per-base shadow
     * snapshot pointer published by the round preheader (via the pass-created
     * global) and collect every load in the neighbor body whose GEP base is one
     * of the in-place bases; those loads are redirected to the shadow so the
     * pair work fn reads the frozen round-start snapshot while stores stay on
     * the live array. */
    DenseMap<const Value *, Value *> ShadowPtrForBase;
    DenseMap<const Value *, Type *> ShadowTyForBase;
    SmallPtrSet<const LoadInst *, 8> ShadowReadLoads;
    for (unsigned si = 0; si < Info.RoundSepBases.size(); ++si)
    {
        const auto &RS = Info.RoundSepBases[si];
        GlobalVariable *SlotG = cast<GlobalVariable>(
            Mod->getOrInsertGlobal(shadowGlobalName(Info, si), I8P));
        Value *ShadP = WB.CreateLoad(I8P, SlotG, "shadow_ptr");
        ShadowPtrForBase[RS.Base] = ShadP;
        ShadowTyForBase[RS.Base] = RS.ElemTy;
    }
    if (!ShadowPtrForBase.empty())
        for (const auto &RS : Info.RoundSepBases)
            if (ShadowPtrForBase.count(RS.Base))
                ShadowReadLoads.insert(RS.CrossReads.begin(), RS.CrossReads.end());

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

    /* Composition R3: the reduction record IS this partition's private state.
     * Scalar slots sit at fixed 8-byte offsets (the runtime pre-initializes
     * them to the operator identity); an array slot holds a pointer to this
     * partition's private copy of that base, published by the preheader's
     * `autograph_priv_bind` call.  Redirecting every GEP on a privatized base
     * to the private pointer is what makes the body's own load/op/store read
     * and write the copy instead of the shared array. */
    DenseMap<const Value *, Value *> PrivPtrForBase;
    if (Phase == PairPhase::All && Info.UsePrivLayout)
    {
        auto RecFieldPtr = [&](uint64_t Off) -> Value *
        { return WB.CreateGEP(Type::getInt8Ty(Ctx), ArgEnv,
                              ConstantInt::get(I64, (int64_t)Off), "priv_field"); };
        uint64_t Off = 0;
        for (const auto &S : Info.Slots)
        {
            if (S.Ptr && S.ElemTy)
                Map[S.Ptr] =
                    WB.CreateBitCast(RecFieldPtr(Off), PointerType::get(S.ElemTy, 0));
            Off += 8;
        }
        for (const auto &A : Info.PrivArrays)
        {
            Value *P = WB.CreateLoad(I8P, RecFieldPtr(Off), "priv_ptr");
            if (A.Base)
                PrivPtrForBase[A.Base] = P;
            Off += 8;
        }
    }

    /* Shadow read GEP: the shadow buffer is a flat array of ET, so the source
     * array's pointer-deref index (a leading constant zero on the canonical
     * 1-D form `[n x ET], ptr @base, i32 0, i32 %idx`) must be dropped.
     * Keeping it emits `getelementptr ET, ptr %shadow, i32 0, i32 %idx`, which
     * is verifier-invalid for the non-indexable ET and crashes later LLVM
     * passes (EarlyCSE simplifyInstruction). */
    auto MakeShadowGEP = [&](Type *ET, Value *Shad, GetElementPtrInst *PGClone,
                             Instruction *InsertPt) -> GetElementPtrInst *
    {
        SmallVector<Value *, 4> Idx;
        bool First = true;
        for (Use &U : PGClone->indices())
        {
            if (First)
            {
                First = false;
                if (PGClone->getNumIndices() > 1)
                    if (auto *CI = dyn_cast<ConstantInt>(U.get()))
                        if (CI->isZero())
                            continue; /* pointer-deref index */
            }
            Idx.push_back(U.get());
        }
        return GetElementPtrInst::Create(ET, Shad, Idx, "shadow_elem", InsertPt);
    };

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
            /* Composition R3: a GEP on a privatized base is re-pointed at this
             * partition's private copy, so the body's own load/op/store chain
             * accumulates there instead of in the shared array.  This arm only
             * covers values cloned on demand (loop-invariant addresses, the
             * UOnly store clones); the body's own GEPs are pre-cloned before
             * operand remap and are rewritten by the pass after remapInst. */
            if (!PrivPtrForBase.empty())
                if (auto *SGEP = dyn_cast<GetElementPtrInst>(I))
                    if (gepIsArrayRoot(SGEP))
                    {
                        Value *PP = PrivPtrForBase.lookup(canonicalArrayBase(SGEP));
                        if (PP)
                            Clone->setOperand(0, PP);
                    }
            /* Round-separation (composition A): a read on an in-place base
             * loads through the shadow snapshot, not the live array.  The
             * original GEP pointer is cloned normally (base = live array) so
             * sibling stores through the same GEP still write the live array;
             * only the load is re-pointed at the shadow copy, reusing the
             * already-cloned index operands. */
            if (auto *LI = dyn_cast<LoadInst>(I))
                if (ShadowReadLoads.count(LI))
                {
                    Value *Ptr = LI->getPointerOperand();
                    auto *PGEP = dyn_cast<GetElementPtrInst>(Ptr);
                    Value *PClone = Map.count(Ptr) ? Map[Ptr] : nullptr;
                    if (PGEP && PClone)
                        if (auto *PGClone = dyn_cast<GetElementPtrInst>(PClone))
                        {
                            const Value *Base = canonicalArrayBase(PGEP);
                            Value *Shad = ShadowPtrForBase.lookup(Base);
                            Type *ET = ShadowTyForBase.lookup(Base);
                            if (Shad && ET)
                            {
                                GetElementPtrInst *ShadowGEP =
                                    MakeShadowGEP(ET, Shad, PGClone, Clone);
                                Clone->setOperand(0, ShadowGEP);
                            }
                        }
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

    /* Composition R3: after every operand is remapped, re-point the body's
     * address computations on a privatized base at this partition's private
     * copy.  This must be a pass of its own: phase 1 above pre-clones every
     * body instruction into Map before operand remap, so CloneValue returns
     * those clones from its Map hit and never reaches its own redirect.
     * Without this the body's load/op/store chain keeps the shared array, the
     * private copies stay at the operator identity, and the loop runs with a
     * lost-update race that the combine cannot repair. */
    if (!PrivPtrForBase.empty())
        for (BasicBlock *BB : BodyBlocks)
            for (Instruction &I : *BB)
            {
                auto *GEP = dyn_cast<GetElementPtrInst>(&I);
                if (!GEP || !Map.count(GEP) || !gepIsArrayRoot(GEP))
                    continue;
                Value *PP = PrivPtrForBase.lookup(canonicalArrayBase(GEP));
                if (PP)
                    cast<Instruction>(Map[GEP])->setOperand(0, PP);
            }

    /* Round-separation (composition A): after every operand is remapped,
     * re-point the shadow reads at the shadow snapshot.  Runs as a separate
     * pass so the cloned index operands of the read's GEP are final no
     * matter which block the GEP lives in.  Stores through the same GEP keep
     * the live array. */
    if (!ShadowPtrForBase.empty())
        for (BasicBlock *BB : BodyBlocks)
            for (Instruction &I : *BB)
            {
                if (!Map.count(&I))
                    continue;
                auto *LI = dyn_cast<LoadInst>(&I);
                if (!LI || !ShadowReadLoads.count(LI))
                    continue;
                Value *Ptr = LI->getPointerOperand();
                auto *PGEP = dyn_cast<GetElementPtrInst>(Ptr);
                Value *PClone = Map.count(Ptr) ? Map[Ptr] : nullptr;
                auto *PGClone = dyn_cast_or_null<GetElementPtrInst>(PClone);
                if (!PGEP || !PGClone)
                    continue;
                const Value *Base = canonicalArrayBase(PGEP);
                Value *Shad = ShadowPtrForBase.lookup(Base);
                Type *ET = ShadowTyForBase.lookup(Base);
                if (!Shad || !ET)
                    continue;
                Instruction *LoadClone = cast<Instruction>(Map[&I]);
                GetElementPtrInst *ShadowGEP =
                    MakeShadowGEP(ET, Shad, PGClone, LoadClone);
                LoadClone->setOperand(0, ShadowGEP);
            }

    /* Rewrite elided frontier appends in-place: the legacy path marks
     * dest_seen[v] and leaves the append to the step; the direct expression
     * path calls the explicit activation primitive, which claims dest_seen and
     * appends on the 0->1 transition.  In that mode the pair wrapper passes the
     * runtime context as the work environment. */
    BasicBlock *FirstClone = nullptr;
    for (BasicBlock *BB : BodyBlocks)
    {
        BasicBlock *CloneBB = BBMap[BB];
        if (!FirstClone)
            FirstClone = CloneBB;
        if (UseEnvelope && (SeenBase || RuntimeActivate))
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
                if (RuntimeActivate)
                {
                    /* Activation is an operation independent of the body's
                     * environment: it always uses the runtime context, so a
                     * fused Reduce/PrivateAccumulate pair callback (whose body
                     * env is the partition state) can carry A+ as well. */
                    FunctionCallee Activate = Mod->getOrInsertFunction(
                        "autograph_frontier_activate",
                        FunctionType::get(I32, {I8P, I32}, false));
                    B3.CreateCall(Activate, {WF->getArg(1), Vtx});
                }
                else
                {
                    Value *Slot = B3.CreateGEP(I32, SeenBase, Vtx);
                    B3.CreateStore(ConstantInt::get(I32, 1), Slot);
                }
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
    /* Per-source first-wins claims: each claim runs once per source in a
     * SOURCE_BEGIN operation and publishes its outcome in the per-source
     * state array S[u][j].  Only claims whose guard dominates the loop gate
     * the pair body (a preamble claim that does not wrap the loop performs
     * its transition without gating the pairs). */
    {
        SmallVector<const Effect *, 2> Claims = perSourceClaims(Info);
        bool AnyGate = false;
        for (const Effect *C : Claims)
            if (claimGatesLoop(Info, C))
                AnyGate = true;
        if (AnyGate)
        {
            GlobalVariable *ClaimG = cast<GlobalVariable>(
                Mod->getOrInsertGlobal(claimGlobalName(Info), I8P));
            Value *Arr = WB.CreateLoad(I8P, ClaimG, "claim_res");
            Value *ArrI32 = WB.CreateBitCast(Arr, PointerType::get(I32, 0));
            const unsigned K = (unsigned)Claims.size();
            Value *Won = nullptr;
            for (unsigned j = 0; j < K; ++j)
            {
                if (!claimGatesLoop(Info, Claims[j]))
                    continue;
                Value *Idx = WB.CreateAdd(
                    WB.CreateMul(ArgU, ConstantInt::get(I32, (int32_t)K)),
                    ConstantInt::get(I32, (int32_t)j), "claim_slot");
                Value *Res = WB.CreateLoad(I32, WB.CreateGEP(I32, ArrI32, Idx),
                                           "claim_won");
                Value *One = WB.CreateICmpNE(Res, ConstantInt::get(I32, 0));
                Won = Won ? WB.CreateAnd(Won, One) : One;
            }
            WB.CreateCondBr(Won, FirstClone, RetStub);
        }
        else
            WB.CreateBr(FirstClone);
    }

    /* Drop the address computations the rewrites left unused: a GEP re-based on
     * a private copy (or a load re-pointed at a shadow) no longer consumes the
     * base-slot load that was cloned for it.  The backend folds those loads
     * away regardless, but leaving the shared base named in the emitted body
     * makes "this body never touches the shared array" unreadable -- for the
     * postcondition below, for the suite's IR guard, and for anyone reading a
     * disassembly. */
    {
        SmallVector<Instruction *, 16> Dead;
        auto isDeadClone = [](Instruction *I)
        { return I->use_empty() && !I->isTerminator() && !I->mayHaveSideEffects(); };
        for (BasicBlock &BB : *WF)
            for (Instruction &I : BB)
                if (isDeadClone(&I))
                    Dead.push_back(&I);
        while (!Dead.empty())
        {
            Instruction *I = Dead.pop_back_val();
            if (!isDeadClone(I))
                continue;
            SmallVector<Value *, 4> Ops(I->op_begin(), I->op_end());
            I->eraseFromParent();
            for (Value *O : Ops)
                if (auto *OI = dyn_cast<Instruction>(O))
                    if (isDeadClone(OI))
                        Dead.push_back(OI);
        }
    }

    /* Composition R3 postcondition.  A privatization proof whose emission is
     * missing is not a proof: if any access to a privatized base survives on
     * the shared array, refuse the work function.  The privatized path then
     * fails to build and the loop falls through to the sequential marker,
     * which answers exactly like the unrewritten program.
     * SGPL_FRONTIER_STRICT=1 turns the refusal into an abort so a test run
     * cannot mistake a silently sequential loop for a passing rewrite. */
    if (Phase == PairPhase::All && Info.UsePrivLayout)
        if (const Value *Shared = privBaseStillShared(WF, Info))
        {
            errs() << "[graph-frontier] POSTCONDITION FAILURE on "
                   << F->getName() << ": privatized base "
                   << (Shared->hasName() ? Shared->getName() : "<unnamed>")
                   << " is still reached through the shared array in "
                   << WF->getName() << "\n";
            if (getenv("SGPL_FRONTIER_STRICT"))
                std::abort();
            WF->eraseFromParent();
            return nullptr;
        }
    return WF;
}

/* Clone the driver's per-source epilogue (`arr[u] = acc`) into the finish hook:
 * the accumulator load becomes `AccRepl` (the partition's partial, or the
 * operator identity for a source with no pairs) and the driver's source value
 * becomes `SrcArg`.  Value-shaping instructions only (loads, GEPs, casts,
 * binary ops, compares, selects); anything that could observe engine or
 * traversal state fails closed, and the caller then declines to rewrite. */
static Value *clonePreambleValue(Value *V, IRBuilder<> &B,
                                 const NeighborLoopInfo &Info, Value *SrcArg,
                                 Value *AccRepl,
                                 SmallDenseMap<Value *, Value *> &Map, bool &Ok,
                                 unsigned Depth)
{
    using namespace llvm;
    if (!V || Depth > 24)
    {
        Ok = false;
        return nullptr;
    }
    if (Map.count(V))
        return Map[V];
    if (isa<Constant>(V) || isa<Argument>(V))
        return V;
    if (Loop *DL = Info.DriverLoop)
        if (V == driverIndVar(DL))
        {
            Value *S = SrcArg;
            if (V->getType() != S->getType())
            {
                if (V->getType()->isIntegerTy(64) && S->getType()->isIntegerTy(32))
                    S = B.CreateZExt(S, V->getType());
                else
                {
                    Ok = false;
                    return nullptr;
                }
            }
            Map[V] = S;
            return S;
        }
    auto *I = dyn_cast<Instruction>(V);
    if (!I)
    {
        Ok = false;
        return nullptr;
    }
    if (auto *LI = dyn_cast<LoadInst>(I))
        if (LI->getPointerOperand()->stripPointerCasts() == Info.ReducePtr)
        {
            Map[V] = AccRepl;
            return AccRepl;
        }
    if (!isa<LoadInst>(I) && !isa<BinaryOperator>(I) && !isa<GetElementPtrInst>(I) &&
        !isa<CastInst>(I) && !isa<ICmpInst>(I) && !isa<SelectInst>(I))
    {
        Ok = false;
        return nullptr;
    }
    Instruction *Cl = I->clone();
    Map[V] = Cl;
    for (unsigned oi = 0; oi < Cl->getNumOperands(); ++oi)
    {
        Value *M = clonePreambleValue(Cl->getOperand(oi), B, Info, SrcArg, AccRepl,
                                      Map, Ok, Depth + 1);
        if (!Ok)
        {
            Cl->deleteValue();
            Map.erase(V);
            return nullptr;
        }
        Cl->setOperand(oi, M);
    }
    B.Insert(Cl);
    return Cl;
}

/* Build the per-source finish hook `void sgpl_source_finish(i32 src, i8* partial)`:
 * consume the accumulated partial (write that source's result) and reset it to
 * the operator identity for the next source in the partition.  Shared by the
 * legacy source-red step and the direct expression path. */
static Function *buildSourceFinishHook(const NeighborLoopInfo &Info, Module *Mod,
                                       LLVMContext &Ctx, Type *ElemTy)
{
    if (!Info.AccConsumeStore)
        return nullptr;
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);
    FunctionType *FFT = FunctionType::get(Type::getVoidTy(Ctx), {I32, I8P}, false);
    Function *Finish = Function::Create(FFT, GlobalValue::InternalLinkage,
                                        "sgpl_source_finish", Mod);
    BasicBlock *FB = BasicBlock::Create(Ctx, "entry", Finish);
    IRBuilder<> FBi(FB);
    Value *PartArg = Finish->getArg(1);
    Value *AccLoad = FBi.CreateLoad(ElemTy,
                                    FBi.CreateBitCast(PartArg, PointerType::get(ElemTy, 0)),
                                    "acc");
    SmallDenseMap<Value *, Value *> FMap;
    bool Ok = true;
    StoreInst *SI = Info.AccConsumeStore;
    Value *V = clonePreambleValue(SI->getValueOperand(), FBi, Info, Finish->getArg(0),
                                  AccLoad, FMap, Ok, 0);
    Value *P = Ok ? clonePreambleValue(SI->getPointerOperand(), FBi, Info,
                                       Finish->getArg(0), AccLoad, FMap, Ok, 0)
                  : nullptr;
    if (!Ok || !V || !P || V->getType() != SI->getValueOperand()->getType())
    {
        Finish->eraseFromParent();
        return nullptr;
    }
    FBi.CreateStore(V, P, SI->isVolatile());
    /* Reset for the next source in this partition. */
    FBi.CreateStore(identityFor(Info.ReduceOp, ElemTy), PartArg, false);
    FBi.CreateRetVoid();
    return Finish;
}

/* Composition R3 setup shared by the legacy privatized step and the direct
 * expression path: build the pair work function, the per-partition record
 * (scalar partials initialized to the operator identity; array copies bound
 * and identity-initialized by the runtime), the targets descriptor and the
 * per-operator folder.  `PreambleFn` is non-null when the driver preamble
 * carries per-source updates (`w[u] += 1`). */
/* Composition R3: fill every partition's scalar partials in the record with
 * their operator identity.  The array copies are initialized by
 * `autograph_priv_bind`; the scalar fields are the compiler's own (the record
 * layout is private to the emitted code), so they are filled here in one pass
 * over the partitions. */
static void emitPrivScalarInit(IRBuilder<> &EB, BasicBlock *Pre, Value *Rec,
                               Value *PartCount, uint64_t RecSize,
                               const NeighborLoopInfo &Info, LLVMContext &Ctx)
{
    Type *I8 = Type::getInt8Ty(Ctx);
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I64 = Type::getInt64Ty(Ctx);
    Instruction *Term = &*EB.GetInsertPoint();
    BasicBlock *Cont = Pre->splitBasicBlock(Term, "priv_fill_cont");
    Function *Fn = Pre->getParent();
    BasicBlock *Cond = BasicBlock::Create(Ctx, "priv_fill_cond", Fn, Cont);
    BasicBlock *Body = BasicBlock::Create(Ctx, "priv_fill_body", Fn, Cont);
    Pre->getTerminator()->eraseFromParent();
    IRBuilder<>(Pre).CreateBr(Cond);
    IRBuilder<> CB(Cond);
    PHINode *IV = CB.CreatePHI(I32, 2, "pi");
    IV->addIncoming(ConstantInt::get(I32, 0), Pre);
    CB.CreateCondBr(CB.CreateICmpSLT(IV, PartCount), Body, Cont);
    IRBuilder<> BB(Body);
    Value *Base = BB.CreateGEP(
        I8, Rec,
        BB.CreateMul(BB.CreateZExt(IV, I64), ConstantInt::get(I64, (int64_t)RecSize)));
    for (unsigned i = 0; i < Info.Slots.size(); ++i)
    {
        const auto &S = Info.Slots[i];
        Value *F = BB.CreateBitCast(
            BB.CreateGEP(I8, Base, ConstantInt::get(I64, 8 * (int64_t)i)),
            PointerType::get(S.ElemTy, 0));
        BB.CreateStore(identityFor(S.Op, S.ElemTy), F);
    }
    Value *Nxt = BB.CreateAdd(IV, ConstantInt::get(I32, 1));
    BB.CreateBr(Cond);
    IV->addIncoming(Nxt, Body);
    EB.SetInsertPoint(Cont->getTerminator());
}

/* Composition R3: fold one partition's private state into the real targets.
 *
 * `targets` is the compiler-built descriptor — one pointer per scalar slot,
 * one pointer per privatized array, then one element count per array.  The
 * runtime calls this once per partition in ascending partition order, so the
 * fold order is deterministic.  Each merge reuses the same per-operator
 * combiner the scalar reduction path uses (`*out = *out op *partial`), which is
 * exactly the element-wise fold an array copy needs; the private copies start
 * at the operator identity, so the running total starts from the array's own
 * pre-round value. */
static Function *emitPrivCombiner(LLVMContext &Ctx, Module *Mod,
                                  const NeighborLoopInfo &Info)
{
    Type *I8 = Type::getInt8Ty(Ctx);
    Type *I64 = Type::getInt64Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);
    FunctionType *FT = FunctionType::get(Type::getVoidTy(Ctx), {I8P, I8P}, false);
    Function *FN = Function::Create(FT, GlobalValue::InternalLinkage,
                                    "sgpl_priv_combine", Mod);
    BasicBlock *Entry = BasicBlock::Create(Ctx, "entry", FN);
    IRBuilder<> B(Entry);
    Value *Rec = FN->getArg(0);
    Value *Tgts = FN->getArg(1);
    const unsigned NS = (unsigned)Info.Slots.size();
    const unsigned NA = (unsigned)Info.PrivArrays.size();

    for (unsigned i = 0; i < NS; ++i)
    {
        const auto &S = Info.Slots[i];
        if (!S.ElemTy)
        {
            FN->eraseFromParent();
            return nullptr;
        }
        Function *Cmb = emitRedCombiner(Ctx, Mod, S.Op, S.ElemTy);
        Value *Part = B.CreateBitCast(
            B.CreateGEP(I8, Rec, ConstantInt::get(I64, 8 * (int64_t)i)), I8P);
        Value *Out = B.CreateLoad(I8P,
                                  B.CreateGEP(I8P, Tgts, ConstantInt::get(I64, i)),
                                  "slot_out");
        B.CreateCall(Cmb, {Part, Out});
    }

    for (unsigned j = 0; j < NA; ++j)
    {
        const auto &A = Info.PrivArrays[j];
        if (!A.ElemTy)
        {
            FN->eraseFromParent();
            return nullptr;
        }
        Function *Cmb = emitRedCombiner(Ctx, Mod, A.Op, A.ElemTy);
        /* Byte offsets: the element type must be i8, or the index is scaled by
         * the pointee size (a pointer-type element multiplies by 8). */
        Value *Priv = B.CreateLoad(
            I8P, B.CreateGEP(I8, Rec, ConstantInt::get(I64, 8 * (int64_t)(NS + j))),
            "priv_copy");
        Value *Arr = B.CreateLoad(
            I8P, B.CreateGEP(I8, Tgts, ConstantInt::get(I64, 8 * (int64_t)(NS + j))),
            "arr_base");
        Value *Cnt = B.CreateLoad(
            I64, B.CreateGEP(I8, Tgts, ConstantInt::get(I64, 8 * (int64_t)(NS + NA + j))),
            "arr_elems");
        const uint64_t ESz =
            std::max<uint64_t>(1, (A.ElemTy->getPrimitiveSizeInBits() + 7) / 8);

        BasicBlock *Cond = BasicBlock::Create(Ctx, "fold_cond", FN);
        BasicBlock *Body = BasicBlock::Create(Ctx, "fold_body", FN);
        BasicBlock *Cont = BasicBlock::Create(Ctx, "fold_cont", FN);
        B.CreateBr(Cond);

        IRBuilder<> CB(Cond);
        PHINode *K = CB.CreatePHI(I64, 2, "k");
        K->addIncoming(ConstantInt::get(I64, 0), Entry);
        CB.CreateCondBr(CB.CreateICmpSLT(K, Cnt), Body, Cont);

        IRBuilder<> BB(Body);
        Value *Off = BB.CreateMul(K, ConstantInt::get(I64, (int64_t)ESz));
        Value *PE = BB.CreateGEP(I8, Priv, Off);
        Value *AE = BB.CreateGEP(I8, Arr, Off);
        BB.CreateCall(Cmb, {PE, AE});
        Value *KN = BB.CreateAdd(K, ConstantInt::get(I64, 1));
        BB.CreateBr(Cond);
        K->addIncoming(KN, Body);

        B.SetInsertPoint(Cont);
    }

    B.CreateRetVoid();
    return FN;
}

struct PrivSetup
{
    Function *WF = nullptr;
    Function *PreambleFn = nullptr;
    Function *Combiner = nullptr;
    AllocaInst *Rec = nullptr;
    AllocaInst *Tgts = nullptr;
    Value *GraphArg = nullptr;
    Value *PartCount = nullptr;
};

static bool buildPrivSetup(const NeighborLoopInfo &Info, PrivSetup &S,
                           bool RuntimeActivate = false)
{
    Function *F = Info.NeighborLoop->getHeader()->getParent();
    LLVMContext &Ctx = F->getContext();
    Module *Mod = F->getParent();
    Type *I8 = Type::getInt8Ty(Ctx);
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I64 = Type::getInt64Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);

    S.WF = emitPairWorkFn(Info, PairPhase::All, RuntimeActivate,
                          PairEnvKind::Partition);
    if (!S.WF)
        return false;

    BasicBlock *Pre = Info.DriverLoop->getLoopPreheader();
    if (!Pre || !Pre->getTerminator())
        return false;
    IRBuilder<> EB(Pre, Pre->getFirstInsertionPt());
    EB.SetInsertPoint(Pre->getTerminator());
    S.GraphArg = Info.GraphPtr;
    if (auto *GL = dyn_cast<LoadInst>(Info.GraphPtr))
        if (GL->getPointerOperand())
            S.GraphArg = EB.CreateLoad(GL->getType(), GL->getPointerOperand());
    FunctionCallee BuildCC = Mod->getOrInsertFunction(
        "autograph_build_clean_cut", FunctionType::get(I32, {I8P, I32}, false));
    S.PartCount = EB.CreateCall(BuildCC, {S.GraphArg, ConstantInt::get(I32, 0)});

    const unsigned NS = (unsigned)Info.Slots.size();
    const unsigned NA = (unsigned)Info.PrivArrays.size();
    const uint64_t RecSize = 8 * (uint64_t)(NS + NA);

    Value *RecBytes =
        EB.CreateMul(EB.CreateZExt(S.PartCount, I64), ConstantInt::get(I64, (int64_t)RecSize));
    S.Rec = EB.CreateAlloca(I8, RecBytes, "priv_rec");

    /* Scalar partials: every partition's slots start at the operator identity
     * (the runtime initializes the array copies inside autograph_priv_bind). */
    if (NS > 0)
        emitPrivScalarInit(EB, Pre, S.Rec, S.PartCount, RecSize, Info, Ctx);

    if (!Info.DriverUStores.empty())
    {
        S.PreambleFn =
            emitPairWorkFn(Info, PairPhase::UOnly, false, PairEnvKind::Partition);
        if (!S.PreambleFn)
            return false;
    }

    FunctionCallee Bind = Mod->getOrInsertFunction(
        "autograph_priv_bind",
        FunctionType::get(I32, {I8P, I8P, I64, I64, I64, I64, I64, I32}, false));
    for (unsigned j = 0; j < NA; ++j)
    {
        const auto &A = Info.PrivArrays[j];
        Value *Elems = EB.CreateZExtOrTrunc(A.Count, I64);
        const uint64_t ESz =
            std::max<uint64_t>(1, (A.ElemTy->getPrimitiveSizeInBits() + 7) / 8);
        Constant *Ident = identityFor(A.Op, A.ElemTy);
        uint64_t IdentBits = 0;
        if (auto *CI = dyn_cast<ConstantInt>(Ident))
            IdentBits = (uint64_t)CI->getZExtValue();
        else if (auto *CF = dyn_cast<ConstantFP>(Ident))
            IdentBits = (uint64_t)CF->getValueAPF().bitcastToAPInt().getZExtValue();
        else if (Ident->isNullValue())
            IdentBits = 0;
        else
            return false;
        EB.CreateCall(Bind, {S.GraphArg, EB.CreateBitCast(S.Rec, I8P),
                             ConstantInt::get(I64, (int64_t)RecSize),
                             ConstantInt::get(I64, 8 * (int64_t)(NS + j)), Elems,
                             ConstantInt::get(I64, (int64_t)ESz),
                             ConstantInt::get(I64, (int64_t)IdentBits),
                             ConstantInt::get(I32, (int32_t)j)});
    }

    /* Targets descriptor: [slot ptrs][array ptrs][array element counts]. */
    const uint64_t TgtSize = 8 * (uint64_t)(NS + 2 * NA);
    S.Tgts = EB.CreateAlloca(I8, ConstantInt::get(I64, (int64_t)TgtSize),
                             "priv_targets");
    auto TgtField = [&](unsigned Index) -> Value *
    { return EB.CreateGEP(I8, S.Tgts, ConstantInt::get(I64, 8 * (int64_t)Index)); };
    for (unsigned i = 0; i < NS; ++i)
    {
        const auto &Slot = Info.Slots[i];
        EB.CreateStore(EB.CreateBitCast(const_cast<Value *>(Slot.Ptr), I8P),
                       TgtField(i));
    }
    for (unsigned j = 0; j < NA; ++j)
    {
        const auto &A = Info.PrivArrays[j];
        EB.CreateStore(EB.CreateBitCast(A.Ptr, I8P), TgtField(NS + j));
        EB.CreateStore(EB.CreateZExtOrTrunc(A.Count, I64), TgtField(NS + NA + j));
    }

    S.Combiner = emitPrivCombiner(Ctx, Mod, Info);
    if (!S.Combiner)
        return false;
    return true;
}

/* Pair-phase write regions (Activate excluded: it is an operation, not a base
 * write).  Per-source preamble mutations — including first-wins claims — are
 * not pair-phase and are realized as source-begin operations. */
static void pairPhaseWriteRegions(const NeighborLoopInfo &Info, bool &U, bool &V)
{
    U = V = false;
    forEachExprEffect(Info, [&](const Effect &E, PhaseSegment Seg)
    {
        if (Seg != PhaseSegment::Pair || !effectIsMutating(E) ||
            E.Kind == EffectKind::Activate)
            return;
        if (E.Reg == Region::U)
            U = true;
        if (E.Reg == Region::V)
            V = true;
    });
}

/* Semantic admissibility of a modelable expression (R7).  This is the only
 * gate besides modelability: it states the correctness conditions the frozen
 * algebra proves (no owner for data-derived writes unless the pure
 * accumulation theorem applies; Top provenance refuses; carried reads on
 * mutated bases refuse unless privatized or snapshot-resolved; per-source
 * claims are occurrence-preserving and stay refused until claim staging;
 * round-separation bases must agree on their write endpoint; dual ownership
 * requires disjoint bases and no cross-phase dependence; frontier appends
 * require the wired envelope; reduction/source-reduction must satisfy their
 * own laws).  Strategy names are not involved. */
static bool supportedByAlgebra(NeighborLoopInfo &Info, const EffectSummary &S,
                               bool Priv, std::string &reason)
{
    bool PairU = false, PairV = false;
    pairPhaseWriteRegions(Info, PairU, PairV);
    if (Info.HasDataWrite && !Priv)
    {
        reason = "data-derived write without a privatization proof";
        return false;
    }
    if (hasPerSourceClaim(Info) && perSourceClaims(Info).empty())
    {
        reason = "per-source claim (occurrence preservation)";
        return false;
    }
    if (S.MutTop)
    {
        reason = "unknown index provenance (Top)";
        return false;
    }
    if (S.HasCarriedOnMut && !Priv)
    {
        reason = "carried read on a mutated base";
        return false;
    }
    if (!Info.RoundSepBases.empty())
    {
        /* Every shadow base must not be written in the pair phase at its
         * read endpoint (the frozen side); writes at its own write endpoint
         * are the normal single-ownership pattern, and bases only read in the
         * pair phase (a claim at U, cross-read at V) are fine. */
        bool RoundSepOK = true;
        for (const auto &RS : Info.RoundSepBases)
        {
            if (!RS.Base)
                continue;
            const Region Opp = RS.WritesV ? Region::U : Region::V;
            forEachExprEffect(Info, [&](const Effect &E, PhaseSegment Seg)
            {
                if (Seg != PhaseSegment::Pair || !effectIsMutating(E))
                    return;
                if (E.Base == RS.Base && E.Reg == Opp)
                    RoundSepOK = false;
            });
        }
        if (!RoundSepOK)
        {
            reason = "pair-phase write at the shadow read endpoint";
            return false;
        }
    }
    if (PairU && PairV && !S.MutG)
    {
        bool Disjoint = true;
        for (const Value *B : S.BaseU)
            if (S.BaseV.count(B))
                Disjoint = false;
        if (!(Disjoint && !S.BaseU.empty() && !S.BaseV.empty() &&
              !crossPhaseDataDep(Info, S.BaseU, S.BaseV) &&
              Info.RoundSepBases.empty()))
        {
            if (Priv)
                return true;
            reason = "same-base or cross-phase U+V without privatization";
            return false;
        }
    }
    if (Info.HasFrontierAppend && !envelopeWired(Info))
    {
        reason = "frontier append without a wired envelope";
        return false;
    }
    if (Info.ReducePtr && !Priv)
    {
        if (Info.AccConsumeStore)
        {
            if (!(Info.AccResetSeen && Info.ReduceOp != RedOp::None &&
                  !Info.HasDataWrite && !S.MutV && !S.MutTop &&
                  !S.HasCarriedOnMut && !S.HasUnrecognizedG &&
                  !Info.HasFrontierAppend && !hasPerSourceClaim(Info) &&
                  preambleMutationCount(Info) == 0))
            {
                reason = "source-reduction conditions not met";
                return false;
            }
        }
        else if (!(S.MutG && S.HasUopG && !S.HasUnrecognizedG && !PairU &&
                   !PairV && !privLayoutNeeded(Info)))
        {
            if (Priv)
                return true;
            reason = "reduction conditions not met";
            return false;
        }
    }
    return true;
}

/* Structural facts derived from the effect expression and the analysis
 * predicates — no strategy names.  These drive the recursive interpreter. */
struct ExprFacts
{
    bool MutU = false, MutV = false, MutG = false;
    bool IsDual = false, IsPriv = false, IsRed = false, IsSourceRed = false;
    bool CrossPhase = false;
    bool IsV = false; /* single stage writes V (destination-owned rows) */
};

static void deriveExprFacts(NeighborLoopInfo &Info, ExprFacts &F)
{
    EffectSummary S;
    summarizeEffects(Info, S);
    F.MutU = S.MutU;
    F.MutV = S.MutV;
    F.MutG = S.MutG;
    /* Pair-phase write regions decide the stage structure; per-source preamble
     * updates (including first-wins claims) are realized as source-begin
     * operations, not as a U pair stage. */
    bool PairU = false, PairV = false;
    pairPhaseWriteRegions(Info, PairU, PairV);
    F.IsPriv = Info.UsePrivLayout;
    F.IsRed = Info.ReducePtr && !Info.AccConsumeStore && !F.IsPriv;
    F.IsSourceRed = Info.ReducePtr && Info.AccConsumeStore && !F.IsPriv;
    F.IsDual = PairU && PairV && !F.IsPriv && !Info.ReducePtr;
    F.CrossPhase = crossPhaseDataDep(Info, S.BaseU, S.BaseV);
    F.IsV = PairV && !PairU;
}

/* Emit a module-constant resource/access table (sgpl_res_access[]) and return
 * its i8* plus the entry count for autograph_exec_op_create.  The layout is the
 * C struct { uint32_t resource; uint8_t mode; } (size 8, align 4). */
static std::pair<Value *, uint32_t>
emitResourceTable(IRBuilder<> &EB, Module *Mod, LLVMContext &Ctx,
                  ArrayRef<std::pair<uint32_t, uint8_t>> Entries)
{
    if (Entries.empty())
        return {nullptr, 0u};
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I8 = Type::getInt8Ty(Ctx);
    StructType *ST = StructType::get(Ctx, {I32, I8});
    ArrayType *AT = ArrayType::get(ST, Entries.size());
    SmallVector<Constant *, 8> Fields;
    for (const auto &E : Entries)
        Fields.push_back(ConstantStruct::get(
            ST, {ConstantInt::get(I32, (int64_t)E.first),
                 ConstantInt::get(I8, (int64_t)E.second)}));
    Constant *Init = ConstantArray::get(AT, Fields);
    GlobalVariable *GV = new GlobalVariable(
        *Mod, AT, true, GlobalValue::InternalLinkage, Init,
        "sgpl_res", nullptr, GlobalValue::NotThreadLocal, 0);
    return {EB.CreateBitCast(GV, PointerType::get(Ctx, 0)),
            (uint32_t)Entries.size()};
}

/* Physical-access compatibility (compiler-authoritative): Read/Read,
 * Private/Private and AtomicWrite/AtomicWrite coexist; anything else on the
 * same resource conflicts.  Semantic ordering (Seq/Par structure, claim
 * priority) is handled by G_E and the expression, never by this predicate. */
static bool resourcesConflict(const std::pair<uint32_t, uint8_t> &A,
                              const std::pair<uint32_t, uint8_t> &B)
{
    if (A.first != B.first)
        return false;
    if (A.second == SGPL_ACCESS_READ && B.second == SGPL_ACCESS_READ)
        return false;
    if (A.second == SGPL_ACCESS_PRIVATE && B.second == SGPL_ACCESS_PRIVATE)
        return false;
    if (A.second == SGPL_ACCESS_ATOMIC_WRITE &&
        B.second == SGPL_ACCESS_ATOMIC_WRITE)
        return false;
    return true;
}

/* Does the expression contain a pair-phase first-wins claim? */
static bool hasPairPhaseClaim(const NeighborLoopInfo &Info)
{
    for (const Effect &E : Info.Effects)
        if (E.Kind == EffectKind::Claim && E.Scope == OccurrenceScope::PerPair)
            return true;
    return false;
}

/* Single-stage realization: one execution context plus an ordered op array
 * (preamble ops, Snapshot ops, the fused pair op).  `IsV` selects the
 * destination-owned traversal; reductions use it too. */
static bool emitSingleStage(NeighborLoopInfo &Info, bool IsRed,
                            bool IsSourceRed, bool IsPriv, bool IsV)
{
    /* Activation requires the wired dest envelope; a frontier append without it
     * is a semantic refusal (the algebra cannot express the activation). */
    const bool WantEnvelope = Info.HasFrontierAppend && envelopeWired(Info);
    if (Info.HasFrontierAppend && !WantEnvelope)
        return false;
    /* Reduction / privatization compose with activation and membership: the
     * fused pair callback carries the partition/private body environment and
     * the A+ primitive reads the runtime context (see emitPairWorkFn). */

    const bool IsSimple = !IsRed && !IsSourceRed && !IsPriv;
    if (IsSourceRed && (!Info.ReducePtr || !Info.AccConsumeStore))
        return false;
    if (IsPriv && !Info.UsePrivLayout)
        return false;
    if (IsRed && Info.UsePrivLayout)
        return false; /* the privatized path owns that shape */
    if (Info.UsePrivLayout && !IsPriv)
        return false;

    Function *F = Info.NeighborLoop->getHeader()->getParent();
    LLVMContext &Ctx = F->getContext();
    Module *Mod = F->getParent();
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I64 = Type::getInt64Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);
    Value *Null = ConstantPointerNull::get(cast<PointerType>(I8P));

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

    /* Round-separation (composition A): snapshot the in-place bases for this
     * round; the pair work fn reads the frozen snapshot through the published
     * global.  Emitted in the same preheader as the context construction; the
     * copy itself is a Snapshot operation (R7). */
    SmallVector<std::pair<Function *, GlobalVariable *>, 4> SnapOps;
    emitRoundSepShadow(EB, Info, Mod, I8P, I64, Ctx, GraphArg, SnapOps);

    FunctionCallee PartStateFn = Mod->getOrInsertFunction(
        "autograph_exec_partition_state", FunctionType::get(I8P, {I8P}, false));

    /* Frontier membership and dest envelope: the executor applies the
     * membership gate to its traversal (sources for OWNER_U/OWNER_V rows) and
     * the activation primitive claims dest_seen / appends to next_frontier. */
    const bool WantFt = Info.MembershipGated || WantEnvelope;
    FrontierEnv Env;
    if (WantFt)
        fillFrontierEnv(EB, Info, Mod, I8P, I32, Ctx, GraphArg, Env, WantEnvelope);
    Value *HeadSlot = Null;
    if (WantEnvelope)
    {
        AllocaInst *HS = EB.CreateAlloca(I32, nullptr, "rt_append_head");
        EB.CreateStore(ConstantInt::get(I32, 0), HS);
        HeadSlot = HS;
    }

    /* Per-source first-wins claims: staged as SOURCE_BEGIN operations with a
     * per-source state array S[u][j] published for the pair guards. */
    SmallVector<Function *, 2> ClaimCBs =
        buildClaimState(EB, Info, Mod, Ctx, I8P, I32, I64, GraphArg);

    const bool UsePartEnv = IsRed || IsSourceRed || IsPriv;
    const PairEnvKind EnvKind =
        UsePartEnv ? PairEnvKind::Partition : PairEnvKind::State;
    /* Class setup: work function, per-partition partials, finish hook and
     * folder.  The executor sets the partition state (this partition's partial
     * or privatized record) before invoking pair/combine/source hooks; the
     * pair work function itself is the runtime pair callback. */
    Function *WF = nullptr;
    Function *FinishHook = nullptr;
    Function *Combiner = nullptr;
    Function *PreambleFn = nullptr;
    Value *OpState = Null;
    Value *PartBase = Null;
    Value *PartStride = ConstantInt::get(I64, 0);
    if (IsPriv)
    {
        PrivSetup PS;
        if (!buildPrivSetup(Info, PS, WantEnvelope))
            return false;
        WF = PS.WF;
        PreambleFn = PS.PreambleFn;
        Combiner = PS.Combiner;
        PartBase = EB.CreateBitCast(PS.Rec, I8P);
        PartStride = ConstantInt::get(
            I64, (int64_t)(8 * (Info.Slots.size() + Info.PrivArrays.size())));
        OpState = EB.CreateBitCast(PS.Tgts, I8P);
    }
    else
    {
        WF = emitPairWorkFn(Info, PairPhase::All, WantEnvelope, EnvKind);
        if (!WF)
            return false;
        if (IsRed || IsSourceRed)
        {
            Type *ElemTy = Type::getDoubleTy(Ctx);
            for (User *U : Info.ReducePtr->users())
                if (auto *LI = dyn_cast<LoadInst>(U))
                    ElemTy = LI->getType();
            AllocaInst *Partials = EB.CreateAlloca(ElemTy, PartCount, "red_partials");
            emitPartialInit(EB, Pre, Partials, PartCount, ElemTy, Info.ReduceOp, Ctx);
            PartBase = EB.CreateBitCast(Partials, I8P);
            PartStride = ConstantInt::get(
                I64, (int64_t)((ElemTy->getPrimitiveSizeInBits() + 7) / 8));
            OpState = EB.CreateBitCast(Info.ReducePtr, I8P);
            if (IsRed)
                Combiner = emitRedCombiner(Ctx, Mod, Info.ReduceOp, ElemTy);
            else
                FinishHook = buildSourceFinishHook(Info, Mod, Ctx, ElemTy);
            if ((IsRed && !Combiner) || (IsSourceRed && !FinishHook))
                return false;
        }
    }

    /* Combine callback: fold this partition's partial into the op state (the
     * fold target arrives as the callback's state argument). */
    Function *CombCB = nullptr;
    if (Combiner)
    {
        FunctionType *CombCBTy =
            FunctionType::get(Type::getVoidTy(Ctx), {I8P, I8P}, false);
        CombCB = Function::Create(CombCBTy, GlobalValue::InternalLinkage,
                                  "sgpl_rt_op_combine", Mod);
        IRBuilder<> B(BasicBlock::Create(Ctx, "entry", CombCB));
        Value *Part = B.CreateCall(PartStateFn, {CombCB->getArg(1)});
        B.CreateCall(Combiner, {Part, CombCB->getArg(0)});
        B.CreateRetVoid();
    }

    /* Per-source preamble callback (privatized `w[u] += 1`): updates the live
     * base, not the private copies, and needs no privatization to do so.  The
     * engine's source lifecycle runs it exactly once per source, in the single
     * partition that owns that source (OWNER_U dispatches it inside the
     * partition body at a source change; OWNER_V from the serial coverage
     * pass), so no two workers ever update the same element.  The copies are
     * identity-seeded, so the combine adds the pair phase's deltas on top of
     * these per-source writes and the total is the serial one.  Routing the
     * preamble through the copies instead would also have to hold under
     * OWNER_V, where the coverage pass drives source_begin from the shared
     * context and binds no partition record at all. */
    Function *SrcBeginCB = nullptr;
    if (PreambleFn)
    {
        FunctionType *SBTy =
            FunctionType::get(Type::getVoidTy(Ctx), {I8P, I8P, I32}, false);
        SrcBeginCB = Function::Create(SBTy, GlobalValue::InternalLinkage,
                                      "sgpl_rt_source_begin", Mod);
        IRBuilder<> B(BasicBlock::Create(Ctx, "entry", SrcBeginCB));
        B.CreateCall(PreambleFn, {Null, SrcBeginCB->getArg(1),
                                  SrcBeginCB->getArg(2), ConstantInt::get(I32, -1)});
        B.CreateRetVoid();
    }

    /* Per-source finish callback (source reduction): consume this partition's
     * partial for source u and reset it to the operator identity. */
    Function *SrcEndCB = nullptr;
    if (FinishHook)
    {
        FunctionType *SETy =
            FunctionType::get(Type::getVoidTy(Ctx), {I8P, I8P, I32}, false);
        SrcEndCB = Function::Create(SETy, GlobalValue::InternalLinkage,
                                    "sgpl_rt_source_end", Mod);
        IRBuilder<> B(BasicBlock::Create(Ctx, "entry", SrcEndCB));
        Value *Part = B.CreateCall(PartStateFn, {SrcEndCB->getArg(1)});
        B.CreateCall(FinishHook, {SrcEndCB->getArg(2), Part});
        B.CreateRetVoid();
    }

    FunctionCallee OpCreate = Mod->getOrInsertFunction(
        "autograph_exec_op_create",
        FunctionType::get(I8P, {I64, I8P, I8P, I8P, I8P, I8P, I8P, I8P, I32},
                          false));
    auto MakeOp = [&](uint64_t Caps, Value *St, Function *P, Function *C,
                      Function *SB, Function *SE, Function *Sn,
                      ArrayRef<std::pair<uint32_t, uint8_t>> Res) -> Value *
    {
        auto RT = emitResourceTable(EB, Mod, Ctx, Res);
        return EB.CreateCall(OpCreate,
                             {ConstantInt::get(I64, (int64_t)Caps), St,
                              P ? EB.CreateBitCast(P, I8P) : Null,
                              C ? EB.CreateBitCast(C, I8P) : Null,
                              SB ? EB.CreateBitCast(SB, I8P) : Null,
                              SE ? EB.CreateBitCast(SE, I8P) : Null,
                              Sn ? EB.CreateBitCast(Sn, I8P) : Null,
                              RT.first ? RT.first : Null,
                              ConstantInt::get(I32, (int64_t)RT.second)});
    };

    /* Ordered operation groups: per-source claim/preamble ops, the round's
     * Snapshot ops, then the fused pair op. */
    const std::pair<uint32_t, uint8_t> ResClaim{SGPL_RES_CLAIM,
                                                SGPL_ACCESS_ATOMIC_WRITE};
    const std::pair<uint32_t, uint8_t> ResPrivate{SGPL_RES_PRIVATE,
                                                  SGPL_ACCESS_PRIVATE};
    const std::pair<uint32_t, uint8_t> ResPartial{SGPL_RES_PARTIAL,
                                                  SGPL_ACCESS_PRIVATE};
    Value *PreambleOp = nullptr;
    if (SrcBeginCB)
        PreambleOp = MakeOp(SGPL_OP_SOURCE_BEGIN, Null, nullptr, nullptr,
                            SrcBeginCB, nullptr, nullptr,
                            {ResPrivate});
    SmallVector<Value *, 2> ClaimOps;
    for (Function *CB : ClaimCBs)
        ClaimOps.push_back(MakeOp(SGPL_OP_SOURCE_BEGIN, Null, nullptr, nullptr,
                                  CB, nullptr, nullptr, {ResClaim}));
    uint64_t PairCaps = SGPL_OP_PAIR;
    if (CombCB)
        PairCaps |= SGPL_OP_COMBINE;
    if (SrcEndCB)
        PairCaps |= SGPL_OP_SOURCE_END;
    SmallVector<std::pair<uint32_t, uint8_t>, 6> PairRes;
    if (WantFt)
        PairRes.push_back({SGPL_RES_MEMBERSHIP, SGPL_ACCESS_READ});
    if (WantEnvelope)
    {
        PairRes.push_back({SGPL_RES_DEST_SEEN, SGPL_ACCESS_ATOMIC_WRITE});
        PairRes.push_back({SGPL_RES_NEXT_FRONTIER, SGPL_ACCESS_ATOMIC_WRITE});
    }
    if (!Info.RoundSepBases.empty())
        PairRes.push_back({SGPL_RES_SNAPSHOT, SGPL_ACCESS_READ});
    if (IsRed || IsSourceRed)
        PairRes.push_back(ResPartial);
    if (IsPriv)
        PairRes.push_back(ResPrivate);
    if (hasPairPhaseClaim(Info))
        PairRes.push_back(ResClaim);
    Value *PairOp = MakeOp(PairCaps, OpState, WF, CombCB, nullptr, SrcEndCB,
                           nullptr, PairRes);

    const unsigned NOps = (PreambleOp ? 1u : 0u) +
                          (unsigned)ClaimOps.size() +
                          (unsigned)SnapOps.size() + 1u;
    AllocaInst *OpsSlot =
        EB.CreateAlloca(I8P, ConstantInt::get(I32, NOps), "rt_ops");
    unsigned Oi = 0;
    for (Value *CO : ClaimOps)
        EB.CreateStore(CO, EB.CreateGEP(I8P, OpsSlot,
                                        ConstantInt::get(I32, Oi++)));
    if (PreambleOp)
        EB.CreateStore(PreambleOp, EB.CreateGEP(I8P, OpsSlot,
                                                ConstantInt::get(I32, Oi++)));
    for (auto &S : SnapOps)
        EB.CreateStore(
            MakeOp(SGPL_OP_SNAPSHOT, S.second, nullptr, nullptr, nullptr,
                   nullptr, S.first,
                   {{SGPL_RES_SNAPSHOT, SGPL_ACCESS_WRITE}}),
            EB.CreateGEP(I8P, OpsSlot, ConstantInt::get(I32, Oi++)));
    EB.CreateStore(PairOp, EB.CreateGEP(I8P, OpsSlot,
                                        ConstantInt::get(I32, Oi++)));

    /* Traversal mechanism: source-owned slices for source-owned, source
     * reduction and privatized loops, destination-owned rows otherwise. */
    const int32_t Traversal = IsRed || IsV ? 1 : 0;
    FunctionCallee CtxCreate = Mod->getOrInsertFunction(
        "autograph_exec_ctx_create",
        FunctionType::get(I8P,
                          {I8P, I32, I32, I8P, I8P, I8P, I32, I8P, I8P, I64,
                           I8P, I32},
                          false));
    SmallVector<Value *, 12> CtxArgs = {
        GraphArg, ConstantInt::get(I32, Traversal),
        ConstantInt::get(I32, Info.MembershipGated ? SGPL_DOMAIN_FRONTIER
                                                   : SGPL_DOMAIN_ALL_VERTICES),
        WantFt ? Env.Membership : Null /* membership */,
        WantEnvelope ? Env.SeenArg : Null /* dest_seen */,
        WantEnvelope ? Env.NextArg : Null /* next_frontier */,
        ConstantInt::get(I32, 0) /* initial_next_size: appends start at 0 */,
        HeadSlot /* append_head */, PartBase, PartStride, OpsSlot,
        ConstantInt::get(I32, NOps)};
    Value *ExecCtx = EB.CreateCall(CtxCreate, CtxArgs);
    if (!SnapOps.empty())
    {
        /* The round owner publishes the snapshots once, before traversal. */
        FunctionCallee Own = Mod->getOrInsertFunction(
            "autograph_exec_ctx_own_round",
            FunctionType::get(Type::getVoidTy(Ctx), {I8P, I32, I32}, false));
        EB.CreateCall(Own, {ExecCtx, ConstantInt::get(I32, 1),
                            ConstantInt::get(I32, 0)});
    }

    FunctionCallee Exec = Mod->getOrInsertFunction(
        "autograph_frontier_execute", FunctionType::get(I32, {I8P, I8P}, false));
    Value *NewSize = EB.CreateCall(Exec, {GraphArg, ExecCtx});
    if (WantEnvelope)
        commitEnvelope(EB, Info, Mod, I8P, Ctx, GraphArg, NewSize);
    FunctionCallee Destroy = Mod->getOrInsertFunction(
        "autograph_exec_ctx_destroy",
        FunctionType::get(Type::getVoidTy(Ctx), {I8P}, false));
    EB.CreateCall(Destroy, {ExecCtx});

    deactivateDriver(Info);
    return true;
}

/* Independent U-domain ∥ V-domain (crossPhaseDataDep false): the two
 * subexpressions are semantically concurrent, so they are realized by the
 * generic fork/join mechanism instead of being silently serialized.  The
 * enclosing owner context carries the round lifecycle (snapshots and the
 * frontier envelope commit); the children are non-owning and share the round
 * resources. */
static bool emitDualForkJoin(NeighborLoopInfo &Info)
{
    /* Per-source claims compose with dual ownership: the claim operations are
     * staged in the owner's source-begin phase (before either child runs) and
     * both children gate on the shared S[u][j] array.  Pair-body claims ride
     * their phase clones.  Only unsupported claim staging refuses. */
    if (hasPerSourceClaim(Info) && perSourceClaims(Info).empty())
        return false;
    if (Info.ReducePtr || Info.UsePrivLayout)
        return false;
    const bool WantEnvelope = Info.HasFrontierAppend && envelopeWired(Info);
    if (Info.HasFrontierAppend && !WantEnvelope)
        return false;

    Function *WFu = emitPairWorkFn(Info, PairPhase::UOnly, false,
                                   PairEnvKind::State);
    Function *WFv = emitPairWorkFn(Info, PairPhase::VOnly, WantEnvelope,
                                   WantEnvelope ? PairEnvKind::Ctx
                                                : PairEnvKind::State);
    if (!WFu || !WFv)
        return false;

    Function *F = Info.NeighborLoop->getHeader()->getParent();
    LLVMContext &Ctx = F->getContext();
    Module *Mod = F->getParent();
    Type *I32 = Type::getInt32Ty(Ctx);
    Type *I64 = Type::getInt64Ty(Ctx);
    Type *I8P = PointerType::get(Ctx, 0);
    Value *Null = ConstantPointerNull::get(cast<PointerType>(I8P));

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

    SmallVector<std::pair<Function *, GlobalVariable *>, 4> SnapOps;
    emitRoundSepShadow(EB, Info, Mod, I8P, I64, Ctx, GraphArg, SnapOps);

    FrontierEnv Env;
    if (WantEnvelope || Info.MembershipGated)
        fillFrontierEnv(EB, Info, Mod, I8P, I32, Ctx, GraphArg, Env, WantEnvelope);
    Value *HeadSlot = Null;
    if (WantEnvelope)
    {
        AllocaInst *HS = EB.CreateAlloca(I32, nullptr, "rt_append_head");
        EB.CreateStore(ConstantInt::get(I32, 0), HS);
        HeadSlot = HS;
    }

    FunctionCallee OpCreate = Mod->getOrInsertFunction(
        "autograph_exec_op_create",
        FunctionType::get(I8P, {I64, I8P, I8P, I8P, I8P, I8P, I8P, I8P, I32},
                          false));
    SmallVector<std::pair<uint32_t, uint8_t>, 4> URes, VRes;
    if (Info.MembershipGated)
    {
        URes.push_back({SGPL_RES_MEMBERSHIP, SGPL_ACCESS_READ});
        VRes.push_back({SGPL_RES_MEMBERSHIP, SGPL_ACCESS_READ});
    }
    if (!Info.RoundSepBases.empty())
    {
        URes.push_back({SGPL_RES_SNAPSHOT, SGPL_ACCESS_READ});
        VRes.push_back({SGPL_RES_SNAPSHOT, SGPL_ACCESS_READ});
    }
    if (WantEnvelope)
    {
        VRes.push_back({SGPL_RES_DEST_SEEN, SGPL_ACCESS_ATOMIC_WRITE});
        VRes.push_back({SGPL_RES_NEXT_FRONTIER, SGPL_ACCESS_ATOMIC_WRITE});
    }
    if (hasPairPhaseClaim(Info))
    {
        URes.push_back({SGPL_RES_CLAIM, SGPL_ACCESS_ATOMIC_WRITE});
        VRes.push_back({SGPL_RES_CLAIM, SGPL_ACCESS_ATOMIC_WRITE});
    }
    for (const auto &A : URes)
        for (const auto &B : VRes)
            if (resourcesConflict(A, B))
            {
                if (getenv("GRAPH_FRONTIER_STATS"))
                    errs() << "[graph-frontier]   Par children conflict on"
                              " resource " << A.first << "\n";
                return false; /* never silently serialize Par */
            }
    auto UResT = emitResourceTable(EB, Mod, Ctx, URes);
    auto VResT = emitResourceTable(EB, Mod, Ctx, VRes);
    Value *OpU = EB.CreateCall(
        OpCreate, {ConstantInt::get(I64, (int64_t)SGPL_OP_PAIR), Null,
                   EB.CreateBitCast(WFu, I8P), Null, Null, Null, Null,
                   UResT.first ? UResT.first : Null,
                   ConstantInt::get(I32, (int64_t)UResT.second)});
    Value *OpV = EB.CreateCall(
        OpCreate, {ConstantInt::get(I64, (int64_t)SGPL_OP_PAIR), Null,
                   EB.CreateBitCast(WFv, I8P), Null, Null, Null, Null,
                   VResT.first ? VResT.first : Null,
                   ConstantInt::get(I32, (int64_t)VResT.second)});

    SmallVector<Function *, 2> DualClaimCBs =
        buildClaimState(EB, Info, Mod, Ctx, I8P, I32, I64, GraphArg);
    auto ClaimRes = emitResourceTable(
        EB, Mod, Ctx, {{SGPL_RES_CLAIM, SGPL_ACCESS_ATOMIC_WRITE}});
    auto SnapRes = emitResourceTable(
        EB, Mod, Ctx, {{SGPL_RES_SNAPSHOT, SGPL_ACCESS_WRITE}});
    AllocaInst *OpsOwner = EB.CreateAlloca(
        I8P, ConstantInt::get(I32, (unsigned)SnapOps.size() +
                                       (unsigned)DualClaimCBs.size() + 1u),
        "rt_ops_owner");
    unsigned Oi = 0;
    for (Function *CB : DualClaimCBs)
        EB.CreateStore(
            EB.CreateCall(OpCreate,
                          {ConstantInt::get(I64, (int64_t)SGPL_OP_SOURCE_BEGIN),
                           Null, Null, Null,
                           EB.CreateBitCast(CB, I8P), Null, Null,
                           ClaimRes.first ? ClaimRes.first : Null,
                           ConstantInt::get(I32, (int64_t)ClaimRes.second)}),
            EB.CreateGEP(I8P, OpsOwner, ConstantInt::get(I32, Oi++)));
    for (auto &S : SnapOps)
        EB.CreateStore(
            EB.CreateCall(OpCreate,
                          {ConstantInt::get(I64, (int64_t)SGPL_OP_SNAPSHOT),
                           S.second, Null, Null, Null, Null,
                           EB.CreateBitCast(S.first, I8P), Null,
                           SnapRes.first ? SnapRes.first : Null,
                           ConstantInt::get(I32, (int64_t)SnapRes.second)}),
            EB.CreateGEP(I8P, OpsOwner, ConstantInt::get(I32, Oi++)));
    AllocaInst *OpsU = EB.CreateAlloca(I8P, ConstantInt::get(I32, 1), "rt_ops_u");
    EB.CreateStore(OpU, OpsU);
    AllocaInst *OpsV = EB.CreateAlloca(I8P, ConstantInt::get(I32, 1), "rt_ops_v");
    EB.CreateStore(OpV, OpsV);

    const int32_t Domain =
        Info.MembershipGated ? SGPL_DOMAIN_FRONTIER : SGPL_DOMAIN_ALL_VERTICES;
    FunctionCallee CtxCreate = Mod->getOrInsertFunction(
        "autograph_exec_ctx_create",
        FunctionType::get(I8P,
                          {I8P, I32, I32, I8P, I8P, I8P, I32, I8P, I8P, I64,
                           I8P, I32},
                          false));
    Value *Membership =
        Info.MembershipGated || WantEnvelope ? Env.Membership : Null;
    /* The owner carries the round lifecycle (snapshots); the U and V stages
     * are its non-owning children. */
    Value *CtxOwner = EB.CreateCall(
        CtxCreate,
        {GraphArg, ConstantInt::get(I32, SGPL_TRAVERSE_OWNER_U),
         ConstantInt::get(I32, Domain), Membership,
         WantEnvelope ? Env.SeenArg : Null,
         WantEnvelope ? Env.NextArg : Null, ConstantInt::get(I32, 0), HeadSlot,
         Null, ConstantInt::get(I64, 0), OpsOwner,
         ConstantInt::get(I32, (int32_t)Oi)});
    Value *CtxU = EB.CreateCall(
        CtxCreate,
        {GraphArg, ConstantInt::get(I32, SGPL_TRAVERSE_OWNER_U),
         ConstantInt::get(I32, Domain), Membership,
         WantEnvelope ? Env.SeenArg : Null,
         WantEnvelope ? Env.NextArg : Null, ConstantInt::get(I32, 0), HeadSlot,
         Null, ConstantInt::get(I64, 0), OpsU, ConstantInt::get(I32, 1)});
    Value *CtxV = EB.CreateCall(
        CtxCreate,
        {GraphArg, ConstantInt::get(I32, SGPL_TRAVERSE_OWNER_V),
         ConstantInt::get(I32, Domain), Membership,
         WantEnvelope ? Env.SeenArg : Null,
         WantEnvelope ? Env.NextArg : Null, ConstantInt::get(I32, 0), HeadSlot,
         Null, ConstantInt::get(I64, 0), OpsV, ConstantInt::get(I32, 1)});
    FunctionCallee Own = Mod->getOrInsertFunction(
        "autograph_exec_ctx_own_round",
        FunctionType::get(Type::getVoidTy(Ctx), {I8P, I32, I32}, false));
    EB.CreateCall(Own, {CtxOwner, ConstantInt::get(I32, 1),
                        ConstantInt::get(I32, 1)});

    FunctionCallee Fork = Mod->getOrInsertFunction(
        "autograph_frontier_fork_join",
        FunctionType::get(I32, {I8P, I8P, I8P, I8P}, false));
    Value *NewSize = EB.CreateCall(Fork, {GraphArg, CtxOwner, CtxU, CtxV});
    if (WantEnvelope)
        commitEnvelope(EB, Info, Mod, I8P, Ctx, GraphArg, NewSize);
    FunctionCallee Destroy = Mod->getOrInsertFunction(
        "autograph_exec_ctx_destroy",
        FunctionType::get(Type::getVoidTy(Ctx), {I8P}, false));
    EB.CreateCall(Destroy, {CtxOwner});
    EB.CreateCall(Destroy, {CtxU});
    EB.CreateCall(Destroy, {CtxV});

    deactivateDriver(Info);
    return true;
}

/* Recursive interpreter entry (R7): derive the realization from the effect
 * expression's structural facts, never from a strategy name. */
static bool emitExprInterp(NeighborLoopInfo &Info)
{
    if ((Info.HasDriverClaim || !Info.DriverUClaims.empty()) &&
        perSourceClaims(Info).empty())
        return false; /* unsupported claim staging */
    ExprFacts F;
    deriveExprFacts(Info, F);
    if (F.IsDual)
        return emitDualForkJoin(Info);
    return emitSingleStage(Info, F.IsRed, F.IsSourceRed, F.IsPriv, F.IsV);
}

/* ── pass ──────────────────────────────────────────────────────── */

/* Legacy shape blocklist.  Kept only for A/B testing
 * (SGPL_FRONTIER_BLOCKLIST_GUARD=1); the default decision is the totality
 * proof below.  History: inline graph queries (hasEdge emits its own scan
 * subloop) made the rewrite emit malformed IR that crashed a later LLVM pass;
 * a floating-point header PHI (per-source reduction register) was cloned with
 * the register substituted by a pointer (`fadd ptr, double`) and the driver
 * deactivated, silently producing 0. */
static bool legacyBlocklistRefuses(Loop *L)
{
    if (!L->getSubLoops().empty())
        return true;
    for (const BasicBlock *BB : L->blocks())
        for (const Instruction &I : *BB)
            if (const auto *PN = dyn_cast<PHINode>(&I))
                if (!PN->getType()->isIntegerTy())
                    return true;
    for (BasicBlock *BB : L->blocks())
        for (Instruction &I : *BB)
        {
            auto *CI = dyn_cast<CallInst>(&I);
            if (!CI)
                continue;
            Function *CF = CI->getCalledFunction();
            if (!CF)
                return true; // indirect call: not modelable
            StringRef N = CF->getName();
            if (N == "autograph_neighbor_iter_init" || N == "autograph_neighbor_iter_next" ||
                N == "roaring_bitmap_add" || N == "roaring_bitmap_remove" ||
                N == "autograph_profile_region_enter" || N == "autograph_profile_region_exit" ||
                N == "autograph_profile_record_kernel_ns" || N == "sgpl_now_ns" ||
                N == "graph_get_edge_weight")
                continue;
            if (CF->isIntrinsic())
                continue;
            return true;
        }
    return false;
}

/* Proof-based refusal for the CleanCut/owner-step rewrite.
 *
 * The rewrite models a neighbour-loop body as pair-local array effects with
 * U/V/D provenance, first-wins claims and set appends.  A body element outside
 * that model has no defined per-pair meaning, so the rewrite may invent
 * semantics (both historical miscompiles above were coverage violations).
 *
 * This is the totality form of the check: every instruction is either part of
 * the model or the loop is refused, so a *new* unmodelled shape is refused by
 * construction instead of being rewritten blind.  The blocklist was the same
 * test applied only to shapes already seen to fail.
 *
 * Refusal reasons are printed under GRAPH_FRONTIER_STATS=1. */
static bool provesModelable(Loop *L, std::string &reason)
{
    /* 1. One loop body only.  Nested control flow has no per-pair lowering. */
    if (!L->getSubLoops().empty())
    {
        reason = "contains subloops";
        return false;
    }

    for (const BasicBlock *BB : L->blocks())
        for (const Instruction &I : *BB)
        {
            /* 2. Scalar loop-carried PHIs are reduction registers.  The engine
             * work function is called once per edge with no per-source state,
             * so a reduction cannot be honoured.  Refuse non-integer PHIs, and
             * integer PHIs whose value escapes the loop (a scalar reduction
             * stored after it, e.g. `int c = 0; for each neighbor { c += 1; }
             * deg[u] = c;`).  Loop-internal integer bookkeeping stays allowed. */
            if (const auto *PN = dyn_cast<PHINode>(&I))
            {
                if (!PN->getType()->isIntegerTy())
                {
                    reason = "loop-carried non-integer PHI (reduction register)";
                    return false;
                }
                for (const User *U : PN->users())
                {
                    const auto *UI = dyn_cast<Instruction>(U);
                    if (UI && !L->contains(UI->getParent()))
                    {
                        reason = "loop-carried PHI escapes the loop (scalar reduction)";
                        return false;
                    }
                }
                continue;
            }

            /* 3. Calls must be runtime helpers whose effects the rewrite
             * models.  Anything indirect or unrecognised is opaque: it could
             * touch any state the engine partitions, so refuse. */
            const auto *CI = dyn_cast<CallInst>(&I);
            if (!CI)
                continue;
            const Function *CF = CI->getCalledFunction();
            if (!CF)
            {
                reason = "indirect call";
                return false;
            }
            if (CF->isIntrinsic())
                continue;
            const StringRef N = CF->getName();
            if (N == "autograph_neighbor_iter_init" || N == "autograph_neighbor_iter_next" ||
                N == "roaring_bitmap_add" || N == "roaring_bitmap_remove" ||
                N == "autograph_profile_region_enter" || N == "autograph_profile_region_exit" ||
                N == "autograph_profile_record_kernel_ns" || N == "sgpl_now_ns" ||
                N == "graph_get_edge_weight")
                continue;
            reason = "unmodelled call: " + N.str();
            return false;
        }
    return true;
}

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
        /* R7: no classification.  privLayout + supportedByAlgebra is the
         * semantic admissibility gate; the recursive interpreter derives the
         * realization structurally from the effect expression. */
        const bool PrivProvable = IsIter && privLayout(Info);
        const bool Priv = PrivProvable && privLayoutNeeded(Info);
        if (Priv)
            Info.UsePrivLayout = true;
        EffectSummary S;
        if (IsIter)
            summarizeEffects(Info, S);
        std::string SemReason;
        const bool Supported =
            IsIter && supportedByAlgebra(Info, S, Priv, SemReason);
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
                   << " shadow=" << Info.RoundSepBases.size()
                   << "  ";
            if (IsIter)
            {
                printEffects(Info);
                if (getenv("GRAPH_FRONTIER_VERBOSE"))
                    printEffectExpr(Info);
            }
            if (IsIter && !Supported)
                errs() << " [refused: " << SemReason << "]";
            errs() << "\n";
        }
        if (IsIter && getenv("SGPL_WITNESS_DUMP"))
            printWitness(Info, /*IsIter=*/true);
        else if (!IsIter && getenv("SGPL_WITNESS_DUMP"))
            errs() << "[witness] iter=0 inner=" << L->getHeader()->getName()
                   << "\n";
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
             * per-pair structure to rewrite.  Released to the PDG under the
             * second-chance switch, conservatively sequential otherwise. */
            if (!IsIter)
            {
                if (!getenv("SGPL_PDG_SECOND_CHANCE"))
                    markSequential(L);
                ++detected;
                continue;
            }
            /* Priority 1: the effect-algebra totality proof.  Priority 2: a
             * refused loop is released to the PDG (which must issue its own
             * fail-closed certificate) instead of being forced sequential,
             * under SGPL_PDG_SECOND_CHANCE=1. */
            std::string refuseReason;
            const bool Modelable = getenv("SGPL_FRONTIER_BLOCKLIST_GUARD")
                                       ? !legacyBlocklistRefuses(L)
                                       : provesModelable(L, refuseReason);
            if (getenv("GRAPH_FRONTIER_STATS"))
                errs() << "[graph-frontier]   modelable=" << (Modelable ? 1 : 0)
                       << (Modelable ? "" : (" reason=" + refuseReason)) << "\n";
            bool Rewritable = Supported && Modelable;
            if (Rewritable)
            {
                /* The recursive structural interpreter is the sole
                 * emitter.  A refused expression fails closed to the
                 * sequential marker. */
                bool Emitted = emitExprInterp(Info);
                if (!Emitted && getenv("GRAPH_FRONTIER_STATS"))
                    errs() << "[graph-frontier]   expression path refused"
                              " -> stays sequential\n";
                /* A refused emit falls through to the terminal sequential
                 * marker below; say so, because otherwise the loop looks like it
                 * was classified sequential when the classifier actually
                 * claimed it parallel. */
                if (!Emitted && getenv("GRAPH_FRONTIER_STATS"))
                    errs() << "[graph-frontier]   emit failed -> stays sequential\n";
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
                    if (getenv("SGPL_EXEC_DUMP"))
                        dumpExecAtoms(Info);
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
                    /* Postcondition: the rewritten function AND the emitted
                     * helper functions must be valid IR.  The emitted pair
                     * work functions live outside F, so a module-level check
                     * is required to catch malformed helper IR (an invalid
                     * shadow GEP once crashed EarlyCSE in the codegen
                     * pipeline while `verifyFunction(F)` still said OK).
                     * SGPL_FRONTIER_STRICT=1 makes an invalid emission abort so
                     * a test run cannot miss it. */
                    std::string Err;
                    raw_string_ostream SS(Err);
                    if (verifyModule(*F.getParent(), &SS))
                    {
                        errs() << "[graph-frontier] POSTCONDITION FAILURE on "
                               << F.getName() << ": " << SS.str() << "\n";
                        if (getenv("SGPL_FRONTIER_STRICT"))
                            std::abort();
                    }
                    else if (getenv("GRAPH_FRONTIER_VERIFY"))
                    {
                        errs() << "[graph-frontier] verify OK on "
                               << F.getName() << "\n";
                    }
                    continue;
                }
            }
            /* Not rewritten: release to the PDG (its certificates are
             * fail-closed), or keep the conservative marker as the terminal
             * refusal.  Default is the safe terminal: traversal state is
             * invisible to the dependence analysis until call effects are
             * modelled, so releasing by default would be unsound. */
            if (!getenv("SGPL_PDG_SECOND_CHANCE"))
                markSequential(L);
            errs().flush();
        }
        ++detected;
    }
    if (getenv("GRAPH_FRONTIER_STATS") && getenv("GRAPH_FRONTIER_VERBOSE"))
        errs() << "[graph-frontier] detected=" << detected << "\n";
    return PreservedAnalyses::none();
}
