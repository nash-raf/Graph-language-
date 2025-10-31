import subprocess
import itertools

# Algorithms
algos = ["bfs", "dfs"]

# Vertex and edge lists
real_graphs = [
    "bio-grid-yeast.txt",
    "fb-pages-company.txt",
    "ia-dbpedia.txt"
] # Different seeds for different graphs

def run_experiment(algo, graph):

    # Step 2: Build logfile name
    log_filename = f"../../log/{algo}_graphskew_synth_{graph}"

    commands = [
        f"python {algo}_graph_tool.py ../../real_graphs/{graph} 0"
    ]
    cmd = " && ".join(commands)

    # Step 4: Run pipeline with /usr/bin/time and capture both program + time into logfile
    full_cmd = f"/usr/bin/time -p bash -c '{cmd}' > {log_filename} 2>&1"

    print(f"Running {algo} on {graph} -> log: {log_filename}")
    subprocess.run(full_cmd, shell=True, check=True)

def main():
    for algo in algos:
        for graph in real_graphs:
            run_experiment(algo, graph)

if __name__ == "__main__":
    main()
