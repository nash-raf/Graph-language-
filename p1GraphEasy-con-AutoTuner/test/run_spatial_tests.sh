#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d)"
trap 'status=$?; if ((status)); then echo "spatial test logs: $TMP"; tail -n 50 "$TMP/${name:-none}.compile" 2>/dev/null || true; else rm -rf "$TMP"; fi' EXIT
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
  gcc -O2 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
g++ -O2 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o "$TMP/roaring_bitmap.o"
g++ -O2 -fopenmp -c graph_loader_runtime.cpp -o "$TMP/graph_loader_runtime.o"
g++ -O2 -c graph_runtime.cpp -o "$TMP/graph_runtime.o"
OBJECTS=("$TMP"/*.o)
fixtures=(destination source sites affine offset_conflict snapshot d_private d_unresolved source_acc source_acc_large mutable_guard global_write global_fold mixed old_value_twice observer unsupported short_guard claim_destination claim_payload frontier_observer source_frontier snapshot_extent)
for name in "${fixtures[@]}"; do
  FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 SGPL_WITNESS_DUMP=1 SGPL_FRONTIER_STRICT=1 \
    ./GraphProgram "test/spatial_$name.graph" >"$TMP/$name.compile" 2>&1 || { cat "$TMP/$name.compile"; exit 1; }
  mv program.o "$TMP/$name.o"
  g++ -fopenmp -no-pie "$TMP/$name.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/$name"
  ref="$name"
  [[ "$name" == snapshot ]] && ref=snapshot_ref
  [[ "$name" == snapshot_extent ]] && ref=snapshot_extent_ref
  FORCE_CPU=1 GRAPH_FRONTIER_REWRITE_OFF=1 ./GraphProgram "test/spatial_$ref.graph" >"$TMP/reference.compile" 2>&1
  mv program.o "$TMP/reference.o"
  g++ -fopenmp -no-pie "$TMP/reference.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt -o "$TMP/reference"
  reference="$(SGPL_NUM_THREADS=1 "$TMP/reference" 2>"$TMP/reference.log")"
  case "$name" in
    destination|snapshot|claim_destination|snapshot_extent)
      grep -q 'status=independent-family group=Destination.*runtime=1 realized=1' "$TMP/$name.compile" ;;
    source|sites|affine|source_acc|source_acc_large|short_guard|source_frontier)
      grep -q 'status=independent-family group=Source.*runtime=1 realized=1' "$TMP/$name.compile" ;;
    d_private|global_fold)
      grep -q 'status=independent-family group=PairOccurrence.*runtime=1 realized=1' "$TMP/$name.compile" ;;
    global_write|old_value_twice|observer|unsupported)
      grep -q 'status=no-partition' "$TMP/$name.compile"
      ! grep -q 'realized=1' "$TMP/$name.compile" ;;
    mutable_guard)
      grep -q 'status=independent-family group=Source.*runtime=0 realized=0' "$TMP/$name.compile" ;;
    *)
      grep -q 'status=inconclusive group=None' "$TMP/$name.compile"
      ! grep -q 'realized=1' "$TMP/$name.compile" ;;
  esac
  if [[ "$name" == snapshot ]]; then
    grep -q 'kind=WAW.*survives=1 removed-by=none' "$TMP/$name.compile"
    grep -q 'removed-by=snapshot' "$TMP/$name.compile"
    ! grep -q 'kind=WAW.*survives=0 removed-by=snapshot' "$TMP/$name.compile"
  fi
  for workers in 1 2 4; do
    output="$(timeout 30 env SGPL_CLEANCUT_PARTITIONS=3 SGPL_NUM_THREADS="$workers" "$TMP/$name" 2>"$TMP/run.log")"
    [[ "$output" == "$reference" ]] || { echo "FAIL $name workers=$workers"; printf '%s\n' "$output" "$reference"; exit 1; }
  done
  if [[ "$name" == source_acc_large ]]; then
    for trial in $(seq 1 24); do
      output="$(timeout 30 env SGPL_CLEANCUT_PARTITIONS=8 SGPL_NUM_THREADS=8 "$TMP/$name" 2>"$TMP/run.log")"
      [[ "$output" == "$reference" ]] || { echo "FAIL source accumulator concurrent final publication trial=$trial"; exit 1; }
    done
  fi
  echo "PASS spatial_$name (proof, reference, workers=1,2,4)"
done
# A relation limit must not turn an unmaterialized graph into independence.
FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 SGPL_REL_CAP=0 ./GraphProgram test/spatial_source.graph >"$TMP/cap.log" 2>&1
grep -q 'status=inconclusive.*spatial relation limit exceeded' "$TMP/cap.log"
! grep -q 'realized=1' "$TMP/cap.log"
echo 'spatial compiler and runtime tests: PASS'
