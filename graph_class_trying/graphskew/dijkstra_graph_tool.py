import sys
from graph_tool.all import Graph, shortest_distance

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <edge_list_file> [source_vertex]")
        sys.exit(1)

    file_path = sys.argv[1]
    source = int(sys.argv[2]) if len(sys.argv) > 2 else 0

    # Create an undirected weighted graph
    g = Graph(directed=False)

    # Add a weight property for edges
    weight = g.new_edge_property("float")

    max_vertex = -1
    edges = []

    # Read edges
    with open(file_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            parts = line.split()
            if len(parts) == 2:
                u, v = map(int, parts)
                w = 1.0  # default weight
            elif len(parts) == 3:
                u, v, w = int(parts[0]), int(parts[1]), float(parts[2])
            else:
                raise ValueError(f"Invalid line: {line}")

            edges.append((u, v, w))
            max_vertex = max(max_vertex, u, v)

    # Ensure graph has enough vertices
    g.add_vertex(max_vertex + 1)

    # Add edges and weights
    for u, v, w in edges:
        e = g.add_edge(u, v)
        weight[e] = w

    # print(f"Number of vertices: {g.num_vertices()}")
    # print(f"Number of edges: {g.num_edges()}")

    # Run Dijkstra’s shortest path from source
    dist = shortest_distance(g, source=source, weights=weight)

    # Print shortest distances
    # print(f"Shortest distances from node {source}:")
    # for i, d in enumerate(dist.a):  # dist is a property map → use .a to access array
    #     if d == float("inf"):
    #         print(f"{i}: unreachable")
    #     else:
    #         print(f"{i}: {d:.2f}")


if __name__ == "__main__":
    main()
