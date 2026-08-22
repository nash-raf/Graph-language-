# GraphEasy

GraphEasy is a graph DSL for writing graph algorithms in a direct, readable way. A program declares a graph, traverses vertices or edges, builds subgraphs with graph comprehension, and can print or draw the result.

The goal is simple: describe the graph problem first, then worry about low-level implementation details later.

## Why GraphEasy

Graph programs often mix two different concerns:

- what graph pattern or algorithm the user wants
- how the graph is stored and traversed efficiently

GraphEasy keeps the first part visible. Users write code over graphs, vertices, edges, neighbors, sets, arrays, and graph comprehensions.

```graph
graph G {
  directed:true;
  edges: file "network.txt";
};

motifs FFL = [G where motif {
  source->middle;
  source->target;
  middle->target;
}];

print numMotifs(FFL);
```

This example finds feed-forward loops in a directed network.

## What You Can Write

| Task | GraphEasy feature |
| --- | --- |
| Load a graph | `edges: file "graph.txt";` |
| Preserve direction | `directed:true;` |
| Traverse vertices | `for each vertex v in G` |
| Traverse neighbors | `for each neighbor v of u in G` |
| Use frontiers | `set frontier; frontier.add(v);` |
| Build subgraphs | `graph H = [G where vertex in P];` |
| Find motifs | `motifs M = [G where motif { ... }];` |
| Draw graphs | `draw G to "graph.png";` |
| Run algorithms | BFS, CC, KCore, PageRank, SSSP |

## Documentation Path

Read the docs in this order:

| Section | Use it for |
| --- | --- |
| [Getting Started](tutorial.md) | Write an edge list, declare a graph, run the first program |
| [Language Manual](language.md) | Learn graphs, direction, types, loops, functions, comprehension, motifs, and drawing |
| [Algorithms](algorithms.md) | See BFS, CC, KCore, PageRank, and SSSP written in GraphEasy |
| [Bron-Kerbosch](bron-kerbosch.md) | Learn a recursive graph-comprehension example |
| [Grammar Reference](grammar.md) | Check the lower-level syntax rules |
| [Results](results.md) | See benchmark plots and comparison results |

## Run A Program

From `p2GraphEasy`:

```bash
./1run.sh test.graph test.graph
./2final_run.sh
```

The first command compiles the `.graph` program. The second command links and runs the generated executable.
