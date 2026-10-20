"""Prepare a private Kaggle diagnostic notebook from a frozen bootstrap."""
from pathlib import Path
import argparse
import ast
import hashlib
import json

HERE = Path(__file__).resolve().parent


def constant(path, name):
    for node in ast.parse(path.read_text()).body:
        if isinstance(node, ast.Assign) and any(isinstance(x, ast.Name) and x.id == name for x in node.targets):
            return ast.literal_eval(node.value)
    raise ValueError(name)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('output')
    p.add_argument('--bootstrap', required=True)
    p.add_argument('--templates-root', required=True)
    args = p.parse_args()
    out = Path(args.output)
    out.mkdir(parents=True, exist_ok=False)
    text = Path(args.bootstrap).read_text()
    templates = Path(args.templates_root)
    payload = {
        'phases': constant(templates/'build_diagnostics.py', 'header'),
        'pool': constant(templates/'build_diagnostics.py', 'pool_header'),
        'work': constant(templates/'work_volume.py', 'header'),
    }
    # No Graptor build is needed in this component study.
    start = text.index(' # Match Graptor\'s original dependency commit.')
    stop = text.index(" os.environ['P1_PORTABLE_WORK']", start)
    text = text[:start] + text[stop:]
    start = text.index(' campaign=bundle/')
    stop = text.index(" print('Campaign completed", start)
    diagnostic = (HERE/'diagnose_static_regressions.py').read_text()
    profiles = (HERE/'profile_support.py').read_text()
    replacement = '\n'.join([
        " os.environ['P1_DIAGNOSTIC_BUNDLE']=str(bundle)",
        " diagnosis=SCRATCH/'diagnose_static_regressions.py'",
        ' diagnosis.write_text('+repr(diagnostic)+')',
        " profile_module=SCRATCH/'profile_support.py'",
        ' profile_module.write_text('+repr(profiles)+')',
        " os.environ['P1_DIAGNOSTIC_PROFILE_MODULE']=str(profile_module)",
        " support=SCRATCH/'p1-campaign/support';support.mkdir(parents=True,exist_ok=True)",
        " (support/'profile_templates.json').write_text("+repr(json.dumps(payload))+')',
        ' run([sys.executable,diagnosis])',
        '',
    ])
    text = text[:start]+replacement+text[stop:]
    text = text.replace("'unzip'])", "'unzip','strace','linux-tools-generic'])")
    text = text.replace("for name in ['comparison.html','comparison.json']:",
                        "for name in ['component_report.html','component_summary.json']:")
    # Preserve source and machine-code evidence, including reusable linked
    # executables. Object symlinks are materialized in this separate archive.
    needle = "  for name in ['component_report.html','component_summary.json']:"
    assert text.count(needle) == 1
    extra = '''  if (root/'versions').exists():
   with tarfile.open(WORK/'diagnostic-builds.tar.gz','w:gz',compresslevel=3,dereference=True) as t:
    for v in (root/'versions').iterdir():
     for p in v.iterdir():
      if p.name in ['src','cases'] or p.suffix in ['.o','.json','.asm']:
       t.add(p,arcname=str(p.relative_to(SCRATCH)))
'''
    text = text.replace(needle, extra+needle, 1)
    compile(text, str(out/'diagnosis.py'), 'exec')
    (out/'diagnosis.py').write_text(text)
    metadata = dict(id='kazisahib/p1-static-schedule-regression-diagnosis',
                    title='P1 static schedule regression diagnosis', code_file='diagnosis.py',
                    language='python', kernel_type='script', is_private=True,
                    enable_gpu=False, enable_internet=True, competition_sources=[],
                    dataset_sources=['kazisahib/p1-static-dag-cpu-campaign-20261020'], kernel_sources=[])
    (out/'kernel-metadata.json').write_text(json.dumps(metadata, indent=2))
    identity = dict(script_sha256=hashlib.sha256(text.encode()).hexdigest(),
                    diagnostic_sha256=hashlib.sha256(diagnostic.encode()).hexdigest(),
                    profile_sha256=hashlib.sha256(profiles.encode()).hexdigest(),
                    input_bundle='existing frozen dataset version 2; exact pinned manifest verified in notebook',
                    private=True, cpu_only=True)
    (out/'diagnostic-identity.json').write_text(json.dumps(identity, indent=2))
    print(json.dumps(identity))


if __name__ == '__main__':
    main()
