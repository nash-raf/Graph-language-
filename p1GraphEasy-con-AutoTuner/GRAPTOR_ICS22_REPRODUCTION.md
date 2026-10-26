# Graptor ICS22 compiler reproduction

This experiment follows the compiler selection in upstream
[reproduce/ICS22.sh](https://github.com/hvdieren/graptor/blob/7d3f561978574fbb6e989a905473e408d278812b/reproduce/ICS22.sh):
GCC 7.2.0, C++17, `-fcilkplus`, and the bundled Cilk Plus runtime.
Neither GCC nor Graptor source code is patched. Generated configuration files,
build commands, probes and measurement tools live outside the upstream sources.
The accepted SGPL implementation and prior Graptor binaries remain unchanged.

The historical Graptor revision is
`7d3f561978574fbb6e989a905473e408d278812b`, which introduced the reproduction
script. The modern frozen source contains newer C++ language constructs;
compatibility attempts with the historical compiler are retained separately.
An historical-source comparison cannot establish a backend-only difference
against the modern GCC13 executables.

GCC is built in WSL from the official GNU GCC 7.2.0 release archive, without
bootstrap or multilib and with C/C++ and Cilk runtime support. Unneeded sanitizer,
MPX and VTV runtime builds are disabled through configure options. Prerequisite
archives are checked against the release's SHA512 manifest. Independent GNU and
kernel.org downloads of GCC have identical SHA256
`1cf7adf8ff4b5aa49041c8734bbcf1ad18cc4c94d0029aae0f4e48841088479a`.
The host compiler builds GCC; the resulting application compiler is GCC 7.2.0.
Source fingerprints are retained to detect source changes.

The paper script targets `skylake-avx512`. This PC's Ryzen 5 5500 supports AVX2,
so the application target is `-march=native`, with `MAX_VL=8`. This is an ISA
configuration adaptation, not a paper-hardware reproduction. Both Cilk and
OpenMP controls use the same target, compiler, historical source and application
settings. The Cilk build uses the language frontend, not the GCC13 Cilk API-only
configuration measured in the preceding experiment. Runtime resolution is
checked to ensure that the bundled runtime is loaded instead of GCC's other
installed Cilk runtime.

PageRank selects the published FP64 variant 11. Historical BFS, Bellman-Ford
SSSP and CC targets use the same reproduction base flags where applicable;
they are additional backend controls, not claims that every target appears in
the paper script. The default numerical formats and algorithm semantics are
preserved. PageRank's iteration limit is 20, but its unchanged convergence
condition may stop it earlier. It redistributes missing mass, normalizes ranks
and uses the padded graph vertex count as its denominator, while leaving padded
output slots zero. The independent reference implements these historical
semantics explicitly; they differ from the modern fixed-iteration recurrence.
Custom low-precision paper variants are excluded
from comparisons with the existing double-precision workload.

Correctness must precede timing. Historical PageRank and CC expose output
options. BFS and SSSP can be inspected using read-only GDB memory extraction
after their run, without adding export code. Independent validators use the
existing frozen graph corpus. Debugger runs are never timing samples. Worker
probes check 1/2/6 workers and allowed CPUs; timed runs use six distinct physical
cores, CPU IDs 0, 2, 4, 6, 8, 10, and eight graph partitions.

The prepared protocol rotates and reverses execution order, measures original
and identical-binary A/A labels for both historical backends, retains five
primary and twenty confirmation observations, and preserves failed controls
and any complete-block retry. Compiler builds, validation and analysis are
separate from timed execution. The whole-process timer and recorded Graptor
timer are reported separately. Graphs, executables, dependencies and harnesses
are fingerprinted before and after timing.

## Refusals before timing

The untouched modern frozen Graptor cannot be built with GCC 7.2: C++23 is an
unsupported language option, and a C++17 compatibility attempt fails on its
`<bit>` include. No replacement headers or language backports are introduced.

All four historical targets initially compiled with both backends after adding
the paper script's required NUMA linkage. Runtime verification rejects two:

- Historical BFS's default produces parents that fail the independent
  shortest-path-level check on `dimacs/hamming8-2.edges`, with one OpenMP worker.
  Its existing `UNCOND_EXEC=0` option was tried for both backends but fails
  template compilation in the unchanged source (`redop_setif`/mask handling).
- Historical `BFv.C` generates its own dyadic edge weights rather than using
  the modern benchmark's supplied weights. Even at one OpenMP worker it aborts
  on the first corpus graph with `active frontier mismatch: nactv=257
  new_frontier: v: 5 e: 1235`. The read-only extractor refuses a traversal that
  does not return normally; no partial arrays count as validation passes.

These targets receive no accepted timing results. Their binaries, failed
attempts, build identities and logs remain available. Benchmark eligibility is
limited to historical CC and the published FP64 PageRank target, contingent on
the full corpus checks. This is a documented partial workload reproduction,
not a four-algorithm replacement baseline.

External artifacts: `/home/kazi/p1_graptor_ics22_20261026` and
`C:/p1-graptor-ics22-benchmark-20261026`. Build logs and any compatibility failures
remain there. Results are appended after the corresponding stages complete;
preparing a protocol does not establish a successful build or benchmark.
