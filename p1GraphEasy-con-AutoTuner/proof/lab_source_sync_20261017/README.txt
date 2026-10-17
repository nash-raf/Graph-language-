Lab source synchronization, 2026-10-17

The repository now contains the accepted compiler/runtime source currently installed
on graph-downloader, through packing_regression_20261017/03_completion_batch.
25 files were imported (9 updates and 16 additions), including regression tests.

Local AutoTunerPass.cpp and test/pdg_tdg_verification/test_autotuner_frequency.py
contain newer iterator-phase/layout fixes absent from the lab installation.
Both were preserved byte-for-byte. All other existing local files outside the
25-file import were left in place. No object files or executables were copied.
Rebuild the compiler and runtime before using these changes locally.

Imported changes include program-lifetime zero-initialized array storage,
proof-directed snapshot/output preparation, reusable scratch/generation tags,
stable scalar/AVX2/parallel compaction, parallel CSR cut construction, packed
weighted records with fallback expansion, and batched completion publication.
The lowering retains ordinary TDG and thread-budget semantics.

sync_manifest.json records imported file hashes and provenance.
applied_changes.patch is the exact normalized pre-sync-to-post-sync source diff.
local_before.zip backs up all compared local files before synchronization.
working_tree_before.patch and git_status_before.txt record the previous Git state.
remote_sources.zip contains the received source snapshot (without machine binaries).
accepted_benchmark/ contains the preceding acceptance report, not new timing data.
rejected_experiments/ contains archived rejected patches and reports only. These
candidates are NOT applied to the compiler. They are retained for inspection.

Validation uses an isolated directory on the lab PC:
/media/readinggroup/Storage/splash/bench/optimizations/local_sync_20261017/source
The installed lab source and stopped historical benchmark are unchanged.

Integration check found an outdated assertion in run_program_array_tests.sh:
it expected the old full-array snapshot binding. The accepted compiler proves
source-domain reads and emits bind_source_snapshot_range for this fixture.
The test now checks that new entry point; all output assertions remain intact.
This test-only adjustment is recorded in sync_manifest.json.

Local generated ANTLR 4.10.1 sources remain intact. The lab uses ANTLR 4.13.1;
token/interpreter grammar data match. Compiler checks use the lab toolchain.

Final validation: merged compiler built on the lab PC. Eight runtime/frontend
suites passed, including sanitizer checks. AutoTuner frequency/frontend checks
passed. The preserved iterator/layout suite passed with compiler-default
calibration. Its automatic SET-choice assertion is not portable to the lab's
saved hardware calibration, which selects PCSR; no compiler change was made
to force a layout. See final_validation.json and validation_evidence.zip.
A 292-file hash audit confirmed the installed lab source is unchanged.
The local binaries were intentionally not replaced; rebuild before running.
