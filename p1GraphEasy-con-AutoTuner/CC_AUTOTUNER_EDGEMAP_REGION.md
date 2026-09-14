# `autotuner/cc` — why the check fails: cc's edgemap kernel is never profiled as a region

Status: analysis of the failure with the session's verified fix (the fix itself lives
in the reverted baseline diff — this tree is at the original commit `e8b89f9` plus
the real-array/INF/2D/foreach-edge/frontier/`isConfused` changes; see §8).

Affected check (from `verify/run.sh`, autotuner category):

```
FAIL  autotuner/cc region-modelled   predicted_ms=0 -> autotuner sees no region
```

---

## 1. What the check does

`verify/run.sh` (lines ~140-154) compiles each algorithm, runs it once, and reads the
runtime's autotuner profile line:

```
line=$(cd $C && ./final_program 2>&1 | grep "total kind=Traverse" | head -1)
p=$(... "predicted_ms=..." )     # must be > 0
if [[ -z "$p" || "$p" == "0.000000" ]]; then
  no "autotuner/$c region-modelled" "predicted_ms=0 -> autotuner sees no region"
```

So for cc the runtime must report a *predicted* time for at least one `Traverse`
region; a prediction of exactly `0.000000` means the autotuner never modelled any
region in that program.

## 2. Symptom, verified on this tree

Compile `verify/cases/algo/cc.graph` and inspect (commands in §9):

1. The runtime profile for cc:

```
[AutoTunerProfile] predicted_vs_measured_region_times
[AutoTunerProfile] total kind=Traverse predicted_ns=0.000 measured_ns=0 predicted_ms=0.000000 measured_ms=0.000000 pure_kernel_ns=3159405 pure_kernel_ms=3.159405
[AutoTunerProfile] total kind=Insert predicted_ns=0.000 ...
[AutoTunerProfile] total kind=Query predicted_ns=0.000 ...
[AutoTunerProfile] injected 0 layout conversions total, conversion_ns=0
```

Note `pure_kernel_ms=3.159405`: the kernel *did* run and its time *was* measured —
but as "pure kernel" time, not as a profiled region. Hence no prediction.

2. The generated IR contains **zero** region annotations:

```
$ llvm-dis /tmp/cc_g.bc   # dumped with DUMP_LLVM_BC_AFTER_OUTLINE
$ grep -c autograph_profile_region_enter  -> 0
$ grep -c autograph_edgemap               -> 2   (1 call + 1 declaration)
```

3. What cc lowers to (IR, in `main`):

```llvm
%edgemap.next.size.i = call i32 @autograph_edgemap(
    ptr %G.graph18.i, i32 1, ptr %frontier.frontier.ptr.i, i32 %frontier_size172.i,
    ptr %next_frontier.frontier.ptr.i, i32 0, ptr %id.frontier.ptr.i, ptr null,
    i32 0, i32 0), !sgpl.edgemap.step
```

## 3. Background: how the autotuner is supposed to work

`AutoTunerPass.cpp` (module pass):

1. **Region discovery** – `collectOpEventsInCallOrder()` walks `main`, classifies
   engine calls through `classifyCall()` / `isTraverseCall()` / `isInsertCall()` /
   `isSetSpecificQuery()` / `isCSRSpecificQuery()` and produces per-graph `OpEvent`
   lists.
2. **Region building** – `buildRegions()` + `mergeSmallRegions()` turn events into
   `Region`s (dominant kind, anchor instruction, op histogram, execution count H).
3. **A second source of regions** – the "CleanCut dual annotation" block
   (`AutoTunerPass.cpp:2166`): owner-computes step calls
   (`autograph_frontier_step_owner_*`) are annotated as `Traverse` regions even when
   they have no accompanying loop events, anchored on the call itself.
4. **Cost model** – `operationCost(R, layout, n, m, ...) = H * totalOps * (fT*uTrav + fI*uIns)`
   (plus insert setup). `uTrav`/`uIns` are per-op costs derived from the hardware
   calibration; the layout DP then picks a layout per region.
5. **Injection** – `injectConversions()` inserts, around each region anchor:
   `autograph_profile_region_enter(regionId, kind, layout, predictedNs)` before, and
   `autograph_profile_region_exit(regionId)` after (for step-anchored regions the
   exit goes immediately after the anchored call, `AutoTunerPass.cpp:1968`).
6. **Runtime accounting** – `autograph_profile_region_enter/exit` accumulate
   predicted/measured per kind; `total kind=<K>` lines are what run.sh reads.

So "predicted_ms=0" means: **no `profile_region_enter` was ever emitted for a
`Traverse` region** — steps 1–3 found nothing to build regions from.

## 4. Root cause

cc is the only algorithm in the suite whose kernel is lowered to the **legacy
`autograph_edgemap` engine call** (IRGen's compositional EdgeMap effect,
`IRGenVisitor.cpp:2204` and the call emission at `IRGenVisitor.cpp:3907`). The
CleanCut path (`autograph_frontier_step_owner_*`) is a different lowering.

The autotuner's classifiers know:

```
isTraverseCall(): bfs_runtime, bfs_runtime_src, dfs_runtime, dfs_runtime_src,
                  autograph_frontier_step
isInsertCall():   graph_add_node, graph_add_edge, graph_remove_node,
                  graph_remove_edge, roaring_bitmap_add/remove*
queries:          roaring_bitmap_contains/get_cardinality/union/intersect,
                  graph_get_edge_weight*, get_edge_src/dst_by_id
```

`autograph_edgemap` matches **none** of them. Consequences, each independently
fatal:

1. **No events** → `collectOpEventsInCallOrder()` returns empty →
   `AutoTunerPass.cpp:2116  if (allEvents.empty()) return;` exits the whole pass.
2. Even without that early-out, the per-graph loop iterates `eventsByGraph`, which
   is empty for cc.
3. Even if it iterated, the dual-annotation block is (a) filtered by
   `isCleanCutStepCall()` (`AutoTunerPass.cpp:1491`), which matches only
   `autograph_frontier_step_owner*`, and (b) sits below
   `AutoTunerPass.cpp:2163  if (regions.empty()) continue;`.

Net effect: zero regions → zero annotations → `predicted_ms=0`. The kernel time is
still recorded (`autotuner_runtime.c:2382` — `autograph_edgemap()` calls
`autograph_profile_record_kernel_ns(0, ...)`, which feeds `pure_kernel_ns`), which
is why the log shows real work but no prediction.

## 5. The fix (implemented and validated in the session, currently reverted)

Four changes in `AutoTunerPass.cpp`:

1. **Step-kernel recognition** — extend `isCleanCutStepCall()` (1491) to match
   `autograph_edgemap` as well (one predicate for both step-style engines):

```cpp
if (Callee && (Callee->getName().starts_with("autograph_frontier_step_owner") ||
               Callee->getName() == "autograph_edgemap"))
    return true;
```

2. **Process graphs even with no call events** — replace the `allEvents.empty()`
   early-out with a `moduleHasStepKernels(M)` gate, and iterate region keys from
   `eventsByGraph ∪ metaByGraphPtr` so an edgemap-only program reaches the
   annotation block.
3. **Collect the step regions before the empty-regions check** — move
   `if (regions.empty()) continue;` *after* the dual-annotation block, so
   step-only graphs produce regions.
4. **Per-pass op-count fallback** — in the step-region block, when there are no
   sibling loop regions, use `totalOps = 1`, **not** `estM`:

```cpp
// (AutoTunerPass.cpp:2203 before the fix)
//   loopOps = static_cast<uint64_t>(estM);      // edge count — wrong
// after:
//   loopOps = 1;   // traversalCost already prices a whole graph pass
```

   Why this matters: `operationCost = H · totalOps · uTrav`, and `uTrav` is already
   the cost of one full graph traversal. With `totalOps = estM = 320000` the
   model predicted **394 725 ms** for a 3.9 ms kernel (ratio ~1e5, failing the
   ≤5× accuracy check); with `totalOps = 1` the prediction is one traversal —
   which is what a step call actually costs.

## 6. Validation of the fix (measured, from the session)

Same commands as §9 on the fixed tree:

```
[AutoTunerProfile] region=0 kind=Traverse layout=CSR visits=2
                   predicted_ns=2467032.000 measured_ns=5251336
                   predicted_ms=2.467032    measured_ms=5.251336
```

- `predicted_ms > 0` → `autotuner/cc region-modelled` PASS.
- `measured/predicted = 2.13`, inside the check's ≤5× window.
- `injected 0 layout conversions` — no layout churn: the step region's effective
  op count (1) is below `kMinRegionOpsToSwitch = 2` (`AutoTunerPass.cpp:149`), so
  the DP keeps CSR, and the engine gets profiled without changing behaviour.
- The other autotuner rows (bfs_level 2.8×, kcore 1.1×, pagerank 1.7×, sssp 0.9×)
  were unaffected.

## 7. Why it is red *right now* on this tree

The fix is part of the uncommitted baseline diff which was reverted on request
(current tree = `e8b89f9` + the language/bug fixes). At this commit the pass only
understands CleanCut step calls and loop events, so the failure is **expected and
pre-existing** — it is one of the same three reds the suite showed before the
`isConfused` guard was added (the guard did not cause or change it).

## 8. How to restore the fix

- From the baseline snapshot: `git checkout pdg-prework-baseline -- p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp`
  (then re-apply the other session changes), or
- Re-apply the four edits of §5 by hand; no kill switch existed for them.

## 9. Reproduce / verify (copy-paste)

```bash
cd /home/user/D/Course/msk1/2nd/Graph-language-/p1GraphEasy-con-AutoTuner

# compile + link + run cc, capture the profile
rm -f final_program
GRAPH_FILE=../verify/cases/algo/cc.graph bash ./03_run.sh </dev/null >/tmp/cc.log 2>&1
grep -E "region=|total kind=Traverse" /tmp/cc.log          # predicted_ms=0.000000 -> failure

# prove there are no region annotations in the IR
rm -f final_program
DUMP_LLVM_BC_AFTER_OUTLINE=/tmp/cc_g.bc GRAPH_FILE=../verify/cases/algo/cc.graph \
    bash ./03_run.sh </dev/null >/tmp/cc_g.log 2>&1
export PATH=/usr/local/llvm-20-polly-rtti/bin:$PATH
llvm-dis /tmp/cc_g.bc -o /tmp/cc_g.ll
grep -c autograph_profile_region_enter /tmp/cc_g.ll   # 0  -> never instrumented
grep -c autograph_edgemap              /tmp/cc_g.ll   # 2  -> the unprofiled kernel

# the suite check itself
cd ../verify && bash run.sh autotuner 2>&1 | grep -E 'cc|passed'
```

## 10. File map

| Concern | Where |
|---|---|
| Region discovery / classification | `p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp:1372-1399` (`classifyCall`), `:582-613` (predicates) |
| Step-kernel predicate | `AutoTunerPass.cpp:1491` (`isCleanCutStepCall`) |
| Early-out on empty events | `AutoTunerPass.cpp:2116` |
| Dual annotation block | `AutoTunerPass.cpp:2166-2242` |
| Empty-regions gate | `AutoTunerPass.cpp:2163` |
| Op-count fallback | `AutoTunerPass.cpp:2203` |
| Step-anchored exit placement | `AutoTunerPass.cpp:1968` |
| Layout-switch floor | `AutoTunerPass.cpp:149` (`kMinRegionOpsToSwitch`) |
| Edgemap lowering (IRGen) | `IRGenVisitor.cpp:2204` (effect), `:3907` (call emission) |
| Edgemap kernel + kernel-time record | `autotuner_runtime.c` (`autograph_edgemap`, `:2382`) |
| Suite check | `verify/run.sh:140-154` |
