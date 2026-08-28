#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Complete process launch-to-exit scalability comparison.
# Overrides: MAX_THREADS=10 ROUNDS=7 SGPL_BIN=... GRAPTOR_ROOT=...
exec python3 ./benchmark_bfs_compare.py scalability \
  --max-threads "${MAX_THREADS:-10}" \
  --rounds "${ROUNDS:-7}"
