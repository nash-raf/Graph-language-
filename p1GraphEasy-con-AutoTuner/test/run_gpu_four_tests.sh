#!/usr/bin/env bash
set -euo pipefail
ulimit -c 0
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NATIVE="${1:?Pass the correctness native build directory}"
TMP="$(mktemp -d "$NATIVE/../four-check.XXXXXX")"
trap 'if (($?)); then echo "GPU four-change correctness logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
for ll in "$NATIVE"/src/*.ll; do ln -s "$ll" "$TMP/$(basename "$ll")"; done
objects=()
for name in runtime parallel_runtime gpu_runtime autotuner_runtime graph_mutation_runtime roaring_bitmap graph_loader_runtime graph_runtime semiring_runtime; do objects+=("$NATIVE/$name.o"); done
fixtures=(spatial_source spatial_destination spatial_snapshot spatial_source_acc spatial_d_private spatial_claim_destination spatial_global_fold spatial_source_frontier gpu_resident_iterations gpu_resident_swap)
if (($#>1)); then shift; fixtures=("$@"); fi
awk 'BEGIN {for(u=0;u<64;u++)for(d=1;d<=16;d++){print u,(u+d)%64;print u,(u+d)%64}print 0,64}' >"$TMP/resident.edges"
for fixture in "${fixtures[@]}"; do
 input="$ROOT/test/$fixture.graph"
 if [[ "$fixture" == gpu_resident_* ]]; then
  sed "s#test/gpu_group_parallel.edges#$TMP/resident.edges#" "$input" >"$TMP/$fixture.graph"
  input="$TMP/$fixture.graph"
 fi
 (
  cd "$TMP"
  FORCE_CPU=1 FORCE_GPU=0 "$NATIVE/GraphProgram" --ir-backend=cpu "$input" >"$fixture.cpu.compile" 2>&1
  g++ -fopenmp -no-pie program.o "${objects[@]}" -pthread -ldl -lnlopt -o "$fixture.cpu"
  FORCE_GPU=1 FORCE_CPU=0 "$NATIVE/GraphProgram" --ir-backend=gpu "$input" >"$fixture.gpu.compile" 2>&1
  g++ -fopenmp -no-pie program.o "${objects[@]}" -pthread -ldl -lnlopt -o "$fixture.gpu"
 )
 cd "$ROOT"
 SGPL_NUM_THREADS=1 timeout 90 "$TMP/$fixture.cpu" >"$TMP/$fixture.reference" 2>"$TMP/$fixture.cpu.err"
 for mask in $(seq 0 15); do
  env SGPL_NUM_THREADS=4 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_DEBUG=1 \
   SGPL_GPU_REUSE_RECORDS=$((mask&1)) SGPL_GPU_EARLY_REJECT=$(((mask>>1)&1)) \
   SGPL_GPU_TRANSFER_REDUCE=$(((mask>>2)&1)) SGPL_GPU_REUSE_CAPACITY=$(((mask>>3)&1)) \
   timeout 90 "$TMP/$fixture.gpu" >"$TMP/$fixture.$mask.out" 2>"$TMP/$fixture.$mask.err"
  cmp "$TMP/$fixture.reference" "$TMP/$fixture.$mask.out"
  grep -q '\[gpu-task\]' "$TMP/$fixture.$mask.err"
 done
 env SGPL_NUM_THREADS=4 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_FAIL=driver \
  timeout 90 "$TMP/$fixture.gpu" >"$TMP/$fixture.fallback.out" 2>"$TMP/$fixture.fallback.err"
 cmp "$TMP/$fixture.reference" "$TMP/$fixture.fallback.out"
 echo "PASS $fixture: all 16 feature subsets equal CPU reference; driver fallback equal"
done
# Existing threshold refusal must occur before materialization and preserve output.
if [[ -x "$TMP/spatial_source.gpu" ]]; then
env SGPL_NUM_THREADS=4 SGPL_GPU_MIN_TRIPS=9223372036854775807 SGPL_GPU_DEBUG=1 SGPL_GPU_EARLY_REJECT=1 \
 timeout 90 "$TMP/spatial_source.gpu" >"$TMP/early.out" 2>"$TMP/early.err"
cmp "$TMP/spatial_source.reference" "$TMP/early.out"
grep -q 'existing early gate' "$TMP/early.err"
! grep -q '\[gpu-task\]' "$TMP/early.err"
set +e
env SGPL_NUM_THREADS=4 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_FAIL=launch:2 \
 timeout 90 "$TMP/spatial_source.gpu" >"$TMP/failure.out" 2>"$TMP/failure.err"
failure=$?
set -e
((failure!=0))
grep -q 'failed' "$TMP/failure.err"
echo 'PASS early existing threshold and post-setup launch failure without replay'
fi
