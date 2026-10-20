"""Private Kaggle CPU script: verify/extract source bundle, build, validate, time.

Only source and measurement artifacts are inputs/outputs. No API token is used
inside this notebook. It is needed only by the local submission client.
"""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,tarfile,time,urllib.request,zipfile,traceback
WORK=Path('/kaggle/working');LOG=WORK/'bootstrap.log'
EXPECTED_MANIFEST='__BUNDLE_MANIFEST_SHA256__'
def run(cmd,cwd=None):
 print('RUN',*map(str,cmd),flush=True)
 with LOG.open('a') as f:
  p=subprocess.run(list(map(str,cmd)),cwd=cwd,stdout=f,stderr=subprocess.STDOUT)
 if p.returncode:
  print(LOG.read_text(errors='replace')[-5000:],flush=True)
  raise RuntimeError('Command failed with exit '+str(p.returncode))
def main():
 archives=list(Path('/kaggle/input').rglob('p1-campaign.tar.gz'))
 if archives:
  archive=archives[0]
  expected=json.loads((archive.parent/'archive-identity.json').read_text())['sha256']
  actual=hashlib.sha256(archive.read_bytes()).hexdigest();assert actual==expected,'Archive checksum mismatch'
  if not (WORK/'p1-bundle').exists():
   with tarfile.open(archive) as t:t.extractall(WORK,filter='data')
 else:
  # Kaggle automatically expands uploaded tar.gz files beneath a directory
  # named after the archive. Verify the manifest before using that tree.
  manifests=list(Path('/kaggle/input').rglob('bundle_manifest.json'))
  assert len(manifests)==1,'Expected exactly one campaign bundle'
  assert hashlib.sha256(manifests[0].read_bytes()).hexdigest()==EXPECTED_MANIFEST,'Manifest checksum mismatch'
  if not (WORK/'p1-bundle').exists():shutil.copytree(manifests[0].parent,WORK/'p1-bundle')
 bundle=WORK/'p1-bundle';manifest=json.loads((bundle/'bundle_manifest.json').read_text())
 for name,h in manifest.items():assert hashlib.sha256((bundle/name).read_bytes()).hexdigest()==h,name
 # Use only distinct physical cores for dependency compilation as well.
 cores={}
 for cpu in sorted(os.sched_getaffinity(0)):
  p=Path('/sys/devices/system/cpu')/f'cpu{cpu}'/'topology'
  k=tuple((p/x).read_text().strip() for x in ['physical_package_id','core_id']);cores.setdefault(k,cpu)
 selected=list(cores.values())[:6];os.sched_setaffinity(0,selected)
 print('Physical CPU representatives:',selected,'; SMT siblings excluded',flush=True)
 key=Path('/etc/apt/keyrings/p1-llvm.asc');key.parent.mkdir(parents=True,exist_ok=True)
 urllib.request.urlretrieve('https://apt.llvm.org/llvm-snapshot.gpg.key',key)
 codename=next(s.split('=',1)[1].strip('"') for s in Path('/etc/os-release').read_text().splitlines() if s.startswith('VERSION_CODENAME='))
 Path('/etc/apt/sources.list.d/p1-llvm.list').write_text(f'deb [signed-by={key}] https://apt.llvm.org/{codename}/ llvm-toolchain-{codename}-20 main\n')
 run(['apt-get','update','-qq'])
 run(['apt-get','install','-y','-qq','llvm-20-dev','libpolly-20-dev','clang-20','libnlopt-dev','libconfig++-dev','pkg-config','unzip'])
 deps=WORK/'dependencies';deps.mkdir(exist_ok=True)
 z=deps/'antlr4-cpp-runtime-4.13.2-source.zip'
 urllib.request.urlretrieve('https://www.antlr.org/download/antlr4-cpp-runtime-4.13.2-source.zip',z)
 save=dict(antlr_archive_sha256=hashlib.sha256(z.read_bytes()).hexdigest(),llvm=subprocess.check_output(['llvm-config-20','--version'],text=True).strip())
 (WORK/'dependency-identity.json').write_text(json.dumps(save,indent=2))
 antlr=deps/'antlr';antlr.mkdir(exist_ok=True)
 with zipfile.ZipFile(z) as t:t.extractall(antlr)
 run(['cmake','-S',antlr,'-B',deps/'antlr-build','-DCMAKE_BUILD_TYPE=Release','-DANTLR_BUILD_CPP_TESTS=OFF','-DANTLR_BUILD_STATIC=OFF'])
 run(['cmake','--build',deps/'antlr-build','-j',str(len(selected))])
 run(['cmake','--install',deps/'antlr-build']);run(['ldconfig'])
 campaign=bundle/'candidate/tools/p1_campaign/portable_campaign.py'
 run([sys.executable,campaign,'all'])
 print('Campaign completed; all changes retained. See p1-campaign/results/comparison.html',flush=True)
if __name__=='__main__':
 try:main()
 except BaseException:
  (WORK/'failure.txt').write_text(traceback.format_exc());raise
 finally:
  # Preserve compact evidence even when a build or validation fails. Exclude
  # caches and executable copies; their hashes and source inputs are retained.
  root=WORK/'p1-campaign'
  with tarfile.open(WORK/'p1-results.tar.gz','w:gz',compresslevel=3) as t:
   for p in [WORK/'bootstrap.log',WORK/'failure.txt',WORK/'dependency-identity.json',root/'host.json',root/'results',root/'runs']:
    if p.exists():t.add(p,arcname=str(p.relative_to(WORK)))
   for p in (root/'versions').glob('*/*'):
    if p.is_file() and p.suffix in ['.json','.patch']:t.add(p,arcname=str(p.relative_to(WORK)))
   for p in root.rglob('*.log'):
    if 'results' not in p.parts and 'runs' not in p.parts:t.add(p,arcname=str(p.relative_to(WORK)))
