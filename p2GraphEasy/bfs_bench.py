import sys
import time
from collections import defaultdict

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <edgelist.txt> [source=2]", file=sys.stderr)
        sys.exit(1)

    edgefile = sys.argv[1]
    source = int(sys.argv[2]) if len(sys.argv) >= 3 else 2

    # Read edge list
    adj = defaultdict(list)
    max_node = -1
    with open(edgefile) as f:
        for line in f:
            parts = line.split()
            if len(parts) < 2:
                continue
            u, v = int(parts[0]), int(parts[1])
            adj[u].append(v)
            adj[v].append(u)
            max_node = max(max_node, u, v)

    n = max_node + 1

    # BFS (same logic as the DSL code)
    dist = [-1] * n
    t0 = time.perf_counter()

    frontier = {source}
    visited = set()
    level = 0

    while len(frontier) > 0:
        next_frontier = set()
        for v in frontier:
            dist[v] = level
            visited.add(v)
            for u in adj[v]:
                if u not in visited:
                    next_frontier.add(u)
        frontier = next_frontier
        level += 1

    t1 = time.perf_counter()
    elapsed = t1 - t0

    print(f"dist[1] = {dist[1]}")
    print(f"dist[2] = {dist[2]}")
    print(f"dist[3] = {dist[3]}")
    print(f"dist[4] = {dist[4]}")
    print(f"dist[5] = {dist[5]}")
    print(f"dist[6] = {dist[6]}")
    print(f"dist[7] = {dist[7]}")
    print(f"dist[8] = {dist[8]}")
    print(f"dist[9] = {dist[9]}")
    print(f"BFS time: {elapsed:.6f} seconds")
    print(f"Vertices: {n}, Edges: {sum(len(v) for v in adj.values()) // 2}")

if __name__ == "__main__":
    main()
