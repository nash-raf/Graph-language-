#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
./GraphProgram test.graph > graph.ll
