import subprocess
from pathlib import Path
root=Path(__file__).resolve().parent.parent
checks=[['bash','validate_reduction.sh'],['bash','validate_roundsep.sh'],['bash','validate_composition.sh'],['bash','validate_tdg_budget.sh'],['python3','test/pdg_tdg_verification/audit_pdg_argmem_calls.py',str(root/'build_tdg_validation/GraphProgram_tdg')]]
failed=0
for command in checks:
    print('RUN '+' '.join(command),flush=True)
    result=subprocess.run(command,cwd=root)
    print('RESULT '+command[1]+' exit='+str(result.returncode),flush=True)
    failed+=result.returncode!=0
print('Remaining-check failures: '+str(failed),flush=True)
raise SystemExit(bool(failed))
