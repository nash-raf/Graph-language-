# The GraphEasy Programming Language

This manual describes GraphEasy as a language for writing graph algorithms, graph filters, motif searches, and static graph visualizations. It follows the same idea as a language reference: start with ordinary program syntax, then move into graph-specific features.

GraphEasy programs are written in `.graph` files. A typical program declares a graph, creates arrays or sets, traverses the graph, builds filtered graphs with graph comprehension, and prints or draws the result.

```graph
graph G {
  edges: file "edgelist.txt";
};

int n = numVertices(G);
int degree[n];

for each vertex v in G {
  degree[v] = 0;
}

for each edge u, v in G {
  degree[u] = degree[u] + 1;
}

print degree[0];
```

## Basics

### Statements

A program is a sequence of statements and function definitions. Statements end with semicolons unless they are blocks such as `if`, `while`, or function bodies.

Common statement forms are:

| Statement | Example |
| --- | --- |
| Graph declaration | `graph G { edges: file "g.txt"; };` |
| Variable declaration | `int source = 0;` |
| Array declaration | `real rank[n];` |
| Set declaration | `set frontier = {0};` |
| Assignment | `dist[v] = nd;` |
| Conditional | `if (changed == 1) { ... }` |
| Loop | `while (round < 20) { ... }` |
| Graph traversal | `for each vertex v in G { ... }` |
| Graph comprehension | `graph H = [G where vertex in P];` |
| Motif collection | `motifs M = [G where motif { a->b; }];` |
| Graph collection | `graphs S = [G where motif { a->b; }];` |
| Drawing | `draw G to "out.png";` |
| Printing | `print numVertices(G);` |

### Comments

GraphEasy supports line comments and block comments.

```graph
// one-line comment

/*
  multi-line comment
*/
```

### Primitive Types

| Type | Meaning |
| --- | --- |
| `int` | Integer values, vertex ids, counters, array indices |
| `real` | Floating-point values such as PageRank scores |
| `bool` | Boolean values |
| `string` | Text values |
| `vertex` | Vertex-typed value |
| `edge` | Edge-typed value |
| `graph` | Graph value |
| `set` | Collection of integer elements, normally vertices |

GraphEasy also has two collection declarations for motif work:

| Declaration | Meaning |
| --- | --- |
| `motifs M = ...;` | A collection of motif role assignments |
| `graphs S = ...;` | A collection of one extracted subgraph per motif match |

`motifs` and `graphs` are declared with their own syntax. They are not ordinary function return types.

### Variables

Variables are declared with a type, name, and optional initializer.

```graph
int source = 0;
real damping = 0.85;
bool active = true;
string label = "bfs";
```

### Arrays

Arrays use C-like syntax.

```graph
int n = numVertices(G);
int parent[n];
real rank[n];
int table[4][4];

parent[0] = 0;
table[1][2] = 7;
```

One-dimensional arrays can be initialized with a literal.

```graph
int colors[6] = [1, 2, 1, 3, 2, 3];
```

### Expressions

GraphEasy supports arithmetic, comparison, logical operations, array access, set operations, function calls, and parentheses.

```graph
int x = 2 + 3 * 4;
real y = (1.0 - damping) / n;
bool ok = dist[v] != INF && active;
int c = colors[v];
```

Supported operators include:

| Kind | Operators |
| --- | --- |
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Comparison | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| Logical | `&&`, `||`, `!` |

`INF` is available as a large integer sentinel for algorithms such as SSSP.

### Functions

Functions start with `fn`, followed by a return type, name, parameters, and a block.

```graph
fn int minValue(int a, int b) {
  if (a < b) {
    return a;
  }
  return b;
}

int m = minValue(3, 5);
```

Functions can return `int`, `real`, `bool`, `string`, `vertex`, `edge`, `graph`, `set`, or `void`.

```graph
fn set neighborsOf(graph G, int v) {
  graph H = [G where edge has v];
  set Nv = H.vertices;
  Nv.remove(v);
  return Nv;
}
```

### Printing And Timing

Use `print` for strings, expressions, arrays, and graph summaries.

```graph
print "done";
print numVertices(G);
print parent[0];
print edges of G;
print nodes of G;
print graph of G;
```

`timer()` returns a real-valued timestamp. A simple timing pattern is:

```graph
real t0 = timer();
// algorithm body
real t1 = timer();
print t1 - t0;
```

## Graph Data Model

### Inline Graphs

Graphs can be written directly in the program.

```graph
graph G {
  nodes: 0, 1, 2, 3;
  edges: 0->1, 1->2, 2->3;
};
```

### Edge-List Files

Graphs can also be loaded from a plain text edge-list file.

```graph
graph G {
  edges: file "graph.txt";
};
```

An unweighted file has two integer columns:

```text
0 1
1 2
2 3
```

A weighted file has three integer columns:

```text
0 1 7
1 2 3
2 3 5
```

### Directed Graphs

Graphs are undirected by default. If `directed:true;` is not present, GraphEasy treats an input edge as an undirected connection.

```graph
graph G {
  directed:true;
  edges: file "directed.txt";
};
```

For a directed graph, an input line `u v` creates only `u -> v`. For an undirected graph, the same line is available from both `u` and `v`.

### Weighted Graphs

There are two weighted forms.

Legacy weighted input uses `TRUE` in the graph block:

```graph
graph G {
  edges: file "weighted.txt";
  TRUE
};
```

The newer property form is more explicit:

```graph
graph G {
  weights:true;
  edges: file "weighted.txt";
};
```

Weight modes can also assign a fixed sign to every loaded edge:

```graph
graph Positive {
  directed:true;
  weights:positive;
  edges: file "edges.txt";
};
```

| Weight mode | Meaning |
| --- | --- |
| `weights:true;` | Read the third column from the file |
| `weights:positive;` | Store every edge with weight `1` |
| `weights:zero;` | Store every edge with weight `0` |
| `weights:negative;` | Store every edge with weight `-1` |

If a graph is unweighted, `weight(G, u, v)` behaves as weight `1` for existing edges.

### Graph Members

Graph vertices and edges can be used as sets.

```graph
set V = G.nodes;
set W = G.vertices;
set E = G.edges;
```

`G.nodes` and `G.vertices` are aliases for the vertex set.

## Control Flow

### Conditionals

```graph
if (dist[v] > nd) {
  dist[v] = nd;
} else {
  print "no update";
}
```

### While Loops

```graph
while (changed == 1) {
  changed = 0;
}
```

`break;` and `continue;` can be used inside loop bodies.

### Swap And Sleep

`swap(a, b);` swaps two variables. `sleep(x);` pauses execution for `x`.

```graph
swap(frontier, next);
sleep(1);
```

## Graph Traversal

GraphEasy has graph-aware loops, so users do not need to manually index CSR arrays.

### Vertices

```graph
for each vertex v in G {
  print v;
}
```

### Edges

```graph
for each edge u, v in G {
  print u;
  print v;
}
```

### Neighbors

```graph
for each neighbor v of u in G {
  print v;
}
```

On an undirected graph, `neighbor`, `out neighbor`, and `in neighbor` traverse the same adjacency. On a directed graph, `neighbor` and `out neighbor` traverse outgoing edges, while `in neighbor` traverses incoming edges.

```graph
for each out neighbor v of u in G {
  print v;
}

for each in neighbor v of u in G {
  print v;
}
```

### Sets

```graph
for each element v in frontier {
  print v;
}
```

### Motif Results

Role names inside `for each motif (...)` correspond to the role names in the motif pattern.

```graph
motifs FFL = [G where motif {
  a->b;
  a->c;
  b->c;
}];

for each motif (a, b, c) in FFL {
  print a;
  print b;
  print c;
}
```

### Graph Collections

`graphs` keeps one separate graph per motif match, so repeated shared edges remain represented in separate answers.

```graph
graphs FFLGraphs = [G where motif {
  a->b;
  a->c;
  b->c;
}];

for each graph H in FFLGraphs {
  print numVertices(H);
  print numEdges(H);
}
```

## Sets

Sets are used for frontiers, candidate vertices, clique state, visited vertices, and subgraph filters.

```graph
set empty = {};
set frontier = {0};

frontier.add(1);
frontier.remove(0);

if (frontier.contains(1)) {
  print "present";
}

int n = frontier.size();
int v = frontier.pop();
```

Set expressions support union and intersection.

```graph
set A = {1, 2, 3};
set B = {2, 3, 4};
set C = A intersect B;
set D = A union B;
```

## Graph Comprehension

Graph comprehension builds a new graph from an existing graph.

```graph
graph H = [G where edge has v];
```

Read this as: from `G`, keep the edges that contain `v`.

### Vertex Filters

```graph
set P = {0, 1, 2};
graph H = [G where vertex in P];
```

This creates the subgraph induced by the vertices in `P`.

### Edge Filters

```graph
graph Incident = [G where edge has v];
```

This is useful for neighborhood algorithms. For example, Bron-Kerbosch can get the neighbors of `v` by taking incident edges and removing `v` from the resulting vertex set.

```graph
fn set neighborsOf(graph G, int v) {
  graph H = [G where edge has v];
  set Nv = H.vertices;
  Nv.remove(v);
  return Nv;
}
```

### Degree, Connectivity, And Cycle Filters

Graph conditions can use degree comparisons, connectivity with a fixed vertex, and cycle filtering.

```graph
graph HighDegree = [G where degree > 2];
graph AroundZero = [G where connected with 0];
graph Cycles = [G where cycle];
```

Graph conditions can be combined.

```graph
graph H = [G where degree > 2 && connected with 0];
```

### Motif Filters In Comprehension

Graph comprehension can also use a motif condition. This returns a graph result, not a list of separate matches.

```graph
graph FFLSubgraph = [G where motif {
  a->b;
  a->c;
  b->c;
}];
```

Use this when the desired output is the union of all matching motif edges. Use `motifs` or `graphs` when the separate matches themselves matter.

## Motif Answers

Motif syntax lets the user write the small pattern directly. The role names are arbitrary identifiers. They are not fixed words; `a`, `b`, `c`, `source`, `middle`, and `target` are all valid names.

### Feed-Forward Loop

```graph
motifs FFL = [G where motif {
  a->b;
  a->c;
  b->c;
}];

print numMotifs(FFL);
```

This finds directed triples where `a` points to `b`, `a` points to `c`, and `b` points to `c`.

### Motifs Versus Graphs Versus Graph

GraphEasy gives three useful result shapes:

| Form | Example | Use |
| --- | --- | --- |
| `motifs` | `motifs M = [G where motif { ... }];` | Count or iterate role assignments |
| `graphs` | `graphs S = [G where motif { ... }];` | Keep one extracted graph per match |
| `graph` | `graph H = [G where motif { ... }];` | Build one merged subgraph containing all motif edges |

For motif research, `motifs` is the lightest answer for counts and `graphs` is the clearest answer when each occurrence must stay separate.

```graph
motifs FFLMatches = [G where motif {
  a->b;
  a->c;
  b->c;
}];

graphs FFLGraphs = [G where motif {
  a->b;
  a->c;
  b->c;
}];

print numMotifs(FFLMatches);
print numGraphs(FFLGraphs);
```

### Signed Motif Edges

Motif edges support two symbols:

| Motif edge | Meaning |
| --- | --- |
| `a->b;` | positive or ordinary directed edge |
| `a-|b;` | negative signed directed edge |

For directed unweighted graphs, `a->b` only requires that edge direction exists. For signed weighted graphs, positive edges can match `->` and negative edges can match `-|`.

```graph
motifs SignedChain = [G where motif {
  a->b;
  b-|c;
}];
```

### Common Three- And Four-Node Motifs

```graph
motifs ThreeChain = [G where motif {
  a->b;
  b->c;
}];
```

```graph
motifs BiParallel = [G where motif {
  a->c;
  b->d;
}];
```

```graph
motifs BiFan = [G where motif {
  a->c;
  a->d;
  b->c;
  b->d;
}];
```

```graph
motifs FFL = [G where motif {
  a->b;
  a->c;
  b->c;
}];
```

## Graph Mutation

Graphs can be updated inside a program.

```graph
add 4 to G;
add 2->4 to G;
add 5, 6, 7 to G;
add 5->6, 6->7 to G;

remove 4 from G;
remove 2->4 from G;
```

## Built-In Functions

| Function | Return | Meaning |
| --- | --- | --- |
| `numVertices(G)` | `int` | Number of vertices |
| `numEdges(G)` | `int` | Number of logical edges |
| `degree(G, v)` | `int` | Degree of `v`; on directed graphs this is in-degree plus out-degree |
| `outDegree(G, v)` | `int` | Outgoing degree |
| `inDegree(G, v)` | `int` | Incoming degree |
| `hasEdge(G, u, v)` | `bool` | Whether edge `u -> v` exists; direction-sensitive on directed graphs |
| `weight(G, u, v)` | `int` | Edge weight, or `1` for an unweighted existing edge |
| `neighbors(G, v)` | `set` | Neighbor set of `v` |
| `subgraph(G, S)` | `graph` | Induced subgraph on vertex set `S` |
| `degreeOrder(G)` | `int[]` | Degree-based ordering array |
| `orient(G, order)` | `graph` | Directed graph oriented by an ordering |
| `setSize(S)` | `int` | Size of set `S` |
| `min(a, b)` | `int` or `real` | Minimum value |
| `max(a, b)` | `int` or `real` | Maximum value |
| `numMotifs(M)` | `int` | Number of motif matches |
| `numGraphs(S)` | `int` | Number of graphs in a graph collection |
| `timer()` | `real` | Current timestamp |

Edge helper functions are also available for edge values: `edgeSrc(e)`, `edgeDst(e)`, and `edgeWeight(e)`.

## Built-In Queries

GraphEasy has a query form for quick graph queries:

```graph
query result : "bfs" of G;
print result;
```

The query name is a string. Runtime support determines which query names are implemented. For benchmark and research work, the project usually writes the full algorithm in GraphEasy so the behavior is explicit.

## Static Visualization

GraphEasy can write static graph images.

```graph
draw G to "network.png";
```

The output format is inferred from the filename. SVG, PNG, PDF, and DOT are supported by the current drawing path.

### Layouts

```graph
draw G to "network.svg" {
  layout: "force";
};
```

Available layout names are:

| Layout | Use |
| --- | --- |
| `"auto"` | Default choice |
| `"hierarchical"` | Directed flow, trees, DAG-like graphs |
| `"force"` | General network structure |
| `"radial"` | Center-out views |
| `"circular"` | Small graphs and cycle-heavy examples |
| `"clustered"` | Grouped visual structure |

### Vertex Labels, Colors, And Sizes

Algorithm arrays can be passed directly to drawing.

```graph
int colors[6] = [1, 2, 1, 3, 2, 3];
int sizes[6] = [5, 10, 5, 20, 10, 20];

draw G to "colored.svg" {
  layout: "force";
  vertices {
    labels: true;
    color: categorical(colors);
    size: sizes;
  }
};
```

Use `categorical(colors)` for labels such as connected-component id or graph-coloring class. Use `continuous(rank)` for scores such as PageRank, degree, or centrality.

```graph
draw G to "rank.svg" {
  layout: "force";
  vertices {
    labels: true;
    color: continuous(rank);
    size: continuous(rank);
  }
};
```

### Edge Labels

Weighted graphs can show edge labels.

```graph
draw G to "weighted.svg" {
  layout: "hierarchical";
  edges {
    labels: true;
  }
};
```

Directed graphs are drawn with arrows. Signed motif examples can use `-|` in motif syntax to represent negative regulation-like edges.

### Drawing Motifs

Motif patterns can be drawn from the graph.

```graph
draw motifs of G to "ffl" {
  layout: "hierarchical";
  a->b;
  a->c;
  b->c;
  vertices {
    labels: true;
  }
};
```

## Complete Example: Bron-Kerbosch Neighbor Helper

This example shows why graph comprehension is useful inside a normal algorithm. It builds the graph incident to `v`, turns that graph into a vertex set, removes `v`, and returns the neighbors.

```graph
fn set neighborsOf(graph G, int v) {
  graph H = [G where edge has v];
  set Nv = H.vertices;
  Nv.remove(v);
  return Nv;
}
```

The full Bron-Kerbosch walkthrough is in the getting started tutorial.

## Complete Example: Feed-Forward Loop Answers

```graph
graph G {
  directed:true;
  nodes: 0, 1, 2, 3;
  edges: 0->1, 1->2, 0->2, 1->3, 0->3;
};

motifs FFLMatches = [G where motif {
  a->b;
  a->c;
  b->c;
}];

graphs FFLGraphs = [G where motif {
  a->b;
  a->c;
  b->c;
}];

print numMotifs(FFLMatches);
print numGraphs(FFLGraphs);

for each motif (a, b, c) in FFLMatches {
  print a;
  print b;
  print c;
}

for each graph H in FFLGraphs {
  print numVertices(H);
  print numEdges(H);
}
```

On this graph, the feed-forward loop pattern appears twice: one match on vertices `{0, 1, 2}` and one match on vertices `{0, 1, 3}`.

## Current Notes

- Graphs are undirected unless `directed:true;` is present.
- `neighbor` means outgoing neighbor on directed graphs.
- Use `in neighbor` when incoming traversal is required.
- Use `motifs` when you need counts or role assignments.
- Use `graphs` when you need one subgraph per motif occurrence.
- Use graph comprehension when you want a merged filtered graph.
- Motif role names are user-chosen identifiers, not fixed keywords.
- For benchmark algorithms, prefer writing the full algorithm in GraphEasy rather than hiding it behind a query string.
