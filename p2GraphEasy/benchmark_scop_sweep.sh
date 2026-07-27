#!/usr/bin/env bash
# Sweep Polly variants on large pure loop kernels; find configs where Polly wins.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

OPT="${OPT:-/usr/local/llvm-20-polly-rtti/bin/opt}"
TIME="${TIME:-/usr/bin/time}"
OUT_DIR="${OUT_DIR:-$ROOT/benchmark_logs/scop_sweep_$(date +%Y%m%d_%H%M%S)}"
RUNS="${RUNS:-3}"
OMP_NUM_THREADS="${OMP_NUM_THREADS:-$(nproc 2>/dev/null || echo 4)}"

mkdir -p "$OUT_DIR"

GRAPHS=(
  scop_triple_loop_512_heavy.graph
  scop_triple_loop_768.graph
  scop_triple_loop_1024.graph
  scop_triple_loop.graph
)

MODES=(
  "polly_off|1|"
  "polly_on|0|"
  "polly_parallel|0|-polly-parallel"
  "polly_parallel_vec|0|-polly-parallel -polly-vectorizer=stripmine"
)

if [[ ! -x ./GraphProgram ]]; then
  echo "GraphProgram missing; run ./1run.sh test.graph test.graph first"
  exit 1
fi

chmod +x detect_scops.py

compile_and_run() {
  local graph="$1"
  local mode_name="$2"
  local disable_polly="$3"
  local polly_flags="$4"
  local tag="${graph%.graph}_${mode_name}"
  local pre_bc="$OUT_DIR/${tag}_pre.bc"

  local -a env_vars=()
  if [[ "$disable_polly" == "1" ]]; then
    env_vars+=(GRAPH_DISABLE_POLLY=1)
  fi
  if [[ -n "$polly_flags" ]]; then
    env_vars+=(GRAPH_POLLY_EXTRA_FLAGS="$polly_flags")
  fi

  DUMP_LLVM_BC_PRE="$pre_bc" DUMP_LLVM_BC_POST="$OUT_DIR/${tag}_post.bc" \
    env "${env_vars[@]}" ./GraphProgram "$graph" >/dev/null 2>"$OUT_DIR/${tag}_compile.log" || {
      echo "GraphProgram failed for $tag" >&2
      tail -5 "$OUT_DIR/${tag}_compile.log" >&2
      return 1
    }

  python3 detect_scops.py "$pre_bc" --function main >"$OUT_DIR/${tag}_scop.txt"

  g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime_mmap.cpp -o graph_loader_runtime_mmap.o
  g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o
  g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime_mmap.o graph_runtime.o \
    -fopenmp -no-pie -o "$OUT_DIR/final_${tag}"

  export OMP_NUM_THREADS
  ulimit -s unlimited
  : >"$OUT_DIR/${tag}_times.txt"
  for ((i = 1; i <= RUNS; i++)); do
    "$TIME" -p "$OUT_DIR/final_${tag}" >"$OUT_DIR/${tag}_stdout_${i}.txt" 2>>"$OUT_DIR/${tag}_times.txt"
  done
  awk '/^real/ {sum+=$2; n++} END { if (n) printf "%.3f\n", sum/n }' "$OUT_DIR/${tag}_times.txt"
}

echo "Output: $OUT_DIR"
echo "threads: $OMP_NUM_THREADS"
echo "graph,mode,avg_real_sec,scops_main,stdout" >"$OUT_DIR/summary.csv"

for graph in "${GRAPHS[@]}"; do
  for spec in "${MODES[@]}"; do
    IFS='|' read -r mode_name disable_polly polly_flags <<<"$spec"
    echo ""
    echo ">>> $graph  [$mode_name]"
    avg="$(compile_and_run "$graph" "$mode_name" "$disable_polly" "$polly_flags")"
    scops="$(awk '{print $2, $3}' "$OUT_DIR/${graph%.graph}_${mode_name}_scop.txt" | head -1)"
    stdout="$(tail -2 "$OUT_DIR/${graph%.graph}_${mode_name}_stdout_1.txt" | tr '\n' ' ' | sed 's/ *$//')"
    echo "  avg real: ${avg}s  ${scops}  out: ${stdout}"
    scop_n="$(echo "$scops" | awk '{print $2}')"
    scop_n="${scop_n:-0}"
    echo "$graph,$mode_name,$avg,$scop_n,\"$stdout\"" >>"$OUT_DIR/summary.csv"
  done
done

echo ""
echo "=== Summary (lower is better) ==="
column -t -s, "$OUT_DIR/summary.csv" 2>/dev/null || cat "$OUT_DIR/summary.csv"

python3 - "$OUT_DIR/summary.csv" <<'PY'
import csv, sys
from collections import defaultdict
rows = list(csv.DictReader(open(sys.argv[1])))
by_graph = defaultdict(list)
for r in rows:
    by_graph[r['graph']].append(r)
for graph, items in by_graph.items():
    base = next((x for x in items if x['mode'] == 'polly_off'), None)
    if not base:
        continue
    b = float(base['avg_real_sec'])
    print(f"\nSpeedup vs polly_off for {graph} (baseline {b}s):")
    for x in sorted(items, key=lambda z: float(z['avg_real_sec'])):
        t = float(x['avg_real_sec'])
        print(f"  {x['mode']:22s} {t:7.3f}s  {b/t:5.2f}x")
PY
