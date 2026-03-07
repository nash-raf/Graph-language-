#!/usr/bin/env bash
set -euxo pipefail

# g++ -c -O2 -std=c++17 graph_loader_runtime.cpp -o graph_loader_runtime.o
./GraphProgram test.graph
g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime.o -fopenmp -no-pie -o final_program

ulimit -s unlimited
time ./final_program