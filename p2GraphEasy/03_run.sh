
./GraphProgram test.graph

#g++ program.o runtime.o -o final_program -lpthread -lm
clang++-20 program.o runtime.o -o final_program -fopenmp -lpthread -lm -std=c++17


./final_program


