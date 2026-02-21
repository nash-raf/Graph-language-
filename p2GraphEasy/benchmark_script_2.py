import subprocess
import itertools

# Algorithms
algos = ["bfs"]

# Vertex and edge lists
vertices = [50000,250000,500000,750000,1000000]   
edges = [50000,200000,500000,750000,1000000]

# Template for test.graph
graph_template = graph_template = """graph G {{

    edges: file '../synth_graphs_unweighted/synth_v_{v}_e_{e}.txt';

}};
int n = numVertices(G);
int dist[n];
set visited;
set frontier = {{0}};
int level = 0;
while (frontier.size() > 0) {{
    set next_frontier = {{}};
    for each v in frontier {{
        dist[v] = level;
        visited.add(v);
        for each neighbor u of v in G {{
            if (!visited.contains(u)) {{
                next_frontier.add(u);
            }}
        }}
    }}
    frontier = next_frontier;
    level = level + 1;
}}
"""


def run_experiment(algo, v, e):
    # Step 1: Write test.graph
    test_graph_content = graph_template.format(v=v, e=e, algo=algo)
    with open("test2.graph", "w") as f:
        f.write(test_graph_content)

    # Debug: show contents of test.graph
    print("===== test.graph =====")
    print(test_graph_content.strip())
    print("======================")

    # Step 2: Build logfile name
    log_filename = f"../log2/{algo}_p2grapheasy_synth_v_{v}_e_{e}.txt"

    # Step 3: Build LLVM pipeline command
    commands = [
        "./final_run.sh"
    ]
    cmd = " && ".join(commands)

    # Step 4: Run pipeline with /usr/bin/time and capture both program + time into logfile
    full_cmd = f"/usr/bin/time -p bash -c '{cmd}' > {log_filename} 2>&1"

    print(f"Running {algo} on v={v}, e={e} -> log: {log_filename}")
    subprocess.run(full_cmd, shell=True, check=True)

def main():
    for algo in algos:
        for v, e in itertools.product(vertices, edges):
            run_experiment(algo, v, e)

if __name__ == "__main__":
    main()
