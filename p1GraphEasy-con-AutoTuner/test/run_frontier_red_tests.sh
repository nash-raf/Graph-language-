#!/usr/bin/env bash
set -euo pipefail
exec bash "$(cd "$(dirname "$0")" && pwd)/pdg_tdg_verification/run_frontier_red_tests.sh" "$@"
