#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d "$ROOT/.gpu-group-test.XXXXXX")"
trap 'status=$?; if ((status)); then echo "GPU group logs: $TMP"; tail -n 30 "$TMP/reference.compile" "$TMP/gpu.compile" "$TMP/run.log" 2>/dev/null || true; else rm -rf "$TMP"; fi; rm -f test/gpu_group_parallel.edges' EXIT
python3 - <<'PY'
from pathlib import Path
rows=[f'{u} {(u+d)%64}\n' for u in range(64) for d in range(1,17) for occurrence in range(2)]
rows.append('0 64\n') # Vertex 64 has no neighbors.
Path('test/gpu_group_parallel.edges').write_text(''.join(rows))
PY
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
 gcc -O2 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
for source in roaring_bitmap graph_loader_runtime graph_runtime; do
 g++ -O2 -mavx2 -fopenmp -c "$source.cpp" -o "$TMP/$source.o"
done
OBJECTS=("$TMP"/*.o)
for fixture in gpu_group_parallel spatial_d_private spatial_global_fold round_spatial_fold gpu_resident_iterations; do
sed 's#test/spatial_pairs.txt#test/gpu_group_parallel.edges#g' "test/$fixture.graph" >"$TMP/fixture.graph"
FORCE_CPU=1 GRAPH_FRONTIER_REWRITE_OFF=1 ./GraphProgram "$TMP/fixture.graph" >"$TMP/reference.compile" 2>&1
mv program.o "$TMP/reference.o"
g++ -fopenmp -no-pie "$TMP/reference.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/reference"
SGPL_NUM_THREADS=1 "$TMP/reference" >"$TMP/reference.out" 2>"$TMP/reference.log"
FORCE_GPU=1 FORCE_CPU=0 GRAPH_FRONTIER_STATS=1 SGPL_GPU_DEBUG=1 ./GraphProgram "$TMP/fixture.graph" >"$TMP/gpu.compile" 2>&1
mv program.o "$TMP/gpu.o"
g++ -fopenmp -no-pie "$TMP/gpu.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/gpu"
[[ "$fixture" != round_spatial_fold ]] || grep -q 'schedule=round-spatial-dag' "$TMP/gpu.compile"
for workers in 1 2 4 8; do
 for parts in 1 3 8; do
  timeout 60 env SGPL_NUM_THREADS="$workers" SGPL_CLEANCUT_PARTITIONS="$parts" SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_DEBUG=1 SGPL_GPU_GROUP_TRACE=1 "$TMP/gpu" >"$TMP/output" 2>"$TMP/run.log"
  cmp "$TMP/reference.out" "$TMP/output"
  grep -Eq 'active_lanes=([2-9]|[1-9][0-9]+).*within_batch_overlap=1.*parallel=1' "$TMP/run.log"
  [[ "$fixture" == gpu_group_parallel || "$fixture" == gpu_resident_iterations ]] || grep -q 'kernel=gpu_spatial_component_f_' "$TMP/run.log"
  [[ "$fixture" == gpu_group_parallel || "$fixture" == gpu_resident_iterations ]] || grep -q '\[gpu-groups\] groups=2049 ' "$TMP/run.log"
  [[ "$fixture" != gpu_group_parallel ]] || grep -Eq 'resident_bytes=[1-9][0-9]*' "$TMP/run.log"
  if [[ "$fixture" == gpu_resident_iterations ]]; then
    # The array publishes once, after all 20 iterations.
    [[ "$(grep -c '\[gpu-resident\] host-read' "$TMP/run.log")" == 1 ]]
  fi
 done
done
echo "PASS $fixture: real GPU lanes and within-batch overlap with sequential equivalence"
done
echo 'PASS whole proof groups overlap within one CleanCut batch, private folds, source accumulators, claims, frontier sets, duplicates and zero neighbors'
echo 'PASS round/spatial composition and resident captures at 1,2,4,8 CPU workers and 1,3,8 partitions'
sed 's#test/spatial_pairs.txt#test/gpu_group_parallel.edges#g' test/gpu_resident_swap.graph >"$TMP/fixture.graph"
FORCE_CPU=1 GRAPH_FRONTIER_REWRITE_OFF=1 ./GraphProgram "$TMP/fixture.graph" >"$TMP/reference.compile" 2>&1
mv program.o "$TMP/reference.o"
g++ -fopenmp -no-pie "$TMP/reference.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/reference"
SGPL_NUM_THREADS=1 "$TMP/reference" >"$TMP/reference.out" 2>"$TMP/reference.log"
FORCE_GPU=1 FORCE_CPU=0 GRAPH_FRONTIER_STATS=1 ./GraphProgram "$TMP/fixture.graph" >"$TMP/gpu.compile" 2>&1
grep -q 'status=independent-family group=Source.*runtime=1 realized=1' "$TMP/gpu.compile"
grep -q 'rounds=interfering' "$TMP/gpu.compile"
mv program.o "$TMP/gpu.o"
g++ -fopenmp -no-pie "$TMP/gpu.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/gpu"
SGPL_NUM_THREADS=4 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_DEBUG=1 "$TMP/gpu" >"$TMP/output" 2>"$TMP/run.log"
cmp "$TMP/reference.out" "$TMP/output"
grep -q '\[gpu-task\]' "$TMP/run.log"
echo 'PASS allocation-aware swap proof executes spatial GPU work while dependent rounds remain ordered'
