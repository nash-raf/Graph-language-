import subprocess
import itertools

# Algorithms
algos = ["bfs", "dfs", "bk"]

# Vertex and edge lists
vertices = [50000,250000,500000,750000,1000000]   
edges = [50000,200000,500000,750000,1000000] 

# Template for test.graph
graph_template = """graph g {{

    edges: file '../synth_graphs/synth_v_{v}_e_{e}.txt';

}};

query k: '{algo}' of g; 
"""

def run_experiment(algo, v, e):
    # Step 1: Write test.graph
    test_graph_content = graph_template.format(v=v, e=e, algo=algo)
    with open("test.graph", "w") as f:
        f.write(test_graph_content)

    # Debug: show contents of test.graph
    print("===== test.graph =====")
    print(test_graph_content.strip())
    print("======================")

    # Step 2: Build logfile name
    log_filename = f"../log/{algo}_grapheasy_synth_v_{v}_e_{e}.txt"

    # Step 3: Build LLVM pipeline command
    commands = [
        "./GraphProgram > test.ll test.graph",
        "/usr/bin/llvm-link-18 myprog.bc dfs_runtime.bc bfs_runtime.bc bk_runtime.bc -o linked.bc",
        "opt-18 -O3 linked.bc -o linked.opt.bc",
        "opt -passes=\"function(mem2reg,loop-simplify,loop-rotate,lcssa,indvars,simplifycfg)\" linked.opt.bc -S -o linked-optimized.ll",
        "lli-18 linked-optimized.ll"
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
