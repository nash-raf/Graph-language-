"""Serialized development compiler/contracts without creating a timed attempt."""
import concurrent.futures,shutil
import runner as R
R.M=R.bootstrap()
with R.locked('compiler_preflight'):
 files=[p for p in R.REPO.iterdir() if p.suffix in {'.c','.cpp','.h','.inc','.ll'}]+[p for p in (R.REPO/'generated').iterdir() if p.suffix in {'.cpp','.h'}]
 identity=R.key({str(p.relative_to(R.REPO)):R.sha(p) for p in files});d=R.ROOT/'development'/identity;src=d/'src';src.mkdir(parents=True,exist_ok=True)
 for p in files:
  out=src/p.relative_to(R.REPO);out.parent.mkdir(exist_ok=True);shutil.copy2(p,out)
 shutil.copytree(R.REPO/'test',src/'test',dirs_exist_ok=True);logs=d/'logs';logs.mkdir(exist_ok=True)
 flags=['g++','-O2','-mavx2','-march=native','-std=c++17','-fexceptions','-pthread','-I/usr/local/include/antlr4-runtime','-Igenerated','-iquote','.', '-I'+R.cfg('--includedir')[0]+'/polly',*[x for x in R.cfg('--cxxflags') if x!='-fno-exceptions']]
 sources=['main.cpp','IRGenVisitor.cpp','MotifPattern.cpp','MotifIRBuilder.cpp','ASTBuilder.cpp','pdg.cpp','parallel_loop_outline.cpp','graph_frontier_lowering.cpp','SemanticAnalyzer.cpp','roaring_bitmap.cpp','AutoTunerPass.cpp']+[str(p.relative_to(src)) for p in (src/'generated').glob('*.cpp')]
 def compile_one(s):
  R.progress(stage='compiler_preflight',item=s,development=str(d));return R.compile_cache(src/s,src,flags,logs/(s.replace('/','_')+'.log'))
 if not (d/'GraphProgram').exists():
  with concurrent.futures.ThreadPoolExecutor(2) as pool:objects=list(pool.map(compile_one,sources))
  R.checked(['g++',*objects,*R.cfg('--ldflags'),'-Wl,--no-keep-memory','-pthread','-lantlr4-runtime',*R.cfg('--libs','all'),*R.cfg('--system-libs'),'-o',d/'GraphProgram'],src,logs/'link.log')
 for name in R.OBJECTS:
  ext='.cpp' if name in ['roaring_bitmap','graph_loader_runtime','graph_runtime'] else '.c'
  flags=['g++','-std=c++17','-mavx2','-march=native'] if ext=='.cpp' else ['gcc']
  obj=R.compile_cache(src/(name+ext),src,[*flags,'-O3','-g','-fopenmp','-pthread','-iquote','.'],logs/(name+'.log'));out=d/(name+'.o')
  if not out.exists():out.symlink_to(obj)
 R.checked(['python3','test/check_fullgraph.py'],src,logs/'fullgraph_contracts.log',R.M.env(),1200)
 R.save(d/'passed.json',dict(source_hash=identity,compiler=R.sha(d/'GraphProgram'),development_only=True))
 print('Compiler preflight passed: '+str(d),flush=True)
