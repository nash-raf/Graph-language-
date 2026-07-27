#!/usr/bin/env python3
"""Generate mutation-heavy benchmark templates for autotuner testing."""
import os
from pathlib import Path

TEMPLATE_DIR = Path(__file__).parent / "bench_templates"
TEMPLATE_DIR.mkdir(exist_ok=True)

def gen_mutation_lines(graph, start_node, count, kind="add_edge"):
    """Generate explicit add/remove edge lines."""
    lines = []
    for i in range(count):
        src = start_node + i
        dst = start_node + i + 1
        if kind == "add_edge":
            lines.append(f"add {src}->{dst} to {graph};")
        elif kind == "remove_edge":
            lines.append(f"remove {src}->{dst} from {graph};")
        elif kind == "add_node":
            lines.append(f"add {src} to {graph};")
    return lines


def mutation_heavy_template():
    """
    Pattern: Traverse → MANY mutations → Traverse
    
    The DP should choose: CSR → PCSR → CSR
    because PCSR has cheap round-trip (compact gaps) compared to SET (full rebuild).
    With 500 mutations, the savings from PCSR O(deg) vs CSR O(E) realloc
    easily amortize the conversion cost.
    """
    muts = gen_mutation_lines("g1", 600000, 100, "add_edge")

    return f"""graph g1 {{
    edges: file "__EDGE_FILE_A__";
}};

query bfs_before: "bfs" of g1;

{chr(10).join(muts)}

query bfs_after: "bfs" of g1;
"""


def mutation_heavy_pcsr_set_template():
    """
    Pattern: Traverse → MANY mutations → Traverse → MANY mutations (no trailing traverse)
    
    The DP should choose: CSR → PCSR → CSR → SET
    because:
    - First Insert region is followed by Traverse → PCSR (cheap round-trip to CSR)
    - Second Insert region is NOT followed by Traverse → SET (cheapest to enter, no return needed)
    """
    muts1 = gen_mutation_lines("g1", 600000, 80, "add_edge")
    muts2 = gen_mutation_lines("g1", 700000, 80, "add_edge")

    queries = [
        "set s = g1.nodes;",
        "if (s.contains(700399)) {",
        "    print 1;",
        "} else {",
        "    print 0;",
        "}",
    ]

    return f"""graph g1 {{
    edges: file "__EDGE_FILE_A__";
}};

query bfs_phase1: "bfs" of g1;

{chr(10).join(muts1)}

query bfs_phase2: "bfs" of g1;

{chr(10).join(muts2)}

{chr(10).join(queries)}
"""


def mutation_loop_template():
    """
    Pattern: while loop { Traverse → mutations → Traverse }
    
    Tests loop-carried layout state. The autotuner must inject guards
    for each Traverse inside the loop to ensure CSR.
    """
    muts = gen_mutation_lines("g1", 800000, 80, "add_edge")

    return f"""graph g1 {{
    edges: file "__EDGE_FILE_A__";
}};

query bfs_init: "bfs" of g1;

{chr(10).join(muts)}

query bfs_mid: "bfs" of g1;

{chr(10).join(gen_mutation_lines("g1", 900000, 80, "add_edge"))}

query bfs_final: "bfs" of g1;
"""


def mutation_heavy_inbounds_template():
    """
    Pattern: Traverse → MANY in-bounds mutations → Traverse

    All edges are between nodes 0..n-1 (e.g. 100->101, 101->102, ...).
    When in CSR/PCSR we skip canonical - pure layout-native mutations.
    This should show positive gains from PCSR.
    """
    muts = []
    for i in range(200):
        src = 200 + (i % 300)   # nodes 200..499, all in-bounds for n=50k
        dst = 201 + (i % 300)
        muts.append(f"add {src}->{dst} to g1;")

    return f"""graph g1 {{
    edges: file "__EDGE_FILE_A__";
}};

query bfs_before: "bfs" of g1;

{chr(10).join(muts)}

query bfs_after: "bfs" of g1;
"""


def pcsr_heavy_template():
    """
    PCSR-heavy: BFS → many in-bounds mutations → BFS → many in-bounds → BFS.
    All edges in [0..n-1]. DP should schedule [CSR, PCSR, CSR, PCSR, CSR] = 4 switches.
    Option B: no bitmap updates in CSR/PCSR, so pure layout-native cost.
    """
    def inbounds_muts(count, base):
        return [f"add {base + (i % 200)}->{base + (i % 200) + 1} to g1;" for i in range(count)]

    muts1 = inbounds_muts(150, 200)   # 200->201 .. 349->350
    muts2 = inbounds_muts(150, 400)   # 400->401 .. 549->550 (all in-bounds for n=50k/1M)

    return f"""graph g1 {{
    edges: file "__EDGE_FILE_A__";
}};

query bfs_1: "bfs" of g1;

{chr(10).join(muts1)}

query bfs_2: "bfs" of g1;

{chr(10).join(muts2)}

query bfs_3: "bfs" of g1;
"""


def minimal_no_gain_template():
    """
    Pattern: Traverse → 2 mutations → Traverse
    
    With only 2 mutations, the conversion cost dominates.
    The autotuner should choose NOT to switch (all-CSR).
    This proves worst-case = baseline performance.
    """
    return """graph g1 {
    edges: file "__EDGE_FILE_A__";
};

query bfs_a: "bfs" of g1;

add 500001->500002 to g1;
add 500003->500004 to g1;

query bfs_b: "bfs" of g1;
"""


templates = {
    "mutation_heavy.graph.tmpl": mutation_heavy_template(),
    "mutation_heavy_inbounds.graph.tmpl": mutation_heavy_inbounds_template(),
    "pcsr_heavy.graph.tmpl": pcsr_heavy_template(),
    "mutation_pcsr_set.graph.tmpl": mutation_heavy_pcsr_set_template(),
    "mutation_loop_heavy.graph.tmpl": mutation_loop_template(),
    "minimal_no_gain.graph.tmpl": minimal_no_gain_template(),
}

for name, content in templates.items():
    path = TEMPLATE_DIR / name
    path.write_text(content, encoding="utf-8")
    n_lines = content.count("\n")
    print(f"  Generated {name} ({n_lines} lines)")

print("Done.")
