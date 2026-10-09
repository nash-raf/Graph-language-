------------------------- MODULE PDGEdgeCoverage -------------------------
EXTENDS TLAPS

\* An arbitrary ordered pair of dynamic events projected to two static LLVM
\* instructions.  Boolean constants describe that pair.  The theorem is
\* universally applicable because the labels and events are unrestricted.
CONSTANTS SameInst, StoreA, StoreB, BarrierA, BarrierB,
          ActualBefore, ActualAlias, ActualControl, ActualSSA,
          MayReach, PotentialAlias, MayControl, DefUse, InLoop

ASSUME /\ SameInst \in BOOLEAN
       /\ StoreA \in BOOLEAN /\ StoreB \in BOOLEAN
       /\ BarrierA \in BOOLEAN /\ BarrierB \in BOOLEAN
       /\ ActualBefore \in BOOLEAN /\ ActualAlias \in BOOLEAN
       /\ ActualControl \in BOOLEAN /\ ActualSSA \in BOOLEAN
       /\ MayReach \in BOOLEAN /\ PotentialAlias \in BOOLEAN
       /\ MayControl \in BOOLEAN /\ DefUse \in BOOLEAN
       /\ InLoop \in BOOLEAN

\* The first two obligations map to LLVM's isPotentiallyReachable and to
\* AA's NoAlias contract, with common-loop pairs admitted unconditionally.
\* A repeated static instruction in one invocation must be in a loop.
CoverageAssumptions ==
  /\ ActualBefore => MayReach
  /\ ActualAlias => PotentialAlias
  /\ (ActualBefore /\ SameInst) => InLoop
  /\ ActualControl => MayControl
  /\ ActualSSA => DefUse
  /\ SameInst => (StoreA = StoreB /\ BarrierA = BarrierB)

ActualMemoryDependence ==
  ActualBefore /\ ActualAlias /\ (StoreA \/ StoreB)
ActualBarrierDependence ==
  ActualBefore /\ (BarrierA \/ BarrierB)
ActualDependence ==
  ActualMemoryDependence \/ ActualBarrierDependence \/
  ActualControl \/ ActualSSA

\* Mirrors MEM_MAY, WAW_LOOP_MAY, MEM_BARRIER_*, CTRL_MAY and RAW edges.
MemoryEdge ==
  IF SameInst THEN InLoop /\ StoreA
  ELSE MayReach /\ PotentialAlias /\ (StoreA \/ StoreB)
BarrierEdge ==
  IF SameInst THEN InLoop /\ BarrierA
  ELSE MayReach /\ (BarrierA \/ BarrierB)
PDGEdge == MemoryEdge \/ BarrierEdge \/ MayControl \/ DefUse

THEOREM PairwiseEdgeCoverage ==
  CoverageAssumptions => (ActualDependence => PDGEdge)
PROOF BY DEF CoverageAssumptions, ActualDependence,
             ActualMemoryDependence, ActualBarrierDependence,
             PDGEdge, MemoryEdge, BarrierEdge

=============================================================================
