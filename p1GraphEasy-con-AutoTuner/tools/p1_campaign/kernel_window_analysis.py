"""Offline PID/time filtering of continuous instruction sampling."""
import kernel_gap as K
R=K.R
from pathlib import Path
import re,sys,statistics,math
def run():
 out=[]
 for x in R.read(R.OUT/'window_samples.json'):
  windows={s['pid']:s for s in [R.json.loads(l) for l in Path(x['markers']).read_text().splitlines()]}
  totals={p:0 for p in windows};nums={p:0 for p in windows};symbols={};unknown=0;retained=0;all_samples=0
  for line in Path(x['samples']).open():
   m=re.match(r'\s*(\d+)\s+(\d+)\.(\d+):\s+(\d+)\s+([0-9a-f]+)\s+(.+)\s+\((.+)\)\s*$',line)
   if not m:continue
   all_samples+=1;pid=int(m[1]);s=windows.get(pid)
   if not s:continue
   ns=int(m[2])*10**9+int(m[3].ljust(9,'0'))
   if not s['start_ns']<=ns<=s['stop_ns']:continue
   period=int(m[4]);sym=m[6].strip();dso=m[7];totals[pid]+=period;nums[pid]+=1;retained+=1
   key=('libgomp:'+sym if 'libgomp' in dso else sym)
   symbols[key]=symbols.get(key,0)+period
   if sym=='[unknown]':unknown+=period
  total=sum(totals.values());assert total>0 and len(totals)==x['repeats']
  log=Path(x['record_log']).read_text();lost=re.findall(r'(?:lost|Lost)\s+(\d+)',log)
  assert not any(int(n)>0 for n in lost),'Lost samples: '+x['record_log']
  funcs={k:100*v/total for k,v in sorted(symbols.items(),key=lambda p:-p[1])}
  row=dict(algorithm=x['algorithm'],graph=x['graph'],framework=x['framework'],repeats=x['repeats'],retained_samples=retained,all_samples=all_samples,estimated_instructions_per_process=total/x['repeats'],approx_poisson95_relative=1.96/math.sqrt(retained),unknown_percent=100*unknown/total,kernel_ms=statistics.mean((s['stop_ns']-s['start_ns'])/1e6 for s in windows.values()),function_instruction_percent=funcs)
  out.append(row)
 R.save(R.OUT/'window_instruction_analysis.json',out)
 comparisons=[]
 for a,g in sorted(set((x['algorithm'],x['graph']) for x in out)):
  q={x['framework']:x for x in out if x['algorithm']==a and x['graph']==g};p=q['p1'];t=q['graptor']
  row=dict(algorithm=a,graph=g,p1_estimated_instructions=p['estimated_instructions_per_process'],graptor_estimated_instructions=t['estimated_instructions_per_process'],ratio=p['estimated_instructions_per_process']/t['estimated_instructions_per_process'],p1_samples=p['retained_samples'],graptor_samples=t['retained_samples'],approx_ratio95_relative=1.96*math.sqrt(1/p['retained_samples']+1/t['retained_samples']),p1_kernel_ms=p['kernel_ms'],graptor_kernel_ms=t['kernel_ms'])
  comparisons.append(row);print(a,g,'instruction_ratio',round(row['ratio'],2),'samples',row['p1_samples'],row['graptor_samples'],'P1top',list(p['function_instruction_percent'].items())[:4],flush=True)
 R.save(R.OUT/'window_instruction_comparison.json',comparisons)
if __name__=='__main__':
 with R.locked('kernel_window_analysis'):run()
