"""Same-binary Graptor default-vs-passive waiting diagnostic, with A/A."""
import kernel_gap as K
R=K.R
from pathlib import Path
def run():
 records=[];lib='/home/kazi/p1_kernel_20261020/monotonic_gettimeofday.so'
 specs=[('default',{}),('passive',{'OMP_WAIT_POLICY':'PASSIVE','GOMP_SPINCOUNT':'0'}),('AA',{})]
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);v='graptor_double' if a=='pagerank' else 'graptor'
  for n,e in specs:
   z=R.invoke(K.ATTEMPT,'validate_graptor_waiting',v,a,row,0,label=n,extra=dict(e,LD_PRELOAD=lib));assert not z.get('error'),z
  for rep in range(20):
   for n,e in specs if rep%2==0 else list(reversed(specs)):
    z=R.invoke(K.ATTEMPT,'graptor_waiting',v,a,row,rep,label=n,extra=dict(e,LD_PRELOAD=lib));assert not z.get('error'),z;records.append(z)
  R.save(R.OUT/'graptor_waiting.json',records);R.progress(stage='graptor_waiting',algorithm=a,graph=row['graph'])
if __name__=='__main__':
 with R.locked('kernel_graptor_waiting'):
  R.verify_inputs();run()
