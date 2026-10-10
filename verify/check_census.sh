#!/usr/bin/env bash
# Section 19 inventory gate: the certificate census (verify/sweep_certificates.sh)
# must keep exercising every witness, every discharge kind and every schedule
# kind; R5 must never appear spuriously.  Regenerate the census, then run this.
#
# Usage: bash verify/check_census.sh [census-file]
set -uo pipefail
R=$(cd "$(dirname "$0")" && pwd)
CS="${1:-$R/bin/certificate_census.txt}"
[ -r "$CS" ] || { echo "FAIL no census at $CS (run sweep_certificates.sh)"; exit 1; }

fail=0
say(){ printf '%-4s %s\n' "$1" "$2"; }

# witness -> "min_instances discharge-list"
check_witness(){ # $1=Rn $2=min $3=allowed (regex of discharge values, '|' joined)
  local rid="$1" min="$2" allowed="$3"
  local n dis bad
  n=$(grep -c "^witness .* $rid " "$CS")
  dis=$(grep "^witness .* $rid " "$CS" | grep -oE "discharge=[a-z-]+" | sort -u | tr '\n' ' ')
  bad=""
  for d in $dis; do
    [[ "$d" =~ ^discharge=($allowed)$ ]] || bad="$bad $d"
  done
  if [[ "$n" -ge "$min" && -z "$bad" ]]; then say PASS "$rid: $n instances, discharges [${dis:-none}]"
  else say FAIL "$rid: instances=$n (min $min) unexpected=[$bad] all=[$dis]"; fail=1; fi
}

check_witness R1 1 'privatization|none'
check_witness R2 1 'claim-staging'
check_witness R3 1 'none'
check_witness R4 2 'privatization|none'
check_witness R6 1 'privatization|none'
check_witness R7 1 'none'

# R5 is predicate-evaluated but unsatisfiable from the DSL: the RS-eligibility
# rule requires single-region mutations on the base, while the endpoint-conflict
# rule needs a pair-phase mutating write in the opposite region.  The shadow
# case instead discharges R4 through the snapshot (verify/cases/parallel/
# shadow_snapshot.graph).  Any R5 in the corpus is either a new DSL shape that
# can reach it (then this gate must be updated with its fixture) or spurious.
n5=$(grep -c "^witness .* R5 " "$CS")
[[ "$n5" -eq 0 ]] && say PASS "R5: 0 instances (unreachable from the DSL, documented)" \
  || { say FAIL "R5: $n5 instances -- a DSL shape reached the predicate; add its fixture and update this gate"; fail=1; }

# Schedule kinds: nested (the normal admitted shape), both single-axis DAGs, and
# serial for the theorem- and implementation-refusals.
for want in "schedule=nested" "schedule=spatial-dag" "schedule=temporal-dag"; do
  n=$(grep -c "^sched .*$want" "$CS")
  [[ "$n" -ge 1 ]] && say PASS "$want: $n fixtures" || { say FAIL "$want: absent"; fail=1; }
done
n=$(grep -c "^sched .*schedule=serial emitted=1 impl_failure=1" "$CS")
[[ "$n" -ge 3 ]] && say PASS "serial+impl_failure: $n fixtures" \
  || { say FAIL "serial+impl_failure: only $n fixtures"; fail=1; }

# Per-axis splits: both one-axis-dirty directions must be exercised.
for split in "R_S=1 R_T=0" "R_S=0 R_T=1"; do
  n=$(grep -c "^axes $split" "$CS")
  [[ "$n" -ge 1 ]] && say PASS "axis split $split: $n fixtures" \
    || { say FAIL "axis split $split: absent"; fail=1; }
done

# Fixture count floor: the census must cover the whole case tree.
n=$(grep -c '^===' "$CS")
[[ "$n" -ge 40 ]] && say PASS "fixtures censused: $n" || { say FAIL "only $n fixtures"; fail=1; }

[[ "$fail" -eq 0 ]] && echo "CERTIFICATE INVENTORY: PASS" || echo "CERTIFICATE INVENTORY: FAIL"
exit "$fail"
