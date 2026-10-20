"""Audit activation and exact accepted/candidate/disabled PR vectors (no timing)."""
import os
os.environ.setdefault('P1_CAMPAIGN_OUTPUT','/mnt/c/p1-static-schedules-20261020')
import runner as R
import numpy as np
from pathlib import Path
import re,subprocess
R.M=R.bootstrap()
def main():
 candidate='StaticSchedule2';accepted='R4a3';activation=[];exact=[]
 with R.locked('static_schedule_proof_audit'):
  R.verify_inputs();R.verify_fingerprint(candidate);R.verify_fingerprint(accepted)
  for a in R.ALGS:
   for row in R.M.ROWS:
    obj=R.case(candidate,a,row)/'timed.o'
    active='autograph_exec_ctx_bind_static_schedule_v1' in subprocess.check_output(['nm','-u',obj],text=True)
    activation.append(dict(algorithm=a,graph=row['graph'],active=active,object_sha256=R.sha(obj)))
  for w in [1,2,6]:
   for row in R.M.ROWS:
    vectors=[];counts=[]
    for v,label,extra in [(accepted,'accepted',{}),(candidate,'candidate',{'SGPL_STATIC_SCHEDULE_DEBUG':'1'}),(candidate,'disabled',{'SGPL_NO_STATIC_SCHEDULE':'1'})]:
     x=R.invoke('static_schedule_proof','validate_bits',v,'pagerank',row,0,label=label,workers=w,extra=extra)
     assert not x.get('error'),x
     s=Path(x['base']+'.out').read_text().split('VALUES_BEGIN\n',1)[1].split('VALUES_END',1)[0]
     vectors.append(np.array(s.split(),dtype=np.float64).tobytes())
     counts.append(Path(x['base']+'.err').read_text().count('[static-schedule] prepare'))
    assert all(b==vectors[0] for b in vectors),'Rank bit differs: '+row['graph']
    assert counts[0]==counts[2]==0 and counts[1]>0,('Activation/fallback mismatch',row['graph'],w,counts)
    exact.append(dict(graph=row['graph'],workers=w,passed=True,rank_bits_sha256=R.hashlib.sha256(vectors[0]).hexdigest(),static_dispatches=counts[1]))
   R.progress(stage='exact_static_schedule',workers=w,completed=len(exact),total=183)
  R.save(R.OUT/'static_schedule_activation.json',activation);R.save(R.OUT/'static_schedule_exact_vectors.json',exact)
  print('PASS',len(exact),'exact accepted/candidate/disabled vector groups; activation',sum(x['active'] for x in activation),'/',len(activation),flush=True)
if __name__=='__main__':main()
