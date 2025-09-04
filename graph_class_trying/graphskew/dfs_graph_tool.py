import sys
from graph_tool.all import Graph, dfs_iterator

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <edge_list_file>")
        sys.exit(1)

    file_path = sys.argv[1]

    # Create undirected graph
    g = Graph(directed=False)

    # Read edges and add to graph
    edges = []
    max_vertex = -1
    with open(file_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):  # skip empty/comment lines
                continue
            u_str, v_str = line.split()
            u, v = int(u_str), int(v_str)
            edges.append((u, v))
            max_vertex = max(max_vertex, u, v)

    # Ensure graph has enough vertices
    g.add_vertex(max_vertex + 1)

    # Add edges
    g.add_edge_list(edges)

    # DFS across all components (source=None means cover all)
    order = []
    seen = set()
    for e in dfs_iterator(g, source=None):
        u = int(e.source())
        v = int(e.target())
        if u not in seen:
            seen.add(u)
            order.append(u)
        if v not in seen:
            seen.add(v)
            order.append(v)



if __name__ == "__main__":
    main()
