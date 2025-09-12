import random,os

def generate_random_graph(n, m, seed=42, filename="edgelist.txt"):
    """
    Generate an undirected random graph with n nodes and m edges.
    No self-loops, no duplicate edges.

    Saves the edge list to 'edgelist.txt'
    """
    if m > n * (n - 1) // 2:
        raise ValueError("Too many edges for the given number of nodes")

    random.seed(seed)
    edge_set = set()

    while len(edge_set) < m:
        u = random.randint(0, n - 1)
        v = random.randint(0, n - 1)
        if u != v:
            edge = tuple(sorted((u, v)))
            edge_set.add(edge)

    # Save to file
    with open(filename, "w") as f:
        for u, v in edge_set:
            f.write(f"{u} {v}\n")
    print(f"Graph saved: {filename} ({n} nodes, {m} edges)")


if __name__ == "__main__":
    vertices_list = [1000]
    edges_list = [5000]
    seed = 123

    # Make sure output folder exists
    os.makedirs("synth_graphs", exist_ok=True)

    for n in vertices_list:
        for m in edges_list:
            if m <= n * (n - 1) // 2:  # skip invalid combinations
                filename = f"synth_graphs/synth_v_{n}_e_{m}.txt"
                generate_random_graph(n, m, seed=seed, filename=filename)