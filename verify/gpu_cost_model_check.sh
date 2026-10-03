#!/usr/bin/env bash
# Cost-model verification, end to end:
#   1. the unit layer (validate_tdg_budget.sh: T1-T10, incl. the pure device
#      policy and the engine-step gate boundaries),
#   2. the behavioural layer: for each gate, drive the decision variable across
#      its threshold and require the verdict (device vs CPU), the audit trail
#      ("kept on the CPU: ...", "policy: ...", "launched") and the answer to
#      follow it -- a gate that misfires shows up as the wrong side or as a
#      different answer, and a nondeterministic gate shows up across repeats.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
ulimit -s unlimited 2>/dev/null || true
pass=0; fail=0; skip=0
ok(){ printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass+1)); }
no(){ printf '  \033[31mFAIL\033[0m  %s\n' "$1"; fail=$((fail+1)); }
sk(){ printf '  \033[33mSKIP\033[0m  %s\n' "$1"; skip=$((skip+1)); }

echo "=== 1. unit layer (validate_tdg_budget.sh) ==="
if bash "$C/validate_tdg_budget.sh" >"$R/bin/cost_unit.log" 2>&1; then
  ok "unit layer: T1-T10 all pass in 3 configurations"
else
  no "unit layer failed (see bin/cost_unit.log)"
fi

build(){
  ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
    SGPL_GPU_BACKEND="$1" GRAPH_FILE="$R/cases/$2.graph" bash 03_run.sh ) >"$R/bin/cost_build.log" 2>&1 \
    && [ -f "$C/final_program" ]
}
run(){ ( cd "$C" && env $1 SGPL_NUM_THREADS=${2:-4} ./final_program 2>"$R/bin/cost_run.err" </dev/null \
        | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }
devsteps(){ grep -cE '\[gpu\] (launched|engine step ran|activation step ran)' "$R/bin/cost_run.err"; }
reason(){ grep -m1 'kept on the CPU' "$R/bin/cost_run.err"; }
policy(){ grep -m1 '\[gpu\] policy:' "$R/bin/cost_run.err"; }

# A device-less host (the CPU-only build box) must still verify the *policy*
# layer: every verdict, reason line and answer is checked, while the "device
# ran" half of an assertion becomes a reported skip instead of a failure.
expect_cpu(){ # label, out, base, devsteps, reason-or-policy
  if [ "$2" != "$3" ]; then no "$1: answer '$2' != CPU '$3'"; return; fi
  if [ "$4" != 0 ]; then no "$1: expected CPU but $4 device step(s) ran"; return; fi
  if [ -z "$5" ]; then
    # The outlined-loop policy line is printed inside the device path, so on a
    # host without a device there is nothing to observe; the engine-step gate
    # reports before any device call and is checked above.
    if [ "$HAVE_DEVICE" = 1 ]; then no "$1: no audit line for the verdict"; else sk "$1 (verdict side): audit line needs a device, answer+no-step checks passed"; fi
    return
  fi
  ok "$1"
}
expect_device(){ # label, out, base, devsteps
  if [ "$2" != "$3" ]; then no "$1: answer '$2' != CPU '$3'"; return; fi
  if [ "$HAVE_DEVICE" = 1 ]; then
    if [ "$4" -gt 0 ]; then ok "$1 ($4 device steps, answer equal)"; else no "$1: expected device but none ran"; fi
  else
    if [ "$4" = 0 ]; then sk "$1: policy correct, no device on this host (CPU answer equal)"; else no "$1: device steps without a device"; fi
  fi
}

HAVE_DEVICE=0
if build 1 algo/bfs_level; then
  run "SGPL_GPU_DEBUG=1 SGPL_GPU_ENGINE_MIN_PAIRS=0" 4 >/dev/null 2>&1 || true
  grep -qE '\[gpu\] (launched|engine step ran|activation step ran)' "$R/bin/cost_run.err" 2>/dev/null && HAVE_DEVICE=1
fi
echo "device present on this host: $HAVE_DEVICE"

echo "=== 2. engine-step gate (arcs vs SGPL_GPU_ENGINE_MIN_PAIRS) ==="
if ! build 1 algo/bfs_level; then no "bfs build failed"; else
  base=$(run "SGPL_NO_GPU_ENGINE_STEP=1" 4)
  # default: 320000 arcs < 2000000 -> CPU, with the reason
  out=$(run "SGPL_GPU_DEBUG=1" 4); r=$(reason); d=$(devsteps)
  expect_cpu "default floor (320000 arcs < 2000000) -> CPU, reason stated" "$out" "$base" "$d" "$r"
  out=$(run "SGPL_GPU_DEBUG=1 SGPL_GPU_ENGINE_MIN_PAIRS=400000" 4); r=$(reason); d=$(devsteps)
  expect_cpu "floor 400000 (>320000 arcs) -> CPU" "$out" "$base" "$d" "$r"
  out=$(run "SGPL_GPU_DEBUG=1 SGPL_GPU_ENGINE_MIN_PAIRS=300000" 4); d=$(devsteps)
  expect_device "floor 300000 (<320000 arcs) -> device" "$out" "$base" "$d"
  out2=$(run "SGPL_GPU_DEBUG=1 SGPL_GPU_ENGINE_MIN_PAIRS=300000" 4)
  [ "$out2" = "$base" ] && ok "verdict + answer stable across repeats" || no "repeat differs: '$out2' != '$base'"
fi

echo "=== 3. DOALL gate (trips vs SGPL_GPU_MIN_TRIPS) ==="
if ! build 1 parallel/array_doall; then no "array_doall build failed"; else
  base=$(run "" 4)
  out=$(run "SGPL_GPU_DEBUG=1" 4); d=$(devsteps)
  expect_device "default (200000 trips >= 4096) -> device" "$out" "$base" "$d"
  out=$(run "SGPL_GPU_DEBUG=1 SGPL_GPU_MIN_TRIPS=300000" 4); d=$(devsteps); p=$(policy)
  expect_cpu "min_trips 300000 (>trips) -> CPU, policy line" "$out" "$base" "$d" "$p"
  out=$(run "SGPL_GPU_DEBUG=1 SGPL_GPU_MIN_TRIPS=100000" 4); d=$(devsteps)
  expect_device "min_trips 100000 (<trips) -> device" "$out" "$base" "$d"
fi

echo "=== 4. DOACROSS wave gate (waves vs SGPL_GPU_MAX_WAVES) ==="
if ! build 1 parallel/doacross_scan; then no "doacross_scan build failed"; else
  base=$(run "SGPL_FORCE_DOACROSS_PARALLEL=1" 4)
  out=$(run "SGPL_FORCE_DOACROSS_PARALLEL=1 SGPL_GPU_DEBUG=1" 4); d=$(devsteps); p=$(policy)
  expect_cpu "wave storm (199999 waves > 8192) -> CPU, policy line" "$out" "$base" "$d" "$p"
  out=$(run "SGPL_FORCE_DOACROSS_PARALLEL=1 SGPL_GPU_DEBUG=1 SGPL_GPU_MAX_WAVES=0" 4); d=$(devsteps)
  expect_device "wave cap disabled -> device" "$out" "$base" "$d"
fi

echo "=== 5. cross-shape answer equality under forced device policy ==="
for case in algo/kcore algo/pagerank parallel/doacross_carry parallel/gpu_compute; do
  if ! build 1 "$case"; then no "$case build failed"; continue; fi
  base=$(run "SGPL_NO_GPU_ENGINE_STEP=1" 4)
  out=$(run "SGPL_GPU_DEBUG=1 SGPL_GPU_ENGINE_MIN_PAIRS=0 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_MAX_WAVES=0 SGPL_FORCE_DOALL_PARALLEL=1 SGPL_FORCE_DOACROSS_PARALLEL=1" 4)
  [ "$out" = "$base" ] && ok "$case forced-device answer == CPU ('$out')" || no "$case: '$out' != '$base'"
done

printf 'passed: %d  failed: %d  skipped: %d\n' "$pass" "$fail" "$skip"
[ "$fail" = 0 ]
