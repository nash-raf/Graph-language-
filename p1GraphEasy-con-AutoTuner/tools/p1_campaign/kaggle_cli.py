"""Run the official Kaggle CLI with a locally stored token, without printing it."""
from pathlib import Path
import os,subprocess,sys
token=Path(os.environ.get('P1_KAGGLE_TOKEN_FILE','/mnt/c/Users/kazis/.kaggle/access_token'))
env=dict(os.environ)
if token.exists():env['KAGGLE_API_TOKEN']=token.read_text(encoding='utf-8-sig').strip()
env['KAGGLE_CONFIG_DIR']=str(Path.home()/'p1_kaggle_cli/config')
result=subprocess.run([str(Path.home()/'p1_kaggle_cli/bin/kaggle'),*sys.argv[1:]],env=env)
raise SystemExit(result.returncode)
