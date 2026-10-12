#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'if (($?)); then echo "static proof logs: $TMP"; else rm -rf "$TMP"; fi' EXIT
LLVM_CONFIG="${LLVM_CONFIG:-llvm-config-20}"
flags="$($LLVM_CONFIG --cxxflags)"
flags="${flags//-fno-exceptions/}"
g++ -O1 $flags -iquote "$ROOT" "$ROOT/test/spatial_static_test.cpp" \
  $($LLVM_CONFIG --ldflags --libs all --system-libs) -pthread -o "$TMP/static"
"$TMP/static"
