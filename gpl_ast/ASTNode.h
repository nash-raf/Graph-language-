#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <vector>
#include <string> 

enum class ASTNodeType {
    Program,
    IntLiteral,
    Variable,
    BinaryExpr,
    ArrayLiteral,
    ArrayAccess,
    FunctionCall,
    FunctionDecl,
    ReturnStmt,
    BlockStmt,
    Param
      
};

class ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;

class ASTNode {
public:
    ASTNodeType type;
    explicit ASTNode(ASTNodeType t) : type(t) {}
    virtual ~ASTNode() = default;
};

// --- at the top of ASTBuilder.h or your Utils.h ---
template<typename T>
T safe_any_cast(const antlrcpp::Any& a,
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
