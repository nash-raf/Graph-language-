./GraphProgram > graph.ll test.graph

# Rebuild parallel_runtime.o so it includes sleep_runtime, timer_runtime, etc.
gcc -c -O2 parallel_runtime.c -o parallel_runtime.o

# Use g++ for linking (libgomp is usually installed; clang's -lomp often is not)
g++ -pthread program.o parallel_runtime.o -o final_program -fopenmp -no-pie


./final_program


