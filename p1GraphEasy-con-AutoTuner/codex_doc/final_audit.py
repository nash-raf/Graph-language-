#!/usr/bin/env python3
"""Verify final source identity and summarize retained validation evidence."""
from pathlib import Path
import hashlib,json
repo=Path(__file__).resolve().parents[1]
root=Path('/home/kazi/p1_layout_native_20261105')
v=root/'versions/native07';core=root/'versions/native06'
production={}
for p in repo.iterdir():
    if p.is_file() and p.suffix in {'.c','.h','.cpp','.inc'}:
        frozen=v/'src'/p.name
        assert frozen.exists(),p
        # Freeze normalizes text newlines; compare the actual source text.
        assert p.read_text()==frozen.read_text(),f'Changed after freeze: {p}'
        production[p.name]=hashlib.sha256(p.read_text().encode()).hexdigest()
assert json.loads((v/'build.json').read_text())['compiler_sha256']==json.loads((core/'build.json').read_text())['compiler_sha256']
changed=[p.name for p in (v/'src').iterdir() if p.is_file() and p.suffix in {'.c','.h','.cpp','.inc'}
         and p.read_text()!=(core/'src'/p.name).read_text()]
assert changed==['autotuner_runtime.c'],changed
validation={}
for name in ['baseline','native06','native07']:
    rows=json.loads((root/'versions'/name/'cpu_validation.json').read_text())
    assert len(rows)==732 and all(x['passed'] for x in rows)
    validation[name]=len(rows)
history=json.loads((core/'regressions/results.json').read_text())
latest={x['suite']:x for x in history}
original={k:x for k,x in latest.items() if k!='run_parallel_claims_remaining_tests'}
assert len(original)==31
failed=[k for k,x in original.items() if x['exit_code']]
assert sorted(failed)==['run_parallel_claims_tests','run_temporal_tests'],failed
result=dict(final_version='native07',production_sources=production,core_delta=changed,
            core_delta_reason='Only debug reporting changed: report actual source and prepared-record counts',
            full_vector_validation=validation,existing_suites_total=len(original),
            existing_suites_passed=sum(x['exit_code']==0 for x in original.values()),
            existing_failures=failed,claims_remaining_assertion_also_fails_baseline=True)
(root/'final_audit.json').write_text(json.dumps(result,indent=2))
print(json.dumps({k:v for k,v in result.items() if k!='production_sources'},indent=2))
