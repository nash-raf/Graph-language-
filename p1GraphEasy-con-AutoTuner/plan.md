# Complete Two-Axis Interference-Proofing Implementation

Target file:

`/home/user/D/Course/msk1/2nd/Graph-language-/p1GraphEasy-con-AutoTuner/plan.md`

This plan describes the complete end state. All components below are required; none are deferred as optional future work.

## 1. Objective and invariants

Replace rejection-condition absence as the compiler’s source of truth with a complete, transformation-aware dependence proof.

For every candidate graph loop:

1. Derive all logical access footprints.
2. Derive every RAW, WAR, and WAW relation.
3. Classify every relation independently along:
   - spatial locality;
   - temporal locality;
   - phase/order scope;
   - occurrence scope;
   - owner/partition relation.
4. Apply only proof-preserving transformations to remove or rewrite edges.
5. Construct:
   - a spatial dependency graph;
   - a temporal dependency graph;
   - a product graph over temporal units and spatial partitions.
6. Derive spatial and temporal non-interference from graph partitionability.
7. Preserve the current runtime-composition policy:
   - a clean spatial axis is parallelized;
   - a clean temporal axis is parallelized;
   - an interfering axis is ordered according to its surviving graph constraints;
   - a witness on one axis never suppresses proven parallelism on the other;
   - if both axes cannot be safely realized, execution remains serial.
8. Keep R1–R7 as named semantic obligations and diagnostics, but never use their absence as the completeness proof.

The fundamental soundness rule is:

> An edge may be removed only by a proof of disjointness, ownership, privatization, snapshotting, occurrence-preserving staging, or a valid algebraic/commutative transformation.

Unknown aliasing, unknown provenance, relation overflow, unsupported index expressions, and incomplete graph materialization must fail closed.

## 2. Current implementation surface

The implementation must extend, not replace, the existing effect/runtime architecture.

### Compiler-side substrate

`graph_frontier_lowering.cpp` currently contains:

- `EffectKind`, `Region`, `Temporal`, `OccurrenceScope`, and `ValueSource`;
- the structural `EffectExpr` tree with `Preamble`, `Pair`, and `Epilogue` segments;
- provenance analysis for `U`, `V`, `D`, `G`, and `Top`;
- `privLayout`, `ownedLocalizedArray`, round-separation snapshots, frontier envelopes, claim staging, and algebraic reductions;
- preliminary `AccessRelation` records;
- `AxisCertificate` and R1–R7 witnesses;
- preliminary `SpatialGraphTemplate` and `TemporalGraphTemplate`;
- schedule selection through `ScheduleKind::{Serial, SpatialDag, TemporalDag, Nested}`;
- recursive runtime emission through `emitSingleStage`, `emitDualForkJoin`, and `emitExprInterp`.

### Runtime-side substrate

`autotuner_runtime.c/.h` currently contain:

- CleanCut source and destination ownership partitions;
- partition-local lifecycle execution;
- snapshot and frontier-envelope resources;
- staged and concurrent U/V fork/join;
- generic ready-work DAG execution;
- dependency validation, cycle detection, cancellation, and nested thread-budget accounting;
- spatial DAG dispatch with currently independent partition nodes;
- temporal-unit DAG support currently limited to a chain.

The existing lifecycle, resource, budget, GPU-gating, and fork/join mechanisms must remain intact. The implementation work is to make them consume complete proof graphs rather than coarse certificate shortcuts.

## 3. Formal effect and access model

### 3.1 Effect footprints

Extend `Effect` so that a single effect can describe both reads and writes.

This is required for:

- `U_f` read-modify-write operations;
- `U_oplus` algebraic updates;
- claims;
- operations that create both RAW and WAW obligations;
- operations whose read and write footprints have different indices.

Add explicit footprint records containing:

```text
base object
logical index expression
provenance region
read/write mode bits
occurrence scope
phase segment
round-domain binding
value source
origin instruction
```

The current `EffectKind` remains useful for classification and emission, but graph construction must use footprint modes rather than treating every non-`R` effect as one undifferentiated write.

Runtime resources such as:

- `DEST_SEEN`;
- `NEXT_FRONTIER`;
- `CLAIM`;
- `SNAPSHOT`;
- `PRIVATE`;
- `PARTIAL`;

must also be represented as explicit resource footprints so resource conflicts and logical-memory conflicts enter the same proof layer.

### 3.2 Complete relation generation

Replace the current capped pair scan in `deriveAccessRelations`.

For every ordered pair of effect occurrences on potentially aliased bases, independently generate:

```text
RAW: W_source ∩ R_sink
WAR: R_source ∩ W_sink
WAW: W_source ∩ W_sink
```

Read-read pairs do not create dependencies.

Do not impose order between `Par` siblings merely because `flattenTree` visits one child first. Semantic order comes only from:

- `Seq`;
- `SeqDomain`;
- explicit phase order;
- occurrence-domain order;
- round relation;
- a recognized realization transformation.

Remove the hard 64-relation limit. If an implementation limit is unavoidable, add an explicit overflow state and reject the proof rather than silently dropping relations.

Each relation must retain:

```text
source effect
sink effect
base
relation kind
source/sink footprints
source/sink regions
source/sink indices
source/sink phases
source/sink occurrence scopes
original semantic order, if one exists
spatial classification
temporal classification
instance relation
discharge kind
witness id
```

`WrittenBases`, `ReadBases`, `BaseU`, and `BaseV` remain useful summaries but are not substitutes for logical-location relations.

## 4. Temporal relation system

### 4.1 Preserve the existing discovery mechanism

Keep `relateReadWrite` as the basis for existing read/write temporal classification.

It already derives pairwise locality from:

- common base;
- region;
- write phase;
- round domain;
- membership gating.

However, its result must be stored on the individual relation that caused it. The scalar `Effect::Temp` remains only a diagnostic maximum.

Do not build temporal edges from:

```text
Effect::Temp == strongest relation
```

Build them from the complete relation list.

### 4.2 Extend temporal locality to every relation kind

Add temporal derivation for:

- RAW;
- WAR;
- WAW;
- read-modify-write effects;
- claim occurrence ordering;
- snapshot-resolved reads;
- cross-phase writes;
- round-carried writes;
- previous-round reads;
- fixed-offset and affine unit relations.

The temporal relation must include both a human-readable projection and a precise instance relation:

```text
Independent
SameRoundRead
SameRoundWrite
SameRoundOrdered
PreviousRoundRead
PreviousRoundWrite
Carried
FixedOffset(k)
Affine
Unknown
```

Existing names such as `SameRoundRead`, `PreviousRoundRead`, and `Carried` remain available in diagnostics. The graph representation must additionally retain:

```text
phase relation
round offset
occurrence relation
ordering direction
```

WAR and WAW cannot be inferred only from read-centric logic. They require explicit source/sink unit reasoning.

### 4.3 Temporal graph scopes

Construct temporal graphs at both required scopes:

1. **Within-round phase graph**
   - nodes are phase occurrences or phase work units;
   - edges preserve `Preamble → Pair → Epilogue`;
   - `Par` siblings remain unordered unless a footprint/resource relation requires an edge.

2. **Across-round graph**
   - nodes are temporal rounds or stages;
   - edges represent carried, previous-round, fixed-offset, or affine relations;
   - snapshot transformations remove only the specific cross-round relations they resolve.

The temporal certificate must retain all relation edges, even when the summary projection chooses the strongest locality for display.

## 5. Spatial locality and ownership model

### 5.1 Region is not ownership

`Region::U`, `Region::V`, `Region::D`, and `Region::Top` are provenance classes, not complete owner proofs.

A spatial edge may be removed for a U/V access only after an owner proof establishes the mapping from the logical index to a partition.

Examples:

- `A[u]` can be source-owned when `u` is the exact source endpoint.
- `A[u+1]` is U-derived but may cross a source partition boundary.
- `A[v]` can be destination-owned when `v` is the exact destination endpoint.
- `A[f(u,v)]` requires an affine/range proof.
- `D` and `Top` have no owner unless a transformation such as privatization removes the shared location.

Use the existing `isOwnedEndpointIndex`, `canonicalArrayBase`, `sameArraySlot`, and CleanCut metadata as inputs to a new explicit owner resolver.

### 5.2 Owner resolution

Introduce a spatial ownership classification:

```text
ExactOwner
SameOwner
ProvedDisjointOwner
CrossOwner
MayCrossOwner
NoOwner
UnknownOwner
```

For each footprint, resolve:

- source partition ownership;
- destination partition ownership;
- scalar/global ownership;
- private-copy ownership;
- frontier-envelope ownership.

The resolver must support:

1. exact U/V endpoint indices;
2. partition ranges from CleanCut;
3. affine index expressions;
4. constant and loop-invariant indices;
5. data-derived indices;
6. unknown/mixed provenance.

Use LLVM value/range reasoning where available. If an affine or range proof cannot establish disjointness, retain a conservative edge.

### 5.3 Partition maps

The compiler emits a symbolic partition schema. The runtime instantiates it using:

- `partition_start`;
- source ranges;
- destination home partitions;
- source pair slices;
- destination row structures;
- active frontier membership.

Partition maps must be represented independently for:

```text
source-owned U domains
destination-owned V domains
data-derived domains
scalar/global domains
private partition copies
```

Do not infer that all accesses in the same `Region` are automatically in the same partition.

## 6. Spatial dependency graph

### 6.1 Spatial graph nodes

A spatial node represents a realizable partition work unit:

```text
spatial partition id
owner domain
phase scope
runtime work range
```

The compiler-side graph is symbolic over partition variables. The runtime expands it to concrete partition nodes.

The graph must support:

- U-owned partitions;
- V-owned partitions;
- private partitions;
- phase-specific nodes;
- product nodes `(round, partition, phase)`.

### 6.2 Spatial edge construction

For every surviving spatial relation:

- add RAW precedence where a reader must observe a writer;
- add WAR precedence where a writer cannot overtake a prior reader;
- add WAW ordering for noncommuting writes;
- add mutual-exclusion metadata for conflicts that can be lowered to deterministic precedence;
- add realization-order edges for implementation-imposed ordering that is not a semantic dependence.

For partitionability, construct an undirected projection of all surviving cross-partition relations.

Spatial non-interference is proven when:

```text
there are at least two realizable nonempty components
and no surviving dependency crosses the component partition.
```

A directed graph may contain internal cycles without invalidating component analysis, but any graph used for execution must be acyclic after its ordering transformations. Cycles fail closed.

### 6.3 Runtime spatial graph expansion

Add runtime support to materialize spatial edges from symbolic schemas.

The runtime must:

1. enumerate concrete partitions;
2. evaluate owner predicates;
3. map relation schemas to partition pairs;
4. deduplicate edges;
5. retain witness/discharge metadata;
6. construct connected components;
7. execute independent components concurrently;
8. respect precedence inside each component.

If a symbolic relation is unknown, the runtime must conservatively connect the affected partitions or reject the spatial proof.

## 7. Transformations and edge rewrites

Every transformation must be represented as a proof object that consumes specific edges and optionally emits replacement edges.

### 7.1 Privatization

Use `privLayout` and existing private-copy emission only when:

- every privatized update is recognized;
- the operator identity is known;
- the operation is fold-sound;
- reads do not observe an unmerged private value;
- the final combine preserves the required observable order.

Remove only the cross-partition relations made private. Add:

- partition-private write edges;
- combine/fold edges;
- deterministic finalization edges.

Do not mark arbitrary shared RAW/WAR relations discharged merely because `UsePrivLayout` is true.

### 7.2 Owner-computes

Remove destination conflicts only when:

- every destination has exactly one owner;
- the index is proven to be the destination endpoint;
- all readers/writers participating in the relation use the same ownership contract;
- source coverage and zero-pair source lifecycle remain preserved.

Retain relations for cross-owner reads, side effects, and resources not covered by the owner proof.

### 7.3 Snapshot

For each `RoundSepBase`:

- redirect only the designated cross-endpoint reads;
- retain same-owner RMW reads as live;
- add round-begin snapshot publication;
- add snapshot-to-reader ordering;
- remove only the corresponding cross-round RAW relations.

Snapshot is primarily a temporal transformation. It must not remove a spatial edge unless the snapshot also changes the spatial footprint being observed.

### 7.4 Claim staging

Claims must preserve:

- occurrence scope;
- per-source versus per-pair multiplicity;
- guard/transition semantics;
- serial priority order.

Replace unsupported cross-scope claim relations with explicit claim-stage precedence. Atomicity alone never permits reordering.

### 7.5 Algebraic fold

Use `foldSound` and operator/type checks.

A fold may remove WAW ordering only if:

- the operator is associative and commutative under the language semantics;
- identity initialization is valid;
- intermediate observers cannot distinguish the reordered updates;
- the final combine order is deterministic where required.

An algebraic fold may discharge both spatial and temporal relations only when both projections satisfy the proof conditions.

### 7.6 Frontier envelope

The R7 transformation must explicitly model:

```text
F_t read membership
dest_seen claim
F_{t+1} append
append-head ordering
```

A wired envelope discharges only append/read relations separated by the envelope. It does not make arbitrary frontier mutations independent.

### 7.7 R1–R7 mapping

R1–R7 become graph-derived witness classes:

- **R1:** no owner for data-derived writes; unresolved spatial edges unless privatized.
- **R2:** occurrence-preservation conflict for per-source claims; discharge only through claim staging.
- **R3:** unknown/mixed provenance; unresolved spatial edges unless a private proof applies.
- **R4:** carried read observing a mutated base; discharge through snapshot or privatization, otherwise temporal edge.
- **R5:** cross-phase write invalidating round separation; retain temporal phase/round edge.
- **R6:** same-base U/V ownership conflict; discharge through exact disjointness, privatization, or staged realization.
- **R7:** frontier append without a valid envelope; retain resource and spatial edges.
- **R8:** implementation/modelability failure, outside the semantic theorem lattice.

The compiler must no longer infer interference solely by testing whether an R1–R7 witness was emitted.

## 8. Product graph

### 8.1 Product graph node

A product node represents:

```text
temporal unit × spatial partition × phase
```

Conceptually:

```text
(t, p, phase)
```

The product graph is built from the spatial and temporal projections of the same complete relation set.

### 8.2 Product graph edges

Generate:

- spatial edges between `(t,p,phase)` and `(t,q,phase')`;
- temporal edges between `(t,p,phase)` and `(t+k,q,phase')`;
- phase-order edges within one temporal unit;
- snapshot, combine, claim, and lifecycle realization edges;
- cross-axis edges where one relation carries both a spatial owner relation and a temporal offset.

The product graph must not duplicate lifecycle operations:

- round begin executes once per temporal unit;
- snapshots publish once per owning round;
- source coverage executes once under the owner context;
- partition begin/end execute once per spatial node;
- round end executes once after all required children complete.

### 8.3 Product graph use

The product graph is used whenever the runtime wants to overlap temporal and spatial work under coupled constraints.

It is optional only as an optimization when the two projections are completely independent and nested execution already provides the same legal schedule.

The proof layer must nevertheless construct and validate the product graph for every admitted candidate so that coupled dependencies cannot be hidden by separate-axis reasoning.

## 9. Proof certificate

Replace the current certificate verdict logic with a complete `AxisProof` object.

The proof object must contain:

```text
all access relations
all spatial edges
all temporal edges
all product edges
all discharged edges
all unresolved edges
partition candidates
connected components
cycle status
overflow/unknown status
witness records
schedule realization requirements
```

Derive:

```text
NI_S = spatial graph admits a valid nontrivial partition
NI_T = temporal graph admits a valid nontrivial partition
```

The global theorem remains:

```text
NonInterfere = NI_S ∨ NI_T
```

The proof must distinguish:

```text
Proven non-interference
Proven interference / connected graph
Inconclusive / unsupported / overflow
```

An inconclusive proof is not treated as non-interference.

R1–R7 witness records remain useful for diagnostics, census, and transformation accounting, but:

- an unresolved graph edge may exist without an R1–R7 label;
- every unresolved edge must still prevent the corresponding non-interference claim;
- every removed edge must point to a discharge proof;
- every emitted semantic edge must point to a proof/relation witness;
- every realization-only edge must be marked separately.

## 10. Schedule selection and runtime composition

Preserve the current axis-composition policy.

### Both axes clean

Use `Nested` or product execution:

- spatial partition work may run concurrently;
- temporal units may run concurrently;
- all product edges are validated;
- independent components share the thread budget.

### Spatial clean, temporal interfering

Use `SpatialDag`:

- spatial components run concurrently;
- temporal rounds/phases preserve their surviving order edges;
- R4/R5/R2 constraints are realized by temporal ordering, staging, or claim preservation;
- spatial proof is still graph-derived, not inferred from an empty R list.

### Spatial interfering, temporal clean

Use `TemporalDag`:

- temporal components run concurrently where legal;
- spatial graph edges are realized inside each temporal unit;
- U/V staged execution remains available for R6;
- partial spatial DAG execution is permitted when the surviving graph is acyclic and representable.

### Both axes interfering

Use the product graph if it has a valid realizable schedule with partial concurrency. Otherwise use serial execution.

This does not weaken the theorem. A dirty axis may still have internal DAG parallelism, but it must not be classified as non-interfering unless the partition criterion is proven.

### R8 or implementation failure

Use serial execution and mark an implementation failure separately from semantic interference.

The emitted metadata must continue to distinguish:

```text
semantic refusal
implementation refusal
DAG ownership
spatial parallel
temporal parallel
ordered axis
product schedule
```

The existing `autograph_exec_ctx_set_dag_axes` mechanism remains the runtime declaration path. It must be populated from the proof-derived schedule.

## 11. Runtime implementation

### 11.1 Generic DAG descriptor

Extend `sgpl_dag_relation_desc` with:

```text
axis mask
relation kind
phase/round offset
spatial partition relation
temporal unit relation
witness id
discharge id
semantic/realization flags
```

The executor may continue to execute normalized precedence edges, but graph construction must preserve the original relation metadata for validation and diagnostics.

Validate:

- node bounds;
- no self edges;
- witness on semantic edges;
- valid discharge references;
- no duplicate contradictory edges;
- acyclicity for the realization graph;
- complete node/component coverage.

### 11.2 Spatial executor

Extend `sgpl_exec_dag_spatial` to accept a compiler/runtime graph descriptor instead of always creating a zero-edge independent-partition template.

The executor must:

- run independent components concurrently;
- preserve edge order inside components;
- apply weighted nested budgets;
- expose worker indices as today;
- preserve partition/source/pair/combine lifecycle;
- fail closed on invalid graph descriptors.

### 11.3 Temporal executor

Extend temporal execution beyond a fixed chain.

Support:

- round/stage nodes;
- partial DAGs;
- phase-level nodes;
- fixed-offset and affine edges;
- dynamic round production for frontier algorithms;
- dependency-aware publication of newly generated temporal units.

Where rounds are dynamically discovered, use a producer/consumer temporal scheduler that does not create a successor until its frontier/input state is available.

### 11.4 Product executor

Add a product execution entry point that accepts:

```text
product node schema
spatial partition map
temporal unit factory or unit array
phase/lifecycle callbacks
edge schema
worker budget
```

The product executor must materialize reachable `(round, partition, phase)` nodes, apply relation predicates, and dispatch ready nodes through the existing budget ledger.

Nested calls must never claim the full machine behind the parent scheduler’s back.

### 11.5 Existing lifecycle preservation

Do not duplicate or bypass:

- `autograph_frontier_execute`;
- snapshot ownership;
- frontier envelope commit;
- source coverage;
- private partial binding;
- deterministic combine;
- staged U/V fork/join semantics.

The graph scheduler orders and dispatches existing lifecycle units; it does not reimplement their semantic behavior.

### 11.6 GPU/device boundary

Preserve the existing device rule:

- a device step may execute only when the proof declares the spatial axis concurrent;
- ordered spatial constraints force CPU fallback;
- temporal constraints never silently authorize an unsafe device schedule;
- device fallback must produce the same answer as CPU graph execution.

Product schedules with unsupported device descriptors fall back to the CPU product executor.

## 12. Compiler integration points

Update the following areas.

### `graph_frontier_lowering.cpp`

Implement:

- expanded effect footprints;
- complete relation generation;
- temporal relation retention;
- WAR/WAW temporal derivation;
- owner and partition resolution;
- spatial graph construction;
- temporal graph construction;
- product graph construction;
- transformation/discharge application;
- component and cycle analysis;
- proof-derived `AxisCertificate`;
- proof-derived schedule selection;
- graph descriptor emission;
- relation/witness diagnostics;
- complete edge-consumption validation.

Update `emitSingleStage`, `emitDualForkJoin`, and `emitExprInterp` to consume graph-derived realization plans while preserving their current lifecycle behavior.

### `autotuner_runtime.h/.c`

Implement:

- extended graph descriptors;
- graph materialization;
- spatial DAG execution with real edges;
- temporal partial-DAG execution;
- dynamic temporal-unit production;
- product DAG execution;
- edge/discharge validation;
- component scheduling;
- budget propagation;
- lifecycle-safe node wrappers.

### `parallel_loop_outline.cpp` and `pdg.cpp`

Update metadata handling so graph-owned loops are recognized using proof-derived axis/product metadata rather than only the presence of a legacy DAG marker.

The outliner must not independently reclassify a proof-owned loop as safe or unsafe.

### Documentation

Update:

- `EFFECT_SYSTEM.md`;
- `EFFECT_SYSTEM_IMPLEMENTATION_REPORT.md`;
- `proof/EFFECT_ALGEBRA_DESIGN.md`;
- theorem/proof notes;
- runtime and certificate output documentation.

The documentation must explicitly state that R1–R7 are witness classes and transformations, not an assumed exhaustive decision procedure.

## 13. Tests and acceptance criteria

### Compiler analysis tests

Add fixtures for:

- `A[u]` source-owned writes;
- `A[v]` destination-owned writes;
- `A[u+1]` boundary-crossing source expressions;
- exact same-owner and cross-owner U/V accesses;
- unknown `Top` provenance;
- data-derived D writes;
- RMW effects;
- claims at per-source and per-pair scope;
- carried reads;
- previous-round reads;
- WAR across rounds;
- WAW across rounds;
- cross-phase reads/writes;
- frontier append/read conflicts;
- shadow snapshots;
- private reductions;
- algebraic folds;
- more than 64 relation candidates;
- `Par` siblings where source order must not be invented.

### Proof tests

For each fixture, assert:

- complete relation count;
- RAW/WAR/WAW classification;
- spatial owner status;
- temporal locality;
- product edge set;
- discharged versus unresolved edges;
- connected components;
- NI_S and NI_T;
- semantic versus implementation refusal;
- selected schedule;
- witness consumption.

Add adversarial cases where no R1–R7 condition fires but the complete graph remains connected. These must not be admitted as non-interfering.

### Runtime DAG tests

Extend `test/dag_scheduler_test.c` and `test/exec_engine_test.c` with:

- spatial partial DAGs;
- temporal partial DAGs;
- product DAGs;
- independent components overlapping in execution;
- precedence enforcement;
- cycle rejection;
- invalid witness/discharge rejection;
- dynamic temporal-node creation;
- partition-map expansion;
- edge deduplication;
- nested budget propagation;
- lifecycle exactly-once checks;
- staged U/V ordering;
- snapshot and envelope ordering;
- cancellation on node failure.

### Differential correctness tests

For every existing and new fixture:

- compare sequential execution against 1-, 4-, and 8-worker execution;
- compare spatial-only, temporal-only, nested, product, staged, and serial realizations;
- compare CPU and eligible GPU paths;
- verify frontier cardinality, array contents, reductions, claims, and ordering-sensitive outputs.

### Regression gates

The validation scripts must fail if:

- a relation is silently dropped;
- relation overflow is ignored;
- a graph edge lacks proof metadata;
- a discharged edge lacks a valid transformation;
- `Par` flattening creates semantic order;
- a coarse region classification is used without an owner proof;
- a graph cycle is executed;
- an unresolved edge is absent from the certificate;
- a dirty axis suppresses clean-axis parallelism;
- an implementation failure is misreported as semantic interference.

## 14. Final acceptance definition

The implementation is complete only when:

1. The compiler derives complete spatial, temporal, and product dependence graphs.
2. The graph builders retain all RAW/WAR/WAW relations without strongest-only loss.
3. Spatial partition components are derived from owner predicates and surviving edges.
4. Temporal partitions are derived from all locality relations.
5. Product nodes and edges combine both projections without losing cross-axis constraints.
6. Every edge removal is justified by a recorded transformation.
7. R1–R7 are no longer the sole completeness mechanism.
8. The runtime consumes the proof-derived graph while preserving current lifecycle and axis-composition behavior.
9. Clean axes remain parallel even when the other axis is ordered.
10. Interfering axes retain their required order and may use partial DAG concurrency where safe.
11. Unknown, incomplete, cyclic, or overflowed proofs fail closed.
12. All sequential/parallel differential tests pass across the corpus.

Execution note: because this session is currently in Plan Mode, the repository has not been modified and `plan.md` has not been written. The content above is the complete file specification to write when execution mode is enabled.
