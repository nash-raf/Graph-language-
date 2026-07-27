from collections import defaultdict, deque
import sys # Import the sys module to handle command-line arguments

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
            sys.exit(1) # Exit the program if the file doesn't exist

    def bfs_iterative(self):
        """Universal BFS (iterative with queue)"""
        visited = [False] * self.V
        #print("BFS traversal:")

        for start in range(self.V):  # ensures disconnected components are covered
            if not visited[start]:
                queue = deque([start])
                visited[start] = True
                while queue:
                    v = queue.popleft()
                    for neighbor in self.graph[v]:
                        if not visited[neighbor]:
                            visited[neighbor] = True
                            queue.append(neighbor)
                

if __name__ == "__main__":
    # Check if a file path was provided as a command-line argument
    if len(sys.argv) < 2:
        print("Usage: python bfs.py <edge_list_file>")
        sys.exit(1) # Exit if no file is provided
    
    # Get the file path from the command-line arguments (sys.argv[1])
    file_path = sys.argv[1]

    g = Graph()
    g.build_from_edge_list(file_path)
    g.bfs_iterative()