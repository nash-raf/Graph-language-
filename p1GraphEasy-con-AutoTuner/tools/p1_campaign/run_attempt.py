"""Run one complete candidate automatically; decisions never install code.

Only promotion-worthy primary results trigger the expensive full confirmation.
Rejected/inconclusive candidates retain all data and get diagnostic case lists.
"""
from pathlib import Path
import argparse,json,os,subprocess,sys
HERE=Path(__file__).resolve().parent
OUT=Path(os.environ.get('P1_CAMPAIGN_OUTPUT','/mnt/c/p1campaign-results'))
def run(*args):
 log=OUT/(opts.version+'_'+args[0]+('_confirmation' if '--confirmation' in args else '')+'.console.log')
 with log.open('a') as f:
  subprocess.run([sys.executable,str(HERE/'runner.py'),*args],stdout=f,stderr=subprocess.STDOUT,check=True)
 print(json.dumps({'completed_command':args[0],'version':opts.version,'log':str(log)}),flush=True)
def load(p):return json.loads(p.read_text())
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);opts=p.parse_args();OUT.mkdir(exist_ok=True)
for command in ['build','contracts','validate']:run(command,opts.version)
run('benchmark',opts.version,'--baseline',opts.baseline)
primary=load(OUT/(opts.version+'_primary_summary.json'));g=primary['aggregate']['all']['overall']
potential=not primary['failures'] and g['cases']==244 and g['speedup_mean']>1.01 and g['speedup_median']>1.01
if potential:
 run('benchmark',opts.version,'--baseline',opts.baseline,'--confirmation')
 decision=load(OUT/(opts.version+'_decision.json'))
 if decision['decision']=='invalid_aa':
  run('benchmark',opts.version,'--baseline',opts.baseline,'--confirmation','--repeat-block','1')
  decision=load(OUT/(opts.version+'_decision.json'))
else:
 decision=dict(primary,decision='reject',promotable=False,reason='no_worthwhile_primary_gain' if not primary['failures'] else 'failed_primary_measurements')
 (OUT/(opts.version+'_decision.json')).write_text(json.dumps(decision,indent=2))
records=load(OUT/(opts.version+'_primary.json'));cases=[]
for a in ['bfs','sssp','cc','pagerank']:
 graphs=sorted(set(x['graph'] for x in records if x['algorithm']==a))
 ratios=[]
 for graph in graphs:
  samples={v:[x['seconds'] for x in records if x['label']==v and x['algorithm']==a and x['graph']==graph and not x.get('error')] for v in ['baseline','candidate']}
  if len(samples['baseline'])==5 and len(samples['candidate'])==5:ratios.append((sum(samples['candidate'])/sum(samples['baseline']),graph))
 # At least two worst cases and one successful control per algorithm.
 chosen=sorted(ratios,reverse=True)[:2]+sorted(ratios)[:1]
 cases += [dict(algorithm=a,graph=graph) for _,graph in chosen]
for x in decision.get('confirmed_regressions',[]):
 if x.get('graph') and not any(c['algorithm']==x['algorithm'] and c['graph']==x['graph'] for c in cases):cases.append(dict(algorithm=x['algorithm'],graph=x['graph']))
(OUT/(opts.version+'_diagnostic_cases.json')).write_text(json.dumps(cases,indent=2))
print(json.dumps({'decision':decision['decision'],'aggregate':g,'diagnostic_cases':len(cases)}),flush=True)
