# PDG Scheduler Root-Cause Analysis and Fix Report

## 1) Problem Statement

The BFS DSL program was producing incorrect results after PDG-based parallelization: distances were observed as `0` when they should have been positive (for example, `dist[0]` should be `2` for source frontier `{2}` on the chain graph `0-1-2-3-4-5`).

The issue was not in BFS logic itself. The same generated program produced correct answers when executing the sequential path (`main_original`), but failed in the transformed parallel path (`main`).

## 2) What Was Actually Wrong (Root Cause)

The core bug was in the PDG reconstruction strategy inside `reconstructParallelIR` in `pdg.cpp`:

1. It rebuilt a synthetic `main_parallel` function with a linear execution model.
2. It cloned a subset of initialization and then executed topological task levels once in a flat sequence.
3. It renamed/swapped functions so this synthetic function became the program `main`.

This broke semantics for programs with loops/branches because static task levels were executed outside original dynamic control flow.

### Why BFS broke specifically

BFS relies on repeated loop iterations (`while (frontier.size() > 0)`), with frontier updates feeding the next iteration. The synthetic flat schedule executed task levels without preserving loop back-edge semantics. As a result:

- the traversal did not evolve through iterations as intended,
- updates to `dist` were incomplete,
- zero-initialized array values remained visible and appeared as incorrect results.

## 3) Secondary Behavioral Gap Found

Inside the previous reconstruction logic, `serialTasks` were identified but not actually executed in the generated schedule. That could silently drop required sequential work even when loop semantics were not involved.

## 4) Fix Strategy Implemented

We replaced the unsafe flat reconstruction with an in-place, CFG-preserving scheduling approach:

- Keep original `main` control flow as the execution skeleton.
- Perform parallel spawning/joining at real extracted callsites in original basic blocks.
- Group only safe call clusters by `(topological level, loop region, basic block)`.
- Leave anything ambiguous or unsafe untouched (sequential fallback).

This preserves loop/branch semantics by construction.

## 5) Implementation Details

All changes are in `p1GraphEasy/pdg.cpp`, in `reconstructParallelIR`.

### 5.1 Removed unsound synthetic-main path

Removed the block that:

- created `main_parallel`,
- attempted to rematerialize values into that function,
- executed levels in one global loop,
- swapped `main` and `main_original`.

### 5.2 Added control-region and level planning structures

Introduced local scheduler structures:

- `TaskRegionInfo { taskId, regionLoop, anchorCallsites, ambiguousRegion }`
- `LevelPlan { parallelTasks, serialTasks }`
- `RegionSchedule { regionLoop, levels }`
- `CallGroup { levelIdx, regionLoop, bb, taskIds, calls }`

Loop region identity is derived using LLVM `LoopInfo` (`nullptr` means ROOT region).

### 5.3 Built task-to-callsites map from extracted functions

For each extracted task function:

- locate callsites in original `main`,
- assign region from `LI.getLoopFor(callsite->getParent())`,
- mark as ambiguous if task appears in multiple regions.

Ambiguous tasks are not parallelized by this grouping pass.

### 5.4 Derived grouped execution opportunities

For each global topological level:

- extracted task with exactly one unambiguous anchor => candidate for grouped parallel execution,
- otherwise => treated as serial/fallback in plan metadata.

Candidates are grouped by:

- level index,
- loop region,
- basic block.

This ensures we never co-schedule tasks from different dynamic control contexts.

### 5.5 Safety gates before transforming a group

A call group is transformed only if all checks pass:

1. Group has at least 2 calls (otherwise no benefit).
2. Calls form a contiguous cluster in their block (excluding debug intrinsics).
3. Wrapper exists for each task.
4. Call result is not used (void or dead call only).
5. Argument shape matches captured `TaskArgumentInfo`.
6. Argument definitions dominate anchor callsite (`DominatorTree` check).
7. Type-correct argument stores can be generated.

If any check fails, group is left intact (original sequential behavior remains).

### 5.6 In-place pthread scheduling at anchor

For safe groups:

- insert IR before earliest call in group,
- allocate thread handle array,
- allocate and populate per-task argument structs from live call operands,
- spawn wrappers via `pthread_create`,
- barrier via `pthread_join` for the whole level group,
- erase original direct call instructions in reverse order.

This keeps execution inside original CFG and thus preserves loop and branch semantics.

## 6) Script/Build Fix Applied

`p1GraphEasy/03_run.sh` was also patched because it linked conflicting runtime objects:

- old command linked both `parallel_runtime.o` and `runtime.o`, causing duplicate `parallel_for_runtime`,
- and missed `timer_runtime` definition in that link path.

Now it:

- runs `./GraphProgram test.graph`,
- links `program.o roaring_bitmap.o runtime11.o` with `-fopenmp -lpthread`,
- executes resulting `final_program`.

## 7) Validation Performed

### Case A: User BFS regression

Input: chain graph, frontier `{2}`.

Observed output after fix:

- `dist[0] = 2` (correct),
- no all-zero behavior.

### Case B: Multi-iteration BFS sanity

Input: same chain graph, frontier `{0}`.

Observed output after fix:

- distances `0 1 2 3 4 5` (correct iterative traversal).

## 8) Why This Fix Addresses the Root Cause

The failure mode came from executing a static, flattened schedule outside real control flow. The fix eliminates that model and executes parallel work only at original CFG points where dynamic semantics are naturally enforced by LLVM IR control flow.

In short:

- before: schedule drove control flow,
- after: control flow drives schedule.

That inversion is the key reason loop-dependent algorithms (BFS/DFS-like traversals) remain correct.

