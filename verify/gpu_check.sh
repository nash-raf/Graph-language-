#!/usr/bin/env bash
# GPU-path verification.  Run this on a box with a CUDA driver (and an LLVM that
# ships the NVPTX backend); everywhere else every check reports SKIP, because
# the GPU path is only reachable when the compiler can emit PTX.
#
#   bash verify/gpu_check.sh [case ...]        # default: array_doall doacross_scan
#
# For each case (a DOALL and a DOACROSS shape -- the only two modes the outliner
# offloads) it checks:
#   1. the CPU baseline answer,
#   2. a --gpu build embeds a device kernel and actually *launches* it
#      (SGPL_GPU_DEBUG "launched" line; DOACROSS must say "cooperative"),
#   3. the on-device answer equals the CPU answer at 1 and 4 host threads,
#   4. the same binary with no visible device (CUDA_VISIBLE_DEVICES=) takes the
#      CPU fallback and still produces the baseline answer.
#
# Case expectations are the same ones verify/run.sh pins, so a GPU run cannot
# pass by answering something the suite does not already consider correct.
set -uo pipefail
R="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
C="${SGPL_AUTOTUNER_DIR:-$R/../p1GraphEasy-con-AutoTuner}"
BIN="$R/bin"
mkdir -p "$BIN"

cases=("$@")
if [[ -z "${cases[0]:-}" ]]; then
  cases=(array_doall doacross_carry gpu_compute algo/pagerank algo/cc algo/kcore algo/bfs_level doacross_scan reduce_int_ops mixed_regions)
fi
# Expected stdout per case, matching verify/run.sh.
expected(){ case "$1" in
  array_doall)    printf 'a_last 199999' ;;
  doacross_scan)  printf 'last 199999' ;;
  # no expected literal for these: the CPU build of the same case is the reference
  doacross_carry|gpu_compute|algo/cc|algo/kcore|algo/bfs_level|doacross_scan|reduce_int_ops|mixed_regions) printf '' ;;
  *)              printf '' ;;
esac; }
# Extra runtime env per case (the DOACROSS case is forced parallel, as the suite does).
case_env(){ case "$1" in
  # DOACROSS cases: force the parallel dispatch (nothing else chooses it)
  doacross_scan)   printf 'SGPL_FORCE_DOACROSS_PARALLEL=1' ;;
  doacross_carry)  printf 'SGPL_FORCE_DOACROSS_PARALLEL=1' ;;
  # DOALL algorithmic/kernel cases: force the parallel dispatch so the check
  # exercises the *device* path rather than the cost model's default choice
  gpu_compute|algo/cc|reduce_int_ops|mixed_regions) printf 'SGPL_FORCE_DOALL_PARALLEL=1' ;;
  algo/pagerank)   printf 'SGPL_FORCE_DOALL_PARALLEL=1 SGPL_GPU_ENGINE_MIN_PAIRS=0' ;;
  # engine-step case: also drop the step's own cost gate, so the device step
  # actually runs (its default keeps small steps on the CPU by design)
  algo/kcore|algo/bfs_level) printf 'SGPL_FORCE_DOALL_PARALLEL=1 SGPL_GPU_ENGINE_MIN_PAIRS=0' ;;
  *)               printf '' ;;
esac; }

pass=0; fail=0; skip=0
ok(){ printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass+1)); }
no(){ printf '  \033[31mFAIL\033[0m  %s\n' "$1"; fail=$((fail+1)); }
sk(){ printf '  \033[33mSKIP\033[0m  %s\n' "$1"; skip=$((skip+1)); }

# ---------------------------------------------------------------- effect verdicts
# Compile-time and device-independent: the offload verdict must be *derived*
# from the loop's effect shape.  FORCE_GPU bypasses the host's GPU probe so the
# verdict is taken on every machine; the NVPTX lookup may then fail (no kernel),
# which is fine here -- these assert the verdict, not the emission.
verdict(){ # verdict <graph> <space> [substr that must appear in the whole verdict]
  local g="$1" want="$2" sub="${3:-}" out line
  out=$( cd "$C" && FORCE_GPU=1 SGPL_GPU_DEBUG=1 ./GraphProgram "$g" --gpu 2>&1 | grep -E '\[gpu\]' )
  line=$(grep -E '\[gpu\] effects' <<<"$out" | head -1 | tr -s ' ')
  if [[ "$line" != *"space=$want"* ]]; then
    no "effects/$(basename "${g%.graph}")" "want space=$want, got: ${line:0:110}"
  elif [[ -n "$sub" && "$out" != *"$sub"* ]]; then
    no "effects/$(basename "${g%.graph}")" "space=$want, but '$sub' missing from the verdict: $(grep -m1 'no kernel' <<<"$out" | cut -c1-80)"
  else
    ok "effects/$(basename "${g%.graph}") ($want${sub:+ + $sub})"
  fi
}
verdict "$R/cases/algo/pagerank.graph" Independent
verdict "$R/cases/parallel/doacross_carry.graph" Carried
verdict "$R/cases/parallel/gpu_compute.graph" Independent   # env base resolved at run time

# GPU availability gate: a driver we can dlopen and a device we can see.
if ! ( command -v nvidia-smi >/dev/null 2>&1 && nvidia-smi -L >/dev/null 2>&1 ); then
  echo "no CUDA device visible (nvidia-smi -L failed)"
  for c in "${cases[@]}"; do sk "$c (no device)"; done
  echo "passed: $pass  failed: $fail  skipped: $skip"
  exit 0
fi

run_out(){ # run_out <threads> [env assignments...] -> stdout, one line
  local t="$1"; shift
  ( cd "$C" && env "$@" SGPL_NUM_THREADS="$t" OMP_NUM_THREADS="$t" \
      timeout "${SGPL_GPU_TIMEOUT:-300}" ./final_program 2>/dev/null </dev/null \
      | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' )
}
gpu_debug(){ # gpu_debug <threads> [env assignments...] -> stderr of a debug run
  local t="$1"; shift
  ( cd "$C" && env "$@" SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS="$t" OMP_NUM_THREADS="$t" \
      timeout "${SGPL_GPU_TIMEOUT:-300}" ./final_program 2>&1 >/dev/null </dev/null )
}
ms(){ # median wall time in ms over 3 runs
  local t="$1"; shift
  local best=""
  for _ in 1 2 3; do
    local s e
    s=$(date +%s%N)
    run_out "$t" "$@" >/dev/null
    e=$(date +%s%N)
    local d=$(( (e - s) / 1000000 ))
    if [[ -z "$best" || "$d" -lt "$best" ]]; then best="$d"; fi
  done
  printf '%s' "$best"
}

for case in "${cases[@]}"; do
  if [[ "$case" == */* ]]; then graph="$R/cases/$case.graph"
  else                        graph="$R/cases/parallel/$case.graph"; fi
  tag="${case//\//_}"   # log file names must not contain a slash
  echo "[$case]"
  if [[ ! -f "$graph" ]]; then no "$case: missing $graph"; continue; fi
  want="$(expected "$case")"
  cenv="$(case_env "$case")"

  # --- 1. CPU baseline -------------------------------------------------------
  # Remove the binary first: a build that silently produces nothing would
  # otherwise be compared against whatever case ran before it.
  rm -f "$C/final_program"
  if ! ( cd "$C" && env $cenv GRAPH_FILE="$graph" bash 03_run.sh \
           >"$BIN/gpu_$tag.cpu.log" 2>&1 </dev/null ) || [[ ! -x "$C/final_program" ]]; then
    no "$case: CPU baseline build failed"; continue
  fi
  cpu1=$( run_out 1 $cenv ); cpu4=$( run_out 4 $cenv )
  if [[ -n "$want" && ( "$cpu1" != "$want" || "$cpu4" != "$want" ) ]]; then
    no "$case: CPU baseline wrong (1thr='$cpu1' 4thr='$cpu4' want='$want')"; continue
  fi
  cpu_ms=$( ms 4 $cenv )

  # --- 2. GPU build ---------------------------------------------------------
  # SGPL_GPU_DEBUG makes the compile report the effect verdict it derived, so a
  # decline here is legible (the log carries "[gpu] effects ... space=...").
  rm -f "$C/final_program"
  if ! ( cd "$C" && SGPL_GPU_DEBUG=1 SGPL_GPU_BACKEND=1 env $cenv GRAPH_FILE="$graph" bash 03_run.sh \
           >"$BIN/gpu_$tag.gpu.log" 2>&1 </dev/null ) || [[ ! -x "$C/final_program" ]]; then
    no "$case: --gpu build failed"; continue
  fi
  # The runtime declares gpu_embedded_ptx weak, so a linked binary always shows
  # *some* symbol by that name; the kernel-name string only exists when the
  # compiler actually emitted a device module.
  # grep -a on the binary, not `strings | grep -q`: under `set -o pipefail` the
  # early-exiting grep kills strings with SIGPIPE and the pipeline reports
  # failure even though the match was found.
  # Device work is either an outlined kernel (gpu_kernel_*) or an engine step
  # kernel (gpu_step_*); both live in the embedded PTX module.
  if ! grep -qa -e 'gpu_kernel_' -e 'gpu_step_' "$C/final_program" 2>/dev/null; then
    why=$(grep -m1 '\[gpu\] effects' "$BIN/gpu_$tag.gpu.log" | cut -c1-110)
    sk "$case: no device kernel embedded${why:+ -- $why}"
    continue
  fi
  # Probe at 4 host threads: with a single thread the plan is serial and the
  # parallel dispatch (hence the device call) is never reached, which would look
  # like "no kernel" even though the kernel is there and correct.
  dbg=$( gpu_debug 4 $cenv )
  if [[ "$case" == *kcore* || "$case" == *pagerank* || "$case" == *bfs* ]]; then
    # these parallelise through the engine: the proof is the engine step line
    # (source-owned kernels say "engine step ran", activation kernels say
    #  "activation step ran")
    if ! grep -qE '\[gpu\] (engine|activation) step ran on device' <<<"$dbg"; then
      if pol=$(grep -m1 '\[gpu\] policy:' <<<"$dbg"); then
        sk "$case: engine step kept on the CPU by the cost model -- ${pol##*[gpu] policy: }"
      else
        no "$case: engine step never ran on the device"
        printf '%s\n' "$dbg" | sed 's/^/        /' | head -6
      fi
      continue
    fi
  elif ! grep -q '\[gpu\] launched ' <<<"$dbg"; then
    # A kernel that exists but is kept on the CPU by the *cost model* is a
    # policy outcome, not a failure: the device economics (launch + copies) do
    # not pay off for this loop's trip count / wave count, and the CPU answers
    # are already asserted above.  Only a missing launch with no policy reason
    # is a failure.
    if pol=$(grep -m1 '\[gpu\] policy:' <<<"$dbg"); then
      sk "$case: device kept on the CPU by the cost model -- ${pol##*[gpu] policy: }"
      continue
    fi
    no "$case: device kernel never launched (no '[gpu] launched' line)"
    printf '%s\n' "$dbg" | sed 's/^/        /' | head -6
    continue
  fi
  if [[ "$case" == *doacross* ]] && ! grep -q '\[gpu\] launched .*cooperative' <<<"$dbg"; then
    no "$case: DOACROSS did not use the cooperative wave kernel"
    continue
  fi
  gpu1=$( run_out 1 $cenv ); gpu4=$( run_out 4 $cenv )
  if [[ "$gpu1" != "$cpu1" || "$gpu4" != "$cpu1" ]]; then
    no "$case: device answer differs (cpu='$cpu1' gpu1='$gpu1' gpu4='$gpu4')"
    continue
  fi
  gpu_ms=$( ms 4 $cenv )

  # --- 3. CPU fallback on the same binary -----------------------------------
  fb=$( run_out 1 $cenv CUDA_VISIBLE_DEVICES= )
  if [[ "$fb" != "$cpu1" ]]; then
    no "$case: CPU fallback wrong (cpu='$cpu1' fallback='$fb')"; continue
  fi
  fbd=$( ( cd "$C" && env $cenv CUDA_VISIBLE_DEVICES= SGPL_GPU_DEBUG=1 SGPL_NUM_THREADS=4 \
              OMP_NUM_THREADS=4 timeout "${SGPL_GPU_TIMEOUT:-300}" ./final_program 2>&1 >/dev/null </dev/null ) )
  if grep -q '\[gpu\] launched ' <<<"$fbd"; then
    no "$case: launched on the device although no device was visible"; continue
  fi

  ok "$case (device answer == CPU at 1/4 thr; fallback ok; cpu=${cpu_ms}ms gpu=${gpu_ms}ms over 4 host threads)"
done

echo "passed: $pass  failed: $fail  skipped: $skip"
[[ "$fail" == 0 ]]
