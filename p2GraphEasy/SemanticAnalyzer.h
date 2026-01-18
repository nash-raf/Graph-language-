#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ASTNode.h"
#include <string>
#include <unordered_map>
#include <vector>

enum class TypeKind
{
    Int,
    Bool,
    Real,
    String,
    IntArray,
    Graph,
    WeightedGraph,
    Void,
    Unknown
};

struct FunctionInfo
{
    TypeKind returnType = TypeKind::Unknown;
    std::vector<TypeKind> paramTypes;
};

struct Symbol
{
    TypeKind type = TypeKind::Unknown;
    bool isFunction = false;
    FunctionInfo func;
};

class SemanticAnalyzer
{
public:
    explicit SemanticAnalyzer(ProgramNodePtr root);
    void analyze();

private:
    ProgramNodePtr root;
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    std::unordered_map<std::string, ASTNode *> graphDecls;
    std::vector<TypeKind> returnTypeStack;

    void enterScope();
    void exitScope();

    void declareSymbol(const std::string &name, const Symbol &sym);
    Symbol *lookupSymbol(const std::string &name);

    TypeKind typeFromString(const std::string &typeName) const;
    TypeKind analyzeExpr(ASTNode *expr);

    void analyzeProgram(ProgramNodePtr prog);
    void analyzeStatement(ASTNode *node);
    void analyzeBlock(BlockStmtNode *block);
    void analyzeVarDecl(VarDeclNode *decl);
    void analyzeAssignment(AssignmentStmtNode *assign);
    void analyzeReturnStmt(ReturnStmtNode *ret);
    void analyzeConditional(ConditionalNode *ifs);
    void analyzeWhile(WhileStmtNode *ws);
    void analyzeForEach(ForEachStmtNode *fs);
    void analyzeFunctionDecl(FunctionDeclNode *func);
    void analyzeGraphDecl(GraphDeclNode *G);
    void analyzeWeightedGraphDecl(WeightedGraphDeclNode *G);
    void analyzeQuery(QueryNode *Q);
    void analyzePrintStmt(PrintStmtNode *PS);
    void analyzeGraphUpdate(GraphUpdateNode *upd);
    void analyzeShowGraph(ShowGraphNode *S);
    void analyzeGraphComprehension(GraphComprehensionNode *GC);
    void validateGraphCondition(GraphConditionNode *cond, GraphDeclNode *G);

    [[noreturn]] void error(const std::string &msg) const;
};

#endif // SEMANTIC_ANALYZER_H

