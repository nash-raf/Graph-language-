#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd -P)"
cd "$ROOT"
COMPILER="${SGPL_GRAPH_COMPILER:-$ROOT/build_tdg_validation/GraphProgram_tdg}"
mkdir -p build_tdg_validation
TMP="$(mktemp -d "$ROOT/build_tdg_validation/pdg_argmem_tmp.XXXXXX")"
cleanup() {
  local resolved
  resolved="$(realpath "$TMP")"
  case "$resolved" in
    "$ROOT"/build_tdg_validation/pdg_argmem_tmp.*) ;;
    *) echo "refusing to remove unexpected validation path: $resolved" >&2; return 1 ;;
  esac
  if [[ "${SGPL_KEEP_PDG_ARGMEM_TMP:-0}" == 1 ]]; then
    echo "validation files: $resolved"
  else
    rm -rf -- "$resolved"
  fi
}
trap cleanup EXIT

gcc -O2 -std=gnu11 -pthread -c autotuner_runtime.c -o "$TMP/autotuner_runtime.o"
gcc -O2 -std=gnu11 -pthread -c graph_mutation_runtime.c -o "$TMP/graph_mutation_runtime.o"
gcc -O2 -std=gnu11 -pthread -c parallel_runtime.c -o "$TMP/parallel_runtime.o"
gcc -O2 -std=gnu11 -pthread -c runtime.c -o "$TMP/runtime.o"
gcc -O2 -std=gnu11 -pthread -c gpu_runtime.c -o "$TMP/gpu_runtime.o"
gcc -O2 -std=gnu11 -fopenmp -iquote . -c semiring_runtime.c -o "$TMP/semiring_runtime.o"
g++ -O2 -std=c++17 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o "$TMP/roaring_bitmap.o"
g++ -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o "$TMP/graph_loader_runtime.o"
g++ -O2 -std=c++17 -c graph_runtime.cpp -o "$TMP/graph_runtime.o"
RUNTIME=("$TMP/runtime.o" "$TMP/parallel_runtime.o" "$TMP/autotuner_runtime.o"
         "$TMP/graph_mutation_runtime.o" "$TMP/roaring_bitmap.o"
         "$TMP/graph_loader_runtime.o" "$TMP/graph_runtime.o"
         "$TMP/gpu_runtime.o" "$TMP/semiring_runtime.o")

for name in graph_comprehension_valid graph_comprehension_test; do
  GRAPH_DISABLE_PDG=1 "$COMPILER" --ir-backend=cpu "$name.graph" \
    >"$TMP/$name.serial.compile" 2>"$TMP/$name.serial.err"
  g++ -O2 -fopenmp -no-pie program.o "${RUNTIME[@]}" -ldl -lnlopt \
    -o "$TMP/$name.serial.bin"
  SGPL_NUM_THREADS=1 "$TMP/$name.serial.bin" >"$TMP/$name.serial.out"

  DUMP_LLVM_BC_PRE_PDG="$TMP/$name.pre.bc" \
  DUMP_LLVM_BC_PDG_INPUT="$TMP/$name.pdg.bc" \
    "$COMPILER" --ir-backend=cpu "$name.graph" \
    >"$TMP/$name.parallel.compile" 2>"$TMP/$name.parallel.err"
  opt-20 -passes=verify -disable-output "$TMP/$name.pdg.bc"
  llvm-dis-20 "$TMP/$name.pre.bc" -o "$TMP/$name.pre.ll"
  llvm-dis-20 "$TMP/$name.pdg.bc" -o "$TMP/$name.pdg.ll"
  grep -q 'call.*llvm.memcpy' "$TMP/$name.pre.ll"
  if grep -Eq 'call.*llvm\.(memcpy|memset|memmove)' "$TMP/$name.pdg.ll"; then
    echo "unexpanded memory intrinsic in $name PDG input" >&2
    exit 1
  fi
  grep -q 'load i8' "$TMP/$name.pdg.ll"
  grep -q 'store i8' "$TMP/$name.pdg.ll"
  g++ -O2 -fopenmp -no-pie program.o "${RUNTIME[@]}" -ldl -lnlopt \
    -o "$TMP/$name.parallel.bin"
  SGPL_NUM_THREADS=4 "$TMP/$name.parallel.bin" >"$TMP/$name.parallel.out"
  diff -u "$TMP/$name.serial.out" "$TMP/$name.parallel.out"
  echo "PASS $name: memcpy expanded and output matches serial"
done
