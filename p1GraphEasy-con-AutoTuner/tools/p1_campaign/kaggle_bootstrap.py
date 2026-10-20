"""Private Kaggle CPU script: verify/extract source bundle, build, validate, time.

Only source and measurement artifacts are inputs/outputs. No API token is used
inside this notebook. It is needed only by the local submission client.
"""
from pathlib import Path
import hashlib,importlib.util,json,os,shutil,subprocess,sys,tarfile,time,urllib.request,zipfile,traceback
WORK=Path('/kaggle/working');SCRATCH=Path('/kaggle/temp/p1-static-schedules');LOG=WORK/'bootstrap.log'
EXPECTED_MANIFEST='__BUNDLE_MANIFEST_SHA256__'
LIBCONFIG_COMMIT='82d21512f46bf7381e50cbc521b3df85d67d15e5'
def run(cmd,cwd=None):
 print('RUN',*map(str,cmd),flush=True)
 with LOG.open('a') as f:
  p=subprocess.Popen(list(map(str,cmd)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,bufsize=1)
  for line in p.stdout:
   f.write(line)
   if line.startswith('{"stage"'):print(line.rstrip(),flush=True)
  p.wait()
 if p.returncode:
  print(LOG.read_text(errors='replace')[-5000:],flush=True)
  raise RuntimeError('Command failed with exit '+str(p.returncode))
def main():
 print('Verifying private campaign input...',flush=True);SCRATCH.mkdir(parents=True,exist_ok=True)
 archives=list(Path('/kaggle/input').rglob('p1-campaign.blob')) or list(Path('/kaggle/input').rglob('p1-campaign.tar.gz'))
 if archives:
  archive=archives[0]
  expected=json.loads((archive.parent/'archive-identity.json').read_text())['sha256']
  actual=hashlib.sha256(archive.read_bytes()).hexdigest();assert actual==expected,'Archive checksum mismatch'
  if not (SCRATCH/'p1-bundle').exists():
   with tarfile.open(archive) as t:t.extractall(SCRATCH,filter='data')
 else:
  # Kaggle automatically expands uploaded tar.gz files beneath a directory
  # named after the archive. Verify the manifest before using that tree.
  manifests=list(Path('/kaggle/input').rglob('bundle_manifest.json'))
  assert len(manifests)==1,'Expected exactly one campaign bundle'
  assert hashlib.sha256(manifests[0].read_bytes()).hexdigest()==EXPECTED_MANIFEST,'Manifest checksum mismatch'
  if not (SCRATCH/'p1-bundle').exists():shutil.copytree(manifests[0].parent,SCRATCH/'p1-bundle')
 bundle=SCRATCH/'p1-bundle';manifest=json.loads((bundle/'bundle_manifest.json').read_text())
 assert hashlib.sha256((bundle/'bundle_manifest.json').read_bytes()).hexdigest()==EXPECTED_MANIFEST,'Manifest checksum mismatch'
 for name,h in manifest.items():assert hashlib.sha256((bundle/name).read_bytes()).hexdigest()==h,name
 # Use only distinct physical cores for dependency compilation as well.
 spec=importlib.util.spec_from_file_location('p1_portable_preflight',bundle/'candidate/tools/p1_campaign/portable_campaign.py')
 preflight=importlib.util.module_from_spec(spec);spec.loader.exec_module(preflight)
 selected,host=preflight.physical_cpus();os.sched_setaffinity(0,selected)
 (WORK/'preflight.json').write_text(json.dumps(host,indent=2))
 print('Physical CPU representatives:',selected,'; SMT siblings excluded',flush=True)
 key=Path('/etc/apt/keyrings/p1-llvm.asc');key.parent.mkdir(parents=True,exist_ok=True)
 with urllib.request.urlopen('https://apt.llvm.org/llvm-snapshot.gpg.key',timeout=60) as r:key.write_bytes(r.read())
 codename=next(s.split('=',1)[1].strip('"') for s in Path('/etc/os-release').read_text().splitlines() if s.startswith('VERSION_CODENAME='))
 Path('/etc/apt/sources.list.d/p1-llvm.list').write_text(f'deb [signed-by={key}] https://apt.llvm.org/{codename}/ llvm-toolchain-{codename}-20 main\n')
 run(['apt-get','update','-qq'])
 run(['apt-get','install','-y','-qq','llvm-20-dev','libpolly-20-dev','clang-20','libnlopt-dev','libconfig++-dev','pkg-config','unzip'])
 deps=SCRATCH/'dependencies';deps.mkdir(exist_ok=True)
 z=deps/'antlr4-cpp-runtime-4.13.2-source.zip'
 with urllib.request.urlopen('https://www.antlr.org/download/antlr4-cpp-runtime-4.13.2-source.zip',timeout=60) as r:z.write_bytes(r.read())
 save=dict(antlr_archive_sha256=hashlib.sha256(z.read_bytes()).hexdigest(),llvm=subprocess.check_output(['llvm-config-20','--version'],text=True).strip())
 (WORK/'dependency-identity.json').write_text(json.dumps(save,indent=2))
 antlr=deps/'antlr';antlr.mkdir(exist_ok=True)
 with zipfile.ZipFile(z) as t:t.extractall(antlr)
 run(['cmake','-S',antlr,'-B',deps/'antlr-build','-DCMAKE_BUILD_TYPE=Release','-DANTLR_BUILD_CPP_TESTS=OFF','-DANTLR_BUILD_STATIC=OFF'])
 run(['cmake','--build',deps/'antlr-build','-j',str(len(selected))])
 run(['cmake','--install',deps/'antlr-build']);run(['ldconfig'])
 # Match Graptor's original dependency commit. Ubuntu's older packaged
 # libconfig lacks the std::string overloads used by the frozen source.
 libarchive=deps/'libconfig.tar.gz'
 with urllib.request.urlopen('https://github.com/hyperrealm/libconfig/archive/'+LIBCONFIG_COMMIT+'.tar.gz',timeout=60) as r:libarchive.write_bytes(r.read())
 with tarfile.open(libarchive) as t:t.extractall(deps,filter='data')
 libsrc=deps/('libconfig-'+LIBCONFIG_COMMIT)
 run(['cmake','-S',libsrc,'-B',deps/'libconfig-build','-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTS=OFF','-DBUILD_EXAMPLES=OFF','-DBUILD_CXX=ON'])
 run(['cmake','--build',deps/'libconfig-build','-j',str(len(selected))]);run(['cmake','--install',deps/'libconfig-build']);run(['ldconfig'])
 save.update(libconfig_commit=LIBCONFIG_COMMIT,libconfig_archive_sha256=hashlib.sha256(libarchive.read_bytes()).hexdigest())
 (WORK/'dependency-identity.json').write_text(json.dumps(save,indent=2))
 os.environ['P1_PORTABLE_WORK']=str(SCRATCH/'p1-campaign')
 campaign=bundle/'candidate/tools/p1_campaign/portable_campaign.py'
 # Apply the sanitizer launch limit even to earlier frozen test scripts.
 # This adapter changes only the contract subprocess, never acceptance runs.
 launcher=SCRATCH/'launch_campaign.py'
 launcher.write_text('''import sys
sys.path.insert(0,'''+repr(str(campaign.parent))+''')
import portable_campaign as P
R,cpus=P.setup()
original=R.checked
def checked(cmd,cwd,log,env=None,timeout=1800):
    if any(str(x).endswith('run_static_schedule_tests.sh') for x in cmd):
        cmd=['bash','-c','ulimit -s 8192; exec "$@"','p1-sanitizer',*map(str,cmd)]
    return original(cmd,cwd,log,env,timeout)
R.checked=checked
sys.argv=['''+repr(str(campaign))+''','all']
P.main()
''')
 run([sys.executable,launcher])
 print('Campaign completed; all changes retained. See p1-campaign/results/comparison.html',flush=True)
if __name__=='__main__':
 try:main()
 except BaseException:
  (WORK/'failure.txt').write_text(traceback.format_exc());raise
 finally:
  # Preserve compact evidence even when a build or validation fails. Exclude
  # caches and executable copies; their hashes and source inputs are retained.
  root=SCRATCH/'p1-campaign'
  with tarfile.open(WORK/'p1-results.tar.gz','w:gz',compresslevel=3) as t:
   for p in [WORK/'bootstrap.log',WORK/'failure.txt',WORK/'dependency-identity.json',WORK/'preflight.json',root/'host.json',root/'results',root/'runs']:
    if p.exists():t.add(p,arcname=str(p.relative_to(SCRATCH)) if p.is_relative_to(SCRATCH) else p.name)
   for p in (root/'versions').glob('*/*'):
    if p.is_file() and p.suffix in ['.json','.patch']:t.add(p,arcname=str(p.relative_to(SCRATCH)))
   for p in root.rglob('*.log'):
    if 'results' not in p.parts and 'runs' not in p.parts:t.add(p,arcname=str(p.relative_to(SCRATCH)))
  for name in ['comparison.html','comparison.json']:
   if (root/'results'/name).exists():shutil.copy2(root/'results'/name,WORK/name)
