"""Run diagnostic stages sequentially. Each stage owns the campaign lock."""
import subprocess,sys
from pathlib import Path
stages=[('audit','kernel_gap.py','audit'),('clock','kernel_graptor.py','clock'),('measure_mono','kernel_gap.py','measure_mono'),('controls','kernel_gap.py','controls'),('work','kernel_graptor.py','work'),('cc','kernel_graptor.py','cc'),('publication','kernel_publication.py',None),('publication_bits','kernel_publication.py','bits'),('late_opt','kernel_late_opt.py',None),('full_mono','kernel_gap.py','full_mono'),('window_samples','kernel_window_samples.py',None),('window_analysis','kernel_window_analysis.py',None),('graptor_waiting','kernel_graptor_waiting.py',None),('algorithm_controls','kernel_algorithm_controls.py',None),('report_analysis','kernel_report.py',None),('write_report','kernel_write_report.py',None)]
start=sys.argv[1] if len(sys.argv)>1 else 'audit'
for name,script,arg in stages[next(i for i,x in enumerate(stages) if x[0]==start):]:
 print('STAGE',name,flush=True)
 subprocess.run([sys.executable,str(Path(__file__).parent/script),*([arg] if arg else [])],check=True)
