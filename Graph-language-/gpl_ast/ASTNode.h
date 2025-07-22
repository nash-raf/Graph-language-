#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <vector>
#include <string> 
#include <any>
#include <iostream>

enum class ASTNodeType {
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
    Param
};

template<typename T>
T safe_any_cast(const std::any& a,
                 const char* where = "safe_any_cast")
{
    try {
        return std::any_cast<T>(a);
    } catch (const std::bad_any_cast&) {
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

class ASTNode {
public:
    ASTNodeType type;
    explicit ASTNode(ASTNodeType t) : type(t) {}
    virtual ~ASTNode() = default;
};

class ProgramNode : public ASTNode {
public:
  std::vector<ASTNodePtr> topLevel;   // functions, global vars, statements…

  ProgramNode(std::vector<ASTNodePtr> items = {})
    : ASTNode(ASTNodeType::Program), topLevel(std::move(items)) {}
};
using ProgramNodePtr = std::shared_ptr<ProgramNode>;


class IntLiteralNode : public ASTNode {
public:
    int value;
    IntLiteralNode(int val) : ASTNode(ASTNodeType::IntLiteral), value(val) {}
};

class ConditionalNode : public ASTNode {
public:
  ASTNodePtr condition;    // e.g. a BooleanExprNode or just an IntExprNode
  ASTNodePtr thenBlock;    // a BlockStmtNode
  ASTNodePtr elseBlock;    // nullptr if no else

  ConditionalNode(ASTNodePtr cond,
             ASTNodePtr thenBlk,
             ASTNodePtr elseBlk = nullptr)
    : ASTNode(ASTNodeType::Conditional),
      condition(std::move(cond)),
      thenBlock(std::move(thenBlk)),
      elseBlock(std::move(elseBlk)) {}
};
using ConditionalNodePtr = std::shared_ptr<ConditionalNode>;

class VarDeclNode : public ASTNode {
public:
  std::string name;
  ASTNodePtr  initializer;  // nullptr if none

  VarDeclNode(std::string n, ASTNodePtr init = nullptr)
    : ASTNode(ASTNodeType::VarDecl),
      name(std::move(n)),
      initializer(std::move(init)) {}
};


class VariableNode : public ASTNode {
public:
    std::string name;
    VariableNode(const std::string& n) : ASTNode(ASTNodeType::Variable), name(n) {}
};

class BinaryExprNode : public ASTNode {
public:
    std::string op;
    ASTNodePtr lhs;
    ASTNodePtr rhs;

    BinaryExprNode(const std::string& oper, ASTNodePtr l, ASTNodePtr r)
        : ASTNode(ASTNodeType::BinaryExpr), op(oper), lhs(l), rhs(r) {}
};

class ArrayLiteralNode : public ASTNode {
public:
    std::vector<ASTNodePtr> elements;
    ArrayLiteralNode(const std::vector<ASTNodePtr>& elems)
        : ASTNode(ASTNodeType::ArrayLiteral), elements(std::move(elems)) {}
};

class ArrayAccessNode : public ASTNode {
public:
    ASTNodePtr arrayExpr;
    ASTNodePtr indexExpr;
    ArrayAccessNode(ASTNodePtr arr, ASTNodePtr idx)
      : ASTNode(ASTNodeType::ArrayAccess),
        arrayExpr(std::move(arr)),
        indexExpr(std::move(idx)) {}
};

class ReturnStmtNode : public ASTNode {
public:
    ASTNodePtr returnValue;
    ReturnStmtNode(ASTNodePtr value)
        : ASTNode(ASTNodeType::ReturnStmt), returnValue(std::move(value)) {}
};

class AssignmentStmtNode : public ASTNode {
public:
  std::string    lhsName;   // variable being assigned
  ASTNodePtr     rhs;       // expression on the right

  AssignmentStmtNode(std::string name, ASTNodePtr expr)
    : ASTNode(ASTNodeType::AssignmentStmt),
      lhsName(std::move(name)),
      rhs(std::move(expr))
  {}
};
using AssignmentStmtNodePtr = std::shared_ptr<AssignmentStmtNode>;

class ParamNode : public ASTNode {
public:
    std::string typeName;
    std::string paramName;

    ParamNode(const std::string& ty, const std::string& nm)
      : ASTNode(ASTNodeType::Param),
        typeName(ty),
        paramName(nm)
    {}
};

using ParamNodePtr = std::shared_ptr<ParamNode>;

class BlockStmtNode : public ASTNode {
public :
    std::vector<ASTNodePtr> statements;
    BlockStmtNode(const std::vector<ASTNodePtr>& stmts = {})
        :ASTNode(ASTNodeType::BlockStmt), statements(std::move(stmts)) {}
};

struct WhileStmtNode : ASTNode {
  ASTNodePtr condition;
  ASTNodePtr body;
  WhileStmtNode(ASTNodePtr cond, ASTNodePtr bd)
    : ASTNode(ASTNodeType::WhileStmt), 
      condition(std::move(cond)), body(std::move(bd)) {}
};

class FunctionDeclNode : public ASTNode {
public:
    std::string                 returnType;
    std::string                 name;
    std::vector<ParamNodePtr>   parameters; // pointer list, not raw ParamNode
    ASTNodePtr                  body;       // matches constructor

    FunctionDeclNode(
        std::string retTy,
        std::string fnName,
        std::vector<ParamNodePtr> params,
        ASTNodePtr blk
    )
      : ASTNode(ASTNodeType::FunctionDecl),
        returnType(std::move(retTy)),
        name(std::move(fnName)),
        parameters(std::move(params)),
        body(std::move(blk))
    {}
};

class FunctionCallNode : public ASTNode {
public:
    std::string name;
    std::vector<ASTNodePtr> arguments;
    FunctionCallNode(const std::string& n, const std::vector<ASTNodePtr>& args)
        : ASTNode(ASTNodeType::FunctionCall), name(std::move(n)), arguments(std::move(args)) {}
};


#endif // ASTNODE_H
