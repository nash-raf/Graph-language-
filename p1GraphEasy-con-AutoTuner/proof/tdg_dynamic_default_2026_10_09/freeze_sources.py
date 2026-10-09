"""Freeze the source revision used by this promotion's measurements."""
from pathlib import Path
import hashlib
import json
import shutil

OUT = Path(__file__).resolve().parent
ROOT = OUT.parents[1]
protected = json.loads((OUT/'mixed/mixed_manifest.json').read_text())['protected_sha256']
paths = list(protected) + ['experimental/tdg_joint_schedule.inc', 'experimental/tdg_joint_trace.inc']
for folder in ('test/tdg_allocation_sweep', 'test/tdg_allocation_sweep/cases'):
    paths += [str(p.relative_to(ROOT)) for p in (ROOT/folder).iterdir() if p.is_file()]
paths += ['tools/'+name for name in (
    'tdg_joint_runtime_test.c', 'tdg_joint_probe.c', 'tdg_joint_validate.py',
    'tdg_joint_numeric_test.py', 'tdg_ordinary_first_reference_alias.c',
    'tdg_joint_build.py', 'tdg_joint_benchmark.py', 'tdg_joint_reference_check.py',
    'tdg_level_design_probe.c', 'tdg_level_design_validation.py', 'tdg_level_design_audit.py')]
manifest = {}
for name in paths:
    destination = OUT/'after'/name
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(ROOT/name, destination)
    manifest[name] = hashlib.sha256(destination.read_bytes()).hexdigest()
for name, digest in protected.items():
    assert manifest[name] == digest, name
(OUT/'after/manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
print('PASS: post-promotion source snapshot matches measured production hashes.')
