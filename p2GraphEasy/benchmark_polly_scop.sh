#!/usr/bin/env bash
# Analyze SCoP detection and Polly on/off runtime for graph benchmark programs.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

OPT=/usr/local/llvm-20-polly-rtti/bin/opt
TIME=/usr/bin/time
OUT_DIR="${OUT_DIR:-$ROOT/benchmark_logs/polly_scop_$(date +%Y%m%d_%H%M%S)}"
mkdir -p "$OUT_DIR"

# Rebuild GraphProgram if main.cpp changed (caller should run 1run.sh after code edits).
if [[ ! -x ./GraphProgram ]]; then
  echo "GraphProgram missing; run ./1run.sh test.graph test.graph first"
  exit 1
fi

GRAPHS=(
  benchmark_bfs_array.graph
  benchmark_bfs_set.graph
  benchmark_bfs_query.graph
  benchmark_sssp.graph
  benchmark_cc.graph
)

detect_scops() {
  local bc="$1"
  local label="$2"
  local log="$OUT_DIR/${label}_scop_detect.log"
  echo "=== SCoP detect: $label ===" > "$log"
  "$OPT" -polly-print-detect -disable-output "$bc" >>"$log" 2>&1 || true
  python3 - "$log" <<'PY' | tee -a "$log"
import re, sys
text = open(sys.argv[1]).read()
blocks = re.split(r"Printing analysis.*for function '([^']+)':\n", text)
main_scops = total = 0
for i in range(1, len(blocks), 2):
    fn, body = blocks[i], blocks[i+1]
    n = len(re.findall(r"Valid Region for Scop", body))
    total += n
    if fn == "main":
        main_scops = n
print(f"  main SCoPs: {main_scops}")
print(f"  module SCoPs: {total}")
PY
}

compile_graph() {
  local graph="$1"
  local polly_mode="$2"   # "on" or "off"
  local tag="$3"
  local pre_bc="$OUT_DIR/${tag}_pre.bc"
  local post_bc="$OUT_DIR/${tag}_post.bc"
  if [[ "$polly_mode" == "off" ]]; then
    env_prefix=(env GRAPH_DISABLE_POLLY=1)
  else
    env_prefix=()
  fi

  DUMP_LLVM_BC_PRE="$pre_bc" DUMP_LLVM_BC_POST="$post_bc" \
    "${env_prefix[@]}" ./GraphProgram "$graph" >/dev/null 2>"$OUT_DIR/${tag}_compile.log"

  detect_scops "$pre_bc" "${tag}_pre"
  detect_scops "$post_bc" "${tag}_post"
}

link_and_run() {
  local tag="$1"
  g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime_mmap.cpp -o graph_loader_runtime_mmap.o
  g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o
  g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime_mmap.o graph_runtime.o \
    -fopenmp -no-pie -o "final_${tag}"
  export OMP_NUM_THREADS="${OMP_NUM_THREADS:-4}"
  ulimit -s unlimited
  "$TIME" -p "./final_${tag}" >"$OUT_DIR/${tag}_stdout.txt" 2>"$OUT_DIR/${tag}_time.log"
}

echo "Output directory: $OUT_DIR"
echo "timestamp,graph,polly,real_sec,user_sec,sys_sec,scop_pre,scop_post" > "$OUT_DIR/summary.csv"

for graph in "${GRAPHS[@]}"; do
  base="${graph%.graph}"
  for mode in polly_on polly_off; do
    tag="${base}_${mode}"
    echo ""
    echo ">>> $graph  ($mode)"
    compile_graph "$graph" "${mode#polly_}" "$tag"
    link_and_run "$tag"
    real=$(awk '/^real/ {print $2}' "$OUT_DIR/${tag}_time.log" | tail -1)
    user=$(awk '/^user/ {print $2}' "$OUT_DIR/${tag}_time.log" | tail -1)
    sys=$(awk '/^sys/ {print $2}' "$OUT_DIR/${tag}_time.log" | tail -1)
    scop_pre=$(awk '/main SCoPs:/ {print $3}' "$OUT_DIR/${tag}_pre_scop_detect.log" | tail -1)
    scop_post=$(awk '/main SCoPs:/ {print $3}' "$OUT_DIR/${tag}_post_scop_detect.log" | tail -1)
    echo "$(date -Iseconds),$graph,$mode,$real,$user,$sys,$scop_pre,$scop_post" >> "$OUT_DIR/summary.csv"
  done
done

echo ""
echo "Done. Summary: $OUT_DIR/summary.csv"
column -t -s, "$OUT_DIR/summary.csv" 2>/dev/null || cat "$OUT_DIR/summary.csv"
