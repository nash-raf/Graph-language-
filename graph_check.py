import argparse, os ,sys
import networkx as nx

"""
Command: 
python Graph_check.py --i sample.txt --o sample_converted.txt [--s file_of_communities_whose_union_induced_subgraph_will_be_reported]
"""

parser = argparse.ArgumentParser()
parser.add_argument('--i', type=str, default=None, help='which graph file need to check')
parser.add_argument('--o', type=str, default=None, help='edgelist output file')
parser.add_argument('--s', type=str, default="", help='filename containing nodes to output subgraph induced by union of those nodes instead of the whole graph')
args = parser.parse_args()

G = nx.Graph()
f = open(args.i, 'r')
i=0
for line in f:
    vertex = line.rstrip().split()
    G.add_edge(vertex[0], vertex[1])
    i=i+1
print('Edges:',i)
f.close()

print('Edges after removing multi-edges:',len(G.edges()))
print('Nodes:',len(list(G.nodes)))
print('Connected components:',nx.number_connected_components(G))


i=0
#print('Self loops are:')
#se=nx.selfloop_edges(G)
#for edge in se:
#    i=i+1
#    print(edge)
for u, v in G.edges():
    if u == v: 
        G.remove_edge(u,v)
        i = i+1
#G.remove_edges_from(se)
print('Total no. of self loops:',i)
print('Edges after removing self loops:', len(G.edges()))

# largest_component = max(nx.connected_components(G), key=len)
# S = G.subgraph(largest_component).copy()
# print('Edges for the most connected component:',len(S.edges()))

if len(args.s) > 0:
    nodes = set()
    with open(args.s) as file:
        for line in file:
            words = line.rstrip().split()
            if len(words) > 5:
                nodes.update(words)
            #print(nodes)
    #G.remove_nodes_from([n for n in G if n not in nodes])
    sub = G.subgraph(nodes).copy()
    print('Nodes and Edges in subgraph:', len(sub.nodes()), ", ", len(sub.edges()))
    # write edgelist to grid.edgelist
    nx.write_edgelist(G, path=args.o, delimiter=' ', data=False)
else:
	nx.write_edgelist(G, path=args.o, delimiter=' ', data=False)

#node_list = list(G.nodes())
#node_dict = {node_list[i] :i for i in range(len(node_list))}
#f = open(args.o, 'w')
#f.write('\n'.join('{} {}'.format(node_dict[x[0]], node_dict[x[1]]) for x in G.edges()))
#f.close()

print("Output file is saved in",args.o)