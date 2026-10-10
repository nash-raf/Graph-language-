# Effect System — CPU technical report

Complete description of the implemented effect system on the CPU path: for every
part, **theory first**, then **the code that implements it**, then the **logic /
control flow** that ties them together.  Files: `graph_frontier_lowering.cpp`
(compiler), `autotuner_runtime.[ch]` (runtime ABI + executor + ready-work DAG
scheduler), `parallel_runtime.[ch]` (pool + worker-budget ledger),
`pdg.cpp` (verdict consumer).  Line numbers are the state of the working tree
recorded in `verify/TWO_AXIS_STATUS.md`.

---

## 0. The contract, in one screen

**Theory (theorem, normative).**

```
R_S = R1 ∨ R3 ∨ R6 ∨ R7          (spatial interference)
R_T = R2 ∨ R4 ∨ R5              (temporal interference)
NI_S ⟺ ¬R_S ,  NI_T ⟺ ¬R_T ,  ¬NI ⟺ R_S ∧ R_T
pipeline:  R1..R7 → (R_S,R_T) → (NI_S,NI_T) → (G_S,G_T) → schedule
```

R8 is *outside* the equivalences: the implementation-level full-serial guard.
Two invariants drive every design decision below:

1. **A witness on one axis constrains only that axis.**  It cannot suppress
   proven parallelism on the other axis.
2. **Legality comes from witnesses, never from graph heuristics.**  Graphs are
   downstream *realizations* of established semantic facts.

**Code.**  The four identities and the pipeline live in
`graph_frontier_lowering.cpp`: `AxisCertificate` (the (R_S,R_T) state),
`selectSchedule` (the (G_S,G_T) → schedule mapping), and the invariants are
enforced by `witnessesConsumed` (compile time) plus the runtime template
validator in `autograph_execute_dag` (execution time).

**Logic.**  Nothing downstream re-derives legality: the emitter may only
*realize* the chosen schedule; the runtime may only *execute* templates whose
edges carry a witness id (semantic) or are explicitly tagged `RealizationOrder`;
every refusal is either a *semantic* one (an R1–R7 witness with no licensed
route) or an *implementation* one (`impl_failure=1`, with its own reason), never
a mix of the two.

---

## 1. IR vocabulary: regions and provenance

**Theory.**  Every access is mapped to a **region** that names *who owns the
element*: `U` (source-owned, indexed by the driver vertex u), `V`
(destination-owned, indexed by the neighbour v), `G` (graph-derived value such
as a degree), `D` (a data value read from an array element), `Bottom` (no
origin, e.g. a constant), and `Top` (two distinct non-bottom origins — an origin
the analysis cannot name).  The lattice join is what makes `Top` meaningful:
`Bottom ∨ X = X`, and **`A ∨ B = Top` for two distinct non-bottom origins**.
Interference is a property of these regions, not of subscript text.

**Code** (`graph_frontier_lowering.cpp`):

```cpp
enum class Region : uint8_t { U, V, G, D, Top, Bottom };
static Region joinRegion(Region A, Region B) {
    if (A == Region::Bottom) return B;
    if (B == Region::Bottom) return A;
    if (A == B) return A;
    return Region::Top;          /* two distinct non-bottom origins */
}
```

`Provenance::lookup` walks an instruction's operands: the u-slot yields `U`, the
v-slot `V`, an array-element load `D`, graph-derived calls `G`; a PHI/BinOp/GEP/
Select joins its operands, and anything unrecognized is `Top`.  Stores are
classified by `noteWritten`:

```cpp
auto noteWritten = [&](Region R, const Value *Base) {
    if (Base) WrittenBases.insert(Base);
    if (R == Region::V) HasV = true;
    else if (R == Region::U) HasU = true;
    else if (R == Region::D) Info.HasDataWrite = true;
};
```

**Logic.**  A store's region is the provenance of its pointer's index.  So
`A[u] = 1` writes `U`, `A[v] = 1` writes `V`, `A[deg[u]] = 1` writes `D`
(the index reads an array element), `A[u + v] = 1` writes `Top` (the join of U
and V), and `A[0] = 1` writes `Bottom`-ish (no origin) — which is why the
spatial witnesses below key on exactly these cases.

---

## 2. Effects and the effect expression

**Theory.**  The semantic object is not the loop nest but the **effect
expression**: the ordered tree of reads/writes with legal barriers, split into
three segments — `Preamble` (driver-level, once per source), `Pair` (the
neighbour body), `Epilogue`.  Segment identity is what decides temporal
questions (§3) and the occurrence scope (`Once | PerSource | PerPair`) is what
decides staging questions.

**Code.**  `Effect { Kind, Reg, Base, Origin, ... }` with effect kinds
`R | W | Uf | Uop | Claim | Activate` (`Uf` = write depends on the old value;
`Uop` = recognized algebraic combine; `Claim` = first-wins; `Activate` =
frontier append / dest-owned side effect).  `buildEffectExpr` assembles the
tree; `summarizeEffects` computes the per-region summaries (`MutU/MutV/MutG/
MutD/MutTop`, `BaseU/BaseV`, `HasDataWrite`, `HasFrontierAppend`).

**Logic.**  Every later question — "is this read carried?", "is this write an
append?", "is this claim stageable?" — is answered from the tree and the
summaries, never by re-inspecting the loop.  This is what keeps the analysis
monotone: witnesses are derived once (§4/§5) and the emitter merely realizes
them.

---

## 3. Temporal axis — theory

**Theory.**  For each read, the relation τ against every mutating primitive is
one of:

| τ | meaning |
|---|---|
| `Independent` | the read cannot observe the write |
| `SameRoundRead` | the write is sequenced by ownership *within* the round (same region, or a staged control write) |
| `PreviousRoundRead` | frontier-gated: the read observes the round-start frontier's value |
| `Carried` | the read observes a value a previous round wrote — **the only τ that can make the temporal axis dirty via a plain carried read** |

The temporal rejections:

- **R2** — a per-source claim (`information: first-wins`) whose occurrence
  semantics the partition decomposition cannot guarantee (`discharge:
  ClaimStaging` when the claim is staged in the owner's source-begin pass).
- **R4** — a *carried* read on a base the same nest mutates: the value the read
  observes was written in an earlier round, so rounds are ordered by a semantic
  dependence.
- **R5** — the round-separation *shadow-endpoint* conflict: the base is read
  across endpoints (so a snapshot would discharge the read) **and** a pair-phase
  mutating write lands on the shadow's endpoint (so the snapshot is not a valid
  frozen value).

Value source σ (`ValueSource { None, Live, Snapshot }`) is assigned *after*
round separation is established and is **never an input to τ** — this ordering
is what makes the shadow *hide* R4 (the read is discharged) instead of
contradicting it.

**Round-separation bases** are the mechanism: a base read on one endpoint
region and written on the other needs a frozen round-start snapshot, because a
sibling partition may be concurrently writing it.  Eligibility is explicit:

```
*  - same base has both an R effect and a mutating effect;
*  - the mutating effects on the base are single-region (all U or all V,
*    none G/D) -- mixed U+V writes on one base is composition C;
*  - the reads include the opposite region (cross-endpoint);
*  - the element type is uniform and i32/double.
```

and the base record carries `WritesV` (dest-owned vs source-owned) plus the set
of cross-endpoint loads redirected to the snapshot (`RoundSepBase::CrossReads`).

---

## 4. Temporal axis — code and logic

**Code** (`graph_frontier_lowering.cpp`).  τ is computed by a pure function of
the read, the write, the write's phase segment and the round domain:

```cpp
static Temporal relateReadWrite(const Effect &Rd, const Effect &M,
                                PhaseSegment MSeg, DomainKind RoundDomain)
{
    if (!effectIsMutating(M) || M.Kind == EffectKind::Activate)
        return Temporal::Independent;
    if (M.Base != Rd.Base) return Temporal::Independent;
    if (M.Reg == Rd.Reg)   return Temporal::SameRoundRead;
    if (Rd.Reg == Region::U && M.Reg == Region::V) {
        if (MSeg != PhaseSegment::Pair)
            return Temporal::SameRoundRead;      /* per-source write: sequenced */
        return RoundDomain == DomainKind::Frontier ? Temporal::PreviousRoundRead
                                                   : Temporal::Carried;
    }
    if (Rd.Reg == Region::V && M.Reg == Region::U)
        return Temporal::SameRoundRead;          /* staged owner control */
    return Temporal::Independent;
}
```

`deriveAllTemporal` relates every read against every mutating primitive and
keeps the worst rank; `deriveExprFacts` / the witness builder then turn the
facts into witnesses:

```cpp
/* R2 -- temporal: per-source claim occurrence. */
const DischargeKind D = perSourceClaims(Info).empty() ? DischargeKind::None
                                                      : DischargeKind::ClaimStaging;
witness(WitnessAxis::Temporal, RejectionId::R2, ...);

/* R4 -- temporal: carried read on a mutated base. */
if (S.HasCarriedOnMut) {
    const DischargeKind D = Priv ? DischargeKind::Privatization : DischargeKind::None;
    witness(WitnessAxis::Temporal, RejectionId::R4, ...);
}

/* R5 -- temporal: shadow-endpoint write breaks round separation. */
if (roundSepEndpointConflict(Info))
    witness(WitnessAxis::Temporal, RejectionId::R5, ...);
```

`roundSepEndpointConflict` is exactly the predicate stated in §3:

```cpp
if (Info.RoundSepBases.empty()) return false;
const Region Opp = RS.WritesV ? Region::U : Region::V;
forEachExprEffect(Info, [&](const Effect &E, PhaseSegment Seg) {
    if (Seg != PhaseSegment::Pair || !effectIsMutating(E)) return;
    if (E.Base == RS.Base && E.Reg == Opp) Conflict = true;
});
```

**Logic.**  Consequences that matter operationally:

- A **carried** read only appears with an all-vertices round domain
  (`DomainKind::AllVertices`); the frontier-gated domain yields
  `PreviousRoundRead` — so `carried_read_state.graph` (all-vertices driver) makes
  the temporal axis dirty (`R_T=1`) while `kcore`-style frontier peels do not.
- A cross-endpoint read that *does* get a snapshot (`VSource=Snapshot`) is
  discharged and **hides R4** — verified live by `shadow_snapshot.graph`, whose
  certificate shows the R2 claim witness and *no* R4, with the snapshot as the
  discharge.
- R5’s two clauses cannot both be satisfied by a DSL fixture: the
  RS-eligibility rule requires the base's mutations to be single-region, while
  the conflict needs a pair-phase mutating write in the *opposite* region.  The
  predicate is still evaluated per loop and the inventory gate
  (`verify/check_census.sh`) asserts R5 never appears spuriously; if a future
  shape reaches it, the gate fails and demands the fixture.

---

## 5. Spatial axis — theory

**Theory.**  Spatial rejections describe *ownership* failures:

- **R1** — a data-region write (`A[deg[u]] = 1`): the CleanCut owner table is
  keyed by destination vertex id, so a data-valued house cannot be partitioned;
  dischargeable by **privatization** when the private-slot proof applies.
- **R3** — an index with `Top` provenance (`A[u+v] = 1`): the origin cannot be
  named, so no partition map is sound.  No discharge.
- **R6** — the *same base* written through both regions (`arr[u]` and `arr[v]`):
  dual ownership with intersecting write sets.  This is the one spatial
  rejection that is **representable**: the constraint is a *mutual exclusion*
  between the two ownership regions, and a realization may impose a
  deterministic direction on it (labeled `RealizationOrder`, never semantic
  precedence).  Dischargeable by **privatization** when a private copy + fold is
  proven.
- **R7** — a frontier append without the wired envelope (`count[gid] =
  count[gid]+1` / `roaring_bitmap_add` in a nest whose envelope pointers are not
  wired): the engine could not commit the round.  No discharge.

Two more spatial facts are **not** R1–R7 and must never be reported as such:
`HasDataWrite`, and the *implementation-level* DualOwner cross-phase check
(“cross-phase or empty-base U+V”), which is an R8-family guard.

**Code.**  Witness construction mirrors the theory one-to-one:

```cpp
/* R1 -- spatial: data-region write (discharge: privatization proof). */
witness(WitnessAxis::Spatial, RejectionId::R1, nullptr, Region::D, Region::D, ...);

/* R3 -- spatial: unknown index provenance (Top). */
if (S.MutTop)
    witness(WitnessAxis::Spatial, RejectionId::R3, nullptr, Region::Top, Region::Top, ...);

/* R6 -- spatial: same-base U+V dual ownership. */
if (!Disjoint) {
    const DischargeKind D = Priv ? DischargeKind::Privatization : DischargeKind::None;
    witness(WitnessAxis::Spatial, RejectionId::R6, Shared, Region::U, Region::V, ...);
}

/* R7 -- spatial: frontier append without the wired envelope. */
if (Info.HasFrontierAppend && !envelopeWired(Info))
    witness(WitnessAxis::Spatial, RejectionId::R7, nullptr, Region::V, Region::V, ...);
```

**Logic.**  `witness(axis, id, base, srcRegion, sinkRegion, ...)` fills the
`InterferenceWitness` record (Constraint kind, access modes, phase segments,
occurrence scopes, original program order, symbol relation, discharge) and
appends it to the axis list of the certificate.  `supportedByAlgebra` evaluates
**all** checks without short-circuiting (`RecordRefusal`), so the certificate is
complete: both axes are always populated, and the *first* refusal keeps the
historical reason string for compatibility.

---

## 6. When is a rejection realizable?  G_S and G_T

**Theory.**  A witness that is not discharged must be **consumed** by a graph
constraint; a constraint either expresses real precedence (semantic edge, must
carry a witness id) or an imposed deterministic order (realization edge, no
semantic claim).  Representability per rejection:

| witness | constraint | kind |
|---|---|---|
| R6 | one region-level edge `U → V` (`buildTemplates`) | `MutualExclusion`, realized as `RealizationOrder` |
| R4 | the round sequence itself (no graph edge needed) | — |
| R2 (ClaimStaging) | the owner's source-begin pass | — |
| R1/R3/R7 | **no representation** → template invalid | — |

**Code.**

```cpp
struct SpatialEdgeTemplate {
    uint32_t From = 0, To = 0;
    uint32_t WitnessId = 0;
    ConstraintKind SemanticKind = ConstraintKind::MutualExclusion;
    bool RealizationOrder = false;
    InstanceRelation Relation;
};
struct SpatialGraphTemplate { SmallVector<SpatialNodeTemplate,4> Nodes;
                              SmallVector<SpatialEdgeTemplate,8> Edges;
                              bool Valid = false; std::string InvalidReason; };
```

The R6 branch builds nodes per owner region and the tagged edge; the R1/R3/R7
paths leave `Valid=false` with a specific `InvalidReason` (e.g. *"spatial
witness R3 has no partition representation"*).  Runtime validation mirrors the
compile-time rule:

```c
if (!(E->flags & (SGPL_DAG_REL_FLAG_SEMANTIC | SGPL_DAG_REL_FLAG_REALIZATION)))
    return SGPL_DAG_ERR_INVALID;
if ((E->flags & SGPL_DAG_REL_FLAG_SEMANTIC) && E->witness_id == 0)
    return SGPL_DAG_ERR_INVALID;   /* no semantic edge without a witness */
```

and the compiler-side consumption invariant:

```cpp
/* A witness-derived constraint is realized as a tagged RealizationOrder edge
 * (semantic precedence is never claimed for a mutual-exclusion witness), so
 * consumption accepts it. */
if (E.WitnessId == W.Id ||
    (E.RealizationOrder && W.Rejection == RejectionId::R6)) Consumed = true;
```

**Logic.**  `witnessesConsumed` runs before emission: an unresolved witness with
no constraint and no schedule-level order proof fails the emission **closed**
(with the witness id in the reason), so “admitted but unconsumed” is impossible
— the compiler prints `MISMATCH admitted with unresolved witness/guard` if it
ever happens.

---

## 7. Schedule selection (the gate split)

**Theory.**  The four cases of the theorem:

| axes | schedule | realization |
|---|---|---|
| `¬R_S ∧ ¬R_T` | `nested` | engine partition dispatch + round sequence |
| `¬R_S ∧ R_T` (C1) | `spatial-dag` | partitions concurrent; the round sequence carries the temporal order |
| `R_S ∧ ¬R_T` (C2) | `temporal-dag` | units = phases; the spatial constraint is preserved *between* them; each unit internally parallel |
| `R_S ∧ R_T`, R8, unmodelable, failed emit | `serial` (+ reason) | no theorem-licensed route, or an explicit implementation failure |

**Code** (`selectSchedule`):

```cpp
/* C1: spatial clean, temporal dirty. */
Ch.Kind = ScheduleKind::SpatialDag;
Ch.Reason = "spatial partitions concurrent; temporal order preserved by the round sequence";
return Ch;

/* C2: spatial dirty, temporal clean -- only when every constraint edge is a
 * tagged realization order (never semantic precedence). */
for (const SpatialEdgeTemplate &E : Info.SpatialTemplate.Edges)
    if (!E.RealizationOrder) { Ch.ImplementationFailure = true; return Ch; }
Ch.Kind = ScheduleKind::TemporalDag;
Ch.Reason = "spatial constraints staged (U before V); temporal axis clean";
return Ch;
```

**Logic.**  `Rewritable = Modelable && schedule != serial && emitted` — this is
the *gate split*: the old conjunction `¬(R_S ∨ R_T)` no longer force-serializes
a one-axis-dirty nest.  Emission is attempted only for non-serial schedules;
every failure path calls `markSequential` (semantic) or `markImplSerial`
(implementation), so the certificate's `impl_failure` bit is always truthful.

---

## 8. Realization on the CPU

### 8.1 `nested` / `spatial-dag`: the round lifecycle

**Theory.**  A round is the unit of the temporal order: round *i+1* may not
observe round *i*'s mutations except through the frozen frontier/snapshot, so
each round must be dispatched and **joined** before the next begins.  Within a
round, partitions are independent by ownership.

**Code** (`autograph_frontier_execute`, skeleton):

```c
if (ctx->run_round_begin) { sgpl_exec_round_begin_ops(ctx); sgpl_exec_snapshot_ops(ctx); }
if (cov_begin) sgpl_exec_source_coverage(ctx, /*begin=*/1);
sgpl_exec_step_dispatch(ctx, meta);            /* partitions: concurrent + join */
if (cov_end)   sgpl_exec_source_coverage(ctx, /*begin=*/0);
/* deterministic combine: partition partials fold in ascending partition order */
for (p = 0; p < meta->partition_count; ++p) { ... op->combine(op->state, ctx); }
ctx->next_size = ctx->initial_next_size + (append_head ? atomic_load(append_head) : 0);
if (ctx->run_round_end) sgpl_exec_round_end_ops(ctx);
```

**Logic.**  The join inside the dispatch is the temporal order guarantee for
`spatial-dag`; the combine is order-deterministic (associativity is the law,
permutation invariance is never assumed).

### 8.2 `temporal-dag` (C2): the staged dual owner

**Theory.**  In the R6 case the `RealizationOrder` edge says: all U-owned work
before all V-owned work.  That is a sound superset of the mutual exclusion the
witness needs, and it reproduces the serial per-element order for the shapes the
certificate admits (see §11).  Each phase keeps its full internal parallelism.

**Code** (`autotuner_runtime.c`):

```c
int32_t autograph_frontier_staged(void *graph_ptr, sgpl_exec_ctx *owner,
                                  sgpl_exec_ctx *a, sgpl_exec_ctx *b) {
  if (a->run_round_begin || a->run_round_end || b->run_round_begin || b->run_round_end)
    abort();                                  /* children are non-owning */
  if (owner->run_round_begin) { sgpl_exec_round_begin_ops(owner); sgpl_exec_snapshot_ops(owner); }
  if (sgpl_exec_has_cap(owner, SGPL_OP_SOURCE_BEGIN)) sgpl_exec_source_coverage(owner, 1);
  (void)autograph_frontier_execute(graph_ptr, a);      /* U phase: completes */
  (void)autograph_frontier_execute(graph_ptr, b);      /* V phase: then starts */
  if (sgpl_exec_has_cap(owner, SGPL_OP_SOURCE_END)) sgpl_exec_source_coverage(owner, 0);
  owner->next_size = owner->initial_next_size + appended;
  if (owner->run_round_end) sgpl_exec_round_end_ops(owner);
  return owner->next_size;
}
```

The emitter chooses it from the template:

```cpp
Value *NewSize = EB.CreateCall(Fj, {GraphArg, CtxOwner, CtxU, CtxV});
/*   Fj = Concurrent ? "autograph_frontier_fork_join" : "autograph_frontier_staged"
     Concurrent = Info.SpatialTemplate.Edges.empty() */
```

and declares the schedule to the runtime for both children:

```cpp
EB.CreateCall(SetAxes, {C, ConstantInt::get(I32, SpatialPar ? 1 : -1),
                           ConstantInt::get(I32, TemporalPar ? 1 : -1)});
```

**Logic.**  Order = the two sequential `execute` calls (structural, not timing);
parallelism = each child's own partition dispatch, which is declared
`spatial=1` and therefore takes the ready-work DAG dispatch.

### 8.3 The ready-work DAG scheduler

**Theory.**  Nodes are units of work (partitions, temporal units, or phase
partitions); edges are *witness-backed* dependencies; scheduling is
ready-set-driven with a bounded worker budget; every failure is fail-closed
(cancel new dispatch, drain active, return the error).

**Code** (core shapes):

```c
struct sgpl_dag_run { const sgpl_dag_template *tmpl; void *state; int32_t node_count;
  int32_t worker_budget; int32_t *remaining; unsigned char *completed;
  int32_t *ready; int32_t ready_head, ready_tail, ready_count;
  pthread_mutex_t lock; pthread_cond_t cv; int32_t active, completed_count, failed,
  peak_active, grants_multi, grants_serial, max_share; int64_t sum_weight; };

static void dag_complete(sgpl_dag_run *R, int32_t node, int32_t rc) {   /* under lock */
  R->active--;
  if (T->node_weights) { int32_t w = T->node_weights[node]; if (w > 0) R->sum_weight -= w; }
  R->completed[node] = 1; R->completed_count++;
  if (rc != 0) R->failed = 1;                       /* cancel new dispatch */
  else for (E : relations from node) if (--R->remaining[E->sink] == 0) dag_push(R, E->sink);
  pthread_cond_broadcast(&R->cv);
}
```

Cycle detection is `completed_count != node_count` after the ready set drains
(`SGPL_DAG_ERR_CYCLE`); a failing node yields `SGPL_DAG_ERR_NODE` with the
active nodes awaited.  Temporal units are wrapped without duplicating lifecycle:

```c
static int32_t sgpl_dag_temporal_node(void *state, int64_t instance) {
  const sgpl_temporal_unit *unit = &((const sgpl_temporal_unit *)state)[instance];
  if (!unit->graph || !unit->ctx) return SGPL_DAG_ERR_NODE;
  (void)autograph_frontier_execute(unit->graph, unit->ctx);
  return SGPL_DAG_OK;
}
/* sgpl_exec_dag_temporal_chain: unit i -> i+1 edge, witness_id = 1
 * ("round-carried temporal dependence"), SEMANTIC flag */
```

**Logic.**  The scheduler is deliberately policy-free: it never inspects the
program, only the template.  All semantic content is in the edges' flags and
witness ids, which is what makes invariant (2) hold at execution time.

---

## 9. The per-step schedule ABI and the dispatch rule

**Theory.**  The compiled schedule must reach the runtime *per stage*, not via a
global switch, because two stages of the same program can have different
licenses.  A declaration therefore carries two values: `spatial` and `temporal`
(`1` concurrent, `-1` held in order, `0` undeclared).

**Code.**

```c
/* autotuner_runtime.h */
  int32_t dag_spatial;      /* 1 concurrent, -1 ordered, 0 undeclared */
  int32_t dag_temporal;
void autograph_exec_ctx_set_dag_axes(sgpl_exec_ctx *ctx, int32_t spatial, int32_t temporal);
int32_t sgpl_ctx_dag_spatial(const sgpl_exec_ctx *ctx);

/* autotuner_runtime.c: the dispatch follows the declaration */
static int sgpl_dispatch_uses_dag(const sgpl_exec_ctx *ctx) {
  const char *env = getenv("SGPL_DAG_SPATIAL");
  if (env && env[0] == '0' && env[1] == '\0') return 0;  /* kill switch */
  if (sgpl_ctx_dag_spatial(ctx) > 0) return 1;           /* declared concurrent */
  return env != NULL && env[0] == '1' && env[1] == '\0'; /* forced for legacy */
}
```

**Logic — including the calibration rule.**  An engine step site is planned
through the TDG machinery; while its profile is not yet stable
(`sgpl_step_site_ready` false) the historical code forced a single-threaded
calibration pass, which for a **one-shot nest** swallowed the only dispatch and
made the declaration nominal.  The declared schedule is now authoritative:

```c
if (!sgpl_step_site_ready(step_id)) {
    if (!force_calibration && sgpl_ctx_dag_spatial(ctx) > 0) {
        if (sgpl_dispatch_uses_dag(ctx)) sgpl_exec_dag_spatial(ctx, partitions, ...);
        else parallel_for_runtime(0, partitions, 1, sgpl_exec_partition_body, ctx, 0, 0);
        width = min(configured, partitions, ledger_grant);
        sgpl_exec_step_record_sample(step_id, partitions, elapsed, width);  /* × width */
        return;
    }
    sgpl_set_desired_threads(1, 1);   /* SGPL_TDG_CALIBRATE_DECLARED=1 */
    ...
}
```

The calibration sample is still recorded, scaled by the width used (a
conservative serial-equivalent upper bound, so a later plan errs toward more
width, never less).

---

## 10. Budget policy (§15)

**Theory.**  A node's grant must cover the *ready* set (not just the active set),
must never exceed the caller's ledger, must be bounded by a work estimate in a
way that an estimate error can only *under*-grant, and denial must degrade to
serial instead of blocking.

**Code.**

```c
static int32_t dag_dispatch_budget(sgpl_dag_run *R, int32_t node) {
  int32_t denom = R->active + R->ready_count;         /* ready-set reservation */
  if (denom < 1) denom = 1;
  int32_t share = R->worker_budget / denom;
  if (share < 1) share = 1;
  if (R->tmpl->node_weights && R->sum_weight > 0) {   /* ceiling only */
    int64_t w = R->tmpl->node_weights[node];
    if (w > 0) { int64_t cap = ((int64_t)R->worker_budget * w) / R->sum_weight;
                 if (cap < 1) cap = 1;
                 if ((int64_t)share > cap) share = (int32_t)cap; }
  }
  return share;
}
/* entry clamp: never exceed the caller's grant */
int32_t avail = sgpl_current_thread_budget();
if (avail > 0 && worker_budget > avail) worker_budget = avail;
/* the work estimate is measured, not modelled */
tmpl.node_weights = (meta->src_pair_count && meta->partition_count == partitions)
                        ? meta->src_pair_count : NULL;
```

**Logic.**  Occupancy statistics: `grants_multi/grants_serial`, `max_share`,
`peak_active`; `SGPL_DAG_DEBUG=1` prints
`nodes/rels/budget/peak/max_share/grants=multi/serial/weighted/completed/rc`.
The TDG cost model's known mismatch (model light=7/heavy=7 vs measured
light=1/heavy=13) is bypassed on this path because the weights come from the
partition's own measured pair count.

---

## 11. Scope guard: order-sensitivity is excluded by the certificate

**Theory.**  “All U before all V” reproduces the serial per-element order only
for order-insensitive updates.  A shape with non-commuting cross-region updates
(`A[u] = A[u]*2; A[v] = A[v]+1`) would expose the difference.

**Code/evidence.**  Compiling exactly that shape
(`verify/cases/parallel/r6_order_sensitive.graph`) yields:

```
#1 R4 temporal ...          (the cross-region read of the mutated base)
#2 R6 spatial base=A ...    (the same-base dual write)
hdr=... R_S=1 R_T=1 NI_S=0 NI_T=0 impl_guard=...
schedule=serial emitted=1 impl_failure=0 reason=both axes refused (R_S and R_T):
    no theorem-licensed parallel route
```

**Logic.**  The staged realization is therefore *never* applied to an
order-sensitive shape — the theorem refuses parallelism altogether
(`impl_failure=0`, a semantic serial), and the answer equals the unrewritten
build at 1/4/8.  The admitted R6 shapes are write-only (`A[u]=1; A[v]=1`) or
carry a single commutative operator, for which the imposed direction is
irrelevant.

---

## 12. Metadata, consumers, and the failure taxonomy

**Theory.**  Downstream passes must learn *why* a region is engine-owned and
must not re-outline it; and an implementation failure must be distinguishable
from a theorem refusal.

**Code.**

```cpp
static void markDagOwned(Loop *L, StringRef Axes, StringRef Spatial, StringRef Temporal) {
    ... T->setMetadata("sgpl.frontier.dag.axes",    Axes       );
        T->setMetadata("sgpl.frontier.dag.owner",   "engine"   );
        T->setMetadata("sgpl.frontier.dag.spatial", Spatial    );
        T->setMetadata("sgpl.frontier.dag.temporal",Temporal   );
}
static void markImplSerial(Loop *L) { ... "sgpl.frontier.impl.serial" = "unemitted" ... }
```

`markSequential` marks `sgpl.frontier.nested.sequential`, reserved for R8 and
emit/analysis failure; `pdg.cpp` reads the value of `sgpl.frontier.dag.owner`
(and the axis values) and reports the region as engine-owned rather than
re-deriving dependence; `parallel_loop_outline.cpp` never outlines an
engine-owned region.

**Logic — the taxonomy.**  Semantics: R1..R7 → the four schedule cases.
Implementation: R8, unmodelability, cyclic templates, missing proofs,
unsupported symbolic relations, failed descriptor validation, failed emission,
budget/alloc failures → serial with `impl_failure=1` and an explicit reason.
The two never mix: the certificate prints both, the markers separate them, and
`SGPL_FRONTIER_STRICT`-style postconditions check the implication in both
directions (`admitted ⇒ every witness consumed`; `refused ⇒ ∃ witness/guard`).

---

## 13. Evidence (CPU)

| check | result |
|---|---|
| `verify/run.sh parallel` | 67 PASS / 0 FAIL (adds `r3_unknown_provenance`, `r7_append_unwired`, `shadow_snapshot`, `dual_same_base`, `r6_order_sensitive`, `carried_read_state`) |
| certificate census + inventory gate | 159 fixtures; R1×2 (privatized), R2×9 (claim-staged), R3×1, R4×2 (none → `spatial-dag`, privatized), R6×3 (none → staged, privatized), R7×2, **R5×0**, both axis splits, every schedule kind |
| `test/run_exec_engine_tests.sh` | exec engine 0 failures ×3; **dag scheduler 24/24** (order, cancellation, cycle, self-edge, witness rules, unsupported relation, no-flag edge, partial-DAG overlap with rendezvous, budget bounds, weight ceiling, nested clamp, temporal-unit validation) |
| TSan (`setarch -R`, ASLR off) | **0 races** in both suites |
| direction A measured | `carried_read_state`: 16 partitions via the ready-work scheduler, **peak=4**, `== serial` at t=1/4/8 and p=1/3/4; kill switch keeps the answer |
| direction B measured | `dual_same_base_big`: two ordered unit runs (peak=3/4), `== serial`; `dual_same_base`: peak=1, grants=2/3 |
| order-sensitivity | `r6_order_sensitive`: semantic serial, `== serial` (local + pod) |

## 14. Annex — designed, not implemented

- **Per-partition-pair constraints for R6.**  Today the constraint is one
  region-level edge (`U → V`).  The refinement: nodes = the `2P` phase
  partitions, edges = the conflicting pairs (`U_p → V_p`, mutual exclusion
  realized as a deterministic order), all dispatched through the one ready-work
  DAG, so non-conflicting partitions of both phases overlap.  It is a
  *performance* refinement of `ConstraintKind::MutualExclusion`; held pending
  the plan's §8 wording.
- **A scheduler-facing budget API.**  Reservation semantics are implemented
  inside the DAG run; no external reserve/release API was extracted from
  `parallel_runtime`.
- **`sgpl_exec_ctx` growth.**  The per-step declaration added two trailing
  fields; no existing field or lifecycle meaning changed.
