#!/usr/bin/env bash
# Deterministic correctness contracts. No benchmarks or profiling.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NATIVE="${1:?Pass a compatible native runtime build directory}"
TMP="$(mktemp -d "$NATIVE/../storage-check.XXXXXX")"
trap 'if (($?)); then echo "GPU storage correctness logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
cd "$ROOT"
flags=(-O1 -g -fopenmp -fno-omit-frame-pointer)
link=(-fopenmp -no-pie)
if [[ "${SGPL_TEST_SANITIZE:-0}" != 0 ]]; then
 flags+=(-fsanitize=address,undefined)
 link+=(-fsanitize=address,undefined)
fi
for contract in preparation_reuse storage_transfer; do
 objects=()
 for name in runtime parallel_runtime gpu_runtime autotuner_runtime graph_mutation_runtime roaring_bitmap graph_loader_runtime graph_runtime semiring_runtime; do
  [[ "$contract" == preparation_reuse && "$name" == autotuner_runtime ]] && continue
  [[ "$contract" == storage_transfer && "$name" == gpu_runtime ]] && continue
  objects+=("$NATIVE/$name.o")
 done
 extra=(); wrap=()
 if [[ "$contract" == preparation_reuse ]]; then
  extra+=(-DSGPL_GPU_ALLOCATION_FAILURE_TEST)
  wrap+=(-Wl,--wrap=malloc -Wl,--wrap=calloc)
 fi
 gcc "${flags[@]}" "${extra[@]}" -iquote . -c "test/gpu_${contract}_test.c" -o "$TMP/$contract.o"
 g++ "${link[@]}" "$TMP/$contract.o" "${objects[@]}" "${wrap[@]}" -pthread -ldl -lnlopt -lm -o "$TMP/$contract"
 "$TMP/$contract"
done
