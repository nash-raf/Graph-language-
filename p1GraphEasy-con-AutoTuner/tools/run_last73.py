#!/usr/bin/env python3
"""Run size ranks 201..273 with bounded disk use and a final combined report.

Order remains algorithm -> graph -> P1/Graptor/GraphIt/Ligra. Only generated
working formats, caches, binaries and value dumps are removed after each case
group; source graphs, source hashes, sample journals and validation reports stay.
"""
import collections,csv,fcntl,gc,hashlib,json,os,shutil,signal,subprocess,sys,time
from pathlib import Path
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import dijkstra
from prepare_e2e150 import sha,weights
from summarize_e2e273 import summarize

R=Path(__file__).resolve().parent;S=R.parent.parent;OUT=R/'results/run201_273_r5'
ALGS=['bfs','sssp','cc','pagerank'];FWS=['p1','graptor','graphit','ligra']
FIELDS=['algorithm','framework','graph','vertices','arcs','source','workers','weight_provenance',
        'p1_directed','status','validation_passed','samples','median_seconds','mean_seconds',
        'stddev_seconds','min_seconds','max_seconds','q1_seconds','q3_seconds','max_rss_kib','error']
CURRENT=None

def atomic(path,obj):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True);tmp=path.with_suffix(path.suffix+'.tmp')
    tmp.write_text(json.dumps(obj,indent=2)+'\n');tmp.replace(path)

def interrupt(signum,frame):
    if CURRENT and CURRENT.poll() is None:
        try:os.killpg(CURRENT.pid,signal.SIGINT)
        except ProcessLookupError:pass
    raise KeyboardInterrupt

def child(cmd,log=None):
    global CURRENT
    sink=open(log,'w') if log else None
    try:
        CURRENT=subprocess.Popen(cmd,cwd=R,stdin=subprocess.DEVNULL,stdout=sink,
                                 stderr=subprocess.STDOUT if sink else None,start_new_session=True)
        try:return CURRENT.wait()
        except KeyboardInterrupt:
            try:os.killpg(CURRENT.pid,signal.SIGINT)
            except ProcessLookupError:pass
            try:CURRENT.wait(timeout=45)
            except subprocess.TimeoutExpired:
                try:os.killpg(CURRENT.pid,signal.SIGKILL)
                except ProcessLookupError:pass
                CURRENT.wait()
            raise
    finally:
        CURRENT=None
        if sink:sink.close()

def selected():
    rows=list(csv.DictReader(open(R.parent/'undirected_prepared_graphs.csv')))
    for r in rows:
        h=np.fromfile(S/r['undirected_bin_path'],dtype='<u8',count=8)
        assert len(h)==8;r['_n']=int(h[2]);r['_m']=int(h[3])
    rows.sort(key=lambda r:(r['_m'],r['original_path']))
    assert len(rows)==273
    previous=[]
    for batch in ['run100_r5','run101_200_r5']:
        p=json.loads((R/'results'/batch/'protocol.json').read_text());previous+=p['graphs']
        assert p['threads']==32 and p['repeats']==5
        status=json.loads((R/'results'/batch/'status.json').read_text());assert status['completed_cases']==1600
    assert previous==[r['original_path'] for r in rows[:200]]
    assert not set(previous)&{r['original_path'] for r in rows[200:]}
    for i,r in enumerate(rows,1):r['_rank']=i
    return rows[200:]

def base_row(r):
    n,m=r['_n'],r['_m'];p=S/r['undirected_bin_path']
    idx=np.memmap(p,dtype='<u8',mode='r',offset=64,shape=(n,))
    source=None
    for start in range(0,n,65536):
        end=min(n,start+65536);nextidx=np.concatenate((idx[start+1:end],np.array([idx[end] if end<n else m],dtype=np.uint64)))
        live=np.flatnonzero(nextidx>idx[start:end])
        if live.size:source=start+int(live[0]);break
    assert source is not None
    assert r['undirected_preparation_verified']=='yes' and r['original_unchanged']=='True'
    return {'rank':str(r['_rank']),'graph':r['original_path'],'vertices':str(n),
        'arcs_both_directions':str(m),'undirected_edges':str(m//2),
        'original_file':str(S/r['source_path']),'original_sha256':r['original_source_sha256'],
        'original_storage':'both directions' if r['storage_fully_reciprocal']=='yes' else 'one direction per undirected edge',
        'p1_directed':'true' if r['storage_fully_reciprocal']=='yes' else 'false',
        'source':str(source),'source_policy':'smallest vertex with nonzero degree',
        'el':str(S/r['undirected_el_path']),'adj':str(S/r['undirected_adj_path']),'bin':str(p),
        'reference':str(p),'reference_format':'graptor_csr','weight_provenance':'synthetic_positive_1_255_all_graphs',
        'synthetic_seed':'2358','wel':'','wadj':'','weights':'','p1_weighted_file':''}

def prepare(r,alg,work):
    marker=work/'prepared.json'
    if marker.exists():return json.loads(marker.read_text())['row']
    work.mkdir(parents=True,exist_ok=True);row=base_row(r);n,m=r['_n'],r['_m']
    assert sha(row['original_file'])==row['original_sha256'], 'Original source changed'
    if alg=='sssp':
        # Include serialized text, the runtime cache and validation scratch space.
        required=m*80+n*96+3_000_000_000
        assert shutil.disk_usage(work).free>required, f'Not enough scratch disk: need approximately {required/1e9:.1f} GB'
        salt=int.from_bytes(hashlib.sha256(f'2358:{row["graph"]}'.encode()).digest()[:8],'little')
        cmd=[str(R/'bin/prepare_weighted_stream'),row['bin'],row['original_file'],str(work),str(salt),
             '1' if row['p1_directed']=='true' else '0',str(n)]
        print('PREPARE_SYNTHETIC',row['rank'],row['graph'],'arcs',m,flush=True)
        code=child(cmd,work/'weight_generation.log')
        assert code==0, 'Native weighted format preparation failed; '+str(work/'weight_generation.log')
        row.update(wel=str(work/'graph.wel'),wadj=str(work/'graph.wadj'),weights=str(work/'graph.weights'),
                   p1_weighted_file=str(work/'input.original.wel'),sssp_reference_path=str(work/'distances.npy'))
        assert Path(row['weights']).stat().st_size==4*m
        ip=np.empty(n+1,dtype=np.int64)
        ip[:n]=np.memmap(row['bin'],mode='r',dtype='<u8',offset=64,shape=(n,));ip[n]=m
        dst=np.memmap(row['bin'],mode='r',dtype='<u4',offset=64+8*n,shape=(m,))
        data=np.memmap(row['weights'],mode='r',dtype='<f4',shape=(m,))
        rng=np.random.default_rng(2358);sample=rng.integers(0,m,size=min(4096,m),dtype=np.int64)
        owners=np.searchsorted(ip,sample,side='right')-1
        assert np.array_equal(data[sample],weights(owners,dst[sample],row['graph'],2358)), 'Native/Python weight mapping differs'
        print('PREPARE_REFERENCE Dijkstra',row['rank'],row['graph'],flush=True)
        distances=dijkstra(csr_matrix((data,dst,ip),shape=(n,n)),directed=True,indices=int(row['source']))
        maximum=float(distances[np.isfinite(distances)].max());assert maximum<2**24 and maximum<2**31-1,'Distances exceed exact float32 integer range'
        np.save(row['sssp_reference_path'],distances);row['sssp_max_reference_distance']=str(maximum)
        del data,dst,ip,distances;gc.collect()
    paths=[row[k] for k in ['original_file','el','adj','bin','wel','wadj','weights','p1_weighted_file','sssp_reference_path'] if row.get(k)]
    row['format_sha256']=json.dumps({p:sha(p) for p in dict.fromkeys(paths)},sort_keys=True)
    manifest=work/'input.csv'
    with open(manifest,'w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(row));w.writeheader();w.writerow(row)
    atomic(marker,{'row':row,'manifest_sha256':sha(manifest)})
    return row

def group_rows(folder):
    p=folder/'cases.jsonl'
    if not p.exists():return []
    latest={}
    for line in p.read_text().splitlines():
        try:r=json.loads(line)
        except ValueError:break
        latest[(r['algorithm'],r['framework'],r['graph'])]=r
    return list(latest.values())

def update():
    rows=[]
    for p in sorted((OUT/'subruns').glob('*/*')):rows+=group_rows(p)
    assert len({(r['algorithm'],r['framework'],r['graph']) for r in rows})==len(rows)
    if rows:
        temp=OUT/'summary.csv.tmp'
        with open(temp,'w',newline='') as f:
            w=csv.DictWriter(f,fieldnames=FIELDS);w.writeheader();w.writerows(rows)
        temp.replace(OUT/'summary.csv')
    passed=sum(r['status']=='ok' for r in rows)
    atomic(OUT/'status.json',{'completed_cases':len(rows),'expected_cases':1168,'passed_cases':passed,
        'failed_cases':len(rows)-passed,'updated_unix':time.time()})
    summarize(R)
    print(f'LAST73_PROGRESS cases {len(rows)}/1168; passed {passed}; failed {len(rows)-passed}; '
          f'planned launch slots resolved {6*len(rows)}/7008',flush=True)

def safe_remove(path):
    path=Path(path)
    assert path.resolve().is_relative_to(OUT.resolve()) and path.resolve()!=OUT.resolve()
    if path.exists():shutil.rmtree(path)

def cleanup(folder,work):
    if work.exists():
        for p in work.iterdir():
            if p.is_file() and p.suffix in ['.json','.csv','.log']:
                target=folder/'input_audit'/p.name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
    # Retain hashes and short previews instead of multi-gigabyte value dumps.
    for p in (folder/'logs').rglob('*'):
        if p.is_file() and p.name in ['validation.log','validation.values']:
            if p.with_suffix(p.suffix+'.audit.json').exists():continue
            info={'sha256':sha(p),'bytes':p.stat().st_size,'full_dump_removed_after_case_group':True}
            with open(p,'rb') as f:
                first=[];last=collections.deque(maxlen=3)
                for i,line in enumerate(f):
                    if i<8:first.append(line.decode(errors='replace').rstrip())
                    last.append(line.decode(errors='replace').rstrip())
            atomic(p.with_suffix(p.suffix+'.audit.json'),info)
            p.write_text('VALUE_DUMP_AUDIT '+json.dumps(info)+'\nHEAD\n'+'\n'.join(first)+'\nTAIL\n'+'\n'.join(last)+'\n')
    p1=folder/'p1'
    if p1.exists():
        for p in p1.rglob('*'):
            if p.is_file() and (p.suffix in ['.json','.graph','.log']):
                target=folder/'build_audit'/p.relative_to(p1);target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
        safe_remove(p1)
    safe_remove(folder/'references');safe_remove(work)

def failed_group(r,alg,folder,error):
    folder.mkdir(parents=True,exist_ok=True);done={x['framework'] for x in group_rows(folder)}
    for fw in FWS:
        if fw in done:continue
        row={'algorithm':alg,'framework':fw,'graph':r['original_path'],'vertices':r['_n'],'arcs':r['_m'],
             'source':0,'workers':32,'weight_provenance':'synthetic_positive_1_255_all_graphs' if alg=='sssp' else 'unweighted',
             'p1_directed':'true' if r['storage_fully_reciprocal']=='yes' else 'false','status':'failed',
             'validation_passed':False,'samples':0,'error':error}
        # Match the normal summary schema so failures cannot disappear from totals.
        for key in ['median_seconds','mean_seconds','stddev_seconds','min_seconds','max_seconds','q1_seconds','q3_seconds','max_rss_kib']:row[key]=''
        with open(folder/'cases.jsonl','a') as f:f.write(json.dumps(row)+'\n')

def main():
    OUT.mkdir(parents=True,exist_ok=True);lock=open(R/'.last73.workflow.lock','a')
    fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
    signal.signal(signal.SIGINT,interrupt);signal.signal(signal.SIGTERM,interrupt)
    catalog=selected()
    atomic(OUT/'selection.json',{'first_rank':201,'last_rank':273,'count':73,
        'graphs':[r['original_path'] for r in catalog],'disjoint_from_first_200':True,
        'order':'algorithm -> ascending size rank -> P1, Graptor, GraphIt, Ligra',
        'workers':32,'repeats':5,'seed':2358,'runner_sha256':sha(R/'run_benchmarks_201_273_r5.py'),
        'weight_generator_sha256':sha(R/'bin/prepare_weighted_stream')})
    for alg in ALGS:
        for r in catalog:
            label=f'{r["_rank"]:03d}';folder=OUT/'subruns'/alg/label;work=OUT/'workspace'/(alg+'_'+label)
            if len(group_rows(folder))==4:
                cleanup(folder,work);continue
            print('LAST73_GROUP',alg,'rank',r['_rank'],r['original_path'],'arcs',r['_m'],flush=True)
            try:
                prepare(r,alg,work)
                cmd=[sys.executable,'-u',str(R/'run_benchmarks_201_273_r5.py'),'--manifest',str(work/'input.csv'),
                     '--limit','1','--algorithms',alg,'--repeats','5','--threads','32','--compile-jobs','1',
                     '--compile-memory','54G','--compile-timeout','1800','--output',str(folder),'--resume']
                code=child(cmd)
                if len(group_rows(folder))<4:failed_group(r,alg,folder,f'Runner stopped before all four cases completed; exit {code}; see console and subrun logs')
            except KeyboardInterrupt:raise
            except Exception as exc:
                print('LAST73_GROUP_ERROR',alg,r['_rank'],repr(exc),flush=True)
                failed_group(r,alg,folder,repr(exc))
            update();cleanup(folder,work)
    update();result=summarize(R)
    print('LAST73_FINISHED',json.dumps(result),flush=True)
    print('COMBINED_WINS: algorithm baseline P1-wins matched-cases P1-geomean-speedup unavailable-pairs',flush=True)
    for c in result['comparisons']:
        print(c['algorithm'],c['baseline'],c['p1_wins'],c['matched_cases'],c['p1_geomean_speedup'],c['unavailable_pairs'],flush=True)

if __name__=='__main__':
    try:main()
    except KeyboardInterrupt:
        print('LAST73_INTERRUPTED: rerun python3 -u run_last73.py to resume completed case groups.',flush=True);sys.exit(130)
