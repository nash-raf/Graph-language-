#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "round/spatial compiler logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
 gcc -O2 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
g++ -O2 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o "$TMP/roaring_bitmap.o"
g++ -O2 -fopenmp -c graph_loader_runtime.cpp -o "$TMP/graph_loader_runtime.o"
g++ -O2 -c graph_runtime.cpp -o "$TMP/graph_runtime.o"
OBJECTS=("$TMP"/*.o)
for fixture in round_spatial_fold round_spatial_guard round_spatial_twofold; do
 FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 SGPL_FRONTIER_STRICT=1 ./GraphProgram "test/$fixture.graph" >"$TMP/$fixture.compile" 2>&1
 if [[ "$fixture" == round_spatial_twofold ]]; then
  grep -q 'schedule=round-dag' "$TMP/$fixture.compile"
 else
  grep -q 'schedule=round-spatial-dag' "$TMP/$fixture.compile"
 fi
 mv program.o "$TMP/$fixture.o"
 g++ -fopenmp -no-pie "$TMP/$fixture.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/$fixture"
 FORCE_CPU=1 GRAPH_FRONTIER_REWRITE_OFF=1 ./GraphProgram "test/$fixture.graph" >"$TMP/reference.compile" 2>&1
 mv program.o "$TMP/reference.o"
 g++ -fopenmp -no-pie "$TMP/reference.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/reference"
 reference="$(SGPL_NUM_THREADS=1 "$TMP/reference")"
 for workers in 1 2 4 8; do
  for repeat in 1 2 3; do
   output="$(timeout 30 env SGPL_NUM_THREADS="$workers" SGPL_CLEANCUT_PARTITIONS=3 "$TMP/$fixture")"
   [[ "$output" == "$reference" ]] || { echo "FAIL $fixture workers=$workers: $output != $reference"; exit 1; }
  done
 done
 echo "PASS $fixture (selected schedule, outputs, final induction, workers=1,2,4,8)"
done

FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram test/round_spatial_overlap.graph >"$TMP/overlap.compile" 2>&1
grep -q 'schedule=round-spatial-dag' "$TMP/overlap.compile"
mv program.o "$TMP/overlap.o"
gcc -O2 -std=gnu11 -iquote . -c test/round_spatial_generated_probe.c -o "$TMP/probe.o"
g++ -fopenmp -no-pie "$TMP/overlap.o" "$TMP/probe.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt \
 -Wl,--wrap=sgpl_exec_dag_round_spatial -Wl,--wrap=autograph_build_clean_cut -Wl,--wrap=sgpl_exec_spatial_partition -o "$TMP/overlap"
for workers in 1 2 4 8; do
 timeout 15 env SGPL_NUM_THREADS="$workers" SGPL_CLEANCUT_PARTITIONS=2 "$TMP/overlap" >"$TMP/overlap.out" 2>"$TMP/overlap.log" || { cat "$TMP/overlap.log"; exit 1; }
 [[ "$(cat "$TMP/overlap.out")" == $'348\n2' ]] || { cat "$TMP/overlap.out"; exit 1; }
 grep 'generated joint overlap:' "$TMP/overlap.log"
done
