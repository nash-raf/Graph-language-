from pathlib import Path
import subprocess, shlex
root=Path(__file__).resolve().parent.parent
build=root/'build_tdg_validation'
source=build/'AutoTunerPass_original_model.cpp'
source.write_bytes(subprocess.check_output(['git','show','HEAD:p1GraphEasy-con-AutoTuner/AutoTunerPass.cpp'],cwd=root))
def llvm(*args):
    return shlex.split(subprocess.check_output(['llvm-config-20',*args],text=True))
obj=build/'AutoTunerPass_original_model.o'
subprocess.run(['g++','-w','-O1','-std=c++17','-fexceptions','-pthread','-mavx2','-march=native','-iquote',str(root),*llvm('--cxxflags'),'-fexceptions','-c',str(source),'-o',str(obj)],check=True)
sources=['main.cpp','IRGenVisitor.cpp','MotifPattern.cpp','MotifIRBuilder.cpp','ASTBuilder.cpp','pdg.cpp','parallel_loop_outline.cpp','graph_frontier_lowering.cpp','SemanticAnalyzer.cpp','roaring_bitmap.cpp','AutoTunerPass.cpp','generated/BaseBaseListener.cpp','generated/BaseBaseVisitor.cpp','generated/BaseLexer.cpp','generated/BaseListener.cpp','generated/BaseParser.cpp','generated/BaseVisitor.cpp']
objects=[str(obj if name=='AutoTunerPass.cpp' else build/(name.replace('/','_')+'.o')) for name in sources]
exe=build/'GraphProgram_original_cost_model'
subprocess.run(['g++',*objects,*llvm('--ldflags'),'-pthread','-L/usr/local/lib','-Wl,-rpath,/usr/local/lib','-lantlr4-runtime',*llvm('--libs','all'),*llvm('--system-libs'),'-o',str(exe)],check=True)
script=(root/'validate_rt_expr.sh').read_text()
script=script.replace('SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"','SCRIPT_DIR="'+str(root)+'"')
script=script.replace('./GraphProgram',str(exe))
(build/'validate_firstwins_original_model.sh').write_text(script)
print('Built original-model control with all other compiler objects unchanged')
