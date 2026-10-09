#!/usr/bin/env bash
# GraphEasy verification suite.
#   ./run_all.sh              correctness + determinism + regression  (~2 min)
#   ./run_all.sh -k cc        run only cases matching a pattern
#   ./run_all.sh --perf       also report parallel scaling (informational)
# Exits non-zero if any CORRECTNESS or REGRESSION check fails.
set -uo pipefail
V="$(cd "$(dirname "$0")" && pwd)"
C="$V/../p1GraphEasy-con-AutoTuner"
export LD_LIBRARY_PATH="$C/.deps/nlopt/lib:$C/.deps/nlopt/lib64:${LD_LIBRARY_PATH:-}"
ulimit -s unlimited 2>/dev/null
FILTER="${2:-}"; PERF=0; [[ "${1:-}" == "--perf" ]] && PERF=1
[[ "${1:-}" == "-k" ]] && FILTER="${2:-}"
PASS=0; FAIL=0; mkdir -p "$V/bin"
g(){ python3 -c "import json;print(json.load(open('$V/expected/golden.json'))['$1'])"; }

ok(){ printf "  \033[32mPASS\033[0m  %s\n" "$1"; PASS=$((PASS+1)); }
no(){ printf "  \033[31mFAIL\033[0m  %s\n     expected: %s\n     got     : %s\n" "$1" "$2" "$3"; FAIL=$((FAIL+1)); }

build(){ # $1 = case name -> $V/bin/$1
  [[ -n "$FILTER" && "$1" != *"$FILTER"* ]] && return 1
  if [[ ! -x "$V/bin/$1" || "$V/cases/$1.graph" -nt "$V/bin/$1" || "$C/GraphProgram" -nt "$V/bin/$1" ]]; then
    ( cd "$C" && GRAPH_FILE="$V/cases/$1.graph" ./03_run.sh >/dev/null 2>&1 ) || { no "$1 (build)" "compiles" "build failed"; return 1; }
    cp "$C/final_program" "$V/bin/$1"
  fi; return 0
}
run(){ "$V/bin/$1" 2>/dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//'; }
field(){ run "$1" | tr ' ' '\n' | grep -A1 -x "$2" | tail -1; }

echo "=== 1. CORRECTNESS (vs independent scipy/numpy references) ==="
declare -A EXP=(
 [bfs_level]="reached $(g bfs_reached) level_checksum $(g bfs_level_checksum)"
 [cc]="components $(g cc_components) label_checksum $(g cc_label_checksum)"
 [kcore]="kcore_size $(g kcore5_size)"
 [sssp]="reachable $(g sssp_reachable) dist_checksum $(g sssp_dist_checksum)"
)
for c in bfs_level cc kcore sssp; do
  build "$c" || continue
  got=$(run "$c"); [[ "$got" == "${EXP[$c]}" ]] && ok "$c" || no "$c" "${EXP[$c]}" "$got"
done
if build pagerank; then
  got=$(run pagerank)
  python3 - "$got" "$(g pagerank_r0)" "$(g pagerank_r1)" <<'PY' && ok "pagerank (rel tol 1e-12)" || no "pagerank" "r0=$(g pagerank_r0)" "$got"
import sys
t=sys.argv[1].split(); d=dict(zip(t[0::2],t[1::2]))
r0,r1=float(d['rank_0']),float(d['rank_1']); e0,e1=float(sys.argv[2]),float(sys.argv[3])
tot=float(d['rank_total'])
assert abs(r0-e0)<=1e-12*abs(e0) and abs(r1-e1)<=1e-12*abs(e1) and abs(tot-1.0)<1e-9
PY
fi

echo "=== 2. DETERMINISM (5 runs, 4 threads, must be identical) ==="
for c in bfs_level cc kcore pagerank sssp; do
  [[ -x "$V/bin/$c" ]] || continue
  n=$(for i in 1 2 3 4 5; do SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4 "$V/bin/$c" 2>/dev/null | grep -v AutoTuner | tr '\n' ' '; echo; done | sort -u | wc -l)
  [[ "$n" == 1 ]] && ok "$c deterministic" || no "$c deterministic" "1 distinct result" "$n distinct"
done

echo "=== 3. THREAD INVARIANCE (1 thread == 4 threads) ==="
for c in bfs_level cc kcore pagerank sssp; do
  [[ -x "$V/bin/$c" ]] || continue
  a=$(SGPL_NUM_THREADS=1 OMP_NUM_THREADS=1 "$V/bin/$c" 2>/dev/null | grep -v AutoTuner | tr '\n' ' ')
  b=$(SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4 "$V/bin/$c" 2>/dev/null | grep -v AutoTuner | tr '\n' ' ')
  [[ "$a" == "$b" ]] && ok "$c 1thr==4thr" || no "$c 1thr==4thr" "$a" "$b"
done

echo "=== 4. REGRESSIONS (bugs previously found; must not come back) ==="
if [[ -x "$V/bin/pagerank" ]]; then
  r0=$(run pagerank | tr ' ' '\n' | grep -A1 -x rank_0 | tail -1)
  [[ "$r0" != "0.000000" && "$r0" != "0" ]] && ok "real printed at full precision (not %f)" \
    || no "real precision" "rank_0 != 0.000000" "$r0"
fi
if build polly_no_fn_segv; then
  ( cd "$C" && GRAPH_POLLY_EXTRA_FLAGS="-polly-process-unprofitable" GRAPH_FILE="$V/cases/polly_no_fn_segv.graph" ./03_run.sh >/dev/null 2>&1 )
  cp "$C/final_program" "$V/bin/polly_no_fn_segv"
  "$V/bin/polly_no_fn_segv" >/dev/null 2>&1; rc=$?
  [[ $rc -eq 0 ]] && ok "no SIGSEGV under -polly-process-unprofitable" \
                  || no "polly+outliner segv" "exit 0" "exit $rc"
fi

if [[ "$PERF" == 1 ]]; then
  echo "=== 5. PERFORMANCE (informational, never fails) ==="
  for c in bfs_level cc kcore pagerank sssp; do
    [[ -x "$V/bin/$c" ]] || continue
    m=$(/usr/bin/time -f "%e s / %P" "$V/bin/$c" 2>&1 >/dev/null | tail -1)
    printf "  %-12s %s\n" "$c" "$m"
  done
fi

echo
echo "=============================================="
printf "  passed: %d   failed: %d\n" "$PASS" "$FAIL"
echo "=============================================="
exit $(( FAIL > 0 ))
