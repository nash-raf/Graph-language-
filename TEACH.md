# TEACH.md — How to teach motif search in GraphEasy (p1)

**Audience of this file: an AI assistant (GPT) that will teach a human.**
The human knows basic graph theory (vertices, directed edges, in/out-neighbours)
and can run shell commands. Every number, output and command in this file was
executed and verified on the compiler `p1GraphEasy-con-AutoTuner` (GraphEasy).
**Do not invent syntax, outputs or numbers beyond what is in this file.** Where
something does not work or is a trap, it is marked as such — say so honestly.

---

## 0. How to use this file as a teacher

1. Never start with the implementation. Start with a **tiny graph the student can
   count by hand**, ask them to predict the answer, then run it and compare.
2. Follow the lessons in order (L1–L7). Each lesson has: goal → graph → pattern →
   *ask the student* → by-hand answer → command → observed output → takeaway.
3. The surprises are the lessons: induced semantics (L2), symmetry breaking (L4,
   L5), the kind function (L6), and the traps (L7).
4. After each lesson, give one exercise from §8 and ask for a prediction *before*
   running anything.
5. Keep the two reference documents at hand for depth:
   `MOTIF_SEARCH_EXPLAINED.tex` (pipeline) and `MOTIF_SEARCH_THEORY.tex`
   (algorithms/proofs). This file is the *lesson plan*; those are the *manuals*.

---

## 1. The 60-second mental model (what to say first)

> To search for a motif, the compiler writes a small nested loop over the
> pattern's variables. Pick a vertex for `a`. Check nothing yet. Pick a vertex for
> `b`; check the pair `(a,b)` against the pattern — both directions, so "no edge
> here" is also a check. Pick a vertex for `c`; check `(a,c)` and `(b,c)`.
> When every variable has a vertex, you have one match. If a check fails, skip
> that vertex and try the next one.

That **is** the algorithm (depth-first backtracking). Two refinements to mention
once the base picture lands:

* the candidate for a variable is often taken from the **neighbour row of an
  already-bound variable** (a spanning-tree-of-the-pattern idea) instead of
  scanning all vertices — but that is an optimisation, not the definition;
* the same vertex set can be reached by permuting variables that the pattern
  cannot tell apart; the implementation reports **one representative per such
  permutation** (symmetry breaking). This is why a 4-vertex graph that contains a
  bi-fan reports **1**, not 4.

And one rule that governs everything:

> **Matching is induced.** If you do not write a pair in the pattern, that pair
> must be **absent** between the matched vertices. Extra edges break the match.

---

## 2. Fact sheet A — the language surface that actually works (p1)

```text
graph G { directed: true; edges: file "edges.txt"; };      # file-backed, directed

motifs M = [G where motif { a->b; a->c; b->c; }];          # matches: count + bindings  (§4b)
graphs L = [G where motif { a->b; a->c; b->c; }];          # matches: same bindings, iterated as fresh subgraphs (§4b)
print numMotifs(M);                                        # count
print numGraphs(L);
for each motif (a,b,c) in M { ... }                        # iterate bindings
for each graph H in L { ... }                              # iterate subgraphs
draw motif of G to "prefix" { a->b; a->c; b->c; }          # render matches
```

Rules and limits (enforced by the compiler):

| Rule | Detail |
|---|---|
| Directed only | the graph must be declared `directed: true` |
| Edge source | **use `edges: file "..."` for motif work** — inline `edges: 0->1, ...` graphs are stored undirected and induced motifs find 0 (see L7) |
| Variables | at least 2, **at most 6** |
| Self edges | rejected (`a->a`) |
| Duplicate edges | rejected (same ordered pair twice) |
| Edge signs | `a->b` = a **positive** edge must exist; `a-|b` = a **negative** edge must exist |

Syntax that does **not** work in p1 (verified):

* `graph F = [G where ...];` — parse error (`graph` starts a declaration).
  Write `F = [G where ...];` instead.
* `F = [G where motif { ... }];` — parses and runs, but the motif condition is
  **silently ignored** (result equals `F = [G];`). Use `motifs`/`graphs`.

---

## 3. Fact sheet B — the semantics (the "kind function")

For an ordered pair of vertices `(u,v)` the matcher computes one number:

| situation | kind | satisfies |
|---|---|---|
| no edge `u->v` | 0 | absence (a pair you did not write in the pattern) |
| unweighted edge | 1 | `u->v` |
| edge with weight > 0 | 1 | `u->v` |
| edge with weight < 0 | −1 | `u-|v` |
| edge with weight = 0 | 2 | **nothing** — it poisons both presence and absence |

And the matching rule: for every pair of variables, `kind` in **both directions**
must exactly equal the pattern's requirement. Absence is a requirement too.

Counting rule: matches are counted **modulo automorphisms of the pattern** — one
representative per symmetry orbit (see L4/L5).

Binding rule: `for each motif (...)` binds **by position**, and the pattern's
variable order is **first-appearance order** (scan the motif edges left to right,
source before target). For `{ a->x; a->y; b->x; b->y; }` the order is
`(a,x,y,b)`. Listing the names in another order silently permutes them (L7).

---

## 4. Fact sheet C — the algorithm (only after the lessons)

* Depth-first backtracking over variables; constraints are binary, so a partial
  assignment that violates one can never be extended (safe pruning).
* Each level may enumerate from a bound variable's CSR row (adjacency-driven) —
  "spanning tree of the pattern drives the enumeration; the other edges are
  checks". The driver is the most recently bound variable with a required edge.
* Injectivity is enforced pairwise; canonicality (symmetry breaking) is enforced
  by lower-bound compares computed at compile time from the pattern's
  automorphisms: for each non-identity automorphism with first moved variable
  `p`, require `A[p] < A[π(p)]`; these are compared while descending, so
  non-representative subtrees are pruned.
* Complexity: worst case exponential in the number of variables (subgraph
  isomorphism is NP-hard), which is why the language caps the pattern at 6
  variables.
* Two backends: runtime interpreter (default; produces bindings) and an emitted
  IR loop nest (`GRAPHEASY_MOTIF_BACKEND=ir`; count-only, with
  `GRAPHEASY_MOTIF_ADJDRIVE=1` for row-driven enumeration). Measured on a
  2000-vertex/6000-arc graph, FFL count 26 in all three: 0.279 s (runtime),
  0.219 s (IR), 0.016 s (IR+adjacency) per run.

Code map for the teacher (p1): grammar `Base.g4:135-171`; AST
`ASTBuilder.cpp:230-243`; semantics `SemanticAnalyzer.cpp:980-1010`;
pattern analysis `MotifPattern.cpp`; emitted nest `MotifIRBuilder.cpp`; runtime
matcher `runtime.c:1013-1357`.

---

## 4b. Storage: what is materialised, when, and where

This is what the earlier version of this file left out — the audit is fair.
Here is the complete picture.

### The one struct both backends agree on

```c
/* runtime, heap-allocated (runtime.c:1135) */
typedef struct { int32_t count; int32_t var_count;
                 int32_t *bindings; int32_t capacity; } MotifMatchesRuntime;

/* compiler-side type, deliberately the SAME 24-byte layout, align 8
   (IRGenVisitor.h:40) so a value from either backend is interchangeable */
struct.MotifMatches { i32 count; i32 var_count; i32* bindings; i32 capacity; }
```

| field | meaning | who reads it |
|---|---|---|
| `count` | number of matches found | `numMotifs` / `numGraphs`, loop bound of `for each` |
| `var_count` | number of pattern variables `k` = row width | `for each` (row stride) |
| `bindings` | pointer to a flat, row-major array of `int32` | `for each` (the actual bindings) |
| `capacity` | allocated rows (≥ count) | growth only |

### The bindings array

* **layout**: match `i` occupies `bindings[i*k] … bindings[i*k + k - 1]`, one
  `int32` per variable; `k` = `var_count`.
* **growth**: capacity starts at 16 and **doubles**; each append is a `realloc`
  of `capacity * k` int32s plus a `memcpy` of the row
  (`motif_matches_append_runtime`, `runtime.c:1192`). Appends happen at the leaf
  of the backtracking search, i.e. **as matches are discovered**.
* **lifetime**: the array is heap-allocated and **never freed** — it lives for
  the rest of the program. Memory ≈ `4 · k · count` bytes, with up to 2× slack
  from the last doubling (e.g. 1 M matches of a 3-variable pattern ≈ 12 MB of
  rows, ≤ 24 MB with slack).
* **eager, not lazy**: the full list is computed and stored when the declaration
  executes — even if the program only ever calls `numMotifs`. There is no
  streaming mode except the drawing path (below).

### What each construct materialises

| construct | backend | what is stored | when |
|---|---|---|---|
| `motifs M = [G where motif {...}];` | runtime (default) | heap `MotifMatchesRuntime` + **all bindings** | eagerly, at the declaration |
| `motifs M = [...];` | IR (`GRAPHEASY_MOTIF_BACKEND=ir`) | a **stack** `struct.MotifMatches` with `count` only; `bindings = NULL`, `capacity = 0` | count computed at the declaration by the emitted nest |
| `numMotifs(M)` | either | nothing new — reads `count` | — (the runtime path has already materialised the list) |
| `for each motif (a,b,c) in M` | runtime only | nothing new — reads `bindings[i*k + j]` per iteration | iterates the stored array |
| `graphs L = [G where motif {...}];` | runtime only | same heap struct (bindings of every match) | eagerly, at the declaration |
| `for each graph H in L` | runtime only | `H` is a **fresh heap induced-subgraph object per iteration**, built by `graph_from_match_runtime` (deep CSR copy from the match's binding row); not precomputed, not freed | lazily, one per iteration |
| `draw motif(s) of G to ... {...}` | runtime only | **nothing**: a private streaming backtracker (`draw_motif_backtrack_runtime`, `runtime.c:1522`) renders each match as it is found | streaming |
| `F = [G where motif {...}];` | — | nothing — the condition is **dropped** (see the traps) | — |

### Count-only vs listing, in one sentence each

* **Listing** = any construct that reads the `bindings` array:
  `for each motif`, `for each graph` (plus the drawing path, which streams).
* **Count-only** = `numMotifs` / `numGraphs` (they read `count`), and the
  **IR backend**, which by construction never stores individual matches.
* **Mixing them is the trap**: an IR-built collection has `bindings = NULL`
  while `count > 0`; running `for each motif` over it would read through a null
  pointer. Use the IR backend only where a count is enough, or stay on the
  default runtime backend for listings.

### `motifs F = [...]` vs `graphs F = [...]` — the clear difference

* `motifs` gives you **the variable bindings**: iterate with
  `for each motif (a,b,c) in F` (names positional, first-appearance order).
  Nothing but the flat binding array is materialised.
* `graphs` gives you **one induced subgraph per match**: iterate with
  `for each graph H in F` and treat `H` as an ordinary graph
  (`numVertices(H)`, `numEdges(H)`, edge iteration, nested loops, …). Each `H` is
  *rebuilt on the fly* from the binding row, so a loop over 1 M matches makes
  1 M graph objects (and never frees them).
* Both declarations run the **same matcher** and store the **same** binding
  struct; the only difference is the loop variable type and the per-iteration
  materialisation in `for each graph`.
* `for each motif (...)` binds **by position**; `for each graph H` takes a single
  variable name.



## 4c. Seeing the plan: the `MotifPattern` dumper

The compile-time plan (level sources, drivers, canonical bounds, `|Aut|`) is
computed by `MotifPattern`; its `describe()` method exists but no CLI switch
prints it. Build a 20-line dumper and you can show a student exactly what the
search will do:

```cpp
/* /tmp/plan.cpp — usage: ./plan a b + b c + a c + */
#include "MotifPattern.h"
#include <cstdio>
#include <cstring>
int main(int argc, char **argv) {
    std::vector<MotifEdgeSpec> edges; std::vector<std::string> vars;
    for (int i = 1; i + 2 < argc; i += 3) {
        edges.push_back({argv[i], argv[i+1],
            std::strcmp(argv[i+2], "-") == 0 ? MotifEdgeSign::Negative
                                             : MotifEdgeSign::Positive});
        bool s = false, t = false;
        for (auto &v : vars) { if (v == argv[i]) s = true; if (v == argv[i+1]) t = true; }
        if (!s) vars.push_back(argv[i]);
        if (!t) vars.push_back(argv[i+1]);
    }
    std::printf("%s\n", MotifPattern::build(edges, vars).describe().c_str());
}
```

```bash
cd p1GraphEasy-con-AutoTuner
INC=$(/usr/local/llvm-20-polly-rtti/bin/llvm-config --includedir)
LDF=$(/usr/local/llvm-20-polly-rtti/bin/llvm-config --ldflags --libs support --system-libs)
g++ -std=c++17 -I. -I"$INC" -o /tmp/plan /tmp/plan.cpp MotifPattern.cpp $LDF
```

Observed outputs (variables are listed in first-appearance order):

```text
$ /tmp/plan a b + b c + a c +            # the FFL  a->b; b->c; a->c;
motif k=3 vars=[a,b,c]   |Aut| = 1
  level 0: FullScan
  level 1: OutRow(A[0])          # b comes from a's OUT-row
  level 2: OutRow(A[1])          # c comes from b's OUT-row (not an in-row!)

$ /tmp/plan a b + c b +                  # a->b; c->b;  (edge INTO an earlier var)
motif k=3 vars=[a,b,c]   |Aut| = 2
  level 0: FullScan
  level 1: OutRow(A[0])
  level 2: InRow(A[1])  bounds: cand>A[0]   # c comes from b's IN-row

$ /tmp/plan a b + c d +                  # disconnected: a->b; c->d;
motif k=4 vars=[a,b,c,d] |Aut| = 2
  level 0: FullScan
  level 1: OutRow(A[0])
  level 2: FullScan     bounds: cand>A[0]   # new component: no row to walk
  level 3: OutRow(A[2])

$ /tmp/plan a x + a y + b x + b y +      # bi-fan
motif k=4 vars=[a,x,y,b] |Aut| = 4
  level 0: FullScan
  level 1: OutRow(A[0])
  level 2: OutRow(A[0])  bounds: cand>A[1]  # x < y  (swap targets)
  level 3: InRow(A[2])   bounds: cand>A[0]  # b from y's IN-row; a < b (swap sources)
```

**The rules in one line each.** Level 0 is always a full scan. For every later
level: if some already-bound variable `prev` has a required edge
`prev -> level` (forward), enumerate that variable's **out-row**
(`required[prev][j] != Absent`, most recent `prev` wins); otherwise if some
`prev` has a required edge `level -> prev` (backward), enumerate that variable's
**in-row** (`InRow`); otherwise **full scan**. Note the direction of the last
level of the bi-fan: it walks **incoming** edges — the case you asked about — and
that is exactly the case that is *not yet enabled* in the emitted nest.

**Caveat (verified).** With `GRAPHEASY_MOTIF_ADJDRIVE=1`, an `InRow` level aborts
the compiler with a loud assertion:

```text
GraphProgram: MotifIRBuilder.cpp:168: ... Assertion `plan.source ==
LevelSource::OutRow && "InRow enumeration requires the private transpose
(stage 5)"' failed.        (exit 134)
```

So today the row-driven backend runs plans whose levels are `OutRow`/`FullScan`;
patterns with an `InRow` level (bi-fan, `c->b` shapes) must run with
`GRAPHEASY_MOTIF_ADJDRIVE` unset, or the runtime backend. Also note that a
**canonicality bound is attached to whichever level is bound later** — e.g. the
bi-fan's `cand>A[1]` on level 2 implements `x<y`.

---

## 5. The lesson script

### L0 — Warm-up: the graph we will use

```text
T:  0->1, 1->2, 0->2, 1->3, 0->3      (5 arcs, 4 vertices)
```

Draw it as: `0` points to `1` and `2`; `1` points to `2` and `3`; `2` and `3`
have no outgoing arcs. Save the five arcs one per line in `tiny.txt`.

### L1 — The single-edge pattern: what does `a->b;` count?

*Ask:* "How many matches?"
*By hand:* one match per arc → 5.
*Command:* write

```text
graph T { directed: true; edges: file "tiny.txt"; };
motifs E1 = [T where motif { a->b; }];
print numMotifs(E1);
print numEdges(T);
```

*Observed:* `5` then `2`.
*Takeaway:* the pattern counts **arcs** (directed). `numEdges` prints **2**
because it reports *undirected* edges as `m/2` in this compiler — do not use it
to cross-check a directed count.

### L2 — Induced semantics: `a->b; b->c;` on T is **0**, not 2

*Ask:* "How many length-2 directed paths does T have?" (student says 2:
`0->1->2` and `0->1->3`).
*Then:* "Now add the induced rule: the pair `(a,c)` must be **absent**."
Both candidates have `0->2` / `0->3` present → both rejected.
*Observed:* `0` on T; on a graph that is exactly the path `0->1, 1->2` the same
pattern gives `1`.
*Takeaway:* write the pattern you *mean*. To allow chords, you would have to list
them; to forbid them, you just don't list them.

### L3 — The feed-forward loop: `a->b; a->c; b->c;` on T is 2

*Ask them to enumerate by hand with a table (this is the core exercise):

| assignment | `a->b` | `a->c` | `b->c` | verdict |
|---|---|---|---|---|
| a=0,b=1,c=2 | 0→1 ✓ | 0→2 ✓ | 1→2 ✓ | match |
| a=0,b=1,c=3 | 0→1 ✓ | 0→3 ✓ | 1→3 ✓ | match |
| a=0,b=2,c=3 | 0→2 ✓ | 0→3 ✓ | 2→3 ✗ | no |
| a=1,b=2,c=3 | 1→2 ✓ | 1→3 ✓ | 2→3 ✗ | no |

*Command:* `motifs M = [T where motif { a->b; a->c; b->c; }]; print numMotifs(M);`
plus `for each motif (a,b,c) in M { print a; print b; print c; }`
*Observed:* `2`, bindings `0 1 2` and `0 1 3`.
*Takeaway:* the bindings are exactly the two hand-derived matches; this is the
"hello world" of the feature.

### L4 — The k-clique thread (complete digraph patterns)

A clique in this language is written on a **directed** graph as the complete
digraph on the pattern's variables: all `k(k−1)` ordered pairs.

* 3-clique pattern on `K3` (all 6 ordered pairs among `0,1,2`):
  `motif { a->b; a->c; b->a; b->c; c->a; c->b; }` → **1**
* the same 3-arc pattern from L3 on that symmetric `K3` → **0**
  (the reverse pairs exist, so induced matching rejects — a great link to L7)
* 4-clique pattern (all 12 ordered pairs) on `K4` (4 vertices, both directions)
  → **1**
* the 4-clique pattern on T → `0`

*Ask:* "Why is the count 1 and not 6 (or 24)?" Because the complete digraph has
`k!` symmetries: every permutation of the variables preserves the pattern, so all
`k!` assignments to the same vertex set are one orbit; the compiler emits the
ordering bounds `a<b<c<…` and keeps exactly one representative.
*Takeaway:* pattern symmetry is why "how many cliques" = "how many vertex sets".

### L5 — The bi-fan: symmetry without a full symmetric group

Graph: `K2,2` as `0,1 -> 2,3` (arcs `0->2, 0->3, 1->2, 1->3`).
Pattern: `a->x; a->y; b->x; b->y;` → **1** (four assignments, one orbit: the two
sources are interchangeable, the two targets are interchangeable — group of size
4). Ask the student to name the four assignments and to check the two emitted
bounds `a<b`, `x<y`.
*Binding spelling note:* first-appearance order here is `(a,x,y,b)` — see L7.

### L6 — Signs and zero weights

Weighted graph file (third column = weight) with `TRUE` in the declaration:
`graph S { directed: true; edges: file "w.txt"; TRUE };`

With arcs `0->1 (5)`, `1->2 (5)`, `0->2 (w)`:

| pattern | w = −3 | w = 0 |
|---|---|---|
| `a->b; b->c; a->c;` (all positive) | 0 | 0 |
| `a->b; b->c; a-|c;` (chord negative) | **1** | 0 |
| `a->b; b->c; c-|a;` (negative edge the other way) | 0 | 0 |
| `a->b; b->c;` (chord must be absent) | 0 (chord exists, wrong sign) | 0 (zero-weight edge still "exists") |

*Takeaways:* `a-|b` means "a **negative** edge must be present", not "no edge";
and a zero-weight edge poisons even the absence requirement.

### L7 — The traps (each verified)

1. **Inline edges are stored undirected.** `graph G { directed: true; nodes:
   0,1,2,3; edges: 0->1, ...; }` answers "neighbours of 1" with `{0,2,3}` —
   the reverse arc exists. Consequently the FFL pattern finds **0** matches,
   while the same topology in a file finds **2**. Always use `edges: file`.
2. **Bindings are positional.** Pattern `{ b->c; a->b; a->c; }` (internal order
   `b,c,a`): `for each motif (a,b,c)` prints `hasEdge(a,b)=1, hasEdge(a,c)=0` —
   the names are silently permuted; `(b,c,a)` prints `1 1`. Spell the `for each`
   list in first-appearance order.
3. **`where motif` in a comprehension is ignored.** `F = [G where motif {...}];`
   gives the same vertices as `F = [G];` (verified: `n=4` both), while
   `F = [G where degree > 100];` does filter (`n=0`).
4. **`graph F = [...]` is a parse error** in p1.
5. **The IR backend is count-only** and only for `motifs` declarations; mixing it
   with `for each motif` would read a null binding array.
6. **`numEdges` = m/2** (undirected convention) — see L1.

---

## 6. A suggested 45-minute session plan

| minutes | content |
|---|---|
| 0–5 | §1 mental model + vocabulary (arc, induced, orbit) |
| 5–15 | L1, L2 (predict, then run) |
| 15–25 | L3 (hand table, then run, read bindings) |
| 25–35 | L4 + L5 (clique thread; symmetry; bounds) |
| 35–42 | L6 (signs, zero-weight) |
| 42–45 | L7 traps, then one exercise from §8 as homework |

---

## 7. Misconceptions to pre-empt

| misconception | correction |
|---|---|
| "motifs are subgraph *patterns*; extra edges are fine" | no — matching is induced; unlisted pairs must be absent |
| "`a-|b` means there is no edge" | it means a **negative** edge exists; absence is expressed by omission |
| "it will just count 4 bi-fans in K2,2" | counts are modulo pattern symmetry: 1 |
| "the count is the number of injections" | it is the number of **orbits** of injections |
| "inline edges work the same as files" | inline graphs are stored undirected → motif counts become 0 |
| "`for each motif (a,b,c)` binds by name" | it binds by position; name order must match first-appearance order |
| "the pattern's edge order doesn't matter" | it changes the search plan (speed), not the count |
| "matches are found lazily — calling only `numMotifs` skips the search" | the runtime backend runs the full search and stores every match at the declaration; `numMotifs` reads `count`. Only the IR backend (count-only) and `draw` (streaming) avoid materialising a list |
| "any motif collection can be iterated" | only runtime-backend collections carry `bindings`; an IR-backend collection has `count > 0` and `bindings = NULL` |
| "`graphs L = [...]` precomputes the subgraphs" | it stores only the bindings; each `H` is rebuilt as a fresh induced subgraph during `for each graph H in L` |

---

## 8. Exercise bank (with verified answers)

1. On T (`0->1,1->2,0->2,1->3,0->3`), predict `numMotifs` for `a->b; a->c;` —
   **answer 2**. Reasoning: `a` must have two out-neighbours that are mutually
   non-adjacent (the induced rule forbids the pair `(b,c)` in *both* directions):
   only the pair `{2,3}` qualifies, and only from `a=0` and `a=1`. The two
   assignments `(0,2,3)` and `(0,3,2)` are one orbit of the pattern's `b`/`c`
   symmetry, so the reported bindings are `(0,2,3)` and `(1,2,3)` — count 2, not
   3 and not 4.
2. On T, predict for `a->b; b->c;` — **answer 0** (chords `a->c` exist).
3. On the path graph (`0->1,1->2`), predict for `a->b; b->c;` — **answer 1**.
4. On `K3` (complete digraph on 3 vertices), predict for the 6-pair pattern —
   **1**; and for `a->b; a->c; b->c;` — **0**.
5. On `K4` (complete digraph on 4 vertices), predict for the 12-pair pattern —
   **1**; how many variables does that pattern have? — **4** (limit is 6).
6. On `K2,2`, predict the bi-fan count and the number of raw assignments —
   **1 and 4**.
7. Signed file `0->1(5), 1->2(5), 0->2(-3)`: predict for `a->b; b->c; a-|c;` —
   **1**. Now change the chord's weight to 0 and predict — **0**.
8. Why does `numEdges(T)` print 2 while the single-edge motif prints 5? —
   `numEdges` is `m/2` (undirected convention); the motif counts directed arcs.
9. A student writes `motifs M = [G where motif { a->b; a->c; b->c; }];` on an
   inline-edge graph and gets 0 on a graph that clearly contains FFLs. Diagnose —
   inline graphs are stored undirected, so the reverse pairs are present and the
   induced checks fail; switch to `edges: file`.
10. A student writes `for each motif (a,b,x,y)` for the pattern
    `{ a->x; a->y; b->x; b->y; }` and gets weird bindings. Fix — the internal
    order is `(a,x,y,b)`; list the names in that order.
11. Roughly how much memory does the compiler hold for a 6-variable pattern that
    finds 500 000 matches? — 500 000 rows × 6 × 4 B ≈ 12 MB of bindings; the
    allocated capacity is the next power of two (524 288 rows ≈ 12.6 MB), so the
    live allocation is ≈ 12.6 MB, never more than about 2× the useful data.
12. Which constructs store **no** matches at all? — `draw motif(s)` (a private
    streaming backtracker renders each match as it is found) and the IR backend
    for `motifs` (count only, `bindings = NULL`).
13. A program declares `motifs M = [...]` with
    `GRAPHEASY_MOTIF_BACKEND=ir` and then runs `for each motif (a,b,c) in M`.
    What goes wrong? — the struct has `count > 0` but `bindings = NULL`; the
    loop reads through a null pointer. Count-only backend ⇒ no listings.
14. Why is `for each graph H in L` heavier per iteration than
    `for each motif (a,b,c) in M`? — each `H` is rebuilt from the binding row as
    a fresh induced subgraph (deep CSR copy) at every iteration; the motif loop
    only reads `k` int32s out of the already-stored row.
15. Does `print numMotifs(M);` avoid the work of finding the matches? — No. On
    the runtime backend the declaration already ran the full search and stored
    every match; `numMotifs` just loads `count`. Only the IR backend avoids
    materialising a list (and then you pay by losing the list).

---

## 9. Reproduction commands (hand to the student)

```bash
cd p1GraphEasy-con-AutoTuner

# 1) write your .graph file, e.g. /tmp/t.graph and an edge list /tmp/t.txt
# 2) compile and link
rm -f final_program
GRAPH_FILE=/tmp/t.graph bash ./03_run.sh
# 3) run
./final_program

# optional: other motif backend / row-driven enumeration
GRAPH_FILE=/tmp/t.graph GRAPHEASY_MOTIF_BACKEND=ir bash ./03_run.sh
GRAPH_FILE=/tmp/t.graph GRAPHEASY_MOTIF_BACKEND=ir GRAPHEASY_MOTIF_ADJDRIVE=1 bash ./03_run.sh

# the suite's pinned motif check (bi-fan = 34)
cd ../verify && bash run.sh motif
```

Edge-list format: `u v` per line (add a third column for weights, and write
`TRUE` in the graph declaration to use weights). Node ids are 0-based; the graph
size is inferred from the file.

---

## 10. Glossary for the student

* **arc / directed edge** — an ordered pair `u->v`.
* **induced subgraph on a vertex set** — keep exactly the arcs whose both
  endpoints are in the set.
* **pattern / motif** — the template you write with `a->b; ...` edges.
* **embedding** — an injective mapping from pattern variables to graph vertices
  that satisfies all pair requirements.
* **orbit / canonical representative** — the set of embeddings that differ only by
  a pattern symmetry; the compiler reports one per orbit.
* **kind function** — the 0/1/−1/2 classification of a pair (absent / positive /
  negative / zero-weight-poison).
* **driver** — the already-bound variable whose neighbour row a level enumerates
  when adjacency driving is on.
* **lower bound** — the compile-time-derived constraint `A[p] < A[π(p)]` that
  implements symmetry breaking.
