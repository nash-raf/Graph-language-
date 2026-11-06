#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "lifecycle test artifacts: $TMP"; else rm -rf "$TMP"; fi' EXIT
LLVM_CONFIG="${LLVM_CONFIG:-llvm-config-20}"
g++ -O0 -Wno-deprecated-declarations $($LLVM_CONFIG --cxxflags) -iquote "$ROOT" \
  "$ROOT/test/frontier_lifecycle_test.cpp" \
  $($LLVM_CONFIG --ldflags --libs all --system-libs) -pthread -o "$TMP/lifecycle"
"$TMP/lifecycle"
