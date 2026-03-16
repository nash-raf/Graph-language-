
# ./GraphProgram test.graph

# g++ -pthread -fopenmp -O3 -no-pie program.o parallel_runtime.o roaring_bitmap.o runtime11.o graph_loader_runtime.o -o final_program
# export OMP_NUM_THREADS=4
# ulimit -s unlimited

# LD_LIBRARY_PATH=/usr/lib64 ./final_program


#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# ./GraphProgram test.graph > graph.ll
# # clang++ -O3 -c -x ir graph.ll -o program.o
# gcc -O2 -c parallel_runtime.c -o parallel_runtime.o
# g++ -O3 -mavx2 -march=native -c roaring_bitmap.cpp -o roaring_bitmap.o
# g++ -pthread -fopenmp -O3 -no-pie program.o parallel_runtime.o roaring_bitmap.o runtime11.o graph_loader_runtime.o -o final_program

# export OMP_NUM_THREADS="${OMP_NUM_THREADS:-$(nproc)}"
# ulimit -s unlimited
# LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-/usr/lib64}" ./final_program


./GraphProgram test.graph
g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime.o -fopenmp -no-pie -o final_program
ulimit -s unlimited
time ./final_program