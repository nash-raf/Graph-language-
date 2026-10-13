# Parallel corpus failures after `b95fbd1 "serial fallback fix"` — per-case analysis

> **STATUS 2026-10-13:** of the 9 failures below, 8 were fixed upstream in `b95fbd1` and `dedbc33`. The last one — `race/mutual_deg: build failed` — was the grammar's dropped `hasEdge(G,u,v)` call spelling, fixed in this commit (call-form rules restored); `verify/run.sh parallel` then reports **68 passed / 0 failed / 1 skipped**. Kept as the record of what each failure was and where its contract is written.


Tree: `origin/ARS-1` = `b95fbd1` (pulled 2026-10-13; parent `f4010d5 "grammar updated"`).
Command: `cd verify && bash ./run.sh parallel`.
Result: **passed: 59, failed: 9, skipped: 0** (68 executed rows).

For every failing case this document states, with citations:
**(a)** the harness assertion, **(b)** what the project says the behavior must be and *where that is written*,
**(c)** what the compiler actually does, **(d)** why it fails.

Reproduce any single case with:
```bash
cd p1GraphEasy-con-AutoTuner
GRAPH_FRONTIER_STATS=1 GRAPH_FILE=../verify/cases/parallel/<fixture>.graph bash ./03_run.sh
grep -E 'candidate:|schedule=|reason=|Syntax error' <the log>
```

## Quick table

| case | expected | actual | owner |
| --- | --- | --- | --- |
| `parallel/nested_step` | refused `while`, step gets a **TDG decision** with a dispatch width | schedule realized, but **no TDG decision** recorded | TDG/autotuner path |
| `race/mutual_deg` | `sequential` | **build fails: syntax error** | **grammar surface (ours)** |
| `class/reduce_plus_write` | `privatized` | schedule `source-phase-dag`, but **no candidate line** | instrumentation/analysis |
| `class/priv_reduce_write_big` | `privatized 'red=1'` | same: schedule emitted, **no candidate line** | instrumentation/analysis |
| `race/carried_read_state` | `dest-owner`, `temporal=Carried` | spatial-dag realized, but **R_S/R_T split missing** | witness reporting |
| `class/dual_same_base` | `dual-owner` | `inconclusive`, `discharge=none`, `schedule=serial` | **R6 realization gap** |
| `race/dual_same_base` | staged U→V dispatch via the temporal DAG | `schedule=serial`, no DAG axes/staged children | **R6 realization gap** |
| `race/shadow_snapshot` | nest admitted and **emitted nested** | admitted (`realized=1`) but **not emitted nested** | emission shape |
| `class/reduce_real_ops` | `reduction` for `rsum` (min/max stay `sequential`) | `rsum` refused: `unconditional shared write …` | spatial refusal vs reduction |

---

## 1. `parallel/nested_step` — no TDG decision for the step

- **Harness:** `verify/run.sh:276-283` compiles `cases/parallel/nested_step.graph`, `skip`s if the program is SIGKILLed (`rc=137`), and asserts the barrier line: `grep -q 'call barrier: stateful call' "$R/bin/nested_step.log"`. The failing message is *"no TDG decision for the step"*.
- **What it should be, and where it is written:** the fixture's own comment — *"The outer `while` can never be DOALL-outlined: after the frontier lowering, its body contains the rewrite's setup call, which the loop classifier sees as a stateful call (the call barrier) — the refusal is the design, and this case asserts it happens. The *step* inside it is still a TDG site: it calibrates over a bounded number of single-thread passes, then gets a single-site level whose plan sets its dispatch width. The answer must be identical at 1 and 4 threads (checksum 200 on fixtures/tiny5.txt)."* Design doc `TDG_AND_BUDGET_PLANS.md:291-296` — *"`verify/cases/parallel/nested_step.graph` (new) nests a traversal inside a serial outer loop … The suite check `parallel/nested_step`"*.
- **Actual:** the frontier side is fine — `schedule=spatial-dag emitted=1 impl_failure=0 reason=static pair-work partition family realized by CleanCut`, `[spatial] … status=independent-family group=Destination … runtime=1 realized=1`. What is missing is the **TDG (autotuner) decision record** for the nested step — a different subsystem from the frontier proof, so the refusal-of-`while` design works but the inner step never receives a plan/dispatch decision.
- **Why it fails:** the TDG/planning path does not emit a decision in this nesting. (Expected: refusal on the outer loop only, a decision for the inner step.)

## 2. `race/mutual_deg` — build failed (syntax error)

- **Harness:** `verify/run.sh:348` pins the expected *value and class*: `mutual_deg|mutdegsum 8|sequential`; `:328` notes *"`mutual_deg` still has no per-source partials"*. So the intended verdict is **sequential** — a refusal, which is legal.
- **Actual:** `Syntax error: failed to parse '../verify/cases/parallel/mutual_deg.graph'` — the file uses the **function-call spelling** `if (hasEdge(G, v, u))` (line 10), which the rewritten grammar no longer accepts (it accepts the member form `G.hasEdge(u, v)`).
- **Why it fails / owner:** the grammar rewrite dropped the `hasEdge(G,…)` / by-construction `hasNode(G,…)` call alternatives. The intended verdict cannot even be evaluated because the program never parses. **This one is ours, not the proof layer.** Two more files still contain the deleted `for each vertex` header: `verify/cases/parallel/doall_single.graph`, `doall_scaling.graph`.

## 3. `class/reduce_plus_write` — no candidate line

- **Harness:** `verify/run.sh:469` — `expect_class reduce_plus_write privatized`; the numeric half (`:478`) expects `acc 160000 w0 1`.
- **What it should be, and where it is written:** the current contract is **`privatized`** —
  - `PROBLEMS_AND_SOLUTIONS.md:394`: *"**D** | reduction next to a per-vertex array write (write in the driver preamble) | **solved-general (R3+D)** | privatization + a second work function / runtime entry (`autograph_frontier_step_owner_red_pre`) that runs the preamble exactly once per source, including arc-less sources | `reduce_plus_write`: `acc 160000 w0 1`; `priv_reduce_write_big`: `wsum 20000` (= Σw = n), 1/4 threads, serial-equality ✅"*
  - `COMPOSITION_R3_PRIVATIZATION.md:142`: `| reduce_plus_write | privatized | acc 160000 w0 1 | 1, 4 | ✅ |`
  - `COMPOSITION_SUMMARY_A_TO_I.md:114`: *"Evidence. `reduce_plus_write`: `privatized`, `acc 160000 w0 1`"*
  - **Stale text warning:** the fixture comment still says *"The safe verdict is sequential"* and `COMPOSITION_STATUS.md:28` still lists D as *"refused-general … `class=sequential`"* — those describe the **pre-R3+D** state. The newer documents and the harness assertion supersede them (same pattern as the `two_reduce_slots` comment that was corrected in `b95fbd1`).
- **Actual:** the spatial proof *does* admit it (`[spatial] … status=independent-family group=Source cleancut=Source … runtime=0 realized=0`) and a schedule is emitted (`schedule=source-phase-dag emitted=1 impl_failure=0 reason=original order within each independent temporal component`) — but **no `[graph-frontier] candidate:` line is printed**, so `expect_class` finds nothing to read (`no [graph-frontier] candidate line`).
- **Why it fails:** the analysis reaches a schedule decision but does not record a candidate with its class. The parallelization happens; the *evidence line* the contract is asserted against is missing.

## 4. `class/priv_reduce_write_big` — no candidate line matching `red=1`

- **Harness:** `verify/run.sh:489` — `expect_class priv_reduce_write_big privatized 'red=1'`.
- **What it should be, and where it is written:** `COMPOSITION_R3_PRIVATIZATION.md:145` — `| priv_reduce_write_big | privatized | acc 160000 wsum 20000 (Σw = n) | 1, 4 | ✅ |`; `COMPOSITION_R3_PRIVATIZATION.md:130` lists it among *"the scale cases (g20k)"*.
- **Actual:** identical shape to case 3 — `[spatial] … status=independent-family group=Source`, `schedule=source-phase-dag emitted=1`, and **no candidate line** (the filter `red=1` therefore matches nothing).
- **Why it fails:** same missing candidate record; because the filter is narrower here, the failure message names the filter (`no … matching 'red=1'`) rather than the line's absence.

## 5. `race/carried_read_state` — R_S/R_T split missing

- **Harness:** `verify/run.sh:542` — `expect_class carried_read_state dest-owner "temporal=Carried"`; `:570` also asserts the answer equals the serial build.
- **What it should be, and where it is written:** the fixture comment states the two-axis contract — *"R4, one axis only (temporal dirty, spatial clean) … With an all-vertices round domain the relation is Temporal::Carried (not PreviousRoundRead …), the read is not one of the round-separation shadow's cross-reads, so the temporal witness R4 stays unresolved. Two-axis contract (spec section 17): a witness on one axis must not suppress proven parallelism on the other. This nest is destination-owned (deg[v]…)"*. Expected class: **`dest-owner`** with `temporal=Carried`.
- **Actual:** the nest *is* parallel (`schedule=spatial-dag emitted=1 impl_failure=0 reason=static pair-work partition family realized by CleanCut`, `[spatial] … group=Source … runtime=1 realized=1`) — but the harness's witness-bookkeeping check fails: the **R_S / R_T split is not printed**. So the behavior is right, the *evidence fields* are not emitted.
- **Why it fails:** the two-axis witness reporting for this shape is incomplete (the analysis admits and realizes the nest, then does not report which axis carries a witness and which is proven independent).

## 6. `class/dual_same_base` + `race/dual_same_base` — R6 not realized

- **Harness:** `verify/run.sh:582` — `expect_class dual_same_base dual-owner`; `:599` — the race half asserts the staged ordering (`race/dual_same_base (spatial ordered, staged U->V, == serial)`).
- **What it should be, and where it is written:** the fixture comment defines R6 exactly — *"R6, one axis only (spatial dirty, temporal clean): the pair phase writes the same base in both ownership regions (A[u] in the U region, A[v] in the V region), so the two dual-owner phases must be ordered, but nothing makes the temporal axis dirty. Two-axis contract (spec section 17): the spatial witness sets R_S and must not suppress the proven temporal independence. The realization is the staged dual owner — every U-owned write happens-before every V-owned write (a tagged RealizationOrder edge, never semantic precedence) — while each …"*. Expected: class **`dual-owner`**, realized as the staged U→V dispatch.
- **Actual:** `[spatial] … status=inconclusive group=None … reason=surviving relations require topology-dependent or unresolved grouping`; the certificate lists the relation and its discharge explicitly: `[frontier-cert] #1 R6 spatial base=A src=U.W sink=V.W relation=1 discharge=none reason=same-base U+V dual ownership conflict`; outcome `schedule=serial emitted=1 impl_failure=0`. On the race half: *"no emitted temporal-dag schedule, DAG axes metadata missing, staged children not dispatched via the DAG (trace=0)"*.
- **Why it fails:** the R6 relation is **detected** but not **discharged** (`discharge=none`), so no staged dual-owner realization is emitted and the temporal DAG for the two phases is never built. This is a capability gap in the realization, not an instrumentation gap — the contract is stated in the fixture and the analysis already has the relation with its reason string.

## 7. `race/shadow_snapshot` — admitted, not emitted nested

- **Harness:** `verify/run.sh:635-647` — the compile must succeed and the case asserts *"snapshot discharges the read; claim staged"*.
- **What it should be, and where it is written:** the fixture comment — *"Shadow snapshot discharges the cross-endpoint read (the R4-hiding case) … The round-separation base is read across endpoints (alive[v], the destination region) and mutated single-region (alive[u]) -> the shadow snapshot is the discharge for the read, so no R4 witness survives; the per-source claim is staged (R2 discharge=claim-staging) and **the nest is admitted**. Answers must …"* (also pass equality with the serial build).
- **Actual:** the nest *is* admitted (`schedule=spatial-dag emitted=1`, `[spatial] … runtime=1 realized=1`) but the harness fails on *"not emitted nested"* — the emission shape the check expects (the nest emitted nested inside its driver) is absent.
- **Why it fails:** the analysis and disposition are correct; the **emission form** differs from what the contract requires. (Contrast with cases 3–5: there the missing piece was evidence, here it is code shape.)

## 8. `class/reduce_real_ops` — float `rsum` refused

- **Harness:** `verify/run.sh:812-814` — `expect_class reduce_real_ops reduction 'rsum'`, `expect_class reduce_real_ops sequential 'rmn'`, `expect_class reduce_real_ops sequential 'rmx'`. The failing row is only the `rsum` one.
- **What it should be, and where it is written:**
  - `COMPOSITION_EGI_PROBLEM_AND_SOLUTION.md:112`: *"reduce_real_ops : class=reduction  red=1  temporal=Independent  compat=single"* — the summary block is also referenced at `COMPOSITION_AH_ARS1_SOLUTION.md:127`.
  - `OPEN_PROBLEMS_HANDOFF.md:567`: *"`race/reduce_real_ops` (float sum parallelizes, both float min/max do not)"*; `OPEN_PROBLEMS_HANDOFF.md:245`: *"float `select(fcmp)` min/max stays refused (not reorder-invariant with NaN / ±0). `race/reduce_real_ops` (1 parallel + 2 pinned refusals)"*.
  - `COMPOSITION_STATUS.md:31` (G): the residue is *"the raw float `select(fcmp olt/ogt)` form"* — the **sum** is not that residue.
  - The fixture comment: *"`rsum` is an FAdd reduction … Assertions: the nest is classified `reduction` … 1-thread == 4-thread bit-for-bit (the partition count is fixed by the tuner, so an ordered combine is exact)."*
- **Actual:** `rsum` is refused by the **spatial** rule before the reduction path runs — `[spatial] … status=no-partition … reason=unconditional shared write connects every pair occurrence` (`spatial_partition_proof.inc:482`), candidate line `… class=sequential  U+(rsum,G)  temporal=Independent [refused: ]`. The `rmn`/`rmx` halves are refused by the R8 implementation guard (`reason=R8 guard: reduction realization`), which matches their expected `sequential`.
- **Why it fails:** the spatial "unconditional shared write" rule fires on the accumulator slot and returns `NoPartition`, so the ordered FAdd combine is never emitted. Per the docs above, an ordered combine is exact for this nest, so the refusal is over-conservative — the same rule family that was fixed for the two-slot privacy cases in `b95fbd1` is still firing for the float sum.

---

## How to read the "stale text" cases

Two of these fixtures carry comments that contradict the current assertion, exactly like `two_reduce_slots` did before `b95fbd1` corrected it:

- `reduce_plus_write` — comment says *"The safe verdict is sequential"*, `COMPOSITION_STATUS.md:28` says *"refused-general"*, while `PROBLEMS_AND_SOLUTIONS.md:394`, `COMPOSITION_R3_PRIVATIZATION.md:142`, `COMPOSITION_SUMMARY_A_TO_I.md:114` and `run.sh:469` say **`privatized`** (R3+D solved it).
- `COMPOSITION_STATUS.md` is visible in several rows as the *older* table (rows D, G); `PROBLEMS_AND_SOLUTIONS.md` is its successor. When the two disagree, the newer document plus `run.sh` are the contract.

## Suggested order of work

1. **Ours, cheap:** restore `hasEdge(G,u,v)` / `hasNode(G,u)` parsing in `Base.g4` + `ASTBuilder`; migrate `doall_single.graph`, `doall_scaling.graph` to the new loop headers. Removes the `race/mutual_deg` failure and the two latent ones.
2. **Instrumentation (cases 3–5):** record a `[graph-frontier] candidate:` line (with `class=` and the `red=`/`sep=`/`data=` flags) on the `source-phase-dag` and `spatial-dag` paths, and print the R_S/R_T split for admitted nests. The schedules are already correct; only the evidence the assertions read is missing.
3. **Realization (cases 6–7):** discharge the R6 relation (staged dual-owner, tagged `RealizationOrder` edge) and fix the nested emission shape.
4. **Reduction (case 8):** let the ordered FAdd combine be realized for `rsum` (the spatial rule currently refuses it first), keeping `rmn`/`rmx` refused as the docs require.
5. **TDG (case 1):** record a TDG decision for a step nested in a serial outer loop (the outer refusal itself works — the `call barrier` line is asserted at `run.sh:283`).
