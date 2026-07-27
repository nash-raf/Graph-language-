import subprocess
import itertools
import os

# Algorithm name
algo = "dijkstra"

# Vertex and edge lists
vertices = [50000]   
edges = [1000000]
seeds = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]

# Source vertex (you can change this or make it a list)
source_vertex = 0

# Base directory for edgelist files (relative to where script is run)
# Adjust this path based on where you run the script from
# If running from graphit/graphit/build/bin, use: ../../../../synth_graphs_weighted_graphit/
edgelist_base_dir = "synth_graphs_weighted_graphit/"

# Log directory
log_dir = "log/"
os.makedirs(log_dir, exist_ok=True)

def run_experiment(v, e, s):
    # Build edgelist filename (.wel format)
    edgelist_path = f"{edgelist_base_dir}synth_v_{v}_e_{e}_seed_{s}_w.wel"
    
    # Check if file exists
    if not os.path.exists(edgelist_path):
        print(f"Warning: Edgelist file not found for v={v}, e={e}, s={s}")
        print(f"  Expected: {edgelist_path}")
        return
    
    # Build logfile name
    log_filename = f"{log_dir}{algo}_graphit_synth_v_{v}_e_{e}_seed_{s}.txt"
    
    # Build command: ./dijkstra <edgelist_path> <source_vertex>
    cmd = f"./{algo} {edgelist_path} {source_vertex}"
    
    # Run with /usr/bin/time and capture both program output + timing into logfile
    full_cmd = f"/usr/bin/time -p bash -c '{cmd}' > {log_filename} 2>&1"
    
    print(f"Running {algo} on v={v}, e={e}, s={s}, source={source_vertex}")
    print(f"  Edgelist: {edgelist_path}")
    print(f"  Log: {log_filename}")
    
    try:
        subprocess.run(full_cmd, shell=True, check=True)
        print(f"  ✓ Completed\n")
    except subprocess.CalledProcessError as e:
        print(f"  ✗ Failed with exit code {e.returncode}\n")

def main():
    print(f"GraphIt {algo} Benchmark Script")
    print(f"Source vertex: {source_vertex}")
    print(f"Edgelist directory: {edgelist_base_dir}")
    print(f"Log directory: {log_dir}\n")
    
    for v, e, s in itertools.product(vertices, edges, seeds):
        run_experiment(v, e, s)

if __name__ == "__main__":
    main()

