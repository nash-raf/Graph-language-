# R2 rejection — occurrence preservation for per-source claims

Status: **reconstructed**.  The plan referenced an untracked working note
(`proof/R2_REJECTION.md`) that is not present in the repository, the local tree,
or the pod filesystem (searched with `find / -xdev -name R2_REJECTION.md`); this
file restates the semantics from the live code so the rejection has a written
proof obligation.  It is assistant-authored, not the original note.

## Semantic condition

R2 is the temporal rejection for a **per-source claim**: a check-then-set in the
driver preamble whose result gates the neighbour body.  Producer:
`graph_frontier_lowering.cpp` — `hasPerSourceClaim(Info)` records

    R2 temporal: per-source claim (occurrence preservation)

with `DischargeKind::ClaimStaging` when the claim can be staged into the
runtime's per-source channel (`perSourceClaims(Info)` non-empty, realized as
SOURCE_BEGIN operations writing `S[u][j]`), and `DischargeKind::None` otherwise.
`R_T` is set only for the undischarged form (`refresh()`).

## Why an undischarged claim refuses

In the serial program the claim runs **once per source vertex** and its guard
decides whether that source's neighbour body runs at all.  Every emitted engine
work function is called once per `(u,v)` pair, so neither the claim nor its
guard is reproduced there; a naive graft would evaluate the claim per pair
(wrong occurrence count) or drop the guard (wrong coverage).  The requirement is
therefore an *ordering/occurrence* obligation, not a data-placement one:

    claim(u) executes exactly once per source u, with the serial program's
    tie-breaking, and the pair phase observes its result.

## Discharge and realization

- **Staged claims** (`perSourceClaims` non-empty) are discharged
  (`ClaimStaging`): the claim becomes a SOURCE_BEGIN operation on the shared
  per-source channel, and the owner's source-domain coverage runs before either
  child under dual ownership, so `claim(u) ≺ pairs(u)` holds in both domains.
- **Undischarged claims** have no V1 realization.  `selectSchedule` reports
  `temporal witness R2 has no preserving realization (V1)`; the emission fails
  closed and the nest carries `sgpl.frontier.impl.serial`.  This is an
  *implementation* failure, not a new semantic rejection: the claim itself is
  sound, the engine cannot reproduce it.

## Required proofs to lift the refusal (one of)

1. Prove the driver visits each source exactly once, or
2. prove the claim idempotent and its result unused, or
3. hoist the claim into a per-source pre-pass outside the pair loop.

Until one of these lands, `verify/cases/parallel/claim_driver.graph` stays
`class=sequential` (pinned by `verify/run.sh`, "race/claim_driver"): the serial
semantics are `outsum 5 claim_left 0`.

## Live evidence (2026-10-09, two-axis gate split)

Certificate for `claim_driver`:

    [frontier-cert]   #1 R2 temporal ... discharge=claim-staging
    [graph-frontier]   expression path refused -> stays sequential
    [graph-frontier] candidate: ... class=sequential ... [refused: emit failed]

i.e. the staged claim is discharged on the temporal axis; the refusal is the
engine's emit path and is now reported as an implementation failure
(`impl_failure=1`, reason `emit failed`) with `sgpl.frontier.impl.serial`
instead of a semantic R1-R7 string.
