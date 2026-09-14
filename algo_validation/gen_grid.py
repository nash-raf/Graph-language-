#!/usr/bin/env python3
"""2D grid edge list -- high diameter, the shape that makes frontier loops iterate."""
import numpy as np, networkx as nx
R=C=158
G=nx.grid_2d_graph(R,C); G=nx.convert_node_labels_to_integers(G,ordering="sorted")
rng=np.random.default_rng(42)
E=sorted(map(lambda e:(min(e),max(e)),G.edges())); W=rng.integers(1,101,size=len(E))
base="/home/user/Course/msk1/2nd/Graph-language-/algo_validation/ba/grid_%d_%d"%(R*C,len(E))
with open(base+".txt","w") as fu, open(base+"_w.txt","w") as fw:
    for (a,b),w in zip(E,W):
        fu.write("%d %d\n"%(a,b)); fw.write("%d %d %d\n"%(a,b,w))
core=nx.core_number(G); deg=np.array([d for _,d in G.degree()])
print("GRID %dx%d n=%d edges=%d avg_deg=%.2f max_deg=%d"%(R,C,R*C,len(E),deg.mean(),deg.max()))
print("  degeneracy=%d  eccentricity(from 0)=%d"%(max(core.values()),nx.eccentricity(G,v=0)))
print("written:",base+".txt")
