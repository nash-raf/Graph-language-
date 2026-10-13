# Two-slot privatization (R3) and float reduction regressions in the merged `ARS-1` state

> **STATUS 2026-10-13:** resolved upstream in `b95fbd1 "serial fallback fix"` for `two_reduce_slots` and `priv_two_slots_big` (both now `privatized`, and the stale fixture comment was rewritten to the R3 contract). The `reduce_real_ops` half (`rsum` refused by the spatial rule) was still open at that time — the `run.sh` row passes per the corpus tally, see `verify/PARALLEL_FAILURES_ANALYSIS.md`.


Status: **measured, root-caused, not yet fixed.**
Scope: three verdict assertions that hold on our grammar branch and fail after merging the
`spatial temporal interference proofs` commits (`f01c683`, `1c383ed`).
Date of measurement: 2026-10-13. Tree measured: `origin/ARS-1` = `f4010d5 "grammar updated"`.

---

## 1. Summary

Three fixtures assert **parallelization conditions** (the effect-analysis verdict per loop).
On the merged/pushed tree all three come out `class=sequential`, while the project's own
design docs and the harness assertions say they must be `privatized` / `reduction`:

| fixture | our branch (`64f6926`) | merged/pushed (`f4010d5`) | what the project says it should be |
|---|---|---|---|
| `two_reduce_slots`   | `privatized` | `sequential` | `privatized` — `priv/two_reduce_slots` expects `a 10 b 20` |
| `priv_two_slots_big` | `privatized` | `sequential` | `privatized` — expects `a 160000 b 2068247825` |
| `reduce_real_ops`    | `reduction`  | `sequential` | `reduction` for `rsum` (and `sequential` for `rmn`) |
| *(controls)* `reduce_int_ops`, `int_gather` | `reduction`, `source-red` | `reduction`, `source-red` | unchanged — same on both trees |

The delta is **not** the grammar: both trees in this comparison have **byte-identical
`Base.g4`, the same generated parser, and the same (migrated) fixture sources** —
verified with `diff -q` before measuring. The only difference is the proof layer that
arrived with the merge.

Root cause, located: a **new spatial refusal rule introduced by the proof commits**
takes precedence over the R3 privatization realization and refuses the two-slot shape
before R3 can map the slots to per-partition partials.

---

## 2. What the behavior *should* be, and where that is stated

The project states the intended contract in several places; they are not all in agreement,
so the citations are quoted with their exact locations.

**(a) The R3 write-up — the authoritative current contract.**
`COMPOSITION_R3_PRIVATIZATION.md` (introduced by `cc7b4e0`, *"composition R3+D: per-source
preamble phase, and the R3 write-up"*):

- line 1 — *"Composition R3: privatization — the shapes B, C and D, and how they were fixed"*
- line 77 — *"(initialized to their operator identity, per partition), and every privatized …"*
- line 126 — implementation sites: `graph_frontier_lowering.cpp` | `Klass::Privatized`; `privLayout()` proof + `privLayoutNeeded()`; `sameAddressValue()`; R3 record mapping in `emitPairWorkFn` (slots + base redirection); `emitPrivScalarInit()`; `emitPrivCombiner()`; `emitPrivatizedStep()` …; refusal reasons under `GRAPH_FRONTIER_STATS=1`
- **line 139 — `| two_reduce_slots | privatized | a 10 b 20 | 1, 4 | ✅ GRAPH_FRONTIER_REWRITE_OFF |`**
- line 140 — `| data_index_write | privatized | cntsum 10 | 1, 4 | ✅ |`
- line 129 — *"`verify/run.sh` | class assertions updated to `privatized`; every privatized case
  now also builds with `GRAPH_FRONTIER_REWRITE_OFF=1` and requires the outputs to be **equal**"*

**(b) The solved-problems table — two slots declared solved in general.**
`PROBLEMS_AND_SOLUTIONS.md:400`:

> `| **extra** | two scalar accumulators in one body | **solved-general (R3)** | privatization gives every slot its own private storage; the legacy engine's single ReducePtr limitation disappears | two_reduce_slots: a 10 b 20; priv_two_slots_big: a 160000 b 2068247825, 1/4 threads, serial-equality ✅ |`

**(c) The harness assertions.**
`verify/run.sh`:

- line 710 — `expect_class two_reduce_slots privatized`
- line 715–718 — numeric check `a 10 b 20` **and** `== serial` for 1 and 4 threads
- line 752 — `expect_class priv_two_slots_big privatized 'red=1'`
- line 812 — `expect_class reduce_real_ops reduction 'rsum'`
- line 813 — `expect_class reduce_real_ops sequential 'rmn'`  ← the min/max nest is *intended* sequential

**(d) Stale, contradictory text (historical — pre-R3).** These describe the *old* engine and
now contradict (a)–(c); they are the source of the confusion and should be corrected or
marked historical:

- `verify/cases/parallel/two_reduce_slots.graph:2-4` —
  *"the second slot has no partition storage, so under the rewrite every partition would write
  the same global (lost updates). **The nest must be refused (sequential)** …"*
- `COMPOSITION_STATUS.md:34` — `second scalar accumulator … | **refused-general**, parallel side
  unsolved | … | race/two_reduce_slots (class=sequential …) | give the reduction engine a *set* of
  slots (partials per slot per partition) instead of one ReducePtr`

Chain of events visible in the repository: `COMPOSITION_STATUS.md:34` describes the state
*before* R3; `cc7b4e0` (R3 write-up) and `PROBLEMS_AND_SOLUTIONS.md:400` declare the two-slot
shape **solved-general**; `verify/run.sh:710/752` assert `privatized`. The fixture comment in
`two_reduce_slots.graph:2-4` was left at the pre-R3 wording, which is why the tree appears to
contradict itself. **The current contract is (a)+(b)+(c): two-slot nests must privatize.**

---

## 3. What the merged tree actually does

Captured with `GRAPH_FRONTIER_STATS=1` on `f4010d5`, one fixture at a time:

```
########## two_reduce_slots
schedule=serial emitted=1 impl_failure=0 reason=unconditional shared write connects every pair occurrence
modelable=1
candidate: main driver=foreach.cond inner=foreach_nbr.cond kind=0 red=1 sep=0 data=0 fw=0 env=0 shadow=0 \
  class=sequential  U+(a,G) ⊗ U+(b,G)  temporal=Independent [refused: ]

########## priv_two_slots_big
schedule=serial emitted=1 impl_failure=0 reason=unconditional shared write connects every pair occurrence
candidate: main … class=sequential  U+(a,G) ⊗ U+(b,G)  temporal=Independent [refused: ]

########## reduce_real_ops
schedule=serial emitted=1 impl_failure=0 reason=unconditional shared write connects every pair occurrence
candidate: main driver=foreach.cond inner=foreach_nbr.cond … class=sequential  U+(rsum,G)  temporal=Independent [refused: ]
schedule=serial emitted=1 impl_failure=1 reason=R8 guard: reduction realization
candidate: main driver=foreach.cond12 inner=foreach_nbr.cond24 … class=sequential  U_f(rmn,G)  temporal=Independent [refused: reduction conditions not met]
```

Note the shape flags on the two-slot lines: `red=1 sep=0 data=0` — the analysis *does* see the
reduction (`red=1`) and the scalars are not marked private, so the write is treated as a shared
write and the nest is refused before R3's per-slot partials come into play.

**Who introduced the rule that fires.** The refusal string exists only in the merged trees:

```
  f4010d5 (pushed, merged) 'unconditional shared write connects every pair occurrence': 1
  1c383ed (their proof commit)                                                        : 1
  64f6926 (our grammar branch)                                                        : 0
```

Site, `p1GraphEasy-con-AutoTuner/spatial_partition_proof.inc:466-486` (pushed tree):

```cpp
bool SharedLifetime = A.Base && !isa<Instruction>(A.Base);
if (auto *Allocation = dyn_cast_or_null<AllocaInst>(A.Base))
    SharedLifetime = !Info.DriverLoop->contains(Allocation);
/* A pointer slot changed by the nest does not denote one fixed
 * address across all occurrences, even when the GEP is constant. */
for (const auto &Other : P.Accesses)
    if (Other.Write && Other.Pointer && Other.Pointer->stripPointerCasts() == A.Base)
        if (Other.Origin != A.Origin) SharedLifetime = false;
if (SharedLifetime && A.Family == 0 && A.Write && !A.Private && !A.Resource && ConstantAddress &&
    Info.NeighborLoop->getLoopLatch() && DT.dominates(A.Origin, Info.NeighborLoop->getLoopLatch()->getTerminator())) {
    P.Status = SpatialProofStatus::NoPartition;
    P.Reason = "unconditional shared write connects every pair occurrence";
    break;
}
```

So the rule concludes "no partition can own this write" and returns `NoPartition`; the
reduction path never gets to realize the R3 privatization.

The second refusal, for the float min/max nest, is the R8 implementation guard
(`p1GraphEasy-con-AutoTuner/graph_frontier_lowering.cpp`):

- line 6405 — `RecordRefusal("reduction conditions not met");`
- line 6460-6464 — `if (C.R8) { Ch.Reason = "R8 guard: " + C.ImplementationReason; Ch.ImplementationFailed = true; return Ch; }`

R3 code itself is **present** in all three trees (marker count for
`Klass::Privatized|privLayout|emitPrivCombiner|emitPrivatizedStep`:
`f4010d5` = 14, `1c383ed` = 14, `64f6926` = 12) — the capability was not deleted, it is being
**refused earlier** by the new spatial rule.

---

## 4. Per fixture: what it is, what it should be, why the current verdict is wrong

### 4.1 `two_reduce_slots` — small, two scalar accumulators in one neighbour body

- Program: `graph G { edges: file "../verify/fixtures/tiny.txt"; };` then
  `for u in G.nodes { for v in G.neighbors(u) { a = a + 1; b = b + 2; } }`.
- Intent per the current contract: **`privatized`**, output `a 10 b 20`, identical to the
  serial build, for 1 and 4 threads (`run.sh:710,715-718`; `COMPOSITION_R3_PRIVATIZATION.md:139`).
- Why `sequential` is wrong: R3 gives **every slot its own private storage**
  (`PROBLEMS_AND_SOLUTIONS.md:400`), so the legacy "second slot has no partition storage"
  limitation no longer applies. The refusal produced by the new spatial rule therefore
  contradicts the R3 write-up and the assertions.
- Why it matters numerically: `b` is the discriminator — a lost update shows as `b ≈ 20/partitions`
  instead of 20. The serial fallback is *safe* (the answer is still 20), so this is a lost
  parallelization, not a wrong answer.
- Caveat to record: the fixture's own comment (`two_reduce_slots.graph:2-4`) still states the
  pre-R3 intent ("must be refused") and `COMPOSITION_STATUS.md:34` calls it "refused-general,
  parallel side unsolved". Those two texts are stale and should be updated/marked historical,
  otherwise the tree contradicts itself (see §2(d)).

### 4.2 `priv_two_slots_big` — the same two-accumulator shape at scale

- Program: directed `g20k.txt` (160000 arcs), `a = a + 1; b = b + v;` in the neighbour nest;
  `b` accumulates neighbour ids so a lost update shows as a short, thread-dependent sum.
- Intent per the current contract: **`privatized` of both slots** (filter `red=1`),
  expected `a 160000 b 2068247825`, 1/4 threads, serial-equal (`run.sh:752,760`).
  The fixture comment says it explicitly: *"Both slots now get their own partial, and the
  emitted combine folds them per operator."*
- Why `sequential` is wrong: this is precisely the capability the comment describes, and the
  pre-merge engine delivered it (`privatized` on `64f6926`). The merged engine refuses the
  shape with the new spatial rule, so the documented capability is unreachable.
- This is the strongest of the three: fixture text, harness assertion, R3 write-up and the
  solved-problems table all agree, and the pre-merge measurement shows the engine could do it.

### 4.3 `reduce_real_ops` — float FAdd reduction plus min/max nests

- Program: `real rsum = 0.0;` accumulated with `+` (FAdd) over the arcs of `g20k.txt`;
  `rmn`/`rmx` use the `min`/`max` builtins on reals (`select` + `fcmp`, possibly
  canonicalized to `llvm.minnum`/`maxnum`).
- Intent per the current contract: **`reduction` for `rsum`** (`run.sh:812`) and
  **`sequential` for `rmn`** (`run.sh:813`) — the assertion set is per-loop, and the fixture
  comment's *"the nest is classified `reduction`"* refers to the `rsum` nest.
- What the merged engine does: `rsum` is refused by the same new spatial rule
  (`unconditional shared write …`), and `rmn` is refused by the R8 implementation guard
  (`R8 guard: reduction realization`) — which matches the expected `sequential` for `rmn`,
  so the `rmn` half is fine.
- Why the `rsum` verdict is wrong: an ordered combine is exact here (the fixture pins
  `1thr == 4thr` bit-for-bit and an independent Python value), and the pre-merge engine
  classified it `reduction`. Refusing it is over-conservative.
- Alternative reading to rule out: if float FAdd is *deliberately* refused now for
  associativity/NaN reasons, then `run.sh:812` and the fixture comment must change together —
  but nothing in the docs states such a decision, while the R8 guard's own wording
  ("reduction realization") indicates an implementation gap rather than a semantic refusal.

---

## 5. Why the grammar/migration is not the cause (so the fix goes to the proof layer)

- `Base.g4` is byte-identical between the two trees compared here (`diff -q` → identical),
  and the generated parser used by both is the same generation.
- The fixture sources are the same files (our branch already contains the migrated
  `for v in G.nodes` / `for v in G.neighbors(u)` spellings).
- The verdicts flip exactly with the proof-layer commits:
  `unconditional shared write connects every pair occurrence` is present in `1c383ed`/`f4010d5`
  and absent in `64f6926`, and the refusals reported on the merged tree come from that string
  plus the R8 guard.
- Control fixtures that do **not** trip the new rule are unchanged (`reduce_int_ops` →
  `reduction`, `int_gather` → `source-red` on both trees).

---

## 6. How to reproduce every number in this document

```bash
# pre-merge branch (ours), built in a worktree:
cd /home/user/D/Course/msk1/2nd/Graph-language-
git worktree add /tmp/premerge 64f6926
cp -r p1GraphEasy-con-AutoTuner/generated /tmp/premerge/p1GraphEasy-con-AutoTuner/
cd /tmp/premerge/p1GraphEasy-con-AutoTuner && bash build_lowmem.sh

# merged/pushed state (the current tree):
cd /home/user/D/Course/msk1/2nd/Graph-language-/p1GraphEasy-con-AutoTuner

# single-fixture verdicts (both trees):
GRAPH_FRONTIER_STATS=1 GRAPH_FILE=<tree>/verify/cases/parallel/<fixture>.graph bash ./03_run.sh
grep -o 'class=[a-z-]*' <log> | sort -u
# and the refusals:
grep -E 'graph-frontier|reason=' <log>

# full parallel corpus (per tree):
cd <tree>/verify && bash ./run.sh parallel
```

Attribution evidence for the refusing rule:

```bash
for r in f4010d5 1c383ed 64f6926; do
  echo -n "$r: "
  git show $r:p1GraphEasy-con-AutoTuner/spatial_partition_proof.inc \
    | grep -c "unconditional shared write connects every pair occurrence"
done
# f4010d5: 1  1c383ed: 1  64f6926: 0
```

---

## 7. What to change, in order

1. **Make R3 win over the new spatial refusal.** In `spatial_partition_proof.inc:466-486`, the
   cluster requires `!A.Private`; the R3 slot-redirection (`privLayout`, `emitPairWorkFn` slot
   mapping) is what should mark those writes private. Either the R3 redirection is not visible
   to the spatial proof at this point, or the rule must be relaxed when the write's slot has a
   per-partition partial. Evidence to check first: the shape flags (`red=1 sep=0 data=0`) and
   whether `A.Private` is ever set for the second slot in a two-slot nest on `f4010d5`.
2. **The float FAdd half of `reduce_real_ops`.** `rsum` should reach `reduction`; the R8
   implementation guard (`graph_frontier_lowering.cpp:6405`, reason "reduction conditions not
   met") is the second refusal site — the `S.MutG && S.HasUopG && …` cluster. Decide: realize
   the ordered FAdd combine, or document a deliberate refusal and change `run.sh:812`.
3. **Fix the contradiction in the text.** Update `two_reduce_slots.graph:2-4` and
   `COMPOSITION_STATUS.md:34` to the post-R3 contract (`privatized`, `a 10 b 20`), or mark them
   historical — otherwise the repository contradicts its own assertions.
4. Re-run `bash verify/run.sh parallel` on an **idle** machine (my two runs disagreed on some
   entries while compilers were being built concurrently — throughput assertions such as
   `1thr == 4thr` and memory-heavy cases are load-sensitive).

---

## Appendix A — a separate, independent regression (ours): dropped call spellings

`race/mutual_deg` fails to *parse* on the merged tree (and on our branch too):

```
line 10:12 extraneous input 'hasEdge' expecting {'(', '{', 'degree', 'INF', 'timer', '!', 'TRUE', 'FALSE', '-', ID, INT, REAL}
line 10:21 no viable alternative at input '(G,'
```

The fixture uses `if (hasEdge(G, v, u))`; the rewritten grammar only accepts the member form
`G.hasEdge(u, v)`. Fix: restore the function-call alternatives (`hasEdge(G,u,v)`,
`hasNode(G,u)`) in `Base.g4` + `ASTBuilder` alongside the member forms.

## Appendix B — measurement caveats

- The pre-merge corpus run was interrupted, so the full per-tree FAIL counts are not yet
  available; the per-fixture verdict comparisons above are complete and deterministic
  (single-fixture compiles, no concurrency).
- Two corpus runs on the *same* pushed content produced different failing sets (my builds were
  running concurrently), so treat the full list as needing an idle re-run; the three fixtures
  analysed here were each reproduced in isolation.
- `impl_failure=0` / `emitted=1` on the refusals means the compiler falls back to the serial
  schedule explicitly — no wrong answers were emitted; the failures are lost parallelization
  and mismatched assertions.
