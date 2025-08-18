from collections import defaultdict

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
        with open(file_path, "r") as f:
            for line in f:
                u, v = map(int, line.split())
                self.add_edge(u, v)
                max_node = max(max_node, u, v)
        self.V = max_node + 1

    def dfs_iterative(self):
        """Universal DFS (iterative with stack)"""
        visited = [False] * self.V
        #print("DFS traversal:")

        for start in range(self.V):  # ensures disconnected components are covered
            if not visited[start]:
                stack = [start]
                while stack:
                    v = stack.pop()
                    if not visited[v]:
                        visited[v] = True
                        #print(v, end=" ")
                        # push neighbors in reverse for natural order
                        for neighbor in reversed(self.graph[v]):
                            if not visited[neighbor]:
                                stack.append(neighbor)
                #print()  # new line for each component


if __name__ == "__main__":
    g = Graph()
    g.build_from_edge_list("../bio-ce-cx.txt")
    g.dfs_iterative()