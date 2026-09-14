import numpy as np, pandas as pd, scipy.sparse as sp
UN="/home/user/Course/msk1/synth_graphs_unweighted/synth_v_2000000_e_16000000.txt"
WT="/home/user/Course/msk1/synth_graphs_weighted/synth_v_2000000_e_16000000_w.txt"
def load():
    d=pd.read_csv(WT,sep=' ',header=None,dtype=np.int32).values
    return d[:,0].copy(),d[:,1].copy(),d[:,2].copy()
def csr(n,u,v,data=None):
    U=np.concatenate([u,v]); V=np.concatenate([v,u])
    D=np.ones(len(U),dtype=np.int64) if data is None else np.concatenate([data,data])
    return sp.csr_matrix((D,(U,V)),shape=(n,n))
