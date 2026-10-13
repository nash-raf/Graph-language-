#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$ROOT/proof/parallel_claims_2026_10_13/${1:-before}"
mkdir -p "$OUT"
cd "$ROOT"
ulimit -s unlimited
OBJS=(runtime.o parallel_runtime.o gpu_runtime.o autotuner_runtime.o
      graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o semiring_runtime.o)
for name in nested_step mutual_deg reduce_plus_write priv_reduce_write_big carried_read_state dual_same_base shadow_snapshot reduce_real_ops; do
    graph="$ROOT/../verify/cases/parallel/$name.graph"
    echo "CASE $name"
    if ! GRAPH_FRONTIER_STATS=1 GRAPH_FRONTIER_VERBOSE=1 SGPL_WITNESS_DUMP=1 \
        SGPL_FRONTIER_STRICT=1 SGPL_LOOP_CLASSIFY_DEBUG=1 FORCE_CPU=1 \
        ./GraphProgram "$graph" --ir-backend=cpu --emit-ir-to="$OUT/$name.ll" \
        >"$OUT/$name.compile.out" 2>"$OUT/$name.compile.log"; then
        echo "BUILD FAILED $name"
        tail -n 5 "$OUT/$name.compile.log"
        continue
    fi
    mv program.o "$OUT/$name.o"
    g++ -O3 -fopenmp -no-pie "$OUT/$name.o" "${OBJS[@]}" -ldl -lnlopt -o "$OUT/$name.bin"
    grep -E '^\[graph-frontier\] candidate:|^\[frontier-cert\] (hdr=|schedule=)' "$OUT/$name.compile.log" || true
    GRAPH_FRONTIER_REWRITE_OFF=1 GRAPH_DISABLE_PDG=1 GRAPH_DISABLE_POLLY=1 FORCE_CPU=1 \
        ./GraphProgram "$graph" --ir-backend=cpu >"$OUT/$name.serial.compile.out" 2>"$OUT/$name.serial.compile.log"
    mv program.o "$OUT/$name.serial.o"
    g++ -O3 -fopenmp -no-pie "$OUT/$name.serial.o" "${OBJS[@]}" -ldl -lnlopt -o "$OUT/$name.serial.bin"
    timeout 45 env SGPL_NUM_THREADS=1 OMP_NUM_THREADS=1 FORCE_CPU=1 \
        "$OUT/$name.serial.bin" >"$OUT/$name.serial.out" 2>"$OUT/$name.serial.err"
    for cfg in 1:1 4:3 8:8; do
        suffix="t${cfg%:*}.p${cfg#*:}"
        timeout 45 env SGPL_NUM_THREADS="${cfg%:*}" OMP_NUM_THREADS="${cfg%:*}" \
            SGPL_CLEANCUT_PARTITIONS="${cfg#*:}" SGPL_TDG_DEBUG=1 SGPL_DAG_DEBUG=1 FORCE_CPU=1 \
            "$OUT/$name.bin" >"$OUT/$name.$suffix.out" 2>"$OUT/$name.$suffix.err"
        grep -v AutoTuner "$OUT/$name.$suffix.out" >"$OUT/actual.out"
        grep -v AutoTuner "$OUT/$name.serial.out" >"$OUT/expected.out"
        if cmp -s "$OUT/actual.out" "$OUT/expected.out"; then
            echo "PASS serial equality $name threads:partitions=$cfg"
        else
            echo "FAIL serial equality $name threads:partitions=$cfg"
            diff -u "$OUT/expected.out" "$OUT/actual.out" || true
        fi
    done
done
