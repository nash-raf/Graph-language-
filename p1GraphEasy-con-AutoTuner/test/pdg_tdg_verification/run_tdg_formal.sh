#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

TLA_TOOLS_JAR="${TLA_TOOLS_JAR:-$ROOT/build_tdg_validation/tla2tools.jar}"
TLAPM="${TLAPM:-$ROOT/build_tdg_validation/tlapm/bin/tlapm}"
[[ -f "$TLA_TOOLS_JAR" ]] || {
  echo "Set TLA_TOOLS_JAR to the official TLA+ tools jar" >&2
  exit 1
}
[[ -x "$TLAPM" ]] || {
  echo "Set TLAPM to the official TLAPS proof manager" >&2
  exit 1
}

"$TLAPM" test/pdg_tdg_verification/TDGTaskSafetyProof.tla
"$TLAPM" test/pdg_tdg_verification/PDGEdgeCoverage.tla

mkdir -p build_tdg_validation/tlc_states
safe_log="$(mktemp)"
unsafe_log="$(mktemp)"
trap 'rm -f "$safe_log" "$unsafe_log"' EXIT

java -XX:+UseParallelGC -cp "$TLA_TOOLS_JAR" tlc2.TLC \
  -metadir build_tdg_validation/tlc_states \
  -config test/pdg_tdg_verification/TDGTaskSafety.cfg -workers 1 \
  test/pdg_tdg_verification/TDGTaskSafety.tla >"$safe_log" 2>&1 || {
    cat "$safe_log" >&2
    exit 1
  }
grep -q 'Model checking completed. No error has been found.' "$safe_log" || {
  cat "$safe_log" >&2
  exit 1
}
grep -E 'states generated, .*distinct states found' "$safe_log"

# The intentionally unsound scheduler must yield a concrete violating trace.
# TLC releases differ in exit status on invariant failures, so inspect output.
java -XX:+UseParallelGC -cp "$TLA_TOOLS_JAR" tlc2.TLC \
  -metadir build_tdg_validation/tlc_states \
  -config test/pdg_tdg_verification/TDGTaskSafety_unsafe.cfg -workers 1 \
  test/pdg_tdg_verification/TDGTaskSafety.tla >"$unsafe_log" 2>&1 || true
grep -q 'Invariant Safety is violated' "$unsafe_log" || {
  cat "$unsafe_log" >&2
  exit 1
}
echo 'PASS: TDG and PDG edge TLAPS proofs, exhaustive safe model, and unsafe-model counterexample'
