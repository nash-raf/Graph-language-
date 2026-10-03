#!/usr/bin/env bash
# Repoint the fixtures' absolute graph paths at this checkout.
#
# The case files embed an absolute `edges: file "..."` path (the compiler bakes
# it into the program), so a tree copied to another machine fails at compile
# time with "cannot open graph file <old path>".  This rewrites the prefix to the
# tree's own root:
#
#   bash verify/repoint_paths.sh            # repo root inferred from $PWD
#   bash verify/repoint_paths.sh /workspace/IMTalker/p1
#
# Only the absolute prefix changes; the relative layout (fixtures/, real_graphs/)
# is the same everywhere.
set -uo pipefail
R="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${1:-$(cd "$R/.." && pwd)}"
# Any absolute prefix that ends in /(verify|real_graphs|graphs)/ belongs to a
# checkout; rewrite it to this one.  Prefix-agnostic and idempotent.
files=$(grep -rlE '"/[^"]*/(verify|real_graphs|graphs)/' "$R/cases" 2>/dev/null)
if [[ -z "$files" ]]; then
  echo "repoint_paths: nothing to do (no absolute graph paths in $R/cases)"
  exit 0
fi
n=$(wc -l <<<"$files")
perl -pi -e "s#\"[^\"]*?/(verify|real_graphs|graphs)/#\"$ROOT/\$1/#g" $files
echo "repoint_paths: rewrote $n case file(s) to $ROOT"
