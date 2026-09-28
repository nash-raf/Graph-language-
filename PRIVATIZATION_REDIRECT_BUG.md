# Privatization (R3) miscompile on the merged tree — and the `class=` vocabulary gap

Status: **found + root-caused, not yet fixed.** Written after pulling `origin/ARS-1`
to `1ada7ef` ("effect algebra closed under CKA" → "composable runtime implemented
for effect expressions" → "runtime fixes"), rebuilding clean, and running
`verify/run.sh`: **70 pass / 19 fail**. Two of the failures are real wrong
answers; the other 17 are a debug-format/vocabulary mismatch.

Repo: `/home/user/D/Course/msk1/2nd/Graph-language-`, compiler under
`p1GraphEasy-con-AutoTuner/`. Line numbers are from the merged tree (`1ada7ef`).

---

## Part 1 — The two wrong answers

Both are **pure accumulations that composition R3 privatizes**, and both return a
total that is *short* under multiple threads, varying run to run.

### 1.1 `priv/mixed_regions` — expected `tot 340000`, got `tot 339906…339938`

Fixture `verify/cases/parallel/mixed_regions.graph`: `arr[i] = 1` for all
n = 20 000 vertices, then for each of the 160 000 arcs:

```c
for each vertex u in G {
    for each neighbor v of u in G {
        arr[u] = arr[u] + 1;
        arr[v] = arr[v] + 1;
    }
}
```

`tot = n + 2·m = 20 000 + 320 000 = 340 000`. Both updates are recognized `+`
updates on one base, written through **both** endpoint regions (U and V) — the
shape R3 privatizes: every partition accumulates into its own copy of `arr`, then
a combine folds the copies.

| threads / partitions (`PRIV_CFGS="1:1 4:4 4:3"`) | printed | correct? |
|---|---|---|
| 1 / 1 | `tot 340000` | ✓ |
| 2 / 1 | `tot 339977` | ✗ (−23) |
| 4 / 1 | `tot 339906` | ✗ (−94) |
| 4 / 3 | `tot 339858…339938` (varies per run) | ✗ |

### 1.2 `priv/data_index_big` — expected `cntsum 160000`, got `cntsum 157387`

Fixture `verify/cases/parallel/priv_data_index_big.graph`: a write whose
subscript is a **data value**, `cnt[deg[u]] = cnt[deg[u]] + 1` for every arc, with
`cnt1 = 1335` (vertices of out-degree 1) as a discriminating probe. Same R3
privatization; the writes concentrate on few distinct addresses (degree values),
so collisions are far more frequent than in 1.1 — and the deficit is 20× larger:

| threads / partitions | printed | correct? |
|---|---|---|
| 1 / 1 and the unrewritten serial build | `cntsum 160000 cnt1 1335` | ✓ |
| 4 / 3 | `cntsum 157387 cnt1 1335` | ✗ (−2613) |

### 1.3 Reproduction

```bash
cd p1GraphEasy-con-AutoTuner
rm -f final_program
GRAPH_FRONTIER_STATS=1 GRAPH_FILE=../verify/cases/parallel/mixed_regions.graph bash ./03_run.sh
for t in 1 2 4; do
  SGPL_CLEANCUT_PARTITIONS=1 SGPL_NUM_THREADS=$t OMP_NUM_THREADS=$t \
    ./final_program </dev/null | grep -v AutoTuner
done
# or just: cd ../verify && bash run.sh   (checks priv/mixed_regions, priv/data_index_big)
```

Three fingerprints, all consistent with a **lost-update race on shared memory**
(not with logic that changes semantics):

1. deficit is always *negative* and small relative to the number of increments;
2. deficit scales with thread count (−23 @2, −94 @4) and is 0 at one thread;
3. the value is **not deterministic** between identical runs (339936 vs 339938),
   and it is already wrong at **one partition**, so partitioning is not the axis.

---

## Part 2 — Root cause: the pair body writes the shared array, not the private copy

### 2.1 What the design intends (all of this is in the merged tree, and correct)

- The classifier accepts the pure-accumulation shape and sets `Info.PrivArrays`
  (`privLayout`, `graph_frontier_lowering.cpp:2305-2460`; obligations documented
  at `2100-2135`).
- The preheader calls `autograph_priv_bind`, which allocates **one copy per
  partition** of every privatized array, initializes each to the operator
  identity, and publishes partition `p`'s copy pointer into partition `p`'s
  reduction record (`autotuner_runtime.c:1877-1921`).
- The execution engine runs **one work item per partition**
  (`autograph_frontier_execute` → `parallel_for_runtime(0, meta->partition_count,
  1, sgpl_exec_partition_body, …)`, `autotuner_runtime.c:3334`); the body hands
  the worker for partition `p` the record of partition `p`
  (`sgpl_exec_partition_body`, `3125-3140`), and `autograph_exec_partition_state(ctx)`
  returns exactly that record (`3467-3469`).
- The pair body is supposed to reach its partition's copy **through that
  record**: the lowering builds `PrivPtrForBase` from the record fields
  (`graph_frontier_lowering.cpp:3474-3496`) and redirects GEPs on a privatized
  base to the private pointer (`3550-3556`). The comment says it outright:

  > Composition R3: the reduction record IS this partition's private state. …
  > **Redirecting every GEP on a privatized base to the private pointer is what
  > makes the body's own load/op/store read and write the copy instead of the
  > shared array.** (`3467-3473`)

- The combine then folds the per-partition copies in ascending partition order
  (`autotuner_runtime.c:3340-3350`).

### 2.2 What the emitted binary actually does

`objdump -d final_program`, the emitted pair function:

```asm
<sgpl_pair_work>:
   mov    %rsi,%rdi
   call   425700 <autograph_exec_partition_state>   ; returns this partition's record …
   mov    0x3c40a(%rip),%rax   # 440490 <arr>       ; … and the result is DISCARDED
   incl   (%rax,%rbp,4)                             ; arr[u]++   ← plain RMW on the SHARED array
   incl   (%rax,%rbx,4)                             ; arr[v]++   ← plain RMW on the SHARED array
   ret
```

The body asks for the partition record and then ignores it, incrementing the
**global** `arr` symbol with non-atomic read-modify-write. Any two workers that
touch the same element concurrently lose one increment. That is exactly the
deficit behaviour in Part 1 — and the reason `data_index_big` loses 20× more: its
subscripts concentrate on a handful of addresses.

Consequence: the privatization machinery is inert for the write path (copies are
allocated and identity-initialized but never written; the printed sum reads the
racy global array), so the loop runs with a real data race.

### 2.3 Where the redirection misses (the two candidates to check first)

The redirect block is gated on `Info.UsePrivLayout` (`3475`), and the lookup is
`PrivPtrForBase.lookup(canonicalArrayBase(SGEP))` (`3553`). So either:

- **gate mismatch** — the `autograph_priv_bind` call site is emitted from the new
  expression path (`emitPrivSetup`, `4039-4050`) while `UsePrivLayout` is only
  set by the legacy privatized step, so the redirect block is skipped; or
- **lookup miss** — `canonicalArrayBase(SGEP)` of the body's GEP does not equal
  `A.Base` for the global-array form, so the map lookup returns null.

Both are cheap to distinguish: print `UsePrivLayout`/`PrivArrays.size()` next to
the candidate line, and print `PrivPtrForBase.size()` at the pair-fn build; then
confirm with the guard below.

---

## Part 3 — Solution

### Fix A (the real fix — apply after locating the miss)

Make the emitted pair body use the partition's private copy for **every** access
to a privatized base:

1. Ensure the `PrivPtrForBase` build and the `autograph_priv_bind` emission are
   gated by the *same* predicate (whatever the expression path sets), and that
   the redirect is applied on the direct-expression path, not only the legacy
   privatized step.
2. Make the lookup robust: key the map by the same canonicalisation on both sides
   (or rewrite by walking the cloned body and matching against `Info.PrivArrays`
   bases).
3. Add an **emit-time postcondition** (in the spirit of the existing
   `GRAPH_FRONTIER_STRICT` checks): for every `A ∈ Info.PrivArrays`, no load/store/GEP
   in the pair function may reference the original base (global or alloca); every
   one of them must chain from the record field. Fail the compile (or refuse the
   rewrite) when violated.
4. Add a regression test that greps the disassembly (or the IR) of the pair
   function for the global base symbol — e.g. `objdump -d final_program |
   sed -n '/<sgpl_pair_work>:/,/ret/p'` must not contain the array symbol.

### Fix B (interim, fail-closed — restores correctness immediately)

If the redirect cannot be proven applied for a shape, **refuse** the
privatization (fall back to sequential). The numbers then equal the serial build
(`340000`, `160000`) and only the class expectation changes. This matches the
project doctrine: parallel emission must be earned; a proof whose emission is
missing is not a proof.

### Fix C (belt, optional)

In debug builds, assert in the pair-callback ABI that a step which called
`autograph_priv_bind` never writes a privatized base through any pointer other
than the record's copies (a cheap runtime invariant, e.g. a canary write pattern
checked at combine time).

---

## Part 4 — Verification plan (once fixed)

1. Both fixtures at every config in `PRIV_CFGS="1:1 4:4 4:3"`, values equal to
   the fixture-derived constant **and** to the unrewritten serial build (the
   existing checks already do both).
2. The two tiny cases (`data_index_write` `cntsum 10`, `two_reduce_slots`) —
   they pass today by luck (tiny, few collisions); keep them as the fast gate.
3. Determinism: each config repeated ≥5×, byte-identical output.
4. IR/binary guard from Fix A step 4, wired into the suite so the redirect can
   never silently regress again.
5. Full `verify/run.sh` (expected green once Part 5 is also addressed) and the
   seven `validate_*.sh` scripts.

---

## Part 5 — The other 17 failures: `class=` was removed from the debug line

Not a correctness bug, but it breaks 16 of our checks and 3 of their own scripts.

- Bisect: `38b9848` ("effect algebra closed under CKA") still had
  `enum class Klass` and printed `class=…`; `6a29d3b` ("composable runtime
  implemented for effect expressions") **removed both**; `1ada7ef` keeps them
  removed. Today `grep -c 'class='` over the lowering is **0**.
- The candidate line now reads:

  ```
  [graph-frontier] candidate: main driver=… inner=… kind=3 red=0 sep=1 data=0
    fw=0 env=0 shadow=0  R(arr,U):Carried ⊗ U+(arr,U) ⊗ R(arr,V):SameRoundRead
    ⊗ U+(arr,V)  temporal=Carried
  ```

  i.e. `kind=`/`red=`/`sep=`/`data=`/`fw=`/`env=`/`shadow=` plus the effect
  string, and `[refused: <reason>]` when the algebra rejects the loop — but no
  verdict name.
- Meanwhile **their own documentation and scripts still expect it**:
  `EFFECT_SYSTEM.md` shows `class=source-owner` / `class=sequential` in its
  example output (`570-580`) and defines the full list
  (`SourceOwner, DestOwner, Reduction, DualOwner, Sequential, Privatized,
  SourceReduction`, §5.1), and `validate_composition.sh`,
  `validate_reduction.sh`, `validate_roundsep.sh` all `grep class=`.
- Our harness fails with `want class=…, got: [1 loop(s)]` for all 16 checks —
  including `race/dual_shadow`, whose refusal is demonstrably still detected
  (the log shows `shadow=1`; only the name is missing).

**Options (pick one, coherently):**

1. **Restore a derived `class=` field** in the candidate print (compute the name
   from the same predicates the interpreter uses; the doc already defines the
   seven names as "derived descriptions"). This fixes our 16 checks *and* their
   three validate scripts, and matches their own documentation.
2. **Port the harnesses** to the new vocabulary (`[refused: …]` ⇒ old
   `sequential`; `shadow=[1-9]` ⇒ round-separation; `red=1` ⇒ reduction; the
   privatization/owner distinctions would need new fields to be pinned at all —
   today `kind=` alone does not identify `privatized` vs `dest-owner`).

Option 1 is strongly preferred: option 2 loses the ability to pin verdicts like
`privatized`/`dest-owner` that the checks exist for.

---

## Appendix — quick reference

| What | Where |
|---|---|
| R3 obligations / acceptance | `graph_frontier_lowering.cpp:2100-2135` |
| `privLayout`, `PrivArrays` | `graph_frontier_lowering.cpp:2305-2460` |
| record fields → `PrivPtrForBase` | `graph_frontier_lowering.cpp:3467-3496` |
| GEP redirect consumer (suspected miss) | `graph_frontier_lowering.cpp:3550-3556` |
| `emitPrivSetup` / `autograph_priv_bind` call site | `graph_frontier_lowering.cpp:4039-4050` |
| `autograph_priv_bind` (per-partition copies) | `autotuner_runtime.c:1877-1921` |
| exec dispatch (per partition) + ascending combine | `autotuner_runtime.c:3297-3350` |
| `sgpl_exec_partition_body` (partition record) | `autotuner_runtime.c:3125-3140` |
| `autograph_exec_partition_state` | `autotuner_runtime.c:3467-3469` |
| old `class=` print (for reference) | `git show 4a3c293:./graph_frontier_lowering.cpp` (~3869-3891) |
| fixtures | `verify/cases/parallel/mixed_regions.graph`, `verify/cases/parallel/priv_data_index_big.graph` |
| checks | `verify/run.sh` — `priv/mixed_regions`, `priv/data_index_big`, `expect_class …` |

Context: our earlier `parallel_loop_outline.cpp` (`sgpl_now_ns` attribute) change
was dropped in favour of the merged version — the new runtime removed the warmup
sampler, so that hunk is obsolete; the patch is saved in the session scratchpad
(`outline_sgpl_now_ns.patch`) in case. Our `verify/run.sh` (including the P9b
check) is kept and coexists with their `race/marker_derived`.
