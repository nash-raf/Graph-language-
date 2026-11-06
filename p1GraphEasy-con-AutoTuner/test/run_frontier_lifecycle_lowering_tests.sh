#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "lifecycle lowering artifacts: $TMP"; else rm -rf "$TMP"; fi' EXIT
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
  gcc -O2 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
for source in roaring_bitmap graph_loader_runtime graph_runtime; do
  g++ -O2 -mavx2 -fopenmp -c "$source.cpp" -o "$TMP/$source.o"
done
OBJECTS=("$TMP"/*.o)
for source in small_bfs small_cc small_kcore small_sssp test/array_frontier_order test/nonfrontier_append; do
  tag="${source##*/}"
  for mode in sequential cpu; do
    flags=(FORCE_CPU=1 GRAPH_FRONTIER_STATS=1)
    [[ "$mode" == sequential ]] && flags+=(GRAPH_DISABLE_PDG=1)
    env "${flags[@]}" ./GraphProgram "$source.graph" >"$TMP/$tag.$mode.compile" 2>&1
    if [[ "$mode" == cpu ]]; then
      if [[ "$tag" == nonfrontier_append ]]; then
        [[ $(grep -c 'sequential: unproven frontier lifecycle' "$TMP/$tag.$mode.compile") -eq 2 ]]
        ! grep -q 'Activate(' "$TMP/$tag.$mode.compile"
      else
        grep -q 'Activate(' "$TMP/$tag.$mode.compile"
        ! grep -q 'unproven frontier lifecycle' "$TMP/$tag.$mode.compile"
      fi
    fi
    g++ -fopenmp -no-pie program.o "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/$tag.$mode"
  done
  SGPL_NUM_THREADS=1 "$TMP/$tag.sequential" >"$TMP/$tag.expected" 2>"$TMP/$tag.reference.log"
  for workers in 1 4; do
    SGPL_NUM_THREADS="$workers" "$TMP/$tag.cpu" >"$TMP/$tag.actual" 2>"$TMP/$tag.run.log"
    cmp "$TMP/$tag.expected" "$TMP/$tag.actual"
  done
  echo "PASS $tag: recognition and output at 1 and 4 workers"
done
