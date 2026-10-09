# Effect system — implementation report (CPU path, all axes)

Theory first, then the code for exactly that theory. Every code block is verbatim
from the tree (trimmed with `…`); where an algorithm is long I give faithful
pseudocode and say so. Anchors are `file:line` at the head of each block.

Names: there is no `sgpl_step_info` in this tree. The compile-time carrier is
`NeighborLoopInfo`, the proof object is `AxisCertificate`, the runtime carriers are
`AutoGraphMeta` and `sgpl_exec_ctx`, and the step identity is
`sgpl_exec_ctx.step_id`.

---

## 1. Theory

The system answers one question per candidate nest: *can this step run as
designed without a race; if not, which algebraic obligation is unmet?* It answers
along **two axes** — spatial (who owns the memory being written) and temporal
(which round may observe what) — and the answer is a certificate, not a hint.

**Witnesses (R1–R7).** Each refusal is a witness record: the obligation, its axis,
the constraint kind, both access regions/segments, and a **discharge** (the proof
artefact that clears it).

| id | axis | obligation | discharged by |
|----|------|-----------|----------------|
| R1 | spatial | data-derived write has no owner | privatization |
| R2 | temporal | per-source claim (occurrence preservation) | claim staging |
| R3 | spatial | unknown index provenance (`Top`) | never |
| R4 | temporal | carried read observes a base mutated across the round | privatization |
| R5 | temporal | pair-phase write at the shadow read endpoint | never |
| R6 | spatial | same-base `U`+`V` dual ownership | privatization |
| R7 | spatial | frontier append without a wired envelope | never |

**Axis verdicts.** `R_S` = ∃ spatial witness with `DischargedBy == None` (i.e.
`R1 ∨ R3 ∨ R6 ∨ R7`); `R_T` = the same over temporal (`R2 ∨ R4 ∨ R5`); the
non-interference facts are `NI_S = ¬R_S`, `NI_T = ¬R_T`. **R8** is outside the
lattice: a guard for realizations the lattice cannot model (dual-owner, reduction,
source-reduction) — it says the implementation lacks machinery, not that the
program is racy. **Verdict:** supported iff `¬(R_S ∨ R_T ∨ R8)`.

What a clean verdict buys: every interference obligation is either absent or
*discharged by a concrete artefact* (a privatization proof, claim staging, a
snapshot). The single producer of that certificate is one function, so no other
pass can invent a weaker verdict; a loop that fails is marked sequential, not
parallelized with a risk.

**Relation layer (RAW/WAR/WAW).** For every ordered pair of accesses to one base
where at least one mutates, the system records the kind and the discharge. Those
relations are what the axes' schedules are built from: discharged ones drop out;
undischarged cross-segment ones become *ordering edges*; a semantic edge must
point back to a witness id or it is rejected at runtime.

**Realizations (what the certificate selects).** The structural facts pick one
name: `privatized`, `reduction`, `source-red`, `dual-owner`, `dest-owner`,
`source-owner` — and the emission path is chosen from the facts, never the name.

---

## 2. The code, in the same order

### 2.1 Witness and certificate types

`graph_frontier_lowering.cpp:338-352` — the witness vocabulary:

```cpp
enum class WitnessAxis : uint8_t { Spatial = 0, Temporal };
enum class RejectionId : uint8_t { R1 = 1, R2, R3, R4, R5, R6, R7 };
```

`graph_frontier_lowering.cpp:390-411` — a witness record:

```cpp
struct InterferenceWitness {
    uint32_t Id = 0;
    RejectionId Rejection = RejectionId::R1;
    WitnessAxis Axis = WitnessAxis::Spatial;
    ConstraintKind Constraint = ConstraintKind::MutualExclusion;
    const Value *Base = nullptr;
    AccessMode SourceAccess = AccessMode::Read;
    AccessMode SinkAccess = AccessMode::Write;
    Region SourceRegion = Region::Bottom;
    Region SinkRegion = Region::Bottom;
    PhaseSegment SourceSegment = PhaseSegment::None;
    PhaseSegment SinkSegment = PhaseSegment::None;
    OccurrenceScope SourceScope = OccurrenceScope::PerPair;
    OccurrenceScope SinkScope = OccurrenceScope::PerPair;
    Instruction *SourceOrigin = nullptr;
    Instruction *SinkOrigin = nullptr;
    unsigned OriginalOrder = 0;
    InstanceRelation Relation;
    DischargeKind DischargedBy = DischargeKind::None;
    std::string RefusalReason;
};
```

`graph_frontier_lowering.cpp:492-502` — the certificate (note: both witness lists
are kept, and both axis verdicts are stored):

```cpp
struct AxisCertificate {
    SmallVector<InterferenceWitness, 8> Spatial;
    SmallVector<InterferenceWitness, 8> Temporal;
    bool RS = false;
    bool RT = false;
    bool NIS = false;
    bool NIT = false;
    bool R8 = false;
    std::string ImplementationReason;
};
```

### 2.2 The gate: single producer, verdict computed from discharges

`graph_frontier_lowering.cpp:5044-5106`:

```cpp
static bool supportedByAlgebra(NeighborLoopInfo &Info, const EffectSummary &S,
                               bool Priv, std::string &reason)
{
    AxisCertificate &Cert = Info.Cert;
    /* the gate is the single producer of the theorem certificate … */
    auto refresh = [&Cert]() {
        Cert.RS = false; Cert.RT = false;
        for (const InterferenceWitness &W : Cert.Spatial)
            if (W.DischargedBy == DischargeKind::None) Cert.RS = true;
        for (const InterferenceWitness &W : Cert.Temporal)
            if (W.DischargedBy == DischargeKind::None) Cert.RT = true;
        Cert.NIS = !Cert.RS; Cert.NIT = !Cert.RT;
    };
    auto admit = [&]() -> bool {
        refresh();
        if (Cert.RS || Cert.RT || Cert.R8)
            fprintf(stderr, "[frontier-cert] MISMATCH admitted with unresolved witness/guard\n");
        return true;
    };
    auto refuse = [&](const char *Why) -> bool {
        refresh();
        if (!(Cert.RS || Cert.RT || Cert.R8))
            fprintf(stderr, "[frontier-cert] MISMATCH refused without witness/guard: %s\n", Why);
        reason = Why;
        return false;
    };
    auto witness = [&](WitnessAxis Axis, RejectionId R, const Value *B,
                       Region SrcReg, Region SinkReg, AccessMode SrcAcc,
                       AccessMode SinkAcc, PhaseSegment SrcSeg, PhaseSegment SinkSeg,
                       InstanceRelation Rel, DischargeKind D, const char *Why) {
        InterferenceWitness W;
        W.Rejection = R; W.Axis = Axis;
        W.Constraint = Axis == WitnessAxis::Spatial ? ConstraintKind::MutualExclusion
                                                    : ConstraintKind::Precedence;
        W.Base = B; W.SourceRegion = SrcReg; W.SinkRegion = SinkReg;
        W.SourceAccess = SrcAcc; W.SinkAccess = SinkAcc;
        W.SourceSegment = SrcSeg; W.SinkSegment = SinkSeg;
        W.Relation = Rel; W.DischargedBy = D; W.RefusalReason = Why;
        appendWitness(Cert, std::move(W));
    };
```

Note what the two lambdas *enforce on themselves*: `admit` screams if it is about
to accept with an unresolved witness, `refuse` screams if it refuses with none —
the certificate cannot drift away from the verdict.

### 2.3 The witnesses themselves

`graph_frontier_lowering.cpp:5124-5238` (spatial and temporal interleaved, as in
the source — trimmed to the decisions):

```cpp
/* R1 -- spatial: a data-derived write has no owner. */
if (Info.HasDataWrite) {
    const DischargeKind D = Priv ? DischargeKind::Privatization : DischargeKind::None;
    witness(WitnessAxis::Spatial, RejectionId::R1, nullptr, Region::D, Region::D,
            AccessMode::Write, AccessMode::Write, …, D,
            "data-derived write without a privatization proof");
    if (!Priv) return refuse("data-derived write without a privatization proof");
}
/* R2 -- temporal: per-source claim, occurrence preservation. */
if (hasPerSourceClaim(Info)) {
    const DischargeKind D = perSourceClaims(Info).empty() ? DischargeKind::None
                                                          : DischargeKind::ClaimStaging;
    witness(WitnessAxis::Temporal, RejectionId::R2, …,
            "per-source claim (occurrence preservation)");
    if (D == DischargeKind::None) return refuse(…);
}
/* R3 -- spatial: unknown index provenance (Top). */
if (S.MutTop) {
    witness(WitnessAxis::Spatial, RejectionId::R3, nullptr, Region::Top, Region::Top, …,
            DischargeKind::None, "unknown index provenance (Top)");
    return refuse("unknown index provenance (Top)");
}
/* R4 -- temporal: a carried read observes a base mutated across the round. */
if (S.HasCarriedOnMut) {
    const DischargeKind D = Priv ? DischargeKind::Privatization : DischargeKind::None;
    witness(WitnessAxis::Temporal, RejectionId::R4, …, PhaseSegment::Preamble,
            PhaseSegment::Preamble, prevInstance(), D, "carried read on a mutated base");
    if (!Priv) return refuse("carried read on a mutated base");
}
/* R5 -- temporal: shadow-endpoint write breaks round separation. */
if (roundSepEndpointConflict(Info)) {
    witness(WitnessAxis::Temporal, RejectionId::R5, …, PhaseSegment::Pair,
            PhaseSegment::Pair, sameInstance(), DischargeKind::None,
            "pair-phase write at the shadow read endpoint");
    return refuse("pair-phase write at the shadow read endpoint");
}
/* R6 -- spatial: same-base U+V dual ownership … */
if (PairU && PairV && !S.MutG) {
    bool Disjoint = true;
    for (const Value *B : S.BaseU) if (S.BaseV.count(B)) Disjoint = false;
    if (!(Disjoint && … && Info.RoundSepBases.empty())) {
        if (!Disjoint) {
            const DischargeKind D = Priv ? DischargeKind::Privatization : DischargeKind::None;
            witness(WitnessAxis::Spatial, RejectionId::R6, Shared, Region::U, Region::V,
                    AccessMode::Write, AccessMode::Write, PhaseSegment::Pair,
                    PhaseSegment::Pair, sameInstance(), D,
                    "same-base U+V dual ownership conflict");
        }
        if (Priv) return admit();
        if (Disjoint) { Cert.R8 = true;
            Cert.ImplementationReason = "dual-owner realization: cross-phase or empty-base U+V"; }
        return refuse("same-base or cross-phase U+V without privatization");
    }
}
/* R7 -- spatial: frontier append without the wired envelope. */
if (Info.HasFrontierAppend && !envelopeWired(Info)) {
    witness(WitnessAxis::Spatial, RejectionId::R7, nullptr, Region::V, Region::V, …,
            DischargeKind::None, "frontier append without a wired envelope");
    return refuse("frontier append without a wired envelope");
}
/* R8 (implementation guard, not a witness): reduction realizations */
if (Info.ReducePtr && !Priv) {
    if (Info.AccConsumeStore) {
        if (!(Info.AccResetSeen && Info.ReduceOp != RedOp::None && !Info.HasDataWrite && …)) {
            Cert.R8 = true; Cert.ImplementationReason = "source-reduction realization";
            return refuse("source-reduction conditions not met");
        }
    } else if (!(S.MutG && S.HasUopG && !S.HasUnrecognizedG && !PairU && !PairV && …)) {
        Cert.R8 = true; Cert.ImplementationReason = "reduction realization";
        return refuse("reduction conditions not met");
    }
}
return admit();
```

### 2.4 The relation layer

`graph_frontier_lowering.cpp:2211-2256` — kind by mutation, discharge by proof:

```cpp
static void deriveAccessRelations(NeighborLoopInfo &Info)
{
    Info.AccessRelations.clear();
    SmallVector<std::pair<const Effect *, PhaseSegment>, 32> Prims;
    collectTreePrims(Info.Expr, Prims);
    for (size_t i = 0; i < Prims.size(); ++i) {
        const Effect *A = Prims[i].first;
        if (!A || !A->Base) continue;
        for (size_t j = i + 1; j < Prims.size(); ++j) {
            const Effect *B = Prims[j].first;
            if (!B || B->Base != A->Base) continue;
            const bool AMut = effectIsMutating(*A);
            const bool BMut = effectIsMutating(*B);
            if (!AMut && !BMut) continue;           /* read-read: no obligation */
            AccessRelation R;
            R.Source = A; R.Sink = B; R.Base = A->Base;
            R.SourceRegion = A->Reg; R.SinkRegion = B->Reg;
            R.SourceSegment = Prims[i].second; R.SinkSegment = Prims[j].second;
            if (AMut && BMut)      R.Kind = AccessRelKind::WAW;
            else if (AMut)         R.Kind = AccessRelKind::RAW;
            else                   R.Kind = AccessRelKind::WAR;
            const Effect *Reader = AMut ? B : A;
            if (Reader->VSource == ValueSource::Snapshot)
                R.DischargedBy = DischargeKind::Snapshot;
            else if (Info.UsePrivLayout)
                R.DischargedBy = DischargeKind::Privatization;
            else if ((AMut && A->Kind == EffectKind::Uop && A->Op != RedOp::None) ||
                     (BMut && B->Kind == EffectKind::Uop && B->Op != RedOp::None))
                R.DischargedBy = DischargeKind::AlgebraicFold;
            if (Info.AccessRelations.size() < 64)
                Info.AccessRelations.push_back(R);
        }
    }
}
```

### 2.5 What the certificate selects, and the two markers

`graph_frontier_lowering.cpp:5252-5272` — the facts that pick the realization:

```cpp
struct ExprFacts {
    bool MutU = false, MutV = false, MutG = false;
    bool IsDual = false, IsPriv = false, IsRed = false, IsSourceRed = false;
    bool CrossPhase = false;
    bool IsV = false; /* single stage writes V (destination-owned rows) */
};
…
    F.IsPriv      = Info.UsePrivLayout;
    F.IsRed       = Info.ReducePtr && !Info.AccConsumeStore && !F.IsPriv;
    F.IsSourceRed = Info.ReducePtr && Info.AccConsumeStore && !F.IsPriv;
    F.IsDual      = PairU && PairV && !F.IsPriv && !Info.ReducePtr;
    F.CrossPhase  = crossPhaseDataDep(Info, S.BaseU, S.BaseV);
    F.IsV         = PairV && !PairU;          /* single stage writes V */
```

`graph_frontier_lowering.cpp:1263-1280` (success marker) and `:1235-1256`
(terminal fallback), plus the value at `:6560-6572`:

```cpp
static void markDagOwned(Loop *L, llvm::StringRef Axes) {
    Loop *Top = L; while (Loop *P = Top->getParentLoop()) Top = P;
    std::vector<Loop *> Work{Top};
    while (!Work.empty()) {
        Loop *Cur = Work.back(); Work.pop_back();
        if (Instruction *T = Cur->getHeader()->getTerminator())
            T->setMetadata("sgpl.frontier.dag.axes",
                           MDNode::get(T->getContext(),
                                       MDString::get(T->getContext(), Axes)));
        for (Loop *Sub : Cur->getSubLoops()) Work.push_back(Sub);
    }
}
…
            std::string Axes;
            if (!Info.Cert.Spatial.empty())  Axes = "spatial";
            if (!Info.Cert.Temporal.empty()) Axes = Axes.empty() ? "temporal"
                                                                 : Axes + "+temporal";
            if (Axes.empty()) Axes = "spatial";
            markDagOwned(L, Axes);
```

```cpp
static void markSequential(Loop *L) {        /* sgpl.frontier.nested.sequential */
    … T->setMetadata("sgpl.frontier.nested.sequential",
                     MDNode::get(T->getContext(),
                                 MDString::get(T->getContext(), "sequential")));
}
```

Consumers: PDG classifies a DAG-owned nest `SEQUENTIAL` and leaves it inline
(`pdg.cpp:1390-1394`, `:3605-3618`); the outliner refuses it
(`parallel_loop_outline.cpp:171-174`). The refusal marker is reserved for R8 /
emit failure.

### 2.6 Templates: the certificate projected onto schedules

`graph_frontier_lowering.cpp:475-487`:

```cpp
struct SpatialGraphTemplate  { SmallVector<SpatialNodeTemplate, 4> Nodes;
                               SmallVector<SpatialEdgeTemplate, 8> Edges;
                               bool Valid = false; std::string InvalidReason; };
struct TemporalGraphTemplate { SmallVector<TemporalUnitTemplate, 4> Units;
                               SmallVector<TemporalEdgeTemplate, 8> Edges;
                               bool Valid = false; std::string InvalidReason; };
```

`graph_frontier_lowering.cpp:4907-5002` — spatial nodes are **owner regions
written in the pair phase**; temporal units are the enclosing loop instance,
and undis­charged cross-segment relations become *realization-order* edges:

```cpp
static void buildTemplates(NeighborLoopInfo &Info) {
    AxisCertificate &Cert = Info.Cert;
    SpatialGraphTemplate &SG = Info.SpatialTemplate;
    TemporalGraphTemplate &TG = Info.TemporalTemplate;
    SG = SpatialGraphTemplate(); TG = TemporalGraphTemplate();
    bool PairU = false, PairV = false;
    pairPhaseWriteRegions(Info, PairU, PairV);

    /* --- spatial: one node per owner region actually written in the pair
     * phase; partition counts are the runtime's decision (0 = all). */
    if (Cert.RS)      { SG.Valid = false; SG.InvalidReason = "spatial refusal"; }
    else if (Cert.R8) { SG.Valid = false; SG.InvalidReason = Cert.ImplementationReason; }
    else {
        uint32_t Next = 1;
        if (PairU) { SpatialNodeTemplate N; N.Id = Next++; N.Owned = Region::U; SG.Nodes.push_back(N); }
        if (PairV) { SpatialNodeTemplate N; N.Id = Next++; N.Owned = Region::V; SG.Nodes.push_back(N); }
        if (SG.Nodes.empty()) { … N.Owned = Region::Bottom; SG.Nodes.push_back(N); }
        SG.Valid = true;
    }

    /* --- temporal: one unit for the enclosing loop instance; precedence edges
     * only for order-sensitive relations that are not discharged. */
    if (Cert.RT)      { TG.Valid = false; TG.InvalidReason = "temporal refusal"; }
    else if (Cert.R8) { TG.Valid = false; TG.InvalidReason = Cert.ImplementationReason; }
    else {
        TemporalUnitTemplate U; U.Id = 1;
        U.EnclosingLoop = …; U.SpatialNodes = (uint32_t)SG.Nodes.size();
        TG.Units.push_back(U);
        for (const AccessRelation &R : Info.AccessRelations) {
            if (R.DischargedBy != DischargeKind::None) continue;
            if (R.SourceSegment == R.SinkSegment) continue; /* ordered by construction */
            TemporalEdgeTemplate E; E.From = 1; E.To = 1; E.WitnessId = 0;
            E.RealizationOrder = true;              /* serial order, no witness */
            TG.Edges.push_back(E);
        }
        TG.Valid = true;
```

The runtime re-checks that rule: `autotuner_runtime.c:4730-4750`:

```cpp
    for (i = 0; i < T->relation_count; i++) {
        const sgpl_dag_relation_desc *E = &T->relations[i];
        if (E->kind != SGPL_DAG_REL_PRECEDENCE) return SGPL_DAG_ERR_INVALID;
        if (E->source_node < 0 || E->source_node >= T->node_count ||
            E->sink_node   < 0 || E->sink_node   >= T->node_count)
            return SGPL_DAG_ERR_INVALID;
        if (E->source_node == E->sink_node) return SGPL_DAG_ERR_CYCLE;
        if (!(E->flags & (SGPL_DAG_REL_FLAG_SEMANTIC | SGPL_DAG_REL_FLAG_REALIZATION)))
            return SGPL_DAG_ERR_INVALID;
        if ((E->flags & SGPL_DAG_REL_FLAG_SEMANTIC) && E->witness_id == 0)
            return SGPL_DAG_ERR_INVALID;   /* no semantic edge without a witness */
    }
```

### 2.7 The runtime context (the whole state of a step)

`autotuner_runtime.h:461-491`:

```cpp
typedef struct sgpl_exec_ctx {
  void *graph;
  uint64_t round_id;
  int32_t domain_kind;    /* SGPL_DOMAIN_* */
  int32_t traversal_kind; /* SGPL_TRAVERSE_* */
  const uint8_t *membership;
  int32_t *dest_seen;
  int32_t *next_frontier;
  int32_t initial_next_size;
  int32_t *append_head;   /* atomic counter written by activation ops */
  void *round_state;      /* snapshot/envelope state owned by the round */
  struct sgpl_runtime_op *ops;
  uint32_t op_count;
  void *partition_base;   /* per-partition partial array base (combine) */
  int64_t partition_stride;
  void *partition_state;  /* current partition slot, set by the executor */
  void *source_state;     /* source-scoped claim channel (R7) */
  void *env;
  int32_t next_size;      /* out: initial_next_size + appended */
  int32_t run_round_begin;/* owned by the first stage of a round */
  int32_t run_round_end;  /* owned by the last stage of a round */
  int32_t step_id;        /* TDG site id; -1 = dispatch unchanged */
} sgpl_exec_ctx;
```

---

## 3. CPU scheduling — spatial axis

### 3.1 Spatial template → partitions (pseudocode, one node per partition, no edges)

From `sgpl_exec_dag_spatial` (`autotuner_runtime.c:4854-4882`):

```cpp
int32_t sgpl_exec_dag_spatial(sgpl_exec_ctx *ctx, int32_t partitions,
                              int32_t worker_budget) {
  sgpl_dag_template tmpl;
  if (!ctx || partitions <= 0) return SGPL_DAG_ERR_INVALID;
  if (worker_budget <= 0) worker_budget = 1;
  {   /* a dispatch inside a scheduler node is capped by the run's budget */
    int32_t avail = sgpl_current_thread_budget();
    if (avail > 0 && worker_budget > avail) worker_budget = avail;
  }
  if (worker_budget > partitions) worker_budget = partitions;
  tmpl.node_count = partitions;
  tmpl.relation_count = 0;          /* V1: independent partitions */
  tmpl.relations = NULL;
  tmpl.node_fn = sgpl_dag_partition_node;
  return autograph_execute_dag(&tmpl, ctx, worker_budget);
}
```

```cpp
static int32_t sgpl_dag_partition_node(void *state, int64_t instance) {
  sgpl_exec_partition_body(instance, state);
  return 0;
}
```

### 3.2 The partitions themselves (CleanCut), and the owner mapping

`autotuner_runtime.c:2888-2914` — the partition of vertex `u` is the inverse of
`start[p] = floor(p*n/P)`:

```cpp
    while (cc_arc_next(&it)) {
      /* p(u) = the partition whose [start[p], start[p+1]) contains u
       * (exact inverse of start[p] = floor(p*n/P)). */
      int32_t p = (int32_t)((((int64_t)it.u + 1) * partitions - 1) / n);
      if (p < 0 || p >= partitions) continue;
      src_row_count[p]++;
    }
```

`autotuner_runtime.c:3126-3180` — the partition body, both ownership modes and
the membership gate:

```cpp
static void sgpl_exec_partition_body(int64_t index, void *opaque) {
  sgpl_exec_ctx *ctx = (sgpl_exec_ctx *)opaque;
  sgpl_exec_ctx local = *ctx;      /* per-worker copy: partition_state is local */
  AutoGraphMeta *meta = find_meta(ctx->graph);
  int32_t p = (int32_t)index;
  …
  local.partition_state =
      (local.partition_base && local.partition_stride > 0)
          ? (char *)local.partition_base + (int64_t)p * local.partition_stride
          : local.partition_base;
  …
  if (local.traversal_kind == SGPL_TRAVERSE_OWNER_V) {
    int64_t rows = meta->push_row_count ? meta->push_row_count[p] : 0;
    int64_t *rp = meta->push_rp ? meta->push_rp[p] : NULL;
    int32_t *ci = meta->push_ci ? meta->push_ci[p] : NULL;
    int32_t *indir = meta->push_indir ? meta->push_indir[p] : NULL;
    for (int64_t r = 0; r < rows; ++r) {
      int32_t u = indir[r];
      if (local.membership && !local.membership[u]) continue;   /* F_t gate */
      for (int64_t j = rp[r]; j < rp[r + 1]; ++j)
        SGPL_EXEC_EMIT_PAIR(u, ci[j]);
    }
  } else {                          /* SGPL_TRAVERSE_OWNER_U: source slices */
    int32_t *pairs = meta->src_pairs ? meta->src_pairs[p] : NULL;
    int64_t cnt = meta->src_pair_count ? meta->src_pair_count[p] : 0;
    …
    for (; e < cnt; ++e) {
      int32_t u = pairs[2 * e], v = pairs[2 * e + 1];
      if (local.membership && !local.membership[u]) continue;
      …                             /* source lifecycle inside the slice */
```

Ownership is a closed set (`autotuner_runtime.h:376-377`), and the executor
aborts on anything else (`autotuner_runtime.c:3664-3666`):

```c
#define SGPL_TRAVERSE_OWNER_U 0   /* source-owned flat src_pairs slices */
#define SGPL_TRAVERSE_OWNER_V 1   /* destination-owned push_rp/ci/indir rows */
…
  if (ctx->traversal_kind != SGPL_TRAVERSE_OWNER_U &&
      ctx->traversal_kind != SGPL_TRAVERSE_OWNER_V)
    abort(); /* invalid traversal mechanism: compiler error */
```

### 3.3 Deterministic combine, ascending (spatial reduction of partials)

`autotuner_runtime.c:3705-3717`:

```cpp
  if (sgpl_exec_has_cap(ctx, SGPL_OP_COMBINE)) {
    for (p = 0; p < meta->partition_count; ++p) {
      ctx->partition_state =
          (ctx->partition_base && ctx->partition_stride > 0)
              ? (char *)ctx->partition_base + (int64_t)p * ctx->partition_stride
              : ctx->partition_base;
      for (i = 0; i < ctx->op_count; ++i) {
        sgpl_runtime_op *op = &ctx->ops[i];
        if ((op->capabilities & SGPL_OP_COMBINE) && op->combine)
          op->combine(op->state, ctx);
      }
    }
  }
```

### 3.4 Spatial dispatch of a step, and the width decision

`autotuner_runtime.c:3586-3650` — the dispatch chain: device step (if the gate
accepts) → bounded serial calibration → planned TDG level:

```cpp
static void sgpl_exec_step_dispatch(sgpl_exec_ctx *ctx, AutoGraphMeta *meta) {
  int32_t step_id = ctx->step_id;
  int32_t partitions = (int32_t)meta->partition_count;
  …
  if (step_id < 0 || tdg_engine_disabled) {
    if (getenv("SGPL_DAG_SPATIAL"))
      sgpl_exec_dag_spatial(ctx, (int32_t)(partitions), sgpl_configured_worker_count());
    else
      parallel_for_runtime(0, partitions, 1, sgpl_exec_partition_body, ctx, 0, 0);
    return;
  }
  if (sgpl_gpu_step_try_device(ctx, meta)) return;   /* device path, else falls through */
  sgpl_set_pending_loop_id(step_id);
  if (!sgpl_step_site_ready(step_id)) {              /* bounded calibration */
    uint64_t t0 = now_monotonic_ns();
    sgpl_set_desired_threads(1, 1);
    parallel_for_runtime(0, partitions, 1, sgpl_exec_partition_body, ctx, 0, 0);
    sgpl_set_desired_threads(0, 0);
    sgpl_exec_step_record_sample(step_id, partitions, now_monotonic_ns() - t0);
    return;
  }
  {                                                  /* planned single-site level */
    int32_t site_id = step_id;
    int64_t arcs = canonical_edge_count_cached(meta);
    SgplExecStepArg arg = {ctx, partitions};
    sgpl_tdg_task_desc task; … task.fn = sgpl_exec_step_task; task.profile_id = step_id;
    task.static_work_units = (arcs > (int64_t)INT32_MAX) ? INT32_MAX : (int32_t)arcs;
    task.num_loop_sites = 1; task.loop_site_ids = &site_id;
    sgpl_run_tdg_level(&task, 1, arcs, partitions);
  }
}
```

Pseudocode of the level planner (`parallel_runtime.c:3827/:4055`,
`:3497 sgpl_choose_tdg_threads_with_loop_budget`):

```
plan(task, level):
    width = choose_tdg_threads(effective_work, effective_span, budget, task_cap)
    granted = budget_try_reserve(width)          # fail-serial: may grant 1
    if granted <= 1:  run serially (still correct)
    else:             create granted workers; pthread_join each; release(granted)
```

### 3.5 The shared ledger and the share rule

`parallel_runtime.c:364-397` — share for a nested scope, availability for the
ledger (`max(…, 1)` floors appear twice: a share is never zero):

```c
static int32_t sgpl_thread_budget_share_for_scope(int32_t total) {
    int32_t scope_threads = g_tls_thread_budget_scope_reserved;
    if (scope_threads > 1) { total = total / scope_threads; if (total < 1) total = 1; }
    return total;
}
static int32_t sgpl_budget_available_threads(void) {
    int32_t total = sgpl_thread_budget_cap();
    if (g_tls_thread_budget_scope_reserved > 0) {
        total = sgpl_thread_budget_share_for_scope(total);
        available = total;
    } else {
        int32_t reserved = atomic_load(&g_sgpl_reserved_threads);
        available = total - reserved;
    }
    if (available < 1) available = 1;
    return (int32_t)available;
}
```

`parallel_runtime.c:399-440` — reservation is a CAS on the ledger; when the
budget is exhausted it **grants 1 and never blocks** (fail-serial):

```c
static int32_t sgpl_budget_try_reserve(int32_t requested) {
    int32_t total = sgpl_thread_budget_cap();
    int32_t scope_threads = g_tls_thread_budget_scope_reserved;
    int current = atomic_load(&g_sgpl_reserved_threads);
    if (requested <= 1 || total <= 1) return 1;
    if (requested > total) requested = total;
    if (scope_threads > 0) { … return requested; }
    for (;;) {
        int available = total - current;
        int granted = requested;
        if (available <= 1) { … return 1; /* insufficient-available */ }
        if (granted > available) granted = available;
        …
```

### 3.6 Spatial barriers and joins

`parallel_runtime.c:1376-1388` — the pool's publish/join barriers:

```c
static void sgpl_thread_pool_run(workers_args_t *args, int nthreads) {
    if (nthreads > g_sgpl_thread_pool_size) nthreads = g_sgpl_thread_pool_size;
    pthread_mutex_lock(&g_sgpl_thread_pool_run_lock);
    g_sgpl_thread_pool_job_args = args;
    g_sgpl_thread_pool_job_nthreads = nthreads;
    pthread_barrier_wait(&g_sgpl_thread_pool_barrier);   /* dispatch */
    pthread_barrier_wait(&g_sgpl_thread_pool_barrier);   /* join */
    …
```

Ephemeral launches use create+`pthread_join` (`:1681-1682`, `:1990-1998`,
`:2085-2093`); TDG levels join their workers (`:4350-4359`); the DAG scheduler
broadcasts on completion and joins after drain (`autotuner_runtime.c:4685`,
`:4819-4820`). Iteration-granularity dependencies spin on their slot
(`parallel_runtime.c:1055-1062`):

```c
    while (atomic_load(sgpl_doacross_slot(state, id, slot)) < dep_iter) {
        __builtin_ia32_pause();
    }
```

---

## 4. CPU scheduling — temporal axis

### 4.1 The τ lattice (temporal value of a read) and its decision table

`graph_frontier_lowering.cpp:1133-1148` (rank) and `:2141-2175` (the rule, verbatim
comment included — the write's *phase* is decisive):

```cpp
static int temporalRank(Temporal T) {
    switch (T) {
    case Temporal::Carried:           return 3;
    case Temporal::PreviousRoundRead: return 2;
    case Temporal::SameRoundRead:     return 1;
    default:                          return 0;   /* Independent */
    }
}
```

```cpp
static Temporal relateReadWrite(const Effect &Rd, const Effect &M,
                                PhaseSegment MSeg, DomainKind RoundDomain) {
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
    if (Rd.Reg == Region::V && M.Reg == Region::U) …
```

### 4.2 One round, end to end

`autotuner_runtime.c:3652-3734` (round body — the order is the contract):

```c
int32_t autograph_frontier_execute(void *graph_ptr, sgpl_exec_ctx *ctx) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  …
  if (!meta || meta->partition_count <= 0 || !ctx->ops) return ctx->initial_next_size;
  if (ctx->traversal_kind != SGPL_TRAVERSE_OWNER_U &&
      ctx->traversal_kind != SGPL_TRAVERSE_OWNER_V)
    abort();                                     /* compiler-error guard */
  sgpl_exec_validate(ctx);

  cov_begin = ctx->traversal_kind == SGPL_TRAVERSE_OWNER_V &&
              sgpl_exec_has_cap(ctx, SGPL_OP_SOURCE_BEGIN);
  cov_end   = ctx->traversal_kind == SGPL_TRAVERSE_OWNER_V &&
              sgpl_exec_has_cap(ctx, SGPL_OP_SOURCE_END);

  if (ctx->run_round_begin) {
    sgpl_exec_round_begin_ops(ctx);
    sgpl_exec_snapshot_ops(ctx);                 /* frozen round-start view */
  }
  if (cov_begin) sgpl_exec_source_coverage(ctx, /*begin=*/1);
  sgpl_exec_step_dispatch(ctx, meta);
  if (cov_end)   sgpl_exec_source_coverage(ctx, /*begin=*/0);
  … /* ascending combine (see 3.3) */
  ctx->next_size = ctx->initial_next_size +
                   (ctx->append_head
                        ? (int32_t)__atomic_load_n(ctx->append_head, __ATOMIC_RELAXED)
                        : 0);
  if (ctx->run_round_end) sgpl_exec_round_end_ops(ctx);
  return ctx->next_size;
}
```

### 4.3 Temporal template → a witness-backed chain

`autotuner_runtime.c:4892-4920` — the scheduler orders round wrappers; the
wrapper never re-implements a round:

```cpp
static int32_t sgpl_dag_temporal_node(void *state, int64_t instance) {
  const sgpl_temporal_unit *unit = &((const sgpl_temporal_unit *)state)[instance];
  if (!unit->graph || !unit->ctx) return SGPL_DAG_ERR_NODE;
  (void)autograph_frontier_execute(unit->graph, unit->ctx);
  return SGPL_DAG_OK;
}

int32_t sgpl_exec_dag_temporal_chain(const sgpl_temporal_unit *units,
                                     int32_t unit_count, int32_t worker_budget) {
  if (unit_count > 1) {
    rels = calloc(unit_count - 1, sizeof(*rels));
    for (i = 0; i < unit_count - 1; ++i) {
      rels[i].kind = SGPL_DAG_REL_PRECEDENCE;
      rels[i].source_node = i; rels[i].sink_node = i + 1;
      rels[i].witness_id = 1;                       /* round-carried dependence */
      rels[i].flags = SGPL_DAG_REL_FLAG_SEMANTIC;
    }
  }
  tmpl.node_count = unit_count;
  tmpl.relation_count = unit_count > 1 ? unit_count - 1 : 0;
  tmpl.relations = rels; tmpl.node_fn = sgpl_dag_temporal_node;
  rc = autograph_execute_dag(&tmpl, (void *)units, worker_budget);
  free(rels); return rc;
}
```

### 4.4 The ready-work scheduler and the ledger quota `max(1, W/A)`

`autotuner_runtime.c:4662-4666` — the quota, exactly:

```c
static int32_t dag_dispatch_budget(const sgpl_dag_run *R) {
  int32_t active = R->active > 0 ? R->active : 1;
  int32_t share = R->worker_budget / active;
  return share > 0 ? share : 1;
}
```

The comment above it states the intent: *“A node dispatched while A nodes are
active in a run with budget W may hand max(1, W/A) threads to any nested dispatch
it makes … A chain (A == 1) keeps the full budget, which is exact because the
units are serial.”* It is applied when the node is launched, and pushed onto the
thread's budget stack so nested dispatches inherit it (`:4708-4715`, `:4786`).

`autotuner_runtime.c:4669-4686` — completion is what makes successors ready:

```c
static void dag_complete(sgpl_dag_run *R, int32_t node, int32_t rc) {
  const sgpl_dag_template *T = R->tmpl;
  R->active--; R->completed[node] = 1; R->completed_count++;
  if (rc != 0) { R->failed = 1; }          /* cancel new dispatch; drain */
  else {
    for (int32_t i = 0; i < T->relation_count; i++) {
      const sgpl_dag_relation_desc *E = &T->relations[i];
      if (E->source_node != node) continue;
      if (--R->remaining[E->sink_node] == 0) dag_push(R, E->sink_node);
    }
  }
  pthread_cond_broadcast(&R->cv);
}
```

### 4.5 Round separation (the Snapshot discharge of R4/R5)

Compiler side (`graph_frontier_lowering.cpp:3524`, publish `:3536`): a per-round
shadow global plus a `Snapshot(A)` callback; runtime side
(`autotuner_runtime.c:3868-3892`):

```c
void *autograph_snapshot_publish(void *graph_ptr, const void *live_base,
                                 int64_t elem_bytes, int32_t slot,
                                 const char *name) {
  AutoGraphMeta *meta = find_meta(graph_ptr);
  …
  bytes = meta->csr_n * elem_bytes;
  buf = autograph_scratch_shadow(graph_ptr, bytes, slot);
  if (!buf) return NULL;
  memcpy(buf, live_base, (size_t)bytes);
  if (name && *name && sgpl_gpu_register_pointee)
    sgpl_gpu_register_pointee(name, buf, bytes);   /* device upload per launch */
  return buf;
}
```

---

## 5. Everything else the effect system carries

- **Membership gating (the round domain).** `Info.MembershipGated` is set when
  the driver reads the frontier; the round domain becomes
  `DomainKind::Frontier` and the gate itself is a byte mask applied in the
  partition body (`if (local.membership && !local.membership[u]) continue;` —
  §3.2). Filled by `autograph_fill_frontier_membership`
  (`autotuner_runtime.c:2049-2056`), allocated lazily with the other scratch.
- **Activation envelope (dest_seen / next_frontier / append_head).** The
  primitive is atomic and total (`autotuner_runtime.c:3735-3746`):

  ```c
  int32_t autograph_frontier_activate(sgpl_exec_ctx *ctx, int32_t v) {
    int32_t expected = 0, head;
    if (!ctx || !ctx->dest_seen || v < 0) return 0;
    if (!__atomic_compare_exchange_n(&ctx->dest_seen[v], &expected, 1, 0,
                                     __ATOMIC_RELAXED, __ATOMIC_RELAXED))
      return 0;                                    /* already claimed */
    if (ctx->next_frontier && ctx->append_head) {
      head = __atomic_fetch_add(ctx->append_head, 1, __ATOMIC_RELAXED);
      ctx->next_frontier[ctx->initial_next_size + head] = v;
    }
    return 1;
  }
  ```

  The executor never activates and never appends (header contract,
  `autotuner_runtime.h:497-501`); the append order is an implementation detail
  because the frontier is the *set* `{v | dest_seen[v] = 1}`.
- **Partition state.** `partition_base/stride/state` — set per partition before
  the body runs, read by emitted combine/source-end hooks through
  `autograph_exec_partition_state`.
- **Privatized layouts.** The proof has teeth — it refuses claims and
  non-recognized updates up front (`graph_frontier_lowering.cpp:2633-2660`):

  ```cpp
  if (Info.HasFirstWins || hasPerSourceClaim(Info)) {
      PrivReason = "claim in the loop nest";
      return false;
  }
  for (const Effect &E : Info.Effects) {
      if (E.Kind == EffectKind::R || E.Kind == EffectKind::Activate) continue;
      if (E.Kind != EffectKind::Uop || E.Op == RedOp::None || !E.Base) {
          PrivReason = "mutating effect is not a recognized update";
          return false;
      }
      …
  ```

  and a surviving shared access to a privatized base refuses the work function
  (`:4363-4374`). Runtime binding: per-partition buffers initialized to the
  operator identity (`autotuner_runtime.c:1887-1925`).
- **Ownership.** `SGPL_TRAVERSE_OWNER_U = 0` / `OWNER_V = 1`, the two walks in
  §3.2, the abort on any other kind, and the complete source-domain coverage
  passes for OWNER_V (`:3216-3227`).
- **Snapshots / R7.** §4.5; `ValueSource::Snapshot` and
  `DischargeKind::Snapshot` are what make the relation *discharged* rather than
  an ordering edge.
- **Fail-closed refusals.** Compiler: the `refuse` lambda above + the witness
  sites; emission refusals (append without envelope, priv/red shape mismatch,
  unsupported claim staging, `emitExprInterp` failure → `markSequential`).
  Runtime: `sgpl_gpu_step_refuse` one-shot reasons (no kernel, envelope not
  wired, not mirrored, membership-restricted, combine present, per-source
  lifecycle, cost model too small) and `sgpl_exec_validate`/`abort` for contract
  violations. Everything not admitted stays on the CPU — by design.
  Note (this session): the device path no longer refuses a step whose registered
  kernel is source-owned when it was reached through the activation path; it
  routes to the source-owned device attempt instead, and the compile-time
  decision stands.

---

## 6. Worked example: `verify/cases/algo/kcore.graph`

Build and run (pod, from `p1GraphEasy-con-AutoTuner`; `LD_LIBRARY_PATH` is for
antlr, `ulimit -s unlimited` because generated frames can be tens of MB):

```bash
export LD_LIBRARY_PATH=/usr/local/lib
FORCE_GPU=1 GRAPH_FILE=../verify/cases/algo/kcore.graph bash 03_run.sh
ulimit -s unlimited
SGPL_NUM_THREADS=4 ./final_program                          # CPU-only
SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 \
  SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program               # device allowed
SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 SGPL_GPU_ENGINE_STEP=1 \
  SGPL_GPU_ENGINE_MIN_PAIRS=0 ./final_program 2>&1 | grep -E "ran on device|kept on the CPU"
```

Measured on the pod: both runs byte-identical (`diff -q` on the filtered outputs),
answer `kcore_size`, **16 device dispatches, 0 fallbacks** — and the corpus
differential records the same shape for the other device-eligible fixtures
(`bfs_level` 6, `pagerank` 16, `budget_two_steps` 1600, `data_index_write` 4,
`dg_src_keyed` 16) with **0 DIFF** overall.

Tracing the step through the sections above:

1. **Gate (§2.2–2.3).** `supportedByAlgebra` raises whatever the analysis found —
   for this nest a pure per-vertex accumulation (no frontier append, no claim, no
   carried read on a mutated base, no dual `U`+`V` base). No undischarged witness
   on either axis, no R8 realization guard → `admit()`; `Cert.RS == Cert.RT ==
   false`, `NIS == NIT == true`.
2. **Marker (§2.5).** `markDagOwned(L, Axes)` writes `sgpl.frontier.dag.axes`
   (here `spatial`, since only spatial witnesses were recorded — the value is
   built from the two witness lists, `:6560-6572`), so PDG/the outliner leave the
   nest inline for the engine.
3. **Template (§2.6).** Spatial: one node for the owner region actually written
   in the pair phase (the accumulated vertex field) → one node per partition,
   `relation_count = 0`. Temporal: the unit exists, and only undischarged
   cross-segment relations would add a realization-order edge (none here).
4. **Round (§4.2).** `autograph_frontier_execute` runs begin (no snapshot for
   this nest), source coverage, `sgpl_exec_step_dispatch` → the device attempt
   succeeds (full-domain, no combine), then the empty coverage end, then no
   combine ops — the accumulate-in-place already folded per partition — and the
   round returns.
5. **Partitions (§3.2).** The source-owned walk: each `p` takes its `src_pairs`
   slice from CleanCut (`p(u) = ((u+1)*P-1)/n`) and calls the pair op; two
   partitions never write the same slot by the owner-computes assignment — the
   same argument the device step inherits.
6. **Budget (§3.5, §4.4).** The stage either goes through a planned TDG level
   (reserve → workers → join → release, fail-serial at 1) or, when nested under
   the DAG scheduler, runs with `max(1, W/A)`.

Second data point actually measured this session (`edge_write_v`, a V-shaped step
without an activation envelope): the emitter prints

```
[gpu-emit] step 1 tag=sgpl_pair_work IsV=1 IsSimple=1 IsRed=0 IsSourceRed=0
           IsPriv=0 app=0 fw=0 rs=0 pl=0 rp=0 -> src=y v=n
```

i.e. `HasFrontierAppend == false` (so R7 is not raised and no envelope is
required) and no reduction/private/claim machinery — the verdict admits it, and
the runtime routes its source-owned kernel to the source-owned device attempt.
