# Tutorial: Using the language

This language lets you describe graphs, load them from edgelist files, run built-in queries, or write your own graph algorithms.


## 1. Loading a graph

Suppose we have an `edgelist.txt` file with the following edges:

```
0 1
1 2
1 3
2 4
```

We can define and load the graph like this:

```
graph g {
  edges: file 'edgelist.txt';
};
```

This tells the language to create a graph `g` and load its edges from `edgelist.txt`. Each line is two integers `u v` for an undirected edge.

### Inline nodes and edges

You can also define a graph directly in the program:

```
graph G {
  nodes: 0, 1, 2, 3, 4;
  edges: 0->1, 1->2, 2->0, 2->3;
};
```

### Weighted graphs

For weighted graphs, add `TRUE` inside the graph block and use a weighted edgelist file (`u v weight` per line):

```
graph G {
  edges: file "edgelistw.txt";
  TRUE
};
```


## 2. Built-in queries

Suppose we want to run BFS on graph `g`. We can use a built-in query:

```
query k : "bfs" of g;
```

This performs BFS on `g` and stores the result in `k`. To print it:

```
print k;
```

### Query syntax

```
query <name> : "<algorithm>" [<source>] of <graph>;
```

Supported query strings include:

| Query string | Description |
|--------------|-------------|
| `"bfs"` | Breadth-first search |
| `"dfs"` | Depth-first search |
| `"bfs_src"` | BFS from a source vertex (use optional source integer) |
| `"dfs_src"` | DFS from a source vertex (use optional source integer) |
| `"dijkstra"` | Single-source shortest paths (weighted graph) |
| `"bk"` | Bron–Kerbosch |
| `"transitive_closure"` | Transitive closure |
| `"chromaticity"` | Graph chromatic number |
| `"min_cut"` | Minimum cut |

Example with a source vertex:

```
query path : "bfs_src" 0 of G;
```

Note: `"dijkstra"` requires a weighted graph (`TRUE` in the graph definition).


## 3. Variables and arrays

Declare scalar variables with a type and optional initializer:

```
int n = numVertices(G);
int level = 0;
real x = 1.5;
bool flag = TRUE;
```

### Arrays

Declare fixed-size arrays using the number of vertices:

```
int n = numVertices(G);
int visited[n];
int dist[n];
int frontier[n];
int next_frontier[n];
```

Initialize from a list:

```
int arr[5] = [1, 2, 3, 4, 5];
```

Access and assign elements:

```
visited[0] = 1;
dist[v] = level;
print dist[0];
```

Two-dimensional arrays are also supported:

```
int M[n][n];
M[i][j] = 0;
```

Use `INF` for infinity in shortest-path programs:

```
int inf = INF;
dist[v] = inf;
```


## 4. Sets

Sets store collections of integer vertex (or edge) ids using a Roaring bitmap internally.

### Declaration and literals

```
set visited;
set frontier = {0};
set next = {};
```

Assign from another set or from the graph:

```
frontier = G.nodes;
set E = G.edges;
```

### Set operations

```
result = A union B;
result = A intersect B;
next = {};
frontier = next;
```

### Set methods

```
visited.add(u);
visited.remove(u);
if (!visited.contains(u)) {
  visited.add(u);
}
```

### Set size

```
while (setSize(frontier) > 0) {
  ...
}
```


## 5. For-each loops

The language provides a simple way to iterate over graphs and sets.

### Vertices

```
for each vertex v in G {
  dist[v] = 0;
}
```

### Neighbors

```
for each neighbor u of v in G {
  if (visited[u] == 0) {
    visited[u] = 1;
  }
}
```

The source vertex `v` can be any integer expression.

### Edges

```
for each edge u, v in G {
  ...
}
```

### Set traversal

```
for each element v in frontier {
  dist[v] = level;
}
```

Shorthand (same meaning):

```
for each v in frontier {
  dist[v] = level;
}
```


## 6. While loops, if-else, break, and continue

While loops and conditionals follow a C-like syntax:

```
while (frontier_size > 0) {
  ...
}
```

```
if (dist[u] != inf) {
  ...
} else {
  ...
}
```

You can combine conditions with `&&` and `||`:

```
if (id[u] < id[v] && visited.contains(u)) {
  ...
}
```

Break out of a loop or skip to the next iteration:

```
if (rounds == n) {
  print "negative cycle";
  break;
}

continue;
```


## 7. Built-in functions

| Function | Description |
|----------|-------------|
| `numVertices(G)` | Number of vertices in graph `G` |
| `numEdges(G)` | Number of edges in graph `G` |
| `degree(G, v)` | Degree of vertex `v` |
| `weight(G, u, v)` | Edge weight between `u` and `v` (weighted graph) |
| `edgeWeight(G, e)` | Weight of edge id `e` |
| `edgeSrc(G, e)` | Source vertex of edge id `e` |
| `edgeDst(G, e)` | Destination vertex of edge id `e` |
| `setSize(s)` | Number of elements in set `s` |
| `hasEdge(G, u, v)` | Whether edge `(u, v)` exists |
| `min(a, b)` | Minimum of two values |
| `max(a, b)` | Maximum of two values |
| `timer()` | High-resolution timer value |
| `swap(a, b)` | Swap two array variables |

Example:

```
int n = numVertices(G);
int w = weight(G, 1, 3);
print setSize(frontier);
swap(frontier, next_frontier);
```


## 8. Print and show

Print strings, expressions, or arrays:

```
print "hello";
print dist[0];
print n + 1;
print "value = " + n;
print arr[];
print arr[i];
```

Print graph structure:

```
print nodes of G;
print edges of G;
print graph of G;
```

Show a graph (visualization hook):

```
show G;
```


## 9. Graph updates

Add or remove nodes and edges at runtime (unweighted graphs):

```
add 5 to G;
add 0->5, 5->6 to G;
remove 3 from G;
remove 0->1 from G;
```


## 10. Graph comprehensions

Build a new graph by combining or filtering existing graphs:

```
Inter = [G1 && G2];
Union = [G1 || G2];
Filtered = [G1 where degree >= 2];
Filtered = [G1 where connected with 0];
Filtered = [G1 where cycle];
```

Combine conditions with `&&` and `||`:

```
Filtered = [G1 || G2 where (connected with 0 && degree >= 2) || cycle];
```

Graph comprehensions are supported on unweighted graphs.


## 11. User-defined functions

Define functions with `fn`, parameters, and a return type:

```
fn int myFunc(int x, graph G) {
  return x + numVertices(G);
}
```

Supported types: `graph`, `vertex`, `edge`, `int`, `real`, `bool`, `string`, `set`, `void`.

Call functions like ordinary expressions:

```
int result = myFunc(3, G);
```


## 12. Other statements

### Sleep

Pause execution (duration in seconds):

```
sleep(1.5);
```

### Swap

Swap two same-sized array variables:

```
swap(frontier, next_frontier);
```

### Return, break, continue

Inside functions and loops:

```
return 0;
break;
continue;
```


## 13. Example: BFS with arrays

```
graph G {
  edges: file 'edgelist.txt';
};

int n = numVertices(G);
int visited[n];
int dist[n];
int frontier[n];
int next_frontier[n];
int frontier_size = 1;
int next_size = 0;
int level = 0;

frontier[0] = 0;
visited[0] = 1;

while (frontier_size > 0) {
  next_size = 0;
  int i = 0;

  while (i < frontier_size) {
    int v = frontier[i];
    dist[v] = level;

    for each neighbor u of v in G {
      if (visited[u] == 0) {
        visited[u] = 1;
        next_frontier[next_size] = u;
        next_size = next_size + 1;
      }
    }
    i = i + 1;
  }

  swap(frontier, next_frontier);
  frontier_size = next_size;
  level = level + 1;
}
```


## 14. Example: BFS with sets

```
graph G {
  edges: file 'edgelist.txt';
};

int n = numVertices(G);
int dist[n];
set visited = {0};
set frontier = {0};
set next;
int level = 0;

while (setSize(frontier) > 0) {
  next = {};
  for each element v in frontier {
    dist[v] = level;
    for each neighbor u of v in G {
      if (!visited.contains(u)) {
        visited.add(u);
        next.add(u);
      }
    }
  }
  frontier = next;
  level = level + 1;
}
```


## 15. Example: shortest paths (SSSP)

```
graph G {
  edges: file "edgelistw.txt";
  TRUE
};

int n = numVertices(G);
int dist[n];
set frontier;
set next;
int src = 1;

for each vertex v in G {
  dist[v] = INF;
}

dist[src] = 0;
frontier = {1};

while (setSize(frontier) > 0) {
  next = {};
  for each element u in frontier {
    if (dist[u] != inf) {
      for each neighbor v of u in G {
        int nd = dist[u] + weight(G, u, v);
        if (nd < dist[v]) {
          dist[v] = nd;
          next.add(v);
        }
      }
    }
  }
  frontier = next;
}
```
