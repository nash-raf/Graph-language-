#!/usr/bin/env bash
# Loop-shape fuzzer for the graph-loop nests (the P11 class: shapes that compile
# but hang, crash, or change their answer when parallelised).
#
# Every shape is compiled once, then run at 1 thread and at 4 threads several
# times with DOALL and DOACROSS forced parallel.  A shape fails if the compile
# fails, a run times out, produces nothing, or the 4-thread output differs from
# the 1-thread output.  Shapes with a hand-computable expectation say so in the
# label; the assertions here are self-consistency (1thr == 4thr == repeats),
# which is what catches the silent miscompiles this class produced.
#
# Usage: bash shape_fuzz.sh [only-substring]
set -uo pipefail
R="$(cd "$(dirname "$0")" && pwd)"
C="$R/../p1GraphEasy-con-AutoTuner"
FIX="$R/fixtures/g20k.txt"
SMALL="$R/fixtures/mutual5.txt"
WORK="${SHAPE_FUZZ_WORK:-$(mktemp -d /tmp/shape_fuzz.XXXXXX)}"
mkdir -p "$WORK"
cd "$C"
FAILED=0
ONLY="${1:-}"

run() { # threads, extra-env, timeout
  ( ulimit -s unlimited 2>/dev/null
    env $2 SGPL_NUM_THREADS=$1 OMP_NUM_THREADS=$1 timeout "$3" ./final_program 2>/dev/null </dev/null ) \
    | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//'
}

check() { # name
  local name="$1"
  if [ -n "$ONLY" ] && [[ "$name" != *"$ONLY"* ]]; then return 0; fi
  if [ ! -x final_program ]; then echo "FAIL $name: compile"; FAILED=1; return; fi
  local ref t out
  ref=$(run 1 "" 60)
  if [ -z "$ref" ]; then echo "FAIL $name: 1-thread produced nothing (hang/crash)"; FAILED=1; return; fi
  for t in 4 4 4; do
    for e in "" "SGPL_FORCE_DOALL_PARALLEL=1" "SGPL_FORCE_DOACROSS_PARALLEL=1"; do
      out=$(run $t "$e" 60)
      if [ -z "$out" ]; then echo "FAIL $name: t=$t ${e:-default} produced nothing"; FAILED=1; return; fi
      if [ "$out" != "$ref" ]; then
        echo "FAIL $name: t=$t ${e:-default} '$out' != 1thr '$ref'"; FAILED=1; return
      fi
    done
  done
  echo "ok   $name  ($ref)"
}

compile() { # file
  rm -f final_program
  GRAPH_FILE="$1" bash ./03_run.sh >"$WORK/$(basename "$1").log" 2>&1
}

# 1. while in foreach with counter (P11-a; sum = 3 * 20000)
cat > "$WORK/s1.graph" <<EOF
graph G { edges: file "$FIX"; };
int n = numVertices(G); int acc[n];
for each vertex u in G { acc[u] = 0; int k = 0;
  while (k < 3) { acc[u] = acc[u] + 1; k = k + 1; } }
int s = 0; int i = 0;
while (i < n) { s = s + acc[i]; i = i + 1; }
print "sum"; print s;
EOF
compile "$WORK/s1.graph"; check "while_counter_in_foreach (exp: sum=60000)"

# 2. while containing a neighbour loop (P11-b: this used to hang / drop the trip count)
cat > "$WORK/s2.graph" <<EOF
graph G { edges: file "$FIX"; };
int n = numVertices(G); int acc[n];
for each vertex u in G { acc[u] = 0; int k = 0;
  while (k < 3) { for each neighbor v of u in G { acc[u] = acc[u] + 1; } k = k + 1; } }
print "acc0"; print acc[0];
EOF
compile "$WORK/s2.graph"; check "while_around_neighbor_loop (exp: acc0=57)"

# 3. two nested whiles around a neighbour loop (exp: acc0 = 4 * 19)
cat > "$WORK/s3.graph" <<EOF
graph G { edges: file "$FIX"; };
int n = numVertices(G); int acc[n];
for each vertex u in G { acc[u] = 0; int a = 0;
  while (a < 2) { int b = 0;
    while (b < 2) { for each neighbor v of u in G { acc[u] = acc[u] + 1; } b = b + 1; }
    a = a + 1; } }
print "acc0"; print acc[0];
EOF
compile "$WORK/s3.graph"; check "nested_while_around_neighbor (exp: acc0=76)"

# 4. per-vertex scalar reduction (refused by design; exp: degsum = 2 * 160000)
cat > "$WORK/s4.graph" <<EOF
graph G { edges: file "$FIX"; };
int n = numVertices(G); int deg[n];
for each vertex u in G { int c = 0;
  for each neighbor v of u in G { c = c + 1; } deg[u] = c; }
int s = 0; int i = 0;
while (i < n) { s = s + deg[i]; i = i + 1; }
print "degsum"; print s;
EOF
compile "$WORK/s4.graph"; check "per_vertex_int_reduction (exp: degsum=320000)"

# 5. conditional claim write inside the neighbour loop
cat > "$WORK/s5.graph" <<EOF
graph G { edges: file "$FIX"; };
int n = numVertices(G); int best[n];
for each vertex u in G { best[u] = 1000000; }
for each vertex u in G { for each neighbor v of u in G { if (v < best[u]) { best[u] = v; } } }
int s = 0; int i = 0;
while (i < n) { s = s + best[i]; i = i + 1; }
print "minsum"; print s;
EOF
compile "$WORK/s5.graph"; check "neighbor_min_claim"

# 6. inline hasEdge query inside the neighbour loop (refused: subloop; exp: 8)
cat > "$WORK/s6.graph" <<EOF
graph G { directed: true; edges: file "$SMALL"; };
int n = numVertices(G); int mdeg[n];
for each vertex u in G { int c = 0;
  for each neighbor v of u in G { if (hasEdge(G, v, u)) { c = c + 1; } } mdeg[u] = c; }
int s = 0; int i = 0;
while (i < n) { s = s + mdeg[i]; i = i + 1; }
print "mutdegsum"; print s;
EOF
compile "$WORK/s6.graph"; check "inline_hasedge_in_neighbor (exp: mutdegsum=8)"

# 7. carried prefix scan written as `for each vertex` (exp: last = 19999)
cat > "$WORK/s7.graph" <<EOF
graph G { edges: file "$FIX"; };
int n = numVertices(G); real pref[n];
pref[0] = 0.0;
for each vertex u in G { if (u > 0) { pref[u] = pref[u - 1] + 1.0; } }
print "last"; print pref[n - 1];
EOF
compile "$WORK/s7.graph"; check "foreach_with_scan_body (exp: last=19999)"

# 8. while AFTER a neighbour loop (driver released to the PDG)
cat > "$WORK/s8.graph" <<EOF
graph G { edges: file "$FIX"; };
int n = numVertices(G); int deg[n];
for each vertex u in G { int c = 0;
  for each neighbor v of u in G { c = c + 1; } deg[u] = c; }
int hi = 0; int i = 0;
while (i < n) { if (deg[i] > hi) { hi = deg[i]; } i = i + 1; }
print "maxdeg"; print hi;
EOF
compile "$WORK/s8.graph"; check "while_after_neighbor_loop"

echo "FUZZ DONE failed=$FAILED  (work dir: $WORK)"
exit $FAILED
