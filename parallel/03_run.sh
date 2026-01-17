
./GraphProgram > graph.ll test.graph

clang++ -fopenmp=libomp -O3 program.o parallel_runtime.o roaring_bitmap.o -o final_program

export OMP_NUM_THREADS=4
LD_LIBRARY_PATH=/usr/lib64 ./final_program




