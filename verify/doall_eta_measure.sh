#!/usr/bin/env bash
# Online cost model: predicted vs measured, per thread count (the eta table).
#
# Subjects are the shapes that actually become cost-model sites (verified with
# GRAPH_PARALLEL_DEBUG=1):
#   DOALL     doall_scaling      -- a `for each vertex` loop, the shape the
#                                  outliner versions; tripped by the graph (|V|)
#   DOACROSS  doacross_while_d*  -- a carried `while` with a literal bound and a
#                                  statically sized array, classified DOACROSS
#   (array/while loops with opaque bounds get NO site at all -- measured -- so
#    they cannot be validation subjects; single-store per-vertex loops are
#    vectorized away and likewise get none.)
#
# For each configuration and P in {1,2,3,4}: min of 3 wall times, the answer,
# and one debug run from which the model's own numbers are parsed:
#   c_ns (per-iteration serial cost), L_thread_ns, L_path_ns, L_total_ns,
#   threshold, samples, c_state.
# The model's implied predictions (its own inequality, rearranged) are
#   pred_serial  = c_ns * N
#   pred_P       = L_total(P) + c_ns * N / P
# eta_P = pred_P / measured_P  (1.00x = perfect).
set -u
R=$(cd "$(dirname "$0")" && pwd)
C="$R/../p1GraphEasy-con-AutoTuner"
ulimit -s unlimited 2>/dev/null || true
mkdir -p "$R/bin/doall"
CSV="$R/bin/doall/eta.csv"
echo "fixture,graph,N,P,rounds,t_s,c_ns,L_thread_ns,L_path_ns,L_total_ns,speedup,threshold_trips,samples,c_state,choose,ans,site,decisions" > "$CSV"

parse(){
  python3 "$R/parse_cost_dbg.py" "$1" "$CSV" "$2" "$3" "$4" "$5" "$6" "$7" "$8"
}

best(){ # P -> min seconds of 3
  local P="$1" b=99999 v s2 e2
  for i in 1 2 3; do
    s2=$(date +%s%N)
    ( cd "$C" && SGPL_NUM_THREADS="$P" ./final_program >/dev/null 2>&1 )
    e2=$(date +%s%N)
    v=$(python3 -c "print(f'{(int($e2)-int($s2))/1e9:.6f}')")
    b=$(python3 -c "print(min($b,$v))")
  done
  echo "$b"
}
ans(){ ( cd "$C" && SGPL_NUM_THREADS="$1" ./final_program 2>/dev/null | tr '\n' ' ' | sed 's/ *$//' ); }

# Subject selection and thread list are parameterised for the pod run:
#   SGPL_ETA_P="1 2 3 4 8"   thread counts to measure
#   SGPL_ETA_SUBJECTS="..."  subset of the subjects below (default: all)
PLIST="${SGPL_ETA_P:-1 2 3 4}"
have_graph(){ [ -f "$1" ]; }

run_one(){ # fixture graphfile rounds N tag
  local fx="$1" gf="$2" rounds="$3" N="$4" tag="$5"
  local g="$R/bin/doall/${tag}.graph"
  sed -e "s|edges: file \"[^\"]*\"|edges: file \"$gf\"|" \
      -e "s/round < 200/round < $rounds/" \
      -e "s/\b8192\b/$N/g" "$R/cases/parallel/$fx.graph" > "$g"
  ( cd "$C" && rm -f final_program program.o gpu_runtime.o && \
    SGPL_GPU_BACKEND=0 GRAPH_FILE="$g" bash 03_run.sh ) >"$R/bin/doall/build.log" 2>&1
  if [ ! -f "$C/final_program" ]; then echo "$tag: BUILD FAILED"; return; fi
  for P in $PLIST; do
    local t a
    t=$(best "$P"); a=$(ans "$P")
    ( cd "$C" && GRAPH_PARALLEL_DEBUG=1 SGPL_NUM_THREADS="$P" ./final_program >/dev/null 2>"$R/bin/doall/dbg_${tag}_$P.txt" )
    parse "$R/bin/doall/dbg_${tag}_$P.txt" "$fx" "$(basename "$gf")" "$N" "$P" "$rounds" "$t" "$a"
    tail -1 "$CSV"
  done
}

SUBJ="${SGPL_ETA_SUBJECTS:-all}"

# ---- DOALL: doall_scaling (the shape that IS a cost-model site), tripped by the graph
if [ "$SUBJ" = "all" ] || [ "$SUBJ" = "doall" ]; then
  run_one doall_scaling "$R/fixtures/g20k.txt" 200 20000 doall_g20k
  [ -f "$R/../real_graphs/bio-grid-yeast.txt" ] && run_one doall_scaling "$R/../real_graphs/bio-grid-yeast.txt" 200 0 doall_bio
  [ -f "$R/../real_graphs/ia-dbpedia.txt" ] && run_one doall_scaling "$R/../real_graphs/ia-dbpedia.txt" 80 0 doall_dbpedia
  [ -f "/home/user/Course/msk1/benchmark/graphs/grapheasy/violin/synth_v_1000000_e_1000000_seed_11.txt" ] && run_one doall_scaling "/home/user/Course/msk1/benchmark/graphs/grapheasy/violin/synth_v_1000000_e_1000000_seed_11.txt" 80 0 doall_violin1m
fi

# ---- DOACROSS: N large enough that the loop dominates the measured time
if [ "$SUBJ" = "all" ] || [ "$SUBJ" = "doacross" ]; then
for N in ${SGPL_ETA_DOACROSS_N:-8192 32768}; do
  for d in 1 2 4; do
    run_one "${SGPL_ETA_DOACROSS_FIX:-doacross_while_d}$d" "$R/fixtures/g20k.txt" 200 "$N" "d${d}_N${N}"
  done
done
fi
echo "done"
