from collections import deque, defaultdict

def load_graph(filename):
    """Load graph from a file into an adjacency list."""
    graph = defaultdict(list)
    with open(filename, "r") as f:
        for line in f:
            u, v = map(int, line.split())
            graph[u].append(v)
            graph[v].append(u)  # comment this if the graph is directed
    return graph

def bfs(graph, start):
    """Perform BFS and return parent dictionary."""
    visited = set()
    parent = {start: None}
    queue = deque([start])
    
    while queue:
        node = queue.popleft()
        visited.add(node)
        for neighbor in graph[node]:
            if neighbor not in visited and neighbor not in queue:
                parent[neighbor] = node
                queue.append(neighbor)
    return parent

if __name__ == "__main__":
    graph_file = "g.txt"  # your graph file
    source_node = 0        # change to desired starting node
    graph = load_graph(graph_file)
    parents = bfs(graph, source_node)
    
   
