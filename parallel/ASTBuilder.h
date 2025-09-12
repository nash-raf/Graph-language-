#ifndef ASTBUILDER_H
#define ASTBUILDER_H

#include "BaseBaseVisitor.h"
#include "ASTNode.h"

class ASTBuilder : public BaseBaseVisitor
{
public:
    antlrcpp::Any visitProgram(BaseParser::ProgramContext *ctx) override;
    antlrcpp::Any visitStatement(BaseParser::StatementContext *ctx) override;
    antlrcpp::Any visitExpr(BaseParser::ExprContext *ctx);

    antlrcpp::Any visitVarDecl(BaseParser::VarDeclContext *ctx);
    antlrcpp::Any visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx) override;

    antlrcpp::Any visitConditionalStatement(BaseParser::ConditionalStatementContext *ctx) override;
    antlrcpp::Any visitCondition(BaseParser::ConditionContext *ctx);
    antlrcpp::Any visitBlock(BaseParser::BlockContext *ctx) override;
    // antlrcpp::Any visitLoopStatement(BaseParser::LoopStatementContext* ctx) override;
    antlrcpp::Any visitWhileStatement(BaseParser::WhileStatementContext *ctx) override;

    antlrcpp::Any visitArrayAssignStmt(BaseParser::ArrayAssignStmtContext *ctx) override;
    antlrcpp::Any visitFunction(BaseParser::FunctionContext *ctx) override;
    antlrcpp::Any visitFunctionCall(BaseParser::FunctionCallContext *ctx) override;
    antlrcpp::Any visitGraphDef(BaseParser::GraphDefContext *ctx) override;

    antlrcpp::Any visitQueryStatement(BaseParser::QueryStatementContext *ctx) override;

    antlrcpp::Any visitPrintExpr(BaseParser::PrintExprContext *ctx) override;
    antlrcpp::Any visitPrintStatement(BaseParser::PrintStatementContext *ctx) override;

private:
    std::unordered_map<std::string, int> symbolTable;
    std::unordered_map<std::string, std::vector<ASTNodePtr>> arrayTable;
    std::unordered_map<std::string, std::shared_ptr<FunctionDeclNode>> functionTable;

    int evaluate(ASTNodePtr node);
};

#endif // ASTBUILDER_H