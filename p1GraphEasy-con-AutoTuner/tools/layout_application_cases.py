#!/usr/bin/env python3
"""Distinct application workloads on one shared projected-community fixture."""
import argparse
import json
from pathlib import Path
from layout_switch_benchmark import ROOT, build, compile_case, MODES, run

OUT=ROOT/'bench_results/layout_applications_20261015'

def fixture(core=512, degree=250, tail=32):
    n=core+tail
    pairs={tuple(sorted((u,(u+d)%core))) for u in range(core) for d in range(1,degree+1)}
    pairs.add((0,core))
    pairs.update((u,u+1) for u in range(core,n-1))
    path=OUT/'graphs'/'community_projection.txt'
    path.parent.mkdir(parents=True,exist_ok=True)
    content=''.join(f'{u} {v}\n' for u,v in sorted(pairs))
    if not path.exists() or path.read_text()!=content: path.write_text(content)
    degrees=[0]*n
    for u,v in pairs: degrees[u]+=1;degrees[v]+=1
    return n,pairs,degrees

def mutate(pairs, add=True, cached=False):
    lines=[]
    for u,v in pairs:
        lines.append(f'  G.{"addEdges" if add else "removeEdges"}({u}->{v});')
        if cached:
            op='+' if add else '-'
            lines += [f'  out_degree[{u}] = out_degree[{u}] {op} 1;', f'  out_degree[{v}] = out_degree[{v}] {op} 1;']
    return '\n'.join(lines)

def stream(kind, fresh, old, rounds, cached=False):
    # Different event semantics and loop bodies, not relabeled benchmark scans.
    if kind=='contact_trace':
        ops=mutate(old,False,cached)+'\n'+mutate(fresh,True,cached)+'\n'+mutate(fresh,False,cached)+'\n'+mutate(old,True,cached)
    elif kind=='service_islands':
        ops=mutate(fresh,True,cached)+'\n'+mutate(fresh,False,cached)
    elif kind=='sensor_backbone':
        ops=mutate(old,False,cached)+'\n'+mutate(old,True,cached)
    elif kind=='entity_rank':
        ops=mutate(fresh,True,cached)+'\n'+mutate(old,False,cached)+'\n'+mutate(old,True,cached)+'\n'+mutate(fresh,False,cached)
    elif kind=='route_failover':
        ops='\n'.join(mutate([p],False,cached)+ '\n'+mutate([q],True,cached)+ '\n'+mutate([q],False,cached)+'\n'+mutate([p],True,cached)
                      for p,q in zip(old,fresh))
    else:
        # Grant and revoke each short-lived trust relationship independently.
        ops='\n'.join(mutate([p],True,cached)+'\n'+mutate([p],False,cached) for p in fresh)
    return f'int window = 0;\nwhile (window < {rounds}) {{\n{ops}\n  window = window + 1;\n}}\n'

def bfs_body():
    return '''// Enumerate the exposure cohort and record its furthest hop.
int visited[n]; int parent[n]; int frontier[n]; int next_frontier[n];
int init_i=0;
while(init_i<n) { visited[init_i]=0; parent[init_i]=-1; init_i=init_i+1; }
frontier[0]=0; visited[0]=1; parent[0]=0;
int frontier_size=1; int next_size=0; int hops=0;
while(frontier_size>0) {
  next_size=0; int i=0;
  while(i<frontier_size) {
    int u=frontier[i];
    for v in G.neighbors(u) {
      if(visited[v]==0) {
        visited[v]=1; parent[v]=u;
        next_frontier[next_size]=v; next_size=next_size+1;
      }
    }
    i=i+1;
  }
  swap(frontier,next_frontier); frontier_size=next_size;
  if(frontier_size>0) { hops=hops+1; }
}
int exposed=0; int v=0;
while(v<n) { exposed=exposed+visited[v]; v=v+1; }
print exposed; print hops;
'''

def cc_body():
    return '''// Label all disconnected service islands; emit the largest island.
int label[n]; int frontier[n]; int next_frontier[n];
int frontier_size=n; int next_size=0; int v=0;
while(v<n) { label[v]=v; frontier[v]=v; v=v+1; }
while(frontier_size>0) {
  next_size=0; int i=0;
  while(i<frontier_size) {
    int u=frontier[i];
    for v in G.neighbors(u) {
      if(label[u]<label[v]) {
        label[v]=label[u];
        next_frontier[next_size]=v; next_size=next_size+1;
      }
    }
    i=i+1;
  }
  swap(frontier,next_frontier); frontier_size=next_size;
}
int count[n]; v=0;
while(v<n) { count[v]=0; v=v+1; }
v=0; while(v<n) { count[label[v]]=count[label[v]]+1; v=v+1; }
int islands=0; int largest=0; v=0;
while(v<n) { if(count[v]>0) { islands=islands+1; } if(count[v]>largest) { largest=count[v]; } v=v+1; }
print islands; print largest;
'''

def core_body():
    return '''// Peel sensors that cannot meet the minimum redundant-neighbor budget.
int alive[n]; int deg[n]; int frontier[n]; int next_frontier[n];
int k=490; int frontier_size=0; int next_size=0; int v=0;
while(v<n) { alive[v]=1; deg[v]=out_degree[v]; if(deg[v]<k) { frontier[frontier_size]=v; frontier_size=frontier_size+1; } v=v+1; }
while(frontier_size>0) {
  next_size=0; int i=0;
  while(i<frontier_size) {
    int u=frontier[i];
    if(alive[u]==1) {
      alive[u]=0;
      for v in G.neighbors(u) {
        if(alive[v]==1) {
          deg[v]=deg[v]-1;
          if(deg[v]<k) { next_frontier[next_size]=v; next_size=next_size+1; }
        }
      }
    }
    i=i+1;
  }
  swap(frontier,next_frontier); frontier_size=next_size;
}
int retained=0; v=0;
while(v<n) { retained=retained+alive[v]; v=v+1; }
print retained; print alive[0];
'''

def rank_body():
    return '''// Recompute entity importance after interaction-window replacement.
real rank[n]; real next_rank[n];
real damp=0.85; real beta=0.15/n; int v=0;
while(v<n) { rank[v]=1.0/n; next_rank[v]=0.0; v=v+1; }
int round=0;
while(round<12) {
  for v in G.nodes { next_rank[v]=beta; }
  for u in G.nodes {
    int outd=out_degree[u];
    if(outd>0) {
      real contrib=damp*rank[u]/outd;
      for v in G.neighbors(u) { next_rank[v]=next_rank[v]+contrib; }
    }
  }
  for v in G.nodes { rank[v]=next_rank[v]; }
  round=round+1;
}
real total=0.0; real peripheral=0.0; v=0;
while(v<n) { total=total+rank[v]; if(v>=512) { peripheral=peripheral+rank[v]; } v=v+1; }
print total; print peripheral; print rank[0];
'''

def generate(rounds=10000, batch=8):
    n,base,degrees=fixture()
    fresh=[tuple(sorted(((i*13)%512,((i*13)%512+256)%512))) for i in range(batch)]
    old=[(402+i,502+i) for i in range(batch)]
    assert len(set(fresh))==batch and all(p not in base for p in fresh)
    assert len(set(old))==batch and all(p in base for p in old)
    bodies={'contact_trace':bfs_body(),'service_islands':cc_body(),'sensor_backbone':core_body(),
            'route_failover':'''// Check outage impact, commission the alternate gateway, and verify recovery.
G.removeEdges(0->512);
query outage_core: "bfs_src" 0 of G;
query stranded_clients: "bfs_src" 543 of G;
print outage_core; print stranded_clients;
G.addEdges(1->512);
query recovered: "bfs_src" 0 of G;
query inspection: "dfs_src" 543 of G;
print recovered; print inspection;
''','trust_audit':'''// Enumerate both sides of the revoked trust boundary.
query allowed: "dfs_src" 0 of G;
query isolated: "dfs_src" 543 of G;
print allowed; print isolated;
'''}
    descriptions={
      'contact_trace':'Sliding contact windows and source-based exposure tracing.',
      'service_islands':'Service-topology updates followed by connected-component outage detection.',
      'sensor_backbone':'Radio-link duty cycles followed by a redundant-neighbor k-core audit.',
      'route_failover':'Path switching, gateway outage impact, alternate-route commissioning, and recovery checks.',
      'trust_audit':'Short-lived access grants, revocation, and DFS trust-boundary inventories.'}
    result=[]
    for kind,body in bodies.items():
        cached=kind=='sensor_backbone'
        final=set(base)
        prefix=f'// {descriptions[kind]}\n// Synthetic projected-community fixture shared by all five different programs.\ngraph G {{ edges: file "../graphs/community_projection.txt"; }};\nint n=numVertices(G);\n'
        if cached: prefix+=f'// Degree cache supplied with the snapshot and maintained with every update.\nint out_degree[{n}]=['+','.join(map(str,degrees))+'];\n'
        event=stream(kind,fresh,old,rounds,cached)
        if kind=='contact_trace':
            event+=mutate(old,False)+'\n'+mutate(fresh,True)+'\n'
            final.difference_update(old);final.update(fresh)
        elif kind in ('service_islands','trust_audit'):
            event+=mutate([(0,512)],False)+'\n'; final.remove((0,512))
        elif kind=='sensor_backbone':
            failed=[(0,i) for i in range(1,13)]+[(0,512)]
            event+=mutate(failed,False,True)+'\n';final.difference_update(failed)
        else:
            # The failover body performs the permanent gateway replacement.
            final.remove((0,512)); final.add((1,512))
        # Two useful endpoint reachability checks establish the analysis snapshot.
        if kind not in ('trust_audit','route_failover'):
            event+='query core_snapshot: "bfs_src" 0 of G;\nquery edge_snapshot: "bfs_src" 543 of G;\nprint core_snapshot; print edge_snapshot;\n'
        text=prefix+event+body+'print numEdges(G);\n'
        directory=OUT/'codes';directory.mkdir(parents=True,exist_ok=True)
        name=f'{kind}_events{rounds}_batch{batch}'
        src=directory/(name+'.sgpl')
        if not src.exists() or src.read_text()!=text: src.write_text(text)
        meta=dict(case=name,kind=kind,application=descriptions[kind],n=n,edges=len(base),rounds=rounds,
                  batch=batch,source=str(src),final_pairs=sorted(final),initial_degrees=degrees)
        (directory/(name+'.json')).write_text(json.dumps(meta)+'\n')
        result.append((name,src,meta))
    return result

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--rounds',type=int,default=10000);ap.add_argument('--batch',type=int,default=8)
    ap.add_argument('--compile-auto',action='store_true');ap.add_argument('--case')
    args=ap.parse_args();cases=generate(args.rounds,args.batch)
    if args.compile_auto:
        work,objs=build(OUT)
        # Source-based DFS uses its current, layout-aware C++ implementation.
        obj=work/'dfs_runtime_src.cpp.o'
        if not obj.exists():run(['g++','-O3','-fopenmp','-iquote',ROOT,'-c',ROOT/'dfs_runtime_src.cpp','-o',obj],work)
        objs.append(obj)
        for name,src,meta in cases:
            if args.case and args.case!=meta['kind']:continue
            exe,schedule=compile_case(OUT,work,objs,name+'_motif',src,'AUTO',{'GRAPH_FRONTIER_REWRITE_OFF':'1'})
            print(name,schedule,flush=True)
            ir=(OUT/'binaries'/(name+'_motif')/'AUTO.ll').read_text()
            print('engines:',sum('call ' in line and '@autograph_edgemap(' in line for line in ir.splitlines()),
                  sum('call ' in line and '@autograph_frontier_execute(' in line for line in ir.splitlines()),flush=True)

if __name__=='__main__':main()
