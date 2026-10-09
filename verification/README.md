# GraphEasy verification suite

Regression tests so a change can be validated by running one command instead of
re-deriving correctness by hand each time.

    ./run_all.sh            # correctness + determinism + regressions  (~55 s)
    ./run_all.sh --perf     # also print parallel scaling (informational)
    ./run_all.sh -k cc      # only cases matching a pattern

Exit status is non-zero if any correctness or regression check fails, so it can
be wired into CI or a pre-push hook.

## Layout

    cases/       .graph programs under test, generated from ../algo so they
                 stay in sync; only the input path and frontier cap differ
    fixtures/    g20k: 20,000 vertices / 160,000 edges, connected, avg degree
                 16, zero isolated vertices.  Small enough that the suite runs
                 in under a minute, non-degenerate enough to exercise the code
    reference/   golden.py -- independent scipy/numpy implementations
    expected/    golden.json -- values golden.py produced; the source of truth
    bin/         compiled test binaries (rebuilt when a case or the compiler
                 is newer)

## What it checks

1. **Correctness** -- every algorithm against an independent scipy/numpy
   implementation, not against a previous run of itself.  PageRank compares with
   1e-12 relative tolerance (float summation order differs); everything else is
   exact integer equality.
2. **Determinism** -- 5 runs at 4 threads must give identical output.
3. **Thread invariance** -- 1 thread must equal 4 threads.
4. **Regressions** -- specific bugs that were found and fixed:
   - `real` values print at full precision, not `%f` (per-vertex PageRank is
     ~1e-7 and used to print as `0.000000`, making it unverifiable)
   - no SIGSEGV under `-polly-process-unprofitable` (Polly and the outliner
     both transforming `main` crashed inside the outlined task)

## Regenerating golden values

Only when a fixture changes:

    python3 reference/golden.py

This recomputes `expected/golden.json` from scipy/numpy.  Never edit it to make
a test pass -- if the compiler disagrees with scipy, the compiler is wrong.

## Notes / caveats

- `bfs_parent` is deliberately **not** a correctness case.  Its `parent_checksum`
  is legitimately nondeterministic: when two frontier vertices reach the same
  node in one round, whichever writes first becomes the parent, and every
  outcome is a valid BFS tree.  `bfs_level.graph` records BFS *distances*
  instead, which are unique, and is the case actually asserted on.
- The Polly regression test only means something when Polly is actually wired
  in.  On commit d4443ce `parsePollyFlags()` is defined but never called, so
  Polly is inert and the test passes vacuously.  Re-check it once the Polly
  pipeline is reconnected.
- `--perf` numbers are reported, never asserted.  Timings vary with machine
  load; asserting on them produces flaky tests.
