#!/usr/bin/env python3
"""Record test-only harness corrections without changing frozen production."""
from pathlib import Path
import hashlib,json,sys
repo=Path(__file__).resolve().parents[1]
version=sys.argv[1]
root=Path('/home/kazi/p1_layout_native_20261105/versions')/version
out=root/'test_overlays';out.mkdir(exist_ok=True)
records=[]
for name in sys.argv[2:]:
    relative=Path(name)
    assert not relative.is_absolute() and '..' not in relative.parts
    target=root/'src'/relative
    old=target.read_bytes();new=(repo/relative).read_text().encode()
    backup=out/(relative.name+'.original')
    if not backup.exists():backup.write_bytes(old)
    target.write_bytes(new)
    records.append(dict(path=name,original_sha256=hashlib.sha256(backup.read_bytes()).hexdigest(),
                        updated_sha256=hashlib.sha256(new).hexdigest()))
(out/('overlay_'+relative.stem+'.json')).write_text(json.dumps(records,indent=2))
print(records)
