#!/usr/bin/env python3
"""Retain matched evidence for an existing failure; never mark it passed."""
from pathlib import Path
import os,sys,subprocess,json,hashlib,resource
root=Path('/home/kazi/p1_layout_native_20261105/versions');candidate=sys.argv[1]
out=root/candidate/'known_failures';out.mkdir(exist_ok=True)
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
results=[]
for version in ['baseline',candidate]:
    v=root/version;source=v/'src/test/membership_call_aliases.graph'
    log=out/(version+'_membership_call_aliases.log')
    env=dict(os.environ,FORCE_CPU='1',FORCE_GPU='0',SGPL_GPU_BACKEND='0',
             GRAPH_FRONTIER_STATS='1',SGPL_WITNESS_DUMP='1',SGPL_FRONTIER_STRICT='1')
    # A private compile directory prevents interfering with active test program.o.
    d=out/version;d.mkdir(exist_ok=True)
    for p in (v/'src').glob('*.ll'):
        if not (d/p.name).exists():(d/p.name).symlink_to(p)
    if not (d/'test').exists():(d/'test').symlink_to(v/'src/test',target_is_directory=True)
    with log.open('w') as output:
        run=subprocess.run([str(v/'GraphProgram'),str(source),'--ir-backend=cpu'],cwd=d,
                           env=env,stdout=output,stderr=subprocess.STDOUT,timeout=120)
    assert run.returncode and 'raw CSR access conflicts with a graph bitmap query' in log.read_text()
    results.append(dict(version=version,exit_code=run.returncode,log=str(log),
                        fixture_sha256=hashlib.sha256(source.read_bytes()).hexdigest()))
assert results[0]['fixture_sha256']==results[1]['fixture_sha256']
(out/'membership_call_aliases.json').write_text(json.dumps(results,indent=2))
src=root/candidate/'src/test/run_parallel_claims_tests.sh'
original=src.read_text();updated=original.replace('names=(membership_call_aliases ','names=(')
assert updated!=original
target=src.with_name('run_parallel_claims_remaining_tests.sh');target.write_text(updated)
(out/'remaining_claims_overlay.json').write_text(json.dumps(dict(original=str(src),overlay=str(target),
    original_sha256=hashlib.sha256(original.encode()).hexdigest(),
    overlay_sha256=hashlib.sha256(updated.encode()).hexdigest(),
    reason='Skip only the separately reproduced baseline failure; run every remaining assertion unchanged'),indent=2))
print('Confirmed same existing compiler failure; remaining-fixture overlay ready',flush=True)
