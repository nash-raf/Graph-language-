------------------------ MODULE TDGTaskSafetyProof ------------------------
EXTENDS TDGTaskSafety, TLAPS

THEOREM SummarySoundness ==
  \A a, b \in Tasks:
    (CertificateSound /\ SummaryIndependent(a, b))
      => ActualIndependent(a, b)
PROOF BY DEF CertificateSound, SummaryIndependent, ActualIndependent

InductiveSafety ==
  /\ CertificateSound
  /\ waiting \subseteq Tasks
  /\ running \subseteq Tasks
  /\ PairwiseSafe(running)

THEOREM NoConflictingConcurrentTasks ==
  ASSUME EnforcePairwise = TRUE
  PROVE Spec => []Safety
PROOF
  <1>1. Init => InductiveSafety
    BY DEF Init, InductiveSafety, CertificateSound, PairwiseSafe
  <1>2. InductiveSafety /\ Next => InductiveSafety'
    BY DEF InductiveSafety, Next, Start, Finish, CanStart,
           PairwiseSafe, CertificateSound, SummaryIndependent
  <1>3. InductiveSafety => Safety
    BY SummarySoundness DEF InductiveSafety, PairwiseSafe, Safety
  <1>4. InductiveSafety /\ UNCHANGED vars => InductiveSafety'
    BY DEF InductiveSafety, vars, CertificateSound, PairwiseSafe, SummaryIndependent
  <1>5. QED
    BY PTL, <1>1, <1>2, <1>3, <1>4 DEF Spec

InductiveOrder ==
  /\ CertificateSound
  /\ waiting \subseteq Tasks
  /\ running \subseteq Tasks
  /\ done \subseteq Tasks
  /\ OrderSafety

THEOREM NoPrematureTaskStart ==
  ASSUME EnforcePairwise = TRUE
  PROVE Spec => []OrderSafety
PROOF
  <1>1. Init => InductiveOrder
    BY DEF Init, InductiveOrder, OrderSafety, CertificateSound
  <1>2. InductiveOrder /\ Next => InductiveOrder'
    BY DEF InductiveOrder, Next, Start, Finish, CanStart,
           OrderSafety, CertificateSound
  <1>3. InductiveOrder => OrderSafety
    BY DEF InductiveOrder
  <1>4. InductiveOrder /\ UNCHANGED vars => InductiveOrder'
    BY DEF InductiveOrder, vars, CertificateSound, OrderSafety
  <1>5. QED
    BY PTL, <1>1, <1>2, <1>3, <1>4 DEF Spec

=============================================================================
