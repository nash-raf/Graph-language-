"""Export native candidate patches and immutable source/executable provenance."""
import argparse,re
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--baseline',default='B0_final');args=p.parse_args()
candidate=R.read(R.ROOT/'versions'/args.version/'identity.json');baseline=R.read(R.ROOT/'versions'/args.baseline/'identity.json')
assert candidate and baseline,'Missing frozen source identities'
project=R.git('rev-parse','--show-prefix').strip('/')
uncommitted_paths=re.findall(r'^diff --git a/(.*?) b/',candidate['diff'],re.M)
assert not any(p.startswith(project+'/') and '/' not in p[len(project)+1:] and R.Path(p).suffix in ['.c','.cpp','.h','.inc','.ll'] for p in uncommitted_paths),'Uncommitted native changes need a patch from the frozen source snapshot'
paths=[':(glob)'+project+'/*'+ext for ext in ['.c','.cpp','.h','.inc','.ll']]
patch=R.git('diff','--binary',baseline['commit'],candidate['commit'],'--',*paths)+'\n'
out=R.OUT/'patches';out.mkdir(exist_ok=True);path=out/(args.version+'.patch')
if path.exists():assert path.read_text()==patch,'Archived patch already exists with different content'
else:path.write_text(patch)
metadata={'baseline':baseline,'candidate':candidate,'candidate_build':R.read(R.ROOT/'versions'/args.version/'build.json'),'baseline_version':args.baseline,'native_patch_sha256':R.sha(path),'frozen_tests':str(R.ROOT/'versions'/args.version/'src/test')}
mp=out/(args.version+'.json')
if mp.exists():assert R.read(mp)==metadata
else:R.save(mp,metadata)
print(str(path))
