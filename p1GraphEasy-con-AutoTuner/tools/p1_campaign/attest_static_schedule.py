"""Require byte-identical measured programs after a compiler-proof refinement."""
import os
os.environ.setdefault('P1_CAMPAIGN_OUTPUT','/mnt/c/p1-static-schedules-20261020')
import runner as R
import argparse
R.M=R.bootstrap()
def main():
 p=argparse.ArgumentParser();p.add_argument('before');p.add_argument('after');a=p.parse_args()
 with R.locked('static_schedule_byte_equivalence'):
  R.verify_inputs();before=R.verify_fingerprint(a.before);after=R.verify_fingerprint(a.after)
  checked=[]
  for alg in R.ALGS:
   for row in R.M.ROWS:
    for mode in ['timed','validate']:
     for suffix in ['', '.o']:
      name=mode+suffix;x=R.case(a.before,alg,row)/name;y=R.case(a.after,alg,row)/name
      assert R.sha(x)==R.sha(y),('Program bytes changed; existing measurements cannot qualify the new version',alg,row['graph'],name)
      checked.append(dict(algorithm=alg,graph=row['graph'],artifact=name,sha256=R.sha(x)))
  R.save(R.OUT/(a.after+'_byte_equivalence.json'),dict(before=a.before,after=a.after,
   before_fingerprint=before,after_fingerprint=after,identical=before==after,artifacts=checked,
   before_compiler_sha256=R.sha(R.ROOT/'versions'/a.before/'GraphProgram'),after_compiler_sha256=R.sha(R.ROOT/'versions'/a.after/'GraphProgram'),
   reason='Strengthened compile-time TLS refusal; measurement programs and runtime images unchanged'))
  print('PASS',len(checked),'byte-identical program/object artifacts; compiler proof changed, benchmark executables did not',flush=True)
if __name__=='__main__':main()
