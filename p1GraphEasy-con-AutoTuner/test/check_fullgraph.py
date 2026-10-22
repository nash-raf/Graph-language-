"""General frontend activation/refusal and exact stdout differential fixtures."""
from pathlib import Path
import os,subprocess,tempfile,json
r=Path.cwd();version=r.parent
env=dict(os.environ,SGPL_FULLGRAPH_STATS='1',GRAPH_FRONTIER_STATS='1',SGPL_WITNESS_DUMP='1',SGPL_ALLOW_HT='0',FORCE_CPU='1')
def run(cmd,cwd,e):
 p=subprocess.run(list(map(str,cmd)),cwd=cwd,env=e,capture_output=True,text=True,timeout=180)
 assert not p.returncode,(cmd,p.stdout[-2000:],p.stderr[-4000:]);return p
with tempfile.TemporaryDirectory() as temporary:
 d=Path(temporary)
 for path in r.glob('*.ll'):(d/path.name).symlink_to(path)
 graph=d/'input.edges';graph.write_text('0 2\n0 1\n1 1\n1 1\n2 1\n2 0\n3 1\n3 3\n4 2\n5 2\n')
 pre='graph G { directed: true; edges: file "'+str(graph)+'"; };\nint n=numVertices(G);real x[n];real y[n];int out_degree[n];\nfor v in G.nodes {x[v]=1.0/(v+1);y[v]=0.0;out_degree[v]=0;}\nfor u in G.nodes {for v in G.neighbors(u){out_degree[u]=out_degree[u]+1;}}\n'
 loops={
  'eligible':'for u in G.nodes {for v in G.neighbors(u){y[v]=y[v]+0.85*x[u]/out_degree[u];}}',
  'alias':'for u in G.nodes {for v in G.neighbors(u){x[v]=x[v]+x[u];}}',
  'source_write':'for u in G.nodes {for v in G.neighbors(u){y[u]=y[u]+x[v];}}',
  'opaque':'for u in G.nodes {for v in G.neighbors(u){print v;y[v]=y[v]+x[u];}}',
  'partial':'int u=0;while(u<n-1){for v in G.neighbors(u){y[v]=y[v]+x[u];}u=u+1;}',
 }
 objects=[version/(x+'.o') for x in ['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime','roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']]
 for name,loop in loops.items():
  program=d/(name+'.graph');program.write_text(pre+loop+'\nfor v in G.nodes {print x[v];print y[v];}\n');outputs=[]
  for mode in ['sequential','generic','specialized','no_records']:
   e=dict(env)
   if mode=='sequential':e['GRAPH_DISABLE_PDG']='1'
   if mode=='generic':e['SGPL_NO_FULLGRAPH_LOWERING']='1'
   if mode=='no_records':e.update(SGPL_NO_RECORD_LIVENESS='1',SGPL_NO_LEAN_RECORDS='1')
   compiled=run([version/'GraphProgram','--ir-backend=cpu','--emit-ir-to='+str(d/'fixture.ll'),program],d,e)
   evidence=Path(os.environ['P1_CAMPAIGN_OUTPUT'])/'fullgraph_frontend';evidence.mkdir(exist_ok=True)
   (evidence/(name+'_'+mode+'.log')).write_text(compiled.stderr)
   (evidence/(name+'_'+mode+'.ll')).write_text((d/'fixture.ll').read_text())
   (evidence/(name+'.graph')).write_text(program.read_text())
   ir=(d/'fixture.ll').read_text();active='[fullgraph] activate ' in compiled.stderr
   assert active==(name=='eligible' and mode in ['specialized','no_records']),(name,mode,compiled.stderr[-5000:])
   assert 'sgpl_fullgraph_certificate' not in ir
   if active:assert 'sgpl_full_incoming' in ir and 'autograph_exec_ctx_bind_fullgraph_v1' in ir and 'sgpl_full_source_map' in ir
   exe=d/'check';run(['g++','-O3','-fopenmp','-no-pie',d/'program.o',*objects,'-pthread','-ldl','-lnlopt','-o',exe],d,e)
   for width in [int(w) for w in os.environ.get('P1_TEST_WIDTHS','1 2 6').split()]:
    for disabled in ([False,True] if mode in ['specialized','no_records'] else [False]):
     ee=dict(e,SGPL_NUM_THREADS=str(width),OMP_NUM_THREADS=str(width))
     if disabled:ee['SGPL_NO_FULLGRAPH']='1'
     outputs.append(run(['python3',r/'test/physical_run.py',str(width),exe],d,ee).stdout)
  if name=='alias':
   # Existing accepted P1 snapshots in-place reads. This deliberately refused
   # fixture must preserve that compiler's behavior, not redefine its semantics.
   campaign=Path(os.environ.get('P1_CAMPAIGN_OUTPUT','/mnt/c/p1campaign-results'))
   accepted=json.loads((campaign/'campaign_state.json').read_text())['last_accepted_version']
   base=Path(os.environ.get('P1_CAMPAIGN_ROOT',str(Path.home()/'p1_campaign_20261019')))/'versions'/accepted
   run([base/'GraphProgram','--ir-backend=cpu',program],d,env)
   run(['g++','-O3','-fopenmp','-no-pie',d/'program.o',*[base/p.name for p in objects],'-pthread','-ldl','-lnlopt','-o',exe],d,env)
   reference=[run(['python3',r/'test/physical_run.py',str(width),exe],d,dict(env,SGPL_NUM_THREADS=str(width),OMP_NUM_THREADS=str(width))).stdout for width in [int(w) for w in os.environ.get('P1_TEST_WIDTHS','1 2 6').split()]]
   assert all(x==reference[0] for x in outputs[3:]+reference),(name,outputs,reference)
   (evidence/'alias_reference.json').write_text(json.dumps(dict(accepted=accepted,sequential=outputs[0],generic=outputs[3],accepted_output=reference[0],reason='Existing round snapshot semantics; new full-graph path refused'),indent=2))
  else:assert all(x==outputs[0] for x in outputs),(name,outputs)
  print('PASS fullgraph activation/refusal and differential',name,flush=True)
