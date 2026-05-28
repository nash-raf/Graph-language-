#!/usr/bin/env bash
# Smoke-test BFS, CC, SSSP, KCore with known outputs on tiny graphs.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

if [[ ! -x ./GraphProgram ]]; then
  echo "Build GraphProgram first: ./1run.sh test.graph test.graph" >&2
  exit 1
fi

g++ -c -O2 -std=c++17 -fopenmp graph_loader_runtime.cpp -o graph_loader_runtime.o
g++ -c -O2 -std=c++17 graph_runtime.cpp -o graph_runtime.o

run_case() {
  local name="$1"
  local graph="$2"
  local expected="$3"

  echo "=== ${name} ==="
  ./GraphProgram "$graph" >/dev/null
  g++ program.o runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o -fopenmp -no-pie -o final_program
  mapfile -t got < <(./final_program | rg '^-?[0-9]+(\.[0-9]+)?$|2147483647' || true)

  IFS=' ' read -r -a want <<< "$expected"
  if [[ "${#got[@]}" -ne "${#want[@]}" ]]; then
    echo "FAIL ${name}: length ${#got[@]} != ${#want[@]}"
    echo "  got:  ${got[*]}"
    echo "  want: ${want[*]}"
    return 1
  fi
  for i in "${!want[@]}"; do
    if [[ "${got[$i]}" != "${want[$i]}" ]]; then
      echo "FAIL ${name}: mismatch at index $i"
      echo "  got:  ${got[*]}"
      echo "  want: ${want[*]}"
      return 1
    fi
  done
  echo "PASS ${name}: ${got[*]}"
}

fail=0
run_case bfs   test/smoke_bfs.graph   "0 1 1 -1 -1" || fail=1
run_case cc    test/smoke_cc.graph    "0 0 0 3 3" || fail=1
run_case sssp  test/smoke_sssp.graph  "2147483647 0 100 150 201 501 601 751" || fail=1
run_case kcore test/smoke_kcore.graph "1 1 1 0 0" || fail=1
run_case pagerank_delta test/smoke_pagerank_delta.graph "0.200000 0.200000 0.200000 0.200000 0.200000" || fail=1

if [[ "$fail" -ne 0 ]]; then
  echo "Some smoke tests failed." >&2
  exit 1
fi

echo "All smoke tests passed."
