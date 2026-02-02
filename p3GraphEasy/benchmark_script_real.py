import subprocess
import os

# Algorithms to test
algos = ["dijkstra"]

# List of real graph names (without path, assuming they exist in ../real_graphs/)
real_graphs = [
    "bio-CE-CX.txt",
    "bio-HS-CX.txt"
]

# "bio-grid-yeast.txt",
#     "fb-pages-company.txt",
#     "ia-dbpedia.txt"

# Template for test.graph file
graph_template = """graph g {{
    edges: file '../real_graphs/{graph_name}';
    TRUE
}};

query k: '{algo}' of g;
"""

def run_experiment(algo, graph_name):
    # Step 1: Write test.graph
    test_graph_content = graph_template.format(graph_name=graph_name, algo=algo)
    with open("test.graph", "w") as f:
        f.write(test_graph_content)

    # Debug print
    print("===== test.graph =====")
    print(test_graph_content.strip())
    print("======================")

    # Ensure log directory exists
    os.makedirs("../log", exist_ok=True)

    # Step 2: Build logfile name
    log_filename = f"../log/{algo}_p1grapheasy_real_{os.path.splitext(graph_name)[0]}.txt"

    # Step 3: Build and run the pipeline command
    cmd = "./final_run.sh"
    full_cmd = f"/usr/bin/time -p bash -c '{cmd}' > {log_filename} 2>&1"

    print(f"Running {algo} on graph {graph_name} -> log: {log_filename}")
    subprocess.run(full_cmd, shell=True, check=True)

def main():
    for algo in algos:
        for graph_name in real_graphs:
            run_experiment(algo, graph_name)

if __name__ == "__main__":
    main()
