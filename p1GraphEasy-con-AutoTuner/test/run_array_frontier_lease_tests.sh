#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'status=$?; if ((status)); then echo "lease test artifacts: $TMP"; else rm -rf "$TMP"; fi' EXIT
cd "$ROOT"
gcc -O2 -std=gnu11 -pthread -I. test/array_frontier_lease_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/lease"
for n in 1 4 32; do SGPL_NUM_THREADS=$n SGPL_ALLOW_HT=1 "$TMP/lease"; done
gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -std=gnu11 -pthread -I. test/array_frontier_lease_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/lease_asan"
for n in 1 8; do ASAN_OPTIONS=detect_leaks=0 SGPL_NUM_THREADS=$n "$TMP/lease_asan"; done
