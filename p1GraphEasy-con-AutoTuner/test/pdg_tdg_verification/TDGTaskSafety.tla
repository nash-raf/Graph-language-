--------------------------- MODULE TDGTaskSafety ---------------------------
EXTENDS Naturals, FiniteSets

CONSTANTS Tasks, Objects, EnforcePairwise

VARIABLES summaryRead, summaryWrite, actualRead, actualWrite,
          opaque, declaredBefore, actualBefore,
          waiting, running, done

vars == <<summaryRead, summaryWrite, actualRead, actualWrite,
          opaque, declaredBefore, actualBefore, waiting, running, done>>

TypeOK ==
  /\ summaryRead \in [Tasks -> SUBSET Objects]
  /\ summaryWrite \in [Tasks -> SUBSET Objects]
  /\ actualRead \in [Tasks -> SUBSET Objects]
  /\ actualWrite \in [Tasks -> SUBSET Objects]
  /\ opaque \in SUBSET Tasks
  /\ declaredBefore \in [Tasks -> SUBSET Tasks]
  /\ actualBefore \in [Tasks -> SUBSET Tasks]
  /\ waiting \subseteq Tasks
  /\ running \subseteq Tasks
  /\ done \subseteq Tasks

(* This is the contract of the compiler's whole-task effect certificate and
   its non-memory dependency analysis.  This model verifies scheduling
   decisions under that contract; it does not establish the contract. *)
CertificateSound ==
  /\ \A t \in Tasks:
       actualRead[t] \subseteq summaryRead[t]
       /\ actualWrite[t] \subseteq summaryWrite[t]
       /\ actualBefore[t] \subseteq declaredBefore[t]
  /\ \A t \in Tasks: t \notin declaredBefore[t]

SummaryIndependent(a, b) ==
  /\ a \notin opaque /\ b \notin opaque
  /\ (summaryWrite[a] \cap (summaryRead[b] \cup summaryWrite[b])) = {}
  /\ (summaryWrite[b] \cap (summaryRead[a] \cup summaryWrite[a])) = {}

ActualIndependent(a, b) ==
  /\ a \notin opaque /\ b \notin opaque
  /\ (actualWrite[a] \cap (actualRead[b] \cup actualWrite[b])) = {}
  /\ (actualWrite[b] \cap (actualRead[a] \cup actualWrite[a])) = {}

Init ==
  /\ summaryRead \in [Tasks -> SUBSET Objects]
  /\ summaryWrite \in [Tasks -> SUBSET Objects]
  /\ actualRead \in [Tasks -> SUBSET Objects]
  /\ actualWrite \in [Tasks -> SUBSET Objects]
  /\ opaque \in SUBSET Tasks
  /\ declaredBefore \in [Tasks -> SUBSET Tasks]
  /\ actualBefore \in [Tasks -> SUBSET Tasks]
  /\ CertificateSound
  /\ waiting = Tasks
  /\ running = {}
  /\ done = {}

PairwiseSafe(S) ==
  \A a, b \in S: a = b \/ SummaryIndependent(a, b)

CanStart(S) ==
  /\ S # {}
  /\ S \subseteq waiting
  /\ running = {}
  /\ \A t \in S: declaredBefore[t] \subseteq done
  /\ (EnforcePairwise => PairwiseSafe(S))

Start(S) ==
  /\ CanStart(S)
  /\ waiting' = waiting \ S
  /\ running' = S
  /\ UNCHANGED <<summaryRead, summaryWrite, actualRead, actualWrite,
                 opaque, declaredBefore, actualBefore, done>>

Finish(t) ==
  /\ t \in running
  /\ running' = running \ {t}
  /\ done' = done \cup {t}
  /\ UNCHANGED <<summaryRead, summaryWrite, actualRead, actualWrite,
                 opaque, declaredBefore, actualBefore, waiting>>

Next ==
  \/ \E S \in SUBSET waiting: Start(S)
  \/ \E t \in running: Finish(t)

Spec == Init /\ [][Next]_vars

Safety == \A a, b \in running: a = b \/ ActualIndependent(a, b)
OrderSafety == \A t \in (running \cup done): actualBefore[t] \subseteq done
Partition ==
  /\ waiting \cap running = {}
  /\ waiting \cap done = {}
  /\ running \cap done = {}
  /\ waiting \cup running \cup done = Tasks

=============================================================================
