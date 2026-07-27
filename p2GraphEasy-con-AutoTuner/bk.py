from collections import defaultdict, deque
import sys

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
            sys.exit(1)

    def _bron_kerbosch_with_pivot(self, R, P, X, all_cliques):
        """
        Recursive implementation of the Bron-Kerbosch algorithm with pivoting.
        
        Args:
            R (set): The set of vertices in the current clique.
            P (set): The set of candidate vertices that can extend the clique.
            X (set): The set of excluded vertices that cannot be used.
            all_cliques (list): A list to store the found maximal cliques.
        """
        if not P and not X:
            # R is a maximal clique
            all_cliques.append(list(R))
            return

        # Choose a pivot vertex from P U X
        if not P and X:
            pivot = next(iter(X))
        elif P and not X:
            pivot = next(iter(P))
        elif P and X:
            pivot = max(P | X, key=lambda v: len(self.graph[v]))
        else:
            return # Should not happen

        # Iterate through P without the neighbors of the pivot
        for v in list(P - set(self.graph[pivot])):
            self._bron_kerbosch_with_pivot(R | {v}, P & set(self.graph[v]), X & set(self.graph[v]), all_cliques)
            P.remove(v)
            X.add(v)

    def find_maximal_cliques(self):
        """
        Public method to find all maximal cliques in the graph.
        
        Returns:
            list: A list of lists, where each inner list represents a maximal clique.
        """
        all_cliques = []
        # Initialize R (current clique), P (candidates), and X (excluded) sets
        R, P, X = set(), set(range(self.V)), set()
        self._bron_kerbosch_with_pivot(R, P, X, all_cliques)


if __name__ == "__main__":
    # Check if a file path was provided as a command-line argument
    if len(sys.argv) < 2:
        print("Usage: python bkwithpivot.py <edge_list_file>")
        sys.exit(1)
    
    # Get the file path from the command-line arguments
    file_path = sys.argv[1]

    g = Graph()
    g.build_from_edge_list(file_path)
    g.find_maximal_cliques()