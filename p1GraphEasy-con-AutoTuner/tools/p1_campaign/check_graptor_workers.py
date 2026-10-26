"""Correctness-only worker and affinity probe for the frozen backend builds."""
from pathlib import Path
import sys,subprocess,json,hashlib
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import benchmark_graptor_backends as B
records=[]
for label in ['cilk','openmp']:
    config=B.ROOT/'worker-probe'/label;config.mkdir(exist_ok=True,parents=True)
    text=(B.SRC/'config.h').read_text()
    if label=='cilk':text=text.replace('GRAPTOR_PARALLEL BACKEND_openmp','GRAPTOR_PARALLEL BACKEND_cilk')
    (config/'config.h').write_text(text)
    cmd=['g++','-O3','-std=c++23','-march=native','-DMAX_VL=8','-DLONG','-I'+str(config),'-I'+str(B.RTS/'include'),'-I'+str(B.DEP/'include'),'-I'+str(B.SRC),'-I'+str(B.SRC/'include'),str(B.REPO/'tools/p1_campaign/graptor_worker_probe.cpp'),str(B.HIST/'graptor/constants.o'),'-L'+str(B.DEP/'lib'),'-lconfig++','-o',str(config/'probe')]
    cmd+=['-L'+str(B.RTS/'lib'),'-lcilkrts','-ldl','-pthread'] if label=='cilk' else ['-fopenmp']
    B.checked(cmd,config/'build.log')
    for width in [1,2,6]:
        env=B.env(label,width)
        env['GRAPTOR_PROBE_CPUS']=','.join(map(str,B.CPUS[:width]))
        p=subprocess.run(['taskset','-c',','.join(map(str,B.CPUS[:width])),str(config/'probe'),str(width)],env=env,capture_output=True,text=True,timeout=60)
        print(label,width,p.returncode,p.stdout,p.stderr[-1000:],flush=True);assert p.returncode==0
        records.append(dict(variant=label,workers=width,**json.loads(p.stdout)))
B.save(B.OUT/'worker_probe.json',dict(passed=True,checks=records,source_sha256=B.sha(B.REPO/'tools/p1_campaign/graptor_worker_probe.cpp'),binaries={label:B.sha(B.ROOT/'worker-probe'/label/'probe') for label in ['cilk','openmp']}))
