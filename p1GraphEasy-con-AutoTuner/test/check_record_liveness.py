"""Proof refusals, native occurrence contracts and compiler differential tests."""
from pathlib import Path
import os,shlex,subprocess,tempfile
root=Path.cwd(); version=root.parent
env=dict(os.environ,SGPL_ALLOW_HT='0',SGPL_NUM_THREADS='6',FORCE_CPU='1',ASAN_OPTIONS='detect_leaks=0')
def run(cmd,cwd=root,e=env):
 p=subprocess.run(list(map(str,cmd)),cwd=cwd,env=e,capture_output=True,text=True,timeout=180)
 assert p.returncode==0,(cmd,p.returncode,p.stdout[-2500:],p.stderr[-2500:])
 return p.stdout
def cfg(*args):return shlex.split(subprocess.check_output(['llvm-config-20',*args],text=True))
with tempfile.TemporaryDirectory() as tmp:
 d=Path(tmp); tool=d/'proof'
 run(['g++','-O0','-I.',*cfg('--cxxflags'),'test/record_liveness_ir_test.cpp',*cfg('--ldflags'),*cfg('--libs','core','irreader','analysis'),*cfg('--system-libs'),'-o',tool])
 off=lambda name:int(run([tool,name]))
 specs=[('empty',True,'ret void',''),('safe_load',True,'%x=load ptr,ptr %ctx\nret void',''),
 ('capture',False,'store ptr %ctx,ptr @captured\nret void','@captured=global ptr null'),
 ('unknown',False,'call void @unknown(ptr %ctx)\nret void','declare void @unknown(ptr)'),
 ('write',False,'store i8 0,ptr %ctx\nret void',''),
 ('cast',False,'%x=ptrtoint ptr %ctx to i64\nret void',''),
 ('variable',False,'%i=sext i32 %u to i64\n%p=getelementptr i8,ptr %ctx,i64 %i\n%x=load i8,ptr %p\nret void',''),
 ('append',True,'%x=call i32 @autograph_frontier_activate(ptr %ctx,i32 %v)\nret void','declare i32 @autograph_frontier_activate(ptr,i32)'),
 ('live_accessor',False,'%x=call i64 @autograph_exec_ctx_occurrence(ptr %ctx)\nret void','declare i64 @autograph_exec_ctx_occurrence(ptr)'),
 ('safe_helper',True,'call void @helper(ptr %ctx)\nret void','define internal void @helper(ptr %p){%x=load ptr,ptr %p\nret void}'),
 ('indirect',False,'%f=load ptr,ptr @fn\ncall void %f(ptr %ctx)\nret void','@fn=global ptr null')]
 for name in ['offset','gate_offset','records_offset','sources_offset','work_offset','ops_offset','scratch_offset','format_offset','edges_offset']:
  at=off(name); ty='i32' if name=='format_offset' else 'i64'
  specs.append((name,False,f'%p=getelementptr i8,ptr %ctx,i64 {at}\n%x=load {ty},ptr %p\nret void',''))
 at=off('offset')
 specs.append(('wide',False,f'%p=getelementptr i8,ptr %ctx,i64 {at-8}\n%x=load i128,ptr %p\nret void',''))
 specs.append(('transitive_live',False,'call void @helper(ptr %ctx)\nret void',f'define internal void @helper(ptr %p){{%q=getelementptr i8,ptr %p,i64 {at}\n%x=load i64,ptr %q\nret void}}'))
 for name,want,body,extra in specs:
  path=d/(name+'.ll');path.write_text(extra+'\ndefine internal void @pair(ptr %state,ptr %ctx,i32 %u,i32 %v){\n'+body+'\n}\n')
  p=subprocess.run([tool,path],capture_output=True,text=True)
  assert p.returncode==(0 if want else 1),(name,p.returncode,p.stderr)
 print('PASS LLVM liveness activation/refusal fixtures',len(specs),flush=True)
 base=['gcc','-std=gnu11','-pthread','-I.','test/lean_record_test.c','autotuner_runtime.c','parallel_runtime.c','-lnlopt','-lm','-Wl,--wrap=malloc','-Wl,--wrap=calloc','-Wl,--wrap=pthread_create']
 for mode,flags in [('normal',['-O2']),('sanitized',['-O1','-g','-no-pie','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('no_scratch',['-O2','-DSGPL_EXEC_REUSE_BUFFERS=0'])]:
  exe=d/mode;run([*base,*flags,'-o',exe])
  for workers in [1,2,6]:run([exe],e=dict(env,SGPL_NUM_THREADS=str(workers)))
  print('PASS lean occurrence/weight/layout/failure contracts',mode,flush=True)
 for path in root.glob('*.ll'):(d/path.name).symlink_to(path)
 compiler=version/'GraphProgram'
 objects=[version/(x+'.o') for x in ['runtime','parallel_runtime','gpu_runtime','autotuner_runtime','graph_mutation_runtime','roaring_bitmap','graph_loader_runtime','graph_runtime','semiring_runtime']]
 abi=d/'abi.o';run(['gcc','-O2','-I.',root/'test/execution_abi_test.c','-c','-o',abi])
 run(['g++','-fopenmp',abi,*objects,'-pthread','-ldl','-lnlopt','-o',d/'abi'])
 run([d/'abi']);print('PASS execution descriptor version/size mismatch rejection',flush=True)
 for fixture in ['array_frontier_order','array_frontier_weights','array_program_snapshot','zero_program_array']:
  outputs=[]
  for mode in ['sequential','lean','full']:
   ce=dict(env,SGPL_RECORD_LIVENESS_STATS='1')
   if mode=='sequential':ce['GRAPH_DISABLE_PDG']='1'
   if mode=='full':ce['SGPL_NO_RECORD_LIVENESS']='1'
   run([compiler,'--ir-backend=cpu','--emit-ir-to='+str(d/'fixture.ll'),root/'test'/(fixture+'.graph')],d,ce)
   ir=(d/'fixture.ll').read_text()
   if fixture=='array_frontier_order' and mode=='lean':assert '@autograph_exec_op_create(i64 1025' in ir and '@autograph_execution_abi_check_v1' in ir
   if mode=='full':assert '@autograph_exec_op_create(i64 1025' not in ir
   exe=d/'check';run(['g++','-O3','-fopenmp','-no-pie',d/'program.o',*objects,'-pthread','-ldl','-lnlopt','-o',exe],d)
   for workers in [1,2,6]:outputs.append(run([exe],e=dict(env,SGPL_NUM_THREADS=str(workers))))
  assert all(x==outputs[0] for x in outputs),fixture
  print('PASS sequential/full/lean exact differential',fixture,flush=True)
