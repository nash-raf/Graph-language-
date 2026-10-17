#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
TMP="$(mktemp -d)";trap 'rm -rf "$TMP"' EXIT
for mode in normal sanitized;do
 flags=(-O2)
 [[ "$mode" == sanitized ]] && flags=(-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
 gcc "${flags[@]}" -std=gnu11 -pthread -I. test/empty_partition_effects_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/test"
 for workers in 1 4 32;do ASAN_OPTIONS=detect_leaks=0 SGPL_NUM_THREADS=$workers SGPL_ALLOW_HT=1 "$TMP/test";done
done
