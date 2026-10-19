"""Test legacy executable-section order without shims or operation-body changes."""
import argparse,shutil,struct,subprocess,time
from pathlib import Path
import numpy as np
import runner as R
SECTIONS=['.sgpl_cut_preparation','.sgpl_compaction','.sgpl_preparation','.sgpl_lean_records','.sgpl_lean_dispatch','.sgpl_packed_records']
ANCHORS='\n/* Preserve existing executor-section order when extending the runtime. */\n#if defined(__GNUC__) && defined(__ELF__)\n'+''.join('__asm__(".pushsection '+s+',\\\"ax\\\",@progbits\\n.popsection");\n' for s in SECTIONS)+'#endif\n'
def sections(path):
 data=Path(path).read_bytes();assert data[:6]==b'\x7fELF\x02\x01';off=struct.unpack_from('<Q',data,40)[0];stride,num,strings=struct.unpack_from('<HHH',data,58)
 headers=[struct.unpack_from('<IIQQQQIIQQ',data,off+i*stride) for i in range(num)];h=headers[strings];names=data[h[4]:h[4]+h[5]]
 return {names[h[0]:].split(b'\0',1)[0].decode():data[h[4]:h[4]+h[5]] for h in headers if h[1]!=8}
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);p.add_argument('--cases',required=True);args=p.parse_args();R.M=R.bootstrap()
 with R.locked('fullgraph_section_controls'):
  R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in [args.baseline,args.version]};selected=R.read(args.cases);d=R.ROOT/'versions'/args.version
  text=(d/'src/autotuner_runtime.c').read_text();needle='#include "autotuner_runtime.h"';assert text.count(needle)==1;text=text.replace(needle,needle+ANCHORS)
  identity=R.key([fingerprints,text,selected,R.sha(Path(__file__))])[:16];work=d/('full_section_controls_'+identity);src=work/'src';src.mkdir(parents=True,exist_ok=True)
  for item in (d/'src').iterdir():
   if item.suffix in ['.c','.h','.inc'] and not (src/item.name).exists():shutil.copy2(item,src/item.name)
  (src/'autotuner_runtime.c').write_text(text);obj=work/'section_order.o'
  if not obj.exists():R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.','-c','autotuner_runtime.c','-o',obj],src,work/'build.log')
  default=sections(d/'autotuner_runtime.o');control=sections(obj);code=[s for s in default if s.startswith('.text') or s.startswith('.sgpl_')]
  equality={s:default[s]==control.get(s) for s in code};assert all(equality.values()),equality
  builds={};hashes={}
  for x in selected:
   a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph']);folder=work/a/row['graph'].removesuffix('.edges');folder.mkdir(parents=True,exist_ok=True)
   for mode in ['timed','validate']:
    exe=folder/mode
    if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',R.case(args.version,a,row)/(mode+'.o'),*[obj if n=='autotuner_runtime' else d/(n+'.o') for n in R.OBJECTS],R.ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',exe],folder,folder/(mode+'.link.log'))
    builds[a,row['graph'],mode]=exe;hashes[str(exe)]=R.sha(exe)
   (folder/'sections.txt').write_text(subprocess.check_output(['readelf','-SW',str(folder/'timed')],text=True))
   (folder/'symbols.txt').write_text(subprocess.check_output(['nm','-S','-n',str(folder/'timed')],text=True))
  R.save(R.OUT/(args.version+'_section_controls_'+identity+'_protocol.json'),dict(identity=identity,selected=selected,fingerprints=fingerprints,executables=hashes,unrelocated_code_sections_equal=equality,change='Empty assembler section anchors preserve the six legacy executor sections before the new full runtime. No extra executable instructions, ABI shim, source-map or operation edit.'))
  records=[];start=time.time()
  for completed,x in enumerate(selected,1):
   a=x['algorithm'];row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
   for workers in [1,2,6]:
    configs=[('baseline',args.baseline,{},False),('candidate',args.version,{},False),('disabled',args.version,{'SGPL_NO_FULLGRAPH':'1'},False),('ordered_sections',args.version,{},True),('aa',args.baseline,{},False)]
    for label,v,extra,override in configs:
     rec=R.invoke(args.version,'validate_sections_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'validate'] if override else None);assert rec.get('validation') and not rec.get('error'),rec
     R.invoke(args.version,'sections_warmup_'+identity,v,a,row,0,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'timed'] if override else None)
    for rep in range(20):
     shift=rep%len(configs)
     for label,v,extra,override in configs[shift:]+configs[:shift]:records.append(R.invoke(args.version,'sections_'+identity,v,a,row,rep,label=label,workers=workers,extra=extra,executable_override=builds[a,row['graph'],'timed'] if override else None))
   R.progress(stage='fullgraph_section_controls',algorithm=a,completed=completed,total=len(selected),elapsed_seconds=time.time()-start)
  R.calibrate('cc','fullgraph_section_controls',[selected,20,[1,2,6],hashes],start);R.save(R.OUT/(args.version+'_section_controls_'+identity+'_records.json'),records)
  summary=[];rng=np.random.default_rng(2358)
  for x in selected:
   for workers in [1,2,6]:
    rows=[q for q in records if q['algorithm']==x['algorithm'] and q['graph']==x['graph'] and q['workers']==workers];values={label:np.array([q['seconds'] for q in rows if q['label']==label and not q.get('error')]) for label in {q['label'] for q in rows}}
    entry=dict(x,workers=workers,mean_ms={label:float(v.mean()*1000) for label,v in values.items()},comparisons={})
    for n,de in [('candidate','baseline'),('ordered_sections','candidate'),('ordered_sections','baseline'),('aa','baseline')]:
     b=values[de];c=values[n];assert len(b)==len(c)==20;ix=rng.integers(0,20,(10000,20));entry['comparisons'][n+'_over_'+de]=dict(ratio=float(c.mean()/b.mean()),paired_bootstrap95=list(np.quantile(c[ix].mean(axis=1)/b[ix].mean(axis=1),[.025,.975])))
    summary.append(entry)
  R.save(R.OUT/(args.version+'_section_controls_'+identity+'_summary.json'),summary);print('Section controls complete '+identity,flush=True)
if __name__=='__main__':main()
