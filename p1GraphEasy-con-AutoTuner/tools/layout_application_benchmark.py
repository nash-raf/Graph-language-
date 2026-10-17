#!/usr/bin/env python3
"""Measure useful, structurally distinct SGPL applications with semantic oracles."""
import argparse
import collections
import csv
import hashlib
import json
import random
import statistics
import time
from layout_application_cases import OUT, generate
from layout_switch_benchmark import ROOT, MODES, build, compile_case, environment, run, stage

SETTINGS={'GRAPH_FRONTIER_REWRITE_OFF':'1'}

def objects():
    work,objs=build(OUT)
    obj=work/'dfs_runtime_src.cpp.o'
    if not obj.exists():run(['g++','-O3','-fopenmp','-iquote',ROOT,'-c',ROOT/'dfs_runtime_src.cpp','-o',obj],work)
    return work,[*objs,obj]

def adjacency(meta):
    adj=[set() for _ in range(meta['n'])]
    for u,v in meta['final_pairs']:adj[u].add(v);adj[v].add(u)
    return adj

def reach(adj,seed):
    distances={seed:0};q=collections.deque([seed])
    while q:
        u=q.popleft()
        for v in adj[u]:
            if v not in distances:distances[v]=distances[u]+1;q.append(v)
    return distances

def expected(meta):
    adj=adjacency(meta);kind=meta['kind'];ref=[]
    if kind=='route_failover':
        outage=[set(row) for row in adj];outage[1].remove(512);outage[512].remove(1)
        ref=[('set',set(reach(outage,0))),('set',set(reach(outage,543))),
             ('set',set(reach(adj,0))),('set',set(reach(adj,543)))]
    elif kind=='trust_audit':
        ref=[('set',set(reach(adj,0))),('set',set(reach(adj,543)))]
    else:
        ref=[('set',set(reach(adj,0))),('set',set(reach(adj,543)))]
        if kind=='contact_trace':
            distances=reach(adj,0)
            ref += [('int',len(distances)),('int',max(distances.values()))]
        elif kind=='service_islands':
            remaining=set(range(meta['n']));components=[]
            while remaining:
                part=set(reach(adj,min(remaining)));components.append(part);remaining-=part
            ref += [('int',len(components)),('int',max(map(len,components)))]
        elif kind=='sensor_backbone':
            alive=set(range(meta['n']));degree=list(map(len,adj));q=collections.deque(v for v,d in enumerate(degree) if d<490)
            while q:
                u=q.popleft()
                if u not in alive:continue
                alive.remove(u)
                for v in adj[u]&alive:
                    degree[v]-=1
                    if degree[v]<490:q.append(v)
            ref += [('int',len(alive)),('int',int(0 in alive))]
    return [*ref,('int',len(meta['final_pairs']))]

def verify(stdout,reference):
    lines=stdout.strip().splitlines()
    if len(lines)!=len(reference):raise RuntimeError(f'Output line count: got {len(lines)}, expected {len(reference)}\n{stdout[-500:]}')
    for i,(line,(kind,value)) in enumerate(zip(lines,reference)):
        if kind=='set':
            nodes=list(map(int,line.split()))
            if len(nodes)!=len(set(nodes)) or set(nodes)!=value:raise RuntimeError(f'Visited-set mismatch at line {i}')
        elif int(line)!=value:raise RuntimeError(f'Output line {i}: got {line}, expected {value}')

def benchmark(item,modes,repeats,warmups,timeout,mode_counts=None,case_suffix='_motif'):
    base,src,meta=item;name=base+case_suffix;work,objs=objects()
    bins={};schedules={}
    for m in modes:
        bins[m],schedules[m]=compile_case(OUT,work,objs,name,src,m,SETTINGS)
        print(name,m,schedules[m],flush=True)
    if 'AUTO' in modes and not any(x!='CSR' for x in schedules['AUTO']):raise RuntimeError(f'{name}: no automatic switch')
    bins=stage(OUT,name,bins)
    prebuild=work/'prebuild_graph_cache'
    if not prebuild.exists():
        helper=work/'prebuild_graph_cache.o'
        run(['g++','-O3','-std=c++17','-fopenmp','-iquote',ROOT,'-c',ROOT/'tools/prebuild_graph_cache.cpp','-o',helper],work)
        run(['g++','-fopenmp','-pthread',helper,*objs,'-ldl','-lnlopt','-o',prebuild],work)
    graph=bins[modes[0]].parents[2]/'graphs'/'community_projection.txt'
    run([prebuild,'--refresh',graph],work,environment())
    ref=expected(meta);samples={m:[] for m in modes};rows=[];rng=random.Random(20261015)
    counts=mode_counts or {m:repeats for m in modes}
    for iteration in range(warmups+max(counts.values())):
        order=list(modes);rng.shuffle(order)
        for m in order:
            if iteration>=warmups+counts[m]:continue
            start=time.perf_counter_ns();cp=run([bins[m]],bins[m].parents[2]/'codes',environment(),timeout)
            elapsed=(time.perf_counter_ns()-start)/1e6;verify(cp.stdout,ref)
            print('MEASURED',name,m,f'{elapsed:.3f}ms',flush=True)
            (OUT/'binaries'/name/(m+'.run.log')).write_text(cp.stdout+cp.stderr)
            if iteration>=warmups:
                samples[m].append(elapsed);rows.append(dict(case=name,variant=m,run=iteration-warmups,total_runtime_ms=elapsed,
                    semantic_output_verified=True,stdout_sha256=hashlib.sha256(cp.stdout.encode()).hexdigest()))
        print(name,f'{iteration+1}/{warmups+max(counts.values())}',{m:round(statistics.median(v),3) for m,v in samples.items() if v},flush=True)
    result=dict(case=name,application=meta['application'],kind=meta['kind'],source=str(src),metadata=meta,
                settings=SETTINGS,schedules=schedules,samples_ms=samples,
                medians_ms={m:statistics.median(v) for m,v in samples.items()},semantic_output_verified=True,
                timed_runs={m:len(v) for m,v in samples.items()},warmups=warmups,input_cache_prebuilt=True)
    if all(m in modes for m in MODES):
        result['speedup_vs_best_fixed']=min(result['medians_ms'][m] for m in MODES[1:])/result['medians_ms']['AUTO']
    suffix='result.json' if all(m in modes for m in MODES) else 'pilot.json'
    (OUT/'binaries'/name/suffix).write_text(json.dumps(result,indent=2)+'\n')
    raw=OUT/('validation_raw_runs.csv' if suffix=='result.json' else 'discovery_raw_runs.csv')
    with raw.open('a',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=list(rows[0]));
        if f.tell()==0:writer.writeheader()
        writer.writerows(rows)
    print('RESULT',name,result['medians_ms'],result.get('speedup_vs_best_fixed'),flush=True)
    return result

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--rounds',type=int,default=10000);ap.add_argument('--batch',type=int,default=8)
    ap.add_argument('--modes',default='AUTO,PCSR');ap.add_argument('--repeats',type=int,default=3);ap.add_argument('--warmups',type=int,default=1)
    ap.add_argument('--timeout',type=int,default=300);ap.add_argument('--case')
    ap.add_argument('--adaptive-confirmation',action='store_true');ap.add_argument('--resume',action='store_true')
    a=ap.parse_args()
    for item in generate(a.rounds,a.batch):
        if a.case and a.case!=item[2]['kind']:continue
        existing=OUT/'binaries'/(item[0]+'_motif')/'result.json'
        if a.resume and existing.exists():
            result=json.loads(existing.read_text())
            if result.get('semantic_output_verified') and result.get('speedup_vs_best_fixed',0)>1:
                print('RESUME retaining completed measurements:',item[0],flush=True);continue
        counts={'AUTO':7,'PCSR':7,'SET':7,'CSR':1,'BCSR':1} if a.adaptive_confirmation else None
        benchmark(item,a.modes.split(','),a.repeats,a.warmups,a.timeout,counts)

if __name__=='__main__':main()
