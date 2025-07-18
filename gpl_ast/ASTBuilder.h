#ifndef ASTBUILDER_H
#define ASTBUILDER_H

#include "BaseBaseVisitor.h"
#include "ASTNode.h"

class ASTBuilder : public BaseBaseVisitor {
public:
    antlrcpp::Any visitProgram(BaseParser::ProgramContext* ctx) override;
    antlrcpp::Any visitStatement(BaseParser::StatementContext* ctx) override;
    antlrcpp::Any visitExpr(BaseParser::ExprContext* ctx);

    antlrcpp::Any visitVarDecl(BaseParser::VarDeclContext* ctx) ;
    antlrcpp::Any visitAssignmentStatement(BaseParser::AssignmentStatementContext* ctx) override;

    antlrcpp::Any visitConditionalStatement(BaseParser::ConditionalStatementContext* ctx) override;
    antlrcpp::Any visitCondition(BaseParser::ConditionContext* ctx);
    antlrcpp::Any visitBlock(BaseParser::BlockContext* ctx) override;
    //antlrcpp::Any visitLoopStatement(BaseParser::LoopStatementContext* ctx) override;
    antlrcpp::Any visitWhileStatement(BaseParser::WhileStatementContext* ctx) override;
    

    antlrcpp::Any visitPrintExpr(BaseParser::PrintExprContext *ctx) override;
    antlrcpp::Any visitPrintStatement(BaseParser::PrintStatementContext *ctx) override;

private:
    std::unordered_map<std::string,int> symbolTable;
    int evaluate(ASTNodePtr node);

};

#endif // ASTBUILDER_H
