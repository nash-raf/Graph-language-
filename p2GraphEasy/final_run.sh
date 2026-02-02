
./GraphProgram test.graph
# g++ program.o runtime.o -fopenmp -lomp -no-pie -o final_program
g++ program.o runtime.o -fopenmp -no-pie -o final_program
./final_program