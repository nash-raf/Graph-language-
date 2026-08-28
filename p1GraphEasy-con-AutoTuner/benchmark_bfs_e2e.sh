#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Complete process launch-to-exit comparison at the same thread count.
# Overrides: THREADS=8 ROUNDS=7 SGPL_BIN=... GRAPTOR_ROOT=...
exec python3 ./benchmark_bfs_compare.py e2e \
  --threads "${THREADS:-8}" \
  --rounds "${ROUNDS:-7}"
