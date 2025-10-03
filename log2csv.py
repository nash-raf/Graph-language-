import os
import re
import csv

# Directory where your log files are stored
log_dir = "../log"

# Algorithms, vertices, edges (same as in your benchmark)
algos = ["bfs", "dfs", "bk"]
vertices = [50000]  # extend as needed
edges = [50000, 200000]  # extend as needed

# Prepare CSV output
csv_file = "benchmark_times.csv"

# Open CSV for writing
with open(csv_file, mode="w", newline="") as f:
    writer = csv.writer(f)

    # Write header
    header = ["Algo/Edges"] + [f"edge={e}" for e in edges]
    writer.writerow(header)

    # Process each algorithm
    for algo in algos:
        row = [algo]
        for e in edges:
            log_filename = f"{log_dir}/{algo}_grapheasy_synth_v_{vertices[0]}_e_{e}.txt"

            if not os.path.exists(log_filename):
                row.append("NA")
                continue

            # Read the log file
            with open(log_filename) as logf:
                content = logf.read()

            # Extract real time using regex
            match = re.search(r"real\s+([\d\.]+)", content)
            if match:
                real_time = match.group(1)
                row.append(real_time)
            else:
                row.append("NA")

        writer.writerow(row)

print(f"CSV file created: {csv_file}")
