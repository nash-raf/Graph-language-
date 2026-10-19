#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
ulimit -s 8192
python3 test/check_record_liveness.py
