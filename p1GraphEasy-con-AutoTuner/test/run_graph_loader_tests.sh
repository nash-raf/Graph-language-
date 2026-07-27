#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
TEST_BIN="$TMP/graph_loader_runtime_test"

g++ -O2 -std=c++17 -fopenmp -mavx2 -march=native \
  "$ROOT/test/graph_loader_runtime_test.cpp" \
  "$ROOT/graph_loader_runtime.cpp" "$ROOT/roaring_bitmap.cpp" \
  -o "$TEST_BIN"

cat >"$TMP/unweighted.txt" <<'EOF'
# duplicate and self-loop coverage
0 1
1 2
2 2
0 1
this line is ignored
3 4
EOF

cat >"$TMP/weighted.txt" <<'EOF'
0 1 7
1 2 -3
2 2 9
0 1 11
3 4 5
EOF

semantic() {
  awk '{ print $1, $2, $3, $4, $5, $6, $7 }'
}

baseline="$(SGPL_GRAPH_CACHE=off OMP_NUM_THREADS=1 \
  "$TEST_BIN" unweighted "$TMP/unweighted.txt")"
[[ "$(printf '%s\n' "$baseline" | awk '{print $1, $2, $3, $6, $7}')" == \
   "5 10 5 5 5" ]]

for threads in 1 2 4 8; do
  result="$(SGPL_GRAPH_CACHE=off OMP_NUM_THREADS="$threads" \
    "$TEST_BIN" unweighted "$TMP/unweighted.txt")"
  [[ "$(printf '%s\n' "$result" | semantic)" == \
     "$(printf '%s\n' "$baseline" | semantic)" ]]
done

# Force the bounded-memory atomic fallback and compare its graph semantics.
printf '0 199999\n' >"$TMP/wide.txt"
wide_histogram="$(SGPL_GRAPH_CACHE=off SGPL_CSR_HISTOGRAM_MB=32 \
  OMP_NUM_THREADS=2 "$TEST_BIN" unweighted "$TMP/wide.txt")"
wide_atomic="$(SGPL_GRAPH_CACHE=off SGPL_CSR_HISTOGRAM_MB=1 \
  OMP_NUM_THREADS=2 "$TEST_BIN" unweighted "$TMP/wide.txt")"
[[ "$(printf '%s\n' "$wide_histogram" | semantic)" == \
   "$(printf '%s\n' "$wide_atomic" | semantic)" ]]

refresh="$(SGPL_GRAPH_CACHE=refresh OMP_NUM_THREADS=4 \
  "$TEST_BIN" unweighted "$TMP/unweighted.txt")"
[[ -s "$TMP/unweighted.txt.sgplbin" ]]
warm="$(OMP_NUM_THREADS=8 "$TEST_BIN" unweighted "$TMP/unweighted.txt")"
[[ "$warm" == "$refresh" ]]

weighted_refresh="$(SGPL_GRAPH_CACHE=refresh OMP_NUM_THREADS=4 \
  "$TEST_BIN" weighted "$TMP/weighted.txt")"
[[ -s "$TMP/weighted.txt.weighted.sgplbin" ]]
weighted_warm="$(OMP_NUM_THREADS=2 "$TEST_BIN" weighted "$TMP/weighted.txt")"
[[ "$weighted_warm" == "$weighted_refresh" ]]
[[ "$(printf '%s\n' "$weighted_warm" | awk '{print $1, $2, $3, $6, $7}')" == \
   "5 10 5 5 5" ]]

# Truncated and checksum-corrupt caches must be ignored and rebuilt.
truncate -s 64 "$TMP/unweighted.txt.sgplbin"
recovered="$(OMP_NUM_THREADS=4 "$TEST_BIN" unweighted "$TMP/unweighted.txt")"
[[ "$(printf '%s\n' "$recovered" | semantic)" == \
   "$(printf '%s\n' "$baseline" | semantic)" ]]
printf '\377' | dd of="$TMP/unweighted.txt.sgplbin" bs=1 seek=300 \
  count=1 conv=notrunc status=none
recovered="$(OMP_NUM_THREADS=4 "$TEST_BIN" unweighted "$TMP/unweighted.txt")"
[[ "$(printf '%s\n' "$recovered" | semantic)" == \
   "$(printf '%s\n' "$baseline" | semantic)" ]]

# Source identity changes invalidate the old cache.
printf '4 5\n' >>"$TMP/unweighted.txt"
invalidated="$(OMP_NUM_THREADS=4 "$TEST_BIN" unweighted "$TMP/unweighted.txt")"
[[ "$(printf '%s\n' "$invalidated" | awk '{print $1, $2, $3, $6, $7}')" == \
   "6 12 6 6 6" ]]

# Cache-off must neither read nor publish a sidecar.
rm -f "$TMP/unweighted.txt.sgplbin"
SGPL_GRAPH_CACHE=off OMP_NUM_THREADS=2 \
  "$TEST_BIN" unweighted "$TMP/unweighted.txt" >/dev/null
[[ ! -e "$TMP/unweighted.txt.sgplbin" ]]

# Concurrent writers must both produce correct results and a valid final cache.
OMP_NUM_THREADS=2 "$TEST_BIN" unweighted "$TMP/unweighted.txt" >"$TMP/a.out" &
pid_a=$!
OMP_NUM_THREADS=3 "$TEST_BIN" unweighted "$TMP/unweighted.txt" >"$TMP/b.out" &
pid_b=$!
wait "$pid_a"
wait "$pid_b"
[[ "$(semantic <"$TMP/a.out")" == "$(semantic <"$TMP/b.out")" ]]
OMP_NUM_THREADS=4 "$TEST_BIN" unweighted "$TMP/unweighted.txt" >/dev/null

# Empty graphs and malformed lines remain valid empty inputs.
printf '# no edges\nmalformed\n' >"$TMP/empty.txt"
empty="$(OMP_NUM_THREADS=4 "$TEST_BIN" unweighted "$TMP/empty.txt")"
[[ "$(printf '%s\n' "$empty" | awk '{print $1, $2, $3, $6, $7}')" == \
   "0 0 0 0 0" ]]

# Invalid vertex IDs are rejected before CSR indexing.
printf '%s\n' '-1 2' >"$TMP/negative.txt"
if SGPL_GRAPH_CACHE=off "$TEST_BIN" unweighted "$TMP/negative.txt" \
     >/dev/null 2>&1; then
  echo "negative vertex ID was accepted" >&2
  exit 1
fi
printf '%s\n' '2147483648 2' >"$TMP/overflow.txt"
if SGPL_GRAPH_CACHE=off "$TEST_BIN" unweighted "$TMP/overflow.txt" \
     >/dev/null 2>&1; then
  echo "overflowing vertex ID was accepted" >&2
  exit 1
fi

# Cache publication failure is non-fatal.
mkdir "$TMP/read-only"
cp "$TMP/weighted.txt" "$TMP/read-only/graph.txt"
chmod 555 "$TMP/read-only"
OMP_NUM_THREADS=2 "$TEST_BIN" weighted "$TMP/read-only/graph.txt" >/dev/null
chmod 755 "$TMP/read-only"

echo "graph loader tests: PASS"
