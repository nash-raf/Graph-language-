#!/usr/bin/env bash
# Cost-model sweep: measure device vs CPU across sizes with the gates DISABLED,
# so the empirical crossover can be compared against the defaults
# (SGPL_GPU_MIN_TRIPS=4096, SGPL_GPU_MAX_WAVES=8192, SGPL_GPU_ENGINE_MIN_PAIRS=2M).
#
#   for each size: CPU-only build (median of 3 runs) vs GPU build with the gate
#   dropped (median of 3), warm stopped, answers compared;
#   rows are appended to cost_model_evidence.csv in the same schema, so
#   plot_cost_model.py renders them.
#
# Needs a GPU host.  Usage: bash gpu_cost_sweep.sh [family ...]
#   families: doall doacross engine   (default: all)
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
CSV="$R/cost_model_evidence.csv"
ulimit -s unlimited 2>/dev/null || true
FAMILIES="${*:-doall doacross engine}"
mkdir -p "$R/bin/sweep"

WARMWASRUNNING=0
ps -eo cmd | grep -qE "[m]m_warm.py" && WARMWASRUNNING=1
touch /tmp/mm_warm.stop 2>/dev/null || true
for i in $(seq 20); do ps -eo cmd | grep -qE "[m]m_warm.py" || break; sleep 3; done
restore_warm(){ rm -f /tmp/mm_warm.stop; }
trap restore_warm EXIT

build(){ ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
           SGPL_GPU_BACKEND="$1" GRAPH_FILE="$2" bash 03_run.sh ) >"$R/bin/sweep/build.log" 2>&1 \
           && [ -f "$C/final_program" ]; }
time3(){ # env, threads -> median ms
  local env="$1" thr="$2" t=()
  for i in 1 2 3; do
    local s=$(date +%s%N)
    ( cd "$C" && env $env SGPL_NUM_THREADS="$thr" timeout 900 ./final_program >/dev/null 2>&1 </dev/null )
    local e=$(date +%s%N); t+=($(( (e-s)/1000000 )))
  done
  printf '%s\n' "${t[@]}" | sort -n | sed -n 2p
}
answer(){ ( cd "$C" && env $1 SGPL_NUM_THREADS=4 timeout 900 ./final_program 2>/dev/null </dev/null \
            | grep -v AutoTuner | tr '\n' ' ' | sed 's/ *$//' ); }

emit(){ # family case units work cpu gpu ranondev gate_default gate_correct
  printf '%s,%s,%s,%s,1,%s,%s,%s,%s,%s,%s\n' \
    "$1" "$2" "$3" "$4" "${7:-}" "$5" "$6" \
    "$(python3 -c "print('%.2f'%($5/max(1,$6)))")" "$8" "${9:--}" >> "$CSV"
}

CASE_DOALL="$R/cases/parallel/array_doall.graph"
CASE_DOACROSS="$R/cases/parallel/doacross_carry.graph"
DOACROSS_ENV="SGPL_FORCE_DOACROSS_PARALLEL=1"
GATE_OFF="SGPL_GPU_MIN_TRIPS=0 SGPL_GPU_MAX_WAVES=0 SGPL_GPU_ENGINE_MIN_PAIRS=0"

for fam in $FAMILIES; do
  case "$fam" in
    doall)    sizes="1000 2000 4096 8192 16384 65536 262144 1048576"; tmpl="$CASE_DOALL"; var=int ;;
    doacross) sizes="512 1024 2048 4096 8192 16384 65536";           tmpl="$CASE_DOACROSS"; var=8192 ;;
    engine)   sizes=""; tmpl="" ;;
  esac
  if [ "$fam" = engine ]; then
    for spec in "algo/bfs_level:fixtures/g20k.txt:160000" \
                "algo/kcore:fixtures/g20k.txt:160000" \
                "algo/bfs_level:real_graphs/bio-grid-yeast.txt:156945" \
                "algo/kcore:real_graphs/bio-grid-yeast.txt:156945"; do
      case="${spec%%:*}"; rest="${spec#*:}"; g="${rest%%:*}"; edges="${rest##*:}"
      src="$R/cases/$case.graph"; out="$R/bin/sweep/$(echo $case | tr / _)_$(basename $g).graph"
      sed -E "s|[^\" ]*fixtures/g20k\.txt|$R/../$g|" "$src" > "$out"
      grep -q "$g" "$out" || { echo "$case/$g: rewrite failed"; continue; }
      echo "--- engine $case on $g ($edges edges)"
      if ! build 0 "$out"; then echo "  CPU build failed"; continue; fi
      cpu=$(time3 "" 4); ansA=$(answer "")
      if ! build 1 "$out"; then echo "  GPU build failed"; continue; fi
      gpu=$(time3 "$GATE_OFF" 4); ansB=$(answer "$GATE_OFF")
      if [ "$ansA" != "$ansB" ]; then echo "  ANSWER MISMATCH: '$ansA' vs '$ansB'"; continue; fi
      emit engine-step "$case ($(basename $g))" arcs "$((2*edges))" "$cpu" "$gpu" "engine step" CPU 1
      echo "  cpu=${cpu}ms device=${gpu}ms"
    done
    continue
  fi
  for n in $sizes; do
    out="$R/bin/sweep/${fam}_$n.graph"
    if [ "$var" = int ]; then
      sed -E "s/int n = [0-9]+;/int n = $n;/" "$tmpl" > "$out"
    else
      sed -E "s/\b8192\b/$n/g" "$tmpl" > "$out"
    fi
    echo "--- $fam n=$n"
    envx=""; [ "$fam" = doacross ] && envx="$DOACROSS_ENV"
    if ! build 0 "$out"; then echo "  CPU build failed"; continue; fi
    cpu=$(time3 "$envx" 4); ansA=$(answer "$envx")
    if ! build 1 "$out"; then echo "  GPU build failed"; continue; fi
    gpu=$(time3 "$envx $GATE_OFF" 4); ansB=$(answer "$envx $GATE_OFF")
    if [ "$ansA" != "$ansB" ]; then echo "  ANSWER MISMATCH: '$ansA' vs '$ansB'"; continue; fi
    emit "$fam" "$fam n=$n" trip "$n" "$cpu" "$gpu" "outlined kernel" device 1
    echo "  cpu=${cpu}ms device=${gpu}ms answer='$ansA'"
  done
done
echo "rows appended to $CSV"
