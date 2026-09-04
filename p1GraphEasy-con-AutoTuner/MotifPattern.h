#ifndef MOTIFPATTERN_H
#define MOTIFPATTERN_H

#include <string>
#include <vector>

#include "ASTNode.h"

// Build-time stub: the real MotifPattern.cpp was never committed to git and is
// missing from the tree, so this restores buildability. The benchmark algos
// (bfs/cc/kcore/pagerank/sssp) contain no `motifs` declarations, so the motif
// IR path is never exercised; emitCountOnly() returns 0.
class MotifPattern
{
public:
    static MotifPattern build(const std::vector<MotifEdgeSpec> &edges,
                              const std::vector<std::string> &varNames)
    {
        MotifPattern p;
        p.edges = edges;
        p.varNames = varNames;
        return p;
    }

    std::vector<MotifEdgeSpec> edges;
    std::vector<std::string> varNames;
};

#endif // MOTIFPATTERN_H