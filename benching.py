import os
import re
import matplotlib.pyplot as plt
import numpy as np

log_dir = "log"      # p2grapheasy_bfs_call
log2_dir = "log2"    # p2grapheasy_bfs

pattern = re.compile(r"bfs_p2grapheasy_synth_v_(\d+)_e_(\d+)\.txt")

def parse_real_time(filepath):
    with open(filepath) as f:
        for line in f:
            if line.startswith("real"):
                return float(line.split()[1])
    return None

def parse_bfs_time(filepath):
    with open(filepath) as f:
        for line in f:
            m = re.search(r"Completed in ([\d.]+) seconds", line)
            if m:
                return float(m.group(1))
    return None

rows = []
for fname in sorted(os.listdir(log_dir)):
    m = pattern.match(fname)
    if not m:
        continue
    v, e = int(m.group(1)), int(m.group(2))
    
    log_path = os.path.join(log_dir, fname)
    log2_path = os.path.join(log2_dir, fname)
    
    if not os.path.exists(log2_path):
        continue
    
    call_real = parse_real_time(log_path)
    call_bfs = parse_bfs_time(log_path)
    dsl_real = parse_real_time(log2_path)
    
    rows.append((v, e, call_real, call_bfs, dsl_real))

rows.sort(key=lambda r: (r[0], r[1]))

vertex_counts = sorted(set(r[0] for r in rows))

fig, axes = plt.subplots(1, len(vertex_counts), figsize=(5 * len(vertex_counts), 5), sharey=True)
if len(vertex_counts) == 1:
    axes = [axes]

for ax, vc in zip(axes, vertex_counts):
    subset = [r for r in rows if r[0] == vc]
    edges = [r[1] for r in subset]
    call_real = [r[2] for r in subset]
    dsl_real = [r[4] for r in subset]
    
    x = np.arange(len(edges))
    width = 0.35
    
    labels = [f"{e//1000}K" for e in edges]
    
    ax.bar(x - width/2, call_real, width, label="BFS Call (query runtime)", color="yellow")
    ax.bar(x + width/2, dsl_real, width, label="BFS DsL implement", color="black")
    
    # ax.bar(x - width/2, call_real, width, label="BFS Call (query runtime)", color="#4C72B0")
    # ax.bar(x + width/2, dsl_real, width, label="BFS DsL implement", color="#DD8452")
    
    ax.set_xlabel("Edges")
    ax.set_ylabel("time (s)")
    ax.set_title(f"V = {vc:,}")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=45)
    ax.legend(fontsize=8)

fig.suptitle("BFS: query call vs DSL set-based (wall-clock time)", fontsize=14, y=1.02)
plt.tight_layout()
plt.savefig("bfs_comparison.png", dpi=150, bbox_inches="tight")
plt.show()
print("Saved bfs_comparison.png")