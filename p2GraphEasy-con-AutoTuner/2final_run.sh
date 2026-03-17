#!/usr/bin/env bash
set -euxo pipefail

# g++ -c -O2 -std=c++17 graph_loader_runtime.cpp -o graph_loader_runtime.o

g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime.cpp -o graph_loader_runtime.o

unset GRAPH_DEBUG_DUMP_LL
./GraphProgram test3.graph
# ./GraphProgram --debug-polly test.graph
# ./GraphProgram test.graph -polly-parallel -polly-vectorizer=stripmine

g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime.o -fopenmp -no-pie -o final_program
export OMP_NUM_THREADS=4
ulimit -s unlimited
time ./final_program
