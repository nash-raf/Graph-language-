#!/usr/bin/env bash
set -euo pipefail
exec bash "$(cd "$(dirname "$0")" && pwd)/pdg_tdg_verification/run_tdg_shared_graph_levels.sh" "$@"
