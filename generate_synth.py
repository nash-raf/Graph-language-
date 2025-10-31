import random
import os

def generate_random_graph(n, m, seed, filename="edgelist.txt", weighted=False, weight_range=(1, 10)):
    """
    Generate an undirected random graph with n nodes and m edges.
    No self-loops, no duplicate edges.
    
    If weighted=True, assigns a random weight w in weight_range (inclusive).
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
            if weighted:
                w = random.randint(*weight_range)
                f.write(f"{u} {v} {w}\n")
            else:
                f.write(f"{u} {v}\n")

    print(f"Graph saved: {filename} ({n} nodes, {m} edges){' with weights' if weighted else ''}")


if __name__ == "__main__":
    vertices_list = [50000]
    edges_list = [1000000]
    weighted = True  # Set to True for weighted graphs
    weight_range = (1, 100)

    output_dir = "synth_graphs_weighted" if weighted else "synth_graphs_violin"
    os.makedirs(output_dir, exist_ok=True)

    # Generate 10 different graphs with seeds 1–10
    for n in vertices_list:
        for m in edges_list:
            for i in range(1, 21):  # 20 different seeds
                seed = i
                filename = f"{output_dir}/synth_v_{n}_e_{m}_seed_{seed}{'_w' if weighted else ''}.txt"
                generate_random_graph(
                    n, m, seed=seed, filename=filename,
                    weighted=weighted, weight_range=weight_range
                )
