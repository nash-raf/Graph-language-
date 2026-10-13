#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
java -jar generated/antlr-4.10.1-complete.jar -Dlanguage=Cpp -visitor Base.g4 -o generated/claims-new
for source in generated/claims-new/*; do
    target="generated/$(basename "$source")"
    if ! cmp -s "$source" "$target"; then cp "$source" "$target"; fi
done
bash build_lowmem/private_slot_fix/build.sh > proof/parallel_claims_2026_10_13/build.log 2>&1
gcc -O3 -c autotuner_runtime.c -o autotuner_runtime.o
gcc -O3 -c parallel_runtime.c -o parallel_runtime.o
bash test/run_parallel_claims_tests.sh > proof/parallel_claims_2026_10_13/regressions.log 2>&1
