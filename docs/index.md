# GraphEasy Language

GraphEasy is an efficient, easy-to-use DSL for describing graphs and writing graph algorithms. Programs are written in `.graph` files, compiled by `GraphProgram`, and linked with a C++ runtime.

- **Repo**: https://github.com/nash-raf/Graph-language-
- **Tutorial**: See [Tutorial](tutorial.md)

## What you can do

- Load graphs from an edgelist file or define them inline
- Run built-in graph queries (BFS, DFS, Dijkstra, and more)
- Write custom algorithms with `for each` loops over vertices, edges, neighbors, or sets
- Use arrays, sets, conditionals, and while-loops in a C-like syntax
- Filter and combine graphs with graph comprehensions

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

See the [Tutorial](tutorial.md) for the full language reference.
