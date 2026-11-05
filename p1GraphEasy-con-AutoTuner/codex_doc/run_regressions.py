#!/usr/bin/env python3
"""Run existing suites in the immutable version's isolated source copy.

Usage: venv/bin/python codex_doc/run_regressions.py native01 suite1 suite2 ...
Suite names omit test/ and .sh. Logs/results survive interruption.
"""
from pathlib import Path
import json
import os
import shutil
import subprocess
import sys
import time

repo=Path(__file__).resolve().parents[1]
version=sys.argv[1]
assert version.replace('_','').isalnum()
root=Path('/home/kazi/p1_layout_native_20261105/versions')/version
src=root/'src'
assert (root/'build.json').exists()
out=root/'regressions';out.mkdir(exist_ok=True)
for script in (src/'test').rglob('*.sh'):
    data=script.read_bytes()
    if b'\r\n' in data:script.write_bytes(data.replace(b'\r\n',b'\n'))
for item in repo.iterdir():
    if item.is_file() and item.suffix in {'.sh','.py','.graph','.sgpl','.txt','.json'}:
        target=src/item.name
        if not target.exists():
            if item.suffix=='.sh':target.write_text(item.read_text())
            else:shutil.copy2(item,target)
for name in ['tools','verify']:
    target=src/name;source=repo/name if (repo/name).exists() else repo.parent/name
    if not target.exists() and source.exists():target.symlink_to(source,target_is_directory=True)
verify=root/'verify'
if not verify.exists():verify.symlink_to(repo.parent/'verify',target_is_directory=True)
for item in [root/'GraphProgram',*root.glob('*.o')]:
    target=src/item.name
    if not target.exists():target.symlink_to(item)
summary=out/'results.json'
results=json.loads(summary.read_text()) if summary.exists() else []
for name in sys.argv[2:]:
    assert name.replace('_','').isalnum()
    if any(x['suite']==name and x['exit_code']==0 for x in results):continue
    script=src/'test'/(name+'.sh')
    if not script.exists():script=src/(name+'.sh')
    assert script.exists(),script
    log=out/(name+'.'+str(len(results))+'.log')
    print('TEST',version,name,flush=True)
    start=time.time()
    env=dict(os.environ,FORCE_CPU='1',FORCE_GPU='0',SGPL_GPU_BACKEND='0',
             SGPL_NUM_THREADS='6',SGPL_ALLOW_HT='0')
    env.update(SGPL_ALLOW_LOCAL_CONTRACT_TESTS='1',P1_TEST_WIDTHS='1 2 6',
               P1_SIMPLIFY_TEST_OUTPUT=str(out/(name+'_artifacts')),
               P1_RELAX_SIMPLIFY_TEST_OUTPUT=str(out/(name+'_artifacts')),
               P1_FRONTIER_SIMPLIFY_TEST_OUTPUT=str(out/(name+'_artifacts')))
    campaign=out/'campaign';campaign.mkdir(exist_ok=True)
    state=campaign/'campaign_state.json'
    if not state.exists():state.write_text(json.dumps(dict(last_accepted_version='baseline')))
    env.update(P1_CAMPAIGN_ROOT=str(root.parent.parent),P1_CAMPAIGN_OUTPUT=str(campaign))
    if name.startswith('run_gpu_'):
        for key in ['FORCE_CPU','FORCE_GPU','SGPL_GPU_BACKEND']:env.pop(key,None)
    arguments=[str(root)] if name in {'run_gpu_four_tests','run_gpu_batch_tests','run_gpu_sync_tests','run_gpu_storage_tests'} else []
    with log.open('w') as output:
        try:
            trace=['-x'] if os.environ.get('REGRESSION_TRACE')=='1' else []
            run=subprocess.run(['bash',*trace,str(script),*arguments],cwd=src,env=env,stdout=output,
                               stderr=subprocess.STDOUT,timeout=1800)
            code=run.returncode
        except subprocess.TimeoutExpired:code=124
    result=dict(suite=name,exit_code=code,elapsed_seconds=time.time()-start,log=str(log))
    results.append(result)
    tmp=summary.with_suffix('.tmp');tmp.write_text(json.dumps(results,indent=2));tmp.replace(summary)
    print(json.dumps(result),flush=True)
    if code:print(log.read_text(errors='replace')[-3000:],flush=True)
print('SUITES COMPLETE',summary,flush=True)
latest={x['suite']:x for x in results}
sys.exit(1 if any(x['exit_code'] for x in latest.values()) else 0)
