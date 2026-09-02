// Stage-1 gate for the motif IR lowering.
//
// The emitted nest will not run the runtime's O(k!) canonicality search. It
// will instead apply MotifPattern's reduction: a set of per-level lower bounds
// on candidate values. If that reduction is wrong for even one automorphism
// group, every count for that motif is silently wrong -- and counts alone can't
// reveal it, because picking the wrong orbit representative yields the same
// count as picking the right one.
//
// So: prove the reduction agrees with a literal transcription of
// motif_assignment_is_canonical_runtime, exhaustively, before any IR exists.
//
// Build:
//   g++ -std=c++17 -O2 -I. -I/usr/local/llvm-20-polly-rtti/include \
//       MotifPatternTest.cpp MotifPattern.cpp -o motif_pattern_test
// Run:
//   ./motif_pattern_test

#include "MotifPattern.h"

#include <cstdio>
#include <cstring>
#include <random>
#include <set>
#include <vector>

namespace
{

int g_failures = 0;
long g_patternsChecked = 0;
long g_assignmentsChecked = 0;

std::vector<std::string> makeVarNames(int k)
{
    std::vector<std::string> names;
    for (int i = 0; i < k; ++i)
        names.push_back(std::string(1, static_cast<char>('a' + i)));
    return names;
}

// Build a pattern directly from a required-matrix spec, bypassing edge parsing,
// so the test can sweep matrices the DSL grammar could not express in one go.
MotifPattern patternFromMatrix(int k, const std::vector<ReqKind> &flat)
{
    std::vector<MotifEdgeSpec> edges;
    std::vector<std::string> names = makeVarNames(k);
    for (int i = 0; i < k; ++i)
    {
        for (int j = 0; j < k; ++j)
        {
            if (i == j)
                continue;
            ReqKind kind = flat[i * k + j];
            if (kind == ReqKind::Absent)
                continue;
            MotifEdgeSpec e;
            e.source = names[i];
            e.target = names[j];
            e.sign = (kind == ReqKind::Negative) ? MotifEdgeSign::Negative
                                                 : MotifEdgeSign::Positive;
            edges.push_back(e);
        }
    }
    return MotifPattern::build(edges, names);
}

// Every injective assignment of k distinct vertices drawn from 0..vertexPool-1.
void forEachInjective(int k, int vertexPool,
                      const std::function<void(const int32_t *)> &fn)
{
    std::vector<int32_t> a(k);
    std::vector<bool> used(vertexPool, false);

    std::function<void(int)> rec = [&](int pos) {
        if (pos == k)
        {
            fn(a.data());
            return;
        }
        for (int v = 0; v < vertexPool; ++v)
        {
            if (used[v])
                continue;
            used[v] = true;
            a[pos] = v;
            rec(pos + 1);
            used[v] = false;
        }
    };
    rec(0);
}

void checkPattern(const MotifPattern &p, int vertexPool)
{
    ++g_patternsChecked;
    const int k = p.varCount();

    // 1. The reduction must agree with the runtime transcription on every
    //    injective assignment.
    forEachInjective(k, vertexPool, [&](const int32_t *a) {
        ++g_assignmentsChecked;
        bool ref = p.isCanonicalReference(a);
        bool fast = p.isCanonicalFast(a);
        if (ref != fast && g_failures < 20)
        {
            ++g_failures;
            std::printf("MISMATCH ref=%d fast=%d  assignment=[", (int)ref, (int)fast);
            for (int i = 0; i < k; ++i)
                std::printf("%s%d", i ? "," : "", a[i]);
            std::printf("]\n%s\n", p.describe().c_str());
        }
        else if (ref != fast)
        {
            ++g_failures;
        }
    });

    // 2. Orbit property: over any set of k distinct vertices, the automorphism
    //    group acts on the k! orderings and exactly one ordering per orbit must
    //    be canonical. This is the property the dedup actually relies on -- it
    //    is what makes the emitted count equal the orbit count.
    std::vector<int32_t> base(k);
    for (int i = 0; i < k; ++i)
        base[i] = i * 3 + 1; // arbitrary distinct values, not 0..k-1

    std::vector<int> order(k);
    for (int i = 0; i < k; ++i)
        order[i] = i;

    std::set<std::vector<int32_t>> seenOrbitReps;
    long canonicalCount = 0;
    long total = 0;
    do
    {
        std::vector<int32_t> a(k);
        for (int i = 0; i < k; ++i)
            a[i] = base[order[i]];
        ++total;
        if (p.isCanonicalFast(a.data()))
        {
            ++canonicalCount;
            // canonical representative of this orbit: the lex-min image
            std::vector<int32_t> best = a;
            for (const auto &perm : p.automorphisms())
            {
                std::vector<int32_t> img(k);
                for (int i = 0; i < k; ++i)
                    img[i] = a[perm[i]];
                if (img < best)
                    best = img;
            }
            if (!seenOrbitReps.insert(best).second && g_failures < 20)
            {
                ++g_failures;
                std::printf("TWO CANONICAL IN ONE ORBIT\n%s\n", p.describe().c_str());
            }
        }
    } while (std::next_permutation(order.begin(), order.end()));

    long autSize = static_cast<long>(p.automorphisms().size());
    if (canonicalCount * autSize != total)
    {
        if (g_failures < 20)
        {
            ++g_failures;
            std::printf("ORBIT COUNT WRONG: canonical=%ld * |Aut|=%ld != k!=%ld\n%s\n",
                        canonicalCount, autSize, total, p.describe().c_str());
        }
        else
        {
            ++g_failures;
        }
    }
}

void sweepExhaustive(int k, int vertexPool)
{
    const int offDiag = k * k - k;
    long combos = 1;
    for (int i = 0; i < offDiag; ++i)
        combos *= 3;

    std::vector<ReqKind> flat(k * k, ReqKind::Absent);
    for (long mask = 0; mask < combos; ++mask)
    {
        long m = mask;
        for (int i = 0; i < k; ++i)
        {
            for (int j = 0; j < k; ++j)
            {
                if (i == j)
                {
                    flat[i * k + j] = ReqKind::Absent;
                    continue;
                }
                int digit = static_cast<int>(m % 3);
                m /= 3;
                flat[i * k + j] = digit == 0   ? ReqKind::Absent
                                  : digit == 1 ? ReqKind::Positive
                                               : ReqKind::Negative;
            }
        }
        checkPattern(patternFromMatrix(k, flat), vertexPool);
    }
    std::printf("  k=%d exhaustive: %ld patterns\n", k, combos);
}

void sweepRandom(int k, int vertexPool, int patterns, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> pick(0, 2);
    std::vector<ReqKind> flat(k * k, ReqKind::Absent);
    for (int t = 0; t < patterns; ++t)
    {
        for (int i = 0; i < k; ++i)
            for (int j = 0; j < k; ++j)
            {
                if (i == j)
                {
                    flat[i * k + j] = ReqKind::Absent;
                    continue;
                }
                int digit = pick(rng);
                flat[i * k + j] = digit == 0   ? ReqKind::Absent
                                  : digit == 1 ? ReqKind::Positive
                                               : ReqKind::Negative;
            }
        checkPattern(patternFromMatrix(k, flat), vertexPool);
    }
    std::printf("  k=%d random: %d patterns\n", k, patterns);
}

MotifPattern namedMotif(const char *label,
                        std::vector<std::pair<std::string, std::string>> arcs,
                        std::vector<bool> negative = {})
{
    std::vector<MotifEdgeSpec> edges;
    std::vector<std::string> names;
    std::set<std::string> seen;
    for (size_t i = 0; i < arcs.size(); ++i)
    {
        MotifEdgeSpec e;
        e.source = arcs[i].first;
        e.target = arcs[i].second;
        e.sign = (i < negative.size() && negative[i]) ? MotifEdgeSign::Negative
                                                      : MotifEdgeSign::Positive;
        // first-appearance order, source before target, matching ASTBuilder.cpp:283-291
        if (seen.insert(e.source).second)
            names.push_back(e.source);
        if (seen.insert(e.target).second)
            names.push_back(e.target);
        edges.push_back(e);
    }
    MotifPattern p = MotifPattern::build(edges, names);
    std::printf("\n=== %s ===\n%s", label, p.describe().c_str());
    return p;
}

} // namespace

int main()
{
    std::printf("== Real DSL motifs (compare against the plan's table) ==\n");
    namedMotif("feed-forward loop", {{"s", "i"}, {"s", "t"}, {"i", "t"}});
    namedMotif("three-chain", {{"s", "i"}, {"i", "t"}});
    namedMotif("bi-fan", {{"s1", "t1"}, {"s1", "t2"}, {"s2", "t1"}, {"s2", "t2"}});
    namedMotif("bi-parallel", {{"s", "i1"}, {"s", "i2"}, {"i1", "t"}, {"i2", "t"}});
    namedMotif("three-node feedback loop", {{"a", "b"}, {"b", "c"}, {"c", "a"}});
    namedMotif("four-node feedback loop",
               {{"a", "b"}, {"b", "c"}, {"c", "d"}, {"d", "a"}});
    namedMotif("incoherent FFL (signed)", {{"a", "b"}, {"b", "c"}, {"a", "c"}},
               {false, true, false});

    std::printf("\n== Exhaustive / randomized equivalence sweep ==\n");
    std::mt19937 rng(20260816);
    sweepExhaustive(2, 4);
    sweepExhaustive(3, 5);
    sweepRandom(4, 6, 20000, rng);
    sweepRandom(5, 6, 4000, rng);
    sweepRandom(6, 7, 600, rng);

    std::printf("\npatterns=%ld assignments=%ld failures=%d\n", g_patternsChecked,
                g_assignmentsChecked, g_failures);
    if (g_failures)
    {
        std::printf("FAILED\n");
        return 1;
    }
    std::printf("ALL PASS: fast reduction == runtime O(k!) search on every injective "
                "assignment, and orbits have exactly one canonical representative\n");
    return 0;
}
