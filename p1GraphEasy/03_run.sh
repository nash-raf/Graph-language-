
./GraphProgram > graph.ll test.graph

clang++ -pthread program.o parallel_runtime.o -o final_program -fopenmp


./final_program


