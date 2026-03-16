#!/usr/bin/env bash
set -euo pipefail

GRAPH_INPUT="${1:-test.graph}"

# Keep linked runtime IR in sync with C sources used by GraphProgram.
clang-20 -S -emit-llvm -O2 autotuner_runtime.c -o autotuner_runtime.ll
clang-20 -S -emit-llvm -O2 graph_mutation_runtime.c -o graph_mutation_runtime.ll

# GraphProgram emits a fresh program.o for the chosen DSL workload.
./GraphProgram "$GRAPH_INPUT" >/dev/null 2>&1

# roaring_bitmap.cpp uses AVX2 intrinsics; compile with matching flags.
clang++ -c -O3 -mavx2 -march=native roaring_bitmap.cpp -o roaring_bitmap.o

clang++ -fopenmp=libomp -O3 -L/usr/lib64 -lomp \
	program.o roaring_bitmap.o -o final_program

export OMP_NUM_THREADS="${OMP_NUM_THREADS:-4}"
ulimit -s unlimited || true
./final_program >/dev/null 2>&1




