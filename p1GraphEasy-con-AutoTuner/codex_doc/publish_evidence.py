#!/usr/bin/env python3
"""Copy compact reproducibility evidence into the repository; keep native logs."""
from pathlib import Path
import sys,json,hashlib,shutil,subprocess
repo=Path(__file__).resolve().parent
root=Path('/home/kazi/p1_layout_native_20261105');version=sys.argv[1]
out=repo/'evidence';out.mkdir(exist_ok=True)
paths=[root/'cpu_inputs.json',root/'final_audit.json',root/'walltime.c',root/'versions/baseline/build.json',
       root/'versions/baseline/cpu_validation.json',root/'versions/baseline/regressions/results.json']
v=root/'versions'/version
paths += [v/p for p in ['build.json','cpu_identity.json','cpu_build_progress.json','cpu_validation.json',
                       'regressions/results.json','autotuner_checks/results.json','autotuner_checks/test_overlay.json',
                       'layout_checks/results.json','layout_checks/activation.json','temporal_differential/results.json',
                       'claims_differential/results.json',
                       'known_failures/membership_call_aliases.json','known_failures/remaining_claims_overlay.json',
                       'test_overlays/round_probe.json']]
paths += list((v/'known_failures').glob('*.log'))
paths += list((v/'known_failures').glob('*.json'))
paths += list((v/'known_failures').glob('*.out'))
paths += list((v/'test_overlays').glob('*.json'))
paths += list((root/'versions/baseline/test_overlays').glob('*.json'))
paths += list((v/'regressions').glob('*.log'))
paths += list((root/'versions/baseline/regressions').glob('*.log'))
for attempt in ['native13_asan','native14_tsan','native15_asan']:
    paths+=list((root/'tests'/attempt).glob('*.json'))
    paths+=list((root/'tests'/attempt).glob('*.log'))
paths+=list((root/'benchmarks'/version).glob('*.json')) if (root/'benchmarks'/version).exists() else []
index=[]
for path in paths:
    if not path.exists() or path.name.startswith('stamp_'):continue
    target=out/path.relative_to(root);target.parent.mkdir(parents=True,exist_ok=True)
    if path.suffix=='.log' and path.stat().st_size>250000:
        target=target.with_suffix('.excerpt.txt')
        target.write_text('Full log: '+str(path)+'\nSHA256: '+hashlib.sha256(path.read_bytes()).hexdigest()+
                          '\nLast 12000 characters:\n'+path.read_text(errors='replace')[-12000:])
        index.append(dict(native=str(path),copy=str(target.relative_to(repo)),excerpt=True,
                          full_bytes=path.stat().st_size,full_sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        continue
    shutil.copy2(path,target)
    index.append(dict(native=str(path),copy=str(target.relative_to(repo)),bytes=target.stat().st_size,
                      sha256=hashlib.sha256(target.read_bytes()).hexdigest()))
(out/'machine.txt').write_text(subprocess.check_output(['lscpu'],text=True))
(out/('index_'+version+'.json')).write_text(json.dumps(dict(candidate=version,artifacts=index,
    note='Retained evidence snapshot. Full native build/test logs remain at the recorded paths.'),indent=2))
print('Published',len(index),'artifacts',out)
