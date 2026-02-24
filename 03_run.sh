./GraphProgram > graph.ll test.graph

# # Rebuild parallel_runtime.o so it includes sleep_runtime, timer_runtime, etc.
# gcc -c -O2 parallel_runtime.c -o parallel_runtime.o

# Use g++ for linking (libgomp is usually installed; clang's -lomp often is not)
# g++ program.o runtime.o roaring_bitmap.o -fopenmp -no-pie -pg -o final_program
# g++ -pthread -fopenmp -O3 -no-pie program.o parallel_runtime.o roaring_bitmap.o runtime11.o -o final_program

clang++ -fopenmp -O3 -no-pie program.o parallel_runtime.o roaring_bitmap.o runtime11.o -o final_program
export OMP_NUM_THREADS=4
LD_LIBRARY_PATH=/usr/lib64 ./final_program
# ./final_program 
