#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)";cd "$ROOT"
TMP="$(mktemp -d)"
trap 'status=$?; if ((status));then echo "append epoch test artifacts: $TMP";else rm -rf "$TMP";fi' EXIT
for maximum in 3 4294967295;do
 gcc -O2 -std=gnu11 -pthread -I. -DSGPL_EXEC_APPEND_EPOCH_MAX=$maximum test/array_append_epoch_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/epoch"
 for workers in 1 4 32;do SGPL_NUM_THREADS=$workers SGPL_ALLOW_HT=1 "$TMP/epoch";done
done
gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -std=gnu11 -pthread -I. -DSGPL_EXEC_APPEND_EPOCH_MAX=3 test/array_append_epoch_test.c autotuner_runtime.c parallel_runtime.c -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/epoch_asan"
ASAN_OPTIONS=detect_leaks=0 SGPL_NUM_THREADS=4 "$TMP/epoch_asan"
