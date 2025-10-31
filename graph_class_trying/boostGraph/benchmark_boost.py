import subprocess
import itertools

# Algorithms
algos = ["bfs", "dfs"]

# Vertex and edge lists
vertices = [50000]   
edges = [1000000]  
seeds = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]  # Different seeds for different graphs

def run_experiment(algo, v, e, s):
    # Step 2: Build logfile name
    log_filename = f"../../log/{algo}_BGL_synth_v_{v}_e_{e}_seed_{s}.txt"

    # Step 3: Build LLVM pipeline command
    # Corrected line, matching the file names in your image:
    commands = [
        f"./{algo} ../../synth_graphs_violin/synth_v_{v}_e_{e}_seed_{s}.txt"
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
