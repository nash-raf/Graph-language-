#!/usr/bin/env bash
# Broad cross-mode equivalence over the whole fixture corpus.  Every graph in
# verify/cases is built twice -- CPU-only IR backend and GPU IR backend -- and
# every configuration must reproduce the CPU-only 1-thread answer exactly:
#
#   CPU {1,4}thr, device-forced GPU {1,4}thr x2 repeats
#   parallel/algo fixtures additionally at SGPL_CLEANCUT_PARTITIONS {1,3,7}
#   (partition boundaries move the ownership edges: a race or a wrong
#    ownership assumption shows up as a differing answer)
#
# The device paths are forced past the cost policy so the kernels actually run;
# the policy itself is measured by gpu_cost_model_check.sh.
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
ulimit -s unlimited 2>/dev/null || true
pass=0; fail=0
ok(){ printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass+1)); }
no(){ printf '  \033[31mFAIL\033[0m  %s\n' "$1"; fail=$((fail+1)); }
mkdir -p "$R/bin/broad"

FORCE="SGPL_GPU_ENGINE_MIN_PAIRS=0 SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_MAX_WAVES=0 SGPL_FORCE_DOALL_PARALLEL=1 SGPL_FORCE_DOACROSS_PARALLEL=1"

build(){ ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
           SGPL_GPU_BACKEND="$1" GRAPH_FILE="$R/cases/$2.graph" bash 03_run.sh ) \
           >"$R/bin/broad/build.log" 2>&1 && [ -f "$C/final_program" ]; }
run(){ local env="$1" thr="$2"; ( cd "$C" && env $env SGPL_NUM_THREADS="$thr" timeout 120 ./final_program 2>/dev/null </dev/null \
        | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }

mapfile -t cases < <(cd "$R/cases" && find . -name '*.graph' | sed 's|^\./||; s|\.graph$||' | sort)
echo "fixtures: ${#cases[@]}"
for case in "${cases[@]}"; do
  slug=$(echo "$case" | tr / _)
  if ! build 0 "$case"; then no "$case: CPU-only build failed"; continue; fi
  base=$(run "" 1)
  c4=$(run "" 4)
  if [ "$base" != "$c4" ]; then no "$case: CPU 1thr '$base' != 4thr '$c4'"; continue; fi
  if ! build 1 "$case"; then no "$case: GPU build failed"; continue; fi
  g1a=$(run "$FORCE" 1); g1b=$(run "$FORCE" 1)
  g4a=$(run "$FORCE" 4); g4b=$(run "$FORCE" 4)
  bad=""
  [ "$g1a" = "$base" ] || bad="device 1thr '$g1a'"
  [ "$g4a" = "$base" ] || bad="$bad device 4thr '$g4a'"
  [ "$g1a" = "$g1b" ]  || bad="$bad repeat-1thr '$g1b'"
  [ "$g4a" = "$g4b" ]  || bad="$bad repeat-4thr '$g4b'"
  if [ -n "$bad" ]; then no "$case: expected '$base' got:$bad"; continue; fi
  # partition-count sweep: moves the ownership boundaries
  if [[ "$case" == parallel/* || "$case" == algo/* ]]; then
    for part in 1 3 7; do
      for mode in "cpu:" "gpu:$FORCE"; do
        lbl=${mode%%:*}; envx=${mode#*:}
        out=$(run "$envx SGPL_CLEANCUT_PARTITIONS=$part" 4)
        if [ "$out" != "$base" ]; then bad="$bad $lbl/part=$part '$out'"; fi
      done
    done
    if [ -n "$bad" ]; then no "$case: partitions: expected '$base' got:$bad"; continue; fi
  fi
  ok "$case (cpu 1=4; device 1=4=repeats; device==CPU; partitions sweep)"
done
printf 'passed: %d  failed: %d\n' "$pass" "$fail"
[ "$fail" = 0 ]
