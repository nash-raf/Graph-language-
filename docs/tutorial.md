# Getting Started

This page shows the smallest path from a `.graph` file to a running GraphEasy program.

## Minimal Program

```graph
graph G {
  edges: file "edgelist.txt";
};

query path : "bfs" of G;
print path;
```

The graph file is a plain edgelist. For an unweighted graph, each line contains two integer vertex ids:

```text
0 1
1 2
1 3
2 4
```

GraphEasy loads graphs as undirected unless the graph block says otherwise:

```graph
graph G {
  directed:true;
  edges: file "directed_edgelist.txt";
};
```

In a directed graph, `neighbor` means outgoing neighbor. Use `in neighbor`, `out neighbor`, `inDegree`, and `outDegree` when direction matters.

For a weighted graph, each line contains `source destination weight`:

```text
0 1 7
1 2 3
2 4 5
```

## Run A Program

From a GraphEasy implementation directory such as `p2GraphEasy`:

```bash
./1run.sh test.graph test.graph
./2final_run.sh
```

The first command compiles the language frontend and the `.graph` program. The second command links the generated objects with the runtime and executes the final binary.

## Project Layout

The repository has multiple implementation families:

| Directory | Purpose |
| --- | --- |
| `p1GraphEasy` | Baseline GraphEasy implementation |
| `p2GraphEasy` | Optimized GraphEasy implementation |
| `p1GraphEasy-con-AutoTuner` | P1 with autotuner support |
| `p2GraphEasy-con-AutoTuner` | P2 with autotuner support |

## Documentation Sections

Use the rest of the documentation in this order:

| Section | What it covers |
| --- | --- |
| [Grammar](grammar.md) | Core syntax from `Base.g4` and benchmark language extensions |
| [Algorithms](algorithms.md) | BFS parent, connected components, KCore, PageRank, and SSSP in GraphEasy |
| [Bron-Kerbosch](bron-kerbosch.md) | A recursive maximal-clique program using graph comprehensions and set operations |
| [Results](results.md) | Placeholder for benchmark plots, tables, and notes |
