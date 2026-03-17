# #!/usr/bin/env bash
# set -euxo pipefail

# SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# cd "$SCRIPT_DIR"

# GRAPH_FILE="${GRAPH_FILE:-test.graph}"

# ./GraphProgram "$GRAPH_FILE"

# gcc -O2 -c parallel_runtime.c -o parallel_runtime.o
# g++ -O3 -mavx2 -march=native -c roaring_bitmap.cpp -o roaring_bitmap.o
# g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime.cpp -o graph_loader_runtime.o
# g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o

# g++ program.o parallel_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o -fopenmp -no-pie -o final_program

# ulimit -s unlimited
# time ./final_program


# Prefer user-provided prefix, else local deps, else system
#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

GRAPH_FILE="${GRAPH_FILE:-test.graph}"

# NLOpt: prefer user-provided prefix, else local deps, else system install (libnlopt-dev)
NLOPT_PREFIX="${SGPL_NLOPT_PREFIX:-$SCRIPT_DIR/.deps/nlopt}"

USE_LOCAL_NLOPT=0
if [[ -f "$NLOPT_PREFIX/include/nlopt.h" ]]; then
  USE_LOCAL_NLOPT=1
  NLOPT_INCDIR="$NLOPT_PREFIX/include"
  if [[ -d "$NLOPT_PREFIX/lib64" ]]; then
    NLOPT_LIBDIR="$NLOPT_PREFIX/lib64"
  else
    NLOPT_LIBDIR="$NLOPT_PREFIX/lib"
  fi

  NLOPT_CFLAGS=(-I"$NLOPT_INCDIR")
  NLOPT_LDFLAGS=(-L"$NLOPT_LIBDIR" -Wl,-rpath,"$NLOPT_LIBDIR" -lnlopt)
else
  NLOPT_CFLAGS=()
  NLOPT_LDFLAGS=(-lnlopt)
fi

# 1) Generate program.o (contains the generated code entrypoints)
./GraphProgram "$GRAPH_FILE"

# 2) Build runtimes
gcc -O3 "${NLOPT_CFLAGS[@]}" -c parallel_runtime.c -o parallel_runtime.o
gcc -O3 -c runtime.c -o runtime.o

g++ -O3 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o roaring_bitmap.o
g++ -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o graph_loader_runtime.o
g++ -O2 -std=c++17 -c graph_runtime.cpp -o graph_runtime.o

# 3) Link
g++ -O3 -fopenmp -no-pie \
  program.o runtime.o parallel_runtime.o autotuner_runtime.o graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o \
  "${NLOPT_LDFLAGS[@]}" \
  -o final_program

export OMP_NUM_THREADS="${OMP_NUM_THREADS:-$(nproc)}"
export SGPL_ROARING_THREADS="${SGPL_ROARING_THREADS:-4}"
ulimit -s unlimited

if [[ "$USE_LOCAL_NLOPT" -eq 1 ]]; then
  export LD_LIBRARY_PATH="$NLOPT_LIBDIR:${LD_LIBRARY_PATH:-}"
fi

time ./final_program

