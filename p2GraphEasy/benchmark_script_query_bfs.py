
# python3 benchmark_script_2.py ../synth_graphs_unweighted log/

import subprocess
import itertools
import sys
import os
from pathlib import Path

# Algorithms
algos = ["bfs"]

# Vertex and edge lists
vertices = [50000, 250000, 500000, 750000, 1000000]
edges = [50000, 200000, 500000, 750000, 1000000]

# Template for test.graph (uses {graph_folder} and {v}, {e})
graph_template = """graph G{{
    edges: file '{graph_folder}/synth_v_{v}_e_{e}.txt';
}};

query k : "bfs" of G;
"""

def run_experiment(algo, v, e, graph_folder, log_folder):
    # Step 1: Write test2.graph with edges path under graph_folder
    test_graph_content = graph_template.format(
        graph_folder=graph_folder, v=v, e=e, algo=algo
    )
    with open("test2.graph", "w") as f:
        f.write(test_graph_content)

    # Step 2: Log file path (only real time will be written here)
    log_path = Path(log_folder) / f"{algo}_p2grapheasy_synth_v_{v}_e_{e}.txt"

    # Step 3: Run pipeline and capture output (time -p writes to stderr)
    cmd = ["/usr/bin/time", "-p", "bash", "-c", "./2final_run.sh"]
    print(f"Running {algo} on v={v}, e={e} -> log: {log_path}")

    proc = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        cwd=os.getcwd(),
    )

    # Step 4: Parse "real <seconds>" from stderr and write only that to log
    real_time = None
    for line in proc.stderr.splitlines():
        if line.strip().startswith("real"):
            parts = line.split()
            if len(parts) >= 2:
                real_time = parts[1]
            break

    if real_time is None:
        print(f"  Warning: could not find 'real' time for v={v} e={e}", file=sys.stderr)
        if proc.returncode != 0:
            print(proc.stderr, file=sys.stderr)
        return

    log_path.parent.mkdir(parents=True, exist_ok=True)
    with open(log_path, "w") as f:
        f.write(real_time + "\n")

    print(f"  -> real time: {real_time} s")


def main():
    if len(sys.argv) != 3:
        print("Usage: python benchmark_script_2.py <graph_folder> <log_folder>")
        print("  e.g. python benchmark_script_2.py ../synth_graphs_unweighted log/")
        sys.exit(1)

    graph_folder = sys.argv[1].rstrip("/")
    log_folder = sys.argv[2].rstrip("/")

    for algo in algos:
        for v, e in itertools.product(vertices, edges):
            run_experiment(algo, v, e, graph_folder, log_folder)


if __name__ == "__main__":
    main()