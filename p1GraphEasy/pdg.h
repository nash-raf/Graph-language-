#ifndef PDG_H
#define PDG_H

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include <string>

namespace llvm
{
    struct depEdge
    {
        unsigned source, target;
        std::string type;
        bool isLoop = false;
    };

    struct dependencyGraph
    {
        DenseMap<Instruction *, int> nodes;
        SmallVector<depEdge> edges;
        Instruction *memoryBarrier = nullptr;
        SmallVector<Instruction *> controlBarriers;
    };

    // Structure to represent a partition/component in the task graph
    struct TaskNode
    {
        unsigned taskId;
        SmallVector<unsigned> vertices; // Original PDG vertices in this component
        bool isCutVertex = false;       // True if this represents a single cut vertex
        unsigned originalVertex = 0;    // If isCutVertex, the original PDG vertex ID
    };

    // Task graph after min-cut partitioning
    struct TaskGraph
    {
        SmallVector<TaskNode> tasks;
        SmallVector<depEdge> taskEdges;              // Edges between tasks
        DenseMap<unsigned, unsigned> vertexToTask;   // Maps original vertex to task ID
        SmallVector<unsigned> cutVertices;           // IDs of cut vertices in original graph
        DenseMap<unsigned, unsigned> cutVertexOrder; // Execution order of cut vertices
    };

    dependencyGraph runPDGOnModule(Module &M);

    /// Convenience wrapper: run PDG and also write DOT to dotFilename.
    dependencyGraph runPDGOnModuleAndWriteDot(Module &M);

    void addMemoryBarrier(dependencyGraph &G, Module &M);
    void addControlBarrier(dependencyGraph &G, Module &M);

    // Old function - now deprecated
    SmallVector<SmallVector<unsigned>> topologicalSortByLevels(const dependencyGraph &G);

    // New functions for min-cut and task graph generation
    TaskGraph performMinCutAndCreateTaskGraph(const dependencyGraph &G);
    SmallVector<SmallVector<unsigned>> topologicalSortTaskGraph(const TaskGraph &TG);

    // Visualization
    // void writeTaskGraphDot(const TaskGraph &TG, const dependencyGraph &G,
    //                        const std::string &filename = "task_graph.dot");

    // Parallel IR reconstruction
    void reconstructParallelIR(Module &M,
                               const dependencyGraph &PDG,
                               const TaskGraph &TG,
                               const SmallVector<SmallVector<unsigned>> &levels);

}

#endif