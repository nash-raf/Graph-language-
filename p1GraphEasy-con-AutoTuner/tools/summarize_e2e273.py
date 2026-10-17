#!/usr/bin/env python3
"""Combine validated five-launch means; report wins on matched cases only."""
import csv,json,math,statistics
from pathlib import Path

def summarize(root):
    root=Path(root);last=root/'results/run201_273_r5';last.mkdir(parents=True,exist_ok=True)
    rows=[]
    for batch in ['run100_r5','run101_200_r5','run201_273_r5']:
        p=root/'results'/batch/'summary.csv'
        if p.exists():rows+=list(csv.DictReader(open(p)))
    keys=[(r['algorithm'],r['graph'],r['framework']) for r in rows]
    assert len(keys)==len(set(keys)), 'Overlapping cases across benchmark batches'
    good={k:r for k,r in zip(keys,rows) if r['status']=='ok' and int(r['samples'])==5}
    comparisons=[]
    for alg in ['bfs','sssp','cc','pagerank','all']:
        for fw in ['graptor','graphit','ligra']:
            pairs=[]
            for (a,g,f),r in good.items():
                if f!='p1' or (alg!='all' and a!=alg):continue
                other=good.get((a,g,fw))
                if other:pairs.append(float(other['mean_seconds'])/float(r['mean_seconds']))
            expected=1092 if alg=='all' else 273
            comparisons.append({'algorithm':alg,'baseline':fw,'matched_cases':len(pairs),
                'expected_cases':expected,'unavailable_pairs':expected-len(pairs),
                'p1_wins':sum(x>1 for x in pairs),'p1_losses':sum(x<1 for x in pairs),
                'ties':sum(x==1 for x in pairs),
                'p1_geomean_speedup':math.exp(statistics.mean(math.log(x) for x in pairs)) if pairs else None})
    graph_counts={}
    for a,g,f in good:graph_counts[g]=graph_counts.get(g,0)+1
    result={'all_cases_attempted':len(rows)==4368,'case_count':len(rows),'expected_case_count':4368,
        'passed_cases':len(good),'failed_cases':len(rows)-len(good),
        'fully_validated_graphs':sum(n==16 for n in graph_counts.values()),
        'timing':'five-launch average end-to-end process time, compilation and validation excluded',
        'speedup':'baseline mean / P1 mean; above 1 means P1 faster',
        'sssp_weights':'shared deterministic positive 1..255 weights, seed 2358',
        'comparisons':comparisons}
    target=root/'results/combined273_wins.json';temp=target.with_suffix('.json.tmp')
    temp.write_text(json.dumps(result,indent=2)+'\n');temp.replace(target)
    if rows:
        target=root/'results/combined273_summary.csv';temp=target.with_suffix('.csv.tmp')
        with open(temp,'w',newline='') as f:
            w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
        temp.replace(target)
    target=root/'results/combined273_speedups.csv';temp=target.with_suffix('.csv.tmp')
    with open(temp,'w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(comparisons[0]));w.writeheader();w.writerows(comparisons)
    temp.replace(target)
    return result

if __name__=='__main__':
    result=summarize(Path(__file__).resolve().parent)
    print('COMBINED_REPORT',json.dumps(result),flush=True)
