
./GraphProgram test2.graph
# g++ program.o runtime.o -fopenmp -lomp -no-pie -o final_program
# g++ program.o runtime.o roaring_bitmap.o -fopenmp -no-pie -o final_program
g++ program.o runtime.o roaring_bitmap.o -fopenmp -no-pie -pg -o final_program
./final_program