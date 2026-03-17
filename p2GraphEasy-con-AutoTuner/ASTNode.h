#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <vector>
#include <string>
#include <any>
#include <iostream>
#include <fstream>
#include <cstddef>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <cstring> // for std::memcpy
#include "llvm/Support/Allocator.h"
#include <chrono>
#include <tuple>
#include <unordered_set>
#include <algorithm>
#include "roaring_bitmap.h"


// Define TypeKind enum here to avoid circular dependency issues
// SemanticAnalyzer.h will use this definition
enum class TypeKind
{
    Int,
    Bool,
    Real,
    String,
    IntArray,
    Graph,
    WeightedGraph,
    Set,
    Void,
    Unknown
};

enum class ASTNodeType
{
    Program,
    IntLiteral,
    BoolLiteral,
    RealLiteral,
    StringLiteral,
    Variable,
    VarDecl,
    AssignmentStmt,
    Conditional,
    BinaryExpr,
    ArrayLiteral,
    ArrayAccess,
    FunctionCall,
    FunctionDecl,
    ReturnStmt,
    BlockStmt,
    WhileStmt,
    Param,
    GraphDecl,
    WeightedGraphDecl,
    EdgeList,
    WeightedEdgeList,
    NodeList,
    QueryNode,
    ForEachStmt,
    PrintStmt,
    SleepStmt,
    GraphUpdate,
    ShowGraph,
    GraphComprehension,
    SetDecl,
    SetLiteral,
    SetOperation,
    SetBinaryExpr,
    SetMethodCall,
    GraphMemberSet,
    SetContainsExpr,
    NotExpr,
    BreakStmt,
    ContinueStmt,
    UnaryMinusExpr,
    Array2DAccess,
    SetPopExpr,
    SwapStmt
};

enum class GraphUpdateKind { Add, Remove };
enum class GraphDegreeOp { None, Eq, Ne, Le, Ge, Lt, Gt };
enum class GraphConditionOp { And, Or, Connected, Cycle, Degree };


template <typename T>
T safe_any_cast(const std::any &a,
                const char *where = "safe_any_cast")
{
    try
    {
        return std::any_cast<T>(a);
    }
    catch (const std::bad_any_cast &)
    {
        // Avoid RTTI-dependent diagnostics so we can
        // compile with LLVM's -fno-rtti settings.
        std::cerr << "[bad_any_cast in " << where << "]\n";
        throw;
    }
}

class ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;

class ASTNode
{
public:
    ASTNodeType type;
    explicit ASTNode(ASTNodeType t) : type(t) {}
    virtual ~ASTNode() = default;

    // Add a virtual dump method for AST pretty-printing
    virtual void dump(std::ostream &os, int indent = 0) const
    {
        os << std::string(indent, ' ') << "ASTNode(type=" << static_cast<int>(type) << ")\n";
    }
};

class GraphConditionNode : public ASTNode
{
public:
    GraphConditionOp op;
    int nodeId = -1;  // for Connected
    GraphDegreeOp degreeOp = GraphDegreeOp::None;
    int degreeValue = -1;
    std::shared_ptr<GraphConditionNode> left;   // for And/Or
    std::shared_ptr<GraphConditionNode> right;  // for And/Or

    // Connected: 'connected with nodeID'
    GraphConditionNode(int nid)
        : ASTNode(ASTNodeType::GraphComprehension), op(GraphConditionOp::Connected), nodeId(nid) {}

    // Cycle: 'cycle'
    GraphConditionNode()
        : ASTNode(ASTNodeType::GraphComprehension), op(GraphConditionOp::Cycle) {}

    // Degree: 'degree <op> k'
    GraphConditionNode(GraphDegreeOp dop, int val)
        : ASTNode(ASTNodeType::GraphComprehension),
          op(GraphConditionOp::Degree),
          degreeOp(dop),
          degreeValue(val) {}

    // And/Or: binary condition
    GraphConditionNode(GraphConditionOp o,
                       std::shared_ptr<GraphConditionNode> l,
                       std::shared_ptr<GraphConditionNode> r)
        : ASTNode(ASTNodeType::GraphComprehension), op(o), left(l), right(r) {}
};

enum class GraphExprOp { And, Or };

class GraphComprehensionNode : public ASTNode
{
public:
    std::string targetName;   // j
    std::string graphName;    // base graph
    std::vector<GraphExprOp> ops;
    std::vector<std::string> graphOperands;
    std::shared_ptr<GraphConditionNode> condition;

    GraphComprehensionNode(const std::string &t,
                           const std::string &g,
                           std::vector<GraphExprOp> opList,
                           std::vector<std::string> operands,
                           std::shared_ptr<GraphConditionNode> cond)
        : ASTNode(ASTNodeType::GraphComprehension),
          targetName(t),
          graphName(g),
          ops(std::move(opList)),
          graphOperands(std::move(operands)),
          condition(cond) {}
};


class GraphUpdateNode : public ASTNode {
    public: 
    
        GraphUpdateKind kind;
        std::string graphName;
        std::vector<int> nodes;
        std::vector<std::pair<int, int>> edges;
    
        GraphUpdateNode(GraphUpdateKind k,
                        const std::string &g,
                        const std::vector<int> &n,
                        const std::vector<std::pair<int, int>> &e)
            : ASTNode(ASTNodeType::GraphUpdate),
              kind(k),
              graphName(g),
              nodes(n),
              edges(e)
        {}
    };

class ProgramNode : public ASTNode
{
public:
    std::vector<ASTNodePtr> topLevel; // functions, global vars, statements…

    ProgramNode(std::vector<ASTNodePtr> items = {})
        : ASTNode(ASTNodeType::Program), topLevel(std::move(items)) {}
};
using ProgramNodePtr = std::shared_ptr<ProgramNode>;

class IntLiteralNode : public ASTNode
{
public:
    int value;
    IntLiteralNode(int val) : ASTNode(ASTNodeType::IntLiteral), value(val) {}
};

class BoolLiteralNode : public ASTNode
{
public:
    bool value;
    BoolLiteralNode(bool val) : ASTNode(ASTNodeType::BoolLiteral), value(val) {}
};

class RealLiteralNode : public ASTNode
{
public:
    double value;
    RealLiteralNode(double val) : ASTNode(ASTNodeType::RealLiteral), value(val) {}
};

class StringLiteralNode : public ASTNode
{
public:
    std::string value;
    StringLiteralNode(const std::string &s)
        : ASTNode(ASTNodeType::StringLiteral), value(s) {}
};

class ConditionalNode : public ASTNode
{
public:
    ASTNodePtr condition; // e.g. a BooleanExprNode or just an IntExprNode
    ASTNodePtr thenBlock; // a BlockStmtNode
    ASTNodePtr elseBlock; // nullptr if no else

    ConditionalNode(ASTNodePtr cond,
                    ASTNodePtr thenBlk,
                    ASTNodePtr elseBlk = nullptr)
        : ASTNode(ASTNodeType::Conditional),
          condition(std::move(cond)),
          thenBlock(std::move(thenBlk)),
          elseBlock(std::move(elseBlk)) {}
};
using ConditionalNodePtr = std::shared_ptr<ConditionalNode>;

class VarDeclNode : public ASTNode
{
public:
    std::string typeName;
    std::string name;
    ASTNodePtr initializer; // nullptr if none

    // NEW: array support
    bool isArray = false;
    size_t arraySize = 0;          // static size (0 = unknown at compile time)
    ASTNodePtr arraySizeExpr;      // dynamic size expression (e.g. int arr[n])

    // 2D array support
    bool isArray2D = false;
    size_t array2DRows = 0;
    size_t array2DCols = 0;
    ASTNodePtr array2DRowsExpr;    // dynamic rows
    ASTNodePtr array2DColsExpr;    // dynamic cols

    // Semantic type annotation (set during semantic analysis)
    TypeKind resolvedType = TypeKind::Unknown;

    VarDeclNode(std::string ty, std::string n, ASTNodePtr init = nullptr,
                bool isArr = false, size_t arrSz = 0, ASTNodePtr sizeExpr = nullptr)
        : ASTNode(ASTNodeType::VarDecl),
          typeName(std::move(ty)),
          name(std::move(n)),
          initializer(std::move(init)),
          isArray(isArr),
          arraySize(arrSz),
          arraySizeExpr(std::move(sizeExpr))
    {
    }
};

class VariableNode : public ASTNode
{
public:
    std::string name;
    // Semantic type annotation (set during semantic analysis when variable is looked up)
    TypeKind resolvedType = TypeKind::Unknown;
    
    VariableNode(const std::string &n) : ASTNode(ASTNodeType::Variable), name(n) {}
};

class BinaryExprNode : public ASTNode
{
public:
    std::string op;
    ASTNodePtr lhs;
    ASTNodePtr rhs;
    // Semantic type annotation (set during semantic analysis - result type of expression)
    TypeKind resolvedType = TypeKind::Unknown;

    BinaryExprNode(const std::string &oper, ASTNodePtr l, ASTNodePtr r)
        : ASTNode(ASTNodeType::BinaryExpr), op(oper), lhs(l), rhs(r) {}
};

class ArrayLiteralNode : public ASTNode
{
public:
    std::vector<ASTNodePtr> elements;
    ArrayLiteralNode(const std::vector<ASTNodePtr> &elems)
        : ASTNode(ASTNodeType::ArrayLiteral), elements(std::move(elems)) {}
};

class ArrayAccessNode : public ASTNode
{
public:
    ASTNodePtr arrayExpr;
    ASTNodePtr indexExpr;
    // Semantic type annotation (set during semantic analysis - element type)
    TypeKind resolvedType = TypeKind::Unknown;
    
    ArrayAccessNode(ASTNodePtr arr, ASTNodePtr idx)
        : ASTNode(ASTNodeType::ArrayAccess),
          arrayExpr(std::move(arr)),
          indexExpr(std::move(idx)) {}
};

class ReturnStmtNode : public ASTNode
{
public:
    ASTNodePtr returnValue;
    ReturnStmtNode(ASTNodePtr value)
        : ASTNode(ASTNodeType::ReturnStmt), returnValue(std::move(value)) {}
};

class AssignmentStmtNode : public ASTNode
{
public:
    ASTNodePtr lhs; // variable or ArrayAccessNode (or other lhs-expr later)
    ASTNodePtr rhs; // expression on the right

    AssignmentStmtNode(ASTNodePtr left, ASTNodePtr expr)
        : ASTNode(ASTNodeType::AssignmentStmt),
          lhs(std::move(left)),
          rhs(std::move(expr))
    {
    }
};
using AssignmentStmtNodePtr = std::shared_ptr<AssignmentStmtNode>;

class ParamNode : public ASTNode
{
public:
    std::string typeName;
    std::string paramName;
    // Semantic type annotation (set during semantic analysis)
    TypeKind resolvedType = TypeKind::Unknown;

    ParamNode(const std::string &ty, const std::string &nm)
        : ASTNode(ASTNodeType::Param),
          typeName(ty),
          paramName(nm)
    {
    }
};

using ParamNodePtr = std::shared_ptr<ParamNode>;

class BlockStmtNode : public ASTNode
{
public:
    std::vector<ASTNodePtr> statements;
    BlockStmtNode(const std::vector<ASTNodePtr> &stmts = {})
        : ASTNode(ASTNodeType::BlockStmt), statements(std::move(stmts)) {}
};

struct WhileStmtNode : ASTNode
{
    ASTNodePtr condition;
    ASTNodePtr body;
    WhileStmtNode(ASTNodePtr cond, ASTNodePtr bd)
        : ASTNode(ASTNodeType::WhileStmt),
          condition(std::move(cond)), body(std::move(bd)) {}
};

enum class ForEachTargetType
{
    Vertex,
    Edge,
    Neighbor,
    Element  // for iterating over set elements
};

struct ForEachStmtNode : ASTNode
{
    ForEachTargetType targetType; // vertex, edge, neighbor, element
    std::string var1;             // e.g., vertex or first edge ID or neighbor var or element var
    std::string var2;             // e.g., second edge ID (optional)
    std::string graphName;        // the graph/set over which to iterate
    ASTNodePtr adjNodeExpr;       // expression for neighbor-of (nullptr if not neighbor loop)
    ASTNodePtr body;              // loop body

    ForEachStmtNode(ForEachTargetType tgt, const std::string &v1, const std::string &v2,
                    const std::string &gName, ASTNodePtr adjExpr, ASTNodePtr bd)
        : ASTNode(ASTNodeType::ForEachStmt),
          targetType(tgt), var1(v1), var2(v2), graphName(gName),
          adjNodeExpr(std::move(adjExpr)), body(std::move(bd)) {}
};

class FunctionDeclNode : public ASTNode
{
public:
    std::string returnType;
    std::string name;
    std::vector<ParamNodePtr> parameters; // pointer list, not raw ParamNode
    ASTNodePtr body;                      // matches constructor
    // Semantic type annotation (set during semantic analysis - return type)
    TypeKind resolvedReturnType = TypeKind::Unknown;

    FunctionDeclNode(
        std::string retTy,
        std::string fnName,
        std::vector<ParamNodePtr> params,
        ASTNodePtr blk)
        : ASTNode(ASTNodeType::FunctionDecl),
          returnType(std::move(retTy)),
          name(std::move(fnName)),
          parameters(std::move(params)),
          body(std::move(blk))
    {
    }
};

class FunctionCallNode : public ASTNode
{
public:
    std::string name;
    std::vector<ASTNodePtr> arguments;
    // Semantic type annotation (set during semantic analysis - return type of called function)
    TypeKind resolvedType = TypeKind::Unknown;
    
    FunctionCallNode(const std::string &n, const std::vector<ASTNodePtr> &args)
        : ASTNode(ASTNodeType::FunctionCall), name(std::move(n)), arguments(std::move(args)) {}
};

class NodeListNode : public ASTNode
{
public:
    NodeListNode() : ASTNode(ASTNodeType::NodeList) {}
    virtual std::vector<int> materializeNodeIds() const = 0;
};

class InlineNodeList : public NodeListNode
{
    std::vector<int> _ids;

public:
    InlineNodeList(std::vector<int> ids)
        : NodeListNode(), _ids(std::move(ids)) {}
    std::vector<int> materializeNodeIds() const override
    {
        return _ids;
    }
};

class WeightedEdgeListNode : public ASTNode
{
public:
    WeightedEdgeListNode() : ASTNode(ASTNodeType::WeightedEdgeList) {}
    virtual void materializeEdges(std::vector<std::pair<int, int>> &edgelist, llvm::DenseMap<std::pair<int, int>, int> &weightMap) const = 0;
};

class InlineWeightedEdgeList : public WeightedEdgeListNode
{
    std::vector<std::pair<int, int>> _edges;
    llvm::DenseMap<std::pair<int, int>, int> _weights;

public:
    InlineWeightedEdgeList(std::vector<std::pair<int, int>> edges, llvm::DenseMap<std::pair<int, int>, int> weights)
        : WeightedEdgeListNode(), _edges(std::move(edges)), _weights(std::move(weights)) {}
    void materializeEdges(std::vector<std::pair<int, int>> &_edges, llvm::DenseMap<std::pair<int, int>, int> &_weights) const override
    {
        _edges = this->_edges;
        _weights = this->_weights;
    }
};

class EdgeListNode : public ASTNode
{
public:
    EdgeListNode() : ASTNode(ASTNodeType::EdgeList) {}
    virtual std::vector<std::pair<int, int>> materializeEdges() const = 0;
};

class InlineEdgeList : public EdgeListNode
{
    std::vector<std::pair<int, int>> _edges;

public:
    InlineEdgeList(std::vector<std::pair<int, int>> edges)
        : EdgeListNode(), _edges(std::move(edges)) {}
    std::vector<std::pair<int, int>> materializeEdges() const override
    {
        return _edges;
    }
};

class FileEdgeList : public EdgeListNode
{
    std::string _path;

public:
    FileEdgeList(std::string path)
        : EdgeListNode(), _path(std::move(path)) {}
    std::vector<std::pair<int, int>> materializeEdges() const override
    {
        std::ifstream file(_path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open edge list file: " + _path);
        }
        int u, v;
        std::vector<std::pair<int, int>> edges;
        while (file >> u >> v)
        {
            edges.emplace_back(u, v);
        }
        file.close();
        return edges;
    }
};

class WeightedFileEdgeList : public WeightedEdgeListNode
{
    std::string _path;

public:
    WeightedFileEdgeList(std::string path)
        : WeightedEdgeListNode(), _path(std::move(path)) {}
    void materializeEdges(std::vector<std::pair<int, int>> &edges, llvm::DenseMap<std::pair<int, int>, int> &weights) const override
    {
        std::ifstream file(_path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open edge list file: " + _path);
        }
        int u, v, w;
        while (file >> u >> v >> w)
        {
            edges.emplace_back(u, v);
            weights[{u, v}] = w;
        }
        file.close();
    }
};

class ShowGraphNode : public ASTNode
{
public:
    std::string graphName;

    ShowGraphNode(const std::string &g)
        : ASTNode(ASTNodeType::ShowGraph), graphName(g) {}
};


// class GraphDeclNode : public ASTNode
// {
// public:
//     std::string name;
//     std::unique_ptr<NodeListNode> nodes;
//     std::unique_ptr<EdgeListNode> edges;

//     size_t n = 0, m = 0;
//     std::vector<size_t> row_ptr;
//     std::vector<int> col_idx;

//     GraphDeclNode(
//         std::string nm,
//         std::unique_ptr<NodeListNode> nList,
//         std::unique_ptr<EdgeListNode> eList)
//         : ASTNode(ASTNodeType::GraphDecl),
//           name(std::move(nm)),
//           nodes(std::move(nList)),
//           edges(std::move(eList))
//     {

//         if (!edges)
//             throw std::runtime_error("GraphDeclNode: edges required");

//         auto edgeList = edges->materializeEdges();

//         llvm::DenseSet<int> nodeSet;
//         if (nodes)
//         {
//             auto explicitIds = nodes->materializeNodeIds();
//             for (int id : explicitIds)
//                 nodeSet.insert(id);
//         }

//         for (auto &e : edgeList)
//         {
//             nodeSet.insert(e.first);
//             nodeSet.insert(e.second);
//         }

//         std::vector<int> nodeIds;
//         nodeIds.reserve(nodeSet.size());
//         for (int id : nodeSet)
//             nodeIds.push_back(id);

//         std::sort(nodeIds.begin(), nodeIds.end());

//         nodes = std::make_unique<InlineNodeList>(std::move(nodeIds));

//         auto materializedNodes = nodes->materializeNodeIds();
//         n = materializedNodes.size();

//         llvm::DenseMap<int, int> id2idx;
//         for (int i = 0; i < (int)materializedNodes.size(); ++i)
//             id2idx[materializedNodes[i]] = i;

//         m = 2 * edgeList.size();

//         row_ptr.assign(n + 1, 0);
//         for (auto &e : edgeList)
//         {
//             int u0 = e.first, v0 = e.second;
//             auto it_u = id2idx.find(u0);
//             auto it_v = id2idx.find(v0);
//             if (it_u == id2idx.end() || it_v == id2idx.end())
//             {
//                 continue;
//             }
//             size_t u = it_u->second;
//             size_t v = it_v->second;
//             row_ptr[u + 1]++;
//             row_ptr[v + 1]++;
//         }

//         for (size_t i = 1; i <= n; ++i)
//             row_ptr[i] += row_ptr[i - 1];

//         col_idx.resize(m);
//         std::vector<size_t> next = row_ptr;
//         for (auto &e : edgeList)
//         {
//             int u0 = e.first, v0 = e.second;
//             auto it_u = id2idx.find(u0);
//             auto it_v = id2idx.find(v0);
//             if (it_u == id2idx.end() || it_v == id2idx.end())
//             {
//                 continue;
//             }
//             size_t u = it_u->second;
//             size_t v = it_v->second;
//             col_idx[next[u]++] = (int)v;
//             col_idx[next[v]++] = (int)u;
//         }
//     }
// };

class GraphDeclNode : public ASTNode
{
public:
    std::string name;
    std::unique_ptr<NodeListNode> nodes;
    std::unique_ptr<EdgeListNode> edges;

    bool isFileGraph = false;
    std::string edgeFileName;

    size_t n = 0, m = 0;
    size_t *row_ptr = nullptr;
    int32_t *col_idx = nullptr;
    llvm::BumpPtrAllocator arena;
    std::vector<int> node_ids;
    std::vector<std::pair<int, int>> edge_list;
    std::vector<std::pair<int, int>> edge_id_map;
    std::vector<uint8_t> nodes_blob;
    std::vector<uint8_t> edges_blob;

    // File-based graph: defer all loading to runtime
    GraphDeclNode(std::string nm, std::string fileName)
        : ASTNode(ASTNodeType::GraphDecl),
          name(std::move(nm)),
          isFileGraph(true),
          edgeFileName(std::move(fileName))
    {
    }

    // Inline graph: build CSR at compile time (for small inline edge lists)
    GraphDeclNode(
        std::string nm,
        std::unique_ptr<NodeListNode> nList,
        std::unique_ptr<EdgeListNode> eList)
        : ASTNode(ASTNodeType::GraphDecl),
          name(std::move(nm)),
          nodes(std::move(nList)),
          edges(std::move(eList))
    {
        auto nodeIds = nodes->materializeNodeIds();
        n = nodeIds.size();

        llvm::DenseMap<int, int> id2idx;
        for (int i = 0; i < (int)nodeIds.size(); ++i)
            id2idx[nodeIds[i]] = i;

        auto edgeList = edges->materializeEdges();
        m = 2 * edgeList.size();
        row_ptr = static_cast<size_t *>(arena.Allocate(sizeof(size_t) * (n + 1), alignof(size_t)));
        std::memset(row_ptr, 0, (n + 1) * sizeof(size_t));
        for (auto &e : edgeList)
        {
            int u0 = e.first, v0 = e.second;
            size_t u = id2idx.at(u0);
            size_t v = id2idx.at(v0);
            row_ptr[u + 1]++;
            row_ptr[v + 1]++;
        }

        for (size_t i = 1; i <= n; ++i)
            row_ptr[i] += row_ptr[i - 1];

        col_idx = static_cast<int32_t *>(arena.Allocate(sizeof(int32_t) * (m), alignof(int32_t)));
        size_t *next = static_cast<size_t *>(arena.Allocate(sizeof(size_t) * (n + 1), alignof(size_t)));
        std::memcpy(next, row_ptr, sizeof(size_t) * (n + 1));
        for (auto &e : edgeList)
        {
            size_t u = id2idx[e.first], v = id2idx[e.second];
            col_idx[next[u]++] = static_cast<int32_t>(v);
            col_idx[next[v]++] = static_cast<int32_t>(u);
        }

        node_ids = nodeIds;
        edge_list = edgeList;
        edge_id_map = edgeList;

        // Build roaring bitmap for nodes
        {
            RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
            for (int id : node_ids)
                roaring_bitmap_add(bm, static_cast<uint32_t>(id));

            size_t sz = roaring_bitmap_portable_size_in_bytes(bm);
            nodes_blob.resize(sz);
            roaring_bitmap_portable_serialize(bm, nodes_blob.data());
            roaring_bitmap_free(bm);
        }

        // Build roaring bitmap for edges (IDs are index in edge_list)
        {
            RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
            for (uint32_t eid = 0; eid < edge_list.size(); ++eid)
                roaring_bitmap_add(bm, eid);

            size_t sz = roaring_bitmap_portable_size_in_bytes(bm);
            edges_blob.resize(sz);
            roaring_bitmap_portable_serialize(bm, edges_blob.data());
            roaring_bitmap_free(bm);
        }
    }
};


class WeightedGraphDeclNode : public ASTNode
{
public:
    std::string name;
    std::unique_ptr<NodeListNode> nodes;
    std::unique_ptr<WeightedEdgeListNode> edges;

    bool isFileGraph = false;
    std::string edgeFileName;

    size_t n = 0, m = 0; // number of nodes and edges
    size_t *row_ptr = nullptr;
    int32_t *col_idx = nullptr;
    int32_t *weights = nullptr;
    llvm::BumpPtrAllocator arena;
    std::vector<int> node_ids;
    std::vector<std::pair<int, int>> edge_list;
    std::vector<std::pair<int, int>> edge_id_map;
    std::vector<uint8_t> nodes_blob;
    std::vector<uint8_t> edges_blob;

    WeightedGraphDeclNode(std::string nm, std::string fileName)
        : ASTNode(ASTNodeType::WeightedGraphDecl),
          name(std::move(nm)),
          isFileGraph(true),
          edgeFileName(std::move(fileName))
    {
    }

    WeightedGraphDeclNode(
        std::string nm,
        std::unique_ptr<NodeListNode> nList,
        std::unique_ptr<WeightedEdgeListNode> eList)
        : ASTNode(ASTNodeType::WeightedGraphDecl),
          name(std::move(nm)),
          nodes(std::move(nList)),
          edges(std::move(eList))
    {
        auto t0 = std::chrono::high_resolution_clock::now();

        auto nodeIds = nodes->materializeNodeIds();
        n = nodeIds.size();
        std::cerr << "[ASTBuilder] Weighted graph '" << name << "' with " << n << " nodes\n";

        // Map arbitrary IDs → contiguous [0..n-1]
        // std::unordered_map<int, int> id2idx;
        llvm::DenseMap<int, int> id2idx;
        for (int i = 0; i < (int)nodeIds.size(); ++i)
            id2idx[nodeIds[i]] = i;

        std::vector<std::pair<int, int>> edgeList;
        llvm::DenseMap<std::pair<int, int>, int> weightMap;
        edges->materializeEdges(edgeList, weightMap);
        m = 2 * edgeList.size();
        std::cerr << "[ASTBuilder] Weighted graph '" << name << "' with " << m << " edges\n";
        // 1) degree counts go into row_ptr[i+1]
        row_ptr = static_cast<size_t *>(arena.Allocate(sizeof(size_t) * (n + 1), alignof(size_t)));
        std::memset(row_ptr, 0, (n + 1) * sizeof(size_t));
        for (auto &e : edgeList)
        {
            int u0 = e.first, v0 = e.second;
            size_t u = id2idx.at(u0);
            size_t v = id2idx.at(v0);
            row_ptr[u + 1]++;
            row_ptr[v + 1]++;
        }

        // 2) exclusive prefix‑sum
        for (size_t i = 1; i <= n; ++i)
            row_ptr[i] += row_ptr[i - 1];

        // 3) allocate col_idx and scatter
        col_idx = static_cast<int32_t *>(arena.Allocate(sizeof(int32_t) * (m), alignof(int32_t)));
        weights = static_cast<int32_t *>(arena.Allocate(sizeof(int32_t) * (m), alignof(int32_t)));
        size_t *next = static_cast<size_t *>(arena.Allocate(sizeof(size_t) * (n + 1), alignof(size_t)));
        size_t *wnext = static_cast<size_t *>(arena.Allocate(sizeof(size_t) * (n + 1), alignof(size_t)));
        std::memcpy(next, row_ptr, sizeof(size_t) * (n + 1));
        std::memcpy(wnext, row_ptr, sizeof(size_t) * (n + 1));
        for (auto &e : edgeList)
        {
            size_t u = id2idx[e.first], v = id2idx[e.second];
            col_idx[next[u]++] = static_cast<int32_t>(v);
            col_idx[next[v]++] = static_cast<int32_t>(u);
            int w = weightMap[{e.first, e.second}];
            weights[wnext[u]++] = w;
            weights[wnext[v]++] = w;
        }

        // debug
        // std::cerr << "[GraphDeclNode] CSR row_ptr =";
        // for (auto x : row_ptr) std::cerr << " " << x;
        // std::cerr << "\n[GraphDeclNode] CSR col_idx =";
        // for (auto x : col_idx) std::cerr << " " << x;
        // std::cerr << "\n";
        auto t1 = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        std::cerr << "[ASTBuilder] fulledge '"
                  << "' in " << dur.count() << " ms\n";

        std::cerr << "[ASTBuilder] Weighted graph '" << name << "' with " << weights[0] << " weights\n";

        node_ids = nodeIds;
        edge_list = edgeList;
        edge_id_map = edgeList;

        // Build roaring bitmap for nodes
        {
            RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
            for (int id : node_ids)
                roaring_bitmap_add(bm, static_cast<uint32_t>(id));

            size_t sz = roaring_bitmap_portable_size_in_bytes(bm);
            nodes_blob.resize(sz);
            roaring_bitmap_portable_serialize(bm, nodes_blob.data());
            roaring_bitmap_free(bm);
        }

        // Build roaring bitmap for edges (IDs are index in edge_list)
        {
            RoaringBitmap *bm = roaring_bitmap_create(64 * 1024, 8);
            for (uint32_t eid = 0; eid < edge_list.size(); ++eid)
                roaring_bitmap_add(bm, eid);

            size_t sz = roaring_bitmap_portable_size_in_bytes(bm);
            edges_blob.resize(sz);
            roaring_bitmap_portable_serialize(bm, edges_blob.data());
            roaring_bitmap_free(bm);
        }
    }
};

enum class QueryType
{
    INT,
    ARRAY,
    UNKNOWN
};

class QueryNode : public ASTNode
{
public:
    std::string queryName;
    std::string queryDesc;
    std::string graphName;
    std::optional<int> source;
    QueryType type;
    QueryNode(const std::string &n, const std::string &d, const std::string &g, QueryType t, std::optional<int> src = std::nullopt)
        : ASTNode(ASTNodeType::QueryNode),
          queryName(n), queryDesc(d), graphName(g), type(t), source(src) {}
};

class PrintStmtNode : public ASTNode
{
public:
    ASTNodePtr expr;

    PrintStmtNode(ASTNodePtr e)
        : ASTNode(ASTNodeType::PrintStmt), expr(std::move(e)) {}
};

class SleepStmtNode : public ASTNode
{
public:
    ASTNodePtr duration; // Expression evaluating to int (seconds)

    SleepStmtNode(ASTNodePtr dur)
        : ASTNode(ASTNodeType::SleepStmt), duration(std::move(dur)) {}
};

class PrintArrayNode : public ASTNode
{
public:
    std::string arrayName;
    ASTNodePtr indexExpr;

    // Constructor
    PrintArrayNode(const std::string &name, ASTNodePtr index)
        : ASTNode(ASTNodeType::PrintStmt), arrayName(name), indexExpr(index) {}
};

class SetDeclNode : public ASTNode
{
public:
    std::string name;
    ASTNodePtr initializer;
    SetDeclNode(const std::string &n, ASTNodePtr init = nullptr)
        : ASTNode(ASTNodeType::SetDecl),
          name(n),
          initializer(std::move(init)) {}
};

class SetLiteralNode : public ASTNode
{
public:
    std::vector<ASTNodePtr> elements;

    SetLiteralNode(const std::vector<ASTNodePtr> &elems)
        : ASTNode(ASTNodeType::SetLiteral),
          elements(std::move(elems)) {}
};

class SetOperationNode : public ASTNode
{
public:
    std::string targetName;
    ASTNodePtr expr;

    SetOperationNode(const std::string &target, ASTNodePtr expression)
        : ASTNode(ASTNodeType::SetOperation),
          targetName(target),
          expr(std::move(expression)) {}
};

class SetBinaryExprNode : public ASTNode
{
public:
    std::string op;
    ASTNodePtr lhs;
    ASTNodePtr rhs;

    SetBinaryExprNode(const std::string &operation, ASTNodePtr left, ASTNodePtr right)
        : ASTNode(ASTNodeType::SetBinaryExpr),
          op(operation),
          lhs(std::move(left)),
          rhs(std::move(right)) {}
};

class SetIdNode : public ASTNode
{
public:
    std::string name;

    SetIdNode(const std::string &setName)
        : ASTNode(ASTNodeType::Variable),
          name(setName)
    {
    }
};

enum class GraphMemberKind
{
    Nodes,
    Edges
};

enum class SetTargetKind
{
    Variable,
    GraphNodes,
    GraphEdges
};

class GraphMemberSetNode : public ASTNode
{
public:
    std::string graphName;
    GraphMemberKind member;

    GraphMemberSetNode(const std::string &g, GraphMemberKind m)
        : ASTNode(ASTNodeType::GraphMemberSet), graphName(g), member(m) {}
};

class SetMethodCallNode : public ASTNode
{
public:
    SetTargetKind targetKind;
    std::string targetName;
    std::string methodName;
    ASTNodePtr argument;

    SetMethodCallNode(SetTargetKind kind, const std::string &name,
                      const std::string &method, ASTNodePtr arg)
        : ASTNode(ASTNodeType::SetMethodCall),
          targetKind(kind),
          targetName(name),
          methodName(method),
          argument(std::move(arg)) {}
};

class SetContainsExprNode : public ASTNode
{
public:
    SetTargetKind targetKind;
    std::string targetName;
    ASTNodePtr argument;

    SetContainsExprNode(SetTargetKind kind, const std::string &name, ASTNodePtr arg)
        : ASTNode(ASTNodeType::SetContainsExpr),
          targetKind(kind),
          targetName(name),
          argument(std::move(arg)) {}
};

class SetPopExprNode : public ASTNode
{
public:
    std::string setName;

    SetPopExprNode(const std::string &name)
        : ASTNode(ASTNodeType::SetPopExpr), setName(name) {}
};

class SwapStmtNode : public ASTNode
{
public:
    std::string name1;
    std::string name2;

    SwapStmtNode(const std::string &a, const std::string &b)
        : ASTNode(ASTNodeType::SwapStmt), name1(a), name2(b) {}
};

class NotExprNode : public ASTNode
{
public:
    ASTNodePtr operand;
    TypeKind resolvedType = TypeKind::Unknown;

    NotExprNode(ASTNodePtr op)
        : ASTNode(ASTNodeType::NotExpr), operand(std::move(op)) {}
};

class BreakStmtNode : public ASTNode
{
public:
    BreakStmtNode() : ASTNode(ASTNodeType::BreakStmt) {}
};

class ContinueStmtNode : public ASTNode
{
public:
    ContinueStmtNode() : ASTNode(ASTNodeType::ContinueStmt) {}
};

class UnaryMinusExprNode : public ASTNode
{
public:
    ASTNodePtr operand;
    TypeKind resolvedType = TypeKind::Unknown;

    UnaryMinusExprNode(ASTNodePtr op)
        : ASTNode(ASTNodeType::UnaryMinusExpr), operand(std::move(op)) {}
};

class Array2DAccessNode : public ASTNode
{
public:
    ASTNodePtr arrayExpr;   // the base variable
    ASTNodePtr rowExpr;     // first index
    ASTNodePtr colExpr;     // second index
    TypeKind resolvedType = TypeKind::Unknown;

    Array2DAccessNode(ASTNodePtr arr, ASTNodePtr row, ASTNodePtr col)
        : ASTNode(ASTNodeType::Array2DAccess),
          arrayExpr(std::move(arr)),
          rowExpr(std::move(row)),
          colExpr(std::move(col)) {}
};

// Extended VarDeclNode to support 2D arrays
// isArray2D flag + rows/cols for 2D
class VarDecl2DInfo
{
public:
    bool is2D = false;
    size_t rows = 0;        // static row count
    size_t cols = 0;        // static col count
    ASTNodePtr rowsExpr;    // dynamic row expression
    ASTNodePtr colsExpr;    // dynamic col expression
};

#endif // ASTNODE_H
