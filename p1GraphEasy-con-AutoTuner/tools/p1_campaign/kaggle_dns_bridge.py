"""Local CLI transport: use Windows DNS if WSL's resolver temporarily fails.

The original HTTPS host, SNI and certificate verification remain unchanged.
No resolver settings or credential handling are changed. Never a native runner.
"""
from pathlib import Path
import json,re,runpy,socket,subprocess,sys

original=socket.getaddrinfo
resolved={}
powershell=Path('/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe')

def lookup(host,port,*args,**kwargs):
    name=host.decode('ascii') if isinstance(host,bytes) else host
    if name in resolved:return original(resolved[name],port,*args,**kwargs)
    try:return original(host,port,*args,**kwargs)
    except socket.gaierror:
        if not isinstance(name,str) or not re.fullmatch(r'[A-Za-z0-9.-]+',name) or not powershell.exists():raise
        command="$taskDnsAddresses = @((Resolve-DnsName -Name '"+name+"' -Type A -ErrorAction Stop).IPAddress); ConvertTo-Json -Compress -InputObject $taskDnsAddresses"
        answer=subprocess.run([str(powershell),'-NoProfile','-NonInteractive','-Command',command],capture_output=True,text=True,timeout=20)
        if answer.returncode:raise
        addresses=json.loads(answer.stdout.lstrip('\ufeff'))
        if isinstance(addresses,str):addresses=[addresses]
        addresses=[address for address in addresses if address]
        if not addresses:raise
        resolved[name]=addresses[0]
        print('[Kaggle CLI] Windows DNS fallback: '+name,file=sys.stderr,flush=True)
        return original(resolved[name],port,*args,**kwargs)

socket.getaddrinfo=lookup
entry=Path.home()/'p1_kaggle_cli/bin/kaggle'
sys.argv=[str(entry),*sys.argv[1:]]
runpy.run_path(str(entry),run_name='__main__')
