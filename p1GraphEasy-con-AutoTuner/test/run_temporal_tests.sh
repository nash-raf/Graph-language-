#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
for source in autotuner_runtime graph_mutation_runtime parallel_runtime gpu_runtime runtime semiring_runtime; do
  gcc -O2 -fopenmp -iquote . -c "$source.c" -o "$TMP/$source.o"
done
g++ -O2 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o "$TMP/roaring_bitmap.o"
g++ -O2 -fopenmp -c graph_loader_runtime.cpp -o "$TMP/graph_loader_runtime.o"
g++ -O2 -c graph_runtime.cpp -o "$TMP/graph_runtime.o"
gcc -O2 -std=gnu11 -pthread -iquote . -c test/temporal_generated_probe.c -o "$TMP/probe.o"
OBJECTS=("$TMP"/*.o)
fixtures=(temporal_independent temporal_pb_q temporal_p_bq temporal_connected
          temporal_empty temporal_source_order temporal_claim temporal_rounds temporal_round_one temporal_guarded temporal_round_carried)
expected=('414' '666' '360' '954' '1' $'324\n36\n0' '396' $'4192\n8' $'524\n1' '234' $'1137\n3')
for i in "${!fixtures[@]}"; do
  fixture="${fixtures[i]}"
  FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 SGPL_WITNESS_DUMP=1 SGPL_FRONTIER_STRICT=1 \
    ./GraphProgram "test/$fixture.graph" > "$TMP/$fixture.compile" 2>&1
  mv program.o "$TMP/$fixture.o"
  g++ -fopenmp -no-pie "$TMP/$fixture.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt \
    -Wl,--wrap=sgpl_exec_temporal_components -Wl,--wrap=autograph_neighbor_iter_next -o "$TMP/$fixture"
  # Compile a true sequential reference from the same DSL and graph.
  FORCE_CPU=1 GRAPH_FRONTIER_REWRITE_OFF=1 ./GraphProgram "test/$fixture.graph" > "$TMP/serial.compile" 2>&1
  mv program.o "$TMP/serial.o"
  g++ -fopenmp -no-pie "$TMP/serial.o" "${OBJECTS[@]}" -pthread -ldl -lnlopt \
    -Wl,--wrap=sgpl_exec_temporal_components -Wl,--wrap=autograph_neighbor_iter_next -o "$TMP/serial"
  reference="$(SGPL_NUM_THREADS=1 "$TMP/serial" 2>"$TMP/serial.log")"
  [[ "$reference" == "${expected[i]}" ]] || { echo "FAIL reference $fixture: $reference"; exit 1; }
  EXTRA=()
  case "$fixture" in
    temporal_rounds)
      grep -q 'scope=Round rounds=independent phases=skipped' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_ROUNDS=1 SGPL_TEST_DISPATCHES=1) ;;
    temporal_independent)
      grep -q 'separate components members=0,1,2,' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_DISPATCHES=36 SGPL_TEST_LIVE_COMPONENTS=2) ;;
    temporal_guarded)
      grep -q 'separate components members=0,1,2,' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_DISPATCHES=36 SGPL_TEST_LIVE_COMPONENTS=4) ;;
    temporal_pb_q|temporal_claim)
      grep -q 'scope=SourcePhase.*components=2' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_DISPATCHES=36) ;;
    temporal_round_one)
      grep -q 'scope=SourcePhase rounds=not-applicable phases=independent' "$TMP/$fixture.compile"
      grep -q 'NI_S=0 NI_T=1' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_DISPATCHES=36) ;;
    temporal_round_carried)
      grep -q 'scope=SourcePhase rounds=interfering phases=independent' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_DISPATCHES=108) ;;
    temporal_p_bq)
      grep -q 'separate components members=0,1,1,' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_DISPATCHES=36) ;;
    temporal_source_order)
      grep -q 'scope=SourcePhase.*components=2' "$TMP/$fixture.compile"
      EXTRA=(SGPL_TEST_DISPATCHES=36) ;;
    *) grep -q 'scope=None' "$TMP/$fixture.compile" ;;
  esac
  for workers in 1 2 4; do
    output="$(timeout 30 env SGPL_NUM_THREADS="$workers" "${EXTRA[@]}" "$TMP/$fixture" 2>"$TMP/run.log")"
    [[ "$output" == "$reference" ]] || { echo "FAIL $fixture workers=$workers: $output != $reference"; exit 1; }
  done
  echo "PASS $fixture (sequential equivalence, counts, workers=1,2,4)"
done
for fixture in rt_expr_firstwins roundsep_sssp; do
  FORCE_CPU=1 GRAPH_FRONTIER_STATS=1 SGPL_FRONTIER_STRICT=1 \
    ./GraphProgram "$fixture.graph" > "$TMP/$fixture.compile" 2>&1
  grep -q 'scope=None rounds=inconclusive' "$TMP/$fixture.compile"
  ! grep -q 'scope=Round' "$TMP/$fixture.compile"
  echo "PASS $fixture (frontier progression remains ordered and inconclusive)"
done
echo 'temporal compiler and runtime tests: PASS'
