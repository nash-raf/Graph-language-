"""Verify retained validation outputs and record the exact compiler revision."""
import hashlib
import json
from pathlib import Path
import re
import shutil

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
hash_file = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
validations = []
for rounds, count in ((300, 50), (80000, 10)):
    rows = json.loads((HERE / f"application_validation_rounds{rounds}.json").read_text())
    assert len(rows) == count
    out = HERE / ("applications" if rounds == 300 else f"applications_rounds{rounds}")
    for row in rows:
        assert row["correct"]
        audit = out / "binaries" / row["case"] / f"{row['mode']}.threads{row['threads']}.audit.log"
        text = audit.read_text()
        assert "ITERATOR AUDIT passed" in text and "AUDIT FAILED" not in text
        transitions = [list(map(int, pair)) for pair in re.findall(r"ITERATOR AUDIT layout (\d+) -> (\d+)", text)]
        if rounds == 80000:
            assert transitions == [[0, 3], [3, 0]], (row, transitions)
        validations.append(dict(**row, rounds=rounds, transitions=transitions,
                                audit=str(audit.relative_to(HERE)), audit_sha256=hash_file(audit)))
    assert hash_file(out / "graphs/community_projection.txt") == hash_file(ROOT / "bench_results/layout_applications_20261015/graphs/community_projection.txt")
assert "ARS frequency regression tests: PASS" in (HERE / "frequency-tests.log").read_text()
assert "PASS frontend/runtime" in (HERE / "frequency-tests.log").read_text()
assert "OK" in (HERE / "cost-model-tests.log").read_text()
assert "PASS native AutoTuner:" in (HERE / "final-regression-tests.log").read_text()
shutil.copy2(ROOT / "GraphProgram", HERE / "GraphProgram.after")
files = ["GraphProgram", "AutoTunerPass.cpp", "test/pdg_tdg_verification/test_autotuner_iterator_phases.py",
         "test/pdg_tdg_verification/iterator_phase_audit.c", "test/pdg_tdg_verification/test_autotuner_frequency.py"]
(HERE / "validation_manifest.json").write_text(json.dumps(dict(
    backend="Default effect lowering; GRAPH_FRONTIER_REWRITE_OFF unset",
    full_size_auto_transitions="CSR -> SET -> CSR for all five cases at one and four threads",
    application_executions=len(validations),
    compiler_and_source_sha256={f: hash_file(ROOT / f) for f in files},
    application_executions_detail=validations,
    timings="Correctness/audit executions only; no new speedup claim"), indent=2) + "\n")
print("PASS: 60 retained application executions, full-size runtime transitions, source/compiler hashes, frequency and model checks")
