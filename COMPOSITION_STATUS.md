# Composition status: what is solved, what is refused, what is still unsolved

Written 2026-09-14 on branch `compositions-round2` (base `merge-ars1-roundsep` @
`9115210`).  Companion to `OPEN_PROBLEMS_HANDOFF.md` §2/§2b/§2c; the evidence for
every line below is a named check in `verify/run.sh` plus the verdict census
(`verify/class_census.sh`).

## Three different things get called "solved"

| Term | Meaning | Example |
|---|---|---|
| **solved-general** | the verdict and the emitted code are *derived* from the loop's effect set / provenance — no shape blocklist, no algorithm name, no per-program special case | A (shadow), H (partials+combine) |
| **refused-general** | the loop is refused, but the refusal is a *derived rule* (it says **why**), it is pinned by a check, and the old behaviour is available behind a kill switch | B, C, D, F, multi-slot |
| **unsolved** | the parallel case has no mechanism at all; correctness today comes from an emit failing, an ad-hoc blocklist, or luck | A-dual, the parallel side of B/C/D/F, raw float `select` min/max, `mutual_deg`-style bodies |

A/H are the only two **solved-general** items.  Everything else is either
**refused-general** (a legitimate, tested outcome but *not* a solution) or
**unsolved**.  Nothing below is solved by a hardcode.

## Item by item

| # | Shape | Status | Derived or hardcoded | Evidence | To make the parallel case general |
|---|---|---|---|---|---|
| **A** | in-place cross-endpoint R×W on one base, indexed by an endpoint (`dist[v] = min(dist[v], …)`) | **solved-general** | derived: the classifier recognises carried-read × mutating-write on the same base and emits a per-round shadow snapshot | `race/roundsep`: `shadow=1 class=dest-owner`, identical answer at 1/4 threads × 1/3/4 partitions, matched against an independent Python model. **Caveat:** the shadow implements *frozen-round* semantics, which differs from the serial build for in-place loops (serial overflows int32, rewrite answers 180000) — the language definition still has to say which is meant | — (open question is semantic, not a missing mechanism) |
| **A-dual** | dual-owner step whose cross-region read needs a shadow (claim/activate state machine, `small_kcore`) | **unsolved** (refused-general) | refusal is derived (`Info.RoundSepBases` non-empty disqualifies `DualOwner` because the frozen snapshot breaks within-round observations) | `race/dual_shadow`: 18898 vs 18959 over-peel before the refusal, `class=sequential`, 18959 now | classify each **read** as round-separated or within-round (per-read temporal model), then shadow only the round-separated ones.  A shape list cannot do it: the same array carries both kinds |
| **B** | write at a *data-valued* subscript (`cnt[deg[u]]`) | **refused-general**, parallel side unsolved | derived from index provenance: a subscript whose region is `D` (data-array element) has no vertex owner; `CC_PART_OF` is keyed by destination vertex | `race/data_index_write`: `data=1 class=sequential`, answer matches the serial build | a data-valued owner mapping (e.g. stable hash of the *value* into partitions, computed once per round) plus a conflict argument for equal values; today the value range is unknown at compile time |
| **C** | one array written through both endpoint regions (mixed U+V) | **refused-general**, parallel side unsolved | derived: same-base overlap across `EU`/`EV` fails the disjointness test `DualOwner` needs | `race/mixed_regions` (now pinned): `class=sequential`, `tot 340000`, 1thr==4thr | ordered execution between the two write phases (DOACROSS-style sequencing on the shared base) or a conflict-free decomposition of the two writes — a new step ABI, not a pattern |
| **D** | reduction next to a per-vertex array write | **refused-general**, parallel side unsolved | derived: the reduction engine owns exactly one scalar slot (`ReducePtr`); a vertex write needs vertex ownership — the two do not compose today | `race/reduce_plus_write` (now pinned): `class=sequential`, `acc 160000 w0 1` | generalise P10's finish hook: a step that carries a per-partition partial *and* owns vertex rows, i.e. "reduce into the partial, write the vertex row in the same owner step" |
| **E** | cross-phase dependence between the two regions of a dual-owner loop | **solved-general** | derived: `DualOwner` = `E = EU ∪ EV`, disjoint bases, `Dependence ⊆ Control/Membership`; the front-end effect string shows the three effects explicitly | `race/bfs_level` (race-clean, dest-owner, `R(visited,V):SameRoundRead ⊗ Claim(visited,V) ⊗ R(lvl,U):PreviousRoundRead`) | — |
| **F** | first-wins claim in the **driver preamble**, gating the neighbour body | **refused-general**, parallel side unsolved | derived from *position*: the claim is in the driver preamble, not the body, so no work function can reproduce "once per source, and only if the claim won" | `race/claim_driver` + kill switch `SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1` (switch on → `class=dual-owner` **and** `emit failed … -> stays sequential`, i.e. the old verdict was an accident) | prove the driver visits each source once (or that the claim is idempotent and its result unused), or run a per-source pre-pass outside the pair loop.  Body-level claims already work because they are cloned with their guard |
| **G** | scalar-global reductions with exotic operators | **solved-general for the recognised op family**; one **refused-general** residue | derived: the combine reproduces the body's own operation per flavour (`smin`/`umin`/`minnum`/`minimum`, signedness from the predicate, identity per flavour) — no op list | `race/reduce_int_ops` (7 operators, incl. both guarded and `min()`/`max()` forms, vs an independent Python computation), `race/reduce_real_ops` | the residue is the raw float `select(fcmp olt/ogt)` form: it is **not reorder-invariant** with NaN or signed zeros, so folding partials with it cannot reproduce the serial fold.  General fix = prove the stream finite/NaN-free (fast-math or a finite-value certificate), otherwise it stays sequential |
| **H** | reduction emitted but never validated | **solved-general** | derived: any recognised associative/commutative update maps to per-partition partials + an ordered combine (an identity per operator); the op comes from the IR, not from a name | `validate_reduction.sh` (24-config red matrix) + `race/reduce_int_ops` / `race/reduce_real_ops` in the suite | — |
| **I** | array-frontier / per-source gathers (`c = 0; for each neighbor v { c += f(v) } deg[u] = c`) | **solved-general behind a switch** (`SGPL_COMP_I_SOURCE_REDUCTION=1`, default off) | derived: source-owned step + per-partition partial + a finish hook that is *cloned from the driver epilogue* (no pattern for `deg[u] = c`); arc-less sources are handled by a source-range pass in the runtime | `parallel/int_gather_source_red`: `class=source-red`, `degsum 40` at 1/4 threads × 1/3/4 partitions (bipartite.txt has 10 sources with no out-arcs, so the range pass is exercised); default refusal still pinned by `race/int_gather` | keep it off until more shapes are exercised; then handle bodies with inline graph queries (`mutual_deg`'s `hasEdge`) — those are refused by the totality prover, not by this item |
| **extra** | second scalar accumulator (or a second operator on the same slot) in one body | **refused-general**, parallel side unsolved | derived: only `ReducePtr` gets per-partition storage, so a second slot would be written by every partition; the store is now recorded as an unrecognised global effect (`U_f`) | `race/two_reduce_slots` (`class=sequential`, `a 10 b 20`; a broken emit would give `b ≈ 20/partitions`) | give the reduction engine a *set* of slots (partials per slot per partition) instead of one `ReducePtr` |

## Where hardcoding still exists (and is not the solution path)

- `SGPL_FRONTIER_BLOCKLIST_GUARD=1` — the legacy shape blocklist, kept only for
  A/B.  The default decision is the totality prover.
- The front-end recognisers in `IRGenVisitor.cpp` (`detectPeelKFrontierLoop`,
  first-wins frontier loops, synthesized frontier arrays) are shape/name based and
  run *before* the effect algebra; the frontier forms of `kcore`/`bfs_level` are
  served by that path (`autograph_edgemap`) rather than by CleanCut.
- The syntactic `sgpl.frontier.nested.sequential` marker still vetoes
  reconstruction (P9); `SGPL_NO_FRONTIER_MARKER=1` is the A/B switch.
- `nested_while2` is classified `source-owner` and its **emit fails**, so it is
  sequential by fallback — the last verdict-by-accident; the census prints
  `emit failed for class=source-owner -> stays sequential`.

## How to reproduce every claim above

```bash
cd verify
./run.sh                     # 81 checks, 0 failures on this branch
./shape_fuzz.sh              # loop shapes x threads x forced DOALL/DOACROSS
./class_census.sh            # per-case verdict table (class=, red=, shadow=, fw=, data=)
./class_census.sh parallel   # one subtree
SGPL_COMP_I_SOURCE_REDUCTION=1 ./class_census.sh parallel/int_gather.graph
SGPL_COMP_F_ALLOW_DRIVER_CLAIM=1 ./class_census.sh parallel/claim_driver.graph
```

`run.sh` rebuilds the compiler first when a source or header is newer than
`GraphProgram`, so a green run cannot come from a stale binary.
