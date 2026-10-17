#!/usr/bin/env python3
"""Copy the selected originals and prepare identical positive SSSP weights."""
import argparse,csv,hashlib,json,os,shutil,sys
from pathlib import Path
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import dijkstra

def sha(path):
    h=hashlib.sha256()
    with open(path,'rb') as f:
        for data in iter(lambda:f.read(1024*1024),b''):h.update(data)
    return h.hexdigest()

def weights(u,v,graph,seed):
    lo=np.minimum(u,v).astype(np.uint64);hi=np.maximum(u,v).astype(np.uint64)
    salt=int.from_bytes(hashlib.sha256(f'{seed}:{graph}'.encode()).digest()[:8],'little')
    with np.errstate(over='ignore'):
        x=((lo<<np.uint64(32))|hi)^np.uint64(salt)
        x+=np.uint64(0x9e3779b97f4a7c15)
        x=(x^(x>>np.uint64(30)))*np.uint64(0xbf58476d1ce4e5b9)
        x=(x^(x>>np.uint64(27)))*np.uint64(0x94d049bb133111eb)
        x^=x>>np.uint64(31)
    return (x%np.uint64(255)+1).astype(np.int32)

def prepare(base,manifest,destination,seed=2358,first_rank=1):
    base=Path(base).resolve();destination=Path(destination).resolve()
    rows=list(csv.DictReader(open(manifest)));count=len(rows);assert count>0,'Empty manifest'
    assert len({r['graph'] for r in rows})==count,'Duplicate graph names'
    rows.sort(key=lambda r:(int(r['arcs_both_directions']),r['graph']))
    # Verify the exact requested rank interval against the entire corpus.
    allrows=list(csv.DictReader(open(base/'bench/undirected_prepared_graphs.csv')))
    catalog=[]
    for r in allrows:
        p=Path(r['undirected_bin_path']);p=p if p.is_absolute() else base/p
        header=np.fromfile(p,dtype='<u8',count=8);assert len(header)==8
        catalog.append((int(header[3]),r['original_path']))
    catalog.sort()
    assert [r['graph'] for r in rows]==[g for _,g in catalog[first_rank-1:first_rank-1+count]],'Manifest does not match requested size ranks'
    destination.mkdir(parents=True,exist_ok=True)
    for i,r in enumerate(rows,1):
        source=Path(r['original_file']);copied=destination/r['graph']
        assert copied.resolve().is_relative_to(destination)
        copied.parent.mkdir(parents=True,exist_ok=True)
        digest=sha(source)
        if not copied.exists():shutil.copy2(source,copied)
        assert sha(copied)==digest,('Original copy differs',r['graph'])
        r['downloaded_original']=str(source);r['original_file']=str(copied);r['original_sha256']=digest
        r['rank']=str(first_rank+i-1)
        print(f'[originals {i}/{count}; {count-i} remaining] {r["graph"]}',flush=True)
    result=[]
    for i,r in enumerate(rows,1):
        n,m=int(r['vertices']),int(r['arcs_both_directions']);directed=r['p1_directed'].lower()=='true'
        a=np.loadtxt(r['original_file'],dtype=np.int64,usecols=(0,1),ndmin=2)
        assert len(a)>0 and a.shape[1]==2 and int(a.min())>=0 and int(a.max())+1==n
        raw=(a[:,0].astype(np.uint64)<<np.uint64(32))|a[:,1].astype(np.uint64)
        reverse=(a[:,1].astype(np.uint64)<<np.uint64(32))|a[:,0].astype(np.uint64)
        graph=Path(r['bin']);header=np.fromfile(graph,dtype='<u8',count=8)
        assert tuple(map(int,header[2:4]))==(n,m)
        idx=np.fromfile(graph,dtype='<u8',count=n,offset=64).astype(np.int64)
        dst=np.fromfile(graph,dtype='<u4',count=m,offset=64+8*n)
        rp=np.append(idx,m);assert rp[0]==0 and np.all(np.diff(rp)>=0)
        assert len(dst)==m and (not m or int(dst.max())<n)
        src=np.repeat(np.arange(n,dtype=np.uint32),np.diff(rp))
        keys=(src.astype(np.uint64)<<np.uint64(32))|dst.astype(np.uint64)
        expected=np.sort(raw if directed else np.concatenate((raw,reverse)))
        assert np.array_equal(expected,np.sort(keys)),('P1 direction does not match prepared CSR',r['graph'])
        assert np.array_equal(np.sort(keys),(np.sort((dst.astype(np.uint64)<<np.uint64(32))|src.astype(np.uint64))))
        # Verify the existing text formats too; do not trust their filenames.
        el=np.loadtxt(r['el'],dtype=np.uint32,ndmin=2)
        assert np.array_equal(np.sort((el[:,0].astype(np.uint64)<<np.uint64(32))|el[:,1]),np.sort(keys))
        with open(r['adj']) as f:
            assert f.readline().strip()=='AdjacencyGraph'
            assert int(f.readline())==n and int(f.readline())==m
            adjacency=np.fromstring(f.read(),sep=' ',dtype=np.int64)
        assert np.array_equal(adjacency[:n],idx) and np.array_equal(adjacency[n:],dst)
        folder=destination/f'synthetic_weights_seed{seed}'/Path(r['graph']).with_suffix('')
        folder.mkdir(parents=True,exist_ok=True)
        original_weights=weights(a[:,0],a[:,1],r['graph'],seed)
        arc_weights=weights(src,dst,r['graph'],seed)
        p1=folder/'input.original.wel';wel=folder/'graph.wel';wadj=folder/'graph.wadj';wf=folder/'graph.weights';wi=folder/'graph.weights.i32'
        np.savetxt(p1,np.column_stack((a,original_weights)),fmt='%d')
        np.savetxt(wel,np.column_stack((src,dst,arc_weights)),fmt='%d')
        with open(wadj,'w') as f:
            f.write(f'WeightedAdjacencyGraph\n{n}\n{m}\n')
            for data in [idx,dst,arc_weights]:np.savetxt(f,data,fmt='%d')
        arc_weights.astype('<f4').tofile(wf);arc_weights.astype('<i4').tofile(wi)
        # Check the actual serialized formats and integer-to-float conversion.
        assert np.array_equal(np.fromfile(wf,dtype='<f4'),arc_weights)
        written=np.loadtxt(wel,dtype=np.int64,ndmin=2)
        assert np.array_equal(written,np.column_stack((src,dst,arc_weights)))
        with open(wadj) as f:
            assert f.readline().strip()=='WeightedAdjacencyGraph'
            assert int(f.readline())==n and int(f.readline())==m
            written_adj=np.fromstring(f.read(),sep=' ',dtype=np.int64)
        assert np.array_equal(written_adj,np.concatenate((idx,dst,arc_weights)))
        p1w=np.loadtxt(p1,dtype=np.int64,ndmin=2)
        assert np.array_equal(p1w[:,:2],a) and np.array_equal(p1w[:,2],original_weights)
        # A positive-weight Dijkstra reference certifies actual reachable path
        # sums, rather than rejecting sparse-ID graphs using an n*255 bound.
        distances=dijkstra(csr_matrix((arc_weights.astype(np.float64),dst,rp),shape=(n,n)),directed=True,indices=int(r['source']))
        maximum=float(distances[np.isfinite(distances)].max())
        assert maximum<2**24,('Float32 SSSP cannot represent all distances exactly',r['graph'],maximum)
        assert maximum<2**31-1
        reference=folder/'reference.npz'
        np.savez_compressed(reference,indptr=rp,dst=dst,weights=arc_weights,distances=distances)
        r.update(synthetic_seed=str(seed),weight_provenance='synthetic_positive_1_255_all_graphs',
                 p1_weighted_file=str(p1),wel=str(wel),wadj=str(wadj),weights=str(wf),weights_i32=str(wi),
                 reference=str(reference),sssp_eligible='True',sssp_max_reference_distance=str(maximum))
        r['format_sha256']=json.dumps({str(p):sha(p) for p in [Path(r['original_file']),graph,Path(r['el']),Path(r['adj']),p1,wel,wadj,wf,wi,reference]},sort_keys=True)
        result.append(r)
        print(f'[formats {i}/{count}; {count-i} remaining] {r["graph"]}',flush=True)
    out=destination/('graphs150.csv' if first_rank==1 and count==150 else f'graphs{first_rank}_{first_rank+count-1}.csv')
    fields=list(dict.fromkeys(k for r in result for k in r))
    temporary=out.with_suffix('.csv.tmp')
    with open(temporary,'w',newline='') as f:
        w=csv.DictWriter(f,fields);w.writeheader();w.writerows(result)
    temporary.replace(out)
    metadata={'count':count,'selection':f'Ranks {first_rank}..{first_rank+count-1} by prepared CSR arc count, then graph name',
              'maximum_undirected_edges':int(result[-1]['undirected_edges']),
              'seed':seed,'weights':'synthetic positive integers 1..255 for every graph; original weights ignored',
              'p1_direction':'per-row: false for edge-once originals, true for already symmetric originals',
              'formats_verified':True,'manifest_sha256':sha(out)}
    (destination/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
    print('PREPARED',out,flush=True)
    return out

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--base',type=Path,default=Path('/media/readinggroup/Storage/splash'))
    p.add_argument('--manifest',type=Path);p.add_argument('--destination',type=Path);p.add_argument('--seed',type=int,default=2358)
    p.add_argument('--first-rank',type=int,default=1)
    a=p.parse_args();prepare(a.base,a.manifest or a.base/'bench/graphs150.csv',a.destination or a.base/'original_graphs_150',a.seed,a.first_rank)

if __name__=='__main__':main()
