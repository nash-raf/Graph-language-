# Writing Bron-Kerbosch in GraphEasy

This tutorial builds a maximal-clique program one part at a time. It starts with a plain edge-list file and ends with a complete recursive GraphEasy program.

## 1. Write The Edge List

Create a file named `bk_edges.txt`. Write one edge on each line using two vertex ids:

```text
0 1
1 2
0 2
2 3
```

Vertices `0`, `1`, and `2` form a triangle. Vertex `3` is connected to vertex `2`.

GraphEasy treats a graph as undirected unless `directed:true` is present, so each edge can be written once.

## 2. Declare The Graph

Load the edge list inside a graph declaration:

```graph
graph G {
    edges: file "bk_edges.txt";
};
```

`G` is now the graph used by the rest of the program. Its vertices are available through `G.nodes` or `G.vertices`.

## 3. Define A Function

A GraphEasy function begins with `fn`, followed by its return type, name, and parameters:

```graph
fn set neighborsOf(graph G, int v) {
    return G.nodes;
}
```

This function receives a graph and a vertex id. Its return type is `set`. We will now replace the temporary return value with the actual neighborhood of `v`.

## 4. Use A Graph Comprehension

A graph comprehension creates a graph view by filtering another graph:

```graph
graph H = [G where edge has v];
```

Read this as: create `H` from `G`, keeping the edges that contain vertex `v`.

The vertices incident to those edges are available through `H.vertices`:

```graph
set Nv = H.vertices;
```

This set contains `v` as well as its neighbors. Remove `v` to leave only the neighboring vertices:

```graph
Nv.remove(v);
```

The completed neighborhood function is:

```graph
fn set neighborsOf(graph G, int v) {
    graph H = [G where edge has v];
    set Nv = H.vertices;
    Nv.remove(v);
    return Nv;
}
```

## 5. Create And Modify Sets

Bron-Kerbosch starts with three sets. `R` and `X` are empty, while `P` initially contains every graph vertex:

```graph
set R = {};
set P = G.nodes;
set X = {};
```

GraphEasy sets support methods such as:

```graph
R.add(v);
P.remove(v);
int count = P.size();
int v = P.pop();
```

Sets can also be combined with `intersect`. The following expressions retain only vertices that are also neighbors of `v`:

```graph
set P_new = P intersect Nv;
set X_new = X intersect Nv;
```

Assigning one set to another creates a copy. This lets the function update a working set without changing the original immediately:

```graph
set P_work = P;
```

## 6. Detect A Maximal Clique

The recursive function receives the graph and its three sets:

```graph
fn int bronKerbosch(graph G, set R, set P, set X) {
```

When both `P` and `X` are empty, the current set `R` is a maximal clique:

```graph
if (P.size() == 0 && X.size() == 0) {
    print "maximal clique";
    print R.size();
    return 0;
}
```

## 7. Select A Candidate

The loop removes one candidate vertex from the working set:

```graph
while (P_work.size() > 0) {
    int v = P_work.pop();
    set Nv = neighborsOf(G, v);
```

Create a copy of the current clique and add the selected vertex:

```graph
set R_new = R;
R_new.add(v);
```

Then keep only candidates connected to `v`:

```graph
set P_new = P intersect Nv;
set X_new = X intersect Nv;
```

## 8. Filter Vertices With A Comprehension

Graph comprehensions can also filter vertices using a set:

```graph
graph H = [G where vertex in P_new];
```

Read this as: create a view of `G` whose active vertices come from `P_new`.

The recursive call uses this graph view and the refined sets:

```graph
bronKerbosch(H, R_new, P_new, X_new);
```

After the recursive call, move `v` from `P` to `X`:

```graph
P.remove(v);
X.add(v);
```

## 9. Complete Program

Save the complete program as `bk.graph`:

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

## 10. Run The Program

Place `bk.graph` and `bk_edges.txt` in the `p2GraphEasy` directory, then run:

```bash
./1run.sh bk.graph bk.graph
GRAPH_FILE=bk.graph ./2final_run.sh
```

The example reports two maximal cliques: the edge clique containing vertices `2` and `3`, and the triangle containing vertices `0`, `1`, and `2`.

```text
maximal clique
2
maximal clique
3
```

The same GraphEasy implementation was tested on a separate 40-edge graph against an independent Python implementation. Both produced maximal-clique sizes `4, 4, 5, 8`.
