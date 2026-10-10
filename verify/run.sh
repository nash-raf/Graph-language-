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
if [[ ! -x "$C/GraphProgram" ]] ||
   [[ -n "$(find "$C" -maxdepth 1 \( -name '*.cpp' -o -name '*.h' \) -newer "$C/GraphProgram" -print -quit)" ]]; then
  echo "compiler missing or sources newer than GraphProgram -- rebuilding first"
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
# Threads and CleanCut partitions are independent knobs: the privatized cases
# are swept over both (4 threads on 3 partitions is the interesting one -- the
# partition count is what the private copies are sized by).
runc(){ ( cd "$C" && SGPL_CLEANCUT_PARTITIONS=$2 SGPL_NUM_THREADS=$1 \
            OMP_NUM_THREADS=$1 ./final_program 2>/dev/null </dev/null \
          | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }
PRIV_CFGS="1:1 4:4 4:3"

# Effective CPU budget: a container may report nproc=48 while a cgroup quota
# throttles it to ~5 CPUs.  A throughput assertion measured under throttling
# says nothing about the parallel path (thread bursts are quota-clipped), so the
# scaling check consults this before asserting.
eff_cores(){
  local q p
  if [[ -r /sys/fs/cgroup/cpu.max ]]; then
    read -r q p < /sys/fs/cgroup/cpu.max
    if [[ "$q" != "max" ]]; then
      python3 -c "import math;print(max(1,math.ceil($q/$p)))"; return
    fi
  elif [[ -r /sys/fs/cgroup/cpu/cpu.cfs_quota_us && \
          -r /sys/fs/cgroup/cpu/cpu.cfs_period_us ]]; then
    q=$(cat /sys/fs/cgroup/cpu/cpu.cfs_quota_us)
    p=$(cat /sys/fs/cgroup/cpu/cpu.cfs_period_us)
    if [[ "$q" -gt 0 ]]; then
      python3 -c "import math;print(max(1,math.ceil($q/$p)))"; return
    fi
  fi
  nproc
}

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
      if [[ -x /usr/bin/time ]]; then
        v=$( ( cd "$C" && SGPL_NUM_THREADS=$1 OMP_NUM_THREADS=$1 \
               /usr/bin/time -f 'T=%e' ./final_program >/dev/null </dev/null ) 2>&1 \
             | grep -oE 'T=[0-9.]+' | cut -d= -f2 )
      else
        # No GNU time (some containers): measure with date.
        local s e
        s=$(date +%s.%N)
        ( cd "$C" && SGPL_NUM_THREADS=$1 OMP_NUM_THREADS=$1 \
          ./final_program >/dev/null 2>&1 </dev/null )
        e=$(date +%s.%N)
        v=$(python3 -c "print(f'{${e}-${s}:.3f}')")
      fi
      b=$(python3 -c "print(min($b,${v:-99999}))"); done; echo "$b"; }
  a=$(runt 1); b4=$(runt 4)
  if [[ "$a" != "$b4" ]]; then
    no "race/doall_scaling 1thr==4thr" "1thr='$a' 4thr='$b4'"
  else
    ok "race/doall_scaling 1thr==4thr"
    t1=$(best 1); t4=$(best 4)
    eff=$(eff_cores)
    if [[ "$t1" == "99999" || "$t4" == "99999" ]]; then
      skip "scaling/doall_scaling (timing unavailable in this environment)"
    elif [[ -z "${SGPL_SCALING_MAX_LOAD:-}" && "$eff" -lt "$(nproc)" ]]; then
      skip "scaling/doall_scaling (container quota ${eff} CPUs < nproc $(nproc): not assertable here)"
    else
    sp=$(python3 -c "print(f'{${t1}/${t4}:.2f}')")
    # The speedup assertion is machine-load sensitive: on an oversubscribed box
    # 4 threads can measure slower than 1 through no fault of the parallel path
    # (observed 0.87x at load average 7.7 on 4 cores, while a standalone
    # re-measurement right after showed 1.39-1.60x).  Correctness above is
    # always asserted; the throughput threshold only while the 1-minute load
    # average is at or below the core count.  SGPL_SCALING_MAX_LOAD overrides
    # the bound (0 forces the skip path, for testing).
    max_load="${SGPL_SCALING_MAX_LOAD:-$eff}"
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

# An engine step nested inside a serial outer loop.  The outer loop must be
# refused by the call barrier (its body holds the frontier rewrite's setup
# call), which is the design -- a step is never re-planned into an outlined
# loop.  The step itself must still be TDG-planned (calibration + single-site
# level), and the answer must be identical at 1 and 4 threads.
NSTEP_EXP="checksum 200"
( cd "$C" && GRAPH_FRONTIER_STATS=1 SGPL_TDG_DEBUG=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
       GRAPH_FILE="$R/cases/parallel/nested_step.graph" \
       bash ./03_run.sh >"$R/bin/nested_step.log" 2>&1 </dev/null )
nstep_rc=$?
if [[ "$nstep_rc" -eq 137 ]]; then
  skip "parallel/nested_step (final_program SIGKILLed rc=137 -- container memory limit)"
elif [[ "$nstep_rc" -eq 0 ]] && [[ -f "$C/final_program" ]]; then
  bad=""
  grep -q 'call barrier: stateful call' "$R/bin/nested_step.log" \
    || bad="no call-barrier evidence for the outer loop"
  grep -q '\[tdg.single-site\]' "$R/bin/nested_step.log" \
    || bad="$bad no TDG decision for the step"
  n1=$(runt 1)
  n4=$(runt 4)
  [[ "$n1" == "$NSTEP_EXP" ]] || bad="$bad 1thr got='$n1'"
  [[ "$n1" == "$n4" ]] || bad="$bad 1thr/4thr differ: '$n1' vs '$n4'"
  [[ -z "$bad" ]] && ok "parallel/nested_step (outer refused, step TDG-planned, 1thr==4thr)" \
                  || no "parallel/nested_step" "$bad"
else
  no "parallel/nested_step" "build failed"
fi

# Compute-heavy DOALL loops: the shapes the GPU offload targets.  These are the
# CPU side of verify/gpu_check.sh (which repeats them on a device when one is
# present).  The values are an independent Python reference of the arithmetic,
# because the point of the GPU path is that a device run answers the same thing.
while IFS='|' read -r name exp; do
  [[ -z "${name:-}" ]] && continue
  if ( cd "$C" && SGPL_LOOP_CLASSIFY_DEBUG=1 \
         GRAPH_FILE="$R/cases/parallel/$name.graph" \
         bash ./03_run.sh >"$R/bin/$name.log" 2>&1 </dev/null ); then
    if ! grep -q 'classification=DOALL .*hasProofOfNoCarriedDeps=1' "$R/bin/$name.log"; then
      no "parallel/$name" "compute loop not proven DOALL (the GPU offload needs it)"
    else
      bad=""
      for t in 1 4 4; do
        got=$( runt $t )
        [[ "$got" == "$exp" ]] || bad="threads=$t got='$got'"
      done
      [[ -z "$bad" ]] && ok "parallel/$name (compute DOALL, offload-target shape)" \
                      || no "parallel/$name" "exp='$exp' $bad"
    fi
  else
    no "parallel/$name" "build failed"
  fi
done <<'COMPUTE'
gpu_compute|a_last 12000037 b_last 32000066
gpu_compute_local|a_last 12000037
COMPUTE

# Per-vertex scalar accumulators ("gather" loops).  Composition I now has a
# per-source reduction engine (the old SGPL_COMP_I_SOURCE_REDUCTION gate is
# gone -- the realization is unconditional), so `int_gather` is emitted as
# per-source partials folded by the finish hook; `mutual_deg` still has no
# supported realization (its per-source query subloop is refused) and must run
# serially and exactly.  The expected class is part of the contract in both
# directions: refining a shape updates this table, it never silently flips.
while IFS='|' read -r name exp wantcls; do
  [[ -z "${name:-}" ]] && continue
  if compile "$R/cases/parallel/$name.graph"; then
    expect_class "$name" "$wantcls" 'red=1'
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
int_gather|degsum 40|source-red
mutual_deg|mutdegsum 8|sequential
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
    # Composition A is the one parallel in-place shape.  Since the emitter
    # unification it is realized through the privatized path (which owns the
    # per-round snapshot, hence shadow>=1); a regression to sequential would
    # silently give up the parallel path while still answering correctly.
    expect_class roundsep privatized 'shadow=[1-9]' "$R/bin/roundsep.log"
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
  # The discharge matrix for this shape: R4 (carried read on a mutated base)
  # and R6 (same-base U+V dual ownership) are both present and both discharged
  # by privatization -- the two hazards that rule alone licenses.
  grep -q "#1 R4 temporal" "$R/bin/build.log" || bad="R4 witness missing"
  grep -q "#2 R6 spatial base=arr" "$R/bin/build.log" || bad="$bad R6 witness missing"
  [[ $(grep -c "discharge=privatization" "$R/bin/build.log") -ge 2 ]] \
    || bad="$bad both witnesses not privatized"
  for cfg in $PRIV_CFGS; do
    th="${cfg%%:*}"; pt="${cfg##*:}"
    got=$(runc "$th" "$pt")
    [[ "$got" == "$exp" ]] || bad="threads=$th partitions=$pt got='$got' exp='$exp'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/mixed_regions ($exp, == serial)" \
                  || no "priv/mixed_regions" "$bad"
else
  no "priv/mixed_regions" "build failed"
fi

# The emitted pair body must reach `arr` through the partition's private copy,
# never through the shared symbol.  Values alone do not pin this: a body that
# increments the global non-atomically still prints the right total on a quiet
# machine or at one thread, which is exactly how the privatization machinery sat
# inert (copies allocated, identity-initialized, never written) behind 70 green
# checks.  GRAPH_FRONTIER_DUMP=1 dumps the post-emit module; the assertion is on
# the pair work function's IR: it derives the record (autograph_exec_partition_state)
# and no access inside it names the privatized base.
rm -f /tmp/post_emit_module.ll   # a stale dump would report on the previous build
if ( cd "$C" && GRAPH_FRONTIER_STATS=1 GRAPH_FRONTIER_DUMP=1 \
       GRAPH_FILE="$R/cases/parallel/mixed_regions.graph" \
       bash ./03_run.sh >"$R/bin/mixed_regions_ir.log" 2>&1 </dev/null ) &&
   [[ -f /tmp/post_emit_module.ll ]]; then
  pairfn=$(awk '/^define .*@sgpl_pair_work/{inf=1} inf{print} inf && /^}/{inf=0}' \
             /tmp/post_emit_module.ll)
  if [[ -z "$pairfn" ]]; then
    no "priv/mixed_regions IR" "no sgpl_pair_work in the post-emit module (loop not rewritten)"
  elif grep -qE '@arr\b' <<<"$pairfn"; then
    no "priv/mixed_regions IR" "pair body still references the shared base: $(grep -m1 -E '@arr\b' <<<"$pairfn" | sed 's/^ *//' | cut -c1-60)"
  elif ! grep -q 'autograph_exec_partition_state' <<<"$pairfn"; then
    no "priv/mixed_regions IR" "pair body never derives the partition record"
  else
    ok "priv/mixed_regions IR (pair body writes the private copy, not @arr)"
  fi
else
  no "priv/mixed_regions IR" "dump build failed"
fi

# Composition D: a scalar reduction next to a per-source array write.  The
# preamble write runs once per source, so the privatized step carries it in a
# separate per-source phase; the pair phase accumulates the reduction into the
# partition's partial.  w0 pinpoints the phase (1 with the preamble phase,
# the source's out-degree without it).
if ser=$(compile_serial "$R/cases/parallel/reduce_plus_write.graph") && \
   compile "$R/cases/parallel/reduce_plus_write.graph"; then
  expect_class reduce_plus_write privatized
  exp="acc $arcs w0 1"
  bad=""
  for cfg in $PRIV_CFGS; do
    th="${cfg%%:*}"; pt="${cfg##*:}"
    got=$(runc "$th" "$pt")
    [[ "$got" == "$exp" ]] || bad="threads=$th partitions=$pt got='$got' exp='$exp'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/reduce_plus_write ($exp, == serial)" \
                  || no "priv/reduce_plus_write" "$bad"
else
  no "priv/reduce_plus_write" "build failed"
fi

# Composition D at scale: sum(w) must be exactly n -- one preamble per source,
# with the sources that have no arcs included.  A lost or duplicated preamble
# phase (or one folded into the pair loop) cannot produce both numbers.
if ser=$(compile_serial "$R/cases/parallel/priv_reduce_write_big.graph") && \
   compile "$R/cases/parallel/priv_reduce_write_big.graph"; then
  expect_class priv_reduce_write_big privatized 'red=1'
  exp="acc $arcs wsum 20000"
  bad=""
  for cfg in $PRIV_CFGS; do
    th="${cfg%%:*}"; pt="${cfg##*:}"
    got=$(runc "$th" "$pt")
    [[ "$got" == "$exp" ]] || bad="threads=$th partitions=$pt got='$got' exp='$exp'"
    [[ "$got" == "$ser" ]] || bad="$bad serial='$ser' parallel='$got'"
  done
  [[ -z "$bad" ]] && ok "priv/reduce_write_big ($exp, == serial)" \
                  || no "priv/reduce_write_big" "$bad"
else
  no "priv/reduce_write_big" "build failed"
fi

# Dual-owner + shadow (upstream's small_kcore shape): the shadow freezes
# round-start `alive[]` and the peeling nest must observe removals made earlier
# in the same round.  The pre-unification rewrite peeled 18898 survivors where
# the serial build leaves 18959, so the verdict used to be a hard refusal; the
# merged privatization/shadow path now realizes the nest and reproduces the
# serial answer, so the contract is behavioural: the nest must be realized (a
# regression to a sequential verdict fires this check for a re-review) and the
# answer must equal the serial build's 18959 at every thread count.
if ( cd "$C" && GRAPH_FRONTIER_STATS=1 GRAPH_FRONTIER_STRICT=1 \
       GRAPH_FILE="$R/cases/parallel/dual_shadow.graph" \
       bash ./03_run.sh >"$R/bin/dual_shadow.log" 2>&1 </dev/null ); then
  if ! grep -q 'class=' "$R/bin/dual_shadow.log"; then
    no "race/dual_shadow" "no candidate line (nest not recognised)"
  elif grep -q 'class=sequential' "$R/bin/dual_shadow.log"; then
    no "race/dual_shadow" "shadow-eligible nest regressed to a sequential verdict"
  else
    bad=""
    for t in 1 2 4 8; do
      got=$( ( cd "$C" && SGPL_NUM_THREADS=$t OMP_NUM_THREADS=$t \
                 ./final_program 2>/dev/null </dev/null \
               | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ) )
      [[ "$got" == "alive_sum 18959" ]] || bad="threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "race/dual_shadow (10-core = 18959 at 1/2/4/8 threads)" \
                    || no "race/dual_shadow" "exp='alive_sum 18959' $bad"
  fi
else
  no "race/dual_shadow" "build failed"
fi

# Two-axis C1 (temporal dirty, spatial clean): the carried-read peel must be
# realized with the spatial axis concurrent and the temporal order preserved by
# the round sequence.  The structural assertions pin the property itself
# (R_S/R_T split, schedule, DAG axes metadata) and the answer must equal the
# unrewritten build at 1/4/8 threads -- a temporal witness may no longer force
# the whole nest sequential.
if ser=$(compile_serial "$R/cases/parallel/carried_read_state.graph") && \
   compile "$R/cases/parallel/carried_read_state.graph"; then
  expect_class carried_read_state dest-owner "temporal=Carried"
  bad=""
  grep -q "schedule=spatial-dag" "$R/bin/build.log" \
    || bad="no schedule=spatial-dag in the certificate"
  grep -q "R_S=0 R_T=1" "$R/bin/build.log" \
    || bad="$bad R_S/R_T split missing"
  grep -q "spatial=parallel temporal=ordered" "$R/bin/build.log" \
    || bad="$bad DAG axes metadata missing"
  for t in 1 4 8; do
    got=$(runt $t)
    [[ "$got" == "$ser" ]] || bad="$bad threads=$t got='$got' serial='$ser'"
  done
  # Partition sweep (1/3/4 partitions via the 1:1, 4:4, 4:3 configs): the
  # temporal order is carried by the round sequence, so the answer may not vary
  # with the partition count.
  for cfg in $PRIV_CFGS; do
    th="${cfg%%:*}"; pt="${cfg##*:}"
    got=$(runc "$th" "$pt")
    [[ "$got" == "$ser" ]] || bad="$bad partitions=$pt got='$got' serial='$ser'"
  done
  # ...and the declared spatial dispatch must *actually run*: the witness on
  # the temporal axis may not cost the spatial axis its parallelism.  The
  # declared schedule is authoritative, so even the first (profile-less)
  # dispatch goes through it; SGPL_DAG_DEBUG shows the partition run and
  # peak>=2 proves real concurrency rather than a nominal declaration.
  dagconc=$( ( cd "$C" && SGPL_DAG_DEBUG=1 SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4 \
               ./final_program 2>&1 >/dev/null </dev/null ) | grep -c "\[sgpl-dag\].*peak=[2-9]" )
  [[ "$dagconc" -ge 1 ]] || bad="$bad declared spatial dispatch did not run concurrently (dag lines with peak>=2: $dagconc)"
  [[ -z "$bad" ]] && ok "race/carried_read_state (temporal dirty, spatial concurrent, == serial)" \
                  || no "race/carried_read_state" "$bad"
else
  no "race/carried_read_state" "build failed"
fi

# Two-axis C2 (spatial dirty, temporal clean): the same-base dual owner must be
# realized staged -- every U-owned write before every V-owned write -- instead
# of the former whole-nest refusal, with each child's internal partition
# parallelism intact and the answer equal to the unrewritten build.
if ser=$(compile_serial "$R/cases/parallel/dual_same_base.graph") && \
   compile "$R/cases/parallel/dual_same_base.graph"; then
  expect_class dual_same_base dual-owner
  bad=""
  grep -q "schedule=temporal-dag emitted=1" "$R/bin/build.log" \
    || bad="no emitted temporal-dag schedule"
  grep -q "R_S=1 R_T=0" "$R/bin/build.log" \
    || bad="$bad R_S/R_T split missing"
  grep -q "spatial=ordered temporal=parallel" "$R/bin/build.log" \
    || bad="$bad DAG axes metadata missing"
  for t in 1 4 8; do
    got=$(runt $t)
    [[ "$got" == "$ser" ]] || bad="$bad threads=$t got='$got' serial='$ser'"
  done
  # The staged children really go through the ready-work scheduler: with
  # SGPL_DAG_DEBUG=1 each child prints its own run line (5 partitions here).
  dagtrace=$( ( cd "$C" && SGPL_DAG_DEBUG=1 SGPL_NUM_THREADS=4 OMP_NUM_THREADS=4 \
                ./final_program 2>&1 >/dev/null </dev/null ) | grep -c "\[sgpl-dag\] nodes=5" )
  [[ "$dagtrace" -ge 2 ]] || bad="$bad staged children not dispatched via the DAG (trace=$dagtrace)"
  [[ -z "$bad" ]] && ok "race/dual_same_base (spatial ordered, staged U->V, == serial)" \
                  || no "race/dual_same_base" "$bad"
else
  no "race/dual_same_base" "build failed"
fi

# R3 (unknown index provenance, Top region) and R7 (append without the wired
# envelope) are spatial witnesses with no discharge: the template is
# unrepresentable, so the theorem licenses no parallel route and the failure is
# the *implementation* one (impl_failure=1) -- never a reclassified R1-R7
# refusal.  Answers must equal the unrewritten build.
for w in "r3_unknown_provenance R3" "r7_append_unwired R7"; do
  set -- $w; fx=$1; rid=$2
  if ser=$(compile_serial "$R/cases/parallel/$fx.graph") && \
     compile "$R/cases/parallel/$fx.graph"; then
    bad=""
    grep -q "#1 $rid spatial" "$R/bin/build.log" || bad="witness $rid missing"
    grep -q "discharge=none" "$R/bin/build.log" || bad="$bad discharge not none"
    grep -q "schedule=serial emitted=1 impl_failure=1" "$R/bin/build.log" \
      || bad="$bad not serial+impl_failure"
    grep -q "R_S=1 R_T=0" "$R/bin/build.log" || bad="$bad axis split missing"
    for t in 1 4; do
      got=$(runt $t)
      [[ "$got" == "$ser" ]] || bad="$bad threads=$t got='$got' serial='$ser'"
    done
    [[ -z "$bad" ]] && ok "race/$fx ($rid unrepresentable -> serial+impl_failure, == serial)" \
                    || no "race/$fx" "$bad"
  else
    no "race/$fx" "build failed"
  fi
done

# Shadow snapshot: a round-separation base read across endpoints is discharged
# by the frozen round-start snapshot (the R4-hiding case), the per-source claim
# is staged, and the nest is admitted.  Pins that the snapshot never reports a
# spurious R4 and that claim staging preserves the answer.
if ser=$(compile_serial "$R/cases/parallel/shadow_snapshot.graph") && \
   compile "$R/cases/parallel/shadow_snapshot.graph"; then
  bad=""
  grep -q "#1 R2 temporal" "$R/bin/build.log" || bad="R2 claim witness missing"
  grep -q "discharge=claim-staging" "$R/bin/build.log" || bad="$bad claim not staged"
  grep -q "R4 temporal" "$R/bin/build.log" && bad="$bad spurious R4 under the shadow snapshot"
  grep -q "schedule=nested emitted=1" "$R/bin/build.log" || bad="$bad not emitted nested"
  for t in 1 4; do
    got=$(runt $t)
    [[ "$got" == "$ser" ]] || bad="$bad threads=$t got='$got' serial='$ser'"
  done
  [[ -z "$bad" ]] && ok "race/shadow_snapshot (snapshot discharges the read; claim staged)" \
                  || no "race/shadow_snapshot" "$bad"
else
  no "race/shadow_snapshot" "build failed"
fi

# R6 order-sensitivity: the same base written in both regions with
# non-commuting read-modify-writes (multiply in U, add in V).  The cross-region
# read on the mutated base makes the *temporal* axis dirty as well, so
# R_S ∧ R_T holds and the theorem routes the nest to serial -- and it must be a
# semantic serial (impl_failure=0), never the staged dual owner, because
# "all U before all V" does not reproduce the serial per-element order for
# non-commuting updates.  The answer equality is what would catch a wrong
# realization.
if ser=$(compile_serial "$R/cases/parallel/r6_order_sensitive.graph") && \
   compile "$R/cases/parallel/r6_order_sensitive.graph"; then
  bad=""
  grep -q "R_S=1 R_T=1" "$R/bin/build.log" || bad="both-axes-dirty split missing"
  grep -q "#2 R6 spatial base=A" "$R/bin/build.log" || bad="$bad R6 witness missing"
  grep -q "schedule=serial emitted=1 impl_failure=0" "$R/bin/build.log" \
    || bad="$bad not a semantic serial"
  for t in 1 4 8; do
    got=$(runt $t)
    [[ "$got" == "$ser" ]] || bad="$bad threads=$t got='$got' serial='$ser'"
  done
  [[ -z "$bad" ]] && ok "race/r6_order_sensitive (non-commuting same base -> semantic serial, == serial)" \
                  || no "race/r6_order_sensitive" "$bad"
else
  no "race/r6_order_sensitive" "build failed"
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
  # R2: the driver-preamble claim is witnessed on the temporal axis and
  # discharged by claim staging; the nest stays sequential because the *guard*
  # semantics still need the once-per-source visit proof (the class assertion
  # above), not because the claim itself is unreconciled.
  grep -q "#1 R2 temporal" "$R/bin/build.log" || bad="R2 witness missing"
  grep -q "discharge=claim-staging" "$R/bin/build.log" || bad="$bad claim not staged"
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
  # R1: the data-region write is witnessed and discharged by privatization --
  # the same rule the class assertion trusts, pinned as a certificate fact.
  grep -q "#1 R1 spatial" "$R/bin/build.log" || bad="R1 witness missing"
  grep -q "src=D.W sink=D.W relation=1 discharge=privatization" "$R/bin/build.log" \
    || bad="$bad R1 not privatized"
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
  for cfg in $PRIV_CFGS; do
    th="${cfg%%:*}"; pt="${cfg##*:}"
    got=$(runc "$th" "$pt")
    [[ "$got" == "$exp" ]] || bad="threads=$th partitions=$pt got='$got' exp='$exp'"
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
  for cfg in $PRIV_CFGS; do
    th="${cfg%%:*}"; pt="${cfg##*:}"
    got=$(runc "$th" "$pt")
    [[ "$got" == "$exp" ]] || bad="threads=$th partitions=$pt got='$got' exp='$exp'"
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

# Composition I / P10 (the per-source reduction engine, now unconditional: the
# old SGPL_COMP_I_SOURCE_REDUCTION gate went away with the emitter unification):
# the per-source gather's visible result is written per source (`deg[u] = c`), so
# the plain per-loop reduction path is wrong for it -- that path folds all
# sources into one total and the epilogue never runs (degsum would come out 0).
# The nest must be classified `source-red`, keep the serial answer, and stay
# invariant to the partition/thread split.  bipartite.txt has 10 sources with no
# out-arcs, so the "no pairs" finish path is exercised by the expected 40.
if ( cd "$C" && GRAPH_FRONTIER_STATS=1 \
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
  ( cd "$C" && SGPL_NO_FRONTIER_MARKER=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
         GRAPH_FILE="$R/cases/algo/pagerank.graph" \
         bash ./03_run.sh >"$R/bin/marker_derived.log" 2>&1 </dev/null )
  trace_rc=$?
  if [[ "$trace_rc" -eq 137 ]]; then
    skip "race/marker_derived (final_program SIGKILLed rc=137 -- container memory limit)"
  elif [[ "$trace_rc" -eq 0 ]] &&
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

# P9b: derived verdicts are the *default* configuration -- the marker is no
# longer the veto.  The default build must reach the traversal verdicts from
# the analysis itself (an explicit call barrier in the trace), report the
# engine-owned regions from the metadata path, and never fall back on the
# marker.  1thr == 4thr pins that the derived set is race-free.
# NOTE: the previous "leaf loop gets DOALL" expectation predates the DAG-owner
# marker (step 10): the whole round nest, including its leaf loops, is now
# engine-owned and legitimately classified SEQUENTIAL -- the baseline binary at
# HEAD shows the same 0 DOALL lines, so this is a stale expectation, not a
# regression.  The derived-verdict evidence asserted here is the proof flag the
# classifier derives without any marker.
if compile "$R/cases/algo/pagerank.graph"; then
  ref=$(runt 1)
  ( cd "$C" && SGPL_LOOP_CLASSIFY_DEBUG=1 GRAPH_FILE="$R/cases/algo/pagerank.graph" \
         bash ./03_run.sh >"$R/bin/derived_default.log" 2>&1 </dev/null )
  trace_rc=$?
  if [[ "$trace_rc" -eq 137 ]]; then
    skip "race/derived_default (final_program SIGKILLed rc=137 -- container memory limit)"
  elif [[ "$trace_rc" -eq 0 ]]; then
    bad=""
    grep -q 'call barrier: stateful call' "$R/bin/derived_default.log" \
      || bad="no call-barrier evidence"
    grep -q 'dag-owned region' "$R/bin/derived_default.log" \
      || bad="$bad dag-owned region not reported"
    grep -q 'hasProofOfNoCarriedDeps=1' "$R/bin/derived_default.log" \
      || bad="$bad no derived proof verdict"
    if grep -q 'marker veto' "$R/bin/derived_default.log"; then
      bad="$bad marker veto active by default"
    fi
    for t in 1 4 4; do
      got=$(runt $t)
      [[ "$got" == "$ref" ]] || bad="$bad threads=$t got='$got' want='$ref'"
    done
    [[ -z "$bad" ]] && ok "race/derived_default (verdicts derived, no marker veto)" \
                    || no "race/derived_default" "$bad"
  else
    no "race/derived_default" "classification trace failed"
  fi
else
  no "race/derived_default" "build failed"
fi

# P9c: the scatter case with the frontier rewrite off -- the neighbour loops
# reach the PDG directly, and must say SEQUENTIAL *by derivation*: the iterator
# call is barred by its declared memory effects and the data-dependent
# subscript pairs land on an unknown carrier.  Answers at 1 and 4 threads pin
# that the derived verdict is the safe one.
if ( export GRAPH_FRONTIER_REWRITE_OFF=1; compile "$R/cases/algo/pagerank.graph" ); then
  ref=$(runt 1)
  if ( cd "$C" && GRAPH_FRONTIER_REWRITE_OFF=1 SGPL_LOOP_CLASSIFY_DEBUG=1 \
         GRAPH_FILE="$R/cases/algo/pagerank.graph" \
         bash ./03_run.sh >"$R/bin/derived_scatter.log" 2>&1 </dev/null ); then
    bad=""
    grep -q 'call barrier: stateful call autograph_neighbor_iter_next' "$R/bin/derived_scatter.log" \
      || bad="no iterator call-barrier evidence"
    grep -q 'hdr=foreach_nbr.cond87 depth=3 classification=SEQUENTIAL' "$R/bin/derived_scatter.log" \
      || bad="$bad scatter loop not SEQUENTIAL"
    if grep -q 'marker veto' "$R/bin/derived_scatter.log"; then
      bad="$bad marker veto active"
    fi
    for t in 1 4 4; do
      got=$( cd "$C" && GRAPH_FRONTIER_REWRITE_OFF=1 SGPL_NUM_THREADS=$t OMP_NUM_THREADS=$t \
               timeout 120 ./final_program 2>/dev/null </dev/null | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' )
      [[ "$got" == "$ref" ]] || bad="$bad threads=$t got='$got'"
    done
    [[ -z "$bad" ]] && ok "race/derived_scatter (iterator barrier + unknown carrier, 1thr==4thr)" \
                    || no "race/derived_scatter" "$bad"
  else
    no "race/derived_scatter" "classification trace failed"
  fi
else
  no "race/derived_scatter" "build failed"
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

# ---------------------------------------------------------------- GPU DEVICE
# Opt-in (SGPL_GPU_VERIFY=1): repeats the offload-target cases on a device when
# one is present.  The device answers must equal the CPU builds, the loop must
# actually launch (not silently fall back), and the same binary with no visible
# device must still answer through the CPU fallback.  Skips cleanly elsewhere.
if [[ "${SGPL_GPU_VERIFY:-0}" == "1" ]]; then
  echo "=== GPU DEVICE CHECK ==="
  if out=$(bash "$R/gpu_check.sh" array_doall gpu_compute gpu_compute_local algo/pagerank 2>&1); then
    echo "$out" | grep -E 'PASS|SKIP' | sed 's/^/  /'
    ok "gpu/device-check ($(grep -c 'PASS' <<<"$out") case(s) verified, $(grep -c 'SKIP' <<<"$out") skipped)"
  else
    echo "$out" | grep -E 'PASS|SKIP|FAIL' | sed 's/^/  /'
    no "gpu/device-check" "$(grep -m1 'FAIL' <<<"$out" | cut -c1-96)"
  fi
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
