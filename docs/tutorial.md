# Tutorial: Loading a graph

This language will let you describe graphs and load them from a simple edgelist file.

## Example edge list format
Suppose we have an 'edgelist.txt' file with the following edegs: 
```
0 1
1 2
1 3
2 4
```

We can define and load  the graph like this:
```
graph g {
    edges: file 'edgelist.txt'; 
};
```

This would tell the language to create a graph `g` and load its edges from the file `edgelist.txt`




