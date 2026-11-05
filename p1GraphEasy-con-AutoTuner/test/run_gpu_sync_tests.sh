#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NATIVE="${1:?Pass an existing compatible native runtime build directory}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cd "$ROOT"
objects=()
for name in runtime parallel_runtime autotuner_runtime graph_mutation_runtime roaring_bitmap graph_loader_runtime graph_runtime semiring_runtime; do
 objects+=("$NATIVE/$name.o")
done
# Include the current GPU runtime with stubs; use the real DAG implementation.
gcc -O1 -fopenmp -iquote . -c test/gpu_sync_elision_test.c -o "$TMP/test.o"
g++ -fopenmp -no-pie "$TMP/test.o" "${objects[@]}" -pthread -ldl -lnlopt -lm -o "$TMP/sync"
"$TMP/sync"
