import subprocess
import itertools

# Algorithms
algos = ["bfs", "dfs", "bk"]

# Vertex and edge lists
vertices = [50000,250000,500000,750000,1000000]   
edges = [50000,200000,500000,750000,1000000] 



def run_experiment(algo, v, e):

    # Step 2: Build logfile name
    log_filename = f"../log/{algo}_python_synth_v_{v}_e_{e}.txt"

    commands = [
        f"python {algo}.py ../synth_graphs/synth_v_{v}_e_{e}.txt"
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
