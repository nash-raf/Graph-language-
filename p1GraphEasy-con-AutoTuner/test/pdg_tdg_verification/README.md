# PDG and shared-graph TDG verification

This folder contains the dedicated verification and regression assets for the
lasting PDG memory-intrinsic expansion and shared graph-task scheduling changes.
Run from the repository root on Linux or WSL:

```bash
bash test/pdg_tdg_verification/run_all.sh --focused
bash test/pdg_tdg_verification/run_all.sh --full
```

The focused run builds the validation compiler, checks that argument-memory
intrinsics are expanded before PDG, proves and model-checks the abstract PDG/TDG
properties, compares independent and dependent graph-task fixtures with serial
results, and runs the graph runtime-executor regressions. The shared-level
regression also checks observed overlap, one-thread and allocation fallbacks,
extraction failure, and thread budgets.

The full run additionally invokes the established repository-wide algebra,
totality, emitted-expression, reduction, round-separation, composition, and TDG
budget suites, then inventories argument-memory-only calls across the fixture
corpus. Their shared harnesses and original fixture corpus remain at existing
paths because other workflows use them. The dedicated scripts, C and graph
fixtures, TLA+ models, and proof-status notes are here; thin compatibility
entrypoints remain at the former `test/` script paths.

The ARS/CleanCut frequency regressions and their LLVM driver also live here and
run in both suite modes. To run them independently:

```bash
python3 test/pdg_tdg_verification/test_autotuner_frequency.py
```

Add `--frontend` after rebuilding the root `GraphProgram` compiler to also check
predicted costs, runtime visit counts, and results for one/twenty rounds at
one/four threads.

`PDG_SOUNDNESS.md` and `TDG_TASK_SAFETY.md` state the proof boundaries. The
formal scheduler result assumes complete task-effect summaries and ordering
edges; these tests and models do not establish end-to-end compiler soundness.
