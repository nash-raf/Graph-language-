"""Add immutable source/toolchain settings without rewriting the input manifest."""
import runner as R
R.M=R.bootstrap()
with R.locked('freeze_protocol'):
 R.verify_inputs();base=R.ROOT/'versions/B0_final'
 sources={a+'/'+row['graph']+'/'+mode:R.sha(R.case('B0_final',a,row)/(mode+'.graph')) for a in R.ALGS for row in R.M.ROWS for mode in ['timed','validate']}
 protocol=dict(input_manifest_sha256=R.sha(R.OUT/'manifest.json'),sources=sources,
  source_root=str(base/'cases'),harness_sha256=R.sha(R.ROOT/'support/common.py'),
  compiler_flags=['-O2','-mavx2','-march=native','-std=c++17','-fexceptions','-pthread'],
  runtime_c_flags=['-O3','-g','-fopenmp','-pthread'],runtime_cpp_extra=['-mavx2','-march=native','-std=c++17'],
  link_flags=['-O3','-fopenmp','-no-pie','-pthread','-ldl','-lnlopt'],
  llvm_cxxflags=R.cfg('--cxxflags'),llvm_ldflags=R.cfg('--ldflags'),
  primary=dict(workers=6,physical_cpus=R.CPUS,warmups=1,fresh_process_repetitions=5),
  confirmation=dict(pairs=20,resamples=100000,seed=2358,case_algorithm_family=248,alpha=.05),
  pagerank=dict(iterations=20,p1_precision='double',historical_graptor='pinned PR',equal_precision_graptor='pinned PRdouble',dangling_behavior='frozen source'))
 path=R.OUT/'build_protocol.json'
 if path.exists():assert R.read(path)==protocol
 else:R.save(path,protocol)
 print(str(path))
