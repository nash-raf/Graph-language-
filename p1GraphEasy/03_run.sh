./GraphProgram > graph.ll test.graph

# Build graph loader object (mmap CSR builder)
clang -O3 -c graph_loader.c -o graph_loader.o

# Link final executable
clang++ -pthread program.o parallel_runtime.o graph_loader.o -o final_program -fopenmp

./final_program


