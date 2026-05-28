#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
if [[ ! -x ./GraphProgram ]]; then
  echo "Build GraphProgram first: ./02_run.sh test.graph test.graph" >&2
  exit 1
fi
./GraphProgram test/smoke_pagerank_delta.graph >/dev/null
gcc -O3 -c autotuner_runtime.c -o autotuner_runtime.o
gcc -O3 -c graph_mutation_runtime.c -o graph_mutation_runtime.o
gcc -O3 -c parallel_runtime.c -o parallel_runtime.o
gcc -O3 -c runtime.c -o runtime.o
g++ -O3 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o roaring_bitmap.o
g++ -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o graph_loader_runtime.o
g++ -O2 -std=c++17 -c graph_runtime.cpp -o graph_runtime.o
g++ -O3 -fopenmp -no-pie program.o runtime.o parallel_runtime.o autotuner_runtime.o graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o -lnlopt -o final_program
mapfile -t got < <(./final_program | rg '^0\.[0-9]+$' || true)
want=(0.200000 0.200000 0.200000 0.200000 0.200000)
if [[ "${#got[@]}" -ne 5 ]]; then echo "FAIL pagerank length ${#got[@]}"; exit 1; fi
for i in "${!want[@]}"; do
  if [[ "${got[$i]}" != "${want[$i]}" ]]; then echo "FAIL pagerank got=${got[*]} want=${want[*]}"; exit 1; fi
done
echo "PASS pagerank_delta: ${got[*]}"
