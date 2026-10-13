# Grammar migration — exact change log (rows 1–4 + remove calls)

Scope: the new declaration surface (`graph G = load("f")`, with order-free
`directed`/`weighted` modifiers, bare `graph G;`) and the method-call mutation
surface (`G.addNodes/addEdges/removeNodes/removeEdges`).  Old spellings are kept
(dual syntax).  **No backend file was touched** — every new form builds the same
AST node the old spelling builds.

Files changed: `Base.g4`, `generated/*` (regenerated, not hand-edited),
`ASTBuilder.h`, `ASTBuilder.cpp`.  Nothing else.

---

## 1. `Base.g4`

### 1.1 `graphDef` — before → after

**before** (the text in the tree, which did *not* match the compiled parser):

```antlr
graphDef
    : GRAPH graphID '{' graphProperty? nodes? edges? weighted? '}' ';' ;
...
graphProperty : 'directed' ';' ;
weighted      : 'TRUE' ;
```

**after**:

```antlr
graphDef
    : GRAPH graphID '{' graphProperty* nodes? edges? 'TRUE' '}' ';'   # weightedGraphDef
    | GRAPH graphID '{' graphProperty* nodes? edges? '}' ';'          # unweightedGraphDef
    | ('directed' | 'weighted')* GRAPH graphID ';'                    # plainGraphDef
    | ('directed' | 'weighted')* GRAPH graphID '=' 'load' '(' STRING ')' ';'  # loadGraphDef
    ;
```

The first two alternatives are a **resync**, not new syntax: the compiled parser
(September `generated/`) has exactly those labelled alternatives
(`BaseParser::WeightedGraphDefContext` / `UnweightedGraphDefContext`, which
`ASTBuilder.cpp:112-115` casts to) with `graphProperty*` (a vector) and no
`Weighted` rule.  Reproducing them makes regeneration a no-op for existing
programs.  The last two alternatives are new (rows 1–3), with the modifiers
inlined as an order-free `('directed' | 'weighted')*` block.

### 1.2 `graphProperty` restored to the compiled shape

```antlr
// before (tree text, never regenerated):  graphProperty : 'directed' ';' ;
graphProperty : 'directed' ':' drawBoolLiteral ';' ;
```

Why: the compiled `GraphPropertyContext` exposes `drawBoolLiteral()`, which
`ASTBuilder.cpp:992-994` calls.  The tree's `'directed' ';'` text had no such
accessor, so a real regeneration failed to link with
`undefined reference to BaseParser::GraphPropertyContext::drawBoolLiteral()`.
The rule text was recovered from git history (`82657bd`).  Accepted in-brace
spelling is therefore `directed: true;` — the same as the compiled parser.

### 1.3 `weighted : 'TRUE' ;` — removed

The marker is now inline in the `# weightedGraphDef` alternative, matching the
compiled parser (which has no `WeightedContext`).

### 1.4 `functionCall` — `functionName` rule inlined (resync)

```antlr
// before:  functionCall: functionName '(' argumentList? ')';  /  functionName: ID | 'degree';
functionCall: (ID | 'degree') '(' argumentList? ')';
```

Again a resync: the compiled parser has no `FunctionNameContext`.
Language-neutral.

### 1.5 `graphUpdateCall` — new rule (rows 1 + the remove counterpart)

```antlr
graphUpdateCall
    : graphID '.' 'addNodes' '(' nodeList? ')' ';'      # GraphAddNodesCall
    | graphID '.' 'addEdges' '(' edgeList? ')' ';'      # GraphAddEdgesCall
    | graphID '.' 'removeNodes' '(' nodeList? ')' ';'   # GraphRemoveNodesCall
    | graphID '.' 'removeEdges' '(' edgeList? ')' ';'   # GraphRemoveEdgesCall
    ;
```

Literals only (`nodeList` = `nodeID (',' nodeID)*`, `edgeList` = `edge (',' edge)*`,
`edge : nodeID '->' nodeID` — all pre-existing rules, reused).

### 1.6 `statement` — one line added

```antlr
	| nodeEdgeOperation
	| graphUpdateCall      ← added
```

### 1.7 The old mutation family is kept (compat, on purpose)

```antlr
// (compat surface, kept on purpose:) the old `add … to G` / `remove … from G`
nodeEdgeOperation: addOperation | removeOperation;
addOperation: 'add' graphUpdateTargets 'to' graphID ';';
removeOperation: 'remove' graphUpdateTargets 'from' graphID ';';
graphUpdateTargets: updateNodeTargetList | updateEdgeTargetList;
updateNodeTargetList: expr (',' expr)*;
updateEdgeTargetList: updateEdgeTarget (',' updateEdgeTarget)*;
updateEdgeTarget: expr '->' expr;
```

They and `graphUpdateCall` both desugar to the same `GraphUpdateNode`, so the
backend is identical and no fixture has to move.

### 1.8 NOT mine (pre-existing whitespace drift in the same file)

- `graphComprehension … ']' ';'  ;` (an extra space) and
- three blank lines after `graphListDecl`.
Both were already in the working tree before this work.

---

## 2. `generated/*` — regenerated with the matching tool

```bash
java -jar ~/antlr-4.13.1-complete.jar -Dlanguage=Cpp -visitor Base.g4 -o generated
```

Gate, comparing the context-class set against the compiled September parser:

- **dropped: 0** (nothing that existed disappeared — compat and backend-visible rules intact)
- **added: exactly 7** — `GraphAddNodesCall`, `GraphAddEdgesCall`,
  `GraphRemoveNodesCall`, `GraphRemoveEdgesCall`, `GraphUpdateCall`,
  `PlainGraphDef`, `LoadGraphDef`

Backup of the pre-change parser: `/tmp/gen_sept_backup`.
(The `antlr4` tool is not on PATH here; the jar is at `~/antlr-4.13.1-complete.jar`
and the installed runtime is 4.13.1 — versions must match.)

---

## 3. `ASTBuilder.h` — six declarations

```cpp
    antlrcpp::Any visitPlainGraphDef(BaseParser::PlainGraphDefContext *ctx) override;
    antlrcpp::Any visitLoadGraphDef(BaseParser::LoadGraphDefContext *ctx) override;
    antlrcpp::Any visitGraphAddNodesCall(BaseParser::GraphAddNodesCallContext *ctx) override;
    antlrcpp::Any visitGraphAddEdgesCall(BaseParser::GraphAddEdgesCallContext *ctx) override;
    antlrcpp::Any visitGraphRemoveNodesCall(BaseParser::GraphRemoveNodesCallContext *ctx) override;
    antlrcpp::Any visitGraphRemoveEdgesCall(BaseParser::GraphRemoveEdgesCallContext *ctx) override;
```

---

## 4. `ASTBuilder.cpp`

### 4.1 `graphDef` dispatch — two casts added

```cpp
        else if (auto ug = dynamic_cast<BaseParser::UnweightedGraphDefContext *>(ctx->graphDef()))
            return visitUnweightedGraphDef(ug);
        else if (auto pg = dynamic_cast<BaseParser::PlainGraphDefContext *>(ctx->graphDef()))   // ← added
            return visitPlainGraphDef(pg);                                                      // ← added
        else if (auto lg = dynamic_cast<BaseParser::LoadGraphDefContext *>(ctx->graphDef()))    // ← added
            return visitLoadGraphDef(lg);                                                       // ← added
```

### 4.2 Statement dispatch — new branch before `nodeEdgeOperation`

```cpp
    else if (ctx->graphUpdateCall())
    {
        if (auto *an = dynamic_cast<BaseParser::GraphAddNodesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphAddNodesCall(an);
        if (auto *ae = dynamic_cast<BaseParser::GraphAddEdgesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphAddEdgesCall(ae);
        if (auto *rn = dynamic_cast<BaseParser::GraphRemoveNodesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphRemoveNodesCall(rn);
        if (auto *re = dynamic_cast<BaseParser::GraphRemoveEdgesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphRemoveEdgesCall(re);
        throw std::runtime_error("internal: unknown graphUpdateCall alternative");
    }
```

### 4.3 The six functions (appended block, plus three tiny helpers)

```cpp
namespace {
// The declaration modifiers are an inlined ('directed' | 'weighted')* block, so
// they appear as terminal children of the declaration context.
void readGraphMods(antlr4::ParserRuleContext *ctx, bool &directed, bool &weighted)
{
    for (auto *child : ctx->children)
        if (auto *t = dynamic_cast<antlr4::tree::TerminalNode *>(child))
        {
            const std::string s = t->getText();
            if (s == "directed") directed = true;
            else if (s == "weighted") weighted = true;
        }
}

// Literal node/edge targets: `G.addNodes(0,1,2)` is ONE GraphUpdateNode with
// three Node targets, exactly like `add 0, 1, 2 to G;`.
std::vector<GraphUpdateTarget> litNodeTargets(BaseParser::NodeListContext *nl)
{
    std::vector<GraphUpdateTarget> out;
    if (!nl) return out;
    for (auto *n : nl->nodeID())
        out.emplace_back(std::make_shared<IntLiteralNode>(std::stoi(n->getText())));
    return out;
}

std::vector<GraphUpdateTarget> litEdgeTargets(BaseParser::EdgeListContext *el)
{
    std::vector<GraphUpdateTarget> out;
    if (!el) return out;
    for (auto *e : el->edge())
    {
        ASTNodePtr s = std::make_shared<IntLiteralNode>(std::stoi(e->nodeID(0)->getText()));
        ASTNodePtr d = std::make_shared<IntLiteralNode>(std::stoi(e->nodeID(1)->getText()));
        out.emplace_back(std::move(s), std::move(d));
    }
    return out;
}

std::shared_ptr<ASTNode> makeUpdateNode(const std::string &gname,
                                       GraphUpdateKind kind,
                                       std::vector<GraphUpdateTarget> targets)
{
    return std::static_pointer_cast<ASTNode>(
        std::make_shared<GraphUpdateNode>(kind, gname, std::move(targets)));
}
} // namespace

antlrcpp::Any ASTBuilder::visitPlainGraphDef(BaseParser::PlainGraphDefContext *ctx)
{
    bool directed = false, weighted = false;
    readGraphMods(ctx, directed, weighted);
    const std::string name = ctx->graphID()->getText();
    std::shared_ptr<ASTNode> g;
    if (weighted)
        g = std::make_shared<WeightedGraphDeclNode>(
            name, std::make_unique<InlineNodeList>(std::vector<int>{}),
            std::make_unique<InlineWeightedEdgeList>(
                std::vector<std::pair<int, int>>{},
                llvm::DenseMap<std::pair<int, int>, int>{}));
    else
        g = std::make_shared<GraphDeclNode>(
            name, std::make_unique<InlineNodeList>(std::vector<int>{}),
            std::make_unique<InlineEdgeList>(std::vector<std::pair<int, int>>{}));
    if (directed)
    {
        if (auto *wd = dynamic_cast<WeightedGraphDeclNode *>(g.get()))
            wd->directed = true;
        else
            static_cast<GraphDeclNode *>(g.get())->directed = true;
    }
    return std::static_pointer_cast<ASTNode>(g);
}

antlrcpp::Any ASTBuilder::visitLoadGraphDef(BaseParser::LoadGraphDefContext *ctx)
{
    bool directed = false, weighted = false;
    readGraphMods(ctx, directed, weighted);
    const std::string name = ctx->graphID()->getText();
    std::string file = ctx->STRING()->getText();
    if (file.size() >= 2 && file.front() == '"' && file.back() == '"')
        file = file.substr(1, file.size() - 2);   // strip the STRING quotes
    std::shared_ptr<ASTNode> g;
    if (weighted)
    {
        auto wd = std::make_shared<WeightedGraphDeclNode>(name, file);
        wd->directed = directed;
        g = wd;
    }
    else
    {
        auto d = std::make_shared<GraphDeclNode>(name, file);
        d->directed = directed;
        g = d;
    }
    return std::static_pointer_cast<ASTNode>(g);
}

antlrcpp::Any ASTBuilder::visitGraphAddNodesCall(BaseParser::GraphAddNodesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Add,
                          litNodeTargets(ctx->nodeList()));
}

antlrcpp::Any ASTBuilder::visitGraphAddEdgesCall(BaseParser::GraphAddEdgesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Add,
                          litEdgeTargets(ctx->edgeList()));
}

antlrcpp::Any ASTBuilder::visitGraphRemoveNodesCall(BaseParser::GraphRemoveNodesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Remove,
                          litNodeTargets(ctx->nodeList()));
}

antlrcpp::Any ASTBuilder::visitGraphRemoveEdgesCall(BaseParser::GraphRemoveEdgesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Remove,
                          litEdgeTargets(ctx->edgeList()));
}
```

Nodes reused, nothing new: `GraphDeclNode` (file ctor `ASTNode.h:740`, inline ctor
`:749`), `WeightedGraphDeclNode` (file ctor `:845`), `GraphUpdateNode`
(`:226-238`), `GraphUpdateTarget` (`:210-221`), `IntLiteralNode` (`:253`),
`InlineNodeList` / `InlineEdgeList` / `InlineWeightedEdgeList`.

---

## 5. What is untouched (the backend)

`IRGenVisitor.*`, `SemanticAnalyzer.*`, `pdg.cpp`, `parallel_loop_outline.cpp`,
`graph_frontier_lowering.cpp`, `autotuner_runtime.*`, `gpu_runtime.*` — none
touched.  Proof:

```bash
git diff --name-only                 # expect: Base.g4, generated/*, ASTBuilder.{h,cpp}
git diff --stat -- '*IRGenVisitor*' '*SemanticAnalyzer*' '*graph_frontier*' '*pdg*'
```

---

## 6. Verification results (all run, same graph, old spelling vs new)

Build: clean rebuild `rc=0`, 0 errors (`rm -rf build_lowmem && bash build_lowmem.sh`).

| new form | old spelling (twin) | new | old |
|---|---|---|---|
| `graph G = load("g20k.txt");` | `graph G { edges: file "g20k.txt"; };` | `s -1188220504` | `s -1188220504` |
| `directed graph G = load("g20k.txt");` | `graph G { directed: true; edges: file …; };` | `s 2069847825` | `s 2069847825` |
| `weighted graph G = load("g20k_w.txt");` | `graph G { edges: file …; TRUE };` | `s -1188220504` | `s -1188220504` |
| `directed weighted graph G = load("g20k_w.txt");` | `graph G { directed: true; edges: file …; TRUE };` | `s 2069847825` | `s 2069847825` |
| `G.addEdges(1->2);` | `add 1->2 to G;` | `sum 23` | `sum 23` |
| `G.removeEdges(0->1);` | `remove 0->1 from G;` | `44 → 23` (removal real) | — |
| `G.addNodes(3);` | `add 3 to G;` | `s 10` | `s 10` |
| `G.removeNodes(0);` | `remove 0 from G;` | `s 3` | `s 3` |

Also unchanged: the old-language differential (`doall_single`: serial == 4 threads,
`checksum 1012132.615848`).

---

## 7. Caveats / known limits

1. **`graph G;` is declare-only today.**  A bare declaration parses and builds,
   but a loop over it sees 0 vertices until the graph is filled at *runtime* —
   and the runtime fill does not extend the iteration domain.  The old spelling
   on an empty braced graph (`graph G { }; add 0 to G; …`) crashes identically,
   so this is a pre-existing backend property, not a grammar-change regression.
2. **The old weighted spelling wants the marker after the clause**:
   `graph G { edges: file "w"; TRUE };` — the `edges: file "w", TRUE };` form in
   the migration table never parsed, in either parser.
3. **`build_lowmem.sh` staleness gate** stats only top-level `*.h`, not
   `generated/*.h`, so a regenerated parser leaves stale objects; a clean
   (`rm -rf build_lowmem`) rebuild is required after regeneration.  Same class of
   trap as `02_run.sh`'s "regenerate only if `generated/` is missing".

---

## 8. How to review it yourself

```bash
cd p1GraphEasy-con-AutoTuner
git diff --stat                        # the four files above
git diff Base.g4                       # all grammar edits, before -> after
git diff ASTBuilder.h ASTBuilder.cpp   # the builder wiring
java -jar ~/antlr-4.13.1-complete.jar -Dlanguage=Cpp -visitor Base.g4 -o /tmp/review_gen
grep -oE "class +[A-Za-z0-9_]+Context" /tmp/review_gen/BaseParser.h | wc -l   # expect 141
```
