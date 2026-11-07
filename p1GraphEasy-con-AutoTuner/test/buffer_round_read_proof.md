# Buffer round-read proof

`pdg_buffer_lifecycle.h` supplements the PDG direction proof. It proves that
the input buffer remains unchanged from round entry through the traversal.
It accepts these publication forms:

- Reciprocal pointer-slot swaps after the traversal, on every round backedge.
- Reciprocal pointer PHIs at the round header, with distinct initial objects.
- Element copies or LLVM memory transfers after the traversal. Copy loops
  must occur on every continuing round path.

The slot proof checks initialization, every slot use, and both swap stores.
Whole-object alias and ModRef queries check other stores and calls across the
entire round. The proof rejects input writes during the traversal, unknown
modifications, escaped slots, unsupported lifetimes, and incomplete exchanges.
Runtime names and frontier membership do not supply evidence. If LLVM cannot
prove a call disjoint from the buffers, the certificate is unavailable.

The result describes a state version. The first round can read initialization.
An untouched element can retain older state. The proof does not assert full
copy coverage, a unique reaching writer, or an exact distance of one.

Buffer certificates are stored separately from direction certificates. They
set the read atom's `PreviousRoundRead` label but do not change its snapshot
status or discharge access relations. A suffix copy retains its same-round
write-after-read dependency. Source and neighbor carrying remain separate
from the buffer certificate.

Run `test/run_previous_round_read_tests.sh` for direction and buffer tests.
The fixtures cover stack and heap buffers, local and global slots, SSA swaps,
element copies, memory transfers, and unsafe alias/publication variants.
