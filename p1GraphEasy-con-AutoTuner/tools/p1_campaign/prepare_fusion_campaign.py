"""Package source locally; all build, test, profile and timing work is Kaggle-only."""
from pathlib import Path
import argparse,hashlib,io,json,shutil,subprocess,sys,tarfile
HERE=Path(__file__).resolve().parent;REPO=HERE.parent.parent

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):
    win=subprocess.check_output(['wslpath','-w',str(REPO.parent)],text=True).strip()
    return subprocess.check_output(['/mnt/c/Program Files/Git/cmd/git.exe','-C',win,*args])
def normalized(p):return hashlib.sha256(Path(p).read_text().encode()).hexdigest()

def export_native(commit,destination):
    tree=commit+':p1GraphEasy-con-AutoTuner'
    tracked=git('ls-tree','--name-only',tree).decode().splitlines()
    names=[x for x in tracked if Path(x).suffix in ['.c','.cpp','.h','.inc','.ll']]
    names += [x for x in ['generated','test'] if x in tracked]
    with tarfile.open(fileobj=io.BytesIO(git('archive',tree,'--',*names))) as archive:
        archive.extractall(destination,filter='data')
    if not (destination/'generated').exists():shutil.copytree(REPO/'generated',destination/'generated')

def main():
    p=argparse.ArgumentParser();p.add_argument('output');p.add_argument('--baseline-ref',required=True)
    p.add_argument('--attempt',required=True);p.add_argument('--origin-ref',default='daa63293');p.add_argument('--baseline-note');a=p.parse_args();out=Path(a.output)
    if out.exists():raise FileExistsError('Fresh output required; evidence is immutable')
    out.mkdir(parents=True);baseline=git('rev-parse',a.baseline_ref).decode().strip()
    # Export native inputs and contract fixtures, never repository history or credentials.
    base=out/'baseline';export_native(baseline,base)
    subprocess.run([sys.executable,HERE/'export_kaggle_campaign.py',out/'export','--baseline-source',base],check=True)
    stage=out/'export/bundle'
    origin=git('rev-parse',a.origin_ref).decode().strip()
    if origin!=baseline:
        export_native(origin,stage/'origin_b0')
        for script in (stage/'origin_b0/test').rglob('*.sh'):script.write_bytes(script.read_bytes().replace(b'\r\n',b'\n'))
    for name in ['fusion_campaign.py','fusion_diagnostics.py','audit_local_current_comparison.py','local_current_comparison.py']:
        shutil.copy2(HERE/name,stage/'candidate/tools/p1_campaign'/name)
    protected=['AutoTunerPass.cpp','AutoTunerPass.h','autotuner_runtime.c','autotuner_runtime.h',
        'parallel_runtime.c','parallel_runtime.h','launch_calibration_cache.inc','tdg_scheduler.inc','class_calib.c','hw_calib_bench.c']
    identity={name:normalized(base/name) for name in protected}
    for name,h in identity.items():assert normalized(stage/'candidate'/name)==h,('Protected ARS file changed',name)
    provenance=json.loads((stage/'provenance.json').read_text())
    provenance.update(baseline_commit=baseline,origin_commit=origin,attempt=a.attempt,protected_ars=identity,
        purpose='Effect-proof execution fusion; Kaggle-only; ARS frozen; retain all unsuccessful candidates')
    if a.baseline_note:provenance['baseline_note']=a.baseline_note
    (stage/'provenance.json').write_text(json.dumps(provenance,indent=2))
    manifest={str(f.relative_to(stage)):sha(f) for f in sorted(stage.rglob('*')) if f.is_file() and f.name!='bundle_manifest.json'}
    (stage/'bundle_manifest.json').write_text(json.dumps(manifest,indent=2))
    upload=out/'upload';upload.mkdir()
    with tarfile.open(upload/'p1-campaign.blob','w:gz',compresslevel=3) as t:t.add(stage,arcname='p1-bundle')
    (upload/'archive-identity.json').write_text(json.dumps({'sha256':sha(upload/'p1-campaign.blob')},indent=2))
    slug='p1-effect-fusion-'+a.attempt.lower()
    (upload/'dataset-metadata.json').write_text(json.dumps({'id':'kazisahib/'+slug,'title':'P1 effect fusion '+a.attempt,
        'licenses':[{'name':'other'}],'description':'Private immutable source, exact 61-graph inputs and frozen ARS baseline.'},indent=2))
    notebook=out/'notebook';notebook.mkdir()
    text=(HERE/'kaggle_bootstrap.py').read_text().replace('__BUNDLE_MANIFEST_SHA256__',sha(stage/'bundle_manifest.json'))
    text=text.replace('p1-static-schedules','p1-effect-fusion-'+a.attempt)
    text=text.replace("campaign=bundle/'candidate/tools/p1_campaign/portable_campaign.py'","campaign=bundle/'candidate/tools/p1_campaign/fusion_campaign.py'")
    text=text.replace('import portable_campaign as P','import fusion_campaign as P')
    text=text.replace("['comparison.html','comparison.json']","['comparison.html','comparison.json','final_report.html','final_summary.json','campaign_decision.json']")
    (notebook/'campaign.py').write_text(text)
    (notebook/'kernel-metadata.json').write_text(json.dumps({'id':'kazisahib/'+slug+'-cpu','title':'P1 effect fusion '+a.attempt+' CPU',
        'code_file':'campaign.py','language':'python','kernel_type':'script','is_private':True,'enable_gpu':False,'enable_internet':True,
        'competition_sources':[],'dataset_sources':['kazisahib/'+slug],'kernel_sources':[]},indent=2))
    (out/'identity.json').write_text(json.dumps({'baseline':baseline,'attempt':a.attempt,'manifest':sha(stage/'bundle_manifest.json'),
        'archive':sha(upload/'p1-campaign.blob'),'protected':identity},indent=2))
    print(json.dumps({'output':str(out),'baseline':baseline,'manifest':sha(stage/'bundle_manifest.json')}),flush=True)

if __name__=='__main__':main()
