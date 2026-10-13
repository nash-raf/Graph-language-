# Grammar syntax migration — task list

Source: the 13-row table you showed (current `Base.g4` syntax vs the syntax to adopt,
with its score and verdict).  Bodies are elided as `{ ... }` exactly as in the table.

**How to read the `expected impact` column** — it is a *preliminary* classification from the
measured consumer map (see the bottom of this file), not a code-grounded audit yet:

- **desugar** — can be done as syntax only: `Base.g4` + regenerated `generated/*` + `ASTBuilder.cpp`.
  The new surface is parsed and lowers into *existing* `ASTNodeType` values, so
  `IRGenVisitor.cpp` (145 node-kind references) and `SemanticAnalyzer.cpp` (48) stay untouched.
- **semantics** — needs new node kind(s) / types: `ASTNode.h` (+enum/struct) →
  `SemanticAnalyzer.cpp` → `IRGenVisitor.cpp` (the LLVM IR layer).  Avoidable only by desugaring.
- **retain** — no change (the row keeps the current syntax deliberately).

| # | task | current `Base.g4` syntax | adopt | score | verdict | expected impact |
|---|---|---|---|---|---|---|
| 1 | Graph declaration | `graph G { nodes: 0, 1, 2; edges: 0->1, 1->2; };` | `graph G; G.addNodes(0,1,2); G.addEdges(0->1, 1->2);` | 42 | Decisive → Switch | desugar (bare `graph G;` decl + `G.addNodes/addEdges(...)` onto the existing graph-update / edge-list nodes; `0->1` edge literals already exist) |
| 2 | Load edges from file | `graph G { edges: file "edgelist.txt"; };` | `graph G = load("edgelist.txt");` | 35 | Decisive → Switch | desugar (initializer form onto the existing edge-file form of the graph decl) |
| 3 | Directed graph | `graph G { directed; edges: file "g.txt"; };` | `directed graph G = load("g.txt");` | 24 | Weak → Switch | desugar (modifier keyword + load form; typing already carries directedness) |
| 4 | Weighted graph | `graph G { edges: file "w.txt", TRUE };` | `weighted graph G = load("w.txt");` | 20 | Switch and normalize | desugar (the existing weighted-graph decl node covers it) + **normalize** the load-argument semantics (this part touches `SemanticAnalyzer`) |
| 5 | Iterate vertices | `for each vertex v in G { ... }` | `for v in G.nodes { ... }` | 33 | Switch | desugar onto the existing vertex-`ForEach` node (iterable becomes an expression; encode the iterable kind in the existing field rather than a new node) |
| 6 | Iterate edges | `for each edge u, v in G { ... }` | `for (u, v) in G.edges { ... }` | 28 | Switch | desugar onto the existing edge-`ForEach` node (two-variable binding already exists) |
| 7 | Iterate neighbours | `for each neighbor u of v in G { ... }` | `for u in G.neighbors(v) { ... }` | 48 | Switch | desugar onto the existing neighbour-`ForEach` node.  **This is the hot loop shape for the effect system** — as long as it lowers to the same IR loop structure, `graph_frontier_lowering.cpp` / `pdg.cpp` / `parallel_loop_outline.cpp` are unaffected (they never see the AST) |
| 8 | Membership test | `if (4 in G) { ... }` (integer literal only) | `if (G.hasNode(c)) { ... }` | 43 | Switch | desugar the call onto the existing membership expression **and** widen it from integer-literal-only to an arbitrary expression — the widening is the real work (semantic check + the value being a variable, not the IR *structure*) |
| 9 | Graph-derived set | `set V = G.nodes;` | `set V = G.nodes;` | 25 | Retain for collection consistency | **retain** — no change |
| 10 | Set methods | `V.add(4); V.remove(0);` | `V.add(4); V.remove(0);` | 26 | Retain for generic-set semantics | **retain** — no change |
| 11 | Graph mutation | `add 4 to G; add 2->4 to G;` | `G.addNode(4); G.addEdge(2, 4);` | 52 | Switch | desugar onto the existing graph-update node (the node/edge distinction is already a field there) |
| 12 | Comprehension / filtering | `H = [G where degree > 2];` | `H = subgraph(G, [v for v in G if degree(G, v) > 2]);` | 24 | Switch | **semantics risk**: `[v for v in G if cond]` is a new expression form; if it cannot desugar onto the existing comprehension/query nodes it needs a new `ASTNodeType`, then `SemanticAnalyzer` + `IRGenVisitor`.  Also `subgraph(...)` and `degree(G, v)` must exist as builtins (check the runtime before choosing this surface form) |
| 13 | Motif search | `motifs M = [G where motif { a->b; a->c; b->c; }];` | `M = in G search motif { a->b; a->c; b->c; };` | 32 | Switch | desugar onto the existing motif-matches declaration node (the `motif { ... }` block itself is unchanged); the `M = <expr>` surface may need the assignment path.  Motif consumers are `MotifPattern.cpp` / `MotifIRBuilder.cpp`, not `IRGenVisitor` |

Summary of the preliminary classification: **10 desugar**, **1 retain-pair** (rows 9 and 10),
**1 with a semantics tail** (row 4), **1 genuine semantics risk** (row 12), **1 flag to check
first** (row 7 — the neighbour loop is what the whole parallelization analysis keys on).

## Mechanical prerequisites (measured)

1. **Your grammar edit is a no-op unless you regenerate.**  `02_run.sh` regenerates ANTLR only
   when `generated/` is *missing*, and `build_lowmem.sh` compiles the committed `generated/*.cpp`.
   Run, with the matching tool version (java + jar are present; `antlr4` is not on PATH):
   ```bash
   cd p1GraphEasy-con-AutoTuner
   java -jar ~/antlr-4.13.1-complete.jar -Dlanguage=Cpp -visitor Base.g4 -o generated
   ```
   Consider making the `02_run.sh` gate mtime-based (`Base.g4 -nt generated/BaseParser.cpp`).
2. **Generated file set**: `build_lowmem.sh`, `build_remote.sh`, `run_test.sh` enumerate the
   generated `.cpp`s.  Editing `Base.g4` in place keeps the file set (no script edits); adding a
   second grammar file would require all three.
3. **Parser includes** live in `ASTBuilder.h` (`BaseBaseVisitor.h`), `main.cpp`
   (lexer + parser) and `ast_export_user_request.cpp` (a separate AST-export tool, 3 node-kind
   references — it needs a case only if you add a node kind).

## Verification recipe (proves the LLVM IR layer did not move)

```bash
git diff --name-only                       # expect: Base.g4, generated/*, ASTBuilder.* (+ the files you chose to touch)
git diff --stat -- '*IRGenVisitor*' '*graph_frontier*' '*pdg*' '*parallel_loop*'   # must be empty
bash verify/run.sh parallel                # the 69-check suite must stay green
```

## Consumer map behind the classification (measured)

```
$ grep -c "ASTNodeType::" *.cpp *.h          # files that switch on node kinds
IRGenVisitor.cpp          145   <- the LLVM IR layer
ASTNode.h                  58   <- node enum + structs
SemanticAnalyzer.cpp       48   <- semantic checks
ast_export_user_request.cpp 3   <- separate tool
ASTBuilder.cpp              1   <- node construction (rest via makeNode helpers)

$ grep -ln "BaseBaseVisitor|BaseParser.h|BaseLexer.h|BaseVisitor.h|BaseListener.h" *.cpp *.h
ast_export_user_request.cpp
main.cpp
ASTBuilder.h
```

The mid-end passes (`graph_frontier_lowering.cpp`, `pdg.cpp`, `parallel_loop_outline.cpp`) do
not reference `ASTNodeType` at all — they only see LLVM IR.  A grammar change that lowers to the
same IR shapes leaves them untouched; a *new loop shape* would simply be analysed like any other
loop (and may be refused conservatively rather than break).

## Per-task checklist (fill as you migrate; this doubles as the ledger for `/plan-conformance`)

| # | status | grammar rule(s) changed | builder case | node kinds used | new kind? | IR touched? | evidence |
|---|---|---|---|---|---|---|---|
| 1 | TODO | | | | | | |
| 2 | TODO | | | | | | |
| 3 | TODO | | | | | | |
| 4 | TODO | | | | | | |
| 5 | TODO | | | | | | |
| 6 | TODO | | | | | | |
| 7 | TODO | | | | | | |
| 8 | TODO | | | | | | |
| 9 | retain | – | – | – | – | – | no change by decision |
| 10 | retain | – | – | – | – | – | no change by decision |
| 11 | TODO | | | | | | |
| 12 | TODO | | | | | | |
| 13 | TODO | | | | | | |

Rule for the ledger: a row is `DONE` only with the command and its observed output (e.g. a
fixture printing the same answer before/after the syntax change, and
`git diff --name-only` showing the expected file set).  `PASS*` / `PARTIAL` are not legal
statuses here — if a row cannot be done as syntax-only, it is `BLOCKED` with the question.
