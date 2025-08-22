import random

def generate_connected_graph(n_nodes=1000, extra_edges=2000, output_file="g.txt"):
    edges = set()
    
    # Step 1: Create a spanning tree to ensure connectivity
    for i in range(1, n_nodes):
        u = i
        v = random.randint(0, i - 1)
        edges.add((min(u, v), max(u, v)))

    # Step 2: Add extra random edges
    while len(edges) < n_nodes - 1 + extra_edges:
        u = random.randint(0, n_nodes - 1)
        v = random.randint(0, n_nodes - 1)
        if u != v:
            edge = (min(u, v), max(u, v))
            edges.add(edge)

    # Step 3: Write to file
    with open(output_file, 'w') as f:
        for u, v in edges:
            f.write(f"{u} {v}\n")

    print(f"Graph with {n_nodes} nodes and {len(edges)} edges written to {output_file}")

if __name__ == "__main__":
    generate_connected_graph()
