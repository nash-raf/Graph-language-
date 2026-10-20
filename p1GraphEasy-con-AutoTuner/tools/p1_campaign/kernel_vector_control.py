"""Pinned double Graptor PageRank at different requested layout vector lengths."""
import kernel_gap as K
R=K.R
from pathlib import Path
import sys
def run():
 records=[];original=R.binary_cmd;length=8
 def command(*args):
  c,e,d=original(*args);c[c.index('-l')+1]=str(length);return c,e,d
 R.binary_cmd=command
 for x in R.read(R.OUT/'selected_cases.json'):
  if x['algorithm']!='pagerank':continue
  row=next(r for r in R.M.ROWS if r['graph']==x['graph']);good=[]
  for length in [8,4,2,1]:
   z=R.invoke(K.ATTEMPT,'validate_vector','graptor_double','pagerank',row,0,label='vl'+str(length),extra={'LD_PRELOAD':'/home/kazi/p1_kernel_20261020/monotonic_gettimeofday.so'})
   records.append(z)
   if not z.get('error'):good.append(length)
  for rep in range(20):
   for length in good if rep%2==0 else list(reversed(good)):
    z=R.invoke(K.ATTEMPT,'vector','graptor_double','pagerank',row,rep,label='vl'+str(length),extra={'LD_PRELOAD':'/home/kazi/p1_kernel_20261020/monotonic_gettimeofday.so'});assert not z.get('error'),z;records.append(z)
  R.save(R.OUT/'vector_control.json',records);R.progress(stage='vector_control',algorithm='pagerank',graph=row['graph'],valid_layout_lengths=good)
if __name__=='__main__':
 with R.locked('kernel_vector_control'):
  R.verify_inputs();run()
