"""Run a contract on 1/2/6 distinct physical cores from the inherited mask."""
import os,sys
from pathlib import Path
width=int(sys.argv[1]);assert 1<=width<=6
selected=[];seen=set()
for cpu in sorted(os.sched_getaffinity(0)):
 root=Path('/sys/devices/system/cpu')/('cpu'+str(cpu))/'topology'
 physical=((root/'physical_package_id').read_text(),(root/'core_id').read_text())
 if physical not in seen:seen.add(physical);selected.append(cpu)
assert len(selected)>=width,(width,selected)
os.sched_setaffinity(0,selected[:width])
os.environ.update(SGPL_NUM_THREADS=str(width),SGPL_ALLOW_HT='0',OMP_NUM_THREADS=str(width),OMP_DYNAMIC='FALSE')
os.execvpe(sys.argv[2],sys.argv[2:],os.environ)
