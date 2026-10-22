"""Export only benchmark inputs and source, never credentials or prior binaries."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,tarfile
HERE=Path(__file__).resolve().parent;REPO=HERE.parent.parent
OLD=Path.home()/'p1_campaign_20261019'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def copy(p,q):q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,q)
def main():
 p=argparse.ArgumentParser();p.add_argument('output');p.add_argument('--baseline-source');a=p.parse_args();out=Path(a.output)
 out.mkdir(parents=True,exist_ok=True);stage=out/'bundle'
 if stage.exists():raise FileExistsError('Choose a fresh export directory; existing bundle retained')
 stage.mkdir();paths={};rows=json.loads((OLD/'inputs.json').read_text())
 for r in rows:
  for k in ['original_file','p1_weighted_file','bin','weights','weights_i32']:
   src=Path(r[k]);h=sha(src);target='files/'+h+src.suffix
   if not (stage/target).exists():copy(src,stage/target)
   paths[str(src)]=target
 (stage/'input_paths.json').write_text(json.dumps(paths,indent=2))
 copy(OLD/'inputs.json',stage/'original_inputs.json')
 baseline=Path(a.baseline_source) if a.baseline_source else OLD/'versions/R4a3/src'
 for name,src in [('accepted',baseline),('candidate',REPO)]:
  dst=stage/name;dst.mkdir()
  for f in src.iterdir():
   if f.is_file() and f.suffix in ['.c','.cpp','.h','.inc','.ll']:copy(f,dst/f.name)
  shutil.copytree(src/'generated',dst/'generated',ignore=shutil.ignore_patterns('*.o','*.interp','*.tokens'))
  shutil.copytree(src/'test',dst/'test',ignore=shutil.ignore_patterns('__pycache__','*.o'))
  for script in (dst/'test').rglob('*.sh'):
   script.write_bytes(script.read_bytes().replace(b'\r\n',b'\n'))
 shutil.copytree(OLD/'references',stage/'references')
 shutil.copytree(OLD/'frozen_reports',stage/'frozen_reports')
 copy(OLD/'historical_harness/p1_bench_snapshot.py',stage/'historical_harness/p1_bench_snapshot.py')
 copy(OLD/'support/common.py',stage/'support/common_original.py')
 for name in ['runner.py','portable_campaign.py']:
  copy(HERE/name,stage/'candidate/tools/p1_campaign'/name)
 copy(OLD/'walltime.c',stage/'walltime.c')
 copy(HERE/'exact_bits.c',stage/'exact_bits.c')
 g=Path('/mnt/d/sgpl/graptor')
 for name in ['include','src','bench']:
  shutil.copytree(g/name,stage/'graptor_source'/name,ignore=shutil.ignore_patterns('*.o','__pycache__'))
 copy(g/'LICENSE',stage/'graptor_source/LICENSE')
 copy(g/'build/config.h',stage/'graptor_source/config.h')
 text=(stage/'graptor_source/bench/PRsgpl.C').read_text()
 assert 'using FloatTy = float;' in text
 (stage/'graptor_source/bench/PRdouble.C').write_text(text.replace('using FloatTy = float;','using FloatTy = double;',1))
 git='/mnt/c/Program Files/Git/cmd/git.exe'
 win=subprocess.check_output(['wslpath','-w',str(REPO)],text=True).strip()
 provenance={'accepted':({'source_directory':str(baseline)} if a.baseline_source else json.loads((OLD/'versions/R4a3/identity.json').read_text())),
  'candidate_commit':subprocess.check_output([git,'-C',win,'rev-parse','HEAD'],text=True).strip(),
  'candidate_patch':subprocess.check_output([git,'-C',win,'diff','HEAD','--','.'],text=True),
  'cpu_only':True,'preserve_candidate_on_regression':True,'graphs':61,'cases':244}
 (stage/'provenance.json').write_text(json.dumps(provenance,indent=2))
 manifest={str(f.relative_to(stage)):sha(f) for f in sorted(stage.rglob('*')) if f.is_file()}
 (stage/'bundle_manifest.json').write_text(json.dumps(manifest,indent=2))
 archive=out/'p1-campaign.tar.gz'
 with tarfile.open(archive,'w:gz',compresslevel=3) as t:t.add(stage,arcname='p1-bundle')
 print(json.dumps(dict(archive=str(archive),sha256=sha(archive),bytes=archive.stat().st_size,files=len(manifest))))
if __name__=='__main__':main()
