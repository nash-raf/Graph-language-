# Dense semiring-closure motif in p1 Autotuner

Ported from p2-AT. When the user writes a Floyd–Warshall / Kleene-style
triple nest over a square `int D[n][n]`, IRGen rewrites it to
`autograph_closure(D, n, semiring)` instead of emitting the O(n³) loops.

## Recognized bodies

| combine (OP) | select (REL) | Semiring id | Classic use |
|--------------|--------------|-------------|-------------|
| `+` | `<` | `min_plus` (0) | APSP |
| `+` | `>` | `max_plus` (1) | longest / critical path |
| `*` | `>` | `max_times` (2) | boolean reachability |
| `*` | `<` | `min_times` (3) | min-product |

Shape required: outer `while (k < n)` containing `while (i < n)` containing
`while (j < n)` with body either

```
int cand = D[i][k] OP D[k][j];
if (cand REL D[i][j]) { D[i][j] = cand; }
```

or the inlined form without a temporary.

## Runtime engine (`semiring_runtime.c`)

Always rewrites (no per-semiring decline). Wins by algorithm, not by skipping:

1. **MaxTimes on a {0,1} matrix** → bit-parallel Warshall (word OR). This is
   boolean *transitive closure*, not int32 FW — typically tens to 100× vs a
   Polly nest on the same `D`.
2. **Otherwise** → tiled FW (Venkataraman). One persistent OpenMP team for
   phase-3 row updates when `n ≥ 512` (no per-rectangle fork/join).

Ablation: `CLOSURE_ONLY=1 python3 bench_motif_speedup.py`

## Files

- Detector / emit: `IRGenVisitor.cpp` (`detectSemiringClosureNest`, `sgpl.closure` MD)
- Runtime: `semiring_runtime.c` → `autograph_closure`
- Demos: `semiring_apsp_demo.graph`, `semiring_closure_demo.graph`

## Build / link

```bash
./build_lowmem.sh          # builds GraphProgram + semiring_runtime.o
# 03_run.sh links semiring_runtime.o automatically when present
```

## Smoke check

```bash
GRAPH_DISABLE_POLLY=1 DUMP_LLVM_BC_AFTER_OUTLINE=/tmp/after.bc \
  ./GraphProgram semiring_apsp_demo.graph
llvm-dis /tmp/after.bc -o - | grep autograph_closure
# expect: call i32 @autograph_closure(..., i32 0), !sgpl.closure !{!"D", !"min_plus"}
```
