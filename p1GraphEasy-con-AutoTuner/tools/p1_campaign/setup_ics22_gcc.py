#!/usr/bin/env python3
from pathlib import Path
import subprocess, urllib.request, concurrent.futures, hashlib, json, os
R=Path('/home/kazi/p1_graptor_ics22_20261026'); S=R/'gcc-7.2.0'; D=R/'downloads'
names=['gmp-6.1.0.tar.bz2','mpfr-3.1.4.tar.bz2','mpc-1.0.3.tar.gz','isl-0.16.1.tar.bz2']
def fetch(n):
 p=D/n
 if not p.exists(): urllib.request.urlretrieve('https://gcc.gnu.org/pub/gcc/infrastructure/'+n,p)
 return p
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool: list(pool.map(fetch,names))
checks={line.split()[1]:line.split()[0] for line in (S/'contrib/prerequisites.sha512').read_text().splitlines()}
for n in names:
 p=D/n; assert hashlib.sha512(p.read_bytes()).hexdigest()==checks[n],n
 subprocess.run(['tar','-xf',str(p),'-C',str(S)],check=True)
 short=n.split('-')[0]; version=n.removesuffix('.tar.bz2').removesuffix('.tar.gz')
 if not (S/short).exists(): (S/short).symlink_to(version)
manifest={str(p.relative_to(S)):hashlib.sha256(p.read_bytes()).hexdigest() for p in S.rglob('*') if p.is_file() and not any(x in p.parts for x in ['.git'])}
(R/'gcc-source-before.json').write_text(json.dumps(manifest))
(R/'downloads-sha256.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in D.iterdir() if p.is_file()},indent=2))
b=R/'gcc-build'; b.mkdir(exist_ok=True)
env=os.environ.copy()
cmd=[str(S/'configure'),'--prefix='+str(R/'gcc-7.2-install'),'--disable-bootstrap','--disable-multilib','--enable-languages=c,c++','--enable-libcilkrts','--disable-libsanitizer','--disable-libmpx','--disable-libvtv','--disable-werror']
(R/'gcc-configure-command.json').write_text(json.dumps(cmd,indent=2))
with open(R/'gcc-configure.log','w') as f: subprocess.run(cmd,cwd=b,env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
with open(R/'gcc-build.log','w') as f: subprocess.run(['make','-j6'],cwd=b,env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
with open(R/'gcc-install.log','w') as f: subprocess.run(['make','install'],cwd=b,env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
print('GCC 7.2 installed',flush=True)