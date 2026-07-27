import subprocess
import itertools

# Algorithms
algos = ["dijkstra"]

# Vertex and edge lists
vertices = [50000]   
edges = [1000000]
seeds = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]  

# Template for test.graph
graph_template = """graph g {{

    edges: file '../synth_graphs_weighted/synth_v_{v}_e_{e}_seed_{s}_w.txt';

}};

query k: '{algo}' of g; 
"""

def run_experiment(algo, v, e, s):
    # Step 1: Write test.graph
    test_graph_content = graph_template.format(v=v, e=e, algo=algo, s=s)
    with open("test.graph", "w") as f:
        f.write(test_graph_content)

    # Debug: show contents of test.graph
    print("===== test.graph =====")
    print(test_graph_content.strip())
    print("======================")

    # Step 2: Build logfile name
    log_filename = f"../log/{algo}_p1grapheasy_synth_v_{v}_e_{e}_seed_{s}.txt"

    # Step 3: Build LLVM pipeline command
    commands = [
        "./final_run.sh"
    ]
    cmd = " && ".join(commands)

    # Step 4: Run pipeline with /usr/bin/time and capture both program + time into logfile
    full_cmd = f"/usr/bin/time -p bash -c '{cmd}' > {log_filename} 2>&1"

    print(f"Running {algo} on v={v}, e={e}, s={s} -> log: {log_filename}")
    subprocess.run(full_cmd, shell=True, check=True)

def main():
    for algo in algos:
        for v, e, s in itertools.product(vertices, edges, seeds):
            run_experiment(algo, v, e, s)

if __name__ == "__main__":
    main()
