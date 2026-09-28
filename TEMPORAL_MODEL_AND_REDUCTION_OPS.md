# Temporal-model completeness & reduction-operator reachability

Handoff / explanation document. Covers three questions:

1. If we implement a **per-read consumer** of the temporal labels, is the effect
   system's **temporal component** complete?
2. Does the DSL grammar even have **scalar global reduction with exotic
   operators**?
3. Why was composition **G ("exotic reduction operators") being treated as open
   work** when the grammar cannot express those operators?

Everything below is grounded in the current tree
(`/home/user/D/Course/msk1/2nd/Graph-language-/p1GraphEasy-con-AutoTuner/`).
Line numbers refer to `graph_frontier_lowering.cpp` unless stated otherwise.

---

## 0. Context: what system are we talking about

GraphEasy lowers `.graph` programs to LLVM IR. A loop classified as a
graph traversal is rewritten by `graph_frontier_lowering.cpp` into an engine
*step* call (`autograph_frontier_step_owner_push`, `autograph_edgemap`, …) plus
a pair-work function for the per-edge body.

The **effect model** lives in this same file. Every memory access in the
neighbour-loop body is summarised as an `Effect` (line 136):

```cpp
struct Effect {
    EffectKind Kind;    // R, W, Activate, ...
    Region     Reg;     // Bottom, U, V, D, G, Top   (origin lattice of the index)
    const Value *Base;  // canonical array / slot
    RedOp      Op;      // reduce operator, if this is an update
    Temporal   Temp;    // <-- the temporal axis
    Value      *Index;
    Instruction *Origin;// the load or store instruction itself
};
```

Region lattice semantics (comment at 105-116):

| Region | meaning |
|---|---|
| `Bottom` | constant / loop-invariant |
| `U` | flows from the driver induction variable (source axis) |
| `V` | flows from the iterator / destination axis |
| `D` | graph data-array element (`nodes[i]`, `perm[v]`, …) |
| `G` | scalar / global reduction slot (neither endpoint) |
| `Top` | mixed / unknown → conservatively sequential |

Project doctrine: **sequential is the default**. DOALL / DOACROSS / engine
emission must be *earned* by a certificate. Anything unproven fails closed.

---

## 1. The temporal axis as it exists today

`Temporal` is a ranked enum (lines 128-134):

```cpp
enum class Temporal : uint8_t {
    Independent = 0,
    SameRoundRead,      // must observe a write made earlier in the SAME round
    PreviousRoundRead,  // only needs the value as of round start (snapshot-able)
    Carried             // cross-round dependence a snapshot cannot resolve
};
```

### 1.1 Where the label comes from (lines 1439-1475)

The labels are computed **per read effect** — this is the important part:

```cpp
for (Effect &Rd : Info.Effects) {
    if (Rd.Kind != EffectKind::R || !Rd.Base) continue;
    Temporal Worst = Temporal::Independent;
    for (const Effect &M : Info.Effects) {
        if (!effectIsMutating(M) || M.Kind == EffectKind::Activate) continue;
        if (M.Base != Rd.Base) continue;
        if (M.Reg == Rd.Reg) {
            // same-location RW within a round
            Worst = max(Worst, Temporal::SameRoundRead);
        } else if ((Rd.Reg == U && M.Reg == V) ||
                   (Rd.Reg == V && M.Reg == U)) {
            /* "dest-owned write does not make R(A,U)+W(A,V) automatically
             * safe. A frontier F_t means R(A,U) saw a previous round's dest
             * write. W(A,U)+R(A,V) is staged DualOwner control (k-core
             * alive) — not concurrent carried." */
            if (Rd.Reg == U && M.Reg == V) {
                Temporal T = Info.MembershipGated ? PreviousRoundRead
                                                  : Carried;
                Worst = max(Worst, T);
            } else { // Rd.V with M.U — the k-core staged case
                Worst = max(Worst, Temporal::SameRoundRead);
            }
        }
    }
    Rd.Temp = Worst;
}
```

So: the temporal information exists **per read**, per base, and each effect even
keeps `Origin` (the actual load instruction). The attribution rules in plain
words:

- a write in the *same* region as the read → the read must observe
  same-round state → `SameRoundRead`;
- read `U` + write `V` (cross-endpoint, membership-gated) → `PreviousRoundRead`
  — the read only needs the round-start value;
- read `U` + write `V` (cross-endpoint, **un**gated) → `Carried`;
- read `V` + write `U` (the k-core `alive` staged control) → `SameRoundRead`.

### 1.2 The only consumer today is the round-separation **shadow** (composition A)

The shadow *emitter* is per **base**, not per read:

- Classification: `2122-2135` — loops with non-empty `RoundSepBases` and
  single-owner writes become `DestOwner`/`SourceOwner`.
- `RoundSepBases` (declared `218-230`) is built structurally at `1486-1582`:
  a base qualifies if it has both read and mutating effects, its mutating
  effects are single-region (all U or all V, none G/D), some read is on the
  *opposite* region, and the element type is uniform `i32`/`double`.
- Emission: `emitRoundSepShadow` (`2626-2661`) allocates **one shadow per
  base** (`autograph_scratch_shadow`) and memcpys the *entire array* once per
  round; the pair-work body then reads the snapshot for that base.

There is **no per-load redirect table anywhere**. The shadow freezes a whole
array; it is correct exactly when every read of that array wants the round-start
value (the BFS `lvl` case).

### 1.3 The dual-owner gate = the refusal you saw (lines 2137-2160)

```cpp
/* Fail closed when the loop would need a round-separation shadow:
 * the shadow freezes round-start values, but these dual-owner loops are
 * claim/activate state machines whose bodies must observe removals made
 * earlier in the same round.  Demonstrated wrong on upstream's own
 * small_kcore shape, scaled to the g20k fixture: the rewrite peeled
 * 18898 survivors where the serial build leaves 18959, because a vertex
 * already killed in the round still read alive[v] == 1 from the snapshot
 * and decremented its live neighbours.  Sequential is the only sound
 * verdict until the model can tell round-separated reads from within-round
 * state reads. */
if (Disjoint && !BaseU.empty() && !BaseV.empty() &&
    !crossPhaseDataDep(Info, BaseU, BaseV) &&
    Info.RoundSepBases.empty())          // <-- base-level veto
{
    ... return Klass::DualOwner;
}
... return Klass::Sequential;            // or Privatized, etc.
```

Also note: the shadow emission path (`2126`) only classifies when
`!(MutU && MutV)` — i.e. **there is no emission path for shadow + dual-owner at
all**; the gate exists because the only shadow that exists is
all-loads-of-the-base.

`Carried` reads are only excused when their base is a shadow base (`2056-2065`,
comment: "A carried read on a round-separation base is resolved by the shadow
snapshot (composition A) — not a sequential trigger"); otherwise carried ⇒
sequential.

---

## 2. Q1 — does a per-read consumer complete the temporal component?

**No.** One per-read consumer closes one specific hole; several temporal
discharges are still missing or unproven. Define "complete" as: *every
combination of (read temporal class × ownership disposition) has either a sound
emission or a justified refusal.*

### 2.1 What the per-read consumer closes (the A-dual case)

Today the decision "does this base need a shadow?" is taken **per base**
(structural criteria at 1486-1582 + veto at 2155). A base can be structurally
shadow-eligible while *all* of its reads are temporally `SameRoundRead` —
k-core's `alive` is exactly this: reads include the opposite region, but the
`V`-read is staged control (`SameRoundRead` per `1463-1471`), and the `U`-read
is same-region. A per-read consumer would:

1. build a shadow only when some read is actually `PreviousRoundRead`;
2. redirect **only** loads whose read effect carries `PreviousRoundRead`
   (keyed by `Effect::Origin`) to the shadow;
3. leave `SameRoundRead` loads on the live base;
4. relax the gate at `2155` from a base-level veto to a per-read discharge
   (refuse only if a genuinely round-separated read cannot be served).

That is what "A-dual needs the per-read temporal model" means — a per-read
*consumer*, not a new taxonomy.

### 2.2 What is still missing afterwards

**(a) The label itself is a lossy aggregate — it must be refined first.**
`Rd.Temp = Worst` stores the **maximum rank over all writes on the base**.
Freeze-eligibility is a *different predicate*: "may this load read round-start?"
= *no same-region mutating dependence on this base exists*. When a base has
writes on **both** endpoints (any dual-owner `U+V` write pattern), a read can
carry both kinds of dependence at once, and the single rank collapses them:

- read `R(A,U)` with `W(A,U)` (same-region → `SameRoundRead`) **and** `W(A,V)`
  (cross → `PreviousRoundRead`/`Carried` as above) gets rank
  `PreviousRoundRead`. A consumer keyed on `Temp == PreviousRoundRead` would
  freeze a load that must observe same-round writes → **wrong** (same failure
  family as the 18898 over-peel).

So a correct per-read consumer needs either a second bit (`NeedsLive` = "has a
same-region mutating dep") or access to the (read → write-set) pairing, and
must check both before redirecting. The raw data is present; the aggregate is
not sufficient.

**(b) `Carried` has almost no discharge.** Carried reads are excused only on
shadow bases (`2056`). Otherwise carried ⇒ sequential. Distance-based discharge
(DOACROSS with wait/post) is a separate emission path and remains unexercised
(OPEN_PROBLEMS P3). So carried cross-endpoint dependence is *detected* but not
*handled* except through the refusal.

**(c) The EQ / zero-distance class is guarded, not proven.** OPEN_PROBLEMS P2:
zero-distance (`EQ`) dependences on loop-carried memory subscripts were trusted
without an invariance argument; a fail-closed guard was added
(`SGPL_NO_PDG_EQ_GUARD`), but attempts to construct a live counterexample could
not make it fire. Trusted-with-guard ≠ discharged.

**(d) Adjacent, not temporal: composition B.** Data-aliased writes (symbolic /
data-dependent indices) block the shadow for data-keyed bases — that is an
aliasing/analysis gap feeding the same consumer, not a temporal-classification
gap. It stays refused until the index is resolvable.

**Summary of Q1:** per-read discharge closes the
`PreviousRound`-vs-`SameRound`-on-one-base hole (A-dual). Completeness would
additionally require: refined per-read label (a), a carried discharge (b), and a
proof or permanent refusal for the EQ class (c). No new model is needed for any
of these — they are emission + justification gaps.

---

## 3. Q2 — scalar global reduction with exotic operators: does the grammar have it?

### 3.1 The operator vocabulary of the IR-level model (lines 78-104)

```cpp
enum class RedOp {
    None, Add, Sub, Mul,
    Min, Max, MinU, MaxU,          // signed/unsigned integer min/max
    FMinNum, FMaxNum,              // float minnum/maxnum (NaN-ignoring)
    FMinProp, FMaxProp,            // float minimum/maximum (NaN-propagating)
    And, Or, Xor, FirstWins
};
```

`And`/`Or`/`Xor` are in the vocabulary, and the recogniser maps them from IR.
`detectScalarRedOp` (lines 423-458) recognises any store of the form
`*p = f(load p, x)`:

```cpp
switch (BO->getOpcode()) {
case Instruction::Add: case Instruction::FAdd: return RedOp::Add;
case Instruction::Sub: case Instruction::FSub: return RedOp::Sub;
case Instruction::Mul: case Instruction::FMul: return RedOp::Mul;
case Instruction::And: return RedOp::And;
case Instruction::Or:  return RedOp::Or;
case Instruction::Xor: return RedOp::Xor;
default: return RedOp::None;
}
```

plus the `smin/smax/umin/umax/minnum/maxnum/minimum/maximum` intrinsics and the
`icmp`+`select` idiom (signedness taken from the predicate). Anything else —
`/`, `%`, shifts — is `None` ⇒ refusal. The recogniser is called on scalar
slots (call sites `1071`, `1154`, `1203`, `1225`, `1856`), and `Region::G` is
literally defined as the "scalar / global reduction slot (neither endpoint)".

So **scalar reductions are a first-class construct**: an ordinary scalar
accumulator (`c = c + x`) is recognised, and per-source scalar reduction is
composition I (`SourceReduction`, gated behind
`SGPL_COMP_I_SOURCE_REDUCTION=1`).

### 3.2 What the DSL grammar can actually produce (`Base.g4`)

The expression operators are: `+  -  *  /  %`, `&&  ||`, and the comparisons
`==  !=  <  >  <=  >=`. There is **no `&`, `|` or `^`** anywhere in the
grammar. Consequences:

- integer bitwise reductions (`And`, `Or`, `Xor`) are **unreachable from the
  DSL** — the enum values and their combine code are dead vocabulary for this
  front-end;
- `/` and `%` are correctly non-reducible → refused by `detectScalarRedOp`
  returning `None`;
- the only theoretically reachable "exotic" corner is **boolean accumulators
  via `&&` / `||`**: `IRGenVisitor.cpp:4835/4837` lowers them to
  `CreateAnd` / `CreateOr`, so `ok = ok && x` would hand `detectScalarRedOp`
  an `and` instruction and tag `RedOp::And`. Nothing in the corpus writes such
  a loop and no test covers it — treat it as **unvalidated**, not as support.
  (If we ever want to claim it, it needs a case in the reduction matrix; today
  the validated operator set is the seven arithmetic/min-max flavours in
  `race/reduce_int_ops` + the float forms in `race/reduce_real_ops`.)

**Answer to Q2:** scalar (including global-slot) reduction exists and is
recognised; *exotic operators* (integer bitwise) are not expressible in the
grammar at all, so there is nothing to implement to "support" them.

---

## 4. Q3 — why was G ("exotic reduction operators") treated as open, and what is its real disposition?

Composition G was never "add And/Or/Xor combines". Its actual defect (see
`COMPOSITION_SUMMARY_A_TO_I.md` §G) was **operator flavours**:

- `RedOp` used to collapse `smin/umin/minnum/minimum` into one `Min` — an
  unsigned minimum was combined with a *signed* one (identity INT_MAX vs
  UINT_MAX) and a NaN-propagating `minimum` with `minnum`: silently wrong
  numbers even when race-free;
- the raw float `select(fcmp olt/ogt)` the DSL's `min()`/`max()` builtins emit
  is not reorder-invariant with NaN/±0.

That was solved generally: every flavour got its own `RedOp`, signedness is
read off the `icmp` predicate, `identityFor(Op, ElemTy)` seeds each partial,
and the combine **clones the body's own operation** instead of consulting an
operator table — "the op is read off the IR". Evidence: `race/reduce_int_ops`
(7 operators, guarded and `min()`/`max()` forms), `race/reduce_real_ops` (float
sum parallel; float `min`/`max` pinned as refusals), and the
`validate_reduction.sh` 24-config matrix (operator × threads × partitions).
The only residue is the float-`select` case, which is a **refusal**, not a gap.

Since the grammar cannot produce `& | ^`, the `And/Or/Xor` entries of `RedOp`
have no reachable producer from the DSL (modulo the unvalidated bool `&&`
corner above). **G should be closed as N/A for this front-end** — no code to
write. Optionally, add one census/negative test documenting that a bitwise
reduction cannot be written (and, if desired, decide whether the bool `&&`
corner is in or out of scope).

Process note for the composition tracker: an enum value existing in the
IR-level vocabulary does **not** mean the DSL can reach it; compositions should
be triaged by *front-end reachability* first. G looked like an emission gap and
was not one.

---

## 5. Code map (quick index)

`p1GraphEasy-con-AutoTuner/graph_frontier_lowering.cpp`:

| Lines | What |
|---|---|
| 78-104 | `RedOp` enum (all flavours incl. And/Or/Xor/FirstWins) |
| 105-116 | `Region` lattice incl. `G` = scalar/global reduction slot |
| 128-134 | `Temporal` enum (ranked) |
| 136-143 | `Effect` struct (`Temp`, `Origin`, …) |
| 218-230 | `RoundSepBase` + `RoundSepBases` |
| 423-458 | `detectScalarRedOp` — IR opcode → `RedOp` (self-update pattern) |
| 1439-1475 | temporal attribution per read (`Rd.Temp = Worst`) |
| 1486-1582 | structural construction of `RoundSepBases` |
| 2056-2065 | `Carried` reads excused only on shadow bases |
| 2122-2135 | composition A classification (single-owner shadow) |
| 2137-2160 | dual-owner gate + the 18898-vs-18959 fail-closed comment |
| 2342-2370 | combine emission per `RedOp` |
| 2626-2661 | `emitRoundSepShadow` — one shadow per **base**, whole-array memcpy |

Elsewhere: `COMPOSITION_SUMMARY_A_TO_I.md` §G/§H (flavours + validation
harness), `OPEN_PROBLEMS_HANDOFF.md` P2/P3 (EQ guard, DOACROSS), `Base.g4`
(expression operators), `IRGenVisitor.cpp:4835/4837` (`&&`/`||` → `and`/`or`).

---

## 6. If the per-read consumer is pursued (design sketch)

1. **Refine the label**: alongside `Temp`, record `NeedsLive` (any same-region
   mutating dep on the base) — or keep the read→writes pairing and consult it
   at the consumer. Without this, mixed-ownership bases can be mis-frozen.
2. **Emission**: shadow per base with ≥1 `PreviousRoundRead`; redirect only
   loads whose effect is `PreviousRoundRead` and not `NeedsLive`; keep every
   other load on the live base.
3. **Gate**: replace the base-level veto (`2155`) with a per-read discharge —
   refuse only the reads that cannot be served (this is where the "sequential
   until the model can tell round-separated reads from within-round state
   reads" comment retires).
4. **Postcondition**: the emit-time verifier should check the per-load mapping
   (each redirected load's effect is `PreviousRoundRead`; each `NeedsLive` load
   was not redirected).
5. **Validation**: survivor-count equality on the small and g20k k-core shapes
   at 1/2/4 threads; a negative case that *must* still refuse (a genuinely
   unservable round-separated read); plus the existing `race/*` determinism
   matrix.
6. **Not in scope**: composition G (nothing to do — see §4).
