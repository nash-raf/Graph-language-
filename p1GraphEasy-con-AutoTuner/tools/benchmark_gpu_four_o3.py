#!/usr/bin/env python3
"""Production-flag rerun after full output validation of the O3 runtime."""
from pathlib import Path
import json,subprocess,shutil
import benchmark_gpu_four as R
R.ROOT=Path('/home/kazi/p1_gpu_four_benchmark_20261104/attempt03_o3_native_io')
# Keep all bookkeeping outside the watched Windows workspace while sampling.
R.OUT=R.ROOT/'results'
PUBLISH=R.REPO/'bench_results/gpu_four_benchmark_20261104/attempt03_o3_native_io'
R.CANDIDATE=Path('/home/kazi/p1_gpu_four_correctness_20261104/correct03_o3/native')
R.BASELINE=R.PREVIOUS
R.BASELINE_LABEL='Original frozen pre-four sync runtime, compiled -O3 -g -fopenmp -pthread; objects retained from saved performance build'
R.CORRECTNESS=R.REPO/'bench_results/gpu_four_correctness_20261104/correct03_o3'
R.OPTIMIZATION='-O3'
def main():
    R.OUT.mkdir(parents=True,exist_ok=True)
    files=['benchmark_gpu_four.py','benchmark_gpu_four_o3.py','benchmark_gpu_saved_cpu.py','summarize_gpu_four.py','check_gpu_four.py']
    for name in files:shutil.copy2(R.REPO/'tools'/name,R.ROOT/name)
    R.B.save(R.OUT/'harness.json',{name:R.B.sha(R.ROOT/name) for name in files})
    old=json.loads((R.REPO/'bench_results/gpu_sync_20261104/attempt02/summary.json').read_text())
    assert R.B.sha(R.BASELINE/'gpu_runtime.o')==old['native_gpu_runtime_sha256']
    for name in R.B.OBJECTS:
        if name not in ['gpu_runtime','autotuner_runtime']:
            assert R.B.sha(R.BASELINE/(name+'.o'))==R.B.sha(R.CANDIDATE/(name+'.o'))
    expected={'autotuner_runtime.h','execution_scratch_runtime.inc','gpu_invocation.inc','gpu_record_reuse.inc','autotuner_runtime.c','gpu_residency.inc'}
    changed=[]
    for p in (R.CANDIDATE/'src').iterdir():
        if p.suffix not in {'.c','.h','.inc','.cpp','.ll'} or p.name=='validation_timer.c':continue
        q=R.BASELINE/'src'/p.name
        if not q.exists() or p.read_text()!=q.read_text():changed.append(p.name)
    assert set(changed)==expected,changed
    producers={}
    for label,native in [('baseline',R.BASELINE),('candidate',R.CANDIDATE)]:
        producers[label]={}
        for name in ['gpu_runtime','autotuner_runtime']:
            info=subprocess.check_output(['readelf','--debug-dump=info',native/(name+'.o')],text=True,stderr=subprocess.DEVNULL)
            producer=next(s.strip() for s in info.splitlines() if 'DW_AT_producer' in s)
            assert ' -O3 ' in producer,producer
            producers[label][name]=producer
    R.B.save(R.OUT/'baseline_audit.json',dict(passed=True,original_saved_pre_change_objects=True,shared_objects_identical=True,
        changed_runtime_sources=sorted(changed),native_flags=['-O3','-g','-fopenmp','-pthread'],compiler_producers=producers,
        bookkeeping_location=str(R.OUT),workspace_report_writes_during_sampling=False))
    R.main()
    PUBLISH.mkdir(parents=True,exist_ok=True)
    for p in R.OUT.iterdir():
        if p.is_file():shutil.copy2(p,PUBLISH/p.name)
    print('Published completed results:',PUBLISH,flush=True)
if __name__=='__main__':main()
