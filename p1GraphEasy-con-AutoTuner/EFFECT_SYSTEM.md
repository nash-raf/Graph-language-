# Effect System Deep Dive — Region Lattice, Provenance, Effects, Algebraic Classification, Lowering, Runtime

> **Revision note (v7 refactor).** Line references were refreshed for the
> structural-expression refactor: the region/provenance/effect/expression layer,
> the temporal derivation, the interpretation, the definition index, and the
> pass-run flow cite the current revision.  Sub-references inside the emission
> subsections (§6.2, §6.6) are anchored by function name (see the definition
> index for authoritative ranges); the algebra itself is specified in
> `proof/EFFECT_ALGEBRA_DESIGN.md`.

This document describes the effect system of the GraphFrontierLoweringPass ("CleanCut") in
`graph_frontier_lowering.cpp`, from the region lattice at the bottom of the stack up through
provenance analysis, the effect expression, the algebraic interpretation that selects the
parallelization strategy, IR lowering, and the runtime that actually executes the parallel
step. Every claim is referenced by `file:line` (post-Phase-3 cutover revision).

Companion documents:

- `proof/EFFECT_ALGEBRA_DESIGN.md` — the algebra itself (primitive domain, `;`/`‖`/`*`,
  refinement order, exchange law, soundness obligations outside the algebra).
- `CLEANCUT_LOOP_SPEC.md` — the user-facing spec (loop shapes, techniques, coverage matrix).
- `CC_AUTOTUNER_EDGEMAP_REGION.md` — how the AutoTuner region pass sees CleanCut step calls.
- `AUTOTUNER_INTERACTION_STUDY.md` — CleanCut vs AutoTuner/ARS interaction history.
- `proof/CLEANCUT_PARALLEL_PROOF.md` — end-to-end scaling proof for pagerank.
- `proof/algebra_golden_classify.txt` + `validate_algebra.sh` — locked classification
  verdicts and their regression harness.

---

## 1. Overview: where the system sits

The pass is wired in `main.cpp:671-689` (`runPdgAndOutliner`): a fresh `FunctionPassManager`
containing `GraphFrontierLoweringPass` runs **before** the AutoTuner region-annotation pass,
the PDG, and the loop outliner. The stated contract (`main.cpp:666-670`): any loop that used
graph iterators is lowered to an owner-computes frontier step *before* those passes see it,
"so the racy DOALL path never fires on graph-iterator loops and the region pass never wraps
the rewritten round nest."

The complete stack, bottom-up:

```
IR (post-mem2reg SSA)
   │  detect: iterator shape / graph-data write (analyzeNeighborLoop, loopHasGraphDataWrite)
   ▼
Region lattice      origin of an index value: Bottom < U,V,D,G < Top
   │  seeds + transfer rules
   ▼
Provenance          forward SSA dataflow: value → Region
   │  Prov.regionOf(index) for every store/load subscript
   ▼
Primitive effects   {R, W, Uf, Uop, Claim, Activate}(Base, Reg, Op, Temp, Scope)
   │  segmented into E_t = preamble ; pair ; epilogue  (buildEffectExpr)
   ▼
Effect expression   τ derived from segments + round structure (relateReadWrite)
   │  folded into EffectSummary, interpreted by ⟦·⟧_par = interpretPar
   ▼
classify(E)         Klass: SourceOwner / DestOwner / Reduction / DualOwner /
                    Sequential / Privatized / SourceReduction
   │  provesModelable totality check
   ▼
Emission            pair work fn clone + build/step calls in driver preheader
   │  driver deactivated, round nest marked sequential
   ▼
Runtime             autograph_build_clean_cut + autograph_frontier_step_owner_*
                    (parallel_for_runtime, one worker per partition)
```

Classification is algebraic since the Phase-3 cutover: `classify()` (`:2431-2449`) is
`privLayout` + `summarizeEffects` + `interpretPar`, and the former procedural ladder and
pairwise temporal oracle were removed after the differential harness reported zero
disagreements (64 fixtures; see `proof/EFFECT_ALGEBRA_DESIGN.md` §9).

Environment gates used by the pass (`graph_frontier_lowering.cpp`):

| Variable | Effect | Location |
|---|---|---|
| `GRAPH_FRONTIER_REWRITE_OFF` | Disable the rewrite; graph loops are only marked sequential | `:4123` |
| `GRAPH_FRONTIER_STATS` | Print candidate/classification/effect-set traces | `:2440`, `:4137` |
| `GRAPH_FRONTIER_VERBOSE` | Also print the segmented expression form | `:4157` |
| `GRAPH_FRONTIER_DIAG` | Per-store diagnostic dumps during effect building | `:1192`, `:1427`, `:1473`, `:1533` |
| `GRAPH_FRONTIER_DUMP` | Dump post-emit function/module IR to `/tmp` | `:4221`, `:4231` |
| `GRAPH_FRONTIER_VERIFY` | Print verifier OK per rewritten function | `:4255` |
| `SGPL_FRONTIER_STRICT` | Abort on a verifier failure after emission | `:4252` |
| `SGPL_FRONTIER_BLOCKLIST_GUARD` | Use the legacy shape blocklist instead of the totality proof | `:4188` |
| `SGPL_PDG_SECOND_CHANCE` | Release refused loops to the PDG instead of forcing sequential | `:4178`, `:4268` |
| `SGPL_COMP_I_SOURCE_REDUCTION` | Enable the per-source reduction (gather) class | `:2401` |
| `SGPL_COMP_F_ALLOW_DRIVER_CLAIM` | Restore old behaviour for driver-preamble first-wins claims | `:2351` |
| `SGPL_NO_CLEANCUT_CACHE` | Disable CleanCut partition reuse cache (runtime) | `autotuner_runtime.c:3115` |
| `SGPL_CLEANCUT_PARTITIONS` | Override partition count (runtime) | `autotuner_runtime.c:2930` |
| `SGPL_CLEANCUT_TIMING` / `SGPL_CLEANCUT_DEBUG` | Runtime build/reuse timing and debug (runtime) | `autotuner_runtime.c:3121`, `:2921` |

---

## 2. Region lattice

### 2.1 Theory

The region lattice classifies the **origin of an index value** inside a graph-loop body. The
semantic model is the per-pair body of a neighbor loop: every invocation of the body
corresponds to exactly one walked edge `(u, v)`, where `u` is the driver axis (source) and
`v` is the walked neighbor (destination). Parallelism in this model is **ownership**: if
every write to an array is indexed by a single endpoint, the endpoint space can be
partitioned and each partition serializes its own writes (owner-computes, race-free by
construction). The region of an index therefore answers one question: *which endpoint (if
any) owns the location this subscript addresses?*

The lattice is defined at `graph_frontier_lowering.cpp:155-166`:

```
                    Top          mixed / unknown → conservatively sequential
                   / | \ \
                  U  V  D  G      distinct non-Bottom origins
                   \ | / /
                   Bottom         constant / loop-invariant / neutral
```

- **`Bottom`** — the index is a constant or loop-invariant value; it carries no ownership
  information (writing `arr[0]` every pair is a shared slot, treated as a reduction/global
  candidate, not an endpoint write).
- **`U`** — the index flows from the **driver induction variable** (the source axis `u`).
  Writes indexed by U are *source-owned*: partitioning the source space makes them race-free.
- **`V`** — the index flows from the **iterator v-slot** (the walked neighbor /
  destination). Writes indexed by V are *dest-owned*: partitioning the destination space
  makes them race-free.
- **`D`** — the index is load-derived from a **graph data-array element**
  (`nodes[i]`, `perm[v]`, `deg[v]`). The destination "house" is a data value, not a vertex
  id, so no vertex partition owns it — a D-indexed write forces sequential unless the whole
  loop is a privatizable pure accumulation.
- **`G`** — a **scalar / global reduction slot**: the store has no vertex index at all (a
  scalar accumulator). Neither endpoint owns it; it is handled by per-partition partials.
- **`Top`** — the index mixes two or more distinct origins (or is unknown). No single owner
  exists, so the loop is conservatively sequential.

The lattice is *almost flat*: the only subsumption is `Bottom ⊑ x`. Any two distinct
non-Bottom elements join to `Top`. This reflects the design decision that the analysis
never tries to decompose a mixed index — a mixed origin has no owner, and the system fails
closed.

### 2.2 What it does in the system

Every mutating effect carries a `Region` (`Effect::Reg`, `:202`), and every classification
rule is phrased over the multiset of regions of the effect set:

- `MutTop` (any mutating effect with `Reg == Top`) → `Sequential` (`:2353-2354`).
- A D-indexed write (`HasDataWrite`) → `Sequential` unless the whole effect set is provably
  privatizable (`:2349-2350`).
- `MutU && MutV` with disjoint bases → `DualOwner` (two phases, each single-owner)
  (`:2385-2422`).
- `MutV` only → `DestOwner`, `MutU` only → `SourceOwner` (`:2419-2422`).
- `MutG` with a single recognized operator → `Reduction` (`:2415-2418`).
- Round-separation bases are defined by their single write region vs cross-endpoint reads
  (`:1573-1680`).

The lattice is also the **codomain of the provenance analysis** (section 3): it is the
abstract domain, provenance is the abstract interpretation.

### 2.3 Implementation

```cpp
enum class Region : uint8_t { Bottom = 0, U, V, D, G, Top };   /* :155 */

static Region joinRegion(Region A, Region B)                    /* :157-166 */
{
    if (A == B)  return A;
    if (A == Region::Bottom) return B;
    if (B == Region::Bottom) return A;
    return Region::Top; /* two distinct non-bottom origins */
}
```

`Bottom = 0` lets maps default to `Bottom`; the enum is `uint8_t` because every effect
carries one. `regionName` (`:843-860`) is the diagnostic printer. The join is the only
lattice operation needed — there is no meet, because the analysis is a forward flow, not a
must/may intersection.

---

## 3. Provenance

### 3.1 Theory

Provenance is a **forward dataflow (abstract interpretation) over SSA** that assigns each
SSA value the `Region` of its origin. It is a fixed-point computation with:

- **Seeds** — the boundary of the loop's index world:
  - the driver induction phi and the u SSA value → `U` (the source axis);
  - the v-slot alloca (whose address is passed to `autograph_neighbor_iter_next`) → its
    loads are `V` (the destination axis).
- **Transfer rules** — pure shaping instructions propagate the join of their operands'
  origins; memory loads classify by *what kind of slot they read*.
- **Soundness** — anything not covered by the rules maps to `Top`; a `Top` origin in any
  store subscript forces the loop sequential (`:375-376`). The analysis may only be
  conservative, never optimistic.

A crucial precondition: **mem2reg has already run** before this pass (`main.cpp`), so index
chains are SSA except the escaping v-slot alloca (its address escapes to the iterator
function, so it survives as a memory slot). That single escaping slot is why `transfer` has
an explicit load rule rather than a purely SSA definition.

The `D` region is defined structurally, not by symbol names: a load whose pointer is a
`GetElementPtrInst` is a **graph data-array element** load (an array element has an element
GEP; a scalar slot does not). `isArrayElementLoad` (`:366-369`):

```cpp
static bool isArrayElementLoad(const LoadInst *LI)
{
    return LI && isa<GetElementPtrInst>(LI->getPointerOperand());
}
```

This is what excludes the iterator's scalar v-slot (`load i32, ptr %v_alloca` — no element
GEP) from D and catches hand-rolled `next_rank[nodes[i]]` shapes.

### 3.2 Implementation

`class Provenance` (`graph_frontier_lowering.cpp:377-466`):

```cpp
class Provenance
{
    DenseMap<Value *, Region> M;
    Value *VSlot;                   /* iterator v-slot alloca (loads → V) */
    Value *USlot;                   /* scalar u slot (foreach-set element) */
    SmallPtrSet<Value *, 4> USeeds; /* driver induction phi and u SSA (→ U) */
```

`lookup` (`:384-392`):

- in `USeeds` → `U`;
- `Constant` / `Argument` → `Bottom`;
- else the map, defaulting to `Bottom`.

`transfer` (`:394-418`):

- in `USeeds` → `U`;
- `LoadInst`: pointer `== VSlot` → `V`; pointer `== USlot` → `U`;
  `isArrayElementLoad` → `D`; any other load → `Top`;
- `PHINode` / `BinaryOperator` / `CastInst` / `GetElementPtrInst` / `SelectInst` →
  join of operand origins;
- anything else → `Top`.

The constructor (`:420-443`) seeds `USeeds` and iterates **every basic block, every
instruction** of the function until no map entry changes — a straightforward monotone
worklist-as-full-pass fixed point. `regionOf` (`:445`) is the read interface.

### 3.3 Where it is instantiated

Three instantiation sites:

1. **Per-loop, for the neighbor body** (`:1057-1058`):

   ```cpp
   Provenance Prov (*F, Info.VAlloca, {IndVar, USrc}, USlot);
   Provenance ProvU(*F, Info.VAlloca, {IndVar, USrc, Info.UVal}, USlot);
   ```

   `USrc` is the init call's u operand with casts peeled (`:1049-1055`); `Info.UVal` is the
   raw init operand. Two instances exist because of the **driver preamble**: preamble
   indexes are computed from the driver's own u (the iterator init arg or the frontier-set
   element), whose provenance may be `Top` (a set-iteration call result) in the peeled
   instance. `ProvU` additionally seeds `Info.UVal`, so a preamble index that derives from
   the raw u counts as `U` (`:1418-1424`). This is used throughout the preamble walk
   (`:1455-1456`, `:1489-1490`, `:1516-1517`, `:1547-1548`).

2. **Seedless, for iterator-less loops** — `loopHasGraphDataWrite` (`:1813-1828`):

   ```cpp
   Provenance Prov(*F, nullptr, {}); /* no v/u seeds: only data-array origins */
   ```

   With no U/V seeds, any store subscript whose origin is `D` proves the loop writes
   through a graph-data index. This catches hand-rolled loops (`while (i<n)
   next_rank[nodes[i]] += c`) that never use the neighbor iterator and would otherwise
   reach the racy DOALL path.

3. **As the region oracle inside effect building** — `Prov.regionOf(IX)` for every store
   subscript (`:1256`, `:1073`) and every U/V-indexed load (`:1369`), plus the round-sep
   cross-read classification (`:1674`).

The connection to the effect system is direct: **provenance fills `Effect.Reg`** — the
region lattice is the vocabulary, provenance is the measurement, and the effect set is the
record.

---

## 4. Effect system

### 4.1 The record

```cpp
enum class EffectKind : uint8_t          /* :177-185 */
{
    R = 0,        /* read */
    W,            /* plain write (no old value, no recognized operator) */
    Uf,           /* U_f: write depends on old value; owner-serializable */
    Uop,          /* U_⊕: recognized algebraic combine; privatizable */
    Claim,        /* first-wins */
    Activate      /* frontier append / dest-owned side effect */
};

enum class Temporal : uint8_t            /* :187-192 */
{
    Independent = 0, SameRoundRead, PreviousRoundRead, Carried
};

enum class OccurrenceScope : uint8_t     /* :201-206 (PerRound retired) */
{
    Once = 0, PerSource, PerPair
};

enum class ValueSource : uint8_t         /* :214-219 (reads only) */
{
    None = 0, Live, Snapshot
};

struct Effect                            /* :230-248 */
{
    EffectKind Kind = EffectKind::R;
    Region      Reg  = Region::Bottom;
    const Value *Base = nullptr;      /* canonical array identity */
    RedOp       Op   = RedOp::None;
    Temporal    Temp = Temporal::Independent;
    OccurrenceScope Scope = OccurrenceScope::PerPair; /* diagnostic (binder path) */
    ValueSource VSource = ValueSource::None;          /* σ */
    Value *ClaimGuard = nullptr;      /* C(A,r,γ,δ,≺): γ */
    Value *ClaimTransition = nullptr; /* δ */
    Value *Index = nullptr;
    Instruction *Origin = nullptr;
};
```

`RedOp` (`:79-97`) is the recognized associative-commutative operator family:
`Add, Sub, Mul, Min, Max, MinU, MaxU, FMinNum, FMaxNum, FMinProp, FMaxProp, And, Or, Xor,
FirstWins`. The flavors are deliberately *not* collapsed: the combine must
reproduce the body's operation exactly, so signed vs unsigned min/max and `minnum` vs
NaN-propagating `minimum` are kept apart — the identity elements differ too
(`UINT_MAX` vs `INT_MAX`).

Each operator is **algebraic data** (`RedOpInfo`, `:120-134`; lookup `redOpInfo`):
`f_body` (the body's update), `N_ω` (`RedNorm`: `Identity`/`Negated` — subtraction
contributes `-x` so the fold is an addition), `f_fold` (the combine of partition
partials), and the laws `Assoc`/`Comm`/`Idempotent`. `FoldSound(ω,T)` (`foldSound`,
`:2191-2213`) licenses the fold: integers always (exact modulo width), floats only for
the min/max flavours — float `+/*` re-associate and are refused. Exactness is a property
of the predicate, not an intrinsic operator field.

`Temporal` ranks as (`temporalRank`, `:862-875`): `Independent = 0 < SameRoundRead = 1 <
PreviousRoundRead = 2 < Carried = 3`. It records the strongest temporal relation between a
read and the writes to the same base, derived from the expression (section 4.6).

`OccurrenceScope` is the semantic grade of how often a primitive fires: `PerPair` for the
body, `PerSource` for the driver preamble/finish, `PerRound` reserved for round-level
effects. A rewrite must preserve the scope — a per-source claim may not become a per-pair
claim (`hasPerSourceClaim`, `:1789-1798`).

### 4.2 Canonical array identity

The front end does **not CSE**, so one source-level array appears as several distinct SSA
values: `load @cnt; gep; load elem` is emitted again for the store that follows, and again
for the index it uses. The effect system therefore works on *canonical identities*:

- `canonicalArrayBase(GEP)` (`:453-459`): when the base pointer is `load X` (the common
  `load %arr_slot` then index shape), key on the loaded-from pointer `X` so the read and
  the write of the same logical array map to the same identity; otherwise the underlying
  object.
- `sameAddressValue(A, B, Depth)` (`:472-504`): structural address equality, applied
  recursively — same value; two loads from the same address; two GEPs with the same base
  and equal indices; two casts of the same kind over equal operands. Anything else is not
  the same location.
- `sameIndexVal` (`:506-509`) and `sameArraySlot` (`:511-532`) wrap it for
  index/slot comparison (used by reduction detection and `storedDependsOnOldValue`).

### 4.3 Operator / pattern recognizers

These decide the *kind* of a store (they run inside `classifyStore`):

- `detectScalarRedOp(Stored, RedPtr)` (`:534-641`) — is the stored value a self-update
  through a known operator?
  - `BinaryOperator` with one operand a load of `RedPtr`: `add/fadd` → Add, `sub/fsub` →
    Sub, `mul/fmul` → Mul, `and/or/xor` → And/Or/Xor;
  - `IntrinsicInst` (`llvm.smin/umin/minnum/minimum/smax/umax/maxnum/maximum`) involving a
    load of `RedPtr` → the matching Min/Max flavor;
  - `select(icmp pred a, b, a, b)`: predicate decides the flavor (`slt/sle` signed min,
    `ult/ule` unsigned min, etc.). The **float form `select(fcmp olt a, b, a, b)` is
    deliberately NOT recognized** (`:599-609`): its value is not reorder-invariant once a
    NaN or signed zero is in the stream, so folding partition partials with it cannot
    reproduce the serial left-to-right fold; the raw select shape stays sequential
    (`llvm.minnum/maxnum`, which InstCombine usually produces for this shape, *are*
    recognized above).
- `detectConditionalMinMax(SI, RedPtr)` (`:643-714`) — min/max expressed as a **guarded
  store**: `if (X <cmp> *RedPtr) *RedPtr = X;`. The store's block must have a single
  conditional predecessor whose `icmp` compares the stored value against the current
  `*RedPtr`; predicate, branch direction and operand side together decide Min vs Max.
- `detectFirstWinsClaim(SI, Expected, Desired)` (`:716-749`) — the ordered conditional
  primitive: `if (A[i] == expected) A[i] = desired;` with `expected != desired`. Both guard
  operands are recorded on the effect. The claim is race-free under owner-computes because
  CSR source order decides the winner.
- `storedDependsOnOldValue(Stored, Ptr)` (`:761-786`) — generic `U_f` test: does the
  stored value's SSA use contain a load of the same slot as `Ptr` (bounded walk, PHIs not
  followed)?
- `valueDependsOnBase(V, Base)` (`:788-821`) — does `V` transitively read `Base`
  (used by `crossPhaseDataDep`)?

### 4.4 Loop shape detection (front gate of effect building)

`analyzeNeighborLoop` (`:986-1811`) runs per candidate loop. Detection prerequisites:

1. The loop header's terminator is a conditional branch whose condition is an `ICmp`
   with `autograph_neighbor_iter_next(&iter, &v)` as an operand (`:988-1010`).
2. The parent (driver) loop exists **and** has an SSA induction phi
   (`driverIndVar`, `:948-962`; check at `:1026-1027`). This distinction matters: graph
   iteration loops are emitted as canonical counted loops with an SSA induction phi, while
   DSL `while` loops keep their counter in memory — a missing phi means the parent is not
   the graph-iteration loop and treating it as the driver would delete the `while`'s trip
   count from the emitted nest (a 20k-vertex × 320k-edge effective hang is documented at
   `:1015-1025`).
3. The matching `autograph_neighbor_iter_init(graph, u, &iter)` call is found anywhere in
   the function by iterator pointer equality (`:1029-1046`).

The loop is then a "graph loop". The pass additionally treats a loop with **no iterator**
as a graph loop if it has a graph-data write (`loopHasGraphDataWrite`, `:1813-1828`), and
forces it sequential (no per-pair structure to rewrite) — this closes the racy-DOALL hole
for hand-rolled `a[nodes[i]]` shapes (`:4130-4133`, `:4172-4177`).

### 4.5 Effect discovery: the build walk

Inside `analyzeNeighborLoop`, after the two provenance instances are built (`:1057-1058`):

**Append-counter pre-pass** (`:1060-1097`). Scans all stores in the neighbor body. A store
through a GEP whose non-constant index has region **neither U, nor V, nor D** is a
non-endpoint array write — the frontier-append shape `next_frontier[next_size++] = v`. The
counter load's pointer goes into `IndexSlots` and `Info.AppendCountPtr`; the array base
slot into `Info.AppendArrayPtr`. These counters are *not* reductions. `HasFrontierAppend`
is then set when (a) the counter store value is a self-add (`:1090-1097`), or (b) the body
calls `roaring_bitmap_add` (`:1098-1109`, set-based frontier; `NextSetPtr` recorded).

**Frontier source recognition** (`:1110-1169`). The driver reads `v = frontier[i]` (init's u
operand) and loops while `i < frontier_size`:
- `FrontierArrayPtr` from the init's u GEP base (`:1112-1120`), with a fallback for the
  `int v = frontier[i]` scalar-copy shape (`:1121-1135`);
- `FrontierSizePtr/Val` from the driver exit compare's non-induction operand
  (`:1136-1150`);
- `FrontierSetPtr` from `roaring_bitmap_get_at_index` in the driver loop (`:1151-1159`) or
  `roaring_bitmap_get_cardinality` in its preheader (`:1160-1169`).

`Info.MembershipGated = FrontierSetPtr || FrontierArrayPtr` (`:1171`) — the driver is a
frontier (`F_t`) iteration.

**`classifyStore`** (`:1173-1219`) — the store → kind mapping:

```
store through GEP index with region R:
    detectFirstWinsClaim                → Claim (guard operands recorded)
    detectScalarRedOp / detectCondMinMax → Uop  (op = AOp)
    storedDependsOnOldValue             → Uf
    otherwise                           → W
```

**Body walk** (`:1234-1397`), over all blocks of the neighbor loop:

- *GEP-indexed store* (`:1237-1259`): skip append stores (`:1244-1255`); canonical base;
  `Region R = Prov.regionOf(IX)`; `noteWritten(R, Base)` (`:1222-1232`, records
  `WrittenBases`, `HasV/HasU`, `HasDataWrite`); push `classifyStore(...)`.
- *Scalar store* (`:1260-1361`): reduction-candidate handling.
  - The first non-append scalar slot becomes `Info.ReducePtr` with
    `detectScalarRedOp`/`detectConditionalMinMax`; its effect is recorded with
    `Reg = Region::G`, `Kind = Uop` (recognized) or `Uf`/`W` (`:1269-1292`).
  - A later guarded min/max store on the *same* slot can upgrade the first store to Uop
    (`:1293-1312`).
  - A *second, different* operator on the same recognized slot records a `Uf` `G` effect
    so the interpretation refuses the reduction (a per-loop partial folded with one
    operator would combine with the wrong op) (`:1313-1334`).
  - A *second, distinct* scalar slot records its own `Uop`/`Uf` `G` effect — no longer a
    refusal by itself, because composition R3 gives every recognized accumulator its own
    per-partition partial; only an unrecognized update stays a refusal (`:1335-1361`).
- *Load* (`:1364-1383`): a GEP-indexed load whose index region is U or V records an `R`
  effect on the canonical base (`ReadBases` updated).
- *`roaring_bitmap_add`* (`:1384-1396`): records `Activate` with `Reg = V` on
  `Info.NextSetPtr`.

**Activate synthesis** (`:1399-1414`): if the body has a frontier append
(`HasFrontierAppend && AppendArrayPtr`) but no Activate effect was recorded (the array
form `next_frontier[next_size++] = v`), synthesize `Activate(V)` on the append array so
the interpretation sees the dest-owned side effect.

**Read/write overlap** (`:1418-1420`): `NeedsRoundSep = WrittenBases ∩ ReadBases ≠ ∅`
(informational; classification now uses the refined `RoundSepBases`).

**Driver-preamble walk** (`:1422-1557`), over driver-loop blocks not contained in the
neighbor loop:

- *Accumulator consumption* (`:1448-1461`, composition I / P10): a GEP-indexed store whose
  value depends on the old value of `Info.ReducePtr` and whose index is U (via `Prov` or
  `ProvU`) is `Info.AccConsumeStore` — the per-source epilogue `arr[u] = acc`.
- *Accumulator reset* (`:1462-1469`): a plain store to `Info.ReducePtr` of a value that
  does not depend on it → `Info.AccResetSeen = true` (`acc = 0`).
- *U-indexed preamble stores* (`:1471-1503`): index region U via `Prov`/`ProvU` →
  `noteWritten(U, Base)`, pushed to `Info.DriverUStores`, guard recorded
  (`guardICmpForStore`, `:931-946`), effect pushed via `classifyStore` with
  `Scope = PerSource`; a Claim result sets `HasDriverClaim`.
- *CAS claims* (`:1504-1537`): `AtomicCmpXchgInst` carrying `sgpl.first_wins.claim`
  metadata with a U index → `Claim` U effect (`Scope = PerSource`, guard operands
  `CAS->getCompareOperand()/getNewValOperand()`), `HasFirstWins = true`,
  `DriverUClaims.push_back(CAS)` (kcore's `alive[u]=1 → alive[u]=0`).
- *U-indexed preamble loads* (`:1538-1556`): `R` effects with `Reg = U`,
  `Scope = PerSource`.

### 4.6 Effect analysis: structural expression, derived τ, round separation

**Expression construction** (`buildEffectExpr`, `:1865-1920`). The primitives are
partitioned into three static phase segments by occurrence scope (`PerSource` →
preamble, except the consume store → epilogue; `PerPair` → pair), and the
expression is built as a structural tree (`EffNode`, `:301-308`):

```
E_t = SeqDomain_{u∈F_t}( P(u) ; SeqDomain_{v∈N(u)} B(u,v) ; Q(u) )
```

with the round domain `Frontier` (membership-gated) or `AllVertices`.  Primitives
within a phase are composed with structural `Par` (concurrent; child order carries
no temporal meaning); `P ; B ; Q` is the only source of intra-round order; each
`Prim` node carries its `PhaseSegment` (Preamble/Pair/Epilogue).  `ParDomain` and
`Star` are grammar-only (debug-asserted unreachable).  The flat `EffectExpr`
segments are the derived projection `flatten(E)` (modulo Par permutation; multiset
equality with `Info.Effects`, checked by an assert).  The expression is built once
after all effect-producing walks; no path may append to `Effects` afterwards.

**Temporal derivation** (`relateReadWrite`, `:1931-1953`; `deriveAllTemporal`,
`:1955-1976`). For every read, the worst relation against every mutating primitive
is derived from the write's phase segment and the round-domain kind (never from σ
or `RoundSepBases`):

- same base, same region → `SameRoundRead`;
- `R(A,U)` vs `W(A,V)` with `W` in the pair phase → `PreviousRoundRead` when the
  driver is membership-gated (frontier round boundary), else `Carried`;
- `R(A,U)` vs `W(A,V)` with `W` outside the pair phase (sequenced within the
  round) → `SameRoundRead`;
- `R(A,V)` vs `W(A,U)` → `SameRoundRead` (staged DualOwner control, k-core `alive`);
- other cross-region pairs → `Independent`.

The write-phase condition is authoritative; the design note (§4,
`proof/EFFECT_ALGEBRA_DESIGN.md`) records the errata that rejects the
segment-equality variant (`Seg(e_r)≠Seg(e_w) → SameRoundRead`) and explains why
sssp's preamble read stays `PreviousRoundRead`.

Value source σ is assigned *after* round separation (`:1760-1774`): reads in a
`RoundSepBase::CrossReads` set get `Snapshot`, all other reads `Live`, non-reads
`None`.  σ is never an input to τ (`validate_refactor.sh` checks the dependency
invariant mechanically).

**Round-separation bases** (`:1573-1680`, composition A). A base is a round-separation
base when:

1. it has both an `R` effect and a mutating effect;
2. its mutating effects are single-region — all U or all V, none G/D (mixed U+V writes on
   one base is composition C, handled elsewhere);
3. the reads include the **opposite** endpoint region (cross-endpoint);
4. the element type is uniform `i32`/`double` (`:1657-1658`).

For each such base, `RoundSepBase` records `WritesV = (WReg == Region::V)` and
`CrossReads` — every load on the base whose index origin differs from the write region
(same-region RMW reads stay live: they are the running accumulator, and freezing them
would turn min/max/`+=`-style relaxes into last-write-wins, `:296-301`).

**WriteKind summary** (`:1682-1692`): `WriteData` (D-indexed write) / `WriteMixed` (U+V) /
`WriteV` / `WriteU` / `WriteUnknown` (reduction handled separately). This is the legacy
summary; classification reads the expression and summary directly.

### 4.7 Diagnostic output

`printEffects` (`:2485-2514`) prints the effect set as `Kind<Op>(Base,Reg)[:Temporal]`
joined by `⊗`, plus worst temporal and compatibility class. Under
`GRAPH_FRONTIER_VERBOSE`, `printEffectExpr` (`:2518-2552`) adds the segmented form
`pre=… pair=… epi=… gated=… total=…`. Real examples:

```
[graph-frontier] candidate: main driver=foreach.cond21 inner=foreach_nbr.cond kind=2 red=0
  sep=1 data=0 fw=0 env=0 shadow=0 class=source-owner
  R(out_degree,U):SameRoundRead ⊗ U+(out_degree,U)  temporal=SameRoundRead  compat=single
[graph-frontier] candidate: main driver=foreach_set.cond inner=foreach_nbr.cond64 kind=3
  ... class=sequential  R(alive,V):... ⊗ Claim(alive,U)  temporal=SameRoundRead  compat=no
  expr:  pre=R(alive,U)⊕Claim(alive,U)  pair=R(alive,V)⊕…⊕Activate(next,V)  epi=ε  gated=1 total=1
```

---

## 5. Effects → parallelization decision

### 5.1 The execution classes

```cpp
enum class Klass                          /* :1831-1852 */
{
    SourceOwner,      /* writes indexed by u only  → source-owned step */
    DestOwner,        /* writes indexed by v only  → dest-owned push   */
    Reduction,        /* single scalar Uop         → partials+combine  */
    DualOwner,        /* disjoint U+V writes       → source then dest step */
    Sequential,       /* everything unprovable     → mark sequential   */
    Privatized,       /* pure accumulation, no owner (composition R3)  */
    SourceReduction   /* per-source gather (composition I / P10)       */
};
```

### 5.2 Privatization proof (composition R3) — `privLayout`

`privLayout` (`:2103-2223`) fills `Info.Slots` / `Info.PrivArrays` when the whole effect
set is privatizable. The proof obligations (comment at `:1887-1921`):

> A loop whose every mutating effect is a recognized `U_⊕` update is a pure accumulation:
> the final value of each written location is the operator applied over the updates,
> independent of the order in which the partitions apply them. Give every partition a
> private copy of each written base, let it accumulate there, and fold the copies with the
> operator's own combine. Obligations:

1. **no claim, activation, or non-preserved occurrence** in the expression
   (`exprHasClaimOrActivate`, `:2095-2101`; `HasFrontierAppend`/`HasFirstWins`/
   `hasPerSourceClaim` at `:2115-2123`);
2. **every mutating primitive is an algebraic update `U(A,r,ω)` with ω known** — an
   unrecognized update has no combine to fold with (`:2125-2141`); `Reg` must be in
   {U,V,D,G} — a Top provenance refuses ("unknown index provenance");
3. **one operator per base** — two operators on one base would fold with the wrong one
   (`:2143-2170` for slots, `:2172-2195` for arrays);
4. **every load from a privatized base is a link in the old-value chain of an update to
   that same base, and reaches at least one such update** — a load used as an index, a
   call argument, or a branch condition would observe the real array, not this
   partition's copy (`loadFeedsOnlyUpdates`, `:2011-2048`; `baseIsPrivatizable`,
   `:2050-2093`, which checks the whole nest — neighbor loop *and* driver body, because a
   per-source preamble write `w[u] += 1` is also a store to the base);
5. **the fold law holds for ω on the element type** — integer arithmetic is exact modulo
   the type width; min/max are idempotent and commutative; float `+` and `*`
   re-associate, so they stay sequential (`opIsOrderIndependent`, `:1995-2009`).

Supporting helpers: `slotElemType` (`:1923-1937`, the type a slot's own loads use),
`arrayBaseInfo` (`:1939-1993`, resolve a base to element type / count / array pointer,
including the front end's `store ptr %cnt.data, ptr @cnt` global-slot convention, with
depth limit 3 and a two-different-arrays refusal).

`privLayoutNeeded` (`:2227-2230`): the layout is *needed* when the legacy paths cannot
express the loop — more than one scalar accumulator, or any array base. If the proof holds
but the layout is not needed (single scalar slot, no array), the legacy `Reduction` path
is used instead.

### 5.3 The interpretation ⟦E⟧_par — the classification function

`EffectSummary` (`:2450-2458`) is the fold of the expression into the predicates the laws
are stated over: `MutU/MutV/MutG/MutD/MutTop`, `HasUopG`, `HasUnrecognizedG`,
`HasCarriedOnMut`, `BaseU/BaseV`. `summarizeEffects` (`:2474-2546`) computes it as the
recursive homomorphism Φ over the tree (`forEachExprEffect`, `:2459-2467`, walks the
binder paths), with a defensive fallback to the raw set; `HasCarriedOnMut` is a projection
of the order-sensitive τ layer with the shadow exemption read from σ=Snapshot.
`interpretPar` (`:2554-2632`) is the interpretation function `⟦·⟧_par`; decision order,
exactly as written (note: `EffectExpr::Total` is gone — modelability is the separate
`provesModelable` judgment, §5.4):

1. **Data write** (`:2561-2562`): `HasDataWrite && !Priv` → `Sequential`.
2. **Occurrence preservation** (composition F, `:2563-2566`): a per-source claim cannot
   be replayed per pair → `Sequential` unless `SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1`.
3. **Top provenance** (`:2567-2568`): `MutTop` → `Sequential`.
4. **Carried read on a mutated base** (`:2569-2570`): → `Sequential` unless the read is
   the partition's own accumulator (`Priv`) or σ=Snapshot resolves it.
5. **Round-separation single ownership** (composition A, `:2572-2582`): with
   `RoundSepBases` non-empty and not both `MutU` and `MutV`, and all shadow bases
   agreeing on `WritesV` → `DestOwner` (writes V, reads U through shadow) or
   `SourceOwner` (writes U, reads V through shadow).
6. **Dual ownership** (`:2584-2604`): `MutU && MutV && !MutG`:
   - disjoint `BaseU ∩ BaseV = ∅`, no cross-phase data dependence
     (`crossPhaseDataDep`, `:1869-1891`: a V-effect store whose stored value reads a U
     base, or a U-effect store reading a V base), and no round-separation bases →
     `DualOwner`; if the body has a frontier append the envelope must be wired
     (`envelopeWired`, `:1858-1865`: append array + counter + frontier array + size, or
     frontier set + next set), else Sequential;
   - same-array U+V or a data dependence: `Priv` → `Privatized` (with
     `Info.UsePrivLayout = true`); else Sequential.
7. **Source reduction** (composition I / P10, `:2610-2614`): only under
   `SGPL_COMP_I_SOURCE_REDUCTION`, and only with `ReducePtr` + `AccConsumeStore` +
   `AccResetSeen` + recognized op, no data write, no `MutV/MutTop`, no carried read
   on mutation, no unrecognized G, no frontier append, no per-source claim, and no
   remaining preamble mutation (`preambleMutationCount`, `:1991-2004`, binder-path
   query). → `SourceReduction`. Fails closed on everything the finish hook does not
   reproduce.
8. **Frontier append** (`:2616-2621`): `HasFrontierAppend` → `DestOwner` if
   `envelopeWired && MutV && !MutU && !MutG`, else Sequential.
9. **Legacy reduction** (`:2624-2627`): `MutG && HasUopG && !HasUnrecognizedG &&
   !MutU && !MutV && !privLayoutNeeded` → `Reduction`.
10. **Single ownership** (`:2628-2631`): `MutV && !MutU && !MutG` → `DestOwner`;
    `MutU && !MutV && !MutG` → `SourceOwner`.
11. **Privatization fallback** (`:2632-2636`): `Priv` → `Privatized`; otherwise →
    `Sequential` (`:2637`).

### 5.4 The soundness envelope: `provesModelable`

The rewrite models a neighbor-loop body as pair-local array effects with U/V/D
provenance, first-wins claims and set appends. A body element outside that model has no
defined per-pair meaning, so the rewrite may invent semantics — both historical
miscompiles (inline graph query subloops → malformed IR; float header PHI → `fadd ptr,
double`) were coverage violations (`:4299-4311`). `provesModelable` (`:4312-4373`) is the
**totality form** of the check:

1. no subloops (`:4315-4319`);
2. scalar loop-carried PHIs must be integer and must not escape the loop (a scalar
   reduction stored after it; loop-internal integer bookkeeping is allowed)
   (`:4321-4347`);
3. calls must be the modelled runtime helpers (`autograph_neighbor_iter_init/next`,
   `roaring_bitmap_add/remove`, profile helpers, `sgpl_now_ns`) or LLVM intrinsics;
   anything indirect or unrecognized is opaque and refuses (`:4349-4371`).

`legacyBlocklistRefuses` (`:4268-4297`) is the old shape blocklist kept only for A/B
testing under `SGPL_FRONTIER_BLOCKLIST_GUARD=1`.

### 5.5 The sequential fallback contract

`markSequential` (`:1035-1055`) puts `sgpl.frontier.nested.sequential` metadata on the
header terminator of the loop, **all its ancestors, and every subloop** (beta-init / swap
/ driver nests). The PDG classifier reads it and hard-classifies SEQUENTIAL (never
DOALL/DOACROSS); the outliner's `parseParallelMode` never outlines it; the reconstruction
leaves the nest inline. The motif engine is untouched — it already had its chance before
the PDG.

---

## 6. Lowering (emission)

### 6.1 Pass run flow

`GraphFrontierLoweringPass::run` (`:4375-4538`), per loop in preorder:

1. `analyzeNeighborLoop` (iterator shape) or `loopHasGraphDataWrite` (hand-rolled);
   skip loops that are neither (`:4390-4395`).
2. `Klass K = IsIter ? classify(Info) : Klass::Sequential` (`:4398`) — iterator-less loops
   have no per-pair structure to rewrite.
3. Stats print (`:4399-4420`), including the segmented expression under
   `GRAPH_FRONTIER_VERBOSE`.
4. `GRAPH_FRONTIER_REWRITE_OFF` → `markSequential` only (`:4424-4430`).
5. Iterator-less → sequential (or released to PDG under `SGPL_PDG_SECOND_CHANCE`)
   (`:4435-4442`).
6. Modelable check (`:4445-4454`): `SGPL_FRONTIER_BLOCKLIST_GUARD ? legacyBlocklistRefuses
   : provesModelable`.
7. `Rewritable = (K != Sequential) && Modelable` (`:4456`), then dispatch
   (`:4460-4469`):

   ```cpp
   if      (K == Klass::DualOwner)       Emitted = emitDualCleanCut(Info);
   else if (K == Klass::SourceReduction) Emitted = emitSourceReductionStep(Info);
   else if (K == Klass::Privatized)      Emitted = emitPrivatizedStep(Info);
   else                                  Emitted = emitCleanCutCallbackAndStep(
                                                Info, K == Klass::SourceOwner);
   ```

8. On success: `markSequential(L)` — keep every part of the round nest serial (the step
   is already parallel via CleanCut; the residual beta-init/swap loops must not be
   DOALL/GPU-ified) (`:4478-4484`).
9. **Postcondition** (`:4503-4522`): `verifyFunction(F)` after emission; on failure print
   and, under `SGPL_FRONTIER_STRICT`, abort.
10. Emit failure or non-rewritable → `markSequential` (default), or PDG release under
    `SGPL_PDG_SECOND_CHANCE` (`:4525-4532`).

### 6.2 The pair work function — `emitPairWorkFn`

`emitPairWorkFn(Info, Phase)` (`:3191-3685`) manually clones the per-pair body into the
runtime ABI (no CodeExtractor — single-block bodies were out of reach and its
leave-behind loop reshaping broke later passes):

```cpp
void sgpl_frontier_pair_fn(int32_t source, int32_t destination,
                           int64_t local_index, void *work_env);
```

**Setup** (`:2929-3018`):

- Body = inner-loop blocks minus the header.
- Entry block allocates `uslot`/`vslot` and stores the args (`:2960-2963`).
- u terminals: `U64 = Info.UVal` (init's i64 u), `U32 = U64`'s operand after the cast;
  `U64Src = zext ArgU` (`:2965-2970`).
- Reduction partial: when `Info.ReducePtr` is set, `ArgEnv` is bitcast to the slot's
  element type — the body's load/op/store of `ReducePtr` maps to the partition's running
  partial (the runtime pre-initializes partials to the operator identity) (`:2972-2993`).
  The comment documents the historical bug this mapping fixes: a per-call zero-init slot
  overwrote the partial each edge, zeroing multiplicative reductions.
- Envelope `SeenBase` = `ArgEnv` bitcast to `i32*` when the envelope is wired
  (`:2982-2995`).
- Round-separation shadow pointers: load each per-base shadow global (published by the
  preheader) and collect the `CrossReads` load set (`:2997-3018`).
- Block map for the body, plus a `pair_ret` stub (`:3020-3026`).

**Value map / rebinding** (`:3028-3038`):

```cpp
Map[U32]              = ArgU;      /* i32 source */
Map[U64]              = U64Src;    /* i64 source */
Map[Info.VAlloca]     = VSlot;     /* v ← dst */
Map[Info.UAlloca]     = USlot;     /* foreach-set u slot */
Map[driverIndVar]     = U64Src;
Map[Info.ReducePtr]   = RedPartial;         /* (PairPhase::All && ReducePtr) */
```

**Privatization record mapping** (`:3040-3068`): when `UsePrivLayout`, the reduction
record *is* the partition's private state. Scalar slots sit at fixed 8-byte offsets
(`Map[S.Ptr] = gep i8 ArgEnv, off`); array slots hold a pointer to the partition's
private copy, published by the preheader's `autograph_priv_bind` (`PrivPtrForBase`).

**Cloning** (`:3070-3149`, `CloneValue` recursive lambda):

- mapped value → replacement; constants/args pass through;
- PHIs map to `USlot` (the pair wrapper has no loops; the convention preserves the
  u-slot value);
- clone the instruction, remap operands recursively; on a privatized base, a cloned GEP
  is re-pointed at the partition's private copy (`:3097-3103`); a cloned load that is a
  cross-endpoint read on a round-separation base gets its pointer re-built against the
  shadow snapshot while sibling stores through the same GEP still write the live array
  (`:3104-3133`);
- insertion: into the cloned parent block when possible (keeps load/icmp order relative
  to stores, e.g. kcore `deg--` then `deg<k`), else entry (`:3134-3146`).

**Elision rules** (`:3151-3204`):

- *append elision* (`UseEnvelope`): `roaring_bitmap_add` calls and stores to the append
  counter / append array are elided from the cloned body; a separate pass
  (`:3311-3342`) rewrites each elided append into `dest_seen[v] = 1` in-place (the
  runtime appends dests whose seen flag transitioned 0→1).
- *phase elision* (DualOwner): `VOnly` elides U-region mutating origins; `UOnly` elides
  V-region mutating origins, `roaring_bitmap_add`, and append stores
  (`mutatingOriginIs`, `:2712-2719`).

**U-preamble-only wrapper** (`:3206-3241`): when `Phase == UOnly` and the neighbor body
has no U mutating effect, emit only the driver U-stores behind the cloned
`DriverUGuard`.

**Two-phase clone + remap** (`:3243-3275`): phase 1 clones all non-terminator
instructions into their mapped blocks so the map is complete before operand remap
(avoids hoisting icmps into entry); phase 2 remaps operands; failure anywhere deletes the
function and returns null (the caller falls back to sequential).

**Shadow redirection pass** (`:3277-3309`): after remap, cross-read loads are re-pointed
at the shadow GEP (a separate pass so the cloned index operands are final regardless of
which block the GEP lives in).

**Control flow** (`:3342-3371`): unconditional branches to the mapped successor or
`pair_ret`; conditional branches with the cloned condition (failure → null); non-branch
terminators → `pair_ret`. `FirstClone` becomes the entry's branch target (`:3420-3421`).

**DualOwner U-only grafting** (`:3373-3419`): driver U-stores and first-wins CAS claims
are cloned into the U-phase wrapper's entry (they belong on the source pair fn, not
grafted into a dest-owned body).

### 6.3 Single-phase emission — `emitCleanCutCallbackAndStep`

`emitCleanCutCallbackAndStep(Info, SourceOwner)` (`:3687-3755`), emitting into the
driver's **loop preheader**:

1. `WF = emitPairWorkFn(Info, PairPhase::All)` (`:3427-3429`).
2. **Fresh graph load** (`:3442-3445`): the init's graph load lives in the soon-dead
   driver body; a `load` of the graph global is re-issued at the call site (dead-value
   replacement would turn the old load into poison).
3. `PartCount = autograph_build_clean_cut(graph, 0)` (`:3446-3448`) — `0` means
   "default partition count".
4. **Reduction path** (`:3452-3480`): allocate `red_partials[PartCount]` of the slot's
   element type; `emitPartialInit` (`:2667-2695`) fills each partial with the operator
   identity (`identityFor`, `:2628-2661`: `Mul`→1, `And`→all-ones, `Min`→`+∞`/`INT_MAX`,
   `Max`→`-∞`/`INT_MIN`, `MinU`→all-ones, `MaxU`→0, float min→`+∞`, float max→`-∞`,
   else 0) via a small count loop split out of the preheader; `emitRedCombiner`
   (`:2561-2623`) creates `sgpl_combine_red(out, partial)` performing
   `*out = *out (foldOp) *partial` with the record's fold operation; then call
   `autograph_frontier_step_owner_red(graph, null, 0, WF, partials, elemBytes,
   combiner, ReducePtr, ...)`.
5. **Owner path** (`:3481-3493`): `fillFrontierEnv` (`:2745-2821`) — obtains
   `autograph_scratch_membership(graph)`, and when the envelope is wanted:
   `autograph_scratch_dest_seen`, the next-frontier scratch, and
   `autograph_prepare_frontier_bitmap/array` to materialize `F_t` as a membership mask
   and get its size; `emitRoundSepShadow` (`:2892-2927`) — for each round-sep base:
   grab a `autograph_scratch_shadow` buffer of `n * elemBytes`, publish its pointer
   through the pass-created global `fn.<base>.shadow.<i>`, and `memcpy` the live array
   into it (the pair fn then reads the frozen round-start snapshot);
   `callOwnerStep` (`:2823-2842`) calls `autograph_frontier_step_owner_source` or
   `autograph_frontier_step_owner_push` with
   `(graph, frontier, size, WF, seen?, membership, next?, 0, seen?)`; the step returns
   the new frontier size; `commitEnvelope` (`:2844-2863`) writes it back
   (`autograph_commit_frontier_bitmap` or store to `AppendCountPtr`) and sets
   `sgpl.frontier.first_wins.doall` metadata (`requires-int-append-priv`) on the driver
   header terminator.
6. `deactivateDriver` (`:2865-2872`): the driver header's true edge is rewired to its
   false (exit) successor — the old nest is dead and DCE removes it.

### 6.4 DualOwner emission — `emitDualCleanCut`

`emitDualCleanCut` (`:3758-3804`): builds two work functions —
`WFu = emitPairWorkFn(Info, PairPhase::UOnly)` (source phase: driver U-stores, CAS
claims, U-region writes) and `WFv = emitPairWorkFn(Info, PairPhase::VOnly)` (dest phase:
V-region writes, appends) — then in the preheader: `build_clean_cut`, one
`fillFrontierEnv` (membership `F_t` prepared **once**, shared by both phases),
`emitRoundSepShadow` before *both* phases (both work fns read the same pre-round
snapshot; without this the shadow global is referenced but never initialized — the
documented link failure), then `step_owner_source(WFu, WithEnvelope=false)` followed by
`step_owner_push(WFv, WithEnvelope=WantEnvelope)`, `commitEnvelope`, `deactivateDriver`.
This is "source step then dest step on the same pre-round `F_t`" — k-core's per-round
peel.

### 6.5 SourceReduction emission — `emitSourceReductionStep`

`emitSourceReductionStep` (`:3887-3970`) reproduces the per-source gather:

1. `WF = emitPairWorkFn(Info, PairPhase::All)` with `ReducePtr` mapped to the partition
   partial (same mapping as the reduction class).
2. `red_partials[PartCount]` identity-filled via `emitPartialInit`.
3. A **finish hook** `sgpl_source_finish(i32 src, i8* partial)`: clone the consume
   store's value and pointer through `clonePreambleValue` (`:3550-3617`) — a bounded
   recursive clone allowing only value-shaping instructions (loads, GEPs, casts, binary
   ops, icmps, selects); the accumulator load becomes the hook's `AccRepl` (the
   partition's partial), the driver induction variable becomes `SrcArg`; anything that
   could observe engine or traversal state fails closed and the whole rewrite declines.
   The hook stores the epilogue result, then **resets the partial to the operator
   identity** for the next source (`:3686-3687`).
4. Call `autograph_frontier_step_owner_source_red(graph, null, 0, WF, finish, partials,
   elemBytes, ...)`.
5. `deactivateDriver`.

Sources are owned by disjoint partitions (source ranges), so the epilogue writes are
race-free.

### 6.6 Privatized emission — `emitPrivatizedStep`

`emitPrivatizedStep` (`:4111-4257`) emits the R3 record layout:

1. `WF = emitPairWorkFn(Info, PairPhase::All)` — body GEPs on privatized bases re-pointed
   at the partition's private copy; scalar slots at fixed 8-byte offsets.
2. `build_clean_cut` for `PartCount`; record of `RecSize = 8*(NSlots + NArrays)` bytes
   per partition, allocated as one alloca (`:3879-3883`).
3. Scalar partials identity-filled by `emitPrivScalarInit` (`:3715-3749`), a count loop
   over partitions writing `identityFor(S.Op, S.ElemTy)` into each slot field.
4. Per array: `autograph_priv_bind(graph, rec, RecSize, offset, elems, elemBytes,
   identityBits, slot)` (`:3902-3927`) — the runtime allocates per-partition copies,
   identity-initializes them, and publishes partition p's pointer into its record.
5. Targets descriptor `[slot ptrs][array ptrs][array element counts]` on the stack
   (`:3929-3947`).
6. `emitPrivCombiner` (`:3761-3841`) creates `sgpl_priv_combine(rec, targets)`: for each
   scalar slot, `combiner(partial, out)`; for each array, an element-wise fold loop over
   the private copy into the live array, reusing the same per-operator combiner (the
   copies start at the operator identity, so the running total starts from the array's
   own pre-round value).
7. If a driver-preamble update exists (`DriverUStores` non-empty — `w[u] += 1` runs once
   per source, not per arc), a second work function `PreambleFn =
   emitPairWorkFn(Info, PairPhase::UOnly)` is built and the step becomes
   `autograph_frontier_step_owner_red_pre(graph, PreambleFn, WF, rec, RecSize,
   combiner, targets, null)`; otherwise `autograph_frontier_step_owner_red(graph, null,
   0, WF, rec, RecSize, combiner, targets, ...)` (`:3953-3992`).
8. `deactivateDriver`.

---

## 7. Runtime parallelization

All runtime code is in `autotuner_runtime.c` (CleanCut partition build and the step
functions) and `parallel_runtime.c` (the thread pool). The runtime is unchanged by the
algebraic cutover: `⟦E⟧_par` selects compositions of these primitives.

### 7.1 Partition build — `autograph_build_clean_cut`

**Home partition rule** (`autotuner_runtime.c:2962`):

```
CC_PART_OF(v) = v * P / n          /* destination d belongs to lane d*P/n */
```

i.e. contiguous destination ranges — the owner-computes rule (`:2486-2488`,
`:2661-2670`). Exactly one worker ever writes a given destination slot.

**Cache** (`:74-94`, `:3112-3138`): the partition structures are a pure function of
(graph topology, current layout, partition count); the emitted driver calls build once
per rewritten step, which for round-loop programs means once per round (measured 85 ms
per call on 100k/800k against ~9 ms of step work). The cache reuses them until
`layout_epoch` (bumped by `autograph_set_layout`), the edge counters (canonical mutation
API), or the partition count change; `SGPL_NO_CLEANCUT_CACHE=1` disables reuse.

**`autograph_build_clean_cut_inner`** (`:2907-3105`):

- partitions: default `workers * 4` (min workers), `SGPL_CLEANCUT_PARTITIONS` override,
  capped at `n` (`:2926-2941`);
- `start[p] = p*n/P` (`:2958-2959`);
- **layout-native arc enumeration** (`cc_arcs_init`, `:2761-2872`): never converts to
  CSR —
  - CSR: zero-copy borrowed view of `csr_row_ptr/col_idx` (u-major);
  - PCSR: walk rows, skipping GAP (`-1`) sentinel slots;
  - BCSR: block-decode `(local_row, col)` payloads;
  - SET: walk the loader's static base pairs in raw order, expanding undirected pairs to
    both arcs;
- pass 1 counts unique source rows and edges per partition (`:2964-2986`); pass 2 fills,
  with one ascending source scan, the **per-partition source-grouped CSR**:
  `indir[p]` (source rows), `rp[p]` (row offsets), `ci[p]` (destination columns)
  (`:2996-3034`) — a push worker scans exactly the edges whose destination it owns;
- **source-owned slices**: `src_pairs[p]` = flat `[(u,v),(u,v),...]` arc list for the
  partition owning source range `[start[p], start[p+1])`, by the exact inverse
  `p(u) = ((u+1)*P-1)/n` (`:3037-3071`) — the source step reads these prebuilt slices,
  never the transient layout.

**Thread pool** (`parallel_runtime.c:5162-5258`, `parallel_for_runtime`): computes the
trip count, acquires threads from the loop pool (`sgpl_loop_pool_try_acquire`), falls
back to a **serial run** of the same body when the budget grants ≤1 thread (correctness
is identical — the partition body is just the serial work sliced), and otherwise launches
via `sgpl_parallel_launch_plain_raw`. Worker count: `sgpl_configured_worker_count()`
(`parallel_runtime.c:808`).

### 7.2 The steps

All steps are `parallel_for_runtime(0, partition_count, 1, body, env, 0, 0)` — one task
per partition.

**`autograph_frontier_step_owner_push`** (`:3182-3218`, body `:3154-3180`) — dest-owned
push. Partition p scans its source-grouped CSR: for each source row `u` (membership gate
`membership[u]`), for each `v` in the row: `work_fn(u, v, v, env)`. If the caller wired
`dest_seen`, a destination whose seen flag transitioned 0→1 (the elided append rewrote
`dest_seen[v] = 1` inside `work_fn`) is appended to `next_frontier` under an **atomic
head counter** (`atomic_fetch_add(&appended, 1)`). Race-freedom: partition p exclusively
writes destinations in `[start[p], start[p+1])`; CSR source order within a partition =
serial order → bit-exact.

**`autograph_frontier_step_owner_source`** (`:3264-3298`, body `:3239-3262`) —
source-owned traversal. Partition p walks its flat `src_pairs[p]` slice (membership gate
on u), calling `work_fn(u, v, v, env)`; same `dest_seen` append protocol. Writes indexed
by the source (`out_degree[u]++`) are race-free because each source belongs to exactly
one partition, and sources are visited ascending within a partition → exactly the serial
edge order, zero-copy.

**`autograph_frontier_step_owner_red`** (`:3519-3550`, body `:3415-3431`) — per-partition
partials. Each partition accumulates its pair work into `partial = work_env + p *
partial_bytes` (the pair fn maps its reduction slot to `ArgEnv`); after the parallel
loop, the runtime calls `combine_fn(partial_p, out)` for p in **ascending partition
order** — deterministic combine order, each partial summed in CSR order. (The generic
predecessor `autograph_frontier_step_owner`, `:2617-2659`, is the lane-based
pull-dest variant kept for the old motif walker shape.)

**`autograph_frontier_step_owner_source_red`** (`:3366-3398`, body `:3318-3364`) —
per-source gather. Partition p walks its source-sorted pair slice; when the source
changes, `finish_fn(pending, partial)` consumes the partial and the body keeps
accumulating into the same partition partial; after the loop, a merge walk calls
`finish_fn` for every source in the partition's range **that has no pairs** (the serial
program still runs its accumulator reset + epilogue). Each source is visited by exactly
one partition, so the finish hook and the result write are race-free without atomics.

**`autograph_frontier_step_owner_red_pre`** (`:3488-3517`, body `:3453-3486`) — R3 with a
per-source preamble. Before the first pair of each source, `preamble_fn(u, -1, -1,
partial)` runs the driver-preamble update once per source (again including zero-pair
sources via the merge walk); then `work_fn` runs per pair. Both write only the
partition's private state; the combine folds the copies in ascending partition order.

**`autograph_priv_bind`** (`:1886-1924`): allocates (reusing) `partition_count` buffers
of `elems * elem_bytes` each, re-initializes every element to the operator identity
(`identity_bits` is the little-endian byte pattern of the element type; `elem_bytes` 4 or
8), and publishes partition p's pointer into its reduction record at `rec_off` — the pair
work function reaches its own copy through the same record it uses for scalar partials.

**Scratch / envelope helpers** (`:1834-1959`):

- `autograph_scratch_membership(graph)` → the `F_t` membership byte mask (prepared by
  `autograph_fill_frontier_membership` via `prepare_frontier_array/bitmap`);
- `autograph_scratch_dest_seen(graph)` → zeroed `n`-sized int array;
- `autograph_scratch_next_frontier(graph)` → the next-frontier array;
- `autograph_scratch_shadow(graph, bytes, slot)` → lazily (re)allocated, slot-indexed
  round-separation shadow buffer (4 slots, size rounded to 8);
- `autograph_prepare_frontier_array/bitmap` → materialize the frontier and fill the
  membership mask; `autograph_commit_frontier_bitmap` → clear the next bitmap and append
  the recorded next frontier (`:1952-1959+`).

### 7.3 Why the runtime is race-free by construction

- **Dest-owned push**: partition p exclusively writes destinations in its contiguous
  range; no atomics on the array cells; source order within a partition = serial order
  → bit-exact (`:2661-2670`, `:3182-3218`).
- **Source-owned**: each source belongs to exactly one partition; ascending source order
  → serial edge order (`:3220-3226`).
- **Reduction**: per-partition partials (no shared accumulator), deterministic ascending
  combine (`:3400-3405`).
- **Source reduction**: one partition per source; finish hook and result write
  race-free (`:3300-3304`).
- **Privatization**: every written location is partition-private; the fold is a
  per-operator combine in ascending partition order (`:1677-1710`, `:3433-3443`).
- The only atomics anywhere are the **frontier append head counters** (`appended`), which
  are monotonic counters, never data.

These obligations are *stated* by the algebra (`owner` uniqueness, claim total order)
and *discharged* here by construction — see `proof/EFFECT_ALGEBRA_DESIGN.md` §8.

---

## 8. Appendix

### 8.1 AutoTuner / PDG interplay

- The CleanCut pass runs before the AutoTuner region pass and PDG/outliner
  (`main.cpp:671-689`).
- The AutoTuner pass annotates **both IR versions**: the (dead) loop regions keep their
  events, and each CleanCut step call gets a paired `Traverse` region (same graph, same
  `totalOps` prediction) with `profile_region_enter` before and `profile_region_exit`
  after the call, so predicted-vs-measured and layout decisions cover the executed
  kernel (`CLEANCUT_LOOP_SPEC.md:134-140`; `CC_AUTOTUNER_EDGEMAP_REGION.md:75-97`,
  `AutoTunerPass.cpp:1491`, `:2166`).
- Layout conversions are safe for CleanCut graphs because the partitions are
  layout-agnostic (`cc_arcs_init` enumerates whatever layout the AutoTuner picked).

### 8.2 Example classification trace

`proof/1_classify.txt` (pagerank):

```
[graph-frontier] candidate: main driver=foreach.cond21 inner=foreach_nbr.cond kind=2
  red=0 sep=1 data=0 fw=0 env=0 class=source-owner
  R(out_degree,U):SameRoundRead ⊗ U+(out_degree,U)  temporal=SameRoundRead  compat=single
[graph-frontier] candidate: main driver=foreach.cond64 inner=foreach_nbr.cond87 kind=1
  red=0 sep=1 data=0 fw=0 env=0 class=dest-owner
  R(next_rank,V):SameRoundRead ⊗ U+(next_rank,V)  temporal=SameRoundRead  compat=single
```

Segmented form (kcore dual loop, `GRAPH_FRONTIER_VERBOSE=1`):

```
expr:  pre=R(alive,U)⊕Claim(alive,U)  pair=R(alive,V)⊕R(deg,V)⊕U-(deg,V)⊕…⊕Activate(next,V)
       epi=ε  gated=1 total=1
```

Emitted calls (`proof/3_steps.txt`):

```
call i32 @autograph_frontier_step_owner_push(ptr %5, ptr null, i32 0, ptr @sgpl_pair_work.1, ...)
call i32 @autograph_frontier_step_owner_source(ptr %0, ptr null, i32 0, ptr @sgpl_pair_work, ...)
```

### 8.3 Definition index

| Concept | Definition | Location |
|---|---|---|
| Region lattice | `enum class Region`, `joinRegion` | `graph_frontier_lowering.cpp:164-175` |
| Region boundary | `isProvRegion` (provenance never yields G) | `graph_frontier_lowering.cpp:221-228` |
| Provenance | `class Provenance` | `graph_frontier_lowering.cpp:445-518` |
| Operator data | `enum class RedNorm`, `struct RedOpInfo`, `redOpInfo` | `graph_frontier_lowering.cpp:114-155` |
| Fold-soundness | `foldSound(ω,T)` | `graph_frontier_lowering.cpp:2191-2211` |
| Effect record | `struct Effect`, `EffectKind`, `Temporal`, `OccurrenceScope`, `ValueSource` | `graph_frontier_lowering.cpp:177-248` |
| Expression tree | `DomainKind`, `PhaseSegment`, `struct EffNode`, `struct EffectExpr` | `graph_frontier_lowering.cpp:286-321` |
| Tree builder | `newEffNode`, `buildParTree`, `flattenTree`, `collectTreePrims`, `buildEffectExpr` | `graph_frontier_lowering.cpp:1805-1922` |
| Temporal derivation | `relateReadWrite`, `deriveAllTemporal` | `graph_frontier_lowering.cpp:1931-1976` |
| Occurrence preservation | `hasPerSourceClaim`, `preambleMutationCount` (binder paths) | `graph_frontier_lowering.cpp:1978-2001` |
| Value source σ | assignment after RoundSep (`VSource`) | `graph_frontier_lowering.cpp:1760-1774` |
| Canonical base | `canonicalArrayBase`, `sameArraySlot` | `graph_frontier_lowering.cpp:524-599` |
| Store kinding | `classifyStore` | `graph_frontier_lowering.cpp:1244-1288` |
| Effect build | body walk in `analyzeNeighborLoop` | `graph_frontier_lowering.cpp:1057-1776` |
| Driver preamble | preamble walk | `graph_frontier_lowering.cpp:1496-1650` |
| Round separation | `RoundSepBase` construction | `graph_frontier_lowering.cpp:1638-1759` |
| Privatization proof | `exprHasClaimOrActivate`, `privLayout` | `graph_frontier_lowering.cpp:2302-2430` |
| Summary fold Φ | `EffectSummary`, `summarizeEffects` | `graph_frontier_lowering.cpp:2450-2546` |
| Interpretation ⟦·⟧_par | `interpretPar` | `graph_frontier_lowering.cpp:2554-2637` |
| Classifier entry | `classify` | `graph_frontier_lowering.cpp:2639-2657` |
| Modelability | `provesModelable` | `graph_frontier_lowering.cpp:4312-4373` |
| Witness dump | `printWitness` (`SGPL_WITNESS_DUMP`) | `graph_frontier_lowering.cpp:2767-2814` |
| Pair fn clone | `emitPairWorkFn` | `graph_frontier_lowering.cpp:3191-3685` |
| Single-phase emit | `emitCleanCutCallbackAndStep` | `graph_frontier_lowering.cpp:3687-3755` |
| Dual emit | `emitDualCleanCut` | `graph_frontier_lowering.cpp:3758-3804` |
| Source-red emit | `emitSourceReductionStep` | `graph_frontier_lowering.cpp:3887-3970` |
| Privatized emit | `emitPrivatizedStep` | `graph_frontier_lowering.cpp:4111-4257` |
| Partition build | `autograph_build_clean_cut(_inner)` | `autotuner_runtime.c:2907-3138` |
| Push step | `autograph_frontier_step_owner_push` | `autotuner_runtime.c:3182-3218` |
| Source step | `autograph_frontier_step_owner_source` | `autotuner_runtime.c:3264-3298` |
| Red step | `autograph_frontier_step_owner_red` | `autotuner_runtime.c:3519-3550` |
| Source-red step | `autograph_frontier_step_owner_source_red` | `autotuner_runtime.c:3366-3398` |
| Preamble+priv step | `autograph_frontier_step_owner_red_pre` | `autotuner_runtime.c:3488-3517` |
| Private copy bind | `autograph_priv_bind` | `autotuner_runtime.c:1886-1924` |
| Thread pool launch | `parallel_for_runtime` | `parallel_runtime.c:5162-5258` |
| Golden harness | `validate_algebra.sh`, `validate_refactor.sh` | repo root |
| Algebra design note | `proof/EFFECT_ALGEBRA_DESIGN.md` | repo `proof/` |
