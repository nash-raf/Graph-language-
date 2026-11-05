#!/usr/bin/env python3
"""Freeze and build a named version on native WSL storage; never overwrite it.

Run from the repository with WSL python3 codex_doc/build_version.py baseline.
This builds compiler/runtime only. It does not run benchmarks.
"""
from pathlib import Path
import concurrent.futures
import json
import os
import shutil
import sys

REPO = Path(__file__).resolve().parents[1]
ROOT = Path('/home/kazi/p1_layout_native_20261105')
os.environ.update(P1_CAMPAIGN_ROOT=str(ROOT),
                  P1_CAMPAIGN_OUTPUT=str(ROOT / 'results'))
sys.path.insert(0, str(REPO / 'tools/p1_campaign'))
import runner as R

def main():
    version = sys.argv[1]
    assert version.replace('_', '').isalnum()
    cache = ROOT / 'object_cache'
    cache.mkdir(exist_ok=True)
    d, src, identity = R.freeze(version)
    logs = d / 'build_logs'
    logs.mkdir(exist_ok=True)
    flags = ['g++', '-O2', '-mavx2', '-march=native', '-std=c++17',
             '-fexceptions', '-pthread', '-I/usr/local/include/antlr4-runtime',
             '-Igenerated', '-iquote', '.', '-I' + R.cfg('--includedir')[0] + '/polly',
             *[x for x in R.cfg('--cxxflags') if x != '-fno-exceptions']]
    names = ['main.cpp', 'IRGenVisitor.cpp', 'MotifPattern.cpp', 'MotifIRBuilder.cpp',
             'ASTBuilder.cpp', 'pdg.cpp', 'parallel_loop_outline.cpp',
             'graph_frontier_lowering.cpp', 'SemanticAnalyzer.cpp',
             'roaring_bitmap.cpp', 'AutoTunerPass.cpp']
    names += [str(p.relative_to(src)) for p in (src / 'generated').glob('*.cpp')]
    # Read existing content-addressed objects, but write new objects only here.
    old_cache = Path('/home/kazi/p1_execution_simplification/object_cache')
    def compile_one(name, options):
        deps = R.local_deps(src / name, src)
        import subprocess
        compiler = subprocess.check_output([options[0], '--version'], text=True).splitlines()[0]
        key = R.key(dict(deps=deps, flags=options, compiler=compiler))
        old = old_cache / (key + '.o')
        print('BUILD', version, name, 'cached' if old.exists() else 'compile', flush=True)
        return R.compile_cache(src / name, src, options,
                               logs / (name.replace('/', '_') + '.log'),
                               seed=old if old.exists() else None)
    if not (d / 'GraphProgram').exists():
        with concurrent.futures.ThreadPoolExecutor(2) as pool:
            objects = list(pool.map(lambda name: compile_one(name, flags), names))
        R.checked(['g++', *objects, *R.cfg('--ldflags'), '-Wl,--no-keep-memory',
                   '-pthread', '-lantlr4-runtime', *R.cfg('--libs', 'all'),
                   *R.cfg('--system-libs'), '-o', d / 'GraphProgram'],
                  src, logs / 'compiler_link.log')
    for name in R.OBJECTS:
        cpp = name in ['roaring_bitmap', 'graph_loader_runtime', 'graph_runtime']
        options = ['g++', '-std=c++17', '-mavx2', '-march=native'] if cpp else ['gcc']
        options += ['-O3', '-g', '-fopenmp', '-pthread', '-iquote', '.']
        target = d / (name + '.o')
        if not target.exists():
            obj = compile_one(name + ('.cpp' if cpp else '.c'), options)
            shutil.copy2(obj, target)
    result = dict(version=version, source_hash=identity['source_hash'],
                  source_commit=identity['commit'], compiler_sha256=R.sha(d / 'GraphProgram'),
                  objects={name: R.sha(d / (name + '.o')) for name in R.OBJECTS},
                  native_flags='-O3 -g -fopenmp -pthread; C++ also -mavx2 -march=native',
                  tests_run=False, benchmarks_run=False)
    R.save(d / 'build.json', result)
    R.save(REPO / 'codex_doc' / (version + '_build.json'), result)
    print('BUILD COMPLETE', d, flush=True)

if __name__ == '__main__':
    main()
