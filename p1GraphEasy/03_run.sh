./GraphProgram > graph.ll test.graph

# Rebuild parallel_runtime.o so it includes sleep_runtime, timer_runtime, etc.
gcc -c -O2 parallel_runtime.c -o parallel_runtime.o

# Use g++ for linking (libgomp is usually installed; clang's -lomp often is not)
g++ -pthread program.o parallel_runtime.o runtime11.o roaring_bitmap.o -o final_program -fopenmp -no-pie
# g++ program.o runtime.o roaring_bitmap.o -fopenmp -no-pie -pg -o final_program


./final_program


