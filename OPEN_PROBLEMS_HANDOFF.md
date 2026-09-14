# GraphEasy compiler — open problems handoff

This document is meant to be handed to a fresh reader (human or LLM) with **no prior
context**. It describes the remaining known problems in the GraphEasy compiler work,
excluding the Milo-paper reproduction tasks. Every claim here was verified against the
live tree while writing it; line numbers and commands are included so they can be
re-verified. Do not trust this document blindly — read the code and run the commands.

Date of writing: 2026-09-10. Updated 2026-09-12 with the P2/P7/P12 fixes (§2
items 11–13) and their evidence. Updated again 2026-09-13 (night session): P1,
P3, P5, P6 and P11 are now fixed on the working tree, and P8-residue/P9 have
their first verified pieces — see §2b for that session's evidence.

---

## 1. Context: what this codebase is

GraphEasy is a compiled DSL for graph algorithms. A `.graph` program is parsed
(ANTLR) and lowered to LLVM IR, where a series of custom passes parallelize graph
loops, and the resulting program is compiled and linked with a hand-written C
runtime.

Repo: `/home/user/D/Course/msk1/2nd/Graph-language-`
Branch: `ars1-latest`; upstream base commit `e8b89f9` ("ARS interaction with effect
system fixed", = `origin/ARS-1`).

Layout:

| Path | Role |
|---|---|
| `p1GraphEasy-con-AutoTuner/` | the compiler under discussion (sources + `GraphProgram` binary) |
| `p1GraphEasy-con-AutoTuner/Base.g4` | grammar (note: repo .g4 is stale vs the generated parser in `generated/`) |
| `p1GraphEasy-con-AutoTuner/main.cpp` | driver + pass pipeline + parse entry |
| `p1GraphEasy-con-AutoTuner/IRGenVisitor.cpp` | AST → LLVM IR |
| `p1GraphEasy-con-AutoTuner/graph_frontier_lowering.cpp` | CleanCut/owner-step rewrite of graph loops |
| `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp` | region discovery, cost model, layout DP, profile annotation |
| `p1GraphEasy-con-AutoTuner/pdg.cpp` | Program Dependence Graph + loop classification (DOALL/DOACROSS/SEQUENTIAL) |
| `p1GraphEasy-con-AutoTuner/parallel_loop_outline.cpp` | multiversions parallel loops, emits outlined clones + range bodies |
| `p1GraphEasy-con-AutoTuner/parallel_runtime.{c,h}` | thread pool, cost model, chunked execution, DOACROSS wait/post |
| `verify/` | regression suite (`run.sh`), fixtures, expected values (`expected/golden.json`, `expected/lang.manifest`, `expected/motif.manifest`) |
| `p2GraphEasy/…` | a parallel variant of the project (motif DSL + Milo reproductions); mostly out of scope here |

Build & run (IMPORTANT quirks):

```bash
cd p1GraphEasy-con-AutoTuner
bash ./build_lowmem.sh                 # rebuild GraphProgram (~1-2 min)
GRAPH_FILE=/path/prog.graph bash ./03_run.sh </dev/null   # compile+link+run ONE program
cd ../verify && bash run.sh [category] # suite: lang algo parallel autotuner motif
```

- `03_run.sh` **must be invoked through `bash`** (its first line is a commented-out
  shebang, so direct execution falls back to `sh` which chokes on `set -o pipefail`),
  and it reads stdin — always redirect `</dev/null`.
- Large graphs need `ulimit -s unlimited`.
- The compiler is rebuilt with `build_lowmem.sh`, which had been broken at `e8b89f9`
  (missing `graph_frontier_lowering.cpp` and Polly's include dir); that is fixed in
  the current tree.

### 1.1 What the pipeline does

1. `GraphFrontierLoweringPass` recognizes "for each vertex { for each neighbour }"
   patterns and rewrites them into engine calls: CleanCut owner-steps
   (`autograph_frontier_step_owner_{push,source}`) or the legacy CAS edgemap
   (`autograph_edgemap`). It also marks residual loops with the metadata
   `sgpl.frontier.nested.sequential`.
2. `AutoTunerPass` finds engine regions, runs a cost model, picks layouts, and
   injects `autograph_profile_region_enter/exit` calls (predicted vs measured
   profiling, printed as `[AutoTunerProfile] total kind=…`).
3. `runPDGOnModule` builds a Program Dependence Graph; `analyzeAndAnnotateLoop`
   classifies each loop DOALL / DOACROSS / SEQUENTIAL and writes
   `!my.loop.parallel` on loop latches.
4. `parallel_loop_outline.cpp` clones every parallel-marked loop into a serial
   original + a parallel clone (wrapper + range entry point
   `(lo, count, step, env)` registered via `sgpl_set_pending_range_body`), guarded at
   run time by the cost model (`sgpl_should_parallelize_doall`).
5. `parallel_runtime.c` executes chunks (block-cyclic, `SGPL_LOOP_CHUNK`, default
   256; DOACROSS stays cyclic), with a measured cost model and warm-up calibration.

### 1.2 Current tree state

The tree is **not** the baseline the previous developer had in progress. It is:

- upstream `e8b89f9`, plus a set of fixes applied since (see §2), plus three files
  restored from the developer's baseline snapshot (`parallel_loop_outline.cpp`,
  `parallel_runtime.{c,h}`) to regain the working parallel runtime.

Git tags created along the way (all allow `git reset --hard <tag>` /
`git checkout <tag> -- <file>`):

| Tag | Content |
|---|---|
| `pdg-prework-original` | upstream `e8b89f9` |
| `pdg-prework-baseline` | the developer's uncommitted work-in-progress (554-line diff: pdg proof-override, outliner multi-loop + range body, runtime chunking/calibration, build fix) |
| `pdg-fixes-snapshot` | baseline + a first round of fixes |
| `pdg-session2-snapshot` | upstream + language/DSL fixes + frontier guards |
| `pdg-session3-snapshot` | the above + the `isConfused` guard (i.e. before the runtime restore) |
| `pdg-before-p2p7p12` | `proof-lowering` at `2ca99ad`, i.e. before the P2/P7/P12 fixes (restore the compiler with `git checkout pdg-before-p2p7p12 -- p1GraphEasy-con-AutoTuner/pdg.cpp`; `verify/run.sh` is untracked, see P13) |

### 1.3 Test status (current tree)

Two trees matter as of 2026-09-12:

**(a) `proof-lowering` (HEAD `2ca99ad` + the P2/P7 fixes, tag
`pdg-before-p2p7p12` marks the state before them):** `verify/run.sh` full run
**51 pass / 0 fail / 0 skipped**, exit 0, `scaling/doall_scaling 4thr faster
(1.94x)` asserted.

**(b) `merge-ars1-roundsep` (HEAD `9115210`, the merge of upstream
`origin/ARS-1` = `4f0dc70` into (a); see §1.4):** `verify/run.sh` full run
**53 pass / 0 fail / 1 skip** (54 checks), exit 0. The single skip is the
load-guarded scaling check (P12) reporting 1.32× under load 6.27 on 4 cores.
New checks carried by this tree: `race/same_slot` (P2 regression),
`race/roundsep` (composition A: `shadow=1`, identical across 1/3/4 partitions
and equal to `compute_roundsep_expected.py`), `race/mixed_regions` (composition
C: stays sequential, `tot 340000`), `race/reduce_plus_write` (composition D:
stays sequential, `acc 160000 w0 1`).

Upstream's own acceptance harnesses, all green on the merged tree:

| Harness | Result |
|---|---|
| `test/run_frontier_shadow_tests.sh` | PASS — "3 configs, 0 failures" |
| `test/run_frontier_red_tests.sh` | PASS — "24 configs, 0 failures" |
| `validate_roundsep.sh` | PASS — min/add/sssp, deterministic p1/p3/p8, equals the frozen-read reference |
| `validate_reduction.sh` | PASS — sum/sub/mul/min/max: `class=reduction red=1`, part8 == part1 == seq == python expected |

Coverage sweep (nine algo/parallel graph programs) is unchanged by the merge:
`doall_scaling` 2×`DOALL/proof=1`, `pagerank` 5×`SEQUENTIAL/proof=0`, same
engine classes (`dest-owner` bfs_level, `source-owner` kcore/pagerank,
`source-owner` refused nested_gather, `sequential` int_gather/mutual_deg), all
bit-identical at 1 and 4 threads, every algo program carries a
`[AutoTunerProfile] total kind=Traverse` region model (cc:
`region=0 kind=Traverse layout=CSR visits=2 predicted_ms=2.467 measured_ms=2.267`,
0 layout conversions injected).

The scaling assertion is now **skipped rather than failed** when the 1-minute
load average exceeds the core count (P12). Both branches verified:
`bash run.sh` asserted the threshold at 1.72×; `SGPL_SCALING_MAX_LOAD=0 bash
run.sh parallel` printed `SKIP scaling/doall_scaling 4thr 2.21x, not asserted
(load 2.81 > 0 cores)` and ended `passed: 15 failed: 0 skipped: 1`, exit 0.

Parallel-coverage sweep over all nine algo/parallel graph programs (loop
classifications, frontier engine class, 1thr==4thr, autotuner region model):
`doall_scaling` 2×`DOALL/proof=1` (the DOALL path is intact), `pagerank`
5×`SEQUENTIAL/proof=0`, `bfs_level`/`kcore`/`sssp`/`cc` SEQUENTIAL, engine
classes `dest-owner` (bfs_level), `source-owner` (kcore, pagerank),
`source-owner`+`dest-owner` (pagerank), `source-owner` refused (nested_gather),
`sequential` (int_gather, mutual_deg); all nine bit-identical at 1 and 4
threads; every algo program carries a `[AutoTunerProfile] total kind=Traverse`
region model (cc: `region=0 kind=Traverse layout=CSR visits=2
predicted_ms=2.467 measured_ms=2.267`, 0 layout conversions injected).

### 1.4 Upstream merge `origin/ARS-1` (2026-09-12)

`origin/ARS-1` moved from `e8b89f9` to `4f0dc70` ("added parallel runtime support
for roundsep") — one commit, +1413/-14. Merged as `9115210` on branch
`merge-ars1-roundsep` (parents `2ca99ad` local, `4f0dc70` upstream). The only file
both sides touched is `graph_frontier_lowering.cpp`; the merged file is our version
plus upstream's 299 added lines (three print lines reflowed, nothing of ours lost).
All local work — the P2/P7 fixes, the frontier refusals, envelope/reduction
emission, the whole `verify/` suite — is intact.

What upstream adds:

- **Round-separation shadow (composition A).** `NeighborLoopInfo::RoundSepBases`
  (cross-endpoint R x W on one base, single-region writes, uniform i32/double), a
  `classify()` rule turning such a loop into DestOwner/SourceOwner, and
  `emitRoundSepShadow()` — a per-round `memcpy` of the live array into
  `autograph_scratch_shadow(graph, bytes, slot)` (4 slots, lazily sized) plus
  shadow redirection of every cross-endpoint load in the cloned pair body.
  Same-region RMW reads deliberately stay live (otherwise min/max/+= relaxations
  would degrade to last-write-wins).
- **Reduction validation (composition H).** No emission change: `validate_reduction.sh`
  drives `reduce_{sum,sub,mul,min,max}.graph` over `rtest_small.txt`, requires
  `class=reduction red=1`, then compares partition-8 / partition-1 / sequential /
  independent Python fold; `compute_reduce_expected.py` is the reference;
  `test/frontier_red_test.c` (24 configs) and `test/frontier_shadow_test.c`
  (3 configs) are native unit tests. The old fixtures were wrong shapes
  (`reduce_and.graph` = `&`, `reduce_or.graph` = `|`) and are renamed/replaced by
  single-op programs.

Merge caveats worth knowing:

- `SGPL_FRONTIER_STRICT=1` (our suite default) now also covers the shadow path via
  the new `race/roundsep` check.
- Upstream's `validate_composition.sh` still hardcodes
  `/mnt/d/sgpl/bench/p1GraphEasy-con-AutoTuner` and references the pre-rename
  `reduce_add.graph` plus a missing `pagerank_dataidx.graph`; run it from a patched
  copy (see §5).

**Two defects in the pushed shadow implementation, fixed on the merged tree.**

1. **Dual-owner + shadow never linked.** `emitDualCleanCut()` emits two work
   functions via the shared `emitPairWorkFn()`, which redirects cross-endpoint
   loads to a shadow global, but the dual path never called
   `emitRoundSepShadow()` — the global stayed a declaration and the object failed
   to link: `undefined reference to 'main.alive.shadow.0'`. Upstream's own
   `small_kcore.graph` is exactly this shape (`shadow=1 class=dual-owner`), so the
   harness that was supposed to validate it reported `LINK FAIL` and then a
   spurious `MISMATCH` (it compared files that were never produced). Fix: call
   `emitRoundSepShadow()` once per round in `emitDualCleanCut()`, before both
   phases (file `graph_frontier_lowering.cpp`, `emitDualCleanCut`).
2. **The dual-owner shadow produced wrong answers.** With the link fixed, the
   shape still disagreed with the serial build: on the g20k-scaled k-core peeling
   program the rewrite left **18898** survivors where the serial build and an
   independent Python 10-core computation both give **18959**. Mechanism: the
   shadow freezes round-start `alive[]`, but this loop is a claim/activate state
   machine — a vertex already killed earlier in the same round still read
   `alive[v] == 1` from the snapshot, ran its body and decremented its live
   neighbours' `deg[]`, over-peeling 61 vertices. Fix: fail closed — a dual-owner
   step whose cross-region reads would need a shadow is now classified
   **sequential** (`Info.RoundSepBases` non-empty disqualifies `Klass::DualOwner`).
   The shadow-eligible nest still reports `shadow=1`, but with
   `class=sequential`, and the answer is 18959 at 1 and 4 threads. Both fixes are
   covered by the new `verify/cases/parallel/dual_shadow.graph` +
   `race/dual_shadow` check.

**Open question surfaced by the merge (semantics, not a defect).** The shadow
implements *round-separated* semantics for in-place loops, which is what
`compute_roundsep_expected.py` encodes and what the compiler comment claims
("reads a frozen round-start snapshot"). That is observably different from a
serial execution of the same source: on `verify/cases/parallel/roundsep.graph`
the rewrite answers `sum 180000`, while the no-rewrite build (`SGPL`/`GRAPH_FRONTIER_REWRITE_OFF=1`)
overflows int32 (values compound across the sweep). Both are deterministic, but
the language definition has to say which one `for each vertex { for each neighbor …
}` in-place updates mean; upstream's validators cannot detect this because they
compare against the frozen-read reference, not against the serial build.
- Composition status on the merged tree, from live evidence:

| Class | Status on the merged tree |
|---|---|
| A — round-separated (in-place cross-endpoint R x W) | **implemented + verified for the store-based single-region shape** — shadow snapshot; `race/roundsep`, `validate_roundsep.sh`, shadow matrix green, deterministic across 1/3/4 partitions. NB it implements frozen-round semantics, which differs from the serial build (see the open question above) |
| A-dual — dual-owner step whose cross-region read needs a shadow (claim/activate state machine, upstream's `small_kcore`) | **refused** — was a wrong answer (18898 vs 18959) after fixing the link failure; now `class=sequential` at 18959, `race/dual_shadow` |
| C — mixed same-array U+V writes | **refused** — `race/mixed_regions`: `class=sequential`, `shadow=0`, `tot 340000`, 1thr == 4thr |
| D — reduction + vertex write | **refused** — `race/reduce_plus_write`: `class=sequential`, `acc 160000 w0 1` |
| H — reduction emitted-unvalidated | **validated** — `validate_reduction.sh`, 24-config red matrix |
| E — cross-phase dependence | covered by `bfs_level`'s effect string (`R(visited,V):SameRoundRead ⊗ Claim(visited,V) ⊗ R(lvl,U):PreviousRoundRead`), dest-owner, race-clean |
| B — data-aliased (write at a data-valued subscript) | **refused and pinned** — `race/data_index_write`: the `cnt[deg[u]]` nest reports `data=1 class=sequential`; the owner table is vertex-keyed (`CC_PART_OF`) so a data-valued house has no owner |
| F — claims without envelope | **refused and pinned** — the *driver-preamble* first-wins claim now refuses in `classify()` (it runs once per source in the serial program, once per (u,v) pair in the engine, and its guard is not reproduced).  `race/claim_driver`; before this the verdict was accidental: the classifier said `dual-owner` and the emit failure silently kept the loop sequential |
| G — exotic combines | **audited, fidelity fixed, refusals pinned** — min/max flavours separated (smin / umin / minnum / minimum) with matching identities; unsigned predicates no longer combined with `smin`; float `select(fcmp)` min/max stays refused (not reorder-invariant with NaN / ±0).  `race/reduce_int_ops` (7 loops), `race/reduce_real_ops` (1 parallel + 2 pinned refusals) |
| I — array-frontier / per-source gathers | **refusal pinned** — `race/int_gather`, `race/mutual_deg` now assert `class=sequential` explicitly; parallelizing them is P10 (per-source reduction engine) and that assertion is the line that must change when P10 lands |
| extra — two scalar accumulators in one body | **refused (new)** — only `ReducePtr` gets per-partition storage, so a second slot was silently written by every partition.  Now recorded as an unrecognized global effect and refused; `race/two_reduce_slots` |

Useful debug env vars (compile-time unless stated):

| Var | Effect |
|---|---|
| `SGPL_LOOP_CLASSIFY_DEBUG=1` | PDG prints `[loop-classify] …` per loop |
| `SGPL_PDG_CONFUSED_DEBUG=1` | PDG prints `[pdg-confused …]` pairs |
| `SGPL_NO_PDG_CONFUSED_GUARD=1` | **restores the old unsound read** (A/B only, see P4) |
| `SGPL_NO_PDG_EQ_GUARD=1` | **restores the old zero-distance trust** (A/B only, see P2) |
| `SGPL_PDG_EQ_DEBUG=1` | PDG prints `[pdg-eq-guard]` for pairs the P2 guard downgrades |
| `SGPL_SCALING_MAX_LOAD=N` (suite) | override the scaling check's load bound; `0` forces its skip branch (see P12) |
| `SGPL_OUTLINER_DEBUG=1` | outliner decisions |
| `GRAPH_PARALLEL_DEBUG=1` (runtime) | cost-model serial/parallel decisions |
| `SGPL_FORCE_DOALL_PARALLEL=1` (runtime) | force all DOALL clones parallel |
| `SGPL_NO_RANGE_BODY=1`, `SGPL_LOOP_CHUNK=0`, `SGPL_NO_WARMUP_CALIBRATION=1` (runtime) | A/B switches of the restored runtime |
| `GRAPH_FRONTIER_REWRITE_OFF=1` | disable the frontier engine rewrite (PDG-only path) |
| `GRAPH_FRONTIER_STATS=1`, `GRAPH_FRONTIER_VERIFY=1` | lowering diagnostics |
| `SGPL_FRONTIER_STRICT=1` | an emitted rewrite whose function fails `verifyFunction` aborts the compile (suite default, see P8) |
| `SGPL_FRONTIER_BLOCKLIST_GUARD=1` | use the old shape blocklist instead of the totality prover (A/B only, see P8) |
| `SGPL_PDG_SECOND_CHANCE=1` | on algebra refusal, release the loop to the PDG instead of forcing the sequential marker (see P8) |
| `SGPL_NO_PDG_CALL_BARRIER=1` | **A/B only** — stop treating a stateful call in a loop as a barrier (unsound: the pair analysis cannot see call effects), see §2b |
| `SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1` | **A/B only** — stop refusing a driver-preamble first-wins claim (the emitted work functions run it once per pair and drop its guard), see §2c |
| `SGPL_NO_FRONTIER_MARKER=1` | drop the `sgpl.frontier.nested.sequential` classification veto and let the certificates decide (the marker still gates `reconstructParallelIR`), see §2b |
| `SGPL_NO_PDG_INVARIANT_SLOT_GUARD=1` | **A/B only** — restore trust in a zero-distance verdict on a loop-invariant slot (a store there is visible to later iterations), see §2b |
| `SGPL_NO_PDG_GLOBAL_PROMOTE=1` | disable the single-function-global → alloca promotion that makes pre-PDG canonicalization effective (§2b/P6) |
| `SGPL_NO_PDG_CANON=1` | disable pre-PDG canonicalization entirely (GPU-only behaviour) |
| `SGPL_PDG_CANON_DEBUG=1` | print `[pdg-canon] localized @x in @…` per promoted global |
| `SKIP_PROGRAM_RUN=1` (03_run.sh) | compile but do not execute the produced program (debugging hangs) |
| `DUMP_LLVM_BC_PDG=…`, `DUMP_LLVM_BC_AFTER_OUTLINE=…` | IR dumps (use `llvm-dis` from `/usr/local/llvm-20-polly-rtti/bin`) |
| `DUMP_LLVM_BC_PRE_PDG=…` | dump right before canonicalization + PDG (the frontier pass has already run) |

---

## 2. Already fixed — do not re-investigate (but keep in mind)

These were bugs; they are fixed in the current tree. If you see a tag from before,
they may reappear.

1. `real r[4]` (constant-size real arrays) crashed the compiler. Fixed: static
   arrays use the declared element type.
2. `real m[n][n]` (2D real arrays) was rejected. Fixed: semantic + IRGen support.
3. `int INF = 5` failed to parse (`INF` was reserved as the infinity literal).
   Fixed with a lexer override (`GraphLangLexer` in `main.cpp`).
4. `print` of a real used `%f` (6 decimals); `1e-07` printed as `0.000000`.
   Fixed to `%.13g` in `IRGenVisitor.cpp` (both sites) with updated
   `verify/expected/lang.manifest` rows.
5. `for each edge u, v` on a **directed** graph skipped every reverse arc (`u >= v`
   dedup applied unconditionally). Fixed in `IRGenVisitor.cpp`: the dedup is now
   gated on the runtime `directed` field.
6. The compiler **crashed** compiling a neighbour loop containing an inline graph
   query (`hasEdge` emits its own scan subloop) — the crash surfaced later inside
   LLVM's CalledValuePropagation. Fixed by a fail-closed guard
   `neighborLoopHasForeignCalls()` in `graph_frontier_lowering.cpp`
   (~line 2258): refuse the engine rewrite for loops with subloops, foreign calls,
   or FP header PHIs; such loops fall back to the conservative sequential path.
7. The same guard also fixed the **silent `checksum 0.000000`** miscompile of the
   scalar-reduction gather (`for each vertex u { real s = 0; for each neighbor v {
   s += … } acc[u] = s }`): the rewrite used to clone the reduction body with the
   loop-carried FP PHI replaced by a pointer (`fadd ptr, double`), deactivate the
   driver, and emit nothing. It now refuses → correct sequential result
   (`160136.013072` for `verify/cases/parallel/nested_gather.graph`, deterministic
   at 1 and 4 threads).
8. `scaling/doall_scaling` failed (4 threads slower than 1) because the upstream
   runtime drove parallel bodies one iteration at a time through a function pointer,
   preventing inlining/vectorization. Fixed by restoring the baseline
   `parallel_loop_outline.cpp` + `parallel_runtime.{c,h}` (range body, block-cyclic
   chunking, warm-up calibration). Now 1.45×–2.4×.
9. The `isConfused` false-independence bug — see P4; it is *fixed* but the fix and
   its trade-offs matter for the remaining problems.
10. **Branch `proof-lowering`** (not the base tree; see P1 and P8): the autotuner
    edgemap-region fix (P1), the totality prover + emit-side postcondition +
    priority-2 hand-off (P8), and three new suite regressions in `verify/run.sh`:
    `race/nested_gather_default` (the refused gather through the default pipeline),
    `race/int_gather` (`degsum 40`) and `race/mutual_deg` (`mutdegsum 8`) — both new
    per-vertex scalar-accumulator programs, run at 1 and 4/4 threads.
11. **P2 — zero-distance invariance guard** (2026-09-12, `pdg.cpp`): a
    `ProvenZero` (EQ / distance-0) component is no longer trusted when either
    access's GEP subscripts are derived from a load or a call inside the loop;
    such a pair becomes an unknown carrier (fail closed). Kill switch
    `SGPL_NO_PDG_EQ_GUARD=1`, diagnostics `SGPL_PDG_EQ_DEBUG=1`. Verified a no-op
    on every suite program (A/B sweep identical) — see P2 for the caveat.
12. **P7 — dead `sgpl.frontier.first_wins.candidate` reader deleted** (2026-09-12,
    `pdg.cpp`): the reader, its unreachable `sgpl.frontier.first_wins.doall`
    write, and the now-unused `markNestedLoopsSequential()` helper are gone.
    Nothing in the repo ever wrote `.candidate`; the outliner's live
    `.doall` read and the lowering's conditional writer are untouched.
13. **P12 — suite scaling check is load-aware** (2026-09-12, `verify/run.sh`):
    correctness (`1thr==4thr`) is always asserted; the ≥1.2× throughput
    assertion runs only while the 1-minute load average is ≤ the core count
    (best-of-5, `SGPL_SCALING_MAX_LOAD` override, `SKIP` reporting with the
    measured speedup). See §1.3 for the two-branch evidence.
14. **Upstream `origin/ARS-1` merged** (2026-09-12, `4f0dc70` → merge `9115210` on
    branch `merge-ars1-roundsep`): round-separation shadow snapshot (composition A),
    reduction validation harness (composition H), new fixtures and native tests.
    Upstream's four harnesses and our whole suite are green on the merged tree; see
    §1.3 and §1.4.
15. **Two shadow defects fixed on the merged tree** (2026-09-12,
    `graph_frontier_lowering.cpp`): (a) `emitDualCleanCut()` now emits the shadow
    snapshot, so dual-owner + shadow objects link (`small_kcore` used to fail with
    `undefined reference to 'main.alive.shadow.0'`); (b) a dual-owner step whose
    cross-region reads need a shadow is refused as sequential instead of being
    rewritten with frozen reads — the rewritten k-core peeling gave 18898
    survivors where serial and an independent 10-core count give 18959. Regression:
    `verify/cases/parallel/dual_shadow.graph` + `race/dual_shadow`.

---

## 2b. Night session 2026-09-13 — P1, P3, P5, P6, P11 fixed; P8-residue/P9 started

Working tree on branch `merge-ars1-roundsep` (`9115210` + uncommitted changes).
Full suite at the end of the session: **58 pass / 0 fail** (`scaling/doall_scaling`
asserted at 2.38x). Repro commands are in §5.

### P1 — `autotuner/cc` is region-modelled (was the one red suite check)

`autograph_edgemap` is now recognised as a step kernel: `isCleanCutStepCall()`
matches it (`AutoTunerPass.cpp`), the early-out became
`allEvents.empty() && !moduleHasStepKernels(M)`, the graph keys iterate
`eventsByGraph ∪ metaByGraphPtr`, the `regions.empty()` gate sits **after** the
step-annotation block, and a step-only program uses `totalOps = 1` (the traversal
cost model already prices a whole graph pass). Evidence: cc emits **2**
`autograph_profile_region_enter` calls;
`region=0 kind=Traverse layout=CSR visits=2 predicted_ms=2.467032 measured_ms=1.626550`;
`autotuner/cc region-modelled` and `prediction within 5x (0.8x)` both PASS.

### P6 — pre-PDG canonicalization, and why GlobalOpt alone was never enough

`canonicalizeLoopsForAnalysis()` is now called on **every** pipeline path
(`main.cpp`; `SGPL_NO_PDG_CANON=1` restores GPU-only), and two findings made it
actually do something:

1. Running `GlobalOptPass` (as the baseline diff did) does **not** localize these
   globals. Verified two ways: `opt -passes='globalopt,mem2reg'` on the real
   pre-PDG module leaves `@i`/`@a` in place, and a minimal loop-used scalar global
   behaves the same. GlobalOpt only localizes zero-initialized globals whose uses
   sit outside loops (or in a single block), which is not the DSL shape.
2. The tree therefore localizes them itself: `promoteSingleFunctionGlobals()`
   (`main.cpp`, next to the canonicalization) turns every internal non-constant
   global whose uses are all non-volatile load/stores **in one function** into an
   entry-block alloca with its initializer stored once; `PromotePass` (mem2reg)
   then lifts it to SSA. `SGPL_NO_PDG_GLOBAL_PROMOTE=1` disables just this step,
   `SGPL_PDG_CANON_DEBUG=1` prints `[pdg-canon] localized @x in @main`.

Evidence (positive control, `verify/cases/parallel/array_doall.graph`):
`localized @a in @main`, `localized @i in @main`, loop goes from
`SEQUENTIAL unknown=1` to `classification=DOALL hasProofOfNoCarriedDeps=1`;
new suite check `parallel/array_doall` asserts both the verdict and
`a_last 199999` at 1/4/4 threads.

### P5 — the `isConfused` guard is no longer maximally conservative

With promotion on, PageRank's loops carry positive evidence where they used to
say `unknown=1` (the leaf loops report `proof=1`); the traversal nests stay
SEQUENTIAL. The AA escape hatch is still mostly inert (DSL arrays escape to
calls), so the improvement comes from promotion + the call barrier below, not
from loosening the guard.

### P3 — DOACROSS is exercised and verified

`pref[i] = pref[i-1] + 1` classifies **DOACROSS** (`hasProvenCarriedDep=1`) and
the IR carries `doacross.wait`/`doacross.post` metadata (2 each, grepped out of
the PDG bitcode dump). New suite check `race/doacross_scan`
(`verify/cases/parallel/doacross_scan.graph`, n=200000) asserts the class, the
metadata and the answer under `SGPL_FORCE_DOACROSS_PARALLEL=1` at 1/4/4 threads:
`last 199999` every time. A 2M-element variant was also run by hand: exact at 1
and 4 threads, default and forced (`last 1999999`).

### P11 — the `while`-inside-`for each vertex` report is real: wrong answer + hang

Reproduced with `verify/cases/parallel/nested_while.graph`:
`for each vertex u { acc[u]=0; while (k<3) { for each neighbor v of u { acc[u]+=1 } k=k+1 } }`.
`analyzeNeighborLoop()` took the neighbour loop's **immediate parent** as the
frontier driver (`Info.DriverLoop = L->getParentLoop()`) without checking it is a
graph-iteration loop. Here the parent is the `while`, so the pass emitted
`build_clean_cut + frontier_step_owner_source` in place of the while body,
dropped the while's trip count (the `loopcond` header ended up branching to the
merge on both edges, body unreachable) and ran one whole-graph engine step per
vertex — 20 000 x 320 000 edge visits, i.e. an effective hang, and a wrong answer
if it ever finished.

Fix (fail closed): the driver must have an SSA induction phi
(`driverIndVar(Info.DriverLoop)`), which graph-iteration loops always have and DSL
`while` loops never have before canonicalization. Evidence: the candidate is no
longer claimed, `acc0 57` (= 3 x degree(0) = 3 x 19) at 1/4/4 threads with
`SGPL_FORCE_DOALL_PARALLEL=1`, and a decision sweep over the ten suite programs
shows every other candidate's class unchanged. New suite check
`race/nested_while`, wrapped in `timeout 120` so a regression fails instead of
hanging the suite.

### P8-residue / P9 — calls are memory effects; the frontier marker is now a belt

`analyzeAndAnnotateLoop()` only collected loads/stores, so calls inside a loop were
invisible to the certificates. New call-effect barrier (default ON;
`SGPL_NO_PDG_CALL_BARRIER=1` for A/B): a loop containing a call that is neither
`readnone`, a profiler/clock helper (`autograph_profile_*`, `sgpl_now_ns` —
verified in `autotuner_runtime.c` to touch only their own globals) nor a
lifetime/assume/dbg intrinsic sets `hasUnknownAttributedDep` and cannot be
certified; indirect calls are barriers too.

The frontier marker veto is now switchable: `SGPL_NO_FRONTIER_MARKER=1` drops
`IsNestedFrontierLoop` from the classification cascade (the metadata is still
attached and still gates `reconstructParallelIR`). Evidence:

- Classification A/B, veto off vs on, ten programs: pagerank, nested_gather and
  mutual_deg each gain exactly one DOALL and lose none. All three are genuine
  leaf/scan loops (`foreach.cond47`, `foreach.cond38`, `hasedge.loop`) — the
  marker's `markSequential()` cascade marks every ancestor and subloop, so it was
  over-marking loops that do not touch traversal state.
- Full suite with the veto off: **58 pass / 0 fail**.
- Forced-parallel sweep with the veto off (1 thread vs 4 threads x3, plus
  `SGPL_FORCE_DOALL_PARALLEL=1` x3 and `SGPL_FORCE_DOACROSS_PARALLEL=1` x2) over
  twelve programs: every answer identical to the 1-thread reference.
- New checks: `parallel/call_barrier` (barrier log line + bit-exact
  `checksum 160136.013072` at 1/4) and `race/marker_derived` (marker off: the
  derived DOALL exists and 1thr == 4thr == the default build's answer).

Honest scope: with the marker **and** the barrier both disabled the
classifications do not change on any suite program — the traversal nests are
already refused through their own load/store pairs (the iterator/v slots), so the
barrier is currently *preventive* (an invariant the classifier used to violate),
not a fix with an observable suite delta. The veto default therefore stays ON
(fail closed); flipping it is a one-line change backed by the evidence above.

New guard, currently unexercised by any program: `zeroDistanceOnLoopInvariantSlot()`
downgrades a `ProvenZero`/loop-independent pair to Unknown when every pointer in
the pair is loop-invariant and at least one side is a store — the "same slot
written every iteration" shape (`A[0] += 1` in a while loop), where distance 0
does not mean intra-iteration. `SGPL_NO_PDG_INVARIANT_SLOT_GUARD=1` restores the
old trust, `SGPL_PDG_EQ_DEBUG=1` prints the pairs. Every control built so far is
refused by DI as *unknown* before the guard is reached (LLVM reports direction
ALL, not EQ), so it is defence in depth for the EQ route rather than a fix with a
reproduction.

### P13 (partial) — `03_run.sh` gained `SKIP_PROGRAM_RUN=1`

Compiling a program that hangs (P11) used to make every debugging iteration pay
the hang. `03_run.sh` now skips its final `./final_program` when
`SKIP_PROGRAM_RUN=1` is set; normal runs are unaffected.

### Verification added this session

- `verify/shape_fuzz.sh` — eight loop-shape templates (while-in-foreach with a
  counter, while around a neighbour loop, **two nested whiles around a neighbour
  loop**, per-vertex int reduction, conditional claim in a neighbour loop, inline
  `hasEdge`, carried prefix scan written as `for each vertex`, while after a
  neighbour loop). Each is compiled and run at 1 thread vs 4 threads x3 under
  `SGPL_FORCE_DOALL_PARALLEL=1` and `SGPL_FORCE_DOACROSS_PARALLEL=1`; a shape
  fails on compile error, timeout, empty output or any 1-thread/4-thread
  difference. Result: `FUZZ DONE failed=0`, with the hand-computable answers
  matching (`sum 60000`, `acc0 57`, `acc0 76`, `degsum 320000`, `mutdegsum 8`,
  `last 19999`). The prefix-scan shape is classified SEQUENTIAL (unknown
  carrier) — the safe verdict for a carried memory dependence.
- New suite checks, all passing: `parallel/array_doall`, `race/doacross_scan`,
  `race/nested_while`, `parallel/call_barrier`, `race/marker_derived`,
  `parallel/nested_while2` (`acc0 76`), `parallel/foreach_scan` (`last 19999`),
  plus their fixtures in `verify/cases/parallel/`.
- Full suite after all of the above: **62 pass / 0 fail**
  (`scaling/doall_scaling` asserted at 2.38x).

---

## 2c. Session 2026-09-14 — compositions B/F/G/I, the multi-accumulator hole, harness hardening

Work lands on branch `compositions-round2` (off `merge-ars1-roundsep` @
`9115210`, plus the three commits that first put the pending soundness work and
the verification harness into git).  Baseline before any change:
`verify/run.sh` **62 pass / 0 fail / 0 skip**.

### F — a driver-preamble claim must refuse, and refuse *by classification*
Shape: `if (claim[u] == 1) { claim[u] = 0; <neighbour loop> }` — the guard decides
whether a source's neighbour body runs, and the serial program visits each source
once.  Every emitted work function is called once per (u,v) pair (the
`autograph_*_owner_*` bodies in `autotuner_runtime.c`), so the claim would run per
pair and the body would run for sources whose claim failed.  The dual-owner path
grafted the claim without its guard (`emitPairWorkFn`, `PairPhase::UOnly`); the
single-phase paths grafted neither stores nor claims.

Measured before the fix on the new case: `class=dual-owner fw=1 Claim(claim,U)`
and the answer *accidentally* right — `emitDualCleanCut()` returns false and the
fall-through `markSequential()` keeps the loop serial.  That is exactly the
"verdict by accident" this project is removing, so the fix is in the classifier:
- `NeighborLoopInfo::HasDriverClaim` (store-form claim in the driver preamble,
  sibling of the CAS-form `DriverUClaims`);
- `classify()` refuses any loop carrying a driver claim, kill switch
  `SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1`;
- the emit-failure path now prints `emit failed for class=... -> stays
  sequential` under `GRAPH_FRONTIER_STATS=1`, so a silent fallback can no longer
  masquerade as a classification;
- case `race/claim_driver`: only u∈{0,1} claim, so exactly 5 of tiny.txt's 10
  arcs may be processed; asserts `class=sequential`, the serial answer, and
  1thr == 4thr.

### The multi-accumulator hole (found while designing G's tests)
`Info.ReducePtr` is set by the *first* scalar store in the body and only that
pointer is mapped to a per-partition partial.  A second scalar slot
(`a = a + 1; b = b + 2;`) was previously **not recorded at all**: the loop still
classified `reduction` and every partition wrote `b` (lost updates).  A second,
different recognized operator on the *same* slot was equally invisible while the
combine folds with a single operator.  Both are now recorded as an unrecognized
global effect (`U_f`), and the reduction class requires every global effect to be
a recognized update.  Case `race/two_reduce_slots` (a 10, b 20 serial; a broken
emit gives b ≈ 20/partitions).

### G — min/max fidelity, and one deliberate refusal
`RedOp` collapsed `smin/umin/minnum/minimum` into one `Min`, so an unsigned
minimum was combined with a *signed* one (`smin`, identity INT_MAX instead of
UINT_MAX) and a NaN-propagating `minimum` with `minnum`.  Each form now has its
own flavour and the combine reproduces the body's operation exactly; the
`select(icmp …)` and guarded-store recognizers take signedness from the predicate
(`ICmpInst::isUnsigned`), as does `detectConditionalMinMax`.
The float `select(fcmp olt/ogt …)` that the DSL's `min()`/`max()` builtins emit is
deliberately **not** recognized: that select is not reorder-invariant once a NaN
or a signed zero is in the stream, so folding partition partials cannot reproduce
the serial left-to-right fold.  It stays sequential and is pinned by
`race/reduce_real_ops` (float sum parallelizes, both float min/max do not).

### B and I — refusals, now pinned
- B: `race/data_index_write` (`cnt[deg[u]] = …`) asserts `data=1
  class=sequential`; the CleanCut table is keyed by destination vertex
  (`CC_PART_OF`), so a data-valued house has no owner to steal work from.
- I: `race/int_gather` and `race/mutual_deg` now assert their refusal
  (`class=sequential`, `red=1`) inside the existing gather loop.  These are the
  loops P10 (per-source reduction engine) has to turn parallel — the assertion is
  the contract that must change with it.

### Harness hardening (the suite was not testing the tree it lives in)
1. **The suite drove a stale compiler.**  `03_run.sh` compiles the `.graph` but
   never the compiler (`GP_BIN=./GraphProgram`), so a source edit plus a suite run
   silently reported on the previous binary.  `run.sh` now rebuilds
   (`build_lowmem.sh`, incremental) when any top-level `*.cpp`/`*.h` is newer than
   `GraphProgram`, and exits 2 on a build error.  The first "verification" of the
   F fix here ran against yesterday's binary — that is how the gap was found.
2. **`expect_class`** (`run.sh`): case compiles now run with
   `GRAPH_FRONTIER_STATS=1`, and the helper asserts the verdict recorded in the
   build log (`class=…`, optionally filtered, e.g. `red=1`, `data=1`).  "No race
   today" is no longer the only contract: a loop silently turning sequential (or
   silently claimed parallel) fails the suite.
3. **`verify/class_census.sh`** (new): compiles every case with the lowering and
   PDG diagnostics on and prints one line per candidate loop (`driver=`, `red=`,
   `fw=`, `env=`, `shadow=`, `class=`, effects, plus the `[loop-classify]` PDG
   verdict).  Baseline census on this tree:

| case | class | notes |
|---|---|---|
| `algo/bfs_level` | dest-owner | body claim (`fw=1`) + round-sep shadow |
| `algo/kcore` | source-owner | the degree loop; the peel nest is driven by a DSL `while`, so it is not a candidate (the front end's `autograph_edgemap` handles the frontier form) |
| `algo/pagerank` | source-owner + dest-owner | |
| `lang/foreach_edge`, `lang/foreach_neighbor`, `lang/weight_fn` | reduction | scalar-global reductions |
| `parallel/roundsep` | dest-owner | composition A shadow |
| `parallel/nested_gather`, `parallel/nested_while2` | source-owner | |
| `parallel/dual_shadow` | sequential (+ source-owner init) | A-dual refusal |
| `parallel/int_gather`, `parallel/mutual_deg`, `parallel/reduce_plus_write`, `parallel/mixed_regions` | sequential | I / D / C refusals |

---

## 3. Open problems

Each problem: what it is, evidence, root cause, proposed fix, and how to verify.
Severity: **S1** = produces wrong results or crashes; **S2** = parallel
correctness/performance risk; **S3** = quality/maintenance.

> **Status update 2026-09-13 (§2b has the evidence):** **P1, P3, P5, P6 and P11
> are fixed** in the working tree (suite 62 pass / 0 fail). P8-residue and P9 have
> their first verified pieces — call-effect barrier, the
> `SGPL_NO_FRONTIER_MARKER` switch with the veto still default-on, and the
> invariant-slot guard — so the per-item text below is the *history* for those.
> **P10 remains open** (per-source reduction gathers still run serially, which is
> correct but not parallel); P13 is partially addressed (`SKIP_PROGRAM_RUN`).
> **P14 is new and fixed in the working tree** (found while answering "is CleanCut
> still working"): the engine re-partitioned the whole graph on *every round* —
> 85 ms per call on 100k vertices, against ~9 ms of parallel step work — which
> dominated the runtime and made 4 threads strictly slower.  Now cached; pagerank
> 100k went 1.95 s → 0.28 s at 1 thread and 2.45 s → 0.35 s at 4 threads, outputs
> bit-identical, suite 61 pass / 1 skip / 0 fail.  The one thing that still needs
> a quiet machine is the wall-clock scaling of the parallel step itself (this box
> runs at load 2.5–5.8 on 4 cores; the suite's own scaling gate skips there).

---

### P1 (S2) — `autotuner/cc` is never region-modelled (the one red suite check)

**Symptom.** `verify/run.sh` autotuner category:
```
FAIL  autotuner/cc region-modelled   predicted_ms=0 -> autotuner sees no region
```
Runtime output for `verify/cases/algo/cc.graph`:
```
[AutoTunerProfile] total kind=Traverse predicted_ns=0.000 measured_ns=0 \
    predicted_ms=0.000000 measured_ms=0.000000 pure_kernel_ns=3159405
[AutoTunerProfile] injected 0 layout conversions total
```
The kernel does run (3.16 ms) but is reported only as "pure kernel" time, never as
a region.

**Evidence.** The generated IR contains **0** `autograph_profile_region_enter`
calls and **2** `autograph_edgemap` references
(`llvm-dis` of a `DUMP_LLVM_BC_AFTER_OUTLINE` dump; commands in §5).

**Root cause.** cc is the only suite algorithm lowered to the legacy
`autograph_edgemap` engine (emitted by IRGen, `IRGenVisitor.cpp:3907`) instead of a
CleanCut owner-step. `AutoTunerPass`'s classifiers
(`isTraverseCall`/`isInsertCall`/queries at `AutoTunerPass.cpp:582-613`,
`classifyCall` at `:1372`) do not match `autograph_edgemap`, so:

- `collectOpEventsInCallOrder()` returns no events;
- `AutoTunerPass.cpp:2116  if (allEvents.empty()) return;` exits the pass;
- and even past that, the "step kernel" dual-annotation block (filtered by
  `isCleanCutStepCall()` at `AutoTunerPass.cpp:1491`, which matches only
  `autograph_frontier_step_owner*`) sits below
  `AutoTunerPass.cpp:2163  if (regions.empty()) continue;`.

**Proposed fix (implemented and validated earlier; currently reverted).**
Four edits in `AutoTunerPass.cpp`:
1. `isCleanCutStepCall()` also matches `autograph_edgemap`.
2. Replace the `allEvents.empty()` early-out with a `moduleHasStepKernels()` gate and
   iterate region keys over `eventsByGraph ∪ metaByGraphPtr`.
3. Move the `regions.empty()` check **after** the step-annotation block.
4. In that block, use `totalOps = 1` instead of `estM` when there are no sibling
   loops. (`operationCost = H·totalOps·uTrav`, and `uTrav` already prices a whole
   graph pass; with `totalOps = estM = 320000` the model predicted 394 725 ms for
   a 3.9 ms kernel.)

Validated result after the fix: `region=0 kind=Traverse layout=CSR visits=2
predicted_ms=2.467 / measured_ms=5.251` (ratio 2.13, inside the ≤5× window), and the
check passes with no layout conversions injected. A fully detailed write-up lives in
`p1GraphEasy-con-AutoTuner/CC_AUTOTUNER_EDGEMAP_REGION.md`.

**Status: fixed on branch `proof-lowering`** (commit after `9ab4056`). Re-measured on
that branch: `predicted_ms=2.467032 measured_ms=2.627129 pure_kernel_ms=2.626719`,
`autotuner/cc region-modelled` PASS. Full suite on the branch: **50 pass / 0 fail**
(the base tree is 45/1; the three extra cases are item 10 below).

---

### P2 (S1, FIXED 2026-09-12) — `proveCarrierForDependence` trusted `EQ`/zero distances without an invariance check

**What it is.** When classifying a loop, `pdg.cpp` asks LLVM's `DependenceInfo`
(`DI.depends`) about each memory pair and classifies the returned direction vector
component at the loop's own depth (`classifyDistanceComponent`, `pdg.cpp:~232`,
used by `proveCarrierForDependence`, `pdg.cpp:~292`). An `EQ` component (or an
exact zero distance) is read as "no loop-carried dependence at this level".

**Why that can be unsound.** DI is a static analysis over SCEV expressions. When a
store and a load in the same loop address `A[%v]` where `%v` is a *load that DI
cannot prove loop-invariant* (e.g. `%v = load col_idx[j]`, or a value written by
a call), SCEV represents both subscripts with the *same* symbolic value, and the
analysis can conclude `EQ` — identical addresses — which it then reports as
loop-independent. If the value actually differs between iterations, the conclusion
is wrong and the loop can be marked DOALL incorrectly.

**Status: fixed (fail closed).** `classifyDistanceComponent` is unchanged, but the
caller no longer accepts a `ProvenZero` verdict for a pair whose address is not a
function of the loop's induction variable: `zeroDistanceOnLoopMemorySubscript()`
(`pdg.cpp`, next to `proveCarrierForDependence`) rejects the verdict when either
access's GEP subscripts have a load or a call inside the loop in their operand
DAG (`isLoopMemoryDerived()`, cycle-safe, walks only the index operands, not the
base). Such a pair sets `hasUnknownAttributedDep` and clears
`hasProofOfNoCarriedDeps` — the same fail-closed treatment as P4. Kill switch
`SGPL_NO_PDG_EQ_GUARD=1` restores the old trust; `SGPL_PDG_EQ_DEBUG=1` prints
`[pdg-eq-guard]` for each downgraded pair.

**Evidence (2026-09-12).**
- A/B coverage sweep over all nine algo/parallel graph programs
  (`SGPL_LOOP_CLASSIFY_DEBUG=1`, `GRAPH_FRONTIER_STATS=1`, one compile each):
  identical loop classifications and frontier engine classes with the guard on
  and with `SGPL_NO_PDG_EQ_GUARD=1`; `doall_scaling` keeps its two
  `DOALL/proof=1` loops, `pagerank` stays 5×`SEQUENTIAL/proof=0`, and all nine
  programs are bit-identical at 1 and 4 threads. The guard therefore costs no
  parallel coverage at this tree.
- Positive control attempts: `A[0] = A[0] + 1.0` in a `while`, and
  `int k = 0; … A[k] = A[k] + 1.0` where `k` is read from a scalar slot that
  never changes inside the loop (a genuinely carried dependence — the same
  address every iteration) — both came back `hasUnknownAttributedDep=1` with
  `[pdg-eq-guard]` triggers = 0 — DI answers *confused*, not `EQ`, so the guard's
  branch is not reached by any program we have. It is defense in depth: the EQ
  route becomes reachable once P6 (pre-PDG canonicalization) turns the DSL's
  memory-carried induction variables into SCEV AddRecs. Until then P2 is
  "guarded but not demonstrably reachable", not "demonstrated miscompile".

**Proposed fix.** Before accepting an `EQ`/zero-distance component for an address
pair whose subscript is a load or a call result, require
`ScalarEvolution::isLoopInvariant(subscript, L)`; otherwise downgrade to Unknown
(fail closed). This mirrors the existing "fail closed" policy of P4.

**How to check.** `pdg.cpp` already has `printInstToStderr` and
`SGPL_PDG_CONFUSED_DEBUG`; add a similar env-gated print in the `EQ` branch of
`classifyDistanceComponent`/`proveCarrierForDependence` and run the graph programs
(pagerank default and `GRAPH_FRONTIER_REWRITE_OFF=1`, cc, nested_gather) to see
whether any pair takes the `EQ` route with a load-derived subscript.

---

### P3 (S2) — the DOACROSS wait/post runtime path is unexercised

**What it is.** `pdg.cpp` can classify a loop DOACROSS (distance-1 store-forwarded
scalar PHIs) and `parallel_runtime.c` has a wait/post protocol for DOACROSS chunks
(and deliberately *excludes* DOACROSS from block-cyclic chunking because the
protocol depends on cyclic order, see `SGPL_LOOP_CHUNK` handling around
`parallel_runtime.c:1105`).

**Status.** No program we tested has ever produced a DOACROSS classification at
this tree. Example: a classic scan —
```graph
int n = numVertices(G);  real pref[n];  pref[0] = 0.0;
int i = 1;  while (i < n) { pref[i] = pref[i - 1] + 1.0;  i = i + 1; }
```
— classifies `SEQUENTIAL` with `scalarDoAcrossPhi=0` (verified with
`SGPL_LOOP_CLASSIFY_DEBUG=1`). The reason: the DOACROSS pattern depends on
store-forwarded PHIs that only appear after `mem2reg`/canonicalization, which does
not run before the PDG at this tree (see P6). So the wait/post path has never been
validated — if it is ever exercised, correctness is unknown.

**Proposed fix / next step.** Either (a) restore pre-PDG canonicalization (P6) and
then build a DOACROSS regression test (1-thread vs 4-thread bit equality for the
scan above), or (b) disable DOACROSS classification entirely until the runtime path
has a test.

---

### P4 (S1, FIXED — keep for context; the fix has consequences) — `isConfused()` misread as independence

**What it was.** `DependenceInfo::depends()` returns a *non-null* base-class
`Dependence` object for pairs it cannot analyze, whose accessors carry worst-case
defaults — in particular `isConfused() == true` **and** `isLoopIndependent() ==
true` (`DependenceAnalysis.h:138-145`). `pdg.cpp` checked `isLoopIndependent()`
before `isConfused()`, so "I could not analyze this pair" was consumed as "the pair
is loop-independent", fabricating proofs of no carried dependence. Verified
consequences before the fix (A/B with `SGPL_NO_PDG_CONFUSED_GUARD=1`): the PageRank
scatter driver and both neighbour `+=` loops showed `hasProofOfNoCarriedDeps=1`
(and the nested-gather race in the older baseline was the same bug without the
marker guard).

**The fix (in tree).** At `pdg.cpp:872`:
```cpp
if (!::getenv("SGPL_NO_PDG_CONFUSED_GUARD") && Dep->isConfused()) {
    bool provablyIndependent = false;
    if (AA && both are load/store)
        provablyIndependent = AA->alias(...) == llvm::AliasResult::NoAlias;
    if (provablyIndependent) continue;
    Summary.hasUnknownAttributedDep = true;
    Summary.hasProofOfNoCarriedDeps = false;
    continue;
}
```
with `AAResults` plumbed into `analyzeAndAnnotateLoop` (call sites
`pdg.cpp:1504/1512`). Kill switch `SGPL_NO_PDG_CONFUSED_GUARD=1` restores the old
behavior for A/B only.

**Consequences to be aware of.**
1. The AA escape hatch is nearly useless at this tree: DSL arrays are indirect
   (pointers in globals) before canonicalization, so AA answers MayAlias for almost
   every pair; the guard therefore fails closed very broadly (see P6/P5).
2. Verified A/B effect on PageRank (g20k): every previously `proof=1` loop now
   shows `unknown=1, proof=0`, including the dangerous scatter loops; the trivial
   init loop flips `DOALL → SEQUENTIAL`. On `doall_scaling` the two real compute
   loops remain `DOALL proof=1`.
3. The unsound behavior is one env-var away — do not ship with that switch set.

---

### P5 (S2) — the `isConfused` guard is conservative at this tree (analysis blind without canonicalization)

**What it is.** Because P6 (pre-PDG canonicalization) is reverted, most DSL memory
accesses go through `internal global` pointer slots (`%p = load ptr, @arr`), so
DependenceInfo cannot analyze them at all and returns *confused* for large clusters
of pairs. With the P4 guard on, any loop containing such a pair fails closed.
Observed: the init loops in PageRank and `doall_scaling` moved DOALL → SEQUENTIAL;
several `[pdg-confused]` pairs are benign in reality (a scalar global load next to
an array store) but cannot be separated by AA at this stage.

**Why it matters.** It silently reduces parallel coverage; not a correctness bug
(the direction is safe), but it also masks how much the DOALL machinery would
actually give.

**Proposed fix.** Restore pre-PDG canonicalization (see P6). Then the AA escape
hatch in P4 becomes meaningful and the conservatism shrinks to genuinely
unanalyzable pairs.

---

### P6 (S3, decision pending) — pre-PDG canonicalization is absent

**What it is.** The developer's baseline diff contained a change (item 7 in their
notes) that runs `canonicalizeLoopsForAnalysis(M)` (mem2reg + loop-simplify +
SimplifyCFG + `GlobalOpt`) **before** `runPDGOnModule` on every pipeline path.
Without it the PDG sees reloads of `internal global` DSL variables every iteration
and `DependenceInfo` must report unknown/confused (`main.cpp` is the call site;
the baseline version can be diffed with
`git diff pdg-prework-original pdg-prework-baseline -- p1GraphEasy-con-AutoTuner/main.cpp`).

**Consequences at the current tree.** Weak PDG proofs (P5); DOACROSS never
materializes (P3); the `isConfused` guard is maximally conservative.

**Proposed fix.** Re-apply just that hunk (`SGPL_NO_PDG_CANON=1` kill switch in the
baseline version) and re-run the suite + the classification A/B. Watch for: the
proof-override interaction (baseline item 1) is *not* in this tree — the original
marker logic (`pdg.cpp:~1015`, `IsNestedFrontierLoop → SEQUENTIAL`) is kept, so
canonicalization should only *add* provable DOALLs, but that has not been verified
end-to-end here.

---

### P7 (S3, FIXED 2026-09-12 — was verified dead code with a correctness-shaped intent) — `sgpl.frontier.first_wins.candidate` was read but never emitted

**What it was.** `pdg.cpp` read loop metadata named
`sgpl.frontier.first_wins.candidate` (to identify "verified frontier" loops), but
the emitters attach a different name — `sgpl.frontier.first_wins.doall`
(`graph_frontier_lowering.cpp:1758` writes it under `UseEnvelope`; the outliner's
`RequiredIntAppendPriv` reads it). Nothing anywhere wrote `.candidate` (verified
with a whole-repo grep over all text files), so the `IsVerifiedFrontier`
condition was permanently false.

**Why it mattered.** Either a real analysis path was silently dead (a class of
loops that should be treated specially is not), or the reader was leftover.

**Fix (deleted, not renamed).** The reader was removed together with the
`IsVerifiedFrontier` classification branch, the `sgpl.frontier.first_wins.doall`
write that sat inside that (unreachable) branch, and `markNestedLoopsSequential()`,
whose only non-recursive caller was that branch. Renaming the reader to `.doall`
was rejected: the branch would still have been dead, because the lowering marks
every claimed driver nest `sgpl.frontier.nested.sequential` and that check is
evaluated first (`IsNestedFrontierLoop` → SEQUENTIAL), so activating it would have
reintroduced exactly the unproven "traversal state is parallel" path that P4/P8
exist to remove.

**Evidence (2026-09-12).** Whole-repo grep: `.candidate` appears only in the two
handoff documents and `.doall` has exactly one writer (lowering) and one reader
(outliner). `bfs_level` IR after the deletion: `sgpl.frontier.first_wins.doall`
0, `.candidate` 0, `sgpl.frontier.nested.sequential` 2, with
`autograph_frontier_step_owner_push` ×2 emitted. Full suite 50 pass / 0 fail and
the nine-program coverage sweep is unchanged.

---

### P8 (S2) — the frontier lowering is still shape-fragile (only the hit cases are guarded)

**What it is.** The CleanCut/owner-step rewrite in `graph_frontier_lowering.cpp`
claims graph loops by pattern and emits engine calls plus work functions. Two
miscompile classes have been observed and fixed with the
`neighborLoopHasForeignCalls()` guard (P2 in §2): inline queries (`hasEdge`) and
FP-reduction bodies. The guard is a *blocklist* — subloops, unknown calls, FP
header PHIs. Other unmodeled body shapes can still be claimed, and the failure
mode of a bad claim has historically been either a compiler crash (downstream,
e.g. in LLVM passes) or a silently wrong program (the `0.000000` gather).

**Status: implemented on branch `proof-lowering` (default), blocklist kept behind
`SGPL_FRONTIER_BLOCKLIST_GUARD=1` for A/B.** Three pieces, all in
`graph_frontier_lowering.cpp`:

1. **Totality prover replaces the blocklist** (`provesModelable()`). Every element
   of the candidate loop is either inside the model — array effects with U/V/D
   provenance handled by the emitters, first-wins, set-appends — or the loop is
   refused with a *named reason*. Conditions: no subloops (nested control flow has
   no per-pair meaning); every call must be in the modelled runtime allowlist
   (`autograph_neighbor_iter_{init,next}`, `roaring_bitmap_{add,remove}`, profile
   helpers, intrinsics) — indirect or unknown calls are opaque; every header PHI
   must be integer and must not escape the loop (an escaping scalar is a reduction
   register the engine has no state for). New shapes are now refused by
   construction instead of rewritten blind.
2. **Emit-side postcondition, always on.** After every successful emission the
   function is run through `verifyFunction`. A failure is reported and the loop is
   left on the conservative path; under `SGPL_FRONTIER_STRICT=1` (now exported by
   `verify/run.sh`, so every suite compile is checked) it aborts instead. This is
   the gate that catches the historical emission bug class — an in-memory module
   with `fadd ptr, double` cannot pass verification (negative control:
   `opt -passes=verify` rejects it, and rejects a malformed-but-parseable PHI the
   same way).
3. **Priority-2 hand-off.** `SGPL_PDG_SECOND_CHANCE=1` makes a refusal skip
   `markSequential(L)` and release the loop to the dependence analysis, which then
   has to issue its own fail-closed certificate (`hasProofOfNoCarriedDeps=1`) to
   parallelize it. Default stays the terminal refusal: traversal state is invisible
   to DI until call effects are modelled, so releasing by default is not yet sound.

**Evidence on the branch.** Refusal-reason sweep over every suite graph program:
pagerank 2/2 candidates modelable (claimed, postcondition-verified), bfs_level 1/1,
kcore 1/1, sssp 0, cc 0; the only refusals are the two known classes —
`nested_gather` → `loop-carried non-integer PHI (reduction register)`,
`mutual_deg` (inline `hasEdge`) → `contains subloops`. With
`SGPL_PDG_SECOND_CHANCE=1`: suite **50/0**, and forced-parallel (`4 threads ×3`
vs 1 thread, `SGPL_FORCE_DOALL_PARALLEL=1`) bit-identical for pagerank, bfs_level,
kcore, sssp, nested_gather, int_gather, mutual_deg. The one behaviour change
observed under the switch is an *improvement*: `nested_gather`'s leaf update loop
(`val[v] = 1.0 + acc[v]*0.0001`, owner-computes, no traversal) keeps its honest
PDG proof instead of inheriting SEQUENTIAL from the refusal's ancestor cascade
(`markSequential` marks L, all ancestors and all subloops). The traversal loop
itself stays SEQUENTIAL by analysis, not by marker.

---

### P9 (S3) — the syntactic frontier marker remains the concurrency policy

**What it is.** Loops under a frontier driver are forced SEQUENTIAL by IRGen/
lowering metadata (`sgpl.frontier.nested.sequential`, attached in
`graph_frontier_lowering.cpp:~729`, read in `pdg.cpp:~1015`). The marker stands in
for a real dependence the analysis cannot see: the traversal state (neighbour
variable, iterator slot) that a parallel dispatch would share.

**The designed replacement (not implemented).** Fail-closed certification: DOALL
requires a positive proof at every nesting level; DOACROSS requires a proven
positive-constant distance at the carrying level and `EQ` at all shallower levels;
any confused/unknown/missing level → SEQUENTIAL. With call-effect modeling (treat
unknown-effect calls inside a loop as unknown barriers; give the runtime functions
honest memory attributes or an effect table), the marker becomes redundant and the
verdicts are derived rather than asserted. The `isConfused` guard (P4) is the first
piece of this; the rest is open.

---

### P10 (S2, feature) — per-source reduction (gather) loops cannot use the engine

**What it is.** `for each vertex u { real s = 0; for each neighbor v { s += f(v) }
acc[u] = s }` (nested_gather) must run sequentially: the owner-source engine calls
the work function once per edge with a shared `env`, so there is nowhere to keep
the per-source accumulator, and the epilogue store has no home (the old code
deactivated the driver and lost the store — see §2 item 7).

**Proposed fix (design).** Extend the owner-source engine + codegen:
1. runtime: per-partition env slice (the engine already has per-lane scratch
   mechanics) plus a per-partition completion hook;
2. codegen: emit the work function with source-change detection
   (`if (u != env->last_u) { store acc[last_u] = env->s; env->s = 0; }`),
   a `flush_fn` storing the final accumulator, and the reduction PHI mapped to
   `env->s`;
3. lowering: recognize the pattern (only when the accumulator's sole escape is the
   `acc[u]` store) and keep the store alive.
Because source partitions are disjoint and visited in ascending order, results stay
bit-identical to the serial run. Until then, these loops are correct but serial.

---

### P11 (S1?, unconfirmed) — `while` inside `for each vertex` was reported to segfault

**Origin.** The developer's handoff notes claim "a `while` loop nested inside a
`for each vertex` body segfaults the generated program", with a minimal repro of an
`int k = 0; while (k < 12) { …; k = k + 1; }` body.

**Status.** Not reproduced in this session: the minimal shape compiled and ran
correctly (`acc0 12.000000`) at 5 and 20 000 vertices, under default pipeline,
`SGPL_FORCE_DOALL_PARALLEL=1`, and `GRAPH_FRONTIER_REWRITE_OFF=1`. It may depend on
other body shapes (graph-data reads inside the `while`, larger graphs, specific
kernels). Keep as an unconfirmed report; a small fuzzer over loop-body shapes in
the DSL would settle it.

---

### P12 (S3, FIXED 2026-09-12) — `scaling/doall_scaling` was load-sensitive

The check requires 4-thread ≥ 1.2× of 1-thread. On an idle 4-core box it measured
1.45×–2.4×; once, under load average 7.7, the suite recorded 0.87× and failed. Not
a code bug (standalone re-measurement immediately showed 1.39–1.60×), but the
suite could go red on a busy machine.

**Fix.** `verify/run.sh` now always asserts correctness (`race/doall_scaling
1thr==4thr`) and only asserts the throughput threshold while the 1-minute load
average is ≤ the core count, with best-of-5 timing and a `SKIP` line that still
reports the measured speedup. `SGPL_SCALING_MAX_LOAD=N` overrides the bound
(`0` forces the skip branch); the summary line now prints `skipped: N`.

**Evidence (2026-09-12).** Both branches: the default run asserted the threshold
at `1.94x` (load 3.20, exit 0, suite 51/0/0);
`SGPL_SCALING_MAX_LOAD=0 bash run.sh parallel` printed
`SKIP scaling/doall_scaling 4thr 2.21x, not asserted (load 2.81 > 0 cores)` and
ended `passed: 15   failed: 0   skipped: 1`, exit 0.

---

### P13 (S3) — misc

- `pdg.cpp`'s loop-classify logger is enabled relative to upstream (env-gated,
  intentional; it is a local diff).
- The repo-wide `Base.g4` does not match the parser actually compiled into the
  binary (`generated/` came from a different grammar copy). Editing the grammar and
  regenerating will drift the language (this burned an attempt earlier). Treat
  `generated/` as authoritative.
- `verify/` and `algo_validation/` are untracked directories in the repo; the
  suite's expectations (`expected/*.json|manifest`) are part of that untracked
  state.

### P14 (S2, FIXED 2026-09-13) — the engine re-partitioned the graph every round

**Symptom.** Every CleanCut program (pagerank, bfs_level, kcore, dual_shadow) ran
*slower* with 4 threads than with 1 (0.81×–0.93× across 20k and 100k graphs), and
the slowdown got worse as the graph grew.  CPU/wall stayed ≈ 1.0, i.e. the whole
process consumed about one core no matter how many threads were configured.

**Root cause (measured, not inferred).** The lowering emits
`autograph_build_clean_cut(graph, 0)` **inside** the round loop (verified in the
emitted driver: the call sits in block `foreach.merge50`, between the region-1
exit and the region-2 enter of the per-round scatter).  That function re-enumerates
every arc of the graph and rebuilds all per-partition structures — including
16 fresh partition copies of the arc list (800k arcs at 100k vertices).

```
[clean-cut] build_call=1 ns=85565306 parts=0
[clean-cut] build_call=2 ns=90266692 parts=0
...                                     # one per round, ~85 ms each
```

Per-round cost breakdown on 100k vertices / 800k edges (rounds sweep at 1 thread:
`rounds=1 → 200 ms`, `rounds=4 → 476 ms`, `rounds=20 → 1950 ms`, i.e. 108 ms fixed
+ 92 ms/round): the serial rebuild is ~85 ms of each 92 ms round, while the
parallel step it feeds (`autograph_frontier_step_owner_push`, profiled region 2)
is only ~9 ms.  Nothing about thread count or partition count can fix that —
Amdahl caps the speedup at ~1.05×, and the extra threads only add overhead.  This
also explains why `SGPL_CLEANCUT_PARTITIONS` (1…64) and
`SGPL_FORCE_DOALL_PARALLEL` changed nothing.

**Fix.** `autograph_build_clean_cut()` is now a thin wrapper around the old body
(`autograph_build_clean_cut_inner`) with a reuse cache: the partition structures
are a pure function of (graph, layout epoch, topology, partition count), so the
build is skipped until one of those actually changes.  Invalidation points:

- `meta->layout_epoch` (bumped by `autograph_set_layout`, i.e. every injected
  layout conversion) and the `live_edge_count` / `extra_edge_count` counters in
  the cache key;
- explicit `autograph_clean_cut_cache_invalidate()` in all six mutation entry
  points (`autograph_canonical_{add,remove}_{node,edge}`,
  `autograph_bcsr_{add,remove}_edge`), because the BCSR mutators do not touch the
  counters;
- kill switch `SGPL_NO_CLEANCUT_CACHE=1` restores rebuild-every-call.

`SGPL_CLEANCUT_TIMING=1` prints per-call build/reuse times.

**Verification (all on this tree).**
- pagerank 100k / 20 rounds: 1 thread 1.95 s → **0.28 s**; 4 threads 2.45 s →
  **0.35 s**; output `1.785825304261e-05` unchanged at both thread counts.
- pagerank 100k / 200 rounds (step-dominated): 4 threads **23.9 s** with
  `SGPL_NO_CLEANCUT_CACHE=1` → **2.4 s** with the cache (13×), i.e. the rebuild
  really was the bulk.  Timing lines show 1 build + 20 reuses per 20-round run.
- Cached vs uncached outputs bit-identical on pagerank, bfs_level, kcore, sssp.
- Full suite: **61 pass / 1 skip / 0 fail** (the skip is the load-aware
  `scaling/doall_scaling` gate, which reported 1.45× measured under load 4.29).

**Still open (measurement, not a known defect).** Whether the parallel step
itself shortens wall time cannot be measured on this box: load runs 2.5–5.8 on 4
cores.  The runtime's own dispatch log shows the steps *are* launched with
threads=4 and do **not** take the serial budget fallback:

```
GRAPH_PARALLEL_DEBUG=1 ...
[parallel-runtime] plain-call=1 trips=16 total_trips=16 range=[0,16) step=1 threads=4
```

so the machinery is engaged; a quiet-machine run (or a bigger graph) is needed to
see the step-level speedup.  Do not "fix" a serialisation that the logs say is
not happening.

---

## 4. Suggested order of work

Done since the first edition: **P1** (suite green), **P2** (zero-distance
invariance guard), **P7** (dead metadata reader deleted), **P12** (load-aware
scaling check), **P14** (per-round CleanCut rebuild cached — pagerank 100k 7×
faster), plus the P8 pieces on this branch.

What is left:

1. **P14 follow-up (measurement only)** — confirm the parallel step's wall-clock
   speedup on an idle 4-core machine (this box runs at load 2.5–5.8); the
   dispatch log already shows `threads=4` with no serial fallback, so expect a
   step-level speedup there, not a bug hunt.
2. **P6 then P5/P3** — canonicalization weakens the conservatism, unlocks a real
   DOACROSS test, and is the prerequisite for the P2 guard to become reachable.
   Highest-leverage analysis change; needs the P5/P3 A/B re-run afterwards.
3. **P10** — per-source (per-vertex accumulator) gathers still run serially; the
   global-scalar reduction path (`autograph_frontier_step_owner_red` with
   per-partition partials) landed, so the remaining work is the per-source slice
   + flush plus the `acc[u] = s` epilogue.
4. **P9** — the `sgpl.frontier.nested.sequential` marker is still the concurrency
   policy; a certificate-based classifier (DOALL needs a positive proof at every
   nesting level, DOACROSS needs a proven positive-constant distance with `EQ`
   at all shallower levels) is the designed replacement.
5. **P8 residue / P13** — call-effect modelling for the traversal runtime
   (memory attributes or an effect table) would let refusals hand off to the PDG
   by default; grammar/generated-parser drift and the untracked `verify/` tree
   remain as documented.

## 5. How to reproduce the key observations

```bash
cd /home/user/D/Course/msk1/2nd/Graph-language-/p1GraphEasy-con-AutoTuner

# P1: cc never region-modelled
rm -f final_program
GRAPH_FILE=../verify/cases/algo/cc.graph bash ./03_run.sh </dev/null >/tmp/cc.log 2>&1
grep -E 'region=|total kind=Traverse' /tmp/cc.log            # predicted_ms=0.000000
rm -f final_program
DUMP_LLVM_BC_AFTER_OUTLINE=/tmp/cc.bc GRAPH_FILE=../verify/cases/algo/cc.graph \
    bash ./03_run.sh </dev/null >/dev/null 2>&1
export PATH=/usr/local/llvm-20-polly-rtti/bin:$PATH
llvm-dis /tmp/cc.bc -o /tmp/cc.ll
grep -c autograph_profile_region_enter /tmp/cc.ll            # 0
grep -c autograph_edgemap /tmp/cc.ll                         # 2

# P4/P5: classification A/B on pagerank
rm -f final_program
SGPL_LOOP_CLASSIFY_DEBUG=1 GRAPH_FILE=../verify/cases/algo/pagerank.graph \
    bash ./03_run.sh </dev/null 2>&1 | grep classification=       # guard ON: proof=0
rm -f final_program
SGPL_NO_PDG_CONFUSED_GUARD=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
    GRAPH_FILE=../verify/cases/algo/pagerank.graph bash ./03_run.sh </dev/null 2>&1 \
    | grep classification=                                        # guard OFF: proof=1

# P3: the scan loop classifies SEQUENTIAL (no DOACROSS)
cat > /tmp/scan.graph <<'EOF'
graph G { edges: file "/home/user/D/Course/msk1/2nd/Graph-language-/verify/fixtures/g20k.txt"; };
int n = numVertices(G); real pref[n];
pref[0] = 0.0; int i = 1;
while (i < n) { pref[i] = pref[i - 1] + 1.0; i = i + 1; }
print "last"; print pref[n - 1];
EOF
rm -f final_program
SGPL_LOOP_CLASSIFY_DEBUG=1 GRAPH_FILE=/tmp/scan.graph bash ./03_run.sh </dev/null 2>&1 \
    | grep classification=                                        # SEQUENTIAL, scalarDoAcrossPhi=0

# P2: zero-distance invariance guard A/B (classifications must be identical;
# 4 threads must reproduce the 1-thread answer for the same-address case)
rm -f final_program
SGPL_LOOP_CLASSIFY_DEBUG=1 GRAPH_FILE=../verify/cases/parallel/same_slot.graph \
    bash ./03_run.sh </dev/null 2>&1 | grep classification=   # SEQUENTIAL, unknown carrier
rm -f final_program
SGPL_NO_PDG_EQ_GUARD=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
    GRAPH_FILE=../verify/cases/parallel/same_slot.graph bash ./03_run.sh </dev/null 2>&1 \
    | grep classification=                                    # same verdict (guard not reached)
rm -f final_program
SGPL_PDG_EQ_DEBUG=1 GRAPH_FILE=../verify/cases/parallel/same_slot.graph \
    bash ./03_run.sh </dev/null 2>&1 | grep -c pdg-eq-guard   # 0: no EQ verdict to downgrade

# P7: the dead reader is gone — nothing anywhere writes .candidate
grep -rn 'first_wins.candidate' --include='*.cpp' --include='*.h' .   # (no hits outside docs)
grep -rn 'first_wins.doall'     --include='*.cpp' --include='*.h' .   # lowering writer + outliner reader

# P12: both branches of the load-aware scaling check
bash run.sh parallel                                        # asserts >=1.2x when load <= cores
SGPL_SCALING_MAX_LOAD=0 bash run.sh parallel                # SKIP line, exit 0

# Merged tree (branch merge-ars1-roundsep): upstream's acceptance harnesses
bash test/run_frontier_shadow_tests.sh                      # 3 configs, 0 failures
bash test/run_frontier_red_tests.sh                         # 24 configs, 0 failures
bash validate_roundsep.sh                                   # composition A E2E
bash validate_reduction.sh                                  # composition H E2E
# upstream's older composition harness hardcodes /mnt/d/... and names the
# pre-rename fixtures; run a patched copy:
sed -e "s|^cd /mnt/d/sgpl/bench/p1GraphEasy-con-AutoTuner|cd $PWD|" \
    -e 's/reduce_add reduce_sub/reduce_sum reduce_sub/' validate_composition.sh > /tmp/vc.sh
bash /tmp/vc.sh

# suite status
cd ../verify && bash run.sh
```

## 6. Glossary

- **DOALL / DOACROSS / SEQUENTIAL** — loop classifications written to
  `!my.loop.parallel` metadata; DOALL = all iterations independent; DOACROSS = a
  distance-1 carried dependence handled with wait/post; SEQUENTIAL = not
  parallelised.
- **CleanCut / owner-step** — the current engine: destination-partitioned
  accumulation (owner-push) or source-partitioned traversal (owner-source), so a
  shared array element is written by exactly one thread; results are bit-identical
  to serial (verified repeatedly in the race suite).
- **edgemap** — the legacy CAS/combine engine (`autograph_edgemap`), used for
  min/claim-style updates (cc).
- **Range body** — the outlined loop's `(lo, count, step, env)` entry point that
  lets `-O3` inline/vectorise the parallel clone.
- **Marker** — `sgpl.frontier.nested.sequential` loop metadata, the syntactic
  stand-in for "this loop nests traversal state".
- **Confused** — LLVM DependenceInfo's non-null, all-unknown answer for pairs it
  cannot analyze. Not to be confused with `nullptr` ("no dependence").
