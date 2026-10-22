"""Run the official Kaggle CLI with a locally stored token, without printing it."""
from pathlib import Path
import os,subprocess,sys
token=Path(os.environ.get('P1_KAGGLE_TOKEN_FILE','/mnt/c/Users/kazis/.kaggle/access_token'))
env=dict(os.environ)
if token.exists():env['KAGGLE_API_TOKEN']=token.read_text(encoding='utf-8-sig').strip()
env['KAGGLE_CONFIG_DIR']=str(Path.home()/'p1_kaggle_cli/config')
bridge=Path(__file__).with_name('kaggle_dns_bridge.py')
command=([str(Path.home()/'p1_kaggle_cli/bin/python'),'-B',str(bridge)] if bridge.exists() else
    [str(Path.home()/'p1_kaggle_cli/bin/kaggle')])
result=subprocess.run([*command,*sys.argv[1:]],env=env)
raise SystemExit(result.returncode)
