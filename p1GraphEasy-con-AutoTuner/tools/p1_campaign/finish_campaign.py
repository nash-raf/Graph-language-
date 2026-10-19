"""Finish the campaign after promotion or a completed final rejection diagnosis."""
import argparse,json,subprocess,sys,time
from pathlib import Path
HERE=Path(__file__).resolve().parent
OUT=Path('/mnt/c/p1campaign-results')
p=argparse.ArgumentParser(description=__doc__);p.add_argument('version');p.add_argument('--wait',action='store_true');p.add_argument('--keep-accepted',action='store_true');args=p.parse_args()
decision=OUT/(args.version+'_decision.json')
if args.wait:
 while not decision.exists():time.sleep(5)
result=json.loads(decision.read_text())
if args.keep_accepted:
 assert not result.get('promotable') and result['decision']=='reject'
 assert (OUT/(args.version+'_rejection.txt')).exists(),'Finish the rejection investigation first'
else:assert result.get('promotable') and result['decision']=='accept','Final candidate still needs rejection diagnosis or revision; no automatic promotion'
def command(script,*arguments,label=None):
 label=label or script.removesuffix('.py');log=OUT/('finish_'+args.version+'_'+label+'.console.log')
 with log.open('a') as f:subprocess.run([sys.executable,str(HERE/script),*arguments],stdout=f,stderr=subprocess.STDOUT,check=True)
 print(json.dumps(dict(completed=label,log=str(log))),flush=True)
if not args.keep_accepted:command('promote.py',args.version)
command('archive_attempt.py',args.version,'--baseline','R2a3')
for version in ['B0_final','R2a3']:command('runner.py','validate',version,label='validate_'+version)
command('freeze_protocol.py')
accepted=json.loads((OUT/'campaign_state.json').read_text())['last_accepted_version']
command('runner.py','profile',accepted,'--baseline','B0_final','--mode','phases',label='full_phase_profile')
command('verify_profile_work.py','--accepted',accepted)
command('final_comparison.py','--accepted',accepted)
command('runner.py','report',label='decisions_report')
command('write_final_report.py')
command('complete_campaign.py')
print('CAMPAIGN FINISH COMPLETE '+args.version,flush=True)
