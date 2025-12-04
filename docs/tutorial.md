# Tutorial: Using the language

This language will let you describe graphs and load them from a simple edgelist file.


## 1. Loading a graph :

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



## 2. Using built-in functions of the DSL: 

Suppose we want to find the bfs path of the graph `g` we previously had. In order to use the built-in function of the GraphEasy we can simply query it using : 

```
query k : 'bfs' of g;
```

This would perform bfs on the graph g and load the path in k.

If we want to print the path we can do it in the following way.

```
print k;
```


## 3. Using for-each loop in the graph:

Our language provides user to iterate the graph in a simple, intuitive manner using the for-each method.

```
for each vertices v in g{
    
} 
```

## 4. Using if else, while loops:

Using if else and basic while loops is similar to C++ syntax.

```
if(condition){

}
else{

}
```
and while loop is 
```
while(condition){

}
```




