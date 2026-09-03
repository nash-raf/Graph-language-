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

# graph:function pairs. The SCoP lives in the DSL `fn`, not in main, so each
# entry names the function to report on. Both inputs are tiny on purpose.
GRAPHS=(
  scop_matmul_fn.graph:kernel
  floyd_warshall_small.graph:floydWarshall
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
  local fn="$4"
  local pre_bc="$OUT_DIR/${tag}_pre.bc"
  local post_bc="$OUT_DIR/${tag}_post.bc"
  local env_prefix=()
  if [[ "$polly_mode" == "off" ]]; then
    env_prefix=(env GRAPH_DISABLE_POLLY=1)
  fi

  DUMP_LLVM_BC_PRE="$pre_bc" DUMP_LLVM_BC_POST="$post_bc" \
    "${env_prefix[@]}" ./GraphProgram "$graph" >/dev/null 2>"$OUT_DIR/${tag}_compile.log"

  # SCoP detection needs canonical loops, so measure on a canonicalized copy of
  # the pre-optimization IR. Counting on the raw dump always reports 0 and says
  # nothing about whether the nest is analyzable.
  local canon_bc="$OUT_DIR/${tag}_canon.bc"
  "$OPT" -passes='mem2reg,loop-simplify,simplifycfg,instcombine,loop-simplify' \
    "$pre_bc" -o "$canon_bc" 2>/dev/null
  detect_scops "$canon_bc" "${tag}_canon" "$fn"
  detect_scops "$post_bc" "${tag}_post" "$fn"
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
# polly_generated counts polly.* blocks in the emitted IR: that, not the post
# SCoP count, is what tells you Polly actually transformed something. The post
# count is normally 0 because the loop has already been rewritten by then.
polly_generated() {
  "$OPT" -S "$1" -o - 2>/dev/null | grep -c 'polly\.' || true
}

echo "graph,function,polly,avg_real_sec,scops_canon,polly_generated,stdout" >"$OUT_DIR/summary.csv"

for entry in "${GRAPHS[@]}"; do
  graph="${entry%%:*}"
  fn="${entry##*:}"
  base="${graph%.graph}"
  for mode in polly_on polly_off; do
    tag="${base}_${mode}"
    echo ""
    echo ">>> $graph ($fn, $mode)"
    compile_variant "$graph" "${mode#polly_}" "$tag" "$fn"
    avg_real="$(link_and_run "$tag")"
    scop_canon="$(python3 -c "import json; print(json.load(open('$OUT_DIR/${tag}_canon_scop.json'))['scops'])")"
    generated="$(polly_generated "$OUT_DIR/${tag}_post.bc")"
    stdout="$(tail -1 "$OUT_DIR/${tag}_stdout_1.txt" | tr -d '\r')"
    echo "$graph,$fn,$mode,$avg_real,$scop_canon,$generated,$stdout" >>"$OUT_DIR/summary.csv"
    echo "  avg real: ${avg_real}s  SCoPs(canon): ${scop_canon}  polly.* emitted: ${generated}  out: ${stdout}"
  done
done

echo ""
echo "=== Summary ==="
column -t -s, "$OUT_DIR/summary.csv" 2>/dev/null || cat "$OUT_DIR/summary.csv"
