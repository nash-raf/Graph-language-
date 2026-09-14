# Handoff prompt — GraphEasy PDG / parallelisation work

Copy everything below the line into a fresh Claude Code session.

---

You are picking up work on **GraphEasy**, a compiled graph DSL.

## Repo

- GitHub: `https://github.com/nash-raf/Graph-language-.git` (remote `origin`)
- Local checkout: `/home/user/D/Course/msk1/2nd/Graph-language-`
  (also reachable as `/home/user/Course/msk1/2nd/Graph-language-` — same dir via symlink)
- Branch: `ars1-latest`, based on `origin/ARS-1` at commit `e8b89f9`
  ("ARS interaction with effect system fixed")
- Main compiler dir: `p1GraphEasy-con-AutoTuner/`
- Build the compiler: `./build_lowmem.sh` → produces `GraphProgram`
- Compile+link+run a `.graph` program: `GRAPH_FILE=<file.graph> ./03_run.sh` → produces `final_program`
  - **`03_run.sh` reads stdin.** Always call it with `</dev/null` inside a `while read` loop.
  - Large graphs need `ulimit -s unlimited` or `final_program` segfaults.
- Regression suite: `verify/run.sh` (categories: `lang algo parallel autotuner motif`)
- LLVM toolchain: `/usr/local/llvm-20-polly-rtti/bin` (LLVM 20 with Polly, RTTI enabled)

## What the compiler does

`.graph` source → ANTLR parser → AST → `IRGenVisitor` → LLVM IR → pass pipeline:

1. `GraphFrontierLowering` (main.cpp ~529) — recognises frontier/edgemap patterns
   (`for each vertex u { for each neighbor v of u {...} }`) and rewrites them into
   `autograph_edgemap` engine calls (CAS_FIRST / MIN_COPY / MIN_WEIGHTED / PEEL_K),
   with a CleanCut effect algebra deciding ownership (`class=dest-owner`, `U+(next_rank,V)`).
   Disable with `GRAPH_FRONTIER_REWRITE_OFF=1`.
2. `AutoTunerPass` (~550) — layout selection (CSR/PCSR/BCSR/SET) + a cost model.
3. **`pdg.cpp`** (~584) — builds a Program Dependence Graph and classifies every loop
   `DOALL` / `DOACROSS` / `SEQUENTIAL`, writing `!my.loop.parallel` metadata.
4. **`parallel_loop_outline.cpp`** (~816) — reads that metadata, multiversions each
   parallel loop (serial original + parallel clone), outlines the clone, and emits a
   runtime dispatch guarded by `sgpl_should_parallelize_doall`.
5. Canonicalisation + `-O3` with Polly (~1054/1059).
6. `parallel_runtime.c` — the thread pool + cost model that decides serial vs parallel
   at run time and executes the outlined loop.

Useful debug env vars: `SGPL_LOOP_CLASSIFY_DEBUG=1` (PDG verdicts),
`SGPL_OUTLINER_DEBUG=1` (why a loop was or wasn't outlined),
`GRAPH_PARALLEL_DEBUG=1` (runtime serial/parallel decisions),
`DUMP_LLVM_BC_PDG` / `DUMP_LLVM_BC_POST` / `DUMP_LLVM_BC_AFTER_OUTLINE` (IR dumps).

## Uncommitted local changes (this is what to explain to me)

`git diff` against `e8b89f9` touches 6 files, ~554 insertions. Every behaviour change
is behind an env-var kill switch so it can be A/B tested. Please read the actual diff
(`git diff`) and explain each of these to me in plain terms:

**1. `pdg.cpp` — loops under a frontier driver were all forced SEQUENTIAL**
A loop tagged `sgpl.frontier.nested.sequential` was unconditionally SEQUENTIAL. That is
right for the inner neighbour loop, but it also caught *sibling* vertex loops that merely
share the enclosing round loop (e.g. PageRank's `for each vertex v { next_rank[v] = beta; }`),
which DependenceInfo positively proves carry no dependence. Now a proof of no carried
dependence overrides the marker — **but only for loops with no subloops**. That leaf-only
restriction is load-bearing: a vertex loop that *contains* a neighbour loop also gets
`proofNoCarried=1`, yet is NOT safe, because the nested traversal keeps its state
(neighbour variable, iterator, accumulator) in DSL globals that all threads would share.
Kill switch: `SGPL_NO_PDG_PROOF_WINS=1`.

**2. `parallel_loop_outline.cpp` — only one loop per function was ever parallelised**
`outlineLoop()` found the first candidate and stopped, so a function with three
independent parallel loops got exactly one, chosen by block order. Now it iterates
(cap 64). Kill switch: `SGPL_OUTLINER_SINGLE_LOOP=1`.

**3. `parallel_loop_outline.cpp` — profitability veto ignored trip count**
`countEffectiveLoopBodyInstructions() < 8` rejected any small loop. That measures work
*per iteration* and ignores how many iterations there are, so it rejected graph loops over
|V| or |E|. Loops carrying `autotuner.traverse` metadata (`vertex`/`neighbor`/`edge`, i.e.
non-affine graph domains with dynamic bounds) now use a floor of 3 and let the runtime cost
model decide. Kill switch: `SGPL_NO_GRAPH_TRIVIAL_RELAX=1`.

**4. `parallel_loop_outline.cpp` + `parallel_runtime.{c,h}` — range entry point (the big perf win)**
The runtime drove the outlined body **one iteration at a time through a function pointer**
(`a->body(i, a->env)`), which blocked inlining and vectorisation: the serial loop in `main`
got 188 vector ops from `-O3`, the parallel body got zero. The compiler now also emits a
*range* entry point `(lo, count, step, env)` containing the loop, registered via
`sgpl_set_pending_range_body()`, so `-O3` inlines and vectorises inside it and the runtime
calls it once per chunk. Kill switch: `SGPL_NO_RANGE_BODY=1`.

**5. `parallel_runtime.c` — cyclic → block-cyclic iteration distribution**
Threads were assigned `i = tid, tid+P, tid+2P, ...`. With 8 doubles per cache line and P=4,
every line is written by every thread — false sharing on every line, and no contiguous run
for the prefetcher. Now block-cyclic with a chunk (default 256), keeping load balance for
skewed graph work. **DOACROSS is excluded** — its wait/post protocol depends on the cyclic
order. Kill switch: `SGPL_LOOP_CHUNK=0`.

**6. `parallel_runtime.c` — the DOALL cost model never calibrated**
`sgpl_should_parallelize_doall` had a `warmup-large-trip` shortcut: any loop over 65536
iterations went parallel immediately. But `c_ns_per_iter_ewma` is fed *only* by
`sgpl_record_doall_serial_sample()` on the serial path, so such a loop went parallel forever
and the model never collected a single sample. Now invocation 1 still goes parallel (a
loop called once must not pay for a measurement it will never use) and the next 4 fall
through to serial to fill the batch. Kill switch: `SGPL_NO_WARMUP_CALIBRATION=1`.

**7. `main.cpp` — canonicalisation ran only on the GPU path**
`canonicalizeLoopsForAnalysis(M)` now runs before the PDG on every path, and additionally
runs `GlobalOptPass` before mem2reg. DSL variables are emitted as `internal global`, and
mem2reg only promotes allocas, so without this the PDG sees `%id.ptr = load ptr, ptr @id`
reloaded every iteration and DependenceInfo must report an unknown direction.
Kill switch: `SGPL_NO_PDG_CANON=1`.

**8. `build_lowmem.sh`** — added Polly's include dir and `graph_frontier_lowering.cpp`
to `SOURCES` (the build was missing both).

## Measured results (4-core machine, please re-verify rather than trust)

Vertex-loop microbenchmark, 1e6 vertices, 20 rounds, k = flops per iteration.
Serial (`SGPL_NUM_THREADS=1`) vs 4 threads, best of 3, checksums identical throughout:

| k | serial | 4thr before | 4thr after | before | after |
|---|--------|-------------|------------|--------|-------|
| 0 | 0.08s | 0.19s | 0.07s | 0.42x | **1.14x** |
| 2 | 0.08s | 0.22s | 0.07s | 0.36x | **1.14x** |
| 8 | 0.17s | 0.27s | 0.11s | 0.63x | **1.55x** |
| 24 | 0.68s | 0.98s | 0.28s | 0.69x | **2.43x** |

So before this work, parallel graph loops were *slower* than serial at every work level.

PageRank on a 1M-vertex graph is a wash (3.57s vs 3.32-3.90s, same result 0.884892),
because its runtime is dominated by the edge-scatter loop that the frontier/edgemap
engine owns — the PDG's vertex loops are ~1% of it.

`verify/run.sh`: **40 pass / 5 fail**. All 5 failures are pre-existing language bugs,
unrelated to this work (see "Known broken" below).

## A race I found and fixed — worth understanding

Test program: `verify/cases/parallel/nested_gather.graph` (new). It is a gather —
`for each vertex u { real s = 0; for each neighbor v of u { s += val[v]*0.5 } acc[u] = s }`
— run with `GRAPH_FRONTIER_REWRITE_OFF=1` so the PDG, not the edgemap engine, handles it.

With change (1) unrestricted, the outer vertex loop was classified DOALL and three
consecutive 4-thread runs returned **1090473 / 5651057 / 161274** against a correct
**160136.013072** (verified independently in numpy). Adding the leaf-only restriction
fixed it. This case is now a regression test in `verify/run.sh` category `parallel`.

## Known broken — pre-existing, NOT caused by this work (I verified with all kill switches on)

1. `verify` failures: `lang/real_array_const` (compiler crash on `real x[8]` with a constant
   size — uncaught `std::runtime_error`; `real x[n]` works), `lang/array2d_real` (2-D real
   arrays unsupported), `lang/keyword_INF` (`INF` is a reserved keyword, so `int INF = ...`
   is a parse error), `lang/real_precision` (`print` of a real uses `%f`, so 1e-7 prints as
   0.000000), `autotuner/cc` (predicted_ms=0, autotuner sees no region).
2. **A `while` loop nested inside a `for each vertex` body segfaults the generated program.**
   Reproduces with every kill switch set. Minimal repro: a `for each vertex v` whose body
   contains `int k = 0; while (k < 12) { ...; k = k + 1; }`.
3. **The frontier lowering returns 0 for the nested gather above.** With the default
   pipeline `nested_gather.graph` prints `0.000000` instead of `160136.013072`. The
   lowering claims the pattern (`class=source-owner W(acc,U) temporal=Independent`) and
   produces nothing. This one is serious and I have not investigated it.
4. Dead code: `pdg.cpp` reads metadata `sgpl.frontier.first_wins.candidate`, which nothing
   emits (emitters use `sgpl.frontier.first_wins.doall`), so `IsVerifiedFrontier` is
   permanently false.
5. Raw `while` loops over graph data still classify SEQUENTIAL with `unknownAttr=1` —
   the DSL-globals aliasing problem that change (7) only partly addresses.

## Also explain to me: motif search and the bi-fan calculation

**What a bi-fan is.** In a directed graph, two source nodes `s1,s2` and two targets
`t1,t2` with all four edges `s1→t1, s1→t2, s2→t1, s2→t2`. One of the standard network
motifs from Milo et al. 2002.

**The neural-network task.** My professor gave me trained MLP weight files. For a network
of L layers × 512 units, treat each *pair of consecutive layers* as a bipartite graph
(previous layer = sources, next layer = targets). Per epoch, per layer, keep only the
**strongest X% of weights by magnitude** (X = 20,30,40,50,60), and count bi-fans in the
resulting graph. **The final hidden→output layer is excluded.** Without thresholding a
fully-connected layer makes every bi-fan exist trivially, which is why the top-X% cut
is the whole point.

**The closed form.** Bi-fans do not need search. With `A` the thresholded bipartite
adjacency matrix (sources × targets), let `M = A · Aᵀ`. Then `M[i][j]` is the number of
targets that sources `i` and `j` share, and the bi-fan count is `Σ_{i<j} C(M[i][j], 2)`.
The automorphism group of the bi-fan has order 4, which is exactly what the `i<j` and the
`C(·,2)` account for.

**A performance trap worth knowing.** numpy has **no BLAS path for integer matmul**. Using
an `int32` adjacency matrix made `A @ A.T` take 0.0745s versus 0.0010s for `float32` — a
**75× penalty**. The fix is `float32` + `np.rint`; I verified the results are exact.

**Where the artefacts are.** `/home/user/D/uniques/` holds 60 result CSVs named
`bifan_counts_{,cifar10_,qnli_,qnliproper_}{4,8,16}_top{20..60}.csv` (4 datasets —
MNIST, CIFAR-10, QNLI sampled, QNLI proper — × 3 depths × 5 thresholds), plus
`bifan_census.py`. Columns are `epoch, layer_0..layer_N, total`. Validation: 700
layer-graphs per dataset with 0 mismatches, and 9/9 against the professor's own `.mat`
file.

**GraphEasy side.** `MotifPattern.{h,cpp}` + `MotifIRBuilder.{h,cpp}` are a compile-time
motif engine: the pattern is known at compile time, so it derives the automorphism group,
turns canonicality into loop lower bounds, and emits a specialised CSR backtracking nest
as IR instead of interpreting a `required[][]` matrix at run time. The bi-fan closed form
was also implemented in GraphEasy and matched numpy byte-for-byte across 1,616 values.
DSL sources are under `algo_validation/bifan/` and `verify/cases/motif/bifan.graph`.
There is a longer design document at `/home/user/.claude/plans/so-what-i-want-modular-harp.md`.

## What I want from you

Walk me through all of the above in your own words, verifying against the actual code
rather than taking this summary on trust — I want to understand what changed, why each
change was necessary, and what is still broken. Start by running `git diff` and
`verify/run.sh`, and tell me anything in this description that does not match what you find.
