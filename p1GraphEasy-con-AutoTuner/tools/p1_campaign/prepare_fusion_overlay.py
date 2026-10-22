"""Immutable source overlay, including declared removals; packaging only."""
from pathlib import Path
import argparse,base64,hashlib,json,subprocess
HERE=Path(__file__).resolve().parent;REPO=HERE.parent.parent
def digest(data):return hashlib.sha256(data).hexdigest()
def main():
    p=argparse.ArgumentParser();p.add_argument('output');p.add_argument('--parent',required=True);p.add_argument('--attempt',required=True)
    p.add_argument('--stage',default='all');p.add_argument('--kernel-id');p.add_argument('--title');p.add_argument('--cases');a=p.parse_args()
    out=Path(a.output);out.mkdir(parents=True,exist_ok=False);parent=Path(a.parent);bundle=parent/'export/bundle'
    manifest=json.loads((bundle/'bundle_manifest.json').read_text());files={};deleted=[]
    # A rejected stage may be absent from the next candidate. Remove only
    # native/test files covered by the frozen candidate manifest, never a
    # baseline, graph, reference, toolchain input or arbitrary workspace path.
    native_suffixes=['.c','.cpp','.h','.inc','.ll']
    test_suffixes=native_suffixes+['.py','.sh','.graph','.edges','.json','.txt']
    for name in list(manifest):
        if not name.startswith('candidate/'):continue
        relative=Path(name).relative_to('candidate')
        eligible=(len(relative.parts)==1 and relative.suffix in native_suffixes) or (relative.parts[0]=='test' and relative.suffix in test_suffixes)
        if eligible and not (REPO/relative).is_file():deleted.append(name);del manifest[name]
    candidates=[f for f in REPO.iterdir() if f.is_file() and f.suffix in ['.c','.cpp','.h','.inc','.ll']]
    candidates += [f for f in (REPO/'test').rglob('*') if f.is_file() and f.suffix in ['.c','.cpp','.h','.py','.sh','.graph','.edges','.json','.txt']]
    candidates += [f for f in HERE.glob('*.py') if 'candidate/'+str(f.relative_to(REPO)) in manifest or f.name in ['fusion_diagnostics.py']]
    for f in candidates:
        name='candidate/'+str(f.relative_to(REPO));data=f.read_bytes()
        if f.suffix=='.sh':data=data.replace(b'\r\n',b'\n')
        if digest(data)!=manifest.get(name):files[name]=base64.b64encode(data).decode();manifest[name]=digest(data)
    if a.cases:
        name='candidate/tools/p1_campaign/diagnostic_cases.json';data=Path(a.cases).read_bytes()
        assert isinstance(json.loads(data),list)
        files[name]=base64.b64encode(data).decode();manifest[name]=digest(data)
    provenance=json.loads((bundle/'provenance.json').read_text())
    protected=['AutoTunerPass.cpp','AutoTunerPass.h','autotuner_runtime.c','autotuner_runtime.h',
        'parallel_runtime.c','parallel_runtime.h','launch_calibration_cache.inc','tdg_scheduler.inc','class_calib.c','hw_calib_bench.c']
    for name in protected:
        h=digest((bundle/'accepted'/name).read_text().encode())
        assert digest((REPO/name).read_text().encode())==h,('ARS changed',name)
        provenance['protected_ars'][name]=h
    win=subprocess.check_output(['wslpath','-w',str(REPO)],text=True).strip();git='/mnt/c/Program Files/Git/cmd/git.exe'
    provenance.update(attempt=a.attempt,candidate_commit=subprocess.check_output([git,'-C',win,'rev-parse','HEAD'],text=True).strip(),
        candidate_patch=subprocess.check_output([git,'-C',win,'diff','HEAD','--','.'],text=True),parent_manifest=digest((bundle/'bundle_manifest.json').read_bytes()))
    data=json.dumps(provenance,indent=2).encode();files['provenance.json']=base64.b64encode(data).decode();manifest['provenance.json']=digest(data)
    overlay={'parent_manifest':digest((bundle/'bundle_manifest.json').read_bytes()),'files':files,'deleted':sorted(deleted),'manifest':manifest}
    upload=out/'upload';upload.mkdir();payload=json.dumps(overlay,indent=2).encode();(upload/'fusion-overlay.json').write_bytes(payload)
    slug='p1-effect-fusion-'+a.attempt.lower()
    (upload/'dataset-metadata.json').write_text(json.dumps({'id':'kazisahib/'+slug,'title':'P1 fusion source '+a.attempt,
        'licenses':[{'name':'other'}],'description':'Private hash-pinned source overlay; original datasets and attempts retained.'},indent=2))
    text=(parent/'notebook/campaign.py').read_text()
    anchor=' # Use only distinct physical cores for dependency compilation as well.'
    code=''' # Apply an independently hash-pinned source overlay after verifying every parent file.
 overlays=list(Path('/kaggle/input').rglob('fusion-overlay.json'))
 assert len(overlays)==1,'Expected one immutable source overlay'
 payload=overlays[0].read_bytes()
 assert hashlib.sha256(payload).hexdigest()==OVERLAY_HASH,'Overlay checksum mismatch'
 overlay=json.loads(payload)
 assert overlay['parent_manifest']==EXPECTED_MANIFEST,'Wrong overlay parent'
 import base64
 for name in overlay.get('deleted',[]):
  target=bundle/name
  assert name.startswith('candidate/') and target.resolve().is_relative_to((bundle/'candidate').resolve()),'Invalid overlay removal'
  assert target.is_file() and name not in overlay['manifest'],'Removal must identify a frozen candidate file'
  target.unlink()
 for name,encoded in overlay['files'].items():
  target=bundle/name
  assert target.resolve().is_relative_to(bundle.resolve()),'Invalid overlay path'
  target.parent.mkdir(parents=True,exist_ok=True)
  data=base64.b64decode(encoded)
  assert hashlib.sha256(data).hexdigest()==overlay['manifest'][name]
  target.write_bytes(data)
 (bundle/'bundle_manifest.json').write_text(json.dumps(overlay['manifest'],indent=2))
'''.replace('OVERLAY_HASH',repr(digest(payload)))
    assert anchor in text;text=text.replace(anchor,code+anchor,1)
    if a.stage!='all':text=text.replace("'all']","'"+a.stage+"']",1)
    if a.stage=='contracts':
        start=text.index(" deps=SCRATCH/'dependencies'");end=text.index(" os.environ['P1_PORTABLE_WORK']",start)
        text=text[:start]+" (WORK/'dependency-identity.json').write_text(json.dumps({'llvm':subprocess.check_output(['llvm-config-20','--version'],text=True).strip(),'scope':'native contracts only; unused ANTLR and Graptor dependency builds omitted'},indent=2))\n"+text[end:]
    notebook=out/'notebook';notebook.mkdir();(notebook/'campaign.py').write_text(text)
    meta=json.loads((parent/'notebook/kernel-metadata.json').read_text())
    meta['dataset_sources'].append('kazisahib/'+slug)
    if a.kernel_id:meta['id']=a.kernel_id
    if a.title:meta['title']=a.title
    (notebook/'kernel-metadata.json').write_text(json.dumps(meta,indent=2))
    (out/'identity.json').write_text(json.dumps({'overlay':digest(payload),'parent':overlay['parent_manifest'],'attempt':a.attempt,'files':len(files),'deleted':deleted},indent=2))
    print(json.dumps({'output':str(out),'overlay':digest(payload),'files':len(files),'deleted':deleted}),flush=True)
if __name__=='__main__':main()
