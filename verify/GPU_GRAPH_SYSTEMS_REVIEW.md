# How other graph systems use the GPU — and what that says about p1

Reviewed 2026-10-04.  Focused on the mechanisms (not the sales pitch) and on
what we can copy into p1's effect-system realizations.

## Gunrock (GPU, frontier-centric) — arXiv:1501.05387

**Abstraction.** First-class *frontier* (a set of vertices **or edges**), with
three bulk-synchronous operators: **advance** (generate the next frontier from
the current one), **filter** (compact or split it; a parallel scan), **compute**
(regular per-element work).  Primitives are loops of these steps until the
frontier is empty.  Their point: BSP steps need no fine-grained sync, which is
what makes them GPU-friendly.

**The core problem is load balancing**, and their answer is a *hybrid* of two
strategies with a runtime threshold (~4096 neighbours):
1. *Per-thread fine-grained* — one thread per frontier vertex's neighbour list,
   offsets staged in shared memory, and "vertex-cut" to split one huge list
   across threads.
2. *Per-warp / per-CTA coarse-grained* — neighbour lists bucketed by size
   (bigger than a CTA → one block per list; larger than a warp → one warp;
   smaller → one thread), so a hub is processed cooperatively instead of
   serialising its thread.
3. *Load-balanced partitioning* (from Davidson's SSSP) — equal-length **edge
   chunks** per block with a binary search to find the owning vertex.

**Optimizations that matter to us.**
* **Idempotent advance vs atomic advance** — BFS's fastest variant uses the
  idempotent one, i.e. it *avoids atomics*; the atomic variant exists for
  non-idempotent operators that need dedup.
* **Push vs pull** — pull (start from unvisited vertices, keep those with a
  neighbour in the frontier) wins when the frontier is large; measured 1.52×
  (scale-free) and 1.28× (small-degree high-diameter) on BFS.
* **Kernel fusion** — compute functors are fused into advance/filter at compile
  time; they name fusion as the *largest* remaining gap vs hardwired kernels.
* **Layout** — CSR by default, edge-list for edge frontiers, SoA for coalescing,
  scan to turn irregular work into uniform work.

**Scale and numbers.**  Datasets 1M–17M vertices, 5M–302M edges (K40c, 2015):
BFS 45–283 ms (4.7–8.3 GTEPS on the big scale-free graphs), comparable to
hardwired GPU BFS, ≥10× over BGL/PowerGraph, and **roughly comparable to the
CPU framework Ligra** (Ligra wins on the 212M-edge social graph; Gunrock wins on
the large-diameter roadnet/rgg).  CC is 5× slower than hardwired due to
redundant data movement and launches; PageRank is comparable.

## GraphIt (DSL, CPU+GPU) — arXiv:1805.00923

Separates **what** (algorithm) from **how** (schedule); the compiler models
optimizations in a *graph iteration space*.  The schedule language composes:
edge-aware vs vertex-aware traversal, **direction switching** (dense↔sparse
frontier = the pull/push switch), data layout (SoA/AoS, dense/sparse frontier
representations), and program-structure transforms, plus an autotuner that
searches schedules.  This is the closest published analogue of what our effect
system + realization selection does — notably, *both* of GraphIt's headline
schedule levers (direction, edge-vs-vertex traversal) are levers we lack on the
GPU side.

## Graptor (CPU vectorized) — ICS 2020, 10.1145/3392717.3392753

Turns out to be **the origin of our CPU engine**: its three components are
**CleanCut** — "a graph partitioning approach that rules out inter-thread race
conditions" (the name our `autograph_build_clean_cut` comes from), **VectorFast**
— a compact representation that supports fast-forwarding through the edge
stream, and **Graptor** — a DSL/compiler for auto-vectorizing graph codes.  It is
a CPU/Xeon-Phi vectorization framework, with both pull and push styles.  So: our
*CPU* side is Graptor's model, and our *GPU* work is re-deriving Gunrock's
lessons on top of it.

## What this says about p1 (the honest gap list)

| Gunrock / GraphIt lever | p1 GPU today | gap |
|---|---|---|
| size-classed load balancing (hub → block/warp) | one thread per row; a hub row serialises its warp | **missing** |
| direction optimization (pull when frontier is large) | push only | **missing** (our destination partitions already hold the incoming arcs, so pull is available) |
| idempotent advance without atomics | ✅ done for the activation claim (byte marks, host compaction) | — |
| frontier state device-resident, no per-round relaunch/copies | one launch per round, state copied in/out | **missing** (the measured 2–2.5× loss below the floor is mostly this) |
| kernel fusion (compute fused into traversal) | pair body fused into the kernel ✅ (but rounds are not fused) | partial |
| scan-based compaction | host-side compaction of claim marks ✅ (on the host, not the device) | partial |
| layout choice for the machine | CSR/PCSR/BCSR/SET layouts exist, but the *GPU* kernels read one structure | partial |

Nearest-term plan (in order): (1) ~~pull/per-arc activation kernel~~ —
**tried and removed**: the per-arc gate (membership of the arc's source) should
read the same pair set as the row kernel but appended 5 instead of 19 frontier
vertices in round 1 and ended at `reached 8` instead of `reached 20000` on the
20k fixture, i.e. the arc->row source mapping does not reproduce the row
kernel's gate.  A wrong kernel must not be reachable even behind an env, so it
is gone; the direction remains right (Gunrock's pull wins when the frontier is
large) and the next attempt must verify the arc->source array against the row
kernel's gate directly (a unit check on the two pair sets).  (2) size-classed
rows (hub splitting); (3) device-resident round loop with a device worklist
(atomic ticket — the one place atomics are the right tool), which removes the
per-round copies; (4) device-side compaction instead of host; (5) re-measure
above the floor — blocked twice: with our 16M-edge graph the compiler emits the
kernel but the program never registers a step (`[gpu] step 1 kept on the CPU:
no device kernel registered for this step id`), and that graph lives on the
CPU box, so the above-floor arms have to run there first.

## Sources

* [Gunrock: A High-Performance Graph Processing Library on the GPU](https://arxiv.org/pdf/1501.05387v4)
* [GraphIt: A High-Performance Graph DSL](https://arxiv.org/html/1805.00923v2)
* [Graptor: efficient pull and push style vectorized graph processing](https://dl.acm.org/doi/10.1145/3392717.3392753)
* [Graptor repository](https://github.com/hvdieren/graptor)
* [Direction-optimizing breadth-first search (Beamer et al., SC'12)](https://dl.acm.org/doi/10.1109/SC.2012.10)
* [Scalable GPU graph traversal (Merrill et al., PPoPP'12)](https://dl.acm.org/doi/10.1145/2145816.2145832)
