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

# The suite drives the compiler *binary* in $C (03_run.sh compiles the .graph but
# never the compiler).  If a source or header is newer than GraphProgram, a green
# run would be reporting on yesterday's compiler.  Rebuild first (build_lowmem.sh
# skips up-to-date objects, so this is usually a no-op link).
if [[ -n "$(find "$C" -maxdepth 1 \( -name '*.cpp' -o -name '*.h' \) -newer "$C/GraphProgram" -print -quit)" ]]; then
  echo "compiler sources newer than GraphProgram -- rebuilding first"
  if ! ( cd "$C" && bash ./build_lowmem.sh ) >"$R/bin/compiler_build.log" 2>&1; then
    echo "COMPILER BUILD FAILED -- see $R/bin/compiler_build.log"
    exit 2
  fi
fi
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
  rm -f "$C/final_program"
  ( cd "$C" && GRAPH_FRONTIER_STATS=1 GRAPH_FILE="$1" ./03_run.sh >"$R/bin/build.log" 2>&1 </dev/null )
  [[ -f "$C/final_program" ]] || return 2
  return 0
}

# Serial reference for a case: build with the frontier rewrite switched off and
# return the program's output.  Comparing the rewritten build against it is a
# stronger statement than matching a hand-derived value -- it pins *identity*
# with the unrewritten program, which is what "no race, no changed answer"
# means for a composition rule.
compile_serial(){
  rm -f "$C/final_program"
  ( cd "$C" && GRAPH_FRONTIER_REWRITE_OFF=1 GRAPH_FILE="$1" ./03_run.sh >"$R/bin/build_serial.log" 2>&1 </dev/null )
  [[ -f "$C/final_program" ]] || return 2
  ( cd "$C" && ./final_program 2>/dev/null </dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' )
}
runp(){ ( cd "$C" && ./final_program 2>/dev/null </dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }
runt(){ ( cd "$C" && SGPL_NUM_THREADS=$1 OMP_NUM_THREADS=$1 ./final_program 2>/dev/null </dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }

# Assert the effect-algebra verdicts recorded in the *last* compile's log
# (compile() always runs the frontier lowering with GRAPH_FRONTIER_STATS=1).
#   expect_class <case> <class>            every candidate line must report it
#   expect_class <case> <class> <grep -E>  only lines matching the filter count
#   expect_class <case> <class> <filter> <log>   read another log (default build.log)
# The class is printed per candidate loop as `... class=<verdict>`; a case whose
# loop silently turns sequential fails just as loudly as one that silently gets
# claimed parallel, which is the point: "no race today" is not the contract,
# "still classified as intended" is.
expect_class(){
  local case="$1" want="$2" filt="${3:-}" log="${4:-$R/bin/build.log}" lines got n
  lines=$(grep -E '\[graph-frontier\] candidate:' "$log" || true)
  [[ -n "$filt" ]] && lines=$(grep -E "$filt" <<<"$lines" || true)
  if [[ -z "$lines" ]]; then
    no "class/$case" "no [graph-frontier] candidate line${filt:+ matching '$filt'}"
    return
  fi
  got=$(grep -o 'class=[a-z-]*' <<<"$lines" | sed 's/class=//' | sort -u | tr '\n' ',' | sed 's/,$//')
  n=$(grep -c . <<<"$lines")
  if [[ "$got" == "$want" ]]; then
    ok "class/$case ($want${filt:+ for /$filt/}, $n loop(s))"
  else
    no "class/$case" "want class=$want${filt:+ for /$filt/}, got: $got [$n loop(s)]"
  fi
}

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
    # Composition I: the per-source gather has no engine support yet, so the
    # verdict itself is part of the contract -- it must be a *derived* refusal,
    # not an accident.  When the per-source reduction engine lands, this line is
    # the one that has to change.
    expect_class "$name" sequential 'red=1'
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
    # Composition A is the one parallel in-place shape: the shadowed nest must
    # stay dest-owner (a regression to sequential would silently give up the
    # parallel path while still answering correctly).
    expect_class roundsep dest-owner 'shadow=[1-9]' "$R/bin/roundsep.log"
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

# Composition C: one array written through both endpoint regions (mixed U+V).
# It is a pure accumulation (`arr[i] = arr[i] + 1` on both endpoints), so
# composition R3 privatizes the base: every partition accumulates into its own
# copy and the emitted combine folds them.  The verdict, the fixture-derived
# value, the 1-vs-4-thread equality and bit-identity with the unrewritten build
# are all pinned, so a future rule that claims this shape without the proof
# turns the check red.
arcs=$(python3 -c "print(sum(1 for l in open('$R/fixtures/g20k.txt') if l.strip()))")
if ser=$(compile_serial "$R/cases/parallel/mixed_regions.graph") && \
   compile "$R/cases/parallel/mixed_regions.graph"; then
  expect_class mixed_regions privatized
  exp="tot $((20000 + 2 * arcs))"
  bad=""
  for t in 1 4; do
    got=$(runt $t)
    [[ "$got" == "$exp" ]] || bad="threads=$t got='$got' exp='$exp'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/mixed_regions ($exp, == serial)" \
                  || no "priv/mixed_regions" "$bad"
else
  no "priv/mixed_regions" "build failed"
fi

# Composition D: a scalar reduction next to a per-vertex array write.  The
# driver preamble's per-source write is not reproduced by a per-pair step, so
# this stays a derived refusal until the source phase carries it.
if compile "$R/cases/parallel/reduce_plus_write.graph"; then
  expect_class reduce_plus_write sequential
  exp="acc $arcs w0 1"
  bad=""
  for t in 1 4; do
    got=$(runt $t)
    [[ "$got" == "$exp" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/reduce_plus_write ($exp)" \
                  || no "race/reduce_plus_write" "exp='$exp' $bad"
else
  no "race/reduce_plus_write" "build failed"
fi

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

# Composition F: a first-wins claim in the *driver* preamble.  The claim runs
# once per source vertex in the serial program and the branch it feeds decides
# whether that source's neighbour body runs at all; every engine work function
# is called once per (u,v) pair, so neither the claim nor its guard can be
# reproduced there (the dual-owner path grafted the claim without the guard, the
# single-phase paths did not graft it at all).  Must be refused, and the answer
# must equal the serial semantics: only u in {0,1} claim, so only their arcs
# count (5 of the 10 arcs of tiny.txt).
if compile "$R/cases/parallel/claim_driver.graph"; then
  expect_class claim_driver sequential
  bad=""
  for t in 1 4 4; do
    got=$(runt $t)
    [[ "$got" == "outsum 5 claim_left 0" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/claim_driver (driver claim refused, guard semantics kept)" \
                  || no "race/claim_driver" "exp='outsum 5 claim_left 0' $bad"
else
  no "race/claim_driver" "build failed"
fi

# Two scalar accumulators in one neighbour body: the reduction engine gives
# per-partition storage to ReducePtr only, so the second slot would be written
# concurrently by every partition.  `b` discriminates (20 serial, ~20/partitions
# if the second slot were emitted into the work function).
if ser=$(compile_serial "$R/cases/parallel/two_reduce_slots.graph") && \
   compile "$R/cases/parallel/two_reduce_slots.graph"; then
  expect_class two_reduce_slots privatized
  bad=""
  for t in 1 4; do
    got=$(runt $t)
    [[ "$got" == "a 10 b 20" ]] || bad="threads=$t got='$got'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/two_reduce_slots (two partials, == serial)" \
                  || no "priv/two_reduce_slots" "exp='a 10 b 20' $bad"
else
  no "priv/two_reduce_slots" "build failed"
fi

# Composition B: a write whose subscript is a *data* value (`cnt[deg[u]]`).
# The CleanCut owner table is keyed by destination vertex id, so a data-valued
# house cannot be partitioned; the nest must stay sequential.
if ser=$(compile_serial "$R/cases/parallel/data_index_write.graph") && \
   compile "$R/cases/parallel/data_index_write.graph"; then
  expect_class data_index_write privatized 'driver=foreach\..*data=1'
  bad=""
  for t in 1 4; do
    got=$(runt $t)
    [[ "$got" == "cntsum 10" ]] || bad="threads=$t got='$got'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/data_index_write (data-valued subscript privatized)" \
                  || no "priv/data_index_write" "exp='cntsum 10' $bad"
else
  no "priv/data_index_write" "build failed"
fi

# Composition R3 at scale: the two tiny cases above are re-run on the g20k
# fixture, where the step really is split across partitions, against values
# derived independently from the fixture (arcs = 160000, sum of v over all
# arcs, number of vertices with out-degree 1) and against the serial build.
if ser=$(compile_serial "$R/cases/parallel/priv_two_slots_big.graph") && \
   compile "$R/cases/parallel/priv_two_slots_big.graph"; then
  expect_class priv_two_slots_big privatized 'red=1'
  exp="a $arcs b 2068247825"
  bad=""
  for t in 1 4; do
    got=$(runt $t)
    [[ "$got" == "$exp" ]] || bad="threads=$t got='$got' exp='$exp'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/two_slots_big ($exp, == serial)" \
                  || no "priv/two_slots_big" "$bad"
else
  no "priv/two_slots_big" "build failed"
fi

if ser=$(compile_serial "$R/cases/parallel/priv_data_index_big.graph") && \
   compile "$R/cases/parallel/priv_data_index_big.graph"; then
  expect_class priv_data_index_big privatized 'driver=foreach\..*data=1'
  exp="cntsum $arcs cnt1 1335"
  bad=""
  for t in 1 4; do
    got=$(runt $t)
    [[ "$got" == "$exp" ]] || bad="threads=$t got='$got' exp='$exp'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/data_index_big ($exp, == serial)" \
                  || no "priv/data_index_big" "$bad"
else
  no "priv/data_index_big" "build failed"
fi

# Composition G: scalar-global reductions must actually parallelize (the class
# assertion) and must agree with an independent Python computation over the
# fixture -- the operators differ in how the combine has to treat them (Sub
# partials are pre-negated, min/max signedness has to match the body's).
read -r NARC NEG SSUM MINV MAXV <<<"$(python3 -c "
E=[tuple(map(int,l.split())) for l in open('$R/fixtures/g20k.txt') if l.strip()]
A=[(u,v) for (a,b) in E for (u,v) in ((a,b),(b,a))]
print(len(A), -len(A), sum(v for (u,v) in A if v<100), min(v for (u,v) in A), max(v for (u,v) in A))")"
if compile "$R/cases/parallel/reduce_int_ops.graph"; then
  expect_class reduce_int_ops reduction 'red=1'
  GEXP="cnt $NARC subc $NEG ssum $SSUM mn $MINV mn2 $MINV mx $MAXV mx2 $MAXV"
  bad=""
  for t in 1 4 4; do
    got=$(runt $t)
    [[ "$got" == "$GEXP" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/reduce_int_ops (7 int reductions vs python)" \
                  || no "race/reduce_int_ops" "exp='$GEXP' $bad"
else
  no "race/reduce_int_ops" "build failed"
fi
if compile "$R/cases/parallel/reduce_real_ops.graph"; then
  # The float sum parallelizes; the raw `select(fcmp)` min/max do not, by
  # design: a select-min is not reorder-invariant once a NaN or a signed zero is
  # in the stream, so folding partition partials with it cannot reproduce the
  # serial left-to-right fold.  (llvm.minnum/maxnum would be recognized; the
  # DSL's min()/max() builtins emit the select form.)  Both refusals are pinned
  # here so a future reordering-unsafe recognition turns this red.
  expect_class reduce_real_ops reduction 'rsum'
  expect_class reduce_real_ops sequential 'rmn'
  expect_class reduce_real_ops sequential 'rmx'
  # Reals print in %g form, so these exact integers come out without a decimal
  # part.  The sum is exact in double (320000 additions of 1.0), so string
  # equality is the right check here rather than a tolerance.
  REXP="rsum $NARC rmn $MINV rmx $MAXV"
  bad=""
  for t in 1 4 4; do
    got=$(runt $t)
    [[ "$got" == "$REXP" ]] || bad="threads=$t got='$got'"
  done
  [[ -z "$bad" ]] && ok "race/reduce_real_ops (3 real reductions vs python)" \
                  || no "race/reduce_real_ops" "exp='$REXP' $bad"
else
  no "race/reduce_real_ops" "build failed"
fi

# Composition I / P10 under its switch: the per-source gather's visible result is
# written per source (`deg[u] = c`), so the plain per-loop reduction path is
# wrong for it -- that path folds all sources into one total and the epilogue
# never runs (degsum would come out 0).  With SGPL_COMP_I_SOURCE_REDUCTION=1 the
# nest must be classified `source-red`, keep the serial answer, and stay
# invariant to the partition/thread split.  bipartite.txt has 10 sources with no
# out-arcs, so the "no pairs" finish path is exercised by the expected 40.
if ( cd "$C" && SGPL_COMP_I_SOURCE_REDUCTION=1 GRAPH_FRONTIER_STATS=1 \
       GRAPH_FILE="$R/cases/parallel/int_gather.graph" \
       bash ./03_run.sh >"$R/bin/int_gather_red.log" 2>&1 </dev/null ); then
  expect_class int_gather source-red 'red=1' "$R/bin/int_gather_red.log"
  bad=""
  for cfg in "1:1" "4:4" "4:3" "4:1"; do
    th="${cfg%%:*}"; pt="${cfg##*:}"
    got=$( ( cd "$C" && SGPL_CLEANCUT_PARTITIONS=$pt SGPL_NUM_THREADS=$th \
               OMP_NUM_THREADS=$th ./final_program 2>/dev/null </dev/null \
             | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
    [[ "$got" == "degsum 40" ]] || bad="threads=$th partitions=$pt got='$got'"
  done
  [[ -z "$bad" ]] && ok "parallel/int_gather_source_red (P10: per-source partials)" \
                  || no "parallel/int_gather_source_red" "exp='degsum 40' $bad"
else
  no "parallel/int_gather_source_red" "build failed"
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
