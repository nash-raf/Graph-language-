#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NATIVE="${1:?Pass the frozen compiler/runtime build directory}"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "GPU batching test logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
for ll in "$NATIVE"/src/*.ll; do ln -s "$ll" "$TMP/$(basename "$ll")"; done
objects=()
for name in runtime parallel_runtime gpu_runtime autotuner_runtime graph_mutation_runtime roaring_bitmap graph_loader_runtime graph_runtime semiring_runtime; do
 objects+=("$NATIVE/$name.o")
done
awk 'BEGIN {for(u=0;u<129;u++){print u,(u+1)%129;print u,(u+1)%129;print u,u}}' >"$TMP/large.edges"
sed "s#test/spatial_pairs.txt#$TMP/large.edges#" "$ROOT/test/spatial_destination.graph" >"$TMP/large.graph"
fixtures=(spatial_source spatial_destination spatial_snapshot spatial_source_acc spatial_d_private spatial_claim_destination spatial_global_fold spatial_source_frontier large)
if (($#>1)); then shift; fixtures=("$@"); fi
for fixture in "${fixtures[@]}"; do
 input="$ROOT/test/$fixture.graph"
 [[ "$fixture" != large ]] || input="$TMP/large.graph"
 (
  cd "$TMP"
  FORCE_GPU=1 FORCE_CPU=0 "$NATIVE/GraphProgram" --ir-backend=gpu "$input" >"$fixture.compile" 2>&1
  g++ -fopenmp -no-pie program.o "${objects[@]}" -pthread -ldl -lnlopt -o "$fixture"
 )
 cd "$ROOT"
 for partitions in 1 3 64 65; do
  for workers in 1 4; do
   tag="$fixture.$partitions.$workers"
   env SGPL_GPU_MIN_TRIPS=0 SGPL_CLEANCUT_PARTITIONS="$partitions" SGPL_NUM_THREADS="$workers" SGPL_GPU_DEBUG=1 SGPL_GPU_BATCH=0 \
    timeout 60 "$TMP/$fixture" >"$TMP/$tag.control.out" 2>"$TMP/$tag.control.err"
   env SGPL_GPU_MIN_TRIPS=0 SGPL_CLEANCUT_PARTITIONS="$partitions" SGPL_NUM_THREADS="$workers" SGPL_GPU_DEBUG=1 SGPL_GPU_BATCH=1 \
    timeout 60 "$TMP/$fixture" >"$TMP/$tag.batch.out" 2>"$TMP/$tag.batch.err"
   cmp "$TMP/$tag.control.out" "$TMP/$tag.batch.out"
   if grep -q '\[gpu-capability\] existing early gate work=-1; CPU before preparation' "$TMP/$tag.control.err"; then
    # The existing descriptor limit can reject before grouping/launch logging.
    ((partitions>64))
    grep -q '\[gpu-capability\] existing early gate work=-1; CPU before preparation' "$TMP/$tag.batch.err"
    ! grep -q '\[gpu-task\]' "$TMP/$tag.control.err"
    ! grep -q '\[gpu-task\]' "$TMP/$tag.batch.err"
    cmp "$TMP/$fixture.64.$workers.batch.out" "$TMP/$tag.batch.out"
    continue
   fi
   effective=$(sed -n 's/.*\[gpu-batch\] partitions=\([0-9]*\) launches=.*enabled=0/\1/p' "$TMP/$tag.control.err" | head -1)
   [[ -n "$effective" ]]
   grep -q "\[gpu-batch\] partitions=$effective launches=1 enabled=1" "$TMP/$tag.batch.err"
   grep -q "\[gpu-batch\] partitions=$effective launches=$effective enabled=0" "$TMP/$tag.control.err"
   a=$(grep -c '\[gpu-task\]' "$TMP/$tag.control.err" || true)
   b=$(grep -c '\[gpu-task\]' "$TMP/$tag.batch.err" || true)
   if ((effective>64)); then
    # Existing context-descriptor capability limit: both paths must fall back.
    ((a == 0 && b == 0))
   else
    ((b > 0 && a == b * effective))
   fi
  done
 done
 # Driver failure must take the pre-effect CPU fallback and retain results.
 env SGPL_GPU_FAIL=driver SGPL_GPU_MIN_TRIPS=0 SGPL_NUM_THREADS=4 SGPL_GPU_DEBUG=1 \
  timeout 60 "$TMP/$fixture" >"$TMP/$fixture.fallback.out" 2>"$TMP/$fixture.fallback.err"
 cmp "$TMP/$fixture.64.4.batch.out" "$TMP/$fixture.fallback.out"
 ! grep -q '\[gpu-task\]' "$TMP/$fixture.fallback.err"
 echo "PASS batching $fixture: exact output, partitions=1,3,64,65 workers=1,4, launch reduction and driver fallback"
done
