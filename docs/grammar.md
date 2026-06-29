# Grammar

This page summarizes the language syntax used by GraphEasy programs. The first part follows the core grammar in `Base.g4`; the second part documents extensions used by the benchmark programs.

## Program Structure

A program is a sequence of statements and function definitions:

```antlr
program: (statement | function)* EOF;
```

Statements include graph definitions, variable declarations, conditionals, loops, graph comprehensions, queries, graph display, graph mutation, function calls, and print statements.

## Graph Definitions

Graphs are declared with the `graph` keyword:

```graph
graph G {
  nodes: 0, 1, 2, 3;
  edges: 0->1, 1->2, 2->3;
};
```

Graphs can also load edges from a file:

```graph
graph G {
  edges: file "edgelist.txt";
};
```

Graphs are undirected by default. Add `directed:true;` inside the graph block to preserve edge direction:

```graph
graph G {
  directed:true;
  edges: file "directed_edgelist.txt";
};
```

For a directed graph, an input line `u v` creates only `u -> v`. Without `directed:true;`, GraphEasy treats that line as an undirected edge and builds both adjacency directions.

Weighted input is used by the benchmark runtime with `TRUE` inside the graph block:

```graph
graph G {
  edges: file "weighted_edgelist.txt";
  TRUE
};
```

## Types And Variables

Core scalar types are:

| Type | Meaning |
| --- | --- |
| `int` | Integer |
| `real` | Floating-point value |
| `bool` | Boolean |
| `string` | String |
| `vertex` | Vertex value |
| `edge` | Edge value |
| `graph` | Graph value |

Examples:

```graph
int source = 0;
real damping = 0.85;
bool active = true;
string label = "bfs";
```

## Expressions

The core expression grammar supports arithmetic, function calls, identifiers, integers, reals, booleans, and parentheses:

```graph
int x = 2 + 3 * 4;
real y = (1.0 - damping) / n;
int z = numVertices(G);
```

Comparison and logical operators are used in conditions:

```graph
if (dist[v] > dist[u] + w) {
  dist[v] = dist[u] + w;
}

if (a < b && b < c) {
  print "ordered";
}
```

## Loops

GraphEasy has `while` loops:

```graph
while (round < max_rounds) {
  round = round + 1;
}
```

It also has graph traversal loops:

```graph
for each vertex v in G {
  print v;
}

for each edge u, v in G {
  print u;
}

for each neighbor v of u in G {
  print v;
}

for each out neighbor v of u in G {
  print v;
}

for each in neighbor v of u in G {
  print v;
}
```

On undirected graphs, `neighbor`, `out neighbor`, and `in neighbor` traverse the same adjacency. On directed graphs, `neighbor` and `out neighbor` traverse outgoing edges, while `in neighbor` traverses incoming edges.

## Graph Queries

Built-in graph queries use this form:

```graph
query result : "bfs" of G;
print result;
```

The grammar accepts a string query name. Runtime support determines which query strings are implemented.

## Graph Comprehension

Graph comprehensions filter an existing graph:

```graph
H = [G where degree > 2];
K = [G where degree > 2 && connected with 0];
```

Supported graph conditions include degree comparisons, connectivity with a vertex, logical `&&`, logical `||`, and parentheses.

## Graph Mutation

Nodes and edges can be added to or removed from a graph:

```graph
add 4 to G;
add 2->4 to G;
remove 0->1 from G;
```

## Functions

Functions have a return type, name, parameter list, and block:

```graph
fn int add_one(int x) {
  return x + 1;
}

int y = add_one(4);
```

## Print And Show

```graph
print "done";
print x;
print edges of G;
print nodes of G;
print graph of G;
show G;
```

## Benchmark Extensions

The benchmark algorithms use implementation features beyond the minimal `Base.g4` listing.

### Arrays

```graph
int n = numVertices(G);
int parent[n];
real rank[n];

parent[0] = 0;
rank[0] = 1.0 / n;
```

### Sets

```graph
set frontier;
set next;

frontier = {0};
next = {};
next.add(v);

while (setSize(frontier) > 0) {
  frontier = next;
}
```

### Set Traversal

```graph
for each element u in frontier {
  print u;
}
```

### Runtime Helpers

Common helper calls in benchmark programs:

| Helper | Use |
| --- | --- |
| `numVertices(G)` | Number of vertices |
| `numEdges(G)` | Logical edge count; directed graphs are not divided by two |
| `degree(G, v)` | Undirected degree, or `inDegree + outDegree` on directed graphs |
| `outDegree(G, v)` | Outgoing degree |
| `inDegree(G, v)` | Incoming degree |
| `hasEdge(G, u, v)` | Direction-sensitive edge lookup on directed graphs |
| `weight(G, u, v)` | Edge weight for weighted SSSP |
| `setSize(frontier)` | Number of elements in a set |
| `swap(a, b)` | Swap array-backed frontier buffers |
