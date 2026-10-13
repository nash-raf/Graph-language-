#!/usr/bin/env bash
set -euo pipefail
exec bash "$(cd "$(dirname "$0")" && pwd)/pdg_tdg_verification/build_tdg_validation.sh" "$@"
