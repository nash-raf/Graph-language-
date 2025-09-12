import os
import re
import csv

LOG_DIR = "../log"
OUTPUT_DIR = "../log/csv"

# Ensure output directory exists
os.makedirs(OUTPUT_DIR, exist_ok=True)

def extract_user_time(filepath):
    """Extract only the 'user' time from a log file."""
    with open(filepath, "r") as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) == 2 and parts[0] == "user":
                return float(parts[1])
    return None

def main():
    # algo -> list of rows
    algo_data = {}

    for filename in os.listdir(LOG_DIR):
        if not filename.endswith(".txt"):
            continue

        filepath = os.path.join(LOG_DIR, filename)

        # Example filenames:
        #   bfs_python_synth_v_50000_e_50000.txt
        #   dfs_graph_tool_synth_v_250000_e_200000.txt
        match = re.search(r"(\w+)_(\w+)_synth_v_(\d+)_e_(\d+)", filename)
        if not match:
            continue

        algo, impl, v, e = match.groups()
        v, e = int(v), int(e)  # convert for sorting
        user_time = extract_user_time(filepath)
        if user_time is None:
            continue

        row = [impl, v, e, user_time]

        if algo not in algo_data:
            algo_data[algo] = []
        algo_data[algo].append(row)

    # Write one CSV per algo
    for algo, rows in algo_data.items():
        # Sort by Vertices then Edges
        rows.sort(key=lambda x: (x[1], x[2]))

        out_file = os.path.join(OUTPUT_DIR, f"{algo}.csv")
        with open(out_file, "w", newline="") as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["Implementation", "Vertices", "Edges", "UserTime"])
            writer.writerows(rows)

        print(f"✅ Saved {out_file}")

if __name__ == "__main__":
    main()