
./GraphProgram test.graph

g++ -pthread -fopenmp -O3 -no-pie program.o parallel_runtime.o roaring_bitmap.o runtime11.o graph_loader_runtime.o -o final_program
export OMP_NUM_THREADS=4
ulimit -s unlimited

LD_LIBRARY_PATH=/usr/lib64 ./final_program


# ./GraphProgram test2.graph
# g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime.o -fopenmp -no-pie -o final_program

# ulimit -s unlimited
# ./final_program