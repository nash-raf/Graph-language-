#!/usr/bin/env python3
"""Check separate-process cache reuse, keys, corruption, drift and opt-out."""
from pathlib import Path
import os,subprocess,tempfile,struct,time
ROOT=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='sgpl_launch_cache_test_') as temporary:
    folder=Path(temporary);binary=folder/'probe';cache=folder/'cache'
    subprocess.run(['gcc','-O2','-std=gnu11','-pthread','-I'+str(ROOT),
                    str(ROOT/'test/launch_calibration_cache_test.c'),
                    str(ROOT/'autotuner_runtime.c'),'-lnlopt','-lm','-o',str(binary)],check=True)
    env=dict(os.environ,SGPL_NUM_THREADS='4',SGPL_POOL_SPIN_US='50',SGPL_LAUNCH_CACHE_DIR=str(cache),GRAPH_PARALLEL_DEBUG='1')
    def run(extra=None,args=()):
        p=subprocess.run([str(binary),*args],env=dict(env,**(extra or {})),capture_output=True,text=True,timeout=30)
        assert p.returncode==0,p.stdout+p.stderr
        return p.stderr
    first=run();assert '[launch-cache] thread miss' in first,first
    run(args=('tls',))
    assert '[launch-cache] path miss' in first,first
    second=run();assert '[launch-cache] thread hit' in second,second
    assert '[launch-cache] path hit' in second,second
    assert second.count('launch-thread-calibration-sample')==2,second
    assert second.count('launch-total-calibration-sample')==2,second
    changed=run(args=('different-sync-shape',));assert '[launch-cache] path miss' in changed,changed
    changed=run({'OMP_WAIT_POLICY':'ACTIVE'});assert '[launch-cache] thread miss' in changed,changed
    changed=run({'SGPL_POOL_SPIN_US':'0'});assert '[launch-cache] thread miss' in changed,changed
    # Corrupt values, even a valid length, cannot seed the cost model.
    for f in cache.glob('*.bin'):f.write_bytes(b'\xff'*40)
    corrupt=run();assert '[launch-cache] thread miss' in corrupt and '[launch-cache] path miss' in corrupt,corrupt
    def alter_records(**changes):
        for f in cache.glob('*.bin'):
            fields=list(struct.unpack('=QQQdQ',f.read_bytes()))
            if fields[0]!=0x5347504c4c415531:continue
            if 'ns' in changes:fields[3]=changes['ns']
            if 'saved' in changes:fields[2]=changes['saved']
            prefix=struct.pack('=QQQd',*fields[:4]);checksum=1469598103934665603
            for byte in prefix:checksum=((checksum^byte)*1099511628211)&((1<<64)-1)
            fields[4]=checksum;f.write_bytes(struct.pack('=QQQdQ',*fields))
    # A checksum-valid but stale cost must fail the fresh no-op probe.
    alter_records(ns=1e8)
    drift=run();assert '[launch-cache] thread miss' in drift and '[launch-cache] path miss' in drift,drift
    alter_records(saved=int(time.time())-3601)
    expired=run();assert '[launch-cache] thread miss' in expired and '[launch-cache] path miss' in expired,expired
    affinity=sorted(os.sched_getaffinity(0))
    if len(affinity)>1:
        p=subprocess.run([str(binary)],env=env,preexec_fn=lambda:os.sched_setaffinity(0,{affinity[0]}),capture_output=True,text=True,timeout=30)
        assert not p.returncode and '[launch-cache] thread miss' in p.stderr,p.stderr
    disabled=folder/'disabled'
    run({'SGPL_LAUNCH_CACHE':'off','SGPL_LAUNCH_CACHE_DIR':str(disabled)})
    assert not disabled.exists()
    print('PASS cross-process launch costs, shape/policy/affinity keys, corruption, drift, expiry and opt-out')
