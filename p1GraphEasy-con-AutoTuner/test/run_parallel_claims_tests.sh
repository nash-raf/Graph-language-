#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d)"
trap 'status=$?; if ((status)); then echo "claim regression logs: $TMP"; tail -n 45 "$TMP/${name:-none}.compile" 2>/dev/null || true; else rm -rf "$TMP"; fi' EXIT
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
  gcc -O2 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
g++ -O2 -fopenmp -mavx2 -c roaring_bitmap.cpp -o "$TMP/roaring_bitmap.o"
g++ -O2 -fopenmp -c graph_loader_runtime.cpp -o "$TMP/graph_loader_runtime.o"
g++ -O2 -c graph_runtime.cpp -o "$TMP/graph_runtime.o"
OBJECTS=("$TMP"/*.o)
names=(membership_call_aliases spatial_equal_dual spatial_different_dual spatial_subset_dual spatial_claim_repeat spatial_claim_repeat_other
       nested_step mutual_deg reduce_plus_write priv_reduce_write_big carried_read_state dual_same_base shadow_snapshot reduce_real_ops)
for name in "${names[@]}"; do
  graph="test/$name.graph"
  [[ -f "$graph" ]] || graph="$ROOT/../verify/cases/parallel/$name.graph"
  GRAPH_FRONTIER_STATS=1 SGPL_WITNESS_DUMP=1 SGPL_FRONTIER_STRICT=1 FORCE_CPU=1 \
    ./GraphProgram "$graph" --ir-backend=cpu >"$TMP/$name.compile" 2>&1
  mv program.o "$TMP/$name.o"
  g++ -fopenmp -no-pie "$TMP/$name.o" "${OBJECTS[@]}" -ldl -lnlopt -o "$TMP/$name"
  FORCE_CPU=1 GRAPH_FRONTIER_REWRITE_OFF=1 GRAPH_DISABLE_PDG=1 GRAPH_DISABLE_POLLY=1 \
    ./GraphProgram "$graph" --ir-backend=cpu >"$TMP/reference.compile" 2>&1
  mv program.o "$TMP/reference.o"
  g++ -fopenmp -no-pie "$TMP/reference.o" "${OBJECTS[@]}" -ldl -lnlopt -o "$TMP/reference"
  reference="$(SGPL_NUM_THREADS=1 OMP_NUM_THREADS=1 "$TMP/reference" 2>"$TMP/reference.err")"
  case "$name" in
    membership_call_aliases) [[ "$reference" == $'1\n1\n0\n0\n1\n1\n0\n0\n2' ]] ;;
    spatial_equal_dual|dual_same_base)
      grep -q 'schedule=spatial-staged emitted=1' "$TMP/$name.compile"
      grep -q 'class=dual-owner' "$TMP/$name.compile"
      [[ "$name" != spatial_equal_dual || "$reference" == 7 ]] ;;
    spatial_different_dual)
      grep -q 'class=sequential' "$TMP/$name.compile"
      ! grep -q 'schedule=spatial-staged' "$TMP/$name.compile"
      [[ "$reference" == 10 ]] ;;
    spatial_subset_dual)
      ! grep -q 'schedule=spatial-staged' "$TMP/$name.compile"
      [[ "$reference" == 25 ]] ;;
    spatial_claim_repeat|shadow_snapshot)
      grep -q 'discharge=claim-staging' "$TMP/$name.compile"
      grep -q 'candidate:.*fw=1.*class=source-owner' "$TMP/$name.compile"
      [[ "$name" != spatial_claim_repeat || "$reference" == 0 ]] ;;
    spatial_claim_repeat_other)
      grep -q 'candidate:.*fw=1.*class=sequential' "$TMP/$name.compile" ;;
    reduce_plus_write|priv_reduce_write_big)
      grep -q 'candidate:.*red=1.*class=source-phase-dag' "$TMP/$name.compile" ;;
    carried_read_state)
      grep -q 'candidate:.*shadow=1.*class=dest-owner.*temporal=Carried' "$TMP/$name.compile"
      grep -q 'R_S=0 R_T=0' "$TMP/$name.compile" ;;
    reduce_real_ops)
      [[ "$(grep -c 'candidate:.*class=sequential' "$TMP/$name.compile")" == 3 ]] ;;
  esac
  for cfg in 1:1 4:3 8:8; do
    output="$(timeout 45 env SGPL_NUM_THREADS="${cfg%:*}" OMP_NUM_THREADS="${cfg%:*}" \
       SGPL_CLEANCUT_PARTITIONS="${cfg#*:}" SGPL_TDG_DEBUG=1 SGPL_DAG_DEBUG=1 FORCE_CPU=1 \
       "$TMP/$name" 2>"$TMP/$name.$cfg.err")"
    [[ "$output" == "$reference" ]] || { echo "FAIL $name $cfg: $output != $reference"; exit 1; }
  done
  if [[ "$name" == nested_step ]]; then
    grep -q '\[tdg.single-site\].*site_threads=' "$TMP/$name.4:3.err"
  fi
  echo "PASS $name (classification, serial equality, threads:partitions=1:1,4:3,8:8)"
done
echo 'parallel claims regression tests: PASS'
