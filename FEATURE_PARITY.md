# GraphEasy Feature Parity

This file tracks language features developed in `p2GraphEasy` and their
propagation to the other GraphEasy families.

## Rules

- `p2GraphEasy` is the reference implementation for new language features.
- `[x]` means the feature is implemented, compiled, and covered by a focused
  runtime test in that family.
- `[ ]` means the feature is missing or has not been verified.
- A feature is complete only after its grammar, AST, semantic analysis, LLVM
  lowering, runtime support, generated parser, and tests are updated as needed.
- Update this table in the same commit that propagates a feature.

## Current Matrix

| Feature | P2 | P1 | P1 Autotuner | P2 Autotuner | Verification |
|---|---:|---:|---:|---:|---|
| `directed: true` graph property | [x] | [x] | [x] | [x] | Directed semantic tests |
| Directed weighted graph loading | [x] | [x] | [x] | [x] | Weighted directed tests |
| `out neighbor` and `in neighbor` traversal | [x] | [x] | [x] | [x] | Neighbor traversal tests |
| `outDegree`, `inDegree`, and directed `degree` | [x] | [x] | [x] | [x] | Degree semantic tests |
| `hasEdge(G, u, v)` | [x] | [x] | [x] | [x] | Directed edge tests |
| `neighbors`, `subgraph`, `degreeOrder`, and `orient` | [x] | [x] | [x] | [x] | Compiler and runtime tests |
| `G.vertices` alias | [x] | [ ] | [x] | [x] | P2 feature-parity test |
| Optional `graph` prefix in graph comprehensions | [x] | [ ] | [x] | [x] | P2 feature-parity test |
| Comprehension predicate `edge has <expr>` | [x] | [ ] | [x] | [x] | Bron-Kerbosch test |
| Comprehension predicate `vertex in <set>` | [x] | [ ] | [x] | [x] | Bron-Kerbosch test |
| Independent set-copy semantics | [x] | [ ] | [x] | [x] | Set mutation and Bron-Kerbosch tests |
| Basic `show G;` Graphviz PNG output | [x] | [x] | [x] | [x] | Existing implementation |
| Static visualization v2 | [x] | [ ] | [ ] | [ ] | P2 draw tests |

## Next Propagation Work

Plain `p1GraphEasy` still needs:

- `G.vertices`
- `graph H = [G where ...]`
- `edge has <expr>`
- `vertex in <set>`
- independent set-copy semantics

After these are ported, run the P2 feature-parity test and Bron-Kerbosch test
through P1 before checking the boxes.

## Static Visualization V2

Status: implemented and tested in `p2GraphEasy`. Not yet propagated.
Interactive HTML and motif-specific highlighting are intentionally postponed.

### Minimal Form

```graph
show G;

draw G to "network.svg";
```

### Continuous Algorithm Results

```graph
draw G to "pagerank.svg" {
    layout: "force";
    vertices {
        labels: true;
        size: continuous(rank);
        color: continuous(rank);
    }
};
```

### Graph-Coloring Results

```graph
int colors[4] = [1, 2, 1, 3];

draw G to "coloring.svg" {
    layout: "force";
    vertices {
        labels: true;
        color: categorical(colors);
    }
};
```

`categorical(colors)` gives equal integer class IDs equal colors.
`continuous(values)` maps integer or real measurements onto a continuous
color or size scale. Negative categorical values are rendered gray.

### Layouts And Formats

| GraphEasy layout string | Graphviz backend |
|---|---|
| `"auto"` | Selected from graph size and directedness |
| `"hierarchical"` | `dot` |
| `"force"` | `sfdp` |
| `"radial"` | `twopi` |
| `"circular"` | `circo` |
| `"clustered"` | `osage` |

Output format is inferred from `.svg`, `.png`, `.pdf`, or `.dot`. Directed
graphs use arrows automatically. In a weighted graph, `edges { labels: true; }`
prints edge weights.

### P2 Acceptance Checklist

- [x] Directed graphs use arrows and preserve edge direction.
- [x] Undirected graphs emit each logical edge once.
- [x] `svg`, `png`, `pdf`, and `dot` outputs work.
- [x] Output filenames are controlled by the program.
- [x] All abstract layouts produce non-empty output.
- [x] Vertex labels can be enabled and disabled.
- [x] Categorical integer arrays control vertex color.
- [x] Continuous integer and real arrays control vertex color.
- [x] Continuous integer and real arrays control vertex size.
- [x] Weighted edge labels are supported.
- [x] Style arrays are checked against the graph's vertex count.
- [x] `show` opens output while `draw` does not.
- [x] Graphs above the static-rendering limit require filtering.
- [x] Graphviz runs without shell-string interpolation.
- [x] Focused parser, semantic, runtime, and output tests pass.
