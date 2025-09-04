import sys
from graph_tool.all import Graph,max_cliques


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <edge_list_file>")
        sys.exit(1)

    file_path = sys.argv[1]

    # Create undirected graph
    g = Graph(directed=False)

    # Read edges
    edges = []
    max_vertex = -1
    with open(file_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            u_str, v_str = line.split()
            u, v = int(u_str), int(v_str)
            edges.append((u, v))
            max_vertex = max(max_vertex, u, v)

    # Add vertices and edges
    g.add_vertex(max_vertex + 1)
    g.add_edge_list(edges)

    # ---- Bron–Kerbosch Maximal Cliques ----
    for clique in max_cliques(g):
        pass


if __name__ == "__main__":
    main()
