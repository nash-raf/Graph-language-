# Algorithms

This page lists the five aligned algorithms used in the benchmark suite.

## BFS Parent

BFS parent computes a breadth-first traversal tree from source vertex `0`. Each discovered vertex stores the vertex that first reached it.

```graph
graph G {
  edges: file "graph.txt";
};

int n = numVertices(G);
int visited[n];
int parent[n];
int frontier[n];
int next_frontier[n];
int frontier_size = 1;
int next_size = 0;
int init_i = 0;

while (init_i < n) {
  visited[init_i] = 0;
  parent[init_i] = -1;
  init_i = init_i + 1;
}

frontier[0] = 0;
visited[0] = 1;
parent[0] = 0;

while (frontier_size > 0) {
  next_size = 0;
  int i = 0;

  while (i < frontier_size) {
    int v = frontier[i];
    for each neighbor u of v in G {
      if (visited[u] == 0) {
        visited[u] = 1;
        parent[u] = v;
        next_frontier[next_size] = u;
        next_size = next_size + 1;
      }
    }
    i = i + 1;
  }

  swap(frontier, next_frontier);
  frontier_size = next_size;
}
```

## Connected Components

Connected components uses label propagation. Each vertex starts with its own id, then lower labels spread through the graph until convergence.

```graph
graph G {
  edges: file "graph.txt";
};

int n = numVertices(G);
int id[n];
set frontier;
set next;

for each vertex v in G {
  id[v] = v;
}

frontier = G.nodes;

while (setSize(frontier) > 0) {
  next = {};
  for each element u in frontier {
    for each neighbor v of u in G {
      if (id[u] < id[v]) {
        id[v] = id[u];
        next.add(v);
      }
    }
  }
  frontier = next;
}
```

## KCore

KCore removes vertices whose degree is below `k`. Removing a vertex reduces the degree of its live neighbors, which may create more removals.

```graph
graph G {
  edges: file "graph.txt";
};

int n = numVertices(G);
int k = 5;
int deg[n];
int alive[n];
set frontier;
set next;

for each vertex v in G {
  deg[v] = 0;
  alive[v] = 1;
}

for each vertex u in G {
  for each neighbor v of u in G {
    deg[u] = deg[u] + 1;
  }
}

frontier = {};
for each vertex v in G {
  if (deg[v] < k) {
    frontier.add(v);
  }
}

while (setSize(frontier) > 0) {
  next = {};
  for each element u in frontier {
    if (alive[u] == 1) {
      alive[u] = 0;
      for each neighbor v of u in G {
        if (alive[v] == 1) {
          deg[v] = deg[v] - 1;
          if (deg[v] < k) {
            next.add(v);
          }
        }
      }
    }
  }
  frontier = next;
}
```

## PageRank

PageRank runs fixed-count power iteration. The benchmark uses damping factor `0.85`, `20` iterations, and outgoing adjacency.

```graph
graph G {
  edges: file "graph.txt";
};

int n = numVertices(G);
real damp = 0.85;
real one_over_n = 1.0 / n;
real beta = (1.0 - damp) / n;
real cur_rank[n];
real next_rank[n];
int out_degree[n];
int round = 0;
int max_rounds = 20;

for each vertex v in G {
  cur_rank[v] = one_over_n;
  next_rank[v] = beta;
  out_degree[v] = 0;
}

for each vertex u in G {
  out_degree[u] = outDegree(G, u);
}

while (round < max_rounds) {
  round = round + 1;

  for each vertex v in G {
    next_rank[v] = beta;
  }

  for each vertex u in G {
    int outd = out_degree[u];
    if (outd > 0) {
      real contrib = damp * cur_rank[u] / outd;
      for each out neighbor v of u in G {
        next_rank[v] = next_rank[v] + contrib;
      }
    }
  }

  for each vertex v in G {
    cur_rank[v] = next_rank[v];
  }
}
```

## SSSP

SSSP uses a frontier-based Bellman-Ford style relaxation over a weighted graph.

```graph
graph G {
  edges: file "weighted_graph.txt";
  TRUE
};

int n = numVertices(G);
int inf = 2147483647;
int dist[n];
set frontier;
set next;
int src = 0;
int rounds = 0;

for each vertex v in G {
  dist[v] = inf;
}

dist[src] = 0;
frontier = {0};

while (setSize(frontier) > 0) {
  if (rounds == n) {
    print "negative cycle";
    break;
  }

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
  rounds = rounds + 1;
}
```
