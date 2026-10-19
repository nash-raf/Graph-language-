"""Audit inactive CC object code without attributing performance from addresses."""
import argparse,struct
from pathlib import Path
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',required=True);args=p.parse_args();R.M=R.bootstrap()
def sections(path):
 data=Path(path).read_bytes();assert data[:6]==b'\x7fELF\x02\x01'
 off=struct.unpack_from('<Q',data,40)[0];stride,num,strings=struct.unpack_from('<HHH',data,58)
 headers=[struct.unpack_from('<IIQQQQIIQQ',data,off+i*stride) for i in range(num)]
 h=headers[strings];names=data[h[4]:h[4]+h[5]];result={}
 for h in headers:
  name=names[h[0]:].split(b'\0',1)[0].decode()
  if h[1]!=8:result[name]=data[h[4]:h[4]+h[5]]
 return result
with R.locked('audit_inactive_codegen'):
 R.verify_inputs();fingerprints={v:R.verify_fingerprint(v) for v in [args.baseline,args.version]};records=[]
 for row in R.M.ROWS:
  a=sections(R.case(args.baseline,'cc',row)/'timed.o');b=sections(R.case(args.version,'cc',row)/'timed.o')
  left=a['.text'];right=b['.text'];diff=[dict(offset=i,baseline=x,candidate=y) for i,(x,y) in enumerate(zip(left,right)) if x!=y]
  records.append(dict(graph=row['graph'],baseline_text_bytes=len(left),candidate_text_bytes=len(right),text_differences=diff,readonly_sections_equal={n:a[n]==b.get(n) for n in a if n.startswith('.rodata')}))
 private=[]
 for path in (R.ROOT/'versions'/args.version).glob('full_cross_controls_*/data_section.o'):
  a=sections(R.ROOT/'versions'/args.version/'autotuner_runtime.o');b=sections(path)
  private.append(dict(path=str(path),candidate_default_text_equals_data_control=a['.text']==b['.text'],candidate_full_text_equals_data_control=a['.sgpl_full_runtime']==b['.sgpl_full_runtime']))
 result=dict(baseline=args.baseline,candidate=args.version,fingerprints=fingerprints,cc_programs=records,private_controls=private,limitation='Unrelocated object bytes and section placement do not establish a cache, scheduling or instruction-level performance cause.')
 path=R.OUT/(args.version+'_inactive_codegen_audit.json')
 if path.exists():assert R.read(path)==result
 else:R.save(path,result)
 print(path)
