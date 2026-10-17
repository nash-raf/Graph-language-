#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
TMP="$(mktemp -d)";trap 'rm -rf "$TMP"' EXIT
for mode in normal scalar sanitized;do
 flags=(-O3)
 [[ "$mode" == scalar ]] && flags+=(-DSGPL_COMPACTION_SCALAR_ONLY=1)
 [[ "$mode" == sanitized ]] && flags=(-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
 gcc "${flags[@]}" -I. test/stable_compaction_test.c -o "$TMP/test"
 ASAN_OPTIONS=detect_leaks=1 "$TMP/test"
done
