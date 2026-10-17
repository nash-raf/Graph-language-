#!/usr/bin/env python3
"""Write reproducibility notes and independently validate final observations."""
import csv
import hashlib
import json
from pathlib import Path
import statistics

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'bench_results/layout_switch_20261015'
MODES=['AUTO','CSR','PCSR','BCSR','SET']

def main():
    selected=json.loads((OUT/'selected_cases.json').read_text())
    raw=list(csv.DictReader((OUT/'validation_raw_runs.csv').open()))
    assert len(selected)==5 and len(raw)==175
    assert len({(r['case'],r['variant'],r['run']) for r in raw})==175
    lines=[
        '# SGPL AutoTuner layout-switch wins', '',
        'Five synthetic BFS and edge-churn workloads were compared against four fixed graph layouts.',
        'The same SGPL source, compiler, optimization settings, graph input and runtime objects are used for all five variants of each case.', '',
        '## Results', '',
        'Times below are median total executable wall times in milliseconds. Speedup is the fastest fixed-layout median divided by the AutoTuner median.', '',
        '| Case | AUTO ms | CSR ms | PCSR ms | BCSR ms | SET ms | Best fixed | Speedup |',
        '| --- | ---: | ---: | ---: | ---: | ---: | --- | ---: |',
    ]
    stats=[]
    for index,name in enumerate(selected,1):
        r=json.loads((OUT/'binaries'/name/'result.json').read_text())
        a=json.loads((OUT/'binaries'/name/'audit.json').read_text())
        med=r['medians_ms']
        best=min(MODES[1:],key=lambda m:med[m])
        assert med[best]>med['AUTO']
        for mode in MODES:
            samples=[float(row['total_runtime_ms']) for row in raw if row['case']==name and row['variant']==mode]
            assert len(samples)==7
            assert abs(statistics.median(samples)-med[mode])<1e-9
            assert a[mode]['correct']
            assert a[mode]['mutation_effects_checked']
            assert all(row['stdout_sha256']==hashlib.sha256(r['expected_stdout'].encode()).hexdigest()
                       for row in raw if row['case']==name and row['variant']==mode)
            q=statistics.quantiles(samples,n=4,method='inclusive')
            stats.append(dict(case=name,variant=mode,median_ms=med[mode],q1_ms=q[0],q3_ms=q[2],
                              min_ms=min(samples),max_ms=max(samples),runs=7))
        lines.append(f'| {index} | '+ ' | '.join(f'{med[m]:.3f}' for m in MODES)+
                     f' | {best} | {med[best]/med["AUTO"]:.3f}x |')
    lines += ['',
        'The machine-readable five-case comparison is [layout_switch_wins.csv](layout_switch_wins.csv).',
        'All 175 timed observations are retained in [validation_raw_runs.csv](validation_raw_runs.csv).',
        'Per-variant spread is retained in [runtime_spread.csv](runtime_spread.csv).', '',
        '## Saved winning SGPL programs', '']
    for i,name in enumerate(selected,1):
        r=json.loads((OUT/'binaries'/name/'result.json').read_text())
        c=r['config']
        lines.append(f'{i}. [{name}.sgpl](winning_codes/{name}.sgpl): {c["n"]} vertices, {c["n"]*c["degree"]} undirected edges, {c["scans"]} BFS traversals before churn and {c["tail_scans"]} afterward. Each of {c["churn"]} churn batches adds and removes {c["inserts"]} absent edges.')
    lines += ['',
        'Every winning program is saved unchanged from its measured source. Its input edge list is in `graphs/`.',
        'Executable variants, generated IR, compiler logs, timing results and independent audit logs are in `binaries/<case>/`.', '',
        'To run a saved executable directly, use `codes/` or `winning_codes/` as the working directory so its relative graph-input path resolves.', '',
        '## Timing protocol', '',
        '- Seven timed launches per variant after one warm-up, with variant order randomized within every repetition using seed 20261015. Benchmark jobs do not overlap.',
        '- Wall time spans subprocess launch through process exit. It includes graph loading, allocation, all graph conversions, traversal, updates, profiling and output. Compilation and diagnostic audits are excluded.',
        '- Executables and input files are staged on the WSL local filesystem to avoid Windows mount metadata overhead. All timed launches use the same warm graph-loader sidecar cache.',
        '- Ubuntu WSL2 on an AMD Ryzen 5 5500. SGPL, OpenMP and Roaring thread counts are all one.',
        '- AUTO uses the existing AutoTuner cost model and generated schedule. `AUTOTUNER_FORCE_LAYOUT` is absent. There are no manually inserted layout changes in the SGPL programs.',
        '- Fixed variants compile the identical source with `AUTOTUNER_FORCE_LAYOUT=CSR`, `PCSR`, `BCSR` or `SET`.',
        '- The graph loader initially constructs CSR. A requested fixed layout is installed before the first workload operation and kept through the final operation. This initial conversion is included in its runtime.',
        '- The AutoTuner makes its decisions at compile time; the emitted program executes the selected layout transitions at runtime.', '',
        '## Correctness and layout audits', '',
        'Separate diagnostic executables wrap every graph mutation, neighbor-iterator initialization and BFS call. These wrappers are absent from timed executables.',
        'For every BFS, the audit checks the visited vertices, exact arc count and an order-independent 64-bit digest of all arcs against a Python reference generated from the graph and updates.',
        'It checks that each inserted edge becomes present in both directions and each removed edge becomes absent. It also checks the executed mutation count, asserts the requested fixed layout at every graph operation, and records actual AUTO layout transitions.',
        'All 25 case/variant audits passed. The graph edge count and standard-output hash were also checked on every timed run.', '',
        '## Reproduce', '',
        'Run from the repository root in Ubuntu/Linux with the existing GraphProgram compiler and GCC/G++ runtime dependencies:', '',
        '```bash', 'python3 tools/run_layout_switch_suite.py', 'python3 tools/summarize_layout_switch.py', '```', '',
        'The CSV report builder is `report/build_report.mjs`; it authors and validates the requested CSV through the bundled spreadsheet runtime.',
        'The exact source/compiler hashes, calibration and environment are recorded in [manifest.json](manifest.json).', '',
        'These cases were selected in a search for wins. They are synthetic workload examples, not a claim that AutoTuner wins on arbitrary SGPL programs.',
        'Earlier pilot timings and discarded candidates remain separate in `raw_runs.csv` and `codes/`; final conclusions use only the seven-run confirmation data.', '',
    ]
    (OUT/'README.md').write_text('\n'.join(lines))
    with (OUT/'runtime_spread.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(stats[0]));writer.writeheader();writer.writerows(stats)
    report=list(csv.DictReader((OUT/'layout_switch_wins.csv').open()))
    assert len(report)==5 and {r['case'] for r in report}==set(selected)
    manifest=json.loads((OUT/'manifest.json').read_text())
    manifest['benchmark_date']='2026-10-15'
    manifest['client_timezone']='Asia/Dhaka'
    manifest['workload_sha256']={}
    manifest['timed_binary_sha256']={}
    for row in report:
        name=row['case']
        measured=json.loads((OUT/'binaries'/name/'result.json').read_text())
        code=OUT/row['sgpl_file']
        original=OUT/'codes'/(name+'.sgpl')
        assert code.read_bytes()==original.read_bytes()
        manifest['workload_sha256'][name]=hashlib.sha256(code.read_bytes()).hexdigest()
        for mode in MODES:
            assert abs(float(row[mode+'_total_ms'])-measured['medians_ms'][mode])<1e-9
            binary=OUT/'binaries'/name/mode
            staged=Path('/tmp/sgpl_layout_switch_20261015')/name/'binaries'/name/mode
            digest=hashlib.sha256(binary.read_bytes()).hexdigest()
            if staged.exists():
                assert hashlib.sha256(staged.read_bytes()).hexdigest()==digest
            manifest['timed_binary_sha256'][f'{name}/{mode}']=digest
    configs=manifest['configs']
    manifest['graph_sha256']={f'ring_n{c["n"]}_d{c["degree"]}.txt':
        hashlib.sha256((OUT/'graphs'/f'ring_n{c["n"]}_d{c["degree"]}.txt').read_bytes()).hexdigest() for c in configs}
    paths=['tools/layout_switch_benchmark.py','tools/layout_switch_audit.c',
           'tools/run_layout_switch_suite.py','tools/audit_layout_switch_suite.py','tools/summarize_layout_switch.py',
           'bench_results/layout_switch_20261015/report/build_report.mjs']
    manifest['final_verification_scripts_sha256']={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    manifest['checks']=dict(winning_cases=5,audited_variants=25,timed_observations=175,
                           traversal_outputs_verified=True,mutation_effects_verified=True,
                           fixed_layouts_verified=True,saved_source_matches_measured_source=True,
                           saved_binaries_used_as_staging_sources=True,csv_matches_measurements=True)
    manifest['staging_method']='shutil.copy2 copies each retained variant executable to WSL local storage before warm-up and timing; temporary copies may be removed when WSL restarts.'
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print('Verified five winners, 25 audited variants and 175 unique timed observations.')

if __name__=='__main__': main()
