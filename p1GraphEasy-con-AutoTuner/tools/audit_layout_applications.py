#!/usr/bin/env python3
"""Layout/graph-state audit of exact benchmark objects; no new timing samples."""
import json
import re
from layout_application_cases import OUT, generate
from layout_application_benchmark import objects, expected, verify, adjacency, SETTINGS
from layout_switch_benchmark import ROOT, MODES, environment, run

def signature(pairs):
    mask=(1<<64)-1
    def mix(x):
        x=(x^(x>>30))*0xbf58476d1ce4e5b9 & mask;x=(x^(x>>27))*0x94d049bb133111eb & mask;return x^(x>>31)
    return 2*len(pairs),sum(mix((u<<32)|v)+mix((v<<32)|u) for u,v in pairs)&mask

def main():
    work,objs=objects();wrapper=work/'application_audit.o'
    run(['gcc','-O2','-iquote',ROOT,'-c',ROOT/'tools/layout_application_audit.c','-o',wrapper],work)
    wraps=['autograph_ensure_layout','graph_add_edge','graph_remove_edge','bfs_runtime_src','dfs_runtime_src','autograph_edgemap']
    for base,src,meta in generate(80000,8):
        name=base+'_motif';dest=OUT/'binaries'/name
        pairs=set(map(tuple,meta['final_pairs']));sigs={signature(pairs)}
        if meta['kind']=='route_failover':sigs.add(signature(pairs-{(1,512)}))
        multiplier=4 if meta['kind'] in ('contact_trace','route_failover') else 2
        stream_ops=meta['rounds']*meta['batch']*multiplier
        adds=removes=stream_ops//2
        if meta['kind']=='contact_trace':adds+=8;removes+=8
        elif meta['kind']=='sensor_backbone':removes+=13
        elif meta['kind']=='route_failover':adds+=1;removes+=1
        else:removes+=1
        audits={}
        for mode in MODES:
            obj=dest/(mode+'.program.o')
            if not obj.exists():raise RuntimeError(f'Missing measured object {obj}')
            exe=dest/(mode+'.audit')
            run(['g++','-fopenmp','-pthread','-no-pie',obj,wrapper,*objs,*[f'-Wl,--wrap={s}' for s in wraps],'-ldl','-lnlopt','-o',exe],work)
            env=environment()
            if mode!='AUTO':env['BENCH_FIXED_LAYOUT']=str(MODES.index(mode)-1)
            env.update(AUDIT_STREAM_OPERATIONS=str(stream_ops),AUDIT_WINDOW_OPERATIONS=str(meta['batch']*multiplier),
                       AUDIT_FULL_NATIVE='0' if mode in ('CSR','BCSR') else '1')
            cp=run([exe],OUT/'codes',env,300)
            (dest/(mode+'.audit.log')).write_text(cp.stdout+cp.stderr)
            verify(cp.stdout,expected(meta))
            observed=[(int(a),int(b)) for a,b in re.findall(r'AUDIT snapshot.*arcs=(\d+) digest=(\d+)',cp.stderr)]
            if not observed or any(s not in sigs for s in observed):raise RuntimeError(f'{name}/{mode}: graph fingerprint mismatch')
            counts=re.search(r'AUDIT counts adds=(\d+) removes=(\d+) samples=(\d+) native=(\d+)',cp.stderr)
            values=list(map(int,counts.groups()))
            if values[:2]!=[adds,removes]:raise RuntimeError(f'{name}/{mode}: mutation count mismatch {values[:2]} != {[adds,removes]}')
            transitions=re.findall(r'AUDIT layout (\w+) -> (\w+)',cp.stderr)
            if mode=='AUTO' and transitions!=[('CSR','SET'),('SET','CSR')]:raise RuntimeError(f'{name}: wrong runtime AUTO transitions {transitions}')
            audits[mode]=dict(correct=True,layouts_checked_on_all_calls=True,semantic_outputs_verified=True,
                graph_fingerprints_verified=True,mutation_effect_samples=values[2],native_mutations=values[3],adds=adds,removes=removes,
                transitions=transitions,full_native_updates=mode not in ('CSR','BCSR'))
            print('AUDIT',name,mode,transitions,'native',values[3],'samples',values[2],flush=True)
        (dest/'audit.json').write_text(json.dumps(audits,indent=2)+'\n')

if __name__=='__main__':main()
