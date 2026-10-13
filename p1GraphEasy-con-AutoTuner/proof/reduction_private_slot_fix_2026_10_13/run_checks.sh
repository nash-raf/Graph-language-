#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$ROOT/proof/reduction_private_slot_fix_2026_10_13"
cd "$ROOT"
ulimit -s unlimited
if [[ -e program.o ]]; then
  echo "program.o already exists; refusing to overwrite another compilation" >&2
  exit 2
fi
OBJS=(runtime.o parallel_runtime.o gpu_runtime.o autotuner_runtime.o
      graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o
      semiring_runtime.o)
for name in two_reduce_slots priv_two_slots_big reduce_real_ops; do
  graph="$ROOT/../verify/cases/parallel/$name.graph"
  echo "Compiling $name (default pipeline)"
  GRAPH_FRONTIER_STATS=1 GRAPH_FRONTIER_VERBOSE=1 SGPL_FRONTIER_STRICT=1 \
    ./GraphProgram "$graph" --emit-ir-to="$OUT/$name.ll" \
    >"$OUT/$name.compile.out" 2>"$OUT/$name.compile.log"
  mv program.o "$OUT/$name.o"
  g++ -O3 -fopenmp -no-pie "$OUT/$name.o" "${OBJS[@]}" -ldl -lnlopt -o "$OUT/$name.bin"
  grep -E '\[graph-frontier\].*(candidate:|schedule=|rewrite|lowered)' "$OUT/$name.compile.log" || true
  for cfg in 1:1 4:4 4:3; do
    threads="${cfg%:*}"
    partitions="${cfg#*:}"
    SGPL_NUM_THREADS="$threads" OMP_NUM_THREADS="$threads" \
      SGPL_CLEANCUT_PARTITIONS="$partitions" \
      "$OUT/$name.bin" >"$OUT/$name.t$threads.p$partitions.out" \
      2>"$OUT/$name.t$threads.p$partitions.err"
    echo "  threads=$threads partitions=$partitions"
    cat "$OUT/$name.t$threads.p$partitions.out"
  done
  echo "Compiling $name (serial reference)"
  GRAPH_FRONTIER_REWRITE_OFF=1 GRAPH_DISABLE_PDG=1 GRAPH_DISABLE_POLLY=1 \
    ./GraphProgram "$graph" --emit-ir-to="$OUT/$name.serial.ll" \
    >"$OUT/$name.serial.compile.out" 2>"$OUT/$name.serial.compile.log"
  mv program.o "$OUT/$name.serial.o"
  g++ -O3 -fopenmp -no-pie "$OUT/$name.serial.o" "${OBJS[@]}" -ldl -lnlopt -o "$OUT/$name.serial.bin"
  SGPL_NUM_THREADS=1 OMP_NUM_THREADS=1 "$OUT/$name.serial.bin" \
    >"$OUT/$name.serial.out" 2>"$OUT/$name.serial.err"
done
python3 "$OUT/verify_outputs.py"
