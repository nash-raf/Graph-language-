# GraphEasy Language

GraphEasy is an efficient, easy-to-use DSL for describing graphs and writing graph algorithms. Programs are written in `.graph` files, compiled by `GraphProgram`, and linked with a C++ runtime.

- **Repo**: https://github.com/nash-raf/Graph-language-
- **Getting Started**: See [Getting Started](tutorial.md)

## What you can do

- Load graphs from an edgelist file or define them inline
- Run built-in graph queries (BFS, DFS, Dijkstra, and more)
- Write custom algorithms with `for each` loops over vertices, edges, neighbors, or sets
- Use arrays, sets, conditionals, and while-loops in a C-like syntax
- Filter and combine graphs with graph comprehensions

## Documentation

| Section | Description |
| --- | --- |
| [Getting Started](tutorial.md) | How to write and run a small GraphEasy program |
| [Grammar](grammar.md) | Core syntax from `Base.g4` and benchmark extensions |
| [Algorithms](algorithms.md) | The five benchmark algorithms in GraphEasy |
| [Bron-Kerbosch](bron-kerbosch.md) | How GraphEasy describes recursive maximal-clique search |
| [Results](results.md) | Placeholder for plots and benchmark summaries |

## Quick example

```
graph G {
  edges: file 'edgelist.txt';
};

query path : "bfs" of G;
print path;
```

## Running a program (p2GraphEasy)

From the `p2GraphEasy` directory:

```bash
./1run.sh test.graph test.graph   # compile GraphProgram + your program
./2final_run.sh                   # link and run
```

See [Grammar](grammar.md) for syntax, [Algorithms](algorithms.md) for complete benchmark programs, and [Bron-Kerbosch](bron-kerbosch.md) for a recursive maximal-clique example.
