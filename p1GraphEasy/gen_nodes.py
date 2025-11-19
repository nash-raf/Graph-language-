# gen_nodes.py

def find_max_node(edge_file):
    max_node = -1
    with open(edge_file, 'r') as f:
        for line in f:
            if not line.strip():
                continue
            parts = line.strip().split()
            if len(parts) != 2:
                continue  # Skip invalid lines
            u, v = map(int, parts)
            max_node = max(max_node, u, v)
    return max_node

def write_nodes_line(max_node, output_file):
    with open(output_file, 'w') as f:
        nodes_line = "nodes: " + ", ".join(str(i) for i in range(max_node + 1)) + ";"
        f.write(nodes_line + "\n")

if __name__ == "__main__":
    edge_file = "../../sf-20K/scale_free_graph_n20_m_10.txt"
    output_file = "nodes.txt"
    max_node = find_max_node(edge_file)
    if max_node >= 0:
        write_nodes_line(max_node, output_file)
        print(f"'nodes:' line written to {output_file}")
    else:
        print("No valid edges found in the input file.")
