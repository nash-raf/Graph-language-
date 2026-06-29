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
| Visualization v2 | [ ] | [ ] | [ ] | [ ] | Proposed below |

## Next Propagation Work

Plain `p1GraphEasy` still needs:

- `G.vertices`
- `graph H = [G where ...]`
- `edge has <expr>`
- `vertex in <set>`
- independent set-copy semantics

After these are ported, run the P2 feature-parity test and Bron-Kerbosch test
through P1 before checking the boxes.

## Visualization V2 Proposal

Status: syntax proposal only. Nothing in this section is implemented yet.

### Design Goals

- Keep `show G;` as the zero-configuration interactive/display command.
- Use `draw` when saving a reproducible research figure.
- Infer the output format from the filename.
- Use backend-independent layout names in GraphEasy source.
- Accept existing arrays and sets for algorithm-result styling.
- Keep simple drawing simple while allowing an optional style block.

### Minimal Form

```graph
show G;

draw G to "network.svg";
```

### Configured Form

```graph
draw G to "pagerank.svg" {
    layout: force;
    labels: false;
    vertex.size: rank;
    vertex.color: rank;
};
```

### Motif Highlighting

```graph
draw G to "feedforward.svg" {
    layout: hierarchical;
    labels: true;
    highlight.vertices: motifVertices;
    highlight.edges: motifEdges;
};
```

### Abstract Layout Names

| GraphEasy layout | Initial Graphviz backend |
|---|---|
| `auto` | Selected from graph size and directedness |
| `hierarchical` | `dot` |
| `force` | `sfdp` |
| `radial` | `twopi` |
| `circular` | `circo` |
| `clustered` | `osage` |

### Decisions Required Before Grammar Changes

- Whether `vertex.color` should accept only arrays or also literal colors.
- Whether highlight styles should be fixed defaults or configurable.
- How edge arrays are indexed and exposed consistently to the DSL.
- Whether HTML interactive output belongs in `draw` or a separate command.
- The maximum graph size allowed without explicit filtering or sampling.
- Whether weighted-edge labels are automatic or opt-in.

### P2 Acceptance Checklist

- [ ] Directed graphs use arrows and preserve edge direction.
- [ ] Undirected graphs emit each logical edge once.
- [ ] `svg`, `png`, `pdf`, and `dot` outputs work.
- [ ] Output filenames are controlled by the program.
- [ ] All abstract layouts produce non-empty output.
- [ ] Vertex labels can be enabled and disabled.
- [ ] Vertex size and color can be driven by algorithm arrays.
- [ ] Vertex-set and edge-set highlighting works.
- [ ] Weighted edge labels are supported.
- [ ] `show` opens output while `draw` does not.
- [ ] Large graphs require filtering, sampling, or an explicit override.
- [ ] Commands are executed without shell-string interpolation.
- [ ] Focused parser, semantic, runtime, and output tests pass.
