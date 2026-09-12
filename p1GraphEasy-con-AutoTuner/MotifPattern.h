// Compile-time analysis of a DSL motif pattern.
//
// The pattern in `motif { a->b; b->c; a->c; }` is fully known when the compiler
// runs, but runtime.c treats it as data: it marshals the edges into arrays,
// rebuilds a `required[]` matrix at run time, and re-consults that matrix for
// every candidate vertex at every level. Everything in this file is the part of
// that work that can be done once, at compile time, so the emitted IR carries
// constants instead of an interpreter.
//
// Deliberately free of LLVM dependencies so it can be unit-tested standalone
// against a transcription of the runtime semantics (see MotifPatternTest.cpp).

#ifndef MOTIF_PATTERN_H
#define MOTIF_PATTERN_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "ASTNode.h" // MotifEdgeSpec, MotifEdgeSign

// Hard ceiling inherited from runtime.c: motif_assignment_is_canonical_runtime
// declares permutation[6]/used[6], and SemanticAnalyzer rejects var counts above
// this before we ever get here.
static constexpr int kMotifMaxVars = 6;

// The three values a `required` entry can take, matching runtime.c exactly.
// Absent is not "don't care": motif matching is INDUCED, so an Absent entry
// means the edge must not be present in either sign.
enum class ReqKind : int8_t
{
    Absent = 0,
    Positive = 1,
    Negative = -1,
};

// Where the candidates for one nest level come from.
enum class LevelSource
{
    FullScan, // 0..n-1; the only option for level 0 and for disconnected motifs
    OutRow,   // out-neighbours of an already-bound vertex
    InRow,    // in-neighbours of an already-bound vertex
};

// One already-bound variable that a candidate must be checked against.
struct LevelCheck
{
    int prev;         // index of the already-bound variable
    ReqKind forward;  // required[level][prev]: kind(candidate, A[prev])
    ReqKind backward; // required[prev][level]: kind(A[prev], candidate)
};

struct LevelPlan
{
    LevelSource source = LevelSource::FullScan;
    int driverVar = -1; // which bound variable's row drives enumeration (-1 for FullScan)

    // Every prev < level, with the two kinds the candidate must satisfy. The
    // driving edge is still listed; the emitter may skip re-probing it.
    std::vector<LevelCheck> checks;

    // Canonicality, reduced to lower bounds (see MotifPattern::canonLowerBound).
    // Candidate must be strictly greater than A[p] for every p listed here.
    std::vector<int> lowerBounds;
};

class MotifPattern
{
public:
    // Mirrors the required-matrix construction in runtime.c:1115-1121 exactly,
    // including "self edges ignored" and "a later edge overwrites an earlier one".
    static MotifPattern build(const std::vector<MotifEdgeSpec> &edges,
                              const std::vector<std::string> &varNames);

    int varCount() const { return varCount_; }
    const std::vector<std::string> &varNames() const { return varNames_; }

    ReqKind required(int src, int dst) const { return required_[src][dst]; }

    // All permutations preserving the required matrix, identity first.
    const std::vector<std::array<int, kMotifMaxVars>> &automorphisms() const
    {
        return automorphisms_;
    }

    const LevelPlan &level(int i) const { return levels_[i]; }

    // True when some edge is Negative. When false the emitted probe never needs
    // to look at the weights array for a "must be present" check on an
    // unweighted graph.
    bool needsWeights() const;

    // True when any level enumerates from an in-row, i.e. the emitted prologue
    // must build a private transpose.
    bool needsInCsr() const;

    // Canonicality, the honest O(k!) version transcribed from
    // motif_assignment_is_canonical_runtime. Reference only; used by the tests
    // and never by the emitter.
    bool isCanonicalReference(const int32_t *assignment) const;

    // Canonicality via the reduction the emitter actually uses: for every
    // non-identity automorphism pi, with p = min{i : pi(i) != i}, the whole
    // lexicographic comparison collapses to `assignment[p] < assignment[pi(p)]`.
    //
    // Why this is exact: bindings are injective (the emitted nest enforces
    // pairwise distinctness), so assignment[pi(i)] != assignment[i] exactly when
    // pi(i) != i. The first index where the permuted vector differs is therefore
    // p, and "permuted is lexicographically smaller" reduces to a single compare
    // at p. Also p < pi(p) always: if pi(p) = q < p then q is unmoved, so
    // pi(q) = q = pi(p), contradicting injectivity of pi. That ordering is what
    // makes each constraint expressible as a lower bound on a later level.
    //
    // ONLY valid for injective assignments. isCanonicalReference has no such
    // precondition.
    bool isCanonicalFast(const int32_t *assignment) const;

    std::string describe() const; // debugging aid

private:
    int varCount_ = 0;
    std::vector<std::string> varNames_;
    std::array<std::array<ReqKind, kMotifMaxVars>, kMotifMaxVars> required_{};
    std::vector<std::array<int, kMotifMaxVars>> automorphisms_;
    std::array<LevelPlan, kMotifMaxVars> levels_{};

    void computeAutomorphisms();
    void computeCanonLowerBounds();
    void computeLevelPlans();
    bool permutationPreserves(const std::array<int, kMotifMaxVars> &perm) const;
};

#endif // MOTIF_PATTERN_H
