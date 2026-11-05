#!/usr/bin/env python3
"""Existing autotuner pass/model tests on a frozen private tree."""
from pathlib import Path
import subprocess,sys,json,time,hashlib
version=Path('/home/kazi/p1_layout_native_20261105/versions')/sys.argv[1]
out=version/'autotuner_checks';out.mkdir(exist_ok=True)
summary=out/'results.json'
results=json.loads(summary.read_text()) if summary.exists() else []
# One representation-name assertion changed with the public CPU entry point.
# Retain the frozen test and both hashes; production sources stay frozen.
test='test/pdg_tdg_verification/test_autotuner_frequency.py'
fresh=(Path(__file__).resolve().parents[1]/test).read_bytes()
target=version/'src'/test
if target.read_bytes()!=fresh:
    original=out/'frequency_original.py'
    if not original.exists():original.write_bytes(target.read_bytes())
    (out/'test_overlay.json').write_text(json.dumps(dict(path=test,
        original_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),
        updated_sha256=hashlib.sha256(fresh).hexdigest(),
        reason='Accept layout batch construction alongside legacy CleanCut construction; cost/count assertions unchanged'),indent=2))
    target.write_bytes(fresh)
for name in ['test_autotuner_frequency','test_autotuner_iterator_phases','test_layout_cost_model']:
    if any(r['test']==name and not r['exit_code'] for r in results):continue
    log=out/(name+'.'+str(len(results))+'.log');start=time.time()
    with log.open('w') as output:
        p=subprocess.run([sys.executable,str(version/'src/test/pdg_tdg_verification'/(name+'.py'))],
                         cwd=version/'src',stdout=output,stderr=subprocess.STDOUT,timeout=600)
    results.append(dict(test=name,exit_code=p.returncode,seconds=time.time()-start,log=str(log)))
    (out/'results.json').write_text(json.dumps(results,indent=2));print(results[-1],flush=True)
    if p.returncode:print(log.read_text()[-4000:]);sys.exit(p.returncode)
