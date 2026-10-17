#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)";cd "$ROOT"
TMP="$(mktemp -d)"
trap 'status=$?; if ((status));then echo "program array artifacts: $TMP";else rm -rf "$TMP";fi' EXIT
ulimit -c 0
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime;do gcc -O2 -fopenmp -pthread -iquote . -c "$source.c" -o "$TMP/$source.o";done
for source in roaring_bitmap graph_loader_runtime graph_runtime;do g++ -O2 -mavx2 -fopenmp -pthread -c "$source.cpp" -o "$TMP/$source.o";done
OBJECTS=("$TMP"/*.o)
printf '0\n0\n0\n0\n8\n7\n0\n0\n0\n0\n0\n' >"$TMP/zero.expected"
printf '0\n1\n1\n2\n8\n10\n12\n' >"$TMP/snapshot.expected"
for source in zero_program_array array_program_snapshot;do
 FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram --ir-backend=cpu "test/$source.graph" >"$TMP/$source.compile.log" 2>&1
 cp program.o "$TMP/$source.o"
 if [[ "$source" == array_program_snapshot ]];then
  nm -u program.o | grep -q autograph_exec_ctx_bind_source_snapshot_range
  nm -u program.o | grep -q autograph_exec_ctx_bind_array_frontier
  grep -q 'runtime=1 realized=1' "$TMP/$source.compile.log"
 fi
 g++ -fopenmp -no-pie "$TMP/$source.o" "${OBJECTS[@]}" -ldl -lnlopt -o "$TMP/$source"
 for workers in 1 4 32;do
  SGPL_NUM_THREADS=$workers SGPL_ALLOW_HT=1 "$TMP/$source" >"$TMP/output" 2>"$TMP/run.log"
  expected=zero;[[ "$source" == array_program_snapshot ]] && expected=snapshot
  cmp "$TMP/$expected.expected" "$TMP/output"
 done
done
cat >"$TMP/fail.c" <<'EOF'
#include <stddef.h>
void *__real_calloc(size_t,size_t);
void *__wrap_calloc(size_t n,size_t z){return n*z>1048576?0:__real_calloc(n,z);}
EOF
gcc -c "$TMP/fail.c" -o "$TMP/fail.o"
g++ -fopenmp -no-pie "$TMP/zero_program_array.o" "$TMP/fail.o" "${OBJECTS[@]}" -Wl,--wrap=calloc -ldl -lnlopt -o "$TMP/failure"
python3 - "$TMP/failure" <<'PY'
import subprocess,sys
r=subprocess.run([sys.argv[1]],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
assert r.returncode==-4,(r.returncode,r.stdout,r.stderr)
PY
echo 'PASS program-array zeros, swaps, nested lifetime, allocation failure, and snapshot spatial execution at 1/4/32 workers'
