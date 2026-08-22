# Getting Started

This guide introduces GraphEasy through the **Bron-Kerbosch maximal clique algorithm**.

Bron-Kerbosch is a good first GraphEasy example because it uses the parts of the language that make GraphEasy different:

- graph declarations
- functions
- sets
- set intersection
- graph comprehension
- recursion
- graph traversal through filtered subgraphs

A **clique** is a set of vertices where every vertex is connected to every other vertex. A **maximal clique** is a clique that cannot be made larger by adding another vertex.

## Example Graph

The example graph has a triangle and one extra edge:

```text
0 1
1 2
0 2
2 3
```

Vertices `0`, `1`, and `2` form a triangle. Vertex `3` connects only to vertex `2`.

GraphEasy treats a graph as undirected unless `directed:true;` is written in the graph block, so each undirected edge is written once.

## Complete Program

```graph
fn set neighborsOf(graph G, int v) {
    graph H = [G where edge has v];
    set Nv = H.vertices;
    Nv.remove(v);
    return Nv;
}

fn int bronKerbosch(graph G, set R, set P, set X) {
    if (P.size() == 0 && X.size() == 0) {
        print "maximal clique";
        print R.size();
        return 0;
    }

    set P_work = P;

    while (P_work.size() > 0) {
        int v = P_work.pop();

        set Nv = neighborsOf(G, v);

        set R_new = R;
        R_new.add(v);

        set P_new = P intersect Nv;
        set X_new = X intersect Nv;

        graph H = [G where vertex in P_new];

        bronKerbosch(H, R_new, P_new, X_new);

        P.remove(v);
        X.add(v);
    }

    return 0;
}

graph G {
    edges: file "bk_edges.txt";
};

set R = {};
set P = G.nodes;
set X = {};

bronKerbosch(G, R, P, X);
```

## Graph Declaration

```graph
graph G {
    edges: file "bk_edges.txt";
};
```

This loads the edge-list file into a graph named `G`.

The graph object gives access to useful graph members:

```graph
G.nodes
G.vertices
G.edges
```

In this program, `G.nodes` is used to initialize the starting candidate set.

## The Three Sets

Bron-Kerbosch uses three sets:

```graph
set R = {};
set P = G.nodes;
set X = {};
```

Their meanings are:

| Set | Meaning |
| --- | --- |
| `R` | The clique currently being built |
| `P` | Candidate vertices that can still be added |
| `X` | Vertices already considered for this branch |

At the start:

- `R` is empty
- `P` contains all vertices
- `X` is empty

The call starts the recursion:

```graph
bronKerbosch(G, R, P, X);
```

## Function Definition

```graph
fn int bronKerbosch(graph G, set R, set P, set X) {
```

GraphEasy functions begin with `fn`, then the return type, function name, and parameters.

This function receives:

- a graph `G`
- the current clique set `R`
- the candidate set `P`
- the excluded set `X`

It returns an `int`. The return value is not important here; `0` is used only to finish the function.

## Base Case

```graph
if (P.size() == 0 && X.size() == 0) {
    print "maximal clique";
    print R.size();
    return 0;
}
```

If `P` is empty, there are no more candidates to add.

If `X` is also empty, then this branch has found a maximal clique. The current clique is `R`, so the program prints `R.size()`.

## Working Copy Of Candidates

```graph
set P_work = P;
```

The algorithm needs to iterate through the current candidates while also modifying `P`. `P_work` is a copy used for loop control.

## Candidate Loop

```graph
while (P_work.size() > 0) {
    int v = P_work.pop();
```

The loop continues until every candidate in `P_work` has been tried.

`pop()` removes and returns one vertex from the set. That vertex is named `v`.

## Finding Neighbors With Graph Comprehension

```graph
set Nv = neighborsOf(G, v);
```

The helper function `neighborsOf` returns the neighbors of `v`.

Here is the full helper:

```graph
fn set neighborsOf(graph G, int v) {
    graph H = [G where edge has v];
    set Nv = H.vertices;
    Nv.remove(v);
    return Nv;
}
```

The key line is:

```graph
graph H = [G where edge has v];
```

This is graph comprehension. It means:

> Build a graph `H` from `G`, keeping only edges that contain vertex `v`.

Then:

```graph
set Nv = H.vertices;
```

gets all vertices incident to those kept edges. That includes `v` itself, so:

```graph
Nv.remove(v);
```

removes `v`, leaving only its neighbors.

## Add The Candidate To The Clique

```graph
set R_new = R;
R_new.add(v);
```

`R_new` is the clique for the recursive branch where `v` is included.

The original `R` is kept unchanged for other branches.

## Restrict Future Candidates

```graph
set P_new = P intersect Nv;
set X_new = X intersect Nv;
```

This is the central Bron-Kerbosch rule.

After adding `v`, the next clique can only use vertices connected to `v`. Therefore:

- `P_new` keeps only candidate vertices that are also neighbors of `v`
- `X_new` keeps only excluded vertices that are also neighbors of `v`

This is how the algorithm preserves the clique property.

## Filter The Graph

```graph
graph H = [G where vertex in P_new];
```

This is another graph comprehension.

It means:

> Build a graph `H` from `G`, keeping the vertices that are in `P_new`.

This filtered graph is passed to the recursive call:

```graph
bronKerbosch(H, R_new, P_new, X_new);
```

The language-level idea is that the recursive call can work on the graph view relevant to the current branch.

## Move Vertex From P To X

```graph
P.remove(v);
X.add(v);
```

After the recursive call finishes, `v` has been fully explored for the current branch.

The algorithm removes it from `P` and adds it to `X`, so the same clique is not reported again through another branch.

## What The Program Prints

For the example graph:

```text
0 1
1 2
0 2
2 3
```

the maximal cliques are:

```text
{0, 1, 2}
{2, 3}
```

So the program prints clique sizes:

```text
maximal clique
3
maximal clique
2
```

The order may differ depending on set pop order, but the clique sizes are the same.

## Feed-Forward Loop With Graph Comprehension

Bron-Kerbosch used graph comprehension to filter by edges and vertices. The same idea can also express a motif pattern.

A **feed-forward loop** is a directed three-vertex pattern:

```text
a -> b
a -> c
b -> c
```

First declare a directed graph:

```graph
graph G {
    directed:true;
    nodes: 0, 1, 2, 3;
    edges: 0->1, 1->2, 0->2, 1->3, 0->3;
};
```

The graph contains two feed-forward loops:

```text
0 -> 1, 0 -> 2, 1 -> 2
0 -> 1, 0 -> 3, 1 -> 3
```

To build one graph containing the feed-forward-loop part of `G`, write the motif inside a graph comprehension:

```graph
graph FFLSubgraph = [G where motif {
    a->b;
    a->c;
    b->c;
}];

print numVertices(FFLSubgraph);
print numEdges(FFLSubgraph);
```

Read this as:

> From `G`, keep the edges that participate in the motif `a->b`, `a->c`, and `b->c`.

This returns a graph, so the result can be traversed or drawn like any other graph:

```graph
for each edge u, v in FFLSubgraph {
    print u;
    print v;
}

draw FFLSubgraph to "ffl_subgraph.png" {
    layout: "hierarchical";
    vertices {
        labels: true;
    }
};
```

If the user wants every occurrence separately, GraphEasy can store a graph collection. This is still graph-shaped output, not just a motif count:

```graph
graphs FFLGraphs = [G where motif {
    a->b;
    a->c;
    b->c;
}];

print numGraphs(FFLGraphs);

for each graph H in FFLGraphs {
    print numVertices(H);
    print numEdges(H);
}
```

Use `graph FFLSubgraph = ...` when one merged result is enough. Use `graphs FFLGraphs = ...` when the individual motif occurrences must remain separate.

## Why This Shows GraphEasy

This one algorithm introduces the main language ideas:

| Feature | Example |
| --- | --- |
| Graph loading | `graph G { edges: file "bk_edges.txt"; };` |
| Sets | `set P = G.nodes;` |
| Set methods | `R_new.add(v);`, `P.remove(v);` |
| Set algebra | `P intersect Nv` |
| Functions | `fn set neighborsOf(...)` |
| Recursion | `bronKerbosch(H, R_new, P_new, X_new);` |
| Graph comprehension over edges | `[G where edge has v]` |
| Graph comprehension over vertices | `[G where vertex in P_new]` |
| Graph comprehension over motifs | `[G where motif { a->b; a->c; b->c; }]` |
| Graph collection from motifs | `graphs FFLGraphs = [G where motif { ... }];` |

The important part is that the program talks in graph terms. It does not ask the user to manually manage CSR arrays or low-level adjacency storage. The user writes the graph structure they want, and the compiler/runtime handles the graph representation underneath.
