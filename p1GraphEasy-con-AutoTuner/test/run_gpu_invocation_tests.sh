#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
gcc -O1 -w -iquote . -pthread -DSGPL_GPU_MAX_TASKS=32 test/gpu_invocation_test.c gpu_runtime.c autotuner_runtime.c parallel_runtime.c \
 -ldl -lnlopt -lm -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=pthread_create -o "$TMP/gpu-invocation"
for workers in 1 4; do
 timeout 60 env SGPL_NUM_THREADS="$workers" SGPL_GPU_MIN_TRIPS=0 "$TMP/gpu-invocation"
done
