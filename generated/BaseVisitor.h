
// Generated from Base.g4 by ANTLR 4.10.1

#pragma once


#include "antlr4-runtime.h"
#include "BaseParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by BaseParser.
 */
class  BaseVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by BaseParser.
   */
    virtual std::any visitProgram(BaseParser::ProgramContext *context) = 0;

    virtual std::any visitStatement(BaseParser::StatementContext *context) = 0;

    virtual std::any visitGraphDef(BaseParser::GraphDefContext *context) = 0;

    virtual std::any visitNodes(BaseParser::NodesContext *context) = 0;

    virtual std::any visitEdges(BaseParser::EdgesContext *context) = 0;

    virtual std::any visitNodeList(BaseParser::NodeListContext *context) = 0;

    virtual std::any visitEdgeList(BaseParser::EdgeListContext *context) = 0;

    virtual std::any visitGraphID(BaseParser::GraphIDContext *context) = 0;

    virtual std::any visitNodeID(BaseParser::NodeIDContext *context) = 0;

    virtual std::any visitFileEdgeList(BaseParser::FileEdgeListContext *context) = 0;

    virtual std::any visitEdge(BaseParser::EdgeContext *context) = 0;

    virtual std::any visitVarDecl(BaseParser::VarDeclContext *context) = 0;

    virtual std::any visitConditionalStatement(BaseParser::ConditionalStatementContext *context) = 0;

    virtual std::any visitNodeCheck(BaseParser::NodeCheckContext *context) = 0;

    virtual std::any visitLogicalAnd(BaseParser::LogicalAndContext *context) = 0;

    virtual std::any visitRelational(BaseParser::RelationalContext *context) = 0;

    virtual std::any visitEdgeCheck(BaseParser::EdgeCheckContext *context) = 0;

    virtual std::any visitLogicalOr(BaseParser::LogicalOrContext *context) = 0;

    virtual std::any visitGraphComprehension(BaseParser::GraphComprehensionContext *context) = 0;

    virtual std::any visitGraphLogicalAnd(BaseParser::GraphLogicalAndContext *context) = 0;

    virtual std::any visitDegreeCondition(BaseParser::DegreeConditionContext *context) = 0;

    virtual std::any visitConnectedCondition(BaseParser::ConnectedConditionContext *context) = 0;

    virtual std::any visitGraphLogicalOr(BaseParser::GraphLogicalOrContext *context) = 0;

    virtual std::any visitParenGraphCondition(BaseParser::ParenGraphConditionContext *context) = 0;

    virtual std::any visitLoopStatement(BaseParser::LoopStatementContext *context) = 0;

    virtual std::any visitForeachStatement(BaseParser::ForeachStatementContext *context) = 0;

    virtual std::any visitForEachVertex(BaseParser::ForEachVertexContext *context) = 0;

    virtual std::any visitForEachEdge(BaseParser::ForEachEdgeContext *context) = 0;

    virtual std::any visitForEachAdj(BaseParser::ForEachAdjContext *context) = 0;

    virtual std::any visitWhileStatement(BaseParser::WhileStatementContext *context) = 0;

    virtual std::any visitQueryStatement(BaseParser::QueryStatementContext *context) = 0;

    virtual std::any visitShowgraph(BaseParser::ShowgraphContext *context) = 0;

    virtual std::any visitFunction(BaseParser::FunctionContext *context) = 0;

    virtual std::any visitReturnType(BaseParser::ReturnTypeContext *context) = 0;

    virtual std::any visitParamList(BaseParser::ParamListContext *context) = 0;

    virtual std::any visitParam(BaseParser::ParamContext *context) = 0;

    virtual std::any visitType(BaseParser::TypeContext *context) = 0;

    virtual std::any visitFunctionCall(BaseParser::FunctionCallContext *context) = 0;

    virtual std::any visitArgumentList(BaseParser::ArgumentListContext *context) = 0;

    virtual std::any visitBlock(BaseParser::BlockContext *context) = 0;

    virtual std::any visitReturnStatement(BaseParser::ReturnStatementContext *context) = 0;

    virtual std::any visitPrintStatement(BaseParser::PrintStatementContext *context) = 0;

    virtual std::any visitPrintExpr(BaseParser::PrintExprContext *context) = 0;

    virtual std::any visitEdgePrint(BaseParser::EdgePrintContext *context) = 0;

    virtual std::any visitNodePrint(BaseParser::NodePrintContext *context) = 0;

    virtual std::any visitGraphPrint(BaseParser::GraphPrintContext *context) = 0;

    virtual std::any visitMulDivExpr(BaseParser::MulDivExprContext *context) = 0;

    virtual std::any visitIdExpr(BaseParser::IdExprContext *context) = 0;

    virtual std::any visitIntExpr(BaseParser::IntExprContext *context) = 0;

    virtual std::any visitParenExpr(BaseParser::ParenExprContext *context) = 0;

    virtual std::any visitFuncExpr(BaseParser::FuncExprContext *context) = 0;

    virtual std::any visitAddSubExpr(BaseParser::AddSubExprContext *context) = 0;


};

