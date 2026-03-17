#!/usr/bin/env bash
set -euxo pipefail

GRAPH_FILE="${GRAPH_FILE:-test.graph}"

# g++ -c -O2 -std=c++17 graph_loader_runtime.cpp -o graph_loader_runtime.o

g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime.cpp -o graph_loader_runtime.o
g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o

./GraphProgram "$GRAPH_FILE"
# ./GraphProgram --debug-polly "$GRAPH_FILE"
# ./GraphProgram "$GRAPH_FILE" -polly-parallel -polly-vectorizer=stripmine

g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o -fopenmp -no-pie -o final_program
export OMP_NUM_THREADS=4
ulimit -s unlimited
time ./final_program
