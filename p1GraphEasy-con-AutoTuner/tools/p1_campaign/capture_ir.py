"""Capture one frozen case's compiler IR under the experiment lock."""
import argparse
import runner as R
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('algorithm',choices=R.ALGS);p.add_argument('graph');args=p.parse_args();R.M=R.bootstrap()
with R.locked('compiler_diagnostic'):
 R.verify_inputs();R.verify_fingerprint(args.version)
 row=next(x for x in R.M.ROWS if x['graph']==args.graph);d=R.ROOT/'versions'/args.version
 out=d/'compiler_diagnostics'/args.algorithm/args.graph.removesuffix('.edges');out.mkdir(parents=True,exist_ok=True)
 for ll in (d/'src').glob('*.ll'):
  if not (out/ll.name).exists():(out/ll.name).symlink_to(ll)
 source=R.case(args.version,args.algorithm,row)/'timed.graph'
 env=R.M.env();env.update(FORCE_CPU='1',GRAPH_FRONTIER_STATS='1',SGPL_RECORD_LIVENESS_STATS='1',DUMP_LLVM_BC_AFTER_OUTLINE=str(out/'after_outline.bc'))
 if not (out/'after_outline.ll').exists():
  R.checked([d/'GraphProgram','--ir-backend=cpu',source],out,out/'compile.log',env)
  R.checked(['llvm-dis-20',out/'after_outline.bc','-o',out/'after_outline.ll'],out,out/'disassemble.log')
 R.save(out/'protocol.json',dict(version=args.version,compiler=R.sha(d/'GraphProgram'),source=R.sha(source),ir=R.sha(out/'after_outline.ll'),diagnostic_only=True))
 print(str(out/'after_outline.ll'))
