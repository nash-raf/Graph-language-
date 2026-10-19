"""Finish a baseline after an already-running build, without polling full logs."""
import argparse,subprocess,sys,time
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--await-build',action='store_true');args=p.parse_args()
if args.await_build:
 while not (R.ROOT/'versions'/args.version/'build.json').exists():time.sleep(1)
 time.sleep(1) # The builder writes its fingerprint just before releasing its lock.
else:
 subprocess.run([sys.executable,str(R.HERE/'runner.py'),'build',args.version],check=True)
for command in ['contracts','validate','benchmark']:
 log=R.OUT/(args.version+'_'+command+'.console.log')
 with log.open('a') as f:subprocess.run([sys.executable,str(R.HERE/'runner.py'),command,args.version],stdout=f,stderr=subprocess.STDOUT,check=True)
 print(command+' complete: '+str(log),flush=True)
R.M=R.bootstrap();R.save(R.OUT/(args.version+'_comparison.json'),R.aggregate(R.read(R.OUT/(args.version+'_primary.json')),'primary','graptor','candidate'))
s=R.read(R.OUT/'campaign_state.json',{});s.update(last_accepted_version=args.version,baseline_in_progress=None);R.save(R.OUT/'campaign_state.json',s)
print('BASELINE COMPLETE '+args.version,flush=True)
