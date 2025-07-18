#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>

enum class ASTNodeType {
    Program,
    IntLiteral,
    Variable,
    BinaryExpr,
      
};

class ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;

class ASTNode {
public:
    ASTNodeType type;
    explicit ASTNode(ASTNodeType t) : type(t) {}
    virtual ~ASTNode() = default;
};

class IntLiteralNode : public ASTNode {
public:
    int value;
    IntLiteralNode(int val) : ASTNode(ASTNodeType::IntLiteral), value(val) {}
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


#endif // ASTNODE_H
