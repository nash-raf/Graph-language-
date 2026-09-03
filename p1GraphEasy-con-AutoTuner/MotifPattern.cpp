#include "MotifPattern.h"

#include <algorithm>
#include <numeric>
#include <sstream>
#include <unordered_map>

namespace
{

const char *reqName(ReqKind k)
{
    switch (k)
    {
    case ReqKind::Positive: return "->";
    case ReqKind::Negative: return "-|";
    default: return "..";
    }
}

} // namespace

MotifPattern MotifPattern::build(const std::vector<MotifEdgeSpec> &edges,
                                 const std::vector<std::string> &varNames)
{
    MotifPattern p;
    p.varCount_ = static_cast<int>(varNames.size());
    p.varNames_ = varNames;

    for (auto &row : p.required_)
        row.fill(ReqKind::Absent);

    std::unordered_map<std::string, int> index;
    for (int i = 0; i < p.varCount_; ++i)
        index[varNames[i]] = i;

    // runtime.c:1115-1121. Self edges are ignored rather than rejected, and a
    // later edge overwrites an earlier one for the same ordered pair. Both are
    // load-bearing: SemanticAnalyzer already rejects them, but the emitted code
    // must agree with the runtime for any input that slips through.
    for (const MotifEdgeSpec &e : edges)
    {
        auto srcIt = index.find(e.source);
        auto dstIt = index.find(e.target);
        if (srcIt == index.end() || dstIt == index.end())
            continue;
        int src = srcIt->second;
        int dst = dstIt->second;
        if (src == dst)
            continue;
        p.required_[src][dst] =
            (e.sign == MotifEdgeSign::Negative) ? ReqKind::Negative : ReqKind::Positive;
    }

    p.computeAutomorphisms();
    p.computeCanonLowerBounds();
    p.computeLevelPlans();
    return p;
}

bool MotifPattern::permutationPreserves(const std::array<int, kMotifMaxVars> &perm) const
{
    // motif_permutation_preserves_runtime: the full matrix must be preserved,
    // including Absent entries and edge signs. So `->` and `-|` are not
    // interchangeable under an automorphism, which is why an incoherent FFL has
    // a trivial automorphism group while a coherent one may not.
    for (int i = 0; i < varCount_; ++i)
        for (int j = 0; j < varCount_; ++j)
            if (required_[i][j] != required_[perm[i]][perm[j]])
                return false;
    return true;
}

void MotifPattern::computeAutomorphisms()
{
    automorphisms_.clear();

    std::array<int, kMotifMaxVars> perm{};
    std::vector<int> order(varCount_);
    std::iota(order.begin(), order.end(), 0);

    // At most 6! = 720 permutations, so brute force is free at compile time.
    do
    {
        for (int i = 0; i < varCount_; ++i)
            perm[i] = order[i];
        if (permutationPreserves(perm))
            automorphisms_.push_back(perm);
    } while (std::next_permutation(order.begin(), order.end()));

    // Identity sorts first out of next_permutation on an ascending seed, so it
    // is already at index 0; assert-by-construction rather than re-sorting.
}

void MotifPattern::computeCanonLowerBounds()
{
    for (auto &lvl : levels_)
        lvl.lowerBounds.clear();

    for (const auto &perm : automorphisms_)
    {
        // Skip the identity.
        int p = -1;
        for (int i = 0; i < varCount_; ++i)
        {
            if (perm[i] != i)
            {
                p = i;
                break;
            }
        }
        if (p < 0)
            continue;

        // p < perm[p] is guaranteed (see the note in MotifPattern.h), so the
        // constraint A[p] < A[perm[p]] is a lower bound on the later level.
        int j = perm[p];
        auto &bounds = levels_[j].lowerBounds;
        if (std::find(bounds.begin(), bounds.end(), p) == bounds.end())
            bounds.push_back(p);
    }

    for (auto &lvl : levels_)
        std::sort(lvl.lowerBounds.begin(), lvl.lowerBounds.end());
}

void MotifPattern::computeLevelPlans()
{
    for (int j = 0; j < varCount_; ++j)
    {
        LevelPlan &plan = levels_[j];
        plan.checks.clear();
        plan.source = LevelSource::FullScan;
        plan.driverVar = -1;

        for (int prev = 0; prev < j; ++prev)
            plan.checks.push_back(LevelCheck{prev, required_[j][prev], required_[prev][j]});

        // Pick a driver. Any present edge works, including a Negative one: the
        // edge exists in the CSR either way, only its weight sign differs.
        //
        // Prefer the most recently bound variable; it is the most constrained,
        // so its row tends to be the one already in cache.
        for (int prev = j - 1; prev >= 0; --prev)
        {
            if (required_[prev][j] != ReqKind::Absent)
            {
                // prev -> j, so the candidate is an out-neighbour of A[prev].
                plan.source = LevelSource::OutRow;
                plan.driverVar = prev;
                break;
            }
        }
        if (plan.source == LevelSource::FullScan)
        {
            for (int prev = j - 1; prev >= 0; --prev)
            {
                if (required_[j][prev] != ReqKind::Absent)
                {
                    // j -> prev, so the candidate is an in-neighbour of A[prev].
                    plan.source = LevelSource::InRow;
                    plan.driverVar = prev;
                    break;
                }
            }
        }
    }
}

bool MotifPattern::needsWeights() const
{
    for (int i = 0; i < varCount_; ++i)
        for (int j = 0; j < varCount_; ++j)
            if (required_[i][j] == ReqKind::Negative)
                return true;
    return false;
}

bool MotifPattern::needsInCsr() const
{
    for (int j = 0; j < varCount_; ++j)
        if (levels_[j].source == LevelSource::InRow)
            return true;
    return false;
}

bool MotifPattern::isCanonicalReference(const int32_t *assignment) const
{
    // Direct transcription of motif_has_smaller_automorphism_runtime +
    // motif_permutation_is_smaller_runtime (runtime.c:1220-1266). Enumerates
    // every permutation, not just the automorphisms, exactly as the runtime does.
    std::array<int, kMotifMaxVars> perm{};
    std::vector<int> order(varCount_);
    std::iota(order.begin(), order.end(), 0);

    do
    {
        for (int i = 0; i < varCount_; ++i)
            perm[i] = order[i];
        if (!permutationPreserves(perm))
            continue;

        for (int i = 0; i < varCount_; ++i)
        {
            int32_t permuted = assignment[perm[i]];
            if (permuted < assignment[i])
                return false; // a smaller automorphic image exists
            if (permuted > assignment[i])
                break;
        }
    } while (std::next_permutation(order.begin(), order.end()));

    return true;
}

bool MotifPattern::isCanonicalFast(const int32_t *assignment) const
{
    for (int j = 0; j < varCount_; ++j)
        for (int p : levels_[j].lowerBounds)
            if (!(assignment[p] < assignment[j]))
                return false;
    return true;
}

std::string MotifPattern::describe() const
{
    std::ostringstream os;
    os << "motif k=" << varCount_ << " vars=[";
    for (int i = 0; i < varCount_; ++i)
        os << (i ? "," : "") << varNames_[i];
    os << "]\n  required:\n";
    for (int i = 0; i < varCount_; ++i)
    {
        os << "    ";
        for (int j = 0; j < varCount_; ++j)
            os << reqName(required_[i][j]) << ' ';
        os << '\n';
    }
    os << "  |Aut| = " << automorphisms_.size() << '\n';
    for (int j = 0; j < varCount_; ++j)
    {
        const LevelPlan &lvl = levels_[j];
        os << "  level " << j << ": ";
        switch (lvl.source)
        {
        case LevelSource::FullScan: os << "FullScan"; break;
        case LevelSource::OutRow: os << "OutRow(A[" << lvl.driverVar << "])"; break;
        case LevelSource::InRow: os << "InRow(A[" << lvl.driverVar << "])"; break;
        }
        if (!lvl.lowerBounds.empty())
        {
            os << "  bounds:";
            for (int p : lvl.lowerBounds)
                os << " cand>A[" << p << ']';
        }
        os << '\n';
    }
    return os.str();
}
