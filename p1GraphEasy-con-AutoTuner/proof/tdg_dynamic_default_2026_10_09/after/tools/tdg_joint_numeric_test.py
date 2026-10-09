#!/usr/bin/env python3
"""Exercise outlined-site registration and all compiler policy entry points."""
import os
import re
import shutil
import subprocess
from tdg_joint_build import ROOT, BUILD, SNAP, run


def main():
    out = BUILD / 'numeric'
    out.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in ('runtime.c', 'autotuner_runtime.c', 'graph_mutation_runtime.c',
                   'gpu_runtime.c', 'semiring_runtime.c', 'roaring_bitmap.cpp',
                   'graph_loader_runtime.cpp', 'graph_runtime.cpp'):
        obj = out / (source + '.o')
        compiler = 'g++' if source.endswith('.cpp') else 'gcc'
        run([compiler, '-O2', '-mavx2', '-pthread', '-fopenmp', '-I.', '-c', source, '-o', obj])
        objects.append(obj)
    normalized = []
    expected_left = (65535 * 17 + 3) % 1009
    expected_right = (65535 * 31 + 5) % 1013
    for mul, add, mod in ((19, 7, 1013), (23, 11, 1019), (29, 13, 1021)):
        expected_left = (expected_left * mul + add) % mod
    for mul, add, mod in ((37, 17, 1019), (41, 19, 1021), (43, 23, 1031)):
        expected_right = (expected_right * mul + add) % mod
    expected = f'{expected_left}\n{expected_right}\n'
    for policy, source, flags in (
        ('default', ROOT / 'parallel_runtime.c', []),
        ('dynamic', ROOT / 'parallel_runtime.c', []),
        ('current', ROOT / 'parallel_runtime.c', []),
        ('joint-experimental', ROOT / 'parallel_runtime.c', ['-DSGPL_ENABLE_JOINT_SCHEDULER']),
        ('ordinary-first-reference', SNAP / 'source/parallel_runtime.c',
         ['-Dsgpl_run_tdg_level=sgpl_run_tdg_level_ordinary_first_reference']),
    ):
        ir = out / (policy + '.ll')
        env = dict(os.environ, GRAPH_DISABLE_POLLY='1', SGPL_TDG_DEBUG='1')
        result = run([ROOT / 'build_tdg_validation/GraphProgram_tdg', '--ir-backend=cpu',
                      *([] if policy=='default' else [f'--tdg-scheduler={"dynamic" if policy=="ordinary-first-reference" else policy}']), f'--emit-ir-to={ir}',
                      'test/tdg_joint_numeric.graph'], env=env, capture_output=True, text=True)
        (out / (policy + '.compile.txt')).write_text(result.stdout + result.stderr)
        code = ir.read_text()
        assert code.count('private unnamed_addr constant [1 x i32]') >= 2
        assert code.count('call i32 @sgpl_should_parallelize_doall') == 2
        assert len(re.findall(r'store ptr @sgpl.tdg.loop_ids.', code)) == 2
        code = re.sub(r'^declare void @sgpl_run_tdg_level(?:_joint|_ordinary_first_reference)?\(.*\)\n',
                      '', code, flags=re.MULTILINE)
        code = re.sub(r'@sgpl_run_tdg_level(?:_joint|_ordinary_first_reference)?\(',
                      '@sgpl_run_tdg_level(', code)
        code = re.sub(r'\n{3,}', '\n\n', code)
        normalized.append(code)
        obj = out / (policy + '.o')
        shutil.copyfile(ROOT / 'program.o', obj)
        repeated = out / (policy + '.repeat.o')
        run(['objcopy', '--redefine-sym', 'main=graph_main', obj, repeated])
        if policy=='ordinary-first-reference':
            run(['objcopy','--redefine-sym',
                 'sgpl_run_tdg_level=sgpl_run_tdg_level_ordinary_first_reference',repeated])
        driver = out / 'repeat.cpp'
        driver.write_text('extern "C" int graph_main(void);\n'
                          'int main() { for(int i=0;i<14;++i) graph_main(); return 0; }\n')
        runtime = out / (policy + '.runtime.o')
        run(['gcc', '-O2', '-pthread', '-iquote', ROOT, *flags, '-c', source, '-o', runtime])
        extra = []
        if policy == 'ordinary-first-reference':
            alias = out / 'alias.o'
            run(['gcc', '-O2', '-c', 'tools/tdg_ordinary_first_reference_alias.c', '-o', alias])
            extra.append(alias)
        exe = out / (policy + '.bin')
        run(['g++', '-O2', '-no-pie', '-fopenmp', repeated, driver, runtime, *objects,
             *extra, '-ldl', '-lnlopt', '-o', exe])
        for budget in (1, 2, 4, 8):
            result = run([exe], env=dict(env, SGPL_NUM_THREADS=str(budget)),
                         capture_output=True, text=True, timeout=60)
            assert result.stdout == expected * 14, (policy, budget, result.stdout)
            (out / f'{policy}.B{budget}.trace.txt').write_text(result.stderr)
        print(f'PASS numeric generated output/site registration: {policy}', flush=True)
    assert all(code==normalized[0] for code in normalized), 'Workload IR differs between policies'
    print('PASS emitted workload IR identical after policy-entry normalization', flush=True)
    invalid = subprocess.run([str(ROOT / 'build_tdg_validation/GraphProgram_tdg'),
        '--tdg-scheduler=invalid', 'test/tdg_joint_numeric.graph'], cwd=ROOT,
        capture_output=True, text=True)
    assert invalid.returncode != 0 and 'Unknown --tdg-scheduler value' in invalid.stderr
    removed = subprocess.run([str(ROOT / 'build_tdg_validation/GraphProgram_tdg'),
        '--tdg-scheduler=ordinary-first-reference','test/tdg_joint_numeric.graph'],cwd=ROOT,
        capture_output=True,text=True)
    assert removed.returncode != 0 and 'Unknown --tdg-scheduler value' in removed.stderr
    # Both dynamic aliases link the normal runtime without opt-in build flags.
    # The frozen reference remains unavailable in production.
    for policy in ('ordinary-first-reference',):
        mismatch = subprocess.run(list(map(str, ['g++', '-O2', '-no-pie', '-fopenmp',
            out/(policy+'.repeat.o'), driver, out/'current.runtime.o', *objects,
            '-ldl', '-lnlopt', '-o', out/'mismatch.bin'])), cwd=ROOT,
            capture_output=True, text=True)
        assert mismatch.returncode != 0 and 'undefined reference' in mismatch.stderr
    print('PASS invalid option and mismatched runtime are rejected', flush=True)


if __name__ == '__main__':
    main()
