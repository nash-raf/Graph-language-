#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <vector>
#include <string>
#include <any>
#include <iostream>
#include <fstream>
#include <cstddef>

#include <unordered_set>
#include <algorithm>
#include <chrono>

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"

#include <cstring>
#include "llvm/Support/Allocator.h"

enum class ASTNodeType
{
    Program,
    IntLiteral,
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
    EdgeList,
    NodeList,
    QueryNode,
    PrintStmt,
    SetDecl,
    SetLiteral
};

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
        std::cerr
            << "[bad_any_cast in " << where << "] "
            << "stored type = " << a.type().name()
            << ", requested = " << typeid(T).name()
            << "\n";
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
};

class ProgramNode : public ASTNode
{
public:
    std::vector<ASTNodePtr> topLevel;

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

class ConditionalNode : public ASTNode
{
public:
    ASTNodePtr condition;
    ASTNodePtr thenBlock;
    ASTNodePtr elseBlock;

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
    std::string name;
    ASTNodePtr initializer;

    bool isArray = false;
    size_t arraySize = 0;

    VarDeclNode(std::string n, ASTNodePtr init = nullptr, bool isArr = false, size_t arrSz = 0)
        : ASTNode(ASTNodeType::VarDecl),
          name(std::move(n)),
          initializer(std::move(init)),
          isArray(isArr),
          arraySize(arrSz)
    {
    }
};

class VariableNode : public ASTNode
{
public:
    std::string name;
    VariableNode(const std::string &n) : ASTNode(ASTNodeType::Variable), name(n) {}
};

class BinaryExprNode : public ASTNode
{
public:
    std::string op;
    ASTNodePtr lhs;
    ASTNodePtr rhs;

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
    ASTNodePtr lhs;
    ASTNodePtr rhs;

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

class FunctionDeclNode : public ASTNode
{
public:
    std::string returnType;
    std::string name;
    std::vector<ParamNodePtr> parameters;
    ASTNodePtr body;

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
        auto t0 = std::chrono::high_resolution_clock::now();

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

        auto t1 = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        std::cerr << "[ASTBuilder] loaded edge-list file '"
                  << "' in " << dur.count() << " ms\n";
        return edges;
    }
};

class GraphDeclNode : public ASTNode
{
public:
    std::string name;
    std::unique_ptr<NodeListNode> nodes;
    std::unique_ptr<EdgeListNode> edges;

    size_t n, m;
    size_t *row_ptr = nullptr;
    int32_t *col_idx = nullptr;
    llvm::BumpPtrAllocator arena;

    GraphDeclNode(
        std::string nm,
        std::unique_ptr<NodeListNode> nList,
        std::unique_ptr<EdgeListNode> eList)
        : ASTNode(ASTNodeType::GraphDecl),
          name(std::move(nm)),
          nodes(std::move(nList)),
          edges(std::move(eList))
    {
        // auto t0 = std::chrono::high_resolution_clock::now();

        auto nodeIds = nodes->materializeNodeIds();
        n = nodeIds.size();

        // Map arbitrary IDs → contiguous [0..n-1]
        // std::unordered_map<int, int> id2idx;
        llvm::DenseMap<int, int> id2idx;
        for (int i = 0; i < (int)nodeIds.size(); ++i)
            id2idx[nodeIds[i]] = i;

        auto edgeList = edges->materializeEdges();
        m = 2 * edgeList.size();
        // degree counts go into row_ptr[i+1]
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

        // exclusive prefix‑sum
        for (size_t i = 1; i <= n; ++i)
            row_ptr[i] += row_ptr[i - 1];

        // allocate col_idx and scatter
        col_idx = static_cast<int32_t *>(arena.Allocate(sizeof(int32_t) * (m), alignof(int32_t)));
        size_t *next = static_cast<size_t *>(arena.Allocate(sizeof(size_t) * (n + 1), alignof(size_t)));
        std::memcpy(next, row_ptr, sizeof(size_t) * (n + 1));
        for (auto &e : edgeList)
        {
            size_t u = id2idx[e.first], v = id2idx[e.second];
            col_idx[next[u]++] = static_cast<int32_t>(v);
            col_idx[next[v]++] = static_cast<int32_t>(u);
        }

        // debug
        // std::cerr << "[GraphDeclNode] CSR row_ptr =";
        // for (auto x : row_ptr) std::cerr << " " << x;
        // std::cerr << "\n[GraphDeclNode] CSR col_idx =";
        // for (auto x : col_idx) std::cerr << " " << x;
        // std::cerr << "\n";
        // auto t1 = std::chrono::high_resolution_clock::now();
        // auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        // std::cerr << "[ASTBuilder] fulledge '"
        //           << "' in " << dur.count() << " ms\n";
        // exit(0);
    }
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

//         auto t0 = std::chrono::high_resolution_clock::now();

//         auto edgeList = edges->materializeEdges();
//         llvm::DenseSet<int> nodeSet;
//         // std::unordered_set<int> nodeSet;
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
//         // std::unordered_map<int, int> id2idx;
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
//         auto t1 = std::chrono::high_resolution_clock::now();
//         auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
//         std::cerr << "[ASTBuilder] fulledge '"
//                   << "' in " << dur.count() << " ms\n";
//     }
// };

class QueryNode : public ASTNode
{
public:
    std::string queryName;
    std::string queryDesc;
    std::string graphName;
    QueryNode(const std::string &n, const std::string &d, const std::string &g)
        : ASTNode(ASTNodeType::QueryNode),
          queryName(n), queryDesc(d), graphName(g) {}
};

class PrintStmtNode : public ASTNode
{
public:
    ASTNodePtr expr;

    PrintStmtNode(ASTNodePtr e)
        : ASTNode(ASTNodeType::PrintStmt), expr(std::move(e)) {}
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

#endif // ASTNODE_H