#!/usr/bin/env bash
# GraphEasy verification suite.  Reports what is broken; fixes nothing.
#   ./run.sh                 all categories
#   ./run.sh lang            one category: lang | algo | parallel | autotuner | motif
# Exit non-zero if any check fails.
set -uo pipefail
R="$(cd "$(dirname "$0")" && pwd)"
C="$R/../p1GraphEasy-con-AutoTuner"
export LD_LIBRARY_PATH="$C/.deps/nlopt/lib:$C/.deps/nlopt/lib64:${LD_LIBRARY_PATH:-}"
ulimit -s unlimited 2>/dev/null
# Emit-side postcondition: any invalid IR produced by the frontier lowering
# aborts the compiler, so a build failure in this suite means the rewrite
# emitted malformed IR (never a silent wrong answer).
export SGPL_FRONTIER_STRICT=1
ONLY="${1:-all}"; PASS=0; FAIL=0; SKIP=0; declare -a BROKEN
mkdir -p "$R/bin"
ok(){ printf "  \033[32mPASS\033[0m  %s\n" "$1"; PASS=$((PASS+1)); }
no(){ printf "  \033[31mFAIL\033[0m  %-24s %s\n" "$1" "$2"; FAIL=$((FAIL+1)); BROKEN+=("$1: $2"); }
skip(){ printf "  \033[33mSKIP\033[0m  %s\n" "$1"; SKIP=$((SKIP+1)); }
g(){ python3 -c "import json;print(json.load(open('$R/expected/golden.json'))['$1'])"; }
# </dev/null is required: 03_run.sh consumes stdin, which would eat the
# manifest being read by the enclosing `while read` loop.
# The mtime check matters: 03_run.sh can fail *after* linking and still leave a
# stale final_program, so a silent stale binary would otherwise be reported as
# a wrong answer rather than a build failure.
compile(){
  local before after
  before=$(stat -c %Y "$C/final_program" 2>/dev/null || echo 0)
  ( cd "$C" && GRAPH_FILE="$1" ./03_run.sh >"$R/bin/build.log" 2>&1 </dev/null )
  after=$(stat -c %Y "$C/final_program" 2>/dev/null || echo 0)
  [[ "$before" == "$after" ]] && return 2
  return 0
}
runp(){ ( cd "$C" && ./final_program 2>/dev/null </dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }
runt(){ ( cd "$C" && SGPL_NUM_THREADS=$1 OMP_NUM_THREADS=$1 ./final_program 2>/dev/null </dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }

# ---------------------------------------------------------------- 1. LANGUAGE
if [[ "$ONLY" == all || "$ONLY" == lang ]]; then
echo "=== 1. LANGUAGE UNIT TESTS ==="
while IFS='|' read -r name exp; do
  [[ -z "${name:-}" ]] && continue
  if ! ( cd "$C" && ./GraphProgram "$R/cases/lang/$name.graph" ) >/dev/null 2>"$R/bin/$name.err" </dev/null; then
    no "lang/$name" "COMPILE: $(head -1 "$R/bin/$name.err" | cut -c1-58)"; continue
  fi
  if ! compile "$R/cases/lang/$name.graph"; then
    no "lang/$name" "BUILD produced no new binary: $(grep -m1 -iE 'error' "$R/bin/build.log" | cut -c1-50)"; continue
  fi
  got=$(runp)
  [[ "$got" == "$exp" ]] && ok "lang/$name" || no "lang/$name" "exp='$exp' got='$got'"
done < "$R/expected/lang.manifest"
fi

# ---------------------------------------------------------------- 2. ALGORITHMS
if [[ "$ONLY" == all || "$ONLY" == algo ]]; then
echo "=== 2. ALGORITHM CORRECTNESS (vs independent scipy/numpy) ==="
declare -A EXP=(
 [bfs_level]="reached $(g bfs_reached) level_checksum $(g bfs_level_checksum)"
 [cc]="components $(g cc_components) label_checksum $(g cc_label_checksum)"
 [kcore]="kcore_size $(g kcore5_size)"
 [sssp]="reachable $(g sssp_reachable) dist_checksum $(g sssp_dist_checksum)")
for c in bfs_level cc kcore sssp; do
  compile "$R/cases/algo/$c.graph" || { no "algo/$c" "build failed"; continue; }
  got=$(runp); [[ "$got" == "${EXP[$c]}" ]] && ok "algo/$c" || no "algo/$c" "exp='${EXP[$c]}' got='$got'"
done
compile "$R/cases/algo/pagerank.graph"
got=$(runp)
python3 - "$got" "$(g pagerank_r0)" "$(g pagerank_r1)" <<'PY' >/dev/null 2>&1 && ok "algo/pagerank" || no "algo/pagerank" "got='$got'"
import sys
t=sys.argv[1].split(); d=dict(zip(t[0::2],t[1::2]))
r0,r1,tot=float(d['rank_0']),float(d['rank_1']),float(d['rank_total'])
e0,e1=float(sys.argv[2]),float(sys.argv[3])
assert abs(tot-1.0)<1e-6
# tolerance is set by the %f print (6 decimals), not by the algorithm;
# the precision limitation itself is reported by lang/real_precision
assert abs(r0-e0)<=2e-2*max(abs(e0),1e-9) and abs(r1-e1)<=2e-2*max(abs(e1),1e-9)
PY
fi

# ---------------------------------------------------------------- 3. PARALLEL
if [[ "$ONLY" == all || "$ONLY" == parallel ]]; then
echo "=== 3. PARALLELISM & RACES ==="
for c in bfs_level cc kcore pagerank sssp; do
  compile "$R/cases/algo/$c.graph" || continue
  a=$(runt 1); b=$(runt 4)
  [[ "$a" == "$b" ]] && ok "race/$c 1thr==4thr" || no "race/$c 1thr==4thr" "1thr='$a' 4thr='$b'"
  n=$(for i in 1 2 3 4 5; do runt 4; echo; done | sort -u | wc -l)
  [[ "$n" == 1 ]] && ok "determinism/$c" || no "determinism/$c" "$n distinct results over 5 runs"
done

# DOALL must be a speedup, not just a same-answer.  The parallel path used to be
# driven one iteration at a time through a function pointer, which blocked
# inlining and vectorisation and made 4 threads slower than 1 -- invisible to a
# correctness check.  Best-of-5 wall clock.
if compile "$R/cases/parallel/doall_scaling.graph"; then
  best(){ local b=99999 v; for i in 1 2 3 4 5; do
      v=$( ( cd "$C" && SGPL_NUM_THREADS=$1 OMP_NUM_THREADS=$1 \
             /usr/bin/time -f 'T=%e' ./final_program >/dev/null ) 2>&1 \
           | grep -oE 'T=[0-9.]+' | cut -d= -f2 )
      b=$(python3 -c "print(min($b,${v:-99999}))"); done; echo "$b"; }
  a=$(runt 1); b4=$(runt 4)
  if [[ "$a" != "$b4" ]]; then
    no "race/doall_scaling 1thr==4thr" "1thr='$a' 4thr='$b4'"
  else
    ok "race/doall_scaling 1thr==4thr"
    t1=$(best 1); t4=$(best 4)
    sp=$(python3 -c "print(f'{${t1}/${t4}:.2f}')")
    # The speedup assertion is machine-load sensitive: on an oversubscribed box
    # 4 threads can measure slower than 1 through no fault of the parallel path
    # (observed 0.87x at load average 7.7 on 4 cores, while a standalone
    # re-measurement right after showed 1.39-1.60x).  Correctness above is
    # always asserted; the throughput threshold only while the 1-minute load
    # average is at or below the core count.  SGPL_SCALING_MAX_LOAD overrides
    # the bound (0 forces the skip path, for testing).
    max_load="${SGPL_SCALING_MAX_LOAD:-$(nproc)}"
    load1=$(cut -d' ' -f1 /proc/loadavg)
    if python3 -c "import sys; sys.exit(0 if float('$load1') <= float('$max_load') else 1)"; then
      if python3 -c "import sys; sys.exit(0 if $sp >= 1.2 else 1)"; then
        ok "scaling/doall_scaling 4thr faster (${sp}x)"
      else
        no "scaling/doall_scaling" "4thr only ${sp}x of 1thr (${t1}s -> ${t4}s); expected >=1.2x"
      fi
    else
      skip "scaling/doall_scaling 4thr ${sp}x, not asserted (load ${load1} > ${max_load} cores)"
    fi
  fi
else
  no "scaling/doall_scaling" "build failed"
fi

# A vertex loop that *contains* a neighbour loop.  DependenceInfo proves no
# carried dependence on it (the writes are owner-computes, acc[u] only), so a
# classifier that trusts that proof alone marks it DOALL -- but the nested
# traversal keeps its state (neighbour variable, iterator, accumulator) in DSL
# globals, which every thread would then share.  That race is invisible to the
# array-subscript analysis and it is why the loop must stay SEQUENTIAL.
# Caught live: 1090473 / 5651057 / 161274 across three 4-thread runs against a
# correct 160136.013072 (verified independently in numpy).
# GRAPH_FRONTIER_REWRITE_OFF=1 keeps the frontier lowering from claiming the
# pattern first, so the PDG is the component actually under test here.
NG_EXP="checksum 160136.013072"
if ( export GRAPH_FRONTIER_REWRITE_OFF=1; compile "$R/cases/parallel/nested_gather.graph" ); then
  bad=""
  for t in 1 4 4 4; do
    got=$( cd "$C" && GRAPH_FRONTIER_REWRITE_OFF=1 SGPL_NUM_THREADS=$t OMP_NUM_THREADS=$t \
           ./final_program 2>/dev/null </dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' )
    [[ "$got" == "$NG_EXP" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/nested_gather (nested neighbour loop)" \
                  || no "race/nested_gather" "exp='$NG_EXP' $bad"
else
  no "race/nested_gather" "build failed"
fi

# Same nested gather through the DEFAULT pipeline: the frontier lowering sees
# the neighbour loop first and must refuse to rewrite it (FP reduction
# register), so the result must stay bit-exact on 1 and 4 threads.  This is the
# path where the historical silent-0 miscompile (driver deactivated, `fadd ptr,
# double`) happened.
if compile "$R/cases/parallel/nested_gather.graph"; then
  bad=""
  for t in 1 4 4; do
    got=$( runt $t )
    [[ "$got" == "$NG_EXP" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/nested_gather_default (refusal keeps driver)" \
                  || no "race/nested_gather_default" "exp='$NG_EXP' $bad"
else
  no "race/nested_gather_default" "build failed"
fi

# Per-vertex scalar accumulators ("gather" loops).  The effect algebra refuses
# them (reduction register / escaping scalar / inline query subloop), so they
# must run serially and exactly.  Regression for the silent-0 class: before
# the refusal, a gather was rewritten and the accumulator was lost.
while IFS='|' read -r name exp; do
  [[ -z "${name:-}" ]] && continue
  if compile "$R/cases/parallel/$name.graph"; then
    bad=""
    for t in 1 4 4; do
      got=$( runt $t )
      [[ "$got" == "$exp" ]] || bad="threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "race/$name (scalar accumulator gather)" \
                    || no "race/$name" "exp='$exp' $bad"
  else
    no "race/$name" "build failed"
  fi
done <<'GATHERS'
int_gather|degsum 40
mutual_deg|mutdegsum 8
GATHERS

# P2 regression: a scalar-indexed same-address accumulator.  The index is read
# from a DSL scalar slot that never changes inside the loop, so every iteration
# touches A[0] -- a loop-carried dependence whose address expression is
# loop-invariant.  A zero-distance (EQ) verdict trusted without an invariance
# check marks this DOALL and 4 threads lose updates; expected A0 = 20000
# (numVertices of the g20k fixture).
if compile "$R/cases/parallel/same_slot.graph"; then
  bad=""
  for t in 1 4 4; do
    got=$(runt $t)
    [[ "$got" == "A0 20000" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/same_slot (same-address accumulator)" \
                  || no "race/same_slot" "exp='A0 20000' $bad"
else
  no "race/same_slot" "build failed"
fi

# Composition A (in-place cross-endpoint R x W): the lowering must freeze the
# round-start array into a per-round shadow snapshot and stay owner-computes.
# Checked against an independent Python reference of the frozen-read semantics
# (compute_roundsep_expected.py, mode=add), for three partition/thread
# configurations -- the shadow is what makes those three agree.
if ( cd "$C" && GRAPH_FRONTIER_STATS=1 GRAPH_FILE="$R/cases/parallel/roundsep.graph" \
       bash ./03_run.sh >"$R/bin/roundsep.log" 2>&1 </dev/null ); then
  if ! grep -q 'shadow=[1-9]' "$R/bin/roundsep.log"; then
    no "race/roundsep" "shadow snapshot not emitted (round-separation path not taken)"
  else
    exp=$(python3 "$C/compute_roundsep_expected.py" add \
            "$R/cases/parallel/roundsep.graph" "$R/fixtures/g20k.txt" \
          | python3 -c 'import sys;print(sum(int(l) for l in sys.stdin))')
    bad=""
    for cfg in "1:1" "4:4" "4:3"; do
      th="${cfg%%:*}"; pt="${cfg##*:}"
      got=$( ( cd "$C" && SGPL_CLEANCUT_PARTITIONS=$pt SGPL_NUM_THREADS=$th \
                 OMP_NUM_THREADS=$th ./final_program 2>/dev/null </dev/null \
               | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
      [[ "$got" == "sum $exp" ]] || bad="threads=$th partitions=$pt got='$got' exp='sum $exp'"
    done
    [[ -z "$bad" ]] && ok "race/roundsep (in-place shadow, ditto across p1/p3/p4)" \
                    || no "race/roundsep" "$bad"
  fi
else
  no "race/roundsep" "build failed"
fi

# Compositions C and D: shapes the effect algebra must keep sequential.
#   C -- one array written through both endpoint regions (mixed U+V).
#   D -- a scalar reduction next to a per-vertex array write.
# Neither may be rewritten (no owner step, no reduction ownership); both are
# checked against values derived from the fixture (n, arcs) and for equality
# across 1 vs 4 threads, so a future unsound composition rule turns them red.
arcs=$(python3 -c "print(sum(1 for l in open('$R/fixtures/g20k.txt') if l.strip()))")
while IFS='|' read -r name exp; do
  [[ -z "${name:-}" ]] && continue
  if compile "$R/cases/parallel/$name.graph"; then
    bad=""
    for t in 1 4; do
      got=$(runt $t)
      [[ "$got" == "$exp" ]] || bad="threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "race/$name ($exp)" || no "race/$name" "exp='$exp' $bad"
  else
    no "race/$name" "build failed"
  fi
done <<GATHERS
mixed_regions|tot $((20000 + 2 * arcs))
reduce_plus_write|acc $arcs w0 1
GATHERS

# Dual-owner + shadow refusal (upstream's small_kcore shape): the shadow freezes
# round-start `alive[]`, but this peeling loop must observe removals made earlier
# in the same round.  The rewrite used to peel 18898 survivors where the serial
# build leaves 18959 (and before that the object never linked at all), so the
# only sound verdict is sequential; the shadow-eligible nest must be refused and
# the answer must equal the serial build and the independent 10-core count.
if ( cd "$C" && GRAPH_FRONTIER_STATS=1 GRAPH_FRONTIER_STRICT=1 \
       GRAPH_FILE="$R/cases/parallel/dual_shadow.graph" \
       bash ./03_run.sh >"$R/bin/dual_shadow.log" 2>&1 </dev/null ); then
  if ! grep -q 'shadow=[1-9].*class=sequential' "$R/bin/dual_shadow.log"; then
    no "race/dual_shadow" "shadow-eligible nest not refused (expected class=sequential)"
  else
    bad=""
    for t in 1 4; do
      got=$( ( cd "$C" && SGPL_NUM_THREADS=$t OMP_NUM_THREADS=$t \
                 ./final_program 2>/dev/null </dev/null \
               | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
      [[ "$got" == "alive_sum 18959" ]] || bad="threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "race/dual_shadow (shadow refused, 10-core = 18959)" \
                    || no "race/dual_shadow" "exp='alive_sum 18959' $bad"
  fi
else
  no "race/dual_shadow" "build failed"
fi

# P6: pre-PDG canonicalization must promote single-function DSL globals to SSA.
# A plain array loop has a load-derived index (`%i = load i32, ptr @i`), which
# DependenceInfo cannot turn into an AddRec, so before the promotion the loop
# was SEQUENTIAL with an unknown carrier.  With it, the loop must be proven
# DOALL; SGPL_NO_PDG_GLOBAL_PROMOTE=1 restores the old (conservative) verdict.
if ( cd "$C" && SGPL_LOOP_CLASSIFY_DEBUG=1 \
       GRAPH_FILE="$R/cases/parallel/array_doall.graph" \
       bash ./03_run.sh >"$R/bin/array_doall.log" 2>&1 </dev/null ); then
  if ! grep -q 'classification=DOALL .*hasProofOfNoCarriedDeps=1' "$R/bin/array_doall.log"; then
    no "parallel/array_doall" "plain array loop not proven DOALL (global promotion regressed)"
  else
    bad=""
    for t in 1 4 4; do
      got=$(runt $t)
      [[ "$got" == "a_last 199999" ]] || bad="threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "parallel/array_doall (DSL globals promoted, proof=1)" \
                    || no "parallel/array_doall" "exp='a_last 199999' $bad"
  fi
else
  no "parallel/array_doall" "build failed"
fi

# P3: a distance-1 memory recurrence (`pref[i] = pref[i-1] + 1`) must be
# classified DOACROSS, carry doacross.wait/post metadata, and reproduce the
# serial answer through the runtime's wait/post protocol.  Forced parallel so
# the check does not depend on the adaptive cost model picking the sync path;
# a missing protocol shows up as garbage prefixes on 4 threads.
if ( cd "$C" && SGPL_LOOP_CLASSIFY_DEBUG=1 DUMP_LLVM_BC_PDG="$R/bin/doacross_scan.bc" \
       GRAPH_FILE="$R/cases/parallel/doacross_scan.graph" \
       bash ./03_run.sh >"$R/bin/doacross_scan.log" 2>&1 </dev/null ); then
  if ! grep -q 'classification=DOACROSS' "$R/bin/doacross_scan.log"; then
    no "race/doacross_scan" "scan not classified DOACROSS (no DOACROSS coverage)"
  elif ! grep -qa 'doacross.wait' "$R/bin/doacross_scan.bc"; then
    no "race/doacross_scan" "DOACROSS classification without wait/post metadata"
  else
    bad=""
    for t in 1 4 4; do
      got=$( ( cd "$C" && SGPL_FORCE_DOACROSS_PARALLEL=1 SGPL_NUM_THREADS=$t \
                 OMP_NUM_THREADS=$t ./final_program 2>/dev/null </dev/null \
               | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
      [[ "$got" == "last 199999" ]] || bad="threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "race/doacross_scan (wait/post protocol, 1thr==4thr)" \
                    || no "race/doacross_scan" "exp='last 199999' $bad"
  fi
else
  no "race/doacross_scan" "build failed"
fi

# P11: a `while` loop *between* the vertex loop and the neighbour loop
# (`for each vertex u { ... while (k < 3) { for each neighbor v ... } }`).
# The lowering used to take the while as the frontier driver, delete its trip
# count from the emitted nest and run one whole-graph engine step per vertex --
# wrong answer (acc0 = degree, not 3*degree) and an effective hang.  The driver
# must be the graph-iteration loop; this shape has to stay sequential and
# honour the while's count.  g20k degree(0) = 19, so the answer is 57.
if compile "$R/cases/parallel/nested_while.graph"; then
  bad=""
  for t in 1 4 4; do
    got=$( ( cd "$C" && SGPL_FORCE_DOALL_PARALLEL=1 SGPL_NUM_THREADS=$t \
               OMP_NUM_THREADS=$t timeout 120 ./final_program 2>/dev/null </dev/null \
             | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
    [[ "$got" == "acc0 57" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/nested_while (while between vertex and neighbour loop)" \
                  || no "race/nested_while" "exp='acc0 57' $bad"
else
  no "race/nested_while" "build failed"
fi

# P8-residue: calls are memory effects the load/store pairing cannot see.  A
# loop containing a stateful runtime call must fail closed -- the call may write
# exactly the data the pairs were shown independent of.  nested_gather's
# traversal nest is the model: with the frontier rewrite disabled the PDG sees
# it directly and must refuse it through the call barrier (a name-based
# allowlist plus the readnone/profiler exemptions), and the program must stay
# bit-identical at 1 and 4 threads.
if ( cd "$C" && GRAPH_FRONTIER_REWRITE_OFF=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
       SGPL_FRONTIER_STRICT=1 GRAPH_FILE="$R/cases/parallel/nested_gather.graph" \
       bash ./03_run.sh >"$R/bin/call_barrier.log" 2>&1 </dev/null ); then
  if ! grep -q 'call barrier: stateful call autograph_neighbor_iter' "$R/bin/call_barrier.log"; then
    no "parallel/call_barrier" "stateful traversal call not barred (certificate unsound)"
  else
    bad=""
    for t in 1 4 4; do
      got=$( ( cd "$C" && GRAPH_FRONTIER_REWRITE_OFF=1 SGPL_NUM_THREADS=$t \
                 OMP_NUM_THREADS=$t timeout 120 ./final_program 2>/dev/null </dev/null \
               | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
      [[ "$got" == "checksum 160136.013072" ]] || bad="threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "parallel/call_barrier (stateful call refuses the certificate)" \
                    || no "parallel/call_barrier" "exp='checksum 160136.013072' $bad"
  fi
else
  no "parallel/call_barrier" "build failed"
fi

# P9: the frontier marker is a *belt*, not the only defence -- with the call
# barrier in place the PDG derives the traversal verdicts itself.  Dropping the
# veto (SGPL_NO_FRONTIER_MARKER=1) may only add verdicts the analysis certifies,
# so the program must still reproduce the default build's answer exactly at 1
# and 4 threads, and must still let the PDG derive at least one DOALL that the
# marker's ancestor cascade used to hide (pagerank's owner-computes leaf loop).
if compile "$R/cases/algo/pagerank.graph"; then
  ref=$(runt 1)
  if ( cd "$C" && SGPL_NO_FRONTIER_MARKER=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
         GRAPH_FILE="$R/cases/algo/pagerank.graph" \
         bash ./03_run.sh >"$R/bin/marker_derived.log" 2>&1 </dev/null ) &&
     grep -q 'classification=DOALL' "$R/bin/marker_derived.log"; then
    bad=""
    for t in 1 4 4; do
      got=$( ( cd "$C" && SGPL_NO_FRONTIER_MARKER=1 SGPL_NUM_THREADS=$t \
                 OMP_NUM_THREADS=$t ./final_program 2>/dev/null </dev/null \
               | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
      [[ "$got" == "$ref" ]] || bad="threads=$t got='$got' want='$ref'"
    done
    [[ -z "$bad" ]] && ok "race/marker_derived (derived verdicts, 1thr==4thr)" \
                    || no "race/marker_derived" "$bad"
  else
    no "race/marker_derived" "no derived DOALL without the marker (analysis went blind)"
  fi
else
  no "race/marker_derived" "build failed"
fi

# Loop-shape regressions around the P11 class (nests that used to miscompile or
# hang silently): two whiles around a neighbour loop, and a carried prefix scan
# written as a `for each vertex`.  Both are run with DOALL *and* DOACROSS forced
# parallel so a classifier or engine regression shows up as a wrong value.
while IFS='|' read -r name exp; do
  [[ -z "${name:-}" ]] && continue
  if compile "$R/cases/parallel/$name.graph"; then
    bad=""
    for t in 1 4 4; do
      for e in SGPL_FORCE_DOALL_PARALLEL=1 SGPL_FORCE_DOACROSS_PARALLEL=1; do
        got=$( ( cd "$C" && env $e SGPL_NUM_THREADS=$t OMP_NUM_THREADS=$t \
                   timeout 120 ./final_program 2>/dev/null </dev/null \
                 | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
        [[ "$got" == "$exp" ]] || bad="threads=$t $e got='$got'"
      done
    done
    [[ -z "$bad" ]] && ok "parallel/$name ($exp)" || no "parallel/$name" "exp='$exp' $bad"
  else
    no "parallel/$name" "build failed"
  fi
done <<SHAPES
nested_while2|acc0 76
foreach_scan|last 19999
SHAPES
fi

# ---------------------------------------------------------------- 4. AUTOTUNER
if [[ "$ONLY" == all || "$ONLY" == autotuner ]]; then
echo "=== 4. AUTOTUNER / COST MODEL ==="
for c in bfs_level cc kcore pagerank sssp; do
  compile "$R/cases/algo/$c.graph" || continue
  line=$( cd "$C" && ./final_program 2>&1 </dev/null | grep "total kind=Traverse" | head -1 )
  p=$(echo "$line" | grep -oE "predicted_ms=[0-9.]+" | cut -d= -f2)
  m=$(echo "$line" | grep -oE " measured_ms=[0-9.]+" | cut -d= -f2)
  if [[ -z "${p:-}" || "$p" == "0.000000" ]]; then
    no "autotuner/$c region-modelled" "predicted_ms=0 -> autotuner sees no region"
  else
    ok "autotuner/$c region-modelled"
    r=$(python3 -c "p=$p;m=${m:-0};print('%.1f'%(m/p) if p>0 else 0)")
    hi=$(python3 -c "print(1 if $r>5.0 or $r<0.2 else 0)")
    [[ "$hi" == 0 ]] && ok "autotuner/$c prediction within 5x (${r}x)" \
                     || no "autotuner/$c prediction accuracy" "measured/predicted = ${r}x (>5x off)"
  fi
done
fi

# ---------------------------------------------------------------- 5. MOTIF
if [[ "$ONLY" == all || "$ONLY" == motif ]]; then
echo "=== 5. MOTIF MATCHING ==="
while IFS='|' read -r name exp; do
  [[ -z "${name:-}" ]] && continue
  compile "$R/cases/motif/$name.graph" || { no "motif/$name" "build failed"; continue; }
  got=$(runp); [[ "$got" == "$exp" ]] && ok "motif/$name (=$exp)" || no "motif/$name" "exp=$exp got=$got"
done < "$R/expected/motif.manifest"
fi

echo
echo "======================================================"
printf "  passed: %d   failed: %d" "$PASS" "$FAIL"
(( SKIP > 0 )) && printf "   skipped: %d" "$SKIP"
printf "\n"
if (( FAIL > 0 )); then
  echo "  ---- BROKEN ----"
  for b in "${BROKEN[@]}"; do echo "   * $b"; done
fi
echo "======================================================"
exit $(( FAIL > 0 ))
