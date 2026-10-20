"""CPU-only Kaggle hardware and toolchain inspection. No benchmark timings."""
from pathlib import Path
import json,os,platform,shutil,subprocess
allowed=sorted(os.sched_getaffinity(0));cores={};topology=[]
for cpu in allowed:
 root=Path('/sys/devices/system/cpu')/f'cpu{cpu}'/'topology'
 key=tuple((root/name).read_text().strip() for name in ['physical_package_id','core_id'])
 cores.setdefault(key,cpu);topology.append(dict(cpu=cpu,package=key[0],core=key[1]))
quota={str(p):p.read_text().strip() for p in [Path('/sys/fs/cgroup/cpu.max'),Path('/sys/fs/cgroup/cpu/cpu.cfs_quota_us'),Path('/sys/fs/cgroup/cpu/cpu.cfs_period_us')] if p.exists()}
info=Path('/proc/cpuinfo').read_text();versions={}
for tool in ['g++','gcc','cmake','llvm-config-20','clang++-20','python']:
 if shutil.which(tool):versions[tool]=subprocess.run([tool,'--version'],capture_output=True,text=True).stdout.splitlines()[:1]
data=dict(platform=platform.platform(),os_release=Path('/etc/os-release').read_text(),uid=os.getuid(),
 allowed_cpus=allowed,topology=topology,physical_representatives=list(cores.values()),quota=quota,
 cpu_model=next((s for s in info.splitlines() if s.startswith('model name')),''),
 flags=next((s for s in info.splitlines() if s.startswith('flags')),''),tools=versions,
 disk_free_bytes=shutil.disk_usage('/kaggle/working').free)
Path('/kaggle/working/preflight.json').write_text(json.dumps(data,indent=2));print(json.dumps(data,indent=2))
