#!/usr/bin/env python3
"""Retain native test binaries and complete logs; never benchmark here."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import sys

repo=Path(__file__).resolve().parents[1]
attempt=sys.argv[1] if len(sys.argv)>1 else 'native01'
assert attempt.replace('_','').isalnum()
out=Path('/home/kazi/p1_layout_native_20261105/tests')/attempt
out.mkdir(parents=True,exist_ok=True)
files=[p for p in repo.iterdir() if p.is_file() and p.suffix in {'.c','.h','.inc'}]
files+=list((repo/'test').glob('*.c'))
identity={str(p.relative_to(repo)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
manifest=out/'sources.json'
if manifest.exists():assert json.loads(manifest.read_text())==identity,'Use a new attempt for changed sources'
else:manifest.write_text(json.dumps(identity,indent=2))
san=os.environ.get('LAYOUT_TEST_SANITIZER','')
flags=['-O1','-g','-no-pie','-fno-omit-frame-pointer','-fsanitize='+san] if san else ['-O2','-g']
cmd=['gcc',*flags,'-std=gnu11','-pthread','-iquote',str(repo),str(repo/'test/layout_execution_test.c'),
     str(repo/'autotuner_runtime.c'),str(repo/'parallel_runtime.c'),'-lnlopt','-lm',
     '-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create','-o',str(out/'native')]
with (out/'build.log').open('w') as log:
    subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True)
results=[]
for workers in [1,2,6]:
    env=dict(os.environ,SGPL_NUM_THREADS=str(workers),SGPL_ALLOW_HT='0')
    with (out/(str(workers)+'.log')).open('w') as log:
        run=subprocess.run([str(out/'native')],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
    results.append(dict(workers=workers,exit_code=run.returncode))
    (out/'results.json').write_text(json.dumps(results,indent=2))
    print(results[-1],flush=True)
    if run.returncode:sys.exit(run.returncode)
print('PASS',out,flush=True)
