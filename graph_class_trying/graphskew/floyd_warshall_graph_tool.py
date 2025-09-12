import sys, time
from graph_tool.all import Graph, shortest_distance
from graph_tool import topology

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <edge_list_file>")
        sys.exit(1)

    file_path = sys.argv[1]

    # Create directed graph
    g = Graph(directed=True)

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

    start_time = time.time()
    shortest_distance(g, source=None, target=None, weights=None, negative_weights=False, max_dist=None, directed=None, dense=True, dist_map=None, pred_map=False, return_reached=False, dag=False)
    end_time = time.time()

    print(f"Shortest paths computed in {end_time - start_time:.4f} seconds")

if __name__ == "__main__":
    main()
