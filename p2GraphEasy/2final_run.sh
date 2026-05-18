#!/usr/bin/env bash
set -euo pipefail

GRAPH_FILE="${GRAPH_FILE:-test.graph}"
GRAPH_PROGRAM="${GRAPH_PROGRAM:-./GraphProgram}"

if [[ ! -x "$GRAPH_PROGRAM" && -x ./GraphProgram_mmap ]]; then
  GRAPH_PROGRAM="./GraphProgram_mmap"
fi

g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime_mmap.cpp -o graph_loader_runtime_mmap.o
g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o

"$GRAPH_PROGRAM" "$GRAPH_FILE" >/dev/null

g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime_mmap.o graph_runtime.o -fopenmp -no-pie -o final_program_mmap
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-4}"
ulimit -s unlimited
time ./final_program_mmap
