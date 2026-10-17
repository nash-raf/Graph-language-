# AutoTuner layout changes with native iterator lowering

The default compiler no longer replaces an automatically selected schedule with
CSR merely because neighbor iterator calls occur in the program. No motif
backend flag is required.

## Implementation

- Graph bitmap operations are identified from their bitmap operands. Independent
  frontier and visited bitmaps do not create graph update/query regions.
- A native traversal and its nested neighbor loops form one phase. Layout
  conversions are emitted in its preheader, before iterator initialization and
  before the first `next()` call, including a peeled first call.
- CSR-only queries within a traversal constrain that phase to CSR. Graph bitmap
  queries within a traversal constrain that phase to SET. The dynamic program
  includes these requirements when selecting a schedule. Incompatible access
  requirements in one traversal produce a compiler error.
- Unscoped direct access to CSR array pointers still constrains the affected
  graph to CSR. This is represented in feasibility before optimization and does
  not pin unrelated graphs.
- Empty blocks left by effect lowering lose their stale traversal metadata. The
  executor call represents the traversal and is not charged a second time.

Existing benefit thresholds and conversion-cost equations remain in place.

## Validation

Run from the repository in Linux or WSL with LLVM 20:

```sh
python3 test/pdg_tdg_verification/test_autotuner_iterator_phases.py --frontend
python3 test/pdg_tdg_verification/test_autotuner_iterator_phases.py --frontend-only --rounds 80000 --modes AUTO
python3 test/pdg_tdg_verification/test_autotuner_frequency.py --frontend
python3 test/pdg_tdg_verification/test_layout_cost_model.py
```

The new pass regressions use the actual LLVM pass and verifier. They cover
automatic switching, four forced layouts, independent bitmap operations, bitmap
aliases/getters, nested traversal boundaries, a peeled first iterator call,
weighted CSR requirements, separate graphs, raw CSR pointer constraints, and
stale traversal markers after effect lowering.

The application checks compile five distinct SGPL programs on the same graph:
contact tracing, connected components for service outages, sensor k-core pruning,
gateway failover, and trust-boundary DFS inventories. They use the default
compiler backend and assert that no motif `autograph_edgemap` call was emitted.

- At 300 event windows, all five layout variants run at both one and four
  threads: 50 executions.
- At the original 80000 event windows, AUTO runs at both one and four threads:
  10 executions. Every case executes CSR -> SET -> CSR.
- Each execution is checked against the independent Python reference results.
- An audit links the exact compiled program object and checks layout and epoch
  stability during native iterator scans. Fixed variants also check the layout
  at every intercepted iterator initialization. A negative control deliberately
  converts an active iterator's graph and must be rejected by the audit.

The frequency regression fixture now uses the current SGPL syntax and supplies
the graph-bound proof needed by current effect lowering. Its assertions accept
the current spatial-context constructor and legitimate serial fallback callbacks;
traversal counts and predicted costs must still match exactly.

Evidence is saved in `final-regression-tests.log`, `full-size-auto-tests.log`,
`frequency-tests.log`, `cost-model-tests.log`, and
`application_validation_rounds{300,80000}.json`. Application source, LLVM IR,
objects, executables and per-execution audit logs are retained under
`applications/` and `applications_rounds80000/`.

These executions validate the compiler fix. They are not a new timing study.
The earlier CSV in `bench_results/layout_applications_20261015/` still describes
the earlier motif-backend measurements; its speedup values are not reassigned to
the default backend.
