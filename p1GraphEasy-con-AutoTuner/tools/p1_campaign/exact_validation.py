"""Validation-only full-vector double-bit comparison; never acceptance timing."""
import json,struct,time

def run(R,candidate,baseline):
 R.verify_inputs()
 fingerprints={v:R.verify_fingerprint(v) for v in [baseline,candidate]}
 helper=R.HERE/'exact_bits.c';identity=R.key([R.sha(helper),fingerprints,'printf-puts-v1'])[:16]
 root=R.ROOT/'exact_validation'/identity;root.mkdir(parents=True,exist_ok=True)
 obj=root/'exact_bits.o'
 if not obj.exists():R.checked(['gcc','-O2','-c',helper,'-o',obj],root,root/'helper.compile.log')
 # Verify interposition catches all 64 bits, including exceptional values.
 patterns=[0,0x8000000000000000,0x7ff0000000000000,0xfff0000000000000,0x7ff8000000001234,0x3ff0000000000001]
 test=root/'selftest.c'
 test.write_text('#include <stdio.h>\n#include <stdint.h>\n#include <string.h>\nint main(void){uint64_t raw[]={'+','.join(str(x)+'ULL' for x in patterns)+'};puts("VALUES_BEGIN");for(unsigned i=0;i<sizeof(raw)/8;++i){double x;memcpy(&x,raw+i,8);printf("%.13g\\n",x);}puts("VALUES_END");}\n')
 if not (root/'selftest').exists():R.checked(['gcc','-O2',test,obj,'-Wl,--wrap=printf','-Wl,--wrap=puts','-o',root/'selftest'],root,root/'selftest.compile.log')
 env=R.M.env();env['SGPL_BITS_OUTPUT']=str(root/'selftest.bits')
 R.checked([root/'selftest'],root,root/'selftest.log',env)
 assert (root/'selftest.bits').read_bytes()==b''.join(struct.pack('<Q',x) for x in patterns),'Exact-bit interposition failed selftest'
 executable_hashes={}
 for v in [baseline,candidate]:
  d=R.ROOT/'versions'/v
  for row in R.M.ROWS:
   case=R.case(v,'pagerank',row);exe=case/('exact_validate.'+identity)
   if not exe.exists():R.checked(['g++','-O3','-fopenmp','-no-pie',case/'validate.o',*[d/(n+'.o') for n in R.OBJECTS],R.ROOT/'monotonic_timer.o',obj,'-Wl,--wrap=timer_runtime','-Wl,--wrap=printf','-Wl,--wrap=puts','-pthread','-ldl','-lnlopt','-o',exe],case,case/('exact_link.'+identity+'.log'))
   executable_hashes[str(exe)]=R.sha(exe)
 protocol=dict(candidate=candidate,baseline=baseline,fingerprints=fingerprints,helper_sha256=R.sha(helper),executables=executable_hashes,dataset=R.read(R.OUT/'manifest.json')['dataset_hash'])
 path=R.OUT/(candidate+'_exact_bits_'+identity+'_protocol.json')
 if path.exists():assert R.read(path)==protocol,'Exact validation fingerprint changed'
 else:R.save(path,protocol)
 rows=[];start=time.time()
 for workers in [1,2,6]:
  for completed,row in enumerate(R.M.ROWS,1):
   records=[R.invoke(candidate,'validate_exact_bits',v,'pagerank',row,0,label=v,workers=workers,extra={'SGPL_BITS_HELPER_ID':identity},profile='bits') for v in [baseline,candidate]]
   assert all(not x.get('error') and x.get('validation') for x in records),records
   raw=[R.Path(x['base']+'.bits').read_bytes() for x in records]
   assert len(raw[0])==row['vertices']*8 and raw[0]==raw[1],('Floating-point order/bit mismatch',row['graph'],workers)
   rows.append(dict(graph=row['graph'],workers=workers,doubles=row['vertices'],sha256=R.hashlib.sha256(raw[0]).hexdigest(),passed=True))
   R.progress(stage='validate_exact_bits',version=candidate,algorithm='pagerank',completed=completed,total=61,failures=0,elapsed_seconds=time.time()-start)
 R.save(R.OUT/(candidate+'_exact_bits_'+identity+'_results.json'),rows)
 print(json.dumps({'exact_double_vectors':len(rows),'passed':True,'protocol':str(path)}),flush=True)
