#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d)"
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
  gcc -O2 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
for source in roaring_bitmap graph_loader_runtime graph_runtime; do
  g++ -O2 -mavx2 -fopenmp -c "$source.cpp" -o "$TMP/$source.o"
done
OBJECTS=("$TMP"/*.o)
for backend in sequential cpu gpu; do
  if [[ "$backend" == sequential ]]; then
    env FORCE_CPU=1 GRAPH_DISABLE_PDG=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram test/array_frontier_order.graph >"$TMP/$backend.compile" 2>&1
  elif [[ "$backend" == cpu ]]; then
    env FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram test/array_frontier_order.graph >"$TMP/$backend.compile" 2>&1
  else
    env FORCE_CPU=0 FORCE_GPU=1 GRAPH_FRONTIER_STATS=1 SGPL_GPU_DEBUG=1 ./GraphProgram test/array_frontier_order.graph >"$TMP/$backend.compile" 2>&1
  fi
  cp program.o "$TMP/$backend.program"
  ! nm -u program.o | grep -q 'autograph_edgemap'
  grep -q 'bound=sum(degree(frontier\[i\]))\*2' "$TMP/$backend.compile"
  if [[ "$backend" != sequential ]]; then
    grep -q 'status=independent-family group=Destination.*runtime=1 realized=1' "$TMP/$backend.compile"
  fi
  g++ -fopenmp -no-pie "$TMP/$backend.program" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/$backend"
done
SGPL_NUM_THREADS=1 "$TMP/sequential" >"$TMP/reference" 2>"$TMP/reference.log"
for backend in sequential cpu gpu; do
  for workers in 1 2 4 8; do
    for fallback in 0 1; do
      if ((fallback)); then export SGPL_FRONTIER_TEST_UNDERRESERVE=1; else unset SGPL_FRONTIER_TEST_UNDERRESERVE; fi
      env SGPL_NUM_THREADS="$workers" SGPL_CLEANCUT_PARTITIONS=3 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_DEBUG=1 SGPL_FRONTIER_DEBUG=1 \
        "$TMP/$backend" >"$TMP/output" 2>"$TMP/run.log"
      cmp "$TMP/reference" "$TMP/output"
      if ((fallback)); then
        grep -q '\[frontier-buffer\] fallback-grow' "$TMP/run.log"
        ! grep -q '\[frontier-buffer\] reserve' "$TMP/run.log"
      else
        grep -q '\[frontier-buffer\] reserve' "$TMP/run.log"
        ! grep -q '\[frontier-buffer\] fallback-grow' "$TMP/run.log"
      fi
      if [[ "$backend" == gpu ]]; then grep -q '\[gpu-task\]' "$TMP/run.log"; fi
    done
  done
done
unset SGPL_FRONTIER_TEST_UNDERRESERVE
env FORCE_CPU=1 GRAPH_DISABLE_PDG=1 ./GraphProgram test/array_frontier_unrolled.graph >"$TMP/unrolled.compile" 2>&1
cp program.o "$TMP/unrolled.program"
g++ -fopenmp -no-pie "$TMP/unrolled.program" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/unrolled"
env SGPL_FRONTIER_DEBUG=1 "$TMP/unrolled" >"$TMP/unrolled.output" 2>"$TMP/unrolled.log"
printf '14\n1\n1\n2\n2\n2\n2\n1\n1\n2\n2\n2\n2\n2\n2\n' >"$TMP/unrolled.expected"
cmp "$TMP/unrolled.expected" "$TMP/unrolled.output"
grep -q '\[frontier-buffer\] fallback-grow' "$TMP/unrolled.log"
echo 'PASS append guards survive complete source-loop unrolling'
echo 'PASS automatic array-frontier proof, exact append order and duplicates, checked copy/growth, and real GPU execution at 1,2,4,8 workers'
echo "Array frontier lowering logs: $TMP"
