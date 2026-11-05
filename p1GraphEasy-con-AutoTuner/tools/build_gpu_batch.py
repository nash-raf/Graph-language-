import os,sys,concurrent.futures,shutil
from pathlib import Path
repo=Path('/mnt/d/sgpl/Graph-language-/p1GraphEasy-con-AutoTuner')
os.environ.update(P1_CAMPAIGN_ROOT='/home/kazi/p1_gpu_batch_20261104',P1_CAMPAIGN_OUTPUT=str(repo/'bench_results/gpu_batch_20261104'))
sys.path.insert(0,str(repo/'tools/p1_campaign'))
import runner as R
R.ROOT.mkdir(exist_ok=True)
cache=R.ROOT/'object_cache'
if not cache.exists():cache.symlink_to('/home/kazi/p1_execution_simplification/object_cache',target_is_directory=True)
d,src,identity=R.freeze('candidate02');logs=d/'build_logs';logs.mkdir(exist_ok=True)
flags=['g++','-O2','-mavx2','-march=native','-std=c++17','-fexceptions','-pthread','-I/usr/local/include/antlr4-runtime','-Igenerated','-iquote','.', '-I'+R.cfg('--includedir')[0]+'/polly',*[x for x in R.cfg('--cxxflags') if x!='-fno-exceptions']]
sources=['main.cpp','IRGenVisitor.cpp','MotifPattern.cpp','MotifIRBuilder.cpp','ASTBuilder.cpp','pdg.cpp','parallel_loop_outline.cpp','graph_frontier_lowering.cpp','SemanticAnalyzer.cpp','roaring_bitmap.cpp','AutoTunerPass.cpp']+[str(p.relative_to(src)) for p in (src/'generated').glob('*.cpp')]
def cc(s):
 print('BUILD',s,flush=True)
 return R.compile_cache(src/s,src,flags,logs/(s.replace('/','_')+'.log'))
with concurrent.futures.ThreadPoolExecutor(2) as pool:objects=list(pool.map(cc,sources))
R.checked(['g++',*objects,*R.cfg('--ldflags'),'-Wl,--no-keep-memory','-pthread','-lantlr4-runtime',*R.cfg('--libs','all'),*R.cfg('--system-libs'),'-o',d/'GraphProgram'],src,logs/'compiler_link.log')
old=Path('/home/kazi/p1_execution_simplification/versions/iteration_fusion02')
for name in R.OBJECTS:
 if name=='gpu_runtime':R.checked(['gcc','-O3','-g','-fopenmp','-pthread','-iquote','.', '-c','gpu_runtime.c','-o',d/(name+'.o')],src,logs/(name+'.log'))
 else:shutil.copy2(old/(name+'.o'),d/(name+'.o'))
R.save(d/'build_identity.json',dict(source_hash=identity['source_hash'],compiler=R.sha(d/'GraphProgram'),objects={n:R.sha(d/(n+'.o')) for n in R.OBJECTS}))
print('BUILD COMPLETE',d,flush=True)

