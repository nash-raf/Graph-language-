
./GraphProgram test.graph

g++ -pthread -fopenmp -O3 -no-pie program.o parallel_runtime.o roaring_bitmap.o runtime11.o -o final_program
export OMP_NUM_THREADS=4
LD_LIBRARY_PATH=/usr/lib64 ./final_program

