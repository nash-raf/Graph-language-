% bfs_test.m
edges = load('g.txt');  % read edge list

s = edges(:,1);
t = edges(:,2);

offset = 1;
s = s + offset;
t = t + offset;

G = graph(s, t);

start_node = 0 + offset;
bfs_order = bfsearch(G, start_node);

disp('BFS order from node 0:');
disp(bfs_order - offset);
