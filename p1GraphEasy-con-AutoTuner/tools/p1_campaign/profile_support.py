"""Build private diagnostic runtimes and run resumable profiling blocks."""
from pathlib import Path
import ast,json,re,shutil,time

def constant(path,name):
 for node in ast.parse(path.read_text()).body:
  if isinstance(node,ast.Assign) and any(isinstance(t,ast.Name) and t.id==name for t in node.targets):return ast.literal_eval(node.value)
 raise ValueError(name)

def build_profile_runtime(R,v):
 d=R.ROOT/'versions'/v;src=d/'src';out=d/'profile_src'
 if (d/'profile_build.json').exists():return
 out.mkdir(exist_ok=True)
 for p in src.iterdir():
  if p.suffix in ['.c','.h','.inc']:shutil.copy2(p,out/p.name)
 # Pin the diagnostic templates into this campaign before using them.
 templates=R.ROOT/'support/profile_templates.json'
 if not templates.exists():
  R.save(templates,{'phases':constant(R.OLDOUT/'build_diagnostics.py','header'),'pool':constant(R.OLDOUT/'build_diagnostics.py','pool_header'),'work':constant(R.OLDOUT/'work_volume.py','header')})
 t=R.read(templates)
 (out/'diag.h').write_text(t['phases'])
 p=out/'autotuner_runtime.c';text=p.read_text();at=text.index('/* Forward declaring external Roaring C API');text=text[:at]+'#include "diag.h"\n'+text[at:]
 for fn,i in [('sgpl_exec_step_dispatch_body',4),('sgpl_exec_spatial_begin',6),('sgpl_exec_spatial_finish',7)]:
  text,n=re.subn(r'(\b'+fn+r'\([^;]*?\)\s*\{)',lambda m:m[1]+f' NR({i});',text,count=1);assert n==1,fn
 p.write_text(text)
 p=out/'array_frontier_runtime.inc';text=p.read_text()
 for fn,i in [('sgpl_array_records',0),('sgpl_array_weight_indices',3),('sgpl_array_compact',5)]:
  text,n=re.subn(r'(\b'+fn+r'\([^;]*?\)\s*\{)',lambda m:m[1]+f' NR({i});',text,count=1);assert n==1,fn
 needle='static SGPL_PREPARATION_FN int32_t sgpl_array_records(sgpl_exec_ctx *c,AutoGraphMeta *m,int begins,int ends) {'
 assert needle in text;text=text.replace(needle,t['work']+'\n'+needle+' WScope ws __attribute__((cleanup(wv_leave)))={c,m};')
 p.write_text(text)
 p=out/'array_preparation_runtime.inc';text=p.read_text()
 for i,fn in [(1,'sgpl_array_count_chunks'),(2,'sgpl_array_fill_chunks')]:
  text,n=re.subn(r'(  sgpl_run_preparation\([^;]*?'+fn+r'[^;]*?;)',lambda m:'{ NR('+str(i)+'); '+m[1]+' }',text,count=1)
  # Specialized paths can have their own preparation; zero means uninstrumented.
  assert n<=1
 p.write_text(text)
 pool=t['pool'].replace('word==(atomic_uint*)&g_sgpl_pool_epoch?0:(word==&g_sgpl_pool_remaining?1:2)','word==&g_sgpl_pool_remaining?1:0')
 p=out/'parallel_runtime.c';text=p.read_text();insert='static void sgpl_pool_wait(atomic_uint *word, unsigned expected,'
 assert insert in text;text=text.replace(insert,pool+'\n'+insert,1)
 text,n=re.subn(r'(static void sgpl_pool_wait\(atomic_uint \*word, unsigned expected,\s*atomic_uint \*waiters\)\s*\{)',lambda m:m[1]+'\n DiagWait dw __attribute__((cleanup(diag_wait_leave)))=diag_wait_enter(word);',text,count=1);assert n==1
 text=text.replace('uint64_t end = sgpl_now_ns() + g_tls_pool_spin_ns;','uint64_t end = sgpl_now_ns() + g_tls_pool_spin_ns;dw.stats->budget_ns+=g_tls_pool_spin_ns;',1)
 text=text.replace('    atomic_fetch_add(waiters, 1);\n#ifdef __linux__','    dw.park=sgpl_now_ns();\n    atomic_fetch_add(waiters, 1);\n#ifdef __linux__',1);p.write_text(text)
 for name in ['autotuner_runtime','parallel_runtime']:
  R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-c',name+'.c','-o',name+'.o'],out,out/(name+'.build.log'))
 R.save(d/'profile_build.json',{'private':True,'instrumented':True,'template_hash':R.sha(templates)})

def instrument(R,v,selected=None):
 build_profile_runtime(R,v)
 d=R.ROOT/'versions'/v;out=d/'profile_src'
 selected=selected or [{'algorithm':a,'graph':r['graph']} for a in R.ALGS for r in R.M.ROWS]
 for x in selected:
  r=next(r for r in R.M.ROWS if r['graph']==x['graph']);c=R.case(v,x['algorithm'],r)
  for mode in ['timed','validate']:
   target=c/('profile_validate' if mode=='validate' else 'profile')
   if target.exists():continue
   libs=[out/(n+'.o') if n in ['autotuner_runtime','parallel_runtime'] else d/(n+'.o') for n in R.OBJECTS]
   R.checked(['g++','-O3','-fopenmp','-no-pie',c/(mode+'.o'),*libs,R.ROOT/'monotonic_timer.o','-Wl,--wrap=timer_runtime','-pthread','-ldl','-lnlopt','-o',target],c,c/(mode+'.profile_link.log'))

def run_profiles(R,args):
 versions=[args.version]+([args.baseline] if args.baseline else [])
 selected=R.read(args.cases) if args.cases else [{'algorithm':a,'graph':r['graph']} for a in R.ALGS for r in R.M.ROWS]
 if args.mode=='phases':
  for v in versions:instrument(R,v,selected)
 records=[];extra=json.loads(args.extra)
 for a in R.ALGS:
  cases=[x for x in selected if x['algorithm']==a]
  if not cases:continue
  start=time.time()
  for i,x in enumerate(cases):
   r=next(r for r in R.M.ROWS if r['graph']==x['graph'])
   for v in versions:
    valid=R.invoke(args.version,'validate_profile_'+args.mode,v,a,r,0,workers=args.workers,extra=extra,profile=args.mode)
    assert not valid.get('error'),valid
    for rep in range(5):
     rec=R.invoke(args.version,'profile_'+args.mode,v,a,r,rep,workers=args.workers,extra=extra,profile=args.mode);records.append(rec)
   R.progress(stage='profile_'+args.mode,version=args.version,algorithm=a,completed=i+1,total=len(cases),failures=sum(bool(x.get('error')) for x in records))
  R.calibrate(a,'profile_'+args.mode,[selected,len(versions),args.workers,5,extra],start)
 R.save(R.OUT/(args.version+'_profile_'+args.mode+'_w'+str(args.workers)+'_'+R.key([selected,extra])[:8]+'.json'),records)
