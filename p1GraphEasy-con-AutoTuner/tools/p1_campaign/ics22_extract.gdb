set pagination off
set confirm off
set breakpoint pending on
set print elements 0
python
import gdb, os
name='BFSv' if os.environ['ICS22_ALGORITHM']=='bfs' else 'BFv'
gdb.Breakpoint(name+'<GraphVEBOGraptor<(graptor_mode_t)3> >::run()')
end
run
python
import gdb, os, struct, json
pointer=gdb.parse_and_eval('this')
address=int(pointer)
pointer_type=pointer.type
gdb.execute('finish')
if gdb.selected_frame().name()!='main':
    raise RuntimeError('Traversal did not return normally to main; refusing extraction')
obj=gdb.Value(address).cast(pointer_type).dereference()
n=int(os.environ['ICS22_VERTICES'])
alg=os.environ['ICS22_ALGORITHM']
out=os.environ['ICS22_OUTPUT']
inf=gdb.selected_inferior()
mapping=bytes(inf.read_memory(int(obj['GA']['remap']['remap']['mem']),n*4))
indices=struct.unpack('<'+'I'*n,mapping)
extent=max(indices)+1
data_address=int(obj['parent']['mem'] if alg=='bfs' else obj['new_dist']['mem']['alc']['m_ptr'])
data=bytes(inf.read_memory(data_address,extent*4))
reverse=bytes(inf.read_memory(int(obj['GA']['remap']['reverse']['mem']),extent*4))
open(out+'.mapping','wb').write(mapping)
open(out+'.data','wb').write(data)
open(out+'.reverse','wb').write(reverse)
open(out+'.json','w').write(json.dumps(dict(vertices=n,extent=extent,algorithm=alg,method='Read-only GDB memory extraction after run and before post_process; unmodified optimized executable')))
end
quit
