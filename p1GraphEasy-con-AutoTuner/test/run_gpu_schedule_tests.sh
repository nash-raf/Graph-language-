#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
export SGPL_GPU_MIN_TRIPS=0
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "GPU schedule logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
 gcc -O1 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
for source in roaring_bitmap graph_loader_runtime graph_runtime; do
 g++ -O1 -mavx2 -fopenmp -c "$source.cpp" -o "$TMP/$source.o"
done
OBJECTS=("$TMP"/*.o)
fixtures=("${@}")
if ((${#fixtures[@]}==0)); then fixtures=(temporal_independent temporal_p_bq temporal_pb_q temporal_rounds spatial_source spatial_destination spatial_snapshot spatial_source_acc spatial_d_private spatial_claim_destination spatial_global_fold spatial_source_frontier round_spatial_fold round_spatial_guard round_spatial_twofold); fi
for fixture in "${fixtures[@]}"; do
 FORCE_GPU=1 FORCE_CPU=0 SGPL_GPU_DEBUG=1 GRAPH_FRONTIER_STATS=1 ./GraphProgram "test/$fixture.graph" >"$TMP/$fixture.compile" 2>&1
 case "$fixture" in
  round_spatial_fold|round_spatial_guard) grep -q 'schedule=round-spatial-dag' "$TMP/$fixture.compile" ;;
  round_spatial_twofold|temporal_rounds) grep -q 'scope=Round.*rounds=independent' "$TMP/$fixture.compile" ;;
  temporal_*) grep -q 'scope=SourcePhase' "$TMP/$fixture.compile" ;;
  spatial_*) grep -q 'status=independent-family' "$TMP/$fixture.compile" ;;
 esac
 mv program.o "$TMP/$fixture.o"
 g++ -fopenmp -no-pie "$TMP/$fixture.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/$fixture"
 ref="$fixture"
 [[ "$fixture" == spatial_snapshot ]] && ref=spatial_snapshot_ref
 FORCE_CPU=1 GRAPH_FRONTIER_REWRITE_OFF=1 ./GraphProgram "test/$ref.graph" >"$TMP/reference.compile" 2>&1
 mv program.o "$TMP/reference.o"
 g++ -fopenmp -no-pie "$TMP/reference.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/reference"
 reference="$(SGPL_NUM_THREADS=1 "$TMP/reference")"
 for workers in 1 2 4 8; do
  output="$(timeout 60 env SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS="$workers" "$TMP/$fixture" 2>"$TMP/$fixture.$workers.log")"
  [[ "$output" == "$reference" ]] || { cat "$TMP/$fixture.$workers.log"; echo "$output != $reference"; exit 1; }
  grep -q '\[gpu-task\]' "$TMP/$fixture.$workers.log" || { cat "$TMP/$fixture.$workers.log"; echo "FAIL no device execution: $fixture"; exit 1; }
 done
 output="$(SGPL_GPU_DEBUG=1 SGPL_GPU_FAIL=driver SGPL_NUM_THREADS=4 "$TMP/$fixture" 2>"$TMP/fallback.log")"
 [[ "$output" == "$reference" ]] || { cat "$TMP/fallback.log"; echo "FAIL fallback $fixture"; exit 1; }
 grep -q '\[gpu-capability\].*CPU' "$TMP/fallback.log"
 ! grep -q '\[gpu-task\]' "$TMP/fallback.log"
 echo "PASS $fixture GPU execution and sequential equivalence at 1,2,4,8 workers"
done
