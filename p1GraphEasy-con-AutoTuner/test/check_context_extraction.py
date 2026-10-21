"""Verify the executor repair changes only concurrent context snapshot reads."""
from pathlib import Path

root = Path(__file__).resolve().parent.parent
text = (root / 'autotuner_runtime.c').read_text()
start = text.index('static void sgpl_exec_partition_body(int64_t index, void *opaque) {')
end = text.index('/* A new compiler must never silently interpret', start)
expected = text[start:end].strip()
actual = (root / 'execution_partition_bodies.inc').read_text().split('*/', 1)[1].strip()
old = 'sgpl_exec_ctx local = *ctx; /* per-worker copy: partition_state is thread-local */'
new = 'sgpl_exec_ctx local;sgpl_exec_context_copy(&local,ctx); /* worker-local state */'
assert actual.count(new) == 2
assert actual.replace(new, old) == expected, 'Executor changed beyond the snapshot repair'
selection = (root / 'array_frontier_runtime.inc').read_text()
for name in ['sgpl_exec_partition_body', 'sgpl_exec_partition_body_packed']:
    assert f'#define {name} {name}_legacy_context_copy' in selection
print('PASS executor extraction: only the two context snapshots changed')
