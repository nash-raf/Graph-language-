#!/usr/bin/env bash
# SCoP detection + Polly on/off compile/runtime comparison for pure loop kernels.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

OPT="${OPT:-/usr/local/llvm-20-polly-rtti/bin/opt}"
TIME="${TIME:-/usr/bin/time}"
OUT_DIR="${OUT_DIR:-$ROOT/benchmark_logs/scop_kernels_$(date +%Y%m%d_%H%M%S)}"
RUNS="${RUNS:-3}"
OMP_NUM_THREADS="${OMP_NUM_THREADS:-4}"

mkdir -p "$OUT_DIR"

GRAPHS=(
  scop_triple_loop.graph
  for_loop_test.graph
)

if [[ ! -x ./GraphProgram ]]; then
  echo "GraphProgram missing; run ./1run.sh test.graph test.graph first"
  exit 1
fi

chmod +x detect_scops.py

detect_scops() {
  local bc="$1"
  local label="$2"
  local fn="${3:-main}"
  local log="$OUT_DIR/${label}_scop.log"
  python3 detect_scops.py "$bc" --function "$fn" -v | tee "$log"
  python3 detect_scops.py "$bc" --function "$fn" --json >"$OUT_DIR/${label}_scop.json"
}

compile_variant() {
  local graph="$1"
  local polly_mode="$2"
  local tag="$3"
  local pre_bc="$OUT_DIR/${tag}_pre.bc"
  local post_bc="$OUT_DIR/${tag}_post.bc"
  local env_prefix=()
  if [[ "$polly_mode" == "off" ]]; then
    env_prefix=(env GRAPH_DISABLE_POLLY=1)
  fi

  DUMP_LLVM_BC_PRE="$pre_bc" DUMP_LLVM_BC_POST="$post_bc" \
    "${env_prefix[@]}" ./GraphProgram "$graph" >/dev/null 2>"$OUT_DIR/${tag}_compile.log"

  detect_scops "$pre_bc" "${tag}_pre" "main"
  detect_scops "$post_bc" "${tag}_post" "main"
}

link_and_run() {
  local tag="$1"
  g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime_mmap.cpp -o graph_loader_runtime_mmap.o
  g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o
  g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime_mmap.o graph_runtime.o \
    -fopenmp -no-pie -o "$OUT_DIR/final_${tag}"

  export OMP_NUM_THREADS
  ulimit -s unlimited
  local total=0
  : >"$OUT_DIR/${tag}_times.txt"
  for ((i = 1; i <= RUNS; i++)); do
    "$TIME" -p "$OUT_DIR/final_${tag}" >"$OUT_DIR/${tag}_stdout_${i}.txt" 2>>"$OUT_DIR/${tag}_times.txt"
  done
  awk '/^real/ {sum+=$2; n++} END { if (n) printf "%.3f\n", sum/n }' "$OUT_DIR/${tag}_times.txt"
}

echo "Output: $OUT_DIR"
echo "graph,polly,avg_real_sec,scops_pre_main,scops_post_main,stdout" >"$OUT_DIR/summary.csv"

for graph in "${GRAPHS[@]}"; do
  base="${graph%.graph}"
  for mode in polly_on polly_off; do
    tag="${base}_${mode}"
    echo ""
    echo ">>> $graph ($mode)"
    compile_variant "$graph" "${mode#polly_}" "$tag"
    avg_real="$(link_and_run "$tag")"
    scop_pre="$(python3 -c "import json; print(json.load(open('$OUT_DIR/${tag}_pre_scop.json'))['scops'])")"
    scop_post="$(python3 -c "import json; print(json.load(open('$OUT_DIR/${tag}_post_scop.json'))['scops'])")"
    stdout="$(tail -1 "$OUT_DIR/${tag}_stdout_1.txt" | tr -d '\r')"
    echo "$graph,$mode,$avg_real,$scop_pre,$scop_post,$stdout" >>"$OUT_DIR/summary.csv"
    echo "  avg real: ${avg_real}s  main SCoPs pre/post: ${scop_pre}/${scop_post}  out: ${stdout}"
  done
done

echo ""
echo "=== Summary ==="
column -t -s, "$OUT_DIR/summary.csv" 2>/dev/null || cat "$OUT_DIR/summary.csv"
