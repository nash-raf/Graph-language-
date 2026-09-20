# Algebraic Effect System — Design Note

This note is the paper-side companion to the implementation in
`graph_frontier_lowering.cpp` (post-Phase-3 cutover). It states the algebra the
pass implements, separates the two levels (syntactic multiset monoid and
semantic refinement preorder), fixes the exchange-law orientation, records the
derived temporal interpretation and the occurrence-preservation law, and lists
the soundness obligations that remain outside the algebra. Code references are
to the post-cutover revision.

Companion documents:

- `EFFECT_SYSTEM.md` — the implementation deep dive (region lattice through runtime).
- `CLEANCUT_LOOP_SPEC.md` — the user-facing loop/technique spec.
- `proof/CLEANCUT_PARALLEL_PROOF.md` — the end-to-end scaling proof.
- `proof/algebra_golden_classify.txt`, `validate_algebra.sh` — the locked
  classification verdicts and the regression harness.

---

## 1. Architecture

```
LLVM SSA
   │  provenance (external analysis, consumes/does not perform SSA dataflow)
   ▼
region-tagged primitive effects  Σ
   │  expression construction (segments + round domain)
   ▼
effect expressions  e ::= ε | a | e1;e2 | e1‖e2 | e*
   │  algebraic interpretation ⟦·⟧_par
   ▼
{SourceOwner, DestOwner, DualOwner, Reduction, Privatized,
 SourceReduction, Sequential}
   │
   ▼
existing runtime primitives (unchanged)
```

The runtime is not replaced by the algebra: `⟦E⟧_par` selects a composition of
existing runtime mechanisms (`step_U ; step_V`, `privateAccumulate ; combine`,
`pair ; finish`), exactly as before the cutover. What changed is *how the
verdict is derived*: the interpretation is the sole classification path
(`classify()` = `privLayout` + `summarizeEffects` + `interpretPar`,
`graph_frontier_lowering.cpp:2639`), and the former procedural ladder and
pairwise temporal oracle were deleted after the differential harness
(section 8).

---

## 2. Primitive effect domain

**Region boundary (Change 12).** Provenance's codomain excludes `G`:

```
prov : SSA Value → {⊥, U, V, D, ⊤}
```

(`Provenance`, `graph_frontier_lowering.cpp:445-518`; the debug assert in the
fixpoint enforces it, `isProvRegion`). `G` is introduced only while
constructing effects for scalar/global locations:

```
effect construction : location → {⊥, U, V, D, G, ⊤}
```

Primitive effects are drawn from `Σ`:

```
R(A,r)          read of base A at region r
W(A,r)          plain write
U(A,r,ω)        update under operator ω (algebraic data, section 5)
Uf(A,r)         old-value-dependent update with no recognized operator
C(A,r,γ,δ,≺)    ordered conditional claim (guard γ, transition δ, priority ≺)
A⁺(A,r)         activation / frontier effect
```

Each primitive carries an occurrence scope κ and a value source σ:

```
κ ∈ {Once, PerSource, PerPair}          (semantic source: binder path)
σ ∈ {None, Live, Snapshot}              (reads only; None otherwise)
```

(`OccurrenceScope`/`ValueSource`). κ is the iteration domain over which the
primitive is instantiated — derived from the binder path, kept in `Effect` as a
diagnostic projection. σ is the source/lifetime of the value a read observes:
`Snapshot` marks a read resolved by a round-separation shadow snapshot (witness:
membership in `RoundSepBase::CrossReads`), assigned after round separation and
never an input to τ. `PerRound` is retired: round-snapshot lifetime is a
value-source property, not an occurrence scope.

The implementation record is `Effect`: `Kind`, `Reg`, `Base`, `Op`, `Temp`,
`Scope`, `VSource`, `ClaimGuard`/`ClaimTransition` (γ/δ of `C`), `Index`,
`Origin`. The categories `{R, W, Uf, Uop, Claim, Activate}` map onto
`{R, W, Uf, U(·,·,ω), C, A⁺}`.

---

## 3. Effect expressions — syntax level

```
e ::= ε | a | e1;e2 | e1‖e2 | e* | SeqDomain(D,e) | ParDomain(D,e)
D ::= AllVertices | Frontier(t) | Neighbors(u)
```

- `;` is sequential composition: a monoid with identity `ε` (`(e1;e2);e3 =
  e1;(e2;e3)`, `ε;e = e = e;ε`).
- `‖` is **structural** concurrent composition: a commutative monoid with
  identity `ε` and `e‖e ≠ e` (multiplicity preserved).  `Par(e1,e2)` is a real
  constructor, not multiset union: the two children are concurrent, their
  syntactic order carries no meaning, and `Par(e1,e2) ≡ Par(e2,e1)` at the
  semantic layer.  τ never infers an order from Par child position.
- `e*` is finite iteration (grammar-complete; see below).
- `SeqDomain(D,e)` / `ParDomain(D,e)` are iteration binders; the domain is the
  semantic domain with its bound variables.

**Two-level discipline.** The syntactic `‖` (free commutative monoid on
primitives, multiplicity kept) and the semantic refinement order (section 4)
are distinct layers. Standard Concurrent Kleene Algebra uses a subidempotent
`‖` (`x ≤ x‖x`) and KAT order; here the syntax keeps multiplicity and the
refinement order lives in the semantics. The note fixes that separation
explicitly.

**Expression construction** (`buildEffectExpr`) builds the structural tree of
the *original computation* — not the parallelization strategy:

```
E_t = SeqDomain_{u∈F_t}( P(u) ; SeqDomain_{v∈N(u)} B(u,v) ; Q(u) )
```

with `P` the per-source preamble, `B` the per-pair neighbour body and `Q` the
per-source finish.  Both iteration binders are sequential: the serial program
iterates sources and neighbours.  The implementation tree is
`EffNode{Prim, Seq, Par, SeqDomain, ParDomain, Star}` with
`DomainKind{AllVertices, Frontier, Neighbors}`; primitives within a phase are
composed with `Par`, the phase sequence `P ; B ; Q` is the only source of
intra-round order, and each `Prim` node carries its phase-segment identity
`Seg ∈ {Preamble, Pair, Epilogue}` assigned by the builder.

`ParDomain` and `Star` are grammar-complete constructors that the compiler
**never generates** (parallelization is selected by `⟦·⟧_par`; rounds are
explicit sequences).  Debug asserts in `flattenTree` reject them; production
fallbacks are error-handling only and not part of the formal interpretation.

**Flat projection.** The flat `Effect` vector and the three segment multisets
are a derived projection of the tree: `flatten(E) ≡ Info.Effects` **modulo
Par-induced permutation** (multiset equality; a pure reordering is not a
change).  Emission's consumers (`mutatingOriginIs`, etc.) keep reading the flat
multiset; the tree is the semantic source.

**Reduction operators** are algebraic data, `ω = ⟨f_body, N_ω, f_fold, 1_ω,
L_ω⟩` (`RedOpInfo`, with `RedNorm` the normalization: `Negated` for
subtraction, whose contributions are `-x` so the fold uses addition), and
`FoldSound(ω,T)` is the element-type- and machine-dependent predicate that
licenses the fold (`foldSound`; integers always, floats only for the
min/max flavours — float `+/*` re-associate).  Superseding the earlier
four-tuple formulation, the normalization is explicit semantic data.

---

## 4. Refinement semantics

The semantic layer interprets expressions as **pomsets** (partially ordered
multisets of primitive occurrences): `;` adds order between every occurrence
of its operands, `‖` adds none, `*` unfolds to the round sequence. Pomset
semantics is recommended over Mazurkiewicz traces because claims and round
boundaries are order-sensitive; traces quotient by commutation and would erase
the distinction the claim law needs.

Refinement `⪯` is inclusion of permitted behaviours (pomsets): `e ⪯ e'` means
every behaviour of `e` is a behaviour of `e'`. The exchange law is

```
(e1 ‖ e2) ; (e3 ‖ e4)  ⪯  (e1 ; e3) ‖ (e2 ; e4)
```

The orientation is deliberate and consistent with the standard CKA exchange
law `(x ‖ z);(y ‖ w) ≤ (x;y) ‖ (z;w)` when `≤` is read as refinement (fewer
behaviours). The LHS is the rigid two-phase schedule; the RHS interleaves
`e1;e3` with `e2;e4` and admits behaviours such as `e1 e3 e2 e4` that the LHS
forbids. Hence LHS ⪯ RHS. (Papers printing the axiom with the opposite
inequality use KAT order, where larger = more behaviours; the convention must
be stated once and kept.)

**Temporal interpretation is derived, not injected.** `τ(e_r,e_w) =
f(Kind, Base, Region, OccID, BinderPath, Seg, RoundPhase, Domain)` is computed
from the expression tree by `relateReadWrite` and aggregated by
`deriveAllTemporal`; it never consults σ or `RoundSepBases`.  `Seg` is the
phase-segment identity from the tree (never Par child order); in the U/V rule it
is consulted as the **write's** phase.  Oracle-exact table:

| read × mutating primitive | condition | τ |
|---|---|---|
| same base, same region | — | `SameRoundRead` |
| `R(A,U)` vs `W(A,V)` | W in the pair phase, `D = Frontier` | `PreviousRoundRead` |
| `R(A,U)` vs `W(A,V)` | W in the pair phase, `D = AllVertices` | `Carried` |
| `R(A,U)` vs `W(A,V)` | W not in the pair phase | `SameRoundRead` |
| `R(A,V)` vs `W(A,U)` | — | `SameRoundRead` (staged owner control) |
| other cross-region pairs | — | `Independent` |

The specification's "same/different phase segment" is realized by the **write's
phase**: a pair-phase (round-concurrent) dest write is what a frontier-gated U
read observes from the previous round.  `P(u) ; B(u,·) ; Q(u)` is a per-source
sequence, not a per-round barrier — other sources' pair work is concurrent with
every phase — so the read's own segment does not weaken the frontier rule.

> **Correction (errata).** The literal v7 row
> `R(A,U)×W(A,V) ∧ Seg(e_r)≠Seg(e_w) → SameRoundRead` is **incorrect** and is
> superseded by the write-phase condition above.  It arose from misreading the
> pre-v7 `SameSegment` boolean: that flag was write-side (`M ∈ Pair`, set by
> scanning the pair segment), not the equality `Seg(e_r)=Seg(w)`.  Making `Seg`
> explicit turned it into equality and silently reclassified U-reads outside the
> pair phase.  Concretely, for sssp's preamble read `R(dist,U)` vs the pair
> write `W(dist,V)`, `Seg(r) ≠ Seg(w)` holds while `W ∈ Pair`; the frontier
> boundary supplies the value, so the correct verdict — and the one the
> implementation and the G2 per-read τ golden produce — is `PreviousRoundRead`.
> The only reachable divergence between the two formulations is exactly this
> case (`P`/`Q` carry only U-indexed effects, so `W ∉ Pair` is vacuous for
> `V`-writes; pair×pair agrees under both).  The implementation
> (`relateReadWrite`) and the goldens are authoritative; this correction changes
> no code and no golden.

Ranking (`temporalRank`): `Independent < SameRoundRead <
PreviousRoundRead < Carried`.

**Dependency invariant (acyclic).**

```
E ──┬──► pomset ordering
    ├──► τ          (Kind, Base, Region, OccID, BinderPath, Seg, RoundPhase, Domain)
    ├──► RoundSep   (Reads, Mutations, Regions, Bases, ElementType)
    ├──► Occ        (binder paths)
    └──► Φ
RoundSep ──► σ = Snapshot(CrossReads) on reads, Live otherwise, None on non-reads
{τ, RoundSep, Occ, Φ, σ} ──► ⟦E⟧_par ──► NeighborLoopInfo (witnesses)
```

No semantic component may obtain its definition by recursively consulting a
property derived from that component.  In particular there is no edge
`σ→τ`, `τ→RoundSep`, `σ→RoundSep` or `RoundSep→τ`; `validate_refactor.sh`
checks these independence properties mechanically (source ranges and the
goldens).

---

## 5. Reduction operators as algebraic data

A recognized update is represented by

```
ω = ⟨f_body, N_ω, f_fold, 1_ω, L_ω⟩
```

(`RedOpInfo`; lookup `redOpInfo`).  `f_body` is the body update; `N_ω` is the
normalization applied to each contribution (`RedNorm`: `Identity`, or
`Negated` for subtraction, whose contributions are `-x`); `f_fold` combines
partition partials; `1_ω` is the identity (`identityFor`); `L_ω` records the
laws `Assoc`/`Comm`/`Idempotent`.  The minimal law for ordered partition
folding is associativity; commutativity is recorded but only needed when
contributions may be permuted.  The fold-homomorphism obligation

```
fold_ω(concat(S_1, …, S_P)) = fold_ω(fold_ω(S_1), …, fold_ω(S_P))
```

is gated by `FoldSound(ω,T)` (`foldSound`) — element-type- and
machine-dependent, not an intrinsic operator field: integer arithmetic is
exact modulo the width (associativity is the whole obligation), while floats
admit only the idempotent/commutative min/max flavours (float `+/*`
re-associate and are refused).

---

## 6. Interpretation ⟦·⟧_par

`EffectSummary` is the fold of the expression into the predicates the laws are
stated over (`MutU/V/G/D/Top`, `HasUopG`, `HasUnrecognizedG`,
`HasCarriedOnMut`, `BaseU`/`BaseV`); `summarizeEffects` computes it as the
recursive homomorphism Φ over the tree (⊙_seq / ⊙_par / ⊙_dom — idempotent
boolean joins and set unions, so fold order is immaterial), with a defensive
fallback to the raw set.  Temporal facts are not reconstructed from Φ alone:
`HasCarriedOnMut` is a projection of the order-sensitive τ layer, with the
shadow exemption read from σ=Snapshot.  `interpretPar` is the interpretation
function `⟦·⟧_par : E → K`; every rule is stated over the tree/Φ:

1. data-write (`HasDataWrite`) and not `Priv` → `Sequential`.
2. per-source claim present and not `SGPL_COMP_F_ALLOW_DRIVER_CLAIM` →
   `Sequential` (occurrence preservation, section 7).
3. `MutTop` → `Sequential`.
4. carried read on a mutated base and not `Priv` → `Sequential`.
5. round-separation bases, single write region, agreeing `WritesV` →
   `DestOwner`/`SourceOwner`.
6. `MutU ∧ MutV ∧ ¬MutG`: disjoint `BaseU ∩ BaseV = ∅`, no cross-phase data
   dependence, no round-separation base → `DualOwner` (frontier append
   requires the envelope); else `Priv` → `Privatized`; else `Sequential`.
7. per-source gather gate (env-enabled) → `SourceReduction`.
8. frontier append: envelope wired and `MutV` only → `DestOwner`, else
   `Sequential`.
9. single recognized scalar accumulator → `Reduction`.
10. `MutV` only → `DestOwner`; `MutU` only → `SourceOwner`.
11. `Priv` → `Privatized`; otherwise `Sequential`.

**Privatization theorem premises** (`privLayout`; expression premise
`exprHasClaimOrActivate`):

1. no claim, activation, or non-preserved occurrence in the expression;
2. every mutating primitive is an algebraic update `U(A,r,ω)` with ω known;
3. every written base has a single compatible operator;
4. every load from a written base is part of that base's old-value chain and
   reaches at least one update;
5. the fold law holds for ω on the element type.

The transformation: per-partition private state `P_i(A)` initialized to `1_ω`,
`P_i(A) = fold_ω(S_i)`, then `A = fold_ω(A_old, P_1(A), …, P_P(A))` in
ascending partition order.

**Source reduction** is the composition `E_pair ; E_finish` with
`κ(E_finish) = PerSource`: the pair work accumulates into a partition-local
partial and the finish hook consumes it exactly once per source, including
sources with no pairs (occurrence check `preambleMutationCount` over binder
paths; no remaining Preamble mutation).

---

## 7. Occurrence preservation and translation totality

**Occurrence preservation.** For every rewrite `P ⇒ P'`,

```
Occ_P(e) ≅ Occ_P'(e)
```

up to the explicitly permitted algebraic transformation. In particular
`PerSource ≢ PerPair` unless a separate theorem establishes the equivalence.
This is the algebraic form of the driver-claim refusal (composition F): a
first-wins claim in the driver preamble runs once per source in the serial
program, but every emitted work function is called once per pair. κ is derived
from the binder path (`Preamble`/`Epilogue` under the round binder are
per-source, `Pair` under `Neighbors(u)` is per-pair); the implementation
predicates `hasPerSourceClaim` and `preambleMutationCount` are binder-path
queries, and `SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1` restores the old behaviour for
A/B.

**Translation totality — Modelability(P).** `EffectExpr::Total` is removed:
modelability is a separate compiler judgment, not a component of the algebra.
`provesModelable` is the implementation: every instruction must map into the
effect language, or the loop is refused.  Effect construction is defined only
for modelable programs (conceptually `Modelable(P) ⊢ buildEffectExpr(P)=E`,
then `⟦E⟧_par`); the pass still classifies refused loops for diagnostics, and
the rewrite gate is `(K ≠ Sequential) ∧ Modelable`.  The blocklist form
(`legacyBlocklistRefuses`) remains only under `SGPL_FRONTIER_BLOCKLIST_GUARD`
for A/B.

**First-wins claims.** `C(A,r,γ,δ,≺)` is an ordered conditional primitive: the
winner is the first successful claim under the serial priority ≺ the
owner-computes runtime discharges (one worker per destination scans sources in
CSR order).  The guard γ and transition δ are carried on the effect
(`ClaimGuard`/`ClaimTransition`; `detectFirstWinsClaim`; CAS form).  The
global law `C1;C2 = C1` is never imposed — it is invalid for claims on
different locations.

---

## 8. Soundness obligations outside the algebra

Four boundaries remain explicit (totality included):

1. **Provenance soundness.** The transfer rules (`Provenance`) must be a sound
   abstraction of SSA values; the algebra consumes `prov(x) ∈ {⊥,U,V,D,⊤}`
   (never `G`) but does not replace the dataflow proof. Precondition: mem2reg
   has run; the escaping v-slot alloca is handled by the explicit load rule.
2. **Modelability.** `provesModelable` — the program must be representable in
   the effect language; construction is defined only under it (section 7).
3. **Runtime discharge.** The algebra *states* `∀v. ∃!p. owner(v) = p` and the
   claim total-order obligation; the runtime *discharges* them by construction
   (home partitions `p = v·P/n`, one worker per partition, per-partition
   serial scans; `autotuner_runtime.c`). These are proof obligations on the
   runtime, not consequences of the algebra.
4. **Emission validity.** The algebra cannot prove that LLVM cloning,
   rebinding, control-flow construction and driver deactivation produce valid
   IR; `verifyFunction` remains a mandatory postcondition, with
   `SGPL_FRONTIER_STRICT=1` making failure fatal.

---

## 9. Evidence

- **Phase-3 differential harness (pre-refactor).** Every `*.graph` fixture (64
  files) was compiled twice — `SGPL_ALGEBRA_ONLY=1` (interpretation only) and
  `SGPL_LEGACY_ONLY=1` (former ladder) — and the candidate lines compared:
  **0 mismatches**; the derived τ matched the retired pairwise oracle on the
  same sweep.  The gates and the legacy ladder were then removed.
- **Golden harness.** `proof/algebra_golden_classify.txt` locks the verdicts
  for 21 acceptance fixtures; `validate_algebra.sh` reruns them and requires
  byte-identical candidate lines: **21 fixtures, 0 failures**.
- **Refactor gate (v7).** `capture_refactor_goldens.sh` captured, before the
  refactor, every emission-relevant `NeighborLoopInfo` field, the per-effect
  semantic facts (kind/base/region/κ/σ/τ) and the candidate lines for the 21
  acceptance fixtures and all 64 small fixtures, plus emitted-IR dumps for a
  representative subset.  `validate_refactor.sh` re-derives them after the
  refactor and requires byte equality, and additionally checks the dependency
  invariant mechanically (τ excludes σ/RoundSepBases, RoundSep excludes τ, the
  σ assignment excludes τ): **Stage A and Stage B both PASS, 0 failures**.
- **Acceptance suites** (`validate_reduction.sh`,
  `validate_composition.sh`, `validate_roundsep.sh`) pass end-to-end with
  bit-identical results at 1/3/8 partitions, after the refactor.

---

## 10. Open items

1. **Exchange-law proof** for the chosen pomset semantics (section 4) — the
   orientation is fixed; the refinement proof is open formal work.
2. **Claim total-order discharge**: prove that the runtime's per-partition
   serial scan realizes the serial priority ≺ for same-cell claims (currently
   argued structurally; the claim law states the obligation).
3. **Provenance soundness** proof for the transfer rules (transfer is
   conservative by construction; a mechanized argument is future work).
4. **Multiplicity arithmetic under `*`**: occurrence scopes are grades, not
   expressions; a rewrite that changes loop nesting must re-establish the
   grade mapping (the `perSource ≠ perPair` law is the first instance).
   `Star` is grammar-complete but never generated by the current compiler.

---

## 11. Composable runtime (R1–R5)

The runtime exposes **no second expression language**.  `EffNode` remains
compiler-side; the compiler interprets it into a flat execution context plus
executable operation descriptors, and the runtime executes that.  The
operational ABI (in `autotuner_runtime.h`):

- `sgpl_exec_ctx`: round resources (graph, membership, dest_seen,
  next_frontier, atomic append head, round state), traversal mechanism
  (`SGPL_TRAVERSE_OWNER_U` slices / `OWNER_V` rows), domain kind, the flat op
  array, per-partition partial base/stride, round-ownership flags.
- `sgpl_runtime_op`: capability mask (`PAIR`, `SOURCE_BEGIN/END`,
  `PARTITION_BEGIN/END`, `ROUND_BEGIN/END`, `COMBINE`), resource access list
  (`Read`/`Write`/`AtomicWrite`/`Private` over the resource classes
  Membership/DestSeen/NextFrontier/Snapshot/Partial/Private/Claim), state, and
  the eight lifecycle callbacks.  The hot callbacks (`pair`, `combine`) take
  `state` as their first argument so the compiler can emit the cloned work
  function as the callback itself; the descriptor-first lifecycle callbacks are
  per-source/per-partition.
- `autograph_frontier_execute(graph, ctx)`: one generic executor (membership
  gate; per-partition ascending combine; round flags; no semantic work in the
  engine).  `autograph_frontier_fork_join(graph, owner, a, b)` runs two
  incompatible `Par` children concurrently on host threads with a join barrier;
  forked contexts are non-owning round contexts.  `autograph_exec_*` are the
  layout-free construction/accessor helpers the emitted code uses.

Semantics and invariants fixed by the design:

- **Activation is an explicit operation**: `A⁺` calls
  `autograph_frontier_activate(ctx, v)` (CAS `dest_seen[v]` 0→1, then atomic
  append).  The engine never activates.
- **Source lifecycle coverage**: `OWNER_U` carries begin/end inside the
  partition bodies (each source belongs to one partition range, zero-pair
  included).  Under `OWNER_V`, where a source's arcs are split across
  partitions, the executor runs membership-gated pre/post coverage passes over
  the full source domain, so every source observes exactly one
  `source_begin(u)` and one `source_end(u)`, deterministically ascending.
- **Reduction/privatization are operations**: `Reduce(ω)` accumulates into
  partition partials exposed as `partition_state` (per-worker context copy);
  `Combine(ω)` folds in ascending partition order — associativity is the law,
  permutation invariance is never assumed.  Strategy names are semantic
  aliases, not runtime entry points.
- **Frontier contract**: `F_{t+1} = {v | dest_seen[v] = 1}` — unordered set;
  physical append order is an implementation detail; order-sensitive
  primitives are discharged by claim-order ownership or refused.
- **Round ownership**: exactly one enclosing executor runs `RoundBegin`/
  `RoundEnd`, snapshot publication and envelope commit per round; forked
  children are non-owning.

**Compiler status (R5 cutover).** The direct expression path is the sole emitter.
The compiler emits runtime ops and execution contexts for every classified shape:
`SourceOwner`, `DestOwner`, `Reduction`, `SourceReduction`, `Privatized`, `DualOwner`
(two staged contexts sharing one round), activation/envelope (`source_begin`/`A⁺`/
`commit`) including round-separation snapshots, and pair-body first-wins claims
(occurrence-preserved by the partition schedule).  Per-source driver claims and
reduction/privatization fused with activation remain refused and fail closed to the
sequential marker.  The legacy strategy emitters and the
`autograph_frontier_step_owner_*` runtime entry points were deleted after a
corpus-wide refusal census (68 fixtures, zero refusals) reported full coverage.

**Verification.** `test/run_exec_engine_tests.sh` (lifecycle, barriers, combine
order, no implicit activation, traversal enumeration), `test/run_exec_r2_tests.sh`
(OWNER_V exactly-once source coverage incl. zero-pair/membership; fork/join with
concurrent activation, a parent-holds-all-workers watchdog and a balanced budget
ledger), `validate_rt_expr.sh` (partition-count determinism over the reduction
fixtures, source-reduction/private-array/first-wins activation fixtures and a local
pagerank fixture; before the cutover the same script was the legacy differential),
plus `validate_reduction.sh`, `validate_roundsep.sh`, `validate_composition.sh` and
the ported `test/run_frontier_red_tests.sh` / `run_frontier_shadow_tests.sh` — all
passing.  The analysis-side gates (`validate_algebra.sh`, `validate_refactor.sh`)
keep classification, witnesses and statistics byte-identical.

**Performance (R6).** The hot path was restructured so the emulated engine does not
add call depth over the legacy steps:

- `pair`/`combine` op callbacks take the operation's `state` directly
  (`void (*pair)(void *state, sgpl_exec_ctx *, int32_t, int32_t)`); the compiler
  emits the cloned work function *as* that callback (no emitted adapter, no
  per-pair `autograph_exec_op_state` call).  Lifecycle callbacks keep the
  descriptor-first ABI (they are per-source/per-partition).
- `sgpl_exec_partition_body` hoists the single pair operation's callback and state
  into locals before the row/slice loops (compiler-emitted contexts carry exactly
  one pair op) and `sgpl_exec_pair_ops` has a one-op fast path; per-pair dispatch
  is one indirect call with register-held callback/state.
- Reduction/privatization still derive the partition partial from the context, so
  their pair callbacks pay one accessor call per pair (legacy computed the partial
  once per partition in the engine loop); the hoist keeps the simple/activation
  classes at parity.

Measured on a 300k-vertex / 3M-edge synthetic graph (8 threads, 20 rounds, WSL
Ubuntu, gcc -O3; `cc_dir_bench.c` after its port to the exec ABI, and a compiled
DSL pagerank):

| workload | legacy (pre-R5) | direct expression path |
|---|---|---|
| `cc_dir_bench` pagerank rounds | 0.284–0.287 s | 0.247–0.249 s |
| compiled pagerank (wall) | 1.175–1.229 s | 1.117–1.133 s |

Outputs are identical (`DEG_MATCH=YES RANK_MATCH=YES`; same printed ranks).  The
direct path is 7–13% faster in these runs: the descriptor hoist plus the
precomputed partition ownership offsets replace the legacy engine's per-pair
`dest_seen`/append checks with the activation primitive's CAS only where a claim
is actually made.

---

## 12. Totality contract (R7)

**Invariant.**

\forall P\in\mathcal L_{\mathrm{grammar}},\quad \mathsf{Modelable}(P)\;\Longrightarrow\;\mathsf{Lowerable}(P)\;\land\;\mathsf{Executable}(P).

A modelable effect expression may not fall back to a sequential marker merely
because the emitter lacks a composition path.  The only legitimate refusals are
semantic: (1) the program is not modelable; (2) the effect is outside the frozen
algebra; (3) the requested semantics violate a proven correctness condition
(order-sensitive primitives, unsound folds, carried reads on mutated bases, the
frontier contract of §11, ParDomain/Star which the grammar never generates).

**Architectural rules.**

- Every runtime mechanism implements an existing algebraic primitive or
  composition rule; a combination of primitives never creates a runtime concept.
  DualOwner, Reduction+Activation, Privatized+Activation,
  DualOwner+Snapshot, DualOwner+Claims are compositions, never primitives.
- The compiler recursively interprets EffExpr into execution contexts and
  operation groups: E \xrightarrow{\llbracket\cdot\rrbracket_{\mathrm{rt}}} sgpl_exec_ctx.
  There is exactly one semantic composition language (EffExpr); no StepPlan /
  RuntimePlan / sgpl_eff_node is introduced.
- Seq(e1,e2) executes with a barrier between the two realizations;
  Par(e1,e2) is realized by a shared execution context when the children are
  compatible (access-mode predicate) or by `fork_join` otherwise; Par is never
  silently serialized.
- Strategy names are derived semantic descriptions only; nothing named after a
  strategy determines runtime control flow.
- The frontier contract stays frozen: F_{t+1} = {v | dest_seen[v] = 1}; ordered
  or duplicate-preserving worklists are outside the algebra.

**Correctness invariant.** For every admitted program there exists an
ExecOps(P) with

\operatorname{atoms}(\operatorname{ExecOps}(P)) \equiv \operatorname{flatten}(E_P)\pmod{\text{Par}}

(each fused group expands back to the primitives it implements), every
dependency edge e_i \to_{E} e_j is preserved by the realization, and

\operatorname{Exec}\!\left(\llbracket E_P\rrbracket_{\mathrm{rt}}\right) \preceq \llbracket E_P\rrbracket_{\mathrm{pomset}}.

**Status.** The runtime ABI (R1-R2 of the revision) is complete: generic
executor, operation ABI with resource/access contracts, activation,
reduction/combine, source lifecycle incl. Owner-V coverage, fork/join with
unique round ownership (nesting allowed), the Snapshot operation, the
source-scoped state channel, and the frozen frontier contract.  R7 removed
classification entirely: `privLayout` + `supportedByAlgebra` is the semantic
admissibility gate, and the recursive structural interpreter
(`emitExprInterp`/`emitSingleStage`/`emitDualForkJoin`) is the sole emitter.
Independent U-domain and V-domain subexpressions are realized by the generic
fork/join mechanism; round-separation copies are Snapshot operations; domains
are emitted explicitly (`SGPL_DOMAIN_FRONTIER`/`SGPL_DOMAIN_ALL_VERTICES`).

Per-source first-wins claims are staged: the claim runs once per source as a
SOURCE_BEGIN operation (the Owner-V coverage pre-pass, or the inline source
change under Owner-U), performs the guarded transition and publishes the
outcome per source; the pair body is gated on it.  The post-cutover census
(`validate_totality.sh`, 69 fixtures) reports 30 lowered, 2 genuine algebra
refusals (same-base or cross-phase U+V without privatization), 1 unmodelable
fixture and **0 interpreter refusals**.

**Completion (final R7 pass).**  The remaining combination gaps are closed
generically:

- **Activation is orthogonal to the body environment**: the `A+` primitive
  always reads the runtime context, so `Reduce;A+` and `PrivateAccumulate;A+`
  fuse into one pair callback that carries the partition/private body
  environment and activates from the context.  New fixtures
  `rt_expr_reduce_activate` / `rt_expr_priv_activate` match their references.
- **Per-source claims are general and composable**: all store-form claims with
  constant guard/transition stage as SOURCE_BEGIN operations into a per-source
  state array `S[u][j]` (one slot per claim); only claims whose guard dominates
  the neighbour loop gate the pair body.  Under dual ownership the claim ops
  live in the fork/join owner and the runtime runs the owner's source-domain
  coverage before either child, so `claim(u) ≺ pairs(u)` holds across both
  domains (`rt_expr_dual_claim` matches the sequential reference).
- **Resource/access contracts are emitted and enforced**: every operation
  carries a module-constant `sgpl_res_access[]` (membership Read, dest_seen /
  next_frontier AtomicWrite, snapshot Read|Write, partial/private Private,
  claim AtomicWrite); the runtime validator enforces the mode rules, and the
  fork/join lowering checks child compatibility from the contracts
  (`resourcesConflict`) — a conflict is never resolved by serializing `Par`.
- **The atom census is mechanical**: `SGPL_EXEC_DUMP=1` prints canonical atom
  keys per flattened effect and per executable op group; `validate_totality.sh`
  requires exact multiset equality `atoms(ExecOps(E)) == flatten(E)` and proves
  `cas=0` (no CAS-form claims in the target frontend configuration) across all
  72 fixtures.