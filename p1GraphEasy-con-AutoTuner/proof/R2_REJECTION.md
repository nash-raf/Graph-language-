# R2 — the per-source claim rejection condition

> Oracle: `graph_frontier_lowering.cpp` (and the executor in
> `autotuner_runtime.c`). Line numbers refer to that file unless another file is
> named. Where surrounding documents describe this condition differently, this
> document follows the code.

---

## 1. Theory: why the condition exists

### 1.1 The claim primitive

A first-wins claim is an ordered conditional write:

```
C(A, r, γ, δ, ≺)
```

- `A` — the base location being claimed (an array/slot, not an arbitrary value),
- `r` — the provenance region of the index (`U` for source-indexed),
- `γ` — the guard: the expected current value,
- `δ` — the transition: the value written when the guard holds,
- `≺` — the serial priority under which the *first* successful claim wins.

The record is the `Effect` struct (`:234-252`):

```cpp
struct Effect
{
    EffectKind Kind = EffectKind::R;
    ...
    OccurrenceScope Scope = OccurrenceScope::PerPair;
    ...
    /* Ordered claim primitive C(A,r,γ,δ,≺): γ (guard) is the expected value the
     * branch compares against the location; δ (transition) is the stored
     * desired value.  ≺ is the serial priority relation (the CSR scan order
     * the owner-computes runtime discharges); it is an obligation, not a
     * global algebraic law. */
    Value *ClaimGuard = nullptr;
    Value *ClaimTransition = nullptr;
    ...
};
```

Semantically, in the original serial program, the claim is
`if (A[u] == γ) A[u] = δ;` executed **once per source `u`** in the driver
preamble (e.g. k-core's `alive[u] = 1 → alive[u] = 0`). Winning means "this
source claimed itself first"; the pair work that follows belongs to the claim.

### 1.2 Occurrence scopes and phase segments

Every effect carries a κ ∈ {`Once`, `PerSource`, `PerPair`} (`OccurrenceScope`,
`:205-210`; struct default `PerPair` at `:241`). κ is assigned where the effect
is discovered:

- driver-preamble walk stamps `Scope = OccurrenceScope::PerSource` on store-form
  claims (`:1584`) and CAS-form claims (`:1612`);
- everything discovered in the neighbour body keeps the struct default
  `PerPair`.

`buildEffectExpr` (`:1879-1925`) then partitions the effect set into the
expression's three phase segments by κ alone:

```cpp
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
```

with the tree shape

```
E_t = SeqDomain_{u∈F_t}( P(u) ; SeqDomain_{v∈N(u)} B(u,v) ; Q(u) )
```

`P` = Preamble segment (per-source), `B` = Pair segment (per-pair),
`Q` = Epilogue segment (per-source finish). `collectTreePrims` flattens the
tree back to `(Effect*, PhaseSegment)` pairs (`:1871-1877`).

So in this system:

- **PerSource claim** = a `Claim` primitive whose segment is `Preamble` (or
  `Epilogue`) — it fires once per source `u` over the round domain
  `u ∈ F_t`;
- **PerPair claim** = a `Claim` primitive in the `Pair` segment — it fires once
  per walked edge `(u,v)`.

### 1.3 The hazard

The CleanCut rewrite does not execute `P(u)`. It clones the **pair body**
(`B(u,v)`) into a work function of the `sgpl_frontier_pair_fn` ABI —
`(int32_t source, int32_t destination, int64_t destination_index, void *env)`
(`autotuner_runtime.h:296-297`, described as a manual clone at
`graph_frontier_lowering.cpp:24`) — and the runtime invokes it **once per
pair**. If the preamble claim were cloned into that body (or left as an
ordinary store in the old nest), then:

- serial: one claim per source `u`;
- rewritten: one claim per pair `(u,v)` — with `deg(u)` pairs, the guarded
  transition is attempted `deg(u)` times, the `S`-outcome the rest of the round
  might read becomes pair-dependent, and priority `≺` (which relied on sources
  being visited in serial order once each) is no longer discharged by any
  schedule.

That is the occurrence-preservation failure R2 refuses: **κ = PerSource must
not be silently realized as κ = PerPair**. There is no theorem in the code that
equates the two, so the only sound outcomes are (a) refuse, or (b) re-establish
once-per-source by staging.

### 1.4 What staging establishes instead

If the claim is *stagingable* (§4), R2 does not fire and the rewrite
re-establishes the once-per-source discipline explicitly:

1. the claim runs in an emitted `SOURCE_BEGIN` callback — one call per source,
   driven by the executor's source lifecycle (`sgpl_exec_source_begin_ops`,
   `autotuner_runtime.c:3084-3091`; dispatch at `:3181`, `:3205`, `:3290`,
   which guarantee exactly one `source_begin(u)` per source, zero-pair sources
   included);
2. the outcome is published in a per-source state array `S[u][j]`;
3. the pair body is gated on `S[u][j]` (§6.3) — but only for claims whose guard
   dominates the neighbour loop.

Note what is **not** required: nothing in the code requires that pair effects
*depend on* the claim. The claim is not threaded through every pair primitive;
it is an entry gate on the whole pair function, applied only when dominance
says the claim actually wrapped the loop.

---

## 2. Formal statement

```
R2(E) ≡ HasPerSourceClaim(E) ∧ ¬Stagingable(E)
```

implemented as (`graph_frontier_lowering.cpp:4509-4513`):

```cpp
if (hasPerSourceClaim(Info) && perSourceClaims(Info).empty())
{
    reason = "per-source claim (occurrence preservation)";
    return false;
}
```

### 2.1 `HasPerSourceClaim` — existential over primitives

```cpp
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
```

(`:1989-2000`)

Formally: `∃ e ∈ flatten(E). e.Kind = Claim ∧ e.Seg ≠ Pair`.

Keyed on the **tree segment**, not on `Effect.Scope` — but the two agree by
construction, because `buildEffectExpr` puts exactly the `Scope == PerSource`
effects into `Preamble`/`Epilogue` (`:1893-1904`). The distinction matters only
for the staging predicate below, which keys on `Scope` directly.

**This predicate says nothing about pair effects.** It is a pure existential
over the primitives of the expression. There is no universal quantification of
the form "every pair effect requires the claim" anywhere in the file.

### 2.2 `Stagingable` — universal, fail-closed

```cpp
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
```

(`:2002-2023`)

Formally, for every effect `e` with `e.Kind = Claim ∧ e.Scope ≠ PerPair`:

| # | obligation | code |
|---|---|---|
| 1 | base exists | `E.Base != null` |
| 2 | guard is an LLVM constant (γ) | `isa<Constant>(E.ClaimGuard)` |
| 3 | transition is an LLVM constant (δ) | `isa<Constant>(E.ClaimTransition)` |
| 4 | base is module-reachable (not a stack slot) | `!isa<AllocaInst>(E.Base)` |
| 5 | guard is integer-typed | `E.ClaimGuard->getType()->isIntegerTy()` |

Any single violation on any single claim **clears the entire list** (`Found.clear()`)
→ `perSourceClaims(...).empty()` → R2 fires. Multiple stagingable claims are
fine: the list grows (`SmallVector<..., 2>` with growth), and the emitters index
it as `K = Claims.size()` (`:3317`).

The reason each obligation exists is mechanical — §6.1 shows the emitted
callback re-deriving the claim as `GEP(Base, u)`, `load`, `ICmpEQ(vs γ)`,
`store δ`; obligations 1–5 are exactly the preconditions for that re-derivation
to typecheck and address the right cell.

### 2.3 Where R2 sits in the gate

`GraphFrontierLoweringPass::run` (`:5325`):

```cpp
const bool Supported =
    IsIter && supportedByAlgebra(Info, S, Priv, SemReason);
...
bool Rewritable = Supported && Modelable;
if (Rewritable)
{
    bool Emitted = emitExprInterp(Info);
    ...
}
...
if (!getenv("SGPL_PDG_SECOND_CHANCE"))
    markSequential(L);
```

(`:5359-5360`, `:5424-5430`, `:5500-5501`)

R2 is refusal **#2** inside `supportedByAlgebra` (`:4499`), ordered after the
data-write refusal and before the `MutTop` refusal:

```cpp
static bool supportedByAlgebra(NeighborLoopInfo &Info, const EffectSummary &S,
                               bool Priv, std::string &reason)
{
    ...
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
    if (S.MutTop) ...
```

So the full rejection path is: `Supported = false` → `Rewritable = false` →
`emitExprInterp` never runs → fall-through to `markSequential(L)` (or, under
`SGPL_PDG_SECOND_CHANCE=1`, release to the PDG). The loop is never handed to
the racy DOALL path either way.

There is **no environment gate** on this condition: a repository-wide search
for `SGPL_COMP_F_ALLOW_DRIVER_CLAIM` finds it only in
`proof/EFFECT_ALGEBRA_DESIGN.md` (`:288`, `:342`), never in any `.c`/`.cpp`
file. R2 is unconditional in the current code.

---

## 3. What counts as a claim (recognition)

Recognition happens where effects are discovered. Two shapes produce
`Kind == EffectKind::Claim`.

### 3.1 Store form — `detectFirstWinsClaim` (`:801-834`)

```cpp
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
```

All of the following must hold for a `StoreInst` to be a claim:

| # | structural condition |
|---|---|
| 1 | the store's basic block has a **single predecessor** |
| 2 | that predecessor's terminator is a **conditional branch** |
| 3 | the store block is the branch's **true successor** (`getSuccessor(0) == BT`) |
| 4 | the branch condition is an `ICmpInst` with predicate **`ICMP_EQ`** |
| 5 | one icmp operand is a `LoadInst` from the **same array slot** as the store pointer (`sameArraySlot`) — this operand's partner is `γ` |
| 6 | the other operand (`γ`) is **not identical to** the stored value (`δ`) |

IR shape: `if (load(P) == γ) store(P, δ);` with `γ != δ`. Detected inside the
`classifyStore` lambda (`:1258-1273`, claim branch `:1266-1273`), called from
the neighbour-body walk (`:1343`) and from the driver-preamble walk
(`:1582`). A store-form claim also sets `Info.HasFirstWins` and
`Info.HasRecognizedOp` (`:1270-1271`); a preamble-discovered claim additionally
sets `Info.HasDriverClaim` (`:1586`, declared `:420`).

Note obligation: `P` is matched by *slot identity*, not pointer identity, so
the `load` and the `store` need not be CSE'd into one SSA value — the front end
does not CSE.

### 3.2 CAS form — `AtomicCmpXchgInst` (`:1589-1625`)

```cpp
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
        ...
```

Conditions: metadata `"sgpl.first_wins.claim"` (emitted by the front end,
`IRGenVisitor.cpp:3085`), pointer is a `GetElementPtrInst`, its index
originates from region `U` (via either provenance instance). γ = compare
operand, δ = new value. Recognized **only in the driver-preamble walk** (this
block is inside it), so a CAS claim is inherently `Scope = PerSource`
(`:1612`). It also sets `Info.HasFirstWins` and appends to
`Info.DriverUClaims` (`:1616-1617`, declared `:424`) — `DriverUClaims` is the
flag the interpreter entry at `:5197` uses to detect staging need.

### 3.3 Stamping of κ

| discovery site | call site | Scope |
|---|---|---|
| neighbour-body `classifyStore` | `:1343` | `PerPair` (struct default, `:241`) |
| driver-preamble `classifyStore` | `:1582`, stamped `:1584` | `PerSource` |
| driver-preamble CAS | block `:1589-1625`, stamped `:1612` | `PerSource` |

Pair-phase claims are also real and legal: they are recognized in the body and
are *not* subject to R2 (segment `Pair`, `Scope = PerPair`). They are recorded
separately via `hasPairPhaseClaim` (`:4673`) and declared to the runtime as a
`ResClaim` access contract on the pair op (`:4921-4922`).

---

## 4. The two predicates side by side

| | `hasPerSourceClaim` | `perSourceClaims` |
|---|---|---|
| keys on | expression tree `PhaseSegment` | `Effect.Scope` |
| quantifier | **∃** (any one claim trips it) | **∀** (all claims must qualify) |
| effect filter | `Kind == Claim && Seg != Pair` | `Kind == Claim && Scope != PerPair` |
| question | "is there a claim that must run once per source?" | "can every such claim be staged?" |
| on failure | (n/a — it's the trigger) | clears all, returns empty |
| line | `:1992-2000` | `:2006-2023` |

They agree because `buildEffectExpr` (`:1893-1904`) partitions by exactly the
same predicate (`Scope == PerSource` → non-`Pair` segment). Two mechanisms, one
invariant, enforced at `:1922`:

```cpp
assert(Flat.size() == Info.Effects.size() &&
       "flatten(E) must cover every primitive exactly once");
```

---

## 5. All sites that consume these predicates

R2 proper is only `:4509`, but the same two predicates gate other decisions —
knowing them is required to reason about what a "refusal" costs:

| site | condition | effect |
|---|---|---|
| `supportedByAlgebra` `:4509` | `hasPerSourceClaim ∧ ¬Stagingable` | **R2**: `Supported=false` → `markSequential` (`:5500`) |
| `emitExprInterp` `:5197-5199` | `(HasDriverClaim ∨ DriverUClaims≠∅) ∧ ¬Stagingable` | last line of defense at the interpreter entry: `return false` ("unsupported claim staging") before either emitter runs |
| `emitDualForkJoin` `:5003` | same expression, checked again | refuses the dual fork/join emitter **before** any IR is built — fail-closed re-check at the emitter boundary (`:4999-5002`: "Only unsupported claim staging refuses") |
| source-reduction gate `:4578` | `hasPerSourceClaim` | a per-source claim excludes the `SourceReduction` path (its own occurrence check requires zero remaining preamble mutations) |
| `privLayout` `:2372` | `HasFirstWins ∨ hasPerSourceClaim` | claims refuse privatization outright: `PrivReason = "claim in the loop nest"` (`:2374`) — the accumulation theorem's premise 1 is "no claim … in the expression" |
| `ownedLocalizedArray` `:2583` | `HasFirstWins ∨ hasPerSourceClaim ∨ !RoundSepBases.empty()` | claim → the "already race-free, skip the private copies" shortcut is not taken |

Note the double duty of `hasPerSourceClaim` in `privLayout`: it refuses
privatization even for **stagingable** claims. So a stagingable per-source claim
cannot ride the privatization path — it must be realized by the staged
claim emission (§6), and any combination with `ReducePtr`/privatized arrays
falls back through the other refusals.

---

## 6. Flow when R2 does **not** fire (the staging path)

This is included because it is what R2's `Stagingable` half exists to make
possible; the obligations in §2.2 are precisely its preconditions.

### 6.1 `buildClaimState` — allocate `S`, emit one callback per claim (`:3300-3364`)

```cpp
/* Per-source claim staging (R7): allocate the per-source state array
 * S[u][j] (K claims), zero it, publish its base through the module global the
 * pair callbacks read, and emit one SOURCE_BEGIN callback per claim.  Each
 * callback performs its guarded transition on the live base and records the
 * outcome in S[u][j].  Exactly-once per source is discharged by the executor's
 * source lifecycle (OWNER_V coverage pre-pass / OWNER_U inline source change),
 * so the callbacks need no synchronization. */
```

Emitted callback body, per claim `j`:

```cpp
Value *EP = CBI.CreateGEP(ClaimET, BaseT, U);          // Base[u]
Value *Old = CBI.CreateLoad(ClaimET, EP);
Value *Won = CBI.CreateICmpEQ(Old, γ);                  // γ is Constant (obligation 2)
CBI.CreateCondBr(Won, Then, Cont);
TBI.CreateStore(δ, EP);                                 // δ is Constant (obligation 3)
S[u][j] = zext(Won, i32);
```

Where `ClaimET = Claim->ClaimGuard->getType()` (`:3334`) — obligation 5 (integer
γ) is what makes this `CreateGEP` well-typed against the base, and obligation 4
(`!isa<AllocaInst>`) is what makes `ptrFromSlot`/module-global publication
meaningful. The array is zeroed up front (`:3320-3322`), so "not yet claimed"
reads as 0.

The **CAS form is re-realized here as a plain, non-atomic** load/eq/store
(`:3345-3352`). This is sound only because the executor guarantees exactly one
`source_begin(u)` per source — no two workers execute the same claim cell — so
no CAS is needed at runtime.

### 6.2 Operation groups (`:4885-4901`)

Each callback becomes one op:

```cpp
for (Function *CB : ClaimCBs)
    ClaimOps.push_back(MakeOp(SGPL_OP_SOURCE_BEGIN, Null, nullptr, nullptr,
                              CB, nullptr, nullptr, {ResClaim}));
```

with contract `ResClaim = {SGPL_RES_CLAIM, SGPL_ACCESS_ATOMIC_WRITE}`
(`:4887-4888`). Claim ops are stored into the ops array **first** (`:4932-4934`),
before the preamble op, snapshots, and the fused pair op (`:4926-4944`) — the
comment at `:4885` states the order: "per-source claim/preamble ops, the
round's Snapshot ops, then the fused pair op".

### 6.3 Pair-body gate (`:4015-4050`)

```cpp
/* Per-source first-wins claims: each claim runs once per source in a
 * SOURCE_BEGIN operation and publishes its outcome in the per-source
 * state array S[u][j].  Only claims whose guard dominates the loop gate
 * the pair body (a preamble claim that does not wrap the loop performs
 * its transition without gating the pairs). */
```

- `claimGatesLoop` (`:2025-2036`): `DominatorTree.dominates(claim block,
  neighbor-loop header)` — computed fresh over `F`.
- For each gating claim: index `S[u*K + j]`, load, `!= 0`.
- `Won = AND of all gating results`; entry block ends with
  `CreateCondBr(Won, FirstClone, RetStub)` (`:4046`) — the pair body executes
  only if every gating claim won; otherwise the pair returns immediately.
- If no claim gates (none dominates), the entry branches straight to
  `FirstClone` (`:4049`) — the claim still ran its transition once per source,
  it just doesn't gate.

### 6.4 Executor side

`autotuner_runtime.c`:
- `sgpl_exec_source_begin_ops` (`:3084-3091`) dispatches every op carrying
  `SGPL_OP_SOURCE_BEGIN`;
- validation: an op with the capability but no callback aborts (`:3000`);
- the traversal drives it exactly once per source: OWNER_U at source changes
  (`:3181`, `:3205`), OWNER_V coverage pre-pass including zero-pair sources
  (`:3290`, comment `:3223`);
- under dual ownership, claim ops live in the fork/join **owner**, whose
  source-domain coverage runs before either child — `autotuner_runtime.c:3425`
  ("Owner source lifecycle: per-source operations staged in the owner (e.g. …)",
  dispatch at `:3429-3430`) — so `claim(u) ≺ pairs(u)` holds across both
  domains (emitter comment `:4999-5002`, re-check `:5003`).

---

## 7. Supported store form — reference tables

### (a) Recognition: what becomes `Kind == Claim` at all

| property | store form (`detectFirstWinsClaim`, `:801`) | CAS form (`:1589`) |
|---|---|---|
| instruction | `StoreInst` | `AtomicCmpXchgInst` |
| trigger | structural (branch + eq-icmp + same-slot load) | metadata `"sgpl.first_wins.claim"` |
| pointer | any slot (`Ptr`, matched via `sameArraySlot`) | must be `GetElementPtrInst` |
| index region | whatever provenance gives (body or preamble) | must be `U` (via `Prov` or `ProvU`) |
| γ (guard) | the non-load icmp operand | `getCompareOperand()` |
| δ (transition) | the stored value | `getNewValOperand()` |
| where found | neighbour body **and** driver preamble | driver preamble only |
| Scope stamped | default `PerPair` in body; `PerSource` at `:1584` | `PerSource` at `:1612` |

### (b) Staging: what `perSourceClaims` accepts (`:2013-2019`)

Applies to every claim with `Scope != PerPair` (both forms, since CAS claims
are preamble-stamped `PerSource`):

1. `E.Base != null`
2. `isa<Constant>(E.ClaimGuard)` — γ constant
3. `isa<Constant>(E.ClaimTransition)` — δ constant
4. `!isa<AllocaInst>(E.Base)` — module-reachable (global or pointer slot),
   i.e. `Base` must be an address the emitted callback can reach through a
   module global, not a frame-local alloca
5. `E.ClaimGuard->getType()->isIntegerTy()` — integer γ

Failure of any one on any one claim → entire list cleared → R2.

### (c) Emission: the only form actually executed (`:3342-3359`)

```cpp
Value *BasePtr  = ptrFromSlot(CBI, const_cast<Value *>(Claim->Base), I8P);
Value *BaseT    = CBI.CreateBitCast(BasePtr, PointerType::get(ClaimET, 0));
Value *EP       = CBI.CreateGEP(ClaimET, BaseT, U);   // element = γ's type
Value *Old      = CBI.CreateLoad(ClaimET, EP);
Value *Won      = CBI.CreateICmpEQ(Old, γ);
... if (Won) store δ ...
S[u][j]         = zext(Won, i32)
```

i.e. the executed claim is always **`Base` u-indexed array, integer γ/δ,
non-atomic guarded store, outcome recorded per source** — regardless of whether
the source pattern was a branch-guarded store or a CAS. The re-derivation is
the reason obligations 2–5 of (b) are exactly what they are.

---

## 8. Naming

- The condition is not an identifier `R2` anywhere in the code. It is refusal
  **#2** of `supportedByAlgebra` (`:4509`), the reason string
  `"per-source claim (occurrence preservation)"`.
- `proof/EFFECT_ALGEBRA_DESIGN.md` lists it as rule 2 of `interpretPar`
  (`:288-289`) — but that listing's environment gate
  (`SGPL_COMP_F_ALLOW_DRIVER_CLAIM`) exists only in the markdown; there is no
  such `getenv` in any source file, so the code has a single, unconditional R2.
- `test/run_exec_r2_tests.sh` ends with `echo "composable exec R2 tests: PASS"`
  (`test/run_exec_r2_tests.sh:40`) and runs only the executor composition tests
  (`exec_source_cover_test.c`, `exec_forkjoin_test.c`, `exec_snapshot_test.c`);
  it contains no claim test, so its "R2" is unrelated to this condition.
- Related refusals that share `hasPerSourceClaim`/`perSourceClaims`
  (interpreter entry, source-reduction gate, `privLayout`,
  `ownedLocalizedArray`, the `emitDualForkJoin` re-check) are listed in §5;
  only `:4509` is R2 proper.

---

## 9. One-line summary

**R2 rejects a loop iff the effect expression contains at least one claim whose
occurrence scope is per-source (`PhaseSegment != Pair`) *and* at least one such
claim cannot be staged — where "can be staged" means: module-reachable
(non-alloca) base, constant integer guard γ and constant transition δ — because
a per-source claim cloned into a pair-called work function would fire once per
pair instead of once per source; when staging is possible, the claim runs as a
`SOURCE_BEGIN` operation with exactly-once discharged by the executor's source
lifecycle, publishes into `S[u][j]`, and gates the pair function entry (only
for claims whose guard dominates the neighbour loop).**
