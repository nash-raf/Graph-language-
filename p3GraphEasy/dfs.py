from collections import defaultdict
import sys # Import the sys module

class Graph:
    def __init__(self):
        self.graph = defaultdict(list)
        self.V = 0

    def add_edge(self, u, v):
        """Add undirected edge"""
        self.graph[u].append(v)
        self.graph[v].append(u)

    def build_from_edge_list(self, file_path):
        """Read edge list file and build graph"""
        max_node = -1
        try:
            with open(file_path, "r") as f:
                for line in f:
                    u, v = map(int, line.split())
                    self.add_edge(u, v)
                    max_node = max(max_node, u, v)
            self.V = max_node + 1
        except FileNotFoundError:
            print(f"Error: The file '{file_path}' was not found.")
            sys.exit(1) # Exit if file not found

    def dfs_iterative(self):
        """Universal DFS (iterative with stack)"""
        visited = [False] * self.V
        #print("DFS traversal:")

        for start in range(self.V):
            if not visited[start]:
                stack = [start]
                while stack:
                    v = stack.pop()
                    if not visited[v]:
                        visited[v] = True
                        #print(v, end=" ")
                        for neighbor in reversed(self.graph[v]):
                            if not visited[neighbor]:
                                stack.append(neighbor)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python dfs.py <edge_list_file>")
        sys.exit(1) # Exit if not enough arguments
    
    file_path = sys.argv[1] # Get the file path from the command-line arguments
    g = Graph()
    g.build_from_edge_list(file_path)
    g.dfs_iterative()