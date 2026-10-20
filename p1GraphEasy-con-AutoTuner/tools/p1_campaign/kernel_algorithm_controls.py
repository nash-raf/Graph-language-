"""Diagnostic Graptor controls for label numbering and source-read semantics."""
import kernel_gap as K
R=K.R
from pathlib import Path
import shlex,subprocess,sys,re,time
W=Path('/home/kazi/p1_kernel_20261020/algorithm_controls');W.mkdir(parents=True,exist_ok=True)
G=Path('/mnt/d/sgpl/graptor')
SPECS={
 'cc_original_labels':('cc',['-DPUSH_ZERO=0','-DPRESET_ZERO=0','-DFUSION=0','-DINITID=1']),
 'cc_snapshot':('cc',['-DPUSH_ZERO=0','-DPRESET_ZERO=0','-DFUSION=0','-DINITID=1','-DLEVEL_ASYNC=0']),
 'sssp_snapshot':('sssp',['-DLEVEL_ASYNC=0'])}
def build():
 built={}
 for name,(a,extra) in SPECS.items():
  folder=W/name;folder.mkdir(exist_ok=True);source='CCv.C' if a=='cc' else 'BFvExport.C';target='CCv_VEBOGraptorPullDataParCached' if a=='cc' else 'BFExport'
  src=folder/source
  if not src.exists():src.write_text((G/'bench'/source).read_text())
  assert src.read_text()==(G/'bench'/source).read_text()
  opts={}
  for line in (G/'build/bench/CMakeFiles'/(target+'.dir')/'flags.make').read_text().splitlines():
   if line.startswith('CXX_'):k,v=line.split('=',1);opts[k.strip()]=shlex.split(v)
  exe=folder/name;obj=folder/(name+'.o');command=['g++',*opts['CXX_DEFINES'],*opts['CXX_INCLUDES'],*opts['CXX_FLAGS'],*extra,'-c',src,'-o',obj]
  if not exe.exists():
   R.progress(stage='build_algorithm_control',algorithm=a,variant=name)
   R.checked(command,folder,folder/'build.log',timeout=300)
   lib=G/'build/external/Build/libconfig_project/lib'
   R.checked(['g++','-fopenmp','-O3','-DNDEBUG',obj,'-Wl,-rpath,'+str(lib),lib/'libconfig++.so',G/'build/libgraptorlib.a','-o',exe],folder,folder/'link.log')
  built[name]=dict(algorithm=a,flags=extra,command=list(map(str,command)),source_sha256=R.sha(src),executable_sha256=R.sha(exe),archive_sha256=R.sha(G/'build/libgraptorlib.a'),path=str(exe))
 R.save(R.OUT/'algorithm_control_builds.json',built);return built
def run():
 built=build();results=[];work=[];original=R.binary_cmd
 for x in R.read(R.OUT/'selected_cases.json'):
  a=x['algorithm']
  if a not in ['cc','sssp']:continue
  row=next(r for r in R.M.ROWS if r['graph']==x['graph'])
  variants=[('pinned',None)]
  if a=='cc':variants += [('no_zero_no_fusion',R.OLD/'graptor_controls/cc')]
  variants += [(n,Path(z['path'])) for n,z in built.items() if z['algorithm']==a]
  extra={'LD_PRELOAD':'/home/kazi/p1_kernel_20261020/monotonic_gettimeofday.so'}
  good=[]
  for n,p in variants:
   def command(*args):
    c,e,d=original(*args);c.insert(-1,'-itimes');return c,e,d
   R.binary_cmd=command
   z=R.invoke(K.ATTEMPT,'validate_algorithm_controls','graptor',a,row,0,label=n,extra=extra,executable_override=p);R.binary_cmd=original
   if z.get('error'):
    if n=='pinned':raise AssertionError(z)
    work.append(dict(**x,variant=n,error=z['error'],base=z['base'],excluded_from_timing=True));continue
   good.append((n,p))
   lines=[l for l in Path(z['base']+'.err').read_text().splitlines() if l.startswith('Iteration ')]
   arcs=[int(m[1]) for l in lines if (m:=re.search(r'(?:nacte: |E:)(\d+)',l))];vertices=[int(m[1]) for l in lines if (m:=re.search(r'(?:nactv: |V:)(\d+)',l))]
   values=R.np.loadtxt(z['base']+'.values',ndmin=1)[:row['vertices']]
   strict=bool(R.np.array_equal(values,R.M.reference(a,row))) if n in ['cc_original_labels','cc_snapshot'] else None
   if strict is not None:assert strict,'Original CC labels mismatch'
   work.append(dict(**x,variant=n,iterations=len(lines),frontier_arc_volume=sum(arcs),frontier_vertex_volume=sum(vertices),strict_original_cc_labels_match=strict,base=z['base']))
   R.binary_cmd=original
  for rep in range(20):
   for n,p in good if rep%2==0 else list(reversed(good)):
    z=R.invoke(K.ATTEMPT,'algorithm_controls','graptor',a,row,rep,label=n,extra=extra,executable_override=p);assert not z.get('error'),z;results.append(z)
  R.save(R.OUT/'algorithm_controls.json',results);R.save(R.OUT/'algorithm_control_work.json',work);R.progress(stage='algorithm_controls',algorithm=a,graph=row['graph'])
if __name__=='__main__':
 with R.locked('kernel_algorithm_controls'):
  R.verify_inputs();run()
