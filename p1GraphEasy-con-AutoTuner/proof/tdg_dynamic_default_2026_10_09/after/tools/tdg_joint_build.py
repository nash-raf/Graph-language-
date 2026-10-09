#!/usr/bin/env python3
"""Build identical direct TDG workloads with one runtime per executable."""
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / os.environ.get('SGPL_JOINT_RESULTS_DIR', 'proof/tdg_joint_schedule')
BUILD = OUT / 'build'
SNAP = ROOT / 'proof/tdg_joint_schedule/ordinary_first_2026_10_08'

def run(args, **kwargs):
    return subprocess.run(list(map(str, args)), cwd=ROOT, check=True, **kwargs)

def build():
    BUILD.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((SNAP/'manifest.json').read_text())
    for name, digest in manifest['snapshot_sha256'].items():
        assert hashlib.sha256((SNAP/'source'/name).read_bytes()).hexdigest() == digest, name
    bitmap = BUILD/'bitmap.o'
    if not bitmap.exists():
        run(['g++','-O3','-mavx2','-pthread','-fopenmp','-c','roaring_bitmap.cpp','-o',bitmap])
    configurations = [
        ('reference', SNAP/'source/parallel_runtime.c', []),
        ('current', ROOT/'parallel_runtime.c', []),
        ('joint', ROOT/'parallel_runtime.c', ['-DSGPL_ENABLE_JOINT_SCHEDULER','-DJOINT']),
    ]
    if os.environ.get('SGPL_JOINT_COMPARE_PREVIOUS') == '1':
        configurations.append(('previous', ROOT/'proof/tdg_joint_schedule/phase_a_initial_2026_10_08/source/parallel_runtime.c',
                               ['-DSGPL_ENABLE_JOINT_SCHEDULER','-DJOINT']))
    for policy, source, flags in configurations:
        obj = BUILD/f'{policy}_probe.o'
        run(['gcc','-O3','-mavx2','-pthread','-fopenmp','-I',ROOT,'-DBASELINE',*flags,
             f'-DSGPL_RUNTIME_SOURCE="{source}"','-c','tools/tdg_joint_probe.c','-o',obj])
        run(['g++','-pthread','-fopenmp',obj,bitmap,'-lnlopt','-lm','-o',BUILD/f'probe_{policy}'])

if __name__ == '__main__':
    build()
