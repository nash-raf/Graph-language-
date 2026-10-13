#!/usr/bin/env python3
"""Reproducible mixed SGPL levels; shared allocation/order oracle; append exports."""
import argparse
from collections import Counter, defaultdict
import csv
import itertools
import json
import os
from pathlib import Path
import random
import resource
import shutil
import statistics as st
import time
import run as sweep

HERE,ROOT=sweep.HERE,sweep.ROOT
CASES=[
    dict(name='mixed_one_ordinary',ordinary=[512],n=[8192,8192],depth=[8,8],budgets=[2,4,6],purpose='one short ordinary task'),
    dict(name='mixed_two_ordinary',ordinary=[512,512],n=[8192,8192],depth=[8,8],budgets=[2,4,6],purpose='two short tasks; B4 exactly fits parent callbacks'),
    dict(name='mixed_short_ordinary',ordinary=[64]*3,n=[8192,8192],depth=[8,8],budgets=[2,4,6],purpose='three very short tasks; ordinary-first overload at B2/B4'),
    dict(name='mixed_medium_ordinary',ordinary=[4096]*3,n=[8192,8192],depth=[8,8],budgets=[4,6],purpose='ordinary task duration nearer the loop duration'),
    dict(name='mixed_long_ordinary',ordinary=[65536]*3,n=[8192,8192],depth=[8,8],budgets=[4],purpose='ordinary tasks longer than loops; challenge ordinary-first assumption'),
    dict(name='mixed_one_long_ordinary',ordinary=[64,64,65536],n=[8192,8192],depth=[8,8],budgets=[4],purpose='one long serial task among two short ones'),
    dict(name='mixed_compute',ordinary=[512]*3,n=[32768,32768],depth=[16,16],budgets=[4,6],purpose='substantial balanced loops and short ordinary tasks'),
    dict(name='mixed_loop_skew',ordinary=[512]*3,n=[32768,2048],depth=[8,8],budgets=[4,6],purpose='16:1 loop size skew with ordinary siblings'),
]

def orders(case):
    count=len(case['ordinary'])+2
    groups=defaultdict(list)
    for j,r in enumerate(case['ordinary'],2): groups[r].append(j)
    # Identical ordinary callbacks have the same code, rounds and seed. Quotient
    # only label permutations within that class, not distinct execution orders.
    return [list(p) for p in itertools.permutations(range(count))
            if all([j for j in p if j in g]==g for g in groups.values())]

def configs(case,budget,include_fifo=True):
    total=len(case['ordinary'])+2
    extra=lambda w:w if w>=2 else 0
    phased=total>budget
    free=max(0,budget-(2 if phased else total))
    h=max(min(len(case['ordinary']),budget),min(2,budget)) if phased else total
    current=[(0,a,b,0,h) for a in range(1,max(1,free)+1) for b in range(1,max(1,free)+1)
             if extra(a)+extra(b)<=free]
    # Every legal loop width, parent-worker limit, priority ordering and backfill
    # option of the existing joint executor, including widths whose teams cannot
    # coexist. The executor performs admission checks for each callback.
    joint=[(1,a,b,s*2+backfill,parents)
           for a,b,s,backfill,parents in itertools.product(
               range(1,budget),range(1,budget),range(len(case['orders'])),range(2),range(1,min(total,budget)+1))]
    fifo_free=max(0,budget-total)
    fifo=[(2,a,b,0,min(total,budget)) for a in range(1,max(1,fifo_free)+1) for b in range(1,max(1,fifo_free)+1)
          if extra(a)+extra(b)<=fifo_free]
    return current+(fifo if include_fifo else [])+joint

def header(case):
    return (f'{len(case["ordinary"])} {len(case["orders"])}\n'+
            ' '.join(map(str,case['ordinary']))+'\n'+
            ''.join(' '.join(map(str,p))+'\n' for p in case['orders']))

def generate(cases):
    for c in cases:
        lines=[f'// {c["purpose"]}. Test adapter creates one mixed TDG level.',
               '// Ordinary callback rounds: '+','.join(map(str,c['ordinary']))+'. Each callback gets one worker.',
               'fn int sgpl_ordinary_work(int rounds, int seed) {',
               '  int k = 0; int value = seed;',
               '  while (k < rounds) { value = (value * 17 + 11) % 1009; k = k + 1; }',
               '  return value;', '}',
               f'int nl = {c["n"][0]};',f'int nr = {c["n"][1]};',
               'int left[nl];','int right[nr];','int i = 0;','int j = 0;']
        for side,var,j in [('left','i',0),('right','j',1)]:
            lines += [f'while ({var} < {"nr" if j else "nl"}) {{',
                      f'  {side}[{var}] = ({var} * {31 if j else 17} + {5 if j else 3}) % {1013 if j else 1009};']
            for k in range(1,c['depth'][j]):
                lines += [f'  {side}[{var}] = ({side}[{var}] * {19+2*k} + {7+k}) % {1009+2*k};']
            lines += [f'  {var} = {var} + 1;', '}']
        lines += [f'print left[{c["n"][0]-1}];',f'print right[{c["n"][1]-1}];']
        (HERE/'cases'/(c['name']+'.graph')).write_text('\n'.join(lines)+'\n')
    (HERE/'cases/mixed_manifest.json').write_text(json.dumps(cases,indent=2)+'\n')

def measure(cases,args,out):
    physical=0; (out/'traces').mkdir(exist_ok=True)
    with (out/'raw.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=sweep.FIELDS); writer.writeheader()
        for case in cases:
            for budget in case['budgets']:
                all_configs=configs(case,budget)
                for rep in range(args.repeats):
                    policies=['ordinary-first','dynamic']
                    random.Random(f'{args.seed}-{case["name"]}-{budget}-{rep}').shuffle(policies)
                    for policy in policies:
                        # One shared oracle per fresh-process repeat, after the
                        # dynamic native decision. Never train from oracle data.
                        shuffled=list(all_configs) if policy=='dynamic' else []
                        random.Random(f'{args.seed}-{case["name"]}-{budget}-{rep}-oracle').shuffle(shuffled)
                        stdin=header(case)+''.join(' '.join(map(str,c))+'\n' for c in shuffled)
                        env={k:v for k,v in os.environ.items() if not k.startswith(
                            ('SGPL_FORCE','SGPL_JOINT','SGPL_TDG','SGPL_BUDGET','GRAPH_PARALLEL'))}
                        env.update(SGPL_NUM_THREADS=str(budget),OMP_NUM_THREADS=str(budget),SWEEP_SAMPLES=str(args.samples))
                        begin=time.monotonic()
                        result=sweep.run([out/'build'/case['name'],int(policy=='dynamic'),-2 if shuffled else -1,
                                          1,1,0,1,*case['n'],case['depth'][0]*100+case['depth'][1]],
                                         env=env,input=stdin,capture_output=True,text=True,timeout=args.timeout)
                        (out/'traces'/f'{case["name"]}.B{budget}.{policy}.r{rep}.txt').write_text(result.stderr)
                        parsed=list(csv.reader(result.stdout.splitlines()))
                        assert len(parsed)==args.samples*(1+len(shuffled))
                        physical+=len(parsed)
                        for row in parsed:
                            phase,it,rl,rr,al,ar,backend,order,h,ns,pred=map(int,row)
                            config=None if phase==1 else shuffled[phase-100]
                            if config: assert (backend,al,ar,order,h)==config,(config,row)
                            item=dict(case=case['name'],budget=budget,policy=policy,repeat=rep,
                                      kind='model' if config is None else 'forced',
                                      config='' if config is None else ':'.join(map(str,config)),backend=backend,
                                      order=order,parents=h,requested_left=rl,requested_right=rr,
                                      actual_left=al,actual_right=ar,time_ns=ns,prediction_ns=pred)
                            writer.writerow(item)
                            if config:
                                # Same measured oracle referenced by both policy
                                # comparisons. These are NOT independent reruns.
                                writer.writerow(dict(item,policy='ordinary-first'))
                        f.flush()
                        print(f'{case["name"]} B{budget} {policy} r{rep+1}: {len(shuffled)} schedules, '
                              f'{len(parsed)} physical samples, {time.monotonic()-begin:.1f}s',flush=True)
    return physical

def check(out):
    meta=json.loads((out/'mixed_manifest.json').read_text())
    with (out/'raw.csv').open() as f: rows=list(csv.DictReader(f))
    names={c['name'] for c in meta['cases']}; grouped=defaultdict(list)
    for r in rows:
        if r['case'] in names: grouped[r['case'],int(r['budget']),r['policy'],int(r['repeat'])].append(r)
    counts=Counter(); forced_checks=0
    for case in meta['cases']:
        for budget in case['budgets']:
            expected={':'.join(map(str,c)) for c in configs(case,budget,meta.get('oracle_version',1)>=2)}
            for rep in range(meta['arguments']['repeats']):
                shared=[]
                for policy in ('ordinary-first','dynamic'):
                    block=grouped[case['name'],budget,policy,rep]
                    forced_rows=[r for r in block if r['kind']=='forced']
                    assert len(block)==meta['arguments']['samples']*(len(expected)+1)
                    assert Counter(r['config'] for r in forced_rows)==Counter({c:meta['arguments']['samples'] for c in expected})
                    for r in forced_rows:
                        assert tuple(map(int,r['config'].split(':')))==tuple(int(r[k]) for k in
                            ('backend','actual_left','actual_right','order','parents'))
                    native=[r for r in block if r['kind']=='model']
                    assert all(':'.join(r[k] for k in ('backend','actual_left','actual_right','order','parents')) in expected for r in native)
                    shared.append([(r['config'],r['time_ns']) for r in forced_rows])
                    trace=out/'traces'/f'{case["name"]}.B{budget}.{policy}.r{rep}.txt'
                    line=next(s for s in trace.read_text().splitlines() if s.startswith('MIX_VALIDATION,'))
                    _,calls,phased,ordinary,total=line.split(',')
                    assert int(ordinary)==len(case['ordinary']) and int(total)==case['total_tasks']
                    if case['total_tasks']>budget and policy=='ordinary-first': assert int(phased)>0
                    counts['checked executions']+=int(calls); counts['verified ordinary-first phased executions']+=int(phased)
                    forced_checks+=len(forced_rows)
                assert shared[0]==shared[1], 'Policies must reference exactly the same measured oracle'
    for name,digest in meta['protected_sha256'].items(): sweep.verify_recorded_source(name,digest)
    for record in meta['generated']:
        artifact_dir=Path(record.get('build_directory',meta['build_directory']))
        assert sweep.sha(artifact_dir/(record['case']+'.o'))==record['object_sha256']
        ir=artifact_dir/(record['case']+'.ll')
        assert sweep.sha(ir)==record['ir_sha256']
        sweep.ordinary_serial_ir(ir.read_text())
        assert sweep.sha(HERE/'cases'/(record['case']+'.graph'))==record['source_sha256']
    result='PASS: complete mixed allocation/order/parent/backfill coverage; shared oracle; native choices covered.\n'
    result+='PASS: ordinary callbacks once each, serial results, loop outputs/grants, balanced budget, actual ordinary-first phase ordering.\n'
    result+='PASS: SGPL object/source and protected production hashes.\n'+json.dumps(counts,indent=2)+'\n'
    original=out/'two_loop_snapshot/raw.csv'
    if original.exists():
        data=original.read_bytes()
        with (out/'raw.csv').open('rb') as f: assert f.read(len(data))==data
        result+='PASS: original two-loop raw CSV preserved byte for byte as the prefix of the combined CSV.\n'
    (out/'mixed_validation.txt').write_text(result); print(result,flush=True)

def merge(out,target):
    assert out.resolve()!=target.resolve()
    meta=json.loads((out/'mixed_manifest.json').read_text())
    names={c['name'] for c in meta['cases']}
    old_meta_path=target/'mixed_manifest.json'
    old_meta=json.loads(old_meta_path.read_text()) if old_meta_path.exists() else None
    if old_meta:
        retained_cases=[c for c in old_meta['cases'] if c['name'] not in names]
        if retained_cases:
            assert all(meta['arguments'][k]==old_meta['arguments'][k] for k in ('repeats','samples')), \
                'Use --no-append or rerun all mixed cases when changing repeat/sample counts'
            assert meta['protected_sha256']==old_meta['protected_sha256'], 'Retained results use different production sources'
        for case in retained_cases: case.setdefault('build_directory',old_meta['build_directory'])
        for case in meta['cases']: case.setdefault('build_directory',meta['build_directory'])
        retained_records=[r for r in old_meta['generated'] if r['case'] not in names]
        for record in retained_records: record.setdefault('build_directory',old_meta['build_directory'])
        for record in meta['generated']: record.setdefault('build_directory',meta['build_directory'])
        meta['cases']=retained_cases+meta['cases']; meta['generated']=retained_records+meta['generated']
    snapshot=target/'two_loop_snapshot'; snapshot.mkdir(exist_ok=True)
    for name in ['raw.csv','allocations.csv','allocation_columns.csv','schedules.csv','summary.csv',
                 'policy_comparison.csv','analysis.json','findings.txt','report.txt','manifest.json','tables.html','export_validation.txt']:
        original=target/name
        if original.exists() and not (snapshot/name).exists(): shutil.copy2(original,snapshot/name)
    with (target/'raw.csv').open() as f: old=list(csv.DictReader(f))
    with (out/'raw.csv').open() as f: new=list(csv.DictReader(f))
    retained=[r for r in old if r['case'] not in names]
    if old_meta:
        old_names={c['name'] for c in old_meta['cases']}
        retained_models=sum(r['case'] in old_names and r['kind']=='model' for r in retained)
        retained_forced=sum(r['case'] in old_names and r['kind']=='forced' for r in retained)
        assert retained_forced%2==0
        meta['physical_timed_samples']+=retained_models+retained_forced//2
    temp=target/'raw.csv.pending'
    with temp.open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=sweep.FIELDS); writer.writeheader(); writer.writerows(retained); writer.writerows(new)
    temp.replace(target/'raw.csv')
    (target/'mixed_manifest.json').write_text(json.dumps(meta,indent=2)+'\n')
    (target/'traces').mkdir(exist_ok=True)
    for path in (out/'traces').glob('*.txt'): shutil.copy2(path,target/'traces'/path.name)
    # Keep the original two-loop manifest and all its builds/data intact.
    before=[r for r in old if not r['case'].startswith('mixed_')]
    with (target/'raw.csv').open() as f: after=[r for r in csv.DictReader(f) if not r['case'].startswith('mixed_')]
    assert before==after
    del old,new,retained
    sweep.analyze(target); check(target)
    from mixed_report import report
    report(target)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out',type=Path,default=ROOT/'proof/tdg_allocation_sweep/mixed')
    p.add_argument('--append-to',type=Path,default=ROOT/'proof/tdg_allocation_sweep')
    p.add_argument('--cases',nargs='+',default=[c['name'] for c in CASES])
    p.add_argument('--budgets',nargs='+',type=int,help='Restrict each case to these budgets')
    p.add_argument('--repeats',type=int,default=3); p.add_argument('--samples',type=int,default=5)
    p.add_argument('--seed',type=int,default=90219); p.add_argument('--timeout',type=int,default=900)
    p.add_argument('--no-append',action='store_true'); p.add_argument('--check-only',action='store_true')
    p.add_argument('--analyze-only',action='store_true')
    args=p.parse_args(); out=args.out.resolve(); out.mkdir(parents=True,exist_ok=True)
    if args.check_only: check(out); return
    if args.analyze_only:
        sweep.analyze(out)
        if not args.no_append: merge(out,args.append_to.resolve())
        return
    assert args.repeats>=3 and args.samples>=3
    assert set(args.cases)<={c['name'] for c in CASES}
    cases=[dict(c) for c in CASES if c['name'] in args.cases]
    for case in cases:
        if args.budgets: case['budgets']=[b for b in case['budgets'] if b in args.budgets]
        assert case['budgets']; case['total_tasks']=len(case['ordinary'])+2; case['orders']=orders(case)
    before={name:sweep.sha(ROOT/name) for name in sweep.PROTECTED}
    soft,hard=resource.getrlimit(resource.RLIMIT_STACK)
    resource.setrlimit(resource.RLIMIT_STACK,(64*1024*1024 if hard<0 else min(64*1024*1024,hard),hard))
    generate(cases); records=sweep.build(cases,out,HERE/'mixed_driver.c')
    meta=dict(oracle_version=2,cases=cases,arguments={k:str(v) if isinstance(v,Path) else v for k,v in vars(args).items()},
              protected_sha256=before,generated=records,build_directory=str(out/'build'),
              oracle_shared_between_policies=True,ordinary_static_work_units=18,
              scope='Synthetic runtime level of actual SGPL loop callbacks and serial SGPL function invocations; compiler level grouping is not validated.',
              exhaustive='All loop widths, canonical priority orders, parent counts, backfill options of existing executors; identical ordinary task labels deduplicated.',
              started_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()))
    meta['test_source_sha256']={name:sweep.sha(HERE/name) for name in ('driver.c','mixed_driver.c','mixed.py','run.py')}
    (out/'mixed_manifest.json').write_text(json.dumps(meta,indent=2)+'\n')
    meta['physical_timed_samples']=measure(cases,args,out)
    assert before=={name:sweep.sha(ROOT/name) for name in sweep.PROTECTED}
    (out/'mixed_manifest.json').write_text(json.dumps(meta,indent=2)+'\n')
    check(out); sweep.analyze(out)
    from mixed_report import report
    report(out)
    if not args.no_append: merge(out,args.append_to.resolve())

if __name__=='__main__': main()
