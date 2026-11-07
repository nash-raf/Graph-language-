# Conservative carried dependencies

Source IDs are unique by language semantics. This analysis does not inspect
loop increments, source lists, or graph topology to prove that guarantee.

Two exact `A[u]` accesses cannot overlap across source occurrences when they
use the same stable storage and element layout. The analysis checks the full
address shape, not just its U provenance. Shifted indices, truncated indices,
mixed access widths, and mutable bases do not receive this certificate.

Unresolved destination, shared-location, and alias conflicts are Carried.
This label means a possible conflict that requires ordering. It does not
assert that the actual graph contains a conflict.

Each relation records source carrying, neighbor carrying, and round
uncertainty separately. Exact source indexing does not exclude neighbor
carrying: repeated pair updates to `A[u]` still share the source's element.
No carried label supplies a round distance. The existing earlier-round
certificates remain separate.

The relation builder includes repeated executions of one write. It also
includes both WAR and reverse RAW for read-before-write recurrences.
Different base expressions remain alias candidates unless whole-object AA
and stable pointer evidence prove separation across the traversal.
Unknown alias pairs cannot inherit a transformation discharge from one base.

The existing complete spatial and temporal analyses still check unmodeled
calls and memory effects before scheduling. This address certificate does
not authorize removal of their barriers or replace those analyses.

Tests run through `run_previous_round_read_tests.sh`. They cover exact
source accesses, source-list bindings, destination overlaps, shifted and
noninjective indices, shared state, aliases, repeated writes, and graph
projection. The tests also check that no round distance is invented.
