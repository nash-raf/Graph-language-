from pathlib import Path
import tempfile, subprocess
root=Path(__file__).resolve().parent.parent
build=root/'build_tdg_validation'
with tempfile.TemporaryDirectory(prefix='original-model-control-',dir=build) as tmp:
    tmp=Path(tmp)
    for file in root.iterdir():
        if file.is_file() and file.name not in ['program.o','kernels.ptx']:
            (tmp/file.name).symlink_to(file)
    script=(build/'validate_firstwins_original_model.sh').read_text()
    script=script.replace('SCRIPT_DIR="'+str(root)+'"','SCRIPT_DIR="'+str(tmp)+'"')
    path=tmp/'baseline_control.sh'
    path.write_text(script)
    result=subprocess.run(['bash',str(path),'rt_expr_firstwins'])
    print('ORIGINAL_MODEL_CONTROL_EXIT='+str(result.returncode),flush=True)
    raise SystemExit(result.returncode)
