#!/usr/bin/env bash
set -euo pipefail

# validate_totality.sh — R7 totality gate.
#
# The contract: every modelable, in-algebra fixture must be lowerable by the
# recursive interpreter (no "expression path refused").  The census records,
# per fixture, the modelable verdict, the semantic refusal reasons (the only
# legitimate refusals) and the interpreter-refusal count; the golden locks the
# expected census so a regression in coverage or in the semantic gate shows up
# as a diff.
#
# Semantic correctness of the lowered realizations is covered by the other
# suites (validate_rt_expr determinism, validate_reduction expected values,
# validate_roundsep round-separated references, validate_composition serial
# MATCH, runtime tests).
#
# Usage: ./validate_totality.sh [--capture]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
GOLDEN="${GOLDEN:-proof/totality_golden.txt}"
[[ -x ./GraphProgram ]] || { echo "missing ./GraphProgram"; exit 1; }

CAPTURE=0
[[ "${1:-}" == "--capture" ]] && CAPTURE=1

TMP="$(mktemp)"
trap 'rm -f "$TMP"' EXIT

FAILURES=0
for f in *.graph; do
  sz=$(stat -c %s "$f")
  [[ "$sz" -gt 2000000 ]] && continue
  out=$(GRAPH_FRONTIER_STATS=1 ./GraphProgram "$f" 2>&1 || true)
  modelable=$(printf '%s' "$out" | grep -c 'modelable=1' || true)
  sem=$(printf '%s' "$out" | grep -o '\[refused: [^]]*\]' | sort -u | tr '\n' ';' || true)
  refused=$(printf '%s' "$out" | grep -c 'expression path refused' || true)
  # Target frontend configuration: first-wins claims lower to the store form;
  # CAS claims must not be generated (see the R7 closure criteria).  The
  # census proves this mechanically instead of assuming the default setting.
  wit=$(SGPL_WITNESS_DUMP=1 ./GraphProgram "$f" 2>&1 || true)
  cas=$(printf '%s' "$wit" | grep -c 'driveruclaims=[1-9]' || true)
  if [[ "$cas" -gt 0 ]]; then
    echo "FAIL $f: CAS-form claim generated in the target configuration"
    FAILURES=$((FAILURES + 1))
  fi
  if [[ "$refused" -gt 0 ]]; then
    echo "FAIL $f: interpreter refused a modelable expression"
    FAILURES=$((FAILURES + 1))
  fi
  # Atom-identity multiset invariant: atoms(ExecOps(E)) == flatten(E), with
  # occurrences (duplicates) preserved.  Only lowered fixtures emit a dump.
  dump=$(SGPL_EXEC_DUMP=1 ./GraphProgram "$f" 2>&1 || true)
  if printf '%s' "$dump" | grep -q '^\[exec\]'; then
    atoms=$(printf '%s' "$dump" | grep '^\[atom\]' | sed 's/^\[atom\] //' | sort)
    execs=$(printf '%s' "$dump" | grep '^\[exec\]' \
              | sed 's/^\[exec\] op=[a-z]* atoms=//' | tr '|' '\n' \
              | grep -v '^$' | sort)
    if [[ "$atoms" != "$execs" ]]; then
      echo "FAIL $f: atoms(ExecOps) != flatten(E)"
      diff <(printf '%s\n' "$atoms") <(printf '%s\n' "$execs") | head -6
      FAILURES=$((FAILURES + 1))
    fi
  fi
  echo "$f modelable=$modelable refused_interp=$refused cas=$cas sem=[$sem]" >> "$TMP"
done

if [[ "$CAPTURE" -eq 1 ]]; then
  cp "$TMP" "$GOLDEN"
  echo "captured $GOLDEN ($(wc -l < "$GOLDEN") fixtures)"
  exit 0
fi

if [[ ! -f "$GOLDEN" ]]; then
  echo "missing golden: $GOLDEN (run ./validate_totality.sh --capture)"
  exit 1
fi

if ! diff -u "$GOLDEN" "$TMP"; then
  echo "totality census changed"
  FAILURES=$((FAILURES + 1))
fi

if [[ "$FAILURES" -eq 0 ]]; then
  echo "totality gate: PASS ($(wc -l < "$TMP") fixtures, 0 interpreter refusals)"
  exit 0
fi
echo "totality gate: FAIL ($FAILURES)"
exit 1
