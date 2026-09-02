#!/usr/bin/env bash
# Validate composable effect algebra: DualOwner, Temporal, U_f vs U_op.
set -uo pipefail
cd /mnt/d/sgpl/bench/p1GraphEasy-con-AutoTuner
export OMP_NUM_THREADS=4
export SGPL_ROARING_THREADS=2
ulimit -s unlimited 2>/dev/null || true

echo "=== compile runtimes ==="
gcc -O3 -c autotuner_runtime.c -o autotuner_runtime.o
gcc -O3 -c graph_mutation_runtime.c -o graph_mutation_runtime.o
gcc -O3 -c parallel_runtime.c -o parallel_runtime.o
if [[ -f gpu_runtime.c ]]; then
  gcc -O3 -c gpu_runtime.c -o gpu_runtime.o 2>/dev/null || true
fi
gcc -O3 -fopenmp -iquote . -c semiring_runtime.c -o semiring_runtime.o 2>/dev/null || true
gcc -O3 -c runtime.c -o runtime.o
g++ -O3 -mavx2 -march=native -fopenmp -c roaring_bitmap.cpp -o roaring_bitmap.o
g++ -O2 -std=c++17 -fopenmp -c graph_loader_runtime.cpp -o graph_loader_runtime.o
g++ -O2 -std=c++17 -c graph_runtime.cpp -o graph_runtime.o

link_prog() {
  local tag="$1"
  local extra=()
  [[ -f gpu_runtime.o ]] && extra+=(gpu_runtime.o)
  [[ -f semiring_runtime.o ]] && extra+=(semiring_runtime.o)
  g++ -O3 -fopenmp -no-pie program.o runtime.o parallel_runtime.o autotuner_runtime.o \
    graph_mutation_runtime.o roaring_bitmap.o graph_loader_runtime.o graph_runtime.o \
    "${extra[@]}" -ldl -lnlopt -o "final_$tag"
}

compile_rw() {
  local graph="$1" tag="$2" rewrite="$3"
  rm -f program.o
  if [[ "$rewrite" == "1" ]]; then
    GRAPH_FRONTIER_REWRITE=1 GRAPH_FRONTIER_STATS=1 GRAPH_FRONTIER_VERBOSE=1 \
      ./GraphProgram --ir-backend=cpu "$graph" >"/tmp/${tag}.stdout" 2>"/tmp/${tag}.stats"
  else
    ./GraphProgram --ir-backend=cpu "$graph" >"/tmp/${tag}.stdout" 2>"/tmp/${tag}.stats"
  fi
  link_prog "$tag" || { echo "LINK FAIL $tag"; return 1; }
  "./final_$tag" 2>/dev/null | grep -E '^-?[0-9]' > "/tmp/${tag}.nums" || true
}

echo "=== pagerank_100k regression (rewrite on) ==="
compile_rw synthetic/pagerank_100k.graph pr100k 1
echo -n "pr100k nums: "; cat /tmp/pr100k.nums
grep -E 'class=' /tmp/pr100k.stats | head -20

echo "=== pagerank_dataidx ==="
compile_rw pagerank_dataidx.graph didx 1
echo -n "didx nums: "; cat /tmp/didx.nums
grep 'graph-frontier] candidate' /tmp/didx.stats

echo "=== reductions ==="
for k in reduce_add reduce_sub reduce_min reduce_max reduce_mul; do
  compile_rw "${k}.graph" "$k" 1
  echo -n "$k nums: "; cat /tmp/${k}.nums
  grep 'class=' /tmp/${k}.stats | head -3
done

echo "=== inplace_min ==="
compile_rw inplace_min.graph ipmin 1
echo -n "ipmin nums: "; cat /tmp/ipmin.nums
grep 'graph-frontier] candidate' /tmp/ipmin.stats

echo "=== small BFS/SSSP/CC/kcore rewrite vs serial ==="
for k in small_bfs small_sssp small_cc small_kcore; do
  compile_rw "${k}.graph" "${k}_rw" 1
  compile_rw "${k}.graph" "${k}_ser" 0
  echo "---- $k ----"
  grep 'graph-frontier] candidate' /tmp/${k}_rw.stats || true
  echo -n "rw : "; cat /tmp/${k}_rw.nums
  echo -n "ser: "; cat /tmp/${k}_ser.nums
  if ! diff -q /tmp/${k}_rw.nums /tmp/${k}_ser.nums >/dev/null; then
    echo "MISMATCH $k rewrite vs serial"
  else
    echo "MATCH $k"
  fi
  echo "x3 rewrite:"
  for i in 1 2 3; do "./final_${k}_rw" 2>/dev/null | grep -E '^-?[0-9]'; done
done

echo "=== dual_uv (distinct arrays) ==="
compile_rw dual_uv.graph dual_rw 1
compile_rw dual_uv.graph dual_ser 0
grep 'graph-frontier] candidate' /tmp/dual_rw.stats || true
echo -n "rw : "; cat /tmp/dual_rw.nums
echo -n "ser: "; cat /tmp/dual_ser.nums
diff -q /tmp/dual_rw.nums /tmp/dual_ser.nums && echo MATCH dual_uv || echo MISMATCH dual_uv

echo "=== mixed_same_array (must sequential) ==="
compile_rw mixed_same_array.graph mix 1
grep 'graph-frontier] candidate' /tmp/mix.stats || true

echo "=== cross_dep B[v]=A[u] (must sequential) ==="
compile_rw cross_dep.graph xdep 1
grep 'graph-frontier] candidate' /tmp/xdep.stats || true

echo VALIDATE_COMPOSITION_DONE
