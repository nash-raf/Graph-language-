#!/usr/bin/env python3
"""Retry only recognized pre-main TSan address-space startup failures.

Every failed launch is retained. Never retry or suppress a TSan race report.
"""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path('/home/kazi/p1_layout_native_20261105/tests')/(sys.argv[1] if len(sys.argv)>1 else 'native05_tsan')
records=[]
for width in [1,2,6]:
    passed=False
    for attempt in range(12):
        log=root/f'w{width}_attempt{attempt}.log'
        env=dict(os.environ,SGPL_NUM_THREADS=str(width),SGPL_ALLOW_HT='0',TSAN_OPTIONS='halt_on_error=1')
        with log.open('w') as output:
            run=subprocess.run([str(root/'native')],env=env,stdout=output,stderr=subprocess.STDOUT,timeout=180)
        text=log.read_text(errors='replace')
        startup=('TEST layout=' not in text and
                 ('ThreadSanitizer: unexpected memory mapping' in text or
                  (run.returncode==-11 and not text.strip())))
        record=dict(workers=width,attempt=attempt,exit_code=run.returncode,
                    startup_failure=startup,log=str(log))
        records.append(record);(root/'race_results.json').write_text(json.dumps(records,indent=2))
        print(json.dumps(record),flush=True)
        if not run.returncode:
            assert 'layout-native executor: 0 failures' in text
            passed=True;break
        if not startup:raise SystemExit('Race/correctness failure; inspect '+str(log))
    if not passed:raise SystemExit('TSan startup unavailable after bounded retries')
print('PASS TSan at 1, 2, 6 workers; startup failures retained separately',flush=True)
