"""Freeze a fresh private full-matrix repair campaign without changing old data."""
from pathlib import Path
import argparse, hashlib, json, shutil, subprocess, sys, tarfile
HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser();p.add_argument('output');p.add_argument('--previous-bundle',required=True);a=p.parse_args()
    out=Path(a.output);old=Path(a.previous_bundle)
    subprocess.run([sys.executable,HERE/'export_kaggle_campaign.py',out/'export'],check=True)
    stage=out/'export/bundle'
    previous=json.loads((old/'bundle_manifest.json').read_text())
    for name,digest in previous.items():
        if name.startswith('candidate/'):assert sha(old/name)==digest,name
    shutil.copytree(old/'candidate',stage/'unfixed')
    shutil.copy2(HERE/'cursor_repair_campaign.py',stage/'candidate/tools/p1_campaign/cursor_repair_campaign.py')
    provenance=json.loads((stage/'provenance.json').read_text())
    provenance.update(unfixed_provenance=json.loads((old/'provenance.json').read_text()),
                      unfixed_manifest_sha256=sha(old/'bundle_manifest.json'),
                      purpose='Full 244-case production cursor alignment repair; retain all static schedule code',
                      contrasts=['pre-static accepted -> repaired','unfixed static -> repaired','same binary cursor alignment disabled'])
    (stage/'provenance.json').write_text(json.dumps(provenance,indent=2))
    manifest={str(f.relative_to(stage)):sha(f) for f in sorted(stage.rglob('*')) if f.is_file() and f.name!='bundle_manifest.json'}
    (stage/'bundle_manifest.json').write_text(json.dumps(manifest,indent=2))
    upload=out/'upload';upload.mkdir()
    with tarfile.open(upload/'p1-campaign.blob','w:gz',compresslevel=3) as t:t.add(stage,arcname='p1-bundle')
    (upload/'archive-identity.json').write_text(json.dumps({'sha256':sha(upload/'p1-campaign.blob'),'manifest_sha256':sha(stage/'bundle_manifest.json')},indent=2))
    (upload/'dataset-metadata.json').write_text(json.dumps(dict(id='kazisahib/p1-cursor-repair-cpu-20261020',title='P1 cursor repair CPU 20261020',licenses=[{'name':'other'}],description='Private frozen source and exact 61-graph inputs for accepted, unfixed, repaired P1 and pinned Graptor. No credentials.'),indent=2))
    notebook=out/'notebook';notebook.mkdir()
    text=(HERE/'kaggle_bootstrap.py').read_text().replace('__BUNDLE_MANIFEST_SHA256__',sha(stage/'bundle_manifest.json'))
    text=text.replace("campaign=bundle/'candidate/tools/p1_campaign/portable_campaign.py'","campaign=bundle/'candidate/tools/p1_campaign/cursor_repair_campaign.py'")
    text=text.replace('import portable_campaign as P','import cursor_repair_campaign as P')
    text=text.replace("['comparison.html','comparison.json']", "['comparison.html','comparison.json','cursor_repair_report.html','cursor_repair_summary.json']")
    compile(text,'campaign.py','exec');(notebook/'campaign.py').write_text(text)
    (notebook/'kernel-metadata.json').write_text(json.dumps(dict(id='kazisahib/p1-cursor-repair-cpu-campaign',title='P1 cursor repair CPU campaign',code_file='campaign.py',language='python',kernel_type='script',is_private=True,enable_gpu=False,enable_internet=True,competition_sources=[],dataset_sources=['kazisahib/p1-cursor-repair-cpu-20261020'],kernel_sources=[]),indent=2))
    identity=dict(archive_sha256=sha(upload/'p1-campaign.blob'),manifest_sha256=sha(stage/'bundle_manifest.json'),script_sha256=sha(notebook/'campaign.py'),files=len(manifest),cases=244,measurements=44225,candidate_retained=True)
    (out/'identity.json').write_text(json.dumps(identity,indent=2));print(json.dumps(identity),flush=True)
if __name__=='__main__':main()
