#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd -P)"
cd "$ROOT"
SUITE="test/pdg_tdg_verification"
COMPILER="${SGPL_GRAPH_COMPILER:-$ROOT/build_tdg_validation/GraphProgram_tdg}"
[[ -x "$COMPILER" ]] || { echo "Build the compiler with $SUITE/build_tdg_validation.sh" >&2; exit 1; }

mkdir -p build_tdg_validation
TMP="$(mktemp -d "$ROOT/build_tdg_validation/tdg_shared_tmp.XXXXXX")"
LARGE_EDGES="$ROOT/$SUITE/tdg_large_edgelist.txt"
edge_created=0
cleanup() {
  if [[ "$edge_created" == 1 ]]; then rm -f -- "$LARGE_EDGES"; fi
  local resolved
  resolved="$(realpath "$TMP")"
  case "$resolved" in
    "$ROOT"/build_tdg_validation/tdg_shared_tmp.*) ;;
    *) echo "refusing to remove unexpected validation path: $resolved" >&2; return 1 ;;
  esac
  if [[ "${SGPL_KEEP_TDG_TMP:-0}" == 1 ]]; then
    echo "validation files: $resolved"
  else
    rm -rf -- "$resolved"
  fi
}
trap cleanup EXIT

# Large enough for both graph callbacks to be live at once; the file is a
# temporary fixture so the repository does not carry a large edge list.
[[ ! -e "$LARGE_EDGES" ]] || {
  echo "$LARGE_EDGES already exists; preserve it and choose a clean checkout" >&2
  exit 1
}
python3 - "$LARGE_EDGES" <<'PY'
import sys
with open(sys.argv[1], 'w') as edges:
    for i in range(262144):
        edges.write(f'{i % 8192} {(i * 17 + 1) % 8192}\n')
PY
edge_created=1

gcc -O2 -std=gnu11 -pthread -c autotuner_runtime.c -o "$TMP/autotuner_runtime.o"
gcc -O2 -std=gnu11 -pthread -c graph_mutation_runtime.c -o "$TMP/graph_mutation_runtime.o"
read -r -a tdg_runtime_flags <<< "${SGPL_TDG_RUNTIME_FLAGS:-}"
read -r -a tdg_compiler_flags <<< "${SGPL_TDG_COMPILER_FLAGS:-}"
gcc -O2 -std=gnu11 -pthread -iquote "$ROOT" "${tdg_runtime_flags[@]}" \
  -c "${SGPL_TDG_RUNTIME_SOURCE:-parallel_runtime.c}" -o "$TMP/parallel_runtime.o"
gcc -O2 -std=gnu11 -pthread -c runtime.c -o "$TMP/runtime.o"
gcc -O2 -std=gnu11 -pthread -c gpu_runtime.c -o "$TMP/gpu_runtime.o"
gcc -O2 -std=gnu11 -fopenmp -iquote . -c semiring_runtime.c -o "$TMP/semiring_runtime.o"
g++ -O2 -std=c++17 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o "$TMP/roaring_bitmap.o"
g++ -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o "$TMP/graph_loader_runtime.o"
g++ -O2 -std=c++17 -c graph_runtime.cpp -o "$TMP/graph_runtime.o"

runtime_objects=("$TMP/runtime.o" "$TMP/parallel_runtime.o" "$TMP/autotuner_runtime.o"
                 "$TMP/graph_mutation_runtime.o" "$TMP/roaring_bitmap.o"
                 "$TMP/graph_loader_runtime.o" "$TMP/graph_runtime.o"
                 "$TMP/gpu_runtime.o" "$TMP/semiring_runtime.o")
read -r -a tdg_extra_objects <<< "${SGPL_TDG_EXTRA_RUNTIME_OBJECTS:-}"
runtime_objects+=("${tdg_extra_objects[@]}")

compile_case() {
  local fixture="$1" mode="$2" out="$3"
  if [[ "$mode" == serial ]]; then
    GRAPH_DISABLE_PDG=1 "$COMPILER" --ir-backend=cpu "${tdg_compiler_flags[@]}" "$fixture" >"$out.compile" 2>"$out.compile.err"
  else
    SGPL_TDG_DEBUG=1 \
      "$COMPILER" --ir-backend=cpu "${tdg_compiler_flags[@]}" "$fixture" >"$out.compile" 2>"$out.compile.err"
  fi
  g++ -O2 -fopenmp -no-pie program.o "${runtime_objects[@]}" -ldl -lnlopt -o "$out.bin"
}

for name in tdg_graph_ordinary_independent tdg_graph_ordinary_large \
            tdg_two_graph_same tdg_two_graph_distinct tdg_two_graph_distinct_large \
            tdg_two_graph_dependent tdg_graph_mutation_dependent \
            tdg_output_consumed tdg_print_dependent \
            tdg_scalar_capture_dependent tdg_two_graph_large; do
  fixture="$SUITE/$name.graph"
  compile_case "$fixture" serial "$TMP/$name.serial"
  compile_case "$fixture" parallel "$TMP/$name.parallel"
  SGPL_NUM_THREADS=1 "$TMP/$name.serial.bin" >"$TMP/$name.serial.out" 2>"$TMP/$name.serial.err"
  SGPL_NUM_THREADS=4 SGPL_TDG_DEBUG=1 "$TMP/$name.parallel.bin" \
    >"$TMP/$name.parallel.out" 2>"$TMP/$name.parallel.err"
  diff -u "$TMP/$name.serial.out" "$TMP/$name.parallel.out"
  echo "PASS $name output"
  grep -E '\[tdg\.(launch|overlap|loop-plan)\]' "$TMP/$name.parallel.err" || true
  if [[ "$name" == tdg_two_graph_large ]]; then
    SGPL_NUM_THREADS=1 SGPL_TDG_DEBUG=1 "$TMP/$name.parallel.bin" \
      >"$TMP/$name.one.out" 2>"$TMP/$name.one.err"
    diff -u "$TMP/$name.serial.out" "$TMP/$name.one.out"
    SGPL_NUM_THREADS=4 SGPL_TDG_TEST_ALLOC_FAIL=1 SGPL_TDG_DEBUG=1 \
      "$TMP/$name.parallel.bin" >"$TMP/$name.alloc.out" \
      2>"$TMP/$name.alloc.err"
    diff -u "$TMP/$name.serial.out" "$TMP/$name.alloc.out"
    objcopy --redefine-sym main=graph_main program.o "$TMP/graph_repeat.o"
    cat >"$TMP/repeat_main.cpp" <<'CPP'
extern "C" int graph_main(void);
int main(void) { for (int i = 0; i < 6; ++i) graph_main(); return 0; }
CPP
    g++ -O2 -fopenmp -no-pie "$TMP/graph_repeat.o" "$TMP/repeat_main.cpp" \
      "${runtime_objects[@]}" -ldl -lnlopt -o "$TMP/repeat.bin"
    SGPL_NUM_THREADS=4 SGPL_TDG_DEBUG=1 "$TMP/repeat.bin" \
      >"$TMP/repeat.out" 2>"$TMP/repeat.err"
    for ((i = 0; i < 6; i++)); do cat "$TMP/$name.serial.out"; done \
      >"$TMP/repeat.expected"
    diff -u "$TMP/repeat.expected" "$TMP/repeat.out"
    grep -E '\[tdg\.(launch|overlap|loop-plan)\]' "$TMP/repeat.err" | tail -24

    SGPL_TDG_TEST_EXTRACT_FAIL=1 SGPL_TDG_DEBUG=1 "$COMPILER" \
      --ir-backend=cpu "${tdg_compiler_flags[@]}" "$fixture" >"$TMP/extract.compile" \
      2>"$TMP/extract.compile.err"
    g++ -O2 -fopenmp -no-pie program.o "${runtime_objects[@]}" \
      -ldl -lnlopt -o "$TMP/extract.bin"
    SGPL_NUM_THREADS=4 "$TMP/extract.bin" >"$TMP/extract.out" \
      2>"$TMP/extract.err"
    diff -u "$TMP/$name.serial.out" "$TMP/extract.out"
  fi
done

python3 "$SUITE/check_tdg_level_certificates.py" --self-test
for name in tdg_graph_ordinary_independent tdg_graph_ordinary_large \
            tdg_two_graph_same tdg_two_graph_distinct tdg_two_graph_distinct_large \
            tdg_two_graph_dependent tdg_graph_mutation_dependent \
            tdg_output_consumed tdg_print_dependent \
            tdg_scalar_capture_dependent tdg_two_graph_large; do
  python3 "$SUITE/check_tdg_level_certificates.py" \
    "$TMP/$name.parallel.compile.err"
done

# The diagnostic opt-out still forces graph tasks into singleton levels.
for singleton_name in tdg_graph_ordinary_independent tdg_two_graph_same; do
  SGPL_TDG_DISABLE_GRAPH_SHARING=1 SGPL_TDG_DEBUG=1 "$COMPILER" --ir-backend=cpu \
    "${tdg_compiler_flags[@]}" "$SUITE/$singleton_name.graph" >"$TMP/$singleton_name.singleton.compile" \
    2>"$TMP/$singleton_name.singleton.compile.err"
  g++ -O2 -fopenmp -no-pie program.o "${runtime_objects[@]}" \
    -ldl -lnlopt -o "$TMP/$singleton_name.singleton.bin"
  SGPL_NUM_THREADS=4 "$TMP/$singleton_name.singleton.bin" \
    >"$TMP/$singleton_name.singleton.out"
  diff -u "$TMP/$singleton_name.serial.out" "$TMP/$singleton_name.singleton.out"
  python3 "$SUITE/check_tdg_level_certificates.py" --forbid-graph-sharing \
    "$TMP/$singleton_name.singleton.compile.err"
done

python3 - "$TMP" <<'PY'
from pathlib import Path
import re
import sys

root = Path(sys.argv[1])

def groups(name):
    rows = re.findall(r'\[tdg\.group\.task\] level=(\d+) task=\d+ graph=(\d+)',
                      (root / f'{name}.parallel.compile.err').read_text())
    by_level = {}
    for level, graph in rows:
        by_level.setdefault(level, []).append(int(graph))
    return by_level

for name in ('tdg_graph_ordinary_independent', 'tdg_graph_ordinary_large'):
    assert any(0 in kinds and 1 in kinds for kinds in groups(name).values()), name
for name in ('tdg_graph_ordinary_independent', 'tdg_two_graph_same'):
    singleton_rows = re.findall(r'\[tdg\.group\.task\] level=(\d+) task=\d+ graph=(\d+)',
                                (root / f'{name}.singleton.compile.err').read_text())
    singleton_groups = {}
    for level, graph in singleton_rows:
        singleton_groups.setdefault(level, []).append(int(graph))
    assert all(kinds == [1] for kinds in singleton_groups.values() if 1 in kinds), f'opt-out graph task was co-scheduled: {name}'
ordinary = (root / 'tdg_graph_ordinary_large.parallel.err').read_text()
assert re.search(r'\[tdg\.overlap\] tasks=2 max_active=[2-9]', ordinary), 'graph/ordinary tasks never overlapped'
for name in ('tdg_two_graph_same', 'tdg_two_graph_distinct',
             'tdg_two_graph_distinct_large', 'tdg_two_graph_large'):
    assert any(kinds.count(1) >= 2 for kinds in groups(name).values()), name
for name in ('tdg_two_graph_dependent', 'tdg_output_consumed', 'tdg_print_dependent'):
    assert all(kinds.count(1) < 2 for kinds in groups(name).values()), name
scalar_compile = (root / 'tdg_scalar_capture_dependent.parallel.compile.err').read_text()
assert re.search(r'\[tdg\.access\] task=\d+ mode=R graph=0 object=global:bias', scalar_compile), 'callback scalar read missing from task summary'
assert all(not (0 in kinds and 1 in kinds)
           for kinds in groups('tdg_scalar_capture_dependent').values()), 'scalar reader overlapped writer'

large = (root / 'tdg_two_graph_large.parallel.err').read_text()
assert re.search(r'\[tdg\.overlap\] tasks=2 max_active=[2-9]', large), 'graph tasks never overlapped'
assert all(int(n) <= 4 for n in re.findall(r'\[tdg\.launch\].*?reserved=(\d+)', large)), 'level exceeded budget'
distinct = (root / 'tdg_two_graph_distinct_large.parallel.err').read_text()
assert re.search(r'\[tdg\.overlap\] tasks=2 max_active=[2-9]', distinct), 'different-graph tasks never overlapped'

warm = (root / 'repeat.err').read_text()
plans = re.findall(r'\[tdg\.loop-plan\] task_slot=(\d+) loop_id=(\d+) assigned_threads=(\d+)', warm)
assert {int(site) for _, site, _ in plans} == {1, 2}, 'warmed graph sites absent from budget plan'
assert 2 + sum(int(width) - 1 for _, _, width in plans[-2:]) <= 4, 'planned widths exceed one level budget'
assert 'reason=allocation-fallback' in (root / 'tdg_two_graph_large.alloc.err').read_text()
assert 'reason=budget' in (root / 'tdg_two_graph_large.one.err').read_text()
assert 'result=forced-failure' in (root / 'extract.compile.err').read_text()
refusal = (root / 'tdg_print_dependent.parallel.compile.err').read_text()
assert re.search(r'\[tdg\.task\] id=\d+ kind=1 graph=1 opaque=1', refusal)
print('PASS shared levels, effect ordering, measured overlap, warmed widths, and fallbacks')
PY
