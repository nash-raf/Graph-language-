
// Generated from Base.g4 by ANTLR 4.13.0

#pragma once


#include "antlr4-runtime.h"
#include "BaseParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by BaseParser.
 */
class  BaseListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterProgram(BaseParser::ProgramContext *ctx) = 0;
  virtual void exitProgram(BaseParser::ProgramContext *ctx) = 0;

  virtual void enterStatement(BaseParser::StatementContext *ctx) = 0;
  virtual void exitStatement(BaseParser::StatementContext *ctx) = 0;

  virtual void enterWeightedGraphDef(BaseParser::WeightedGraphDefContext *ctx) = 0;
  virtual void exitWeightedGraphDef(BaseParser::WeightedGraphDefContext *ctx) = 0;

  virtual void enterUnweightedGraphDef(BaseParser::UnweightedGraphDefContext *ctx) = 0;
  virtual void exitUnweightedGraphDef(BaseParser::UnweightedGraphDefContext *ctx) = 0;

  virtual void enterNodes(BaseParser::NodesContext *ctx) = 0;
  virtual void exitNodes(BaseParser::NodesContext *ctx) = 0;

  virtual void enterEdges(BaseParser::EdgesContext *ctx) = 0;
  virtual void exitEdges(BaseParser::EdgesContext *ctx) = 0;

  virtual void enterNodeList(BaseParser::NodeListContext *ctx) = 0;
  virtual void exitNodeList(BaseParser::NodeListContext *ctx) = 0;

  virtual void enterEdgeList(BaseParser::EdgeListContext *ctx) = 0;
  virtual void exitEdgeList(BaseParser::EdgeListContext *ctx) = 0;

  virtual void enterGraphID(BaseParser::GraphIDContext *ctx) = 0;
  virtual void exitGraphID(BaseParser::GraphIDContext *ctx) = 0;

  virtual void enterNodeID(BaseParser::NodeIDContext *ctx) = 0;
  virtual void exitNodeID(BaseParser::NodeIDContext *ctx) = 0;

  virtual void enterFileEdgeList(BaseParser::FileEdgeListContext *ctx) = 0;
  virtual void exitFileEdgeList(BaseParser::FileEdgeListContext *ctx) = 0;

  virtual void enterEdge(BaseParser::EdgeContext *ctx) = 0;
  virtual void exitEdge(BaseParser::EdgeContext *ctx) = 0;

  virtual void enterSimpleDeclaration(BaseParser::SimpleDeclarationContext *ctx) = 0;
  virtual void exitSimpleDeclaration(BaseParser::SimpleDeclarationContext *ctx) = 0;

  virtual void enterArrayDeclaration(BaseParser::ArrayDeclarationContext *ctx) = 0;
  virtual void exitArrayDeclaration(BaseParser::ArrayDeclarationContext *ctx) = 0;

  virtual void enterConditionalStatement(BaseParser::ConditionalStatementContext *ctx) = 0;
  virtual void exitConditionalStatement(BaseParser::ConditionalStatementContext *ctx) = 0;

  virtual void enterExprCondition(BaseParser::ExprConditionContext *ctx) = 0;
  virtual void exitExprCondition(BaseParser::ExprConditionContext *ctx) = 0;

  virtual void enterNodeCheck(BaseParser::NodeCheckContext *ctx) = 0;
  virtual void exitNodeCheck(BaseParser::NodeCheckContext *ctx) = 0;

  virtual void enterLogicalAnd(BaseParser::LogicalAndContext *ctx) = 0;
  virtual void exitLogicalAnd(BaseParser::LogicalAndContext *ctx) = 0;

  virtual void enterRelational(BaseParser::RelationalContext *ctx) = 0;
  virtual void exitRelational(BaseParser::RelationalContext *ctx) = 0;

  virtual void enterEdgeCheck(BaseParser::EdgeCheckContext *ctx) = 0;
  virtual void exitEdgeCheck(BaseParser::EdgeCheckContext *ctx) = 0;

  virtual void enterLogicalOr(BaseParser::LogicalOrContext *ctx) = 0;
  virtual void exitLogicalOr(BaseParser::LogicalOrContext *ctx) = 0;

  virtual void enterGraphComprehension(BaseParser::GraphComprehensionContext *ctx) = 0;
  virtual void exitGraphComprehension(BaseParser::GraphComprehensionContext *ctx) = 0;

  virtual void enterGraphLogicalAnd(BaseParser::GraphLogicalAndContext *ctx) = 0;
  virtual void exitGraphLogicalAnd(BaseParser::GraphLogicalAndContext *ctx) = 0;

  virtual void enterDegreeCondition(BaseParser::DegreeConditionContext *ctx) = 0;
  virtual void exitDegreeCondition(BaseParser::DegreeConditionContext *ctx) = 0;

  virtual void enterConnectedCondition(BaseParser::ConnectedConditionContext *ctx) = 0;
  virtual void exitConnectedCondition(BaseParser::ConnectedConditionContext *ctx) = 0;

  virtual void enterGraphLogicalOr(BaseParser::GraphLogicalOrContext *ctx) = 0;
  virtual void exitGraphLogicalOr(BaseParser::GraphLogicalOrContext *ctx) = 0;

  virtual void enterParenGraphCondition(BaseParser::ParenGraphConditionContext *ctx) = 0;
  virtual void exitParenGraphCondition(BaseParser::ParenGraphConditionContext *ctx) = 0;

  virtual void enterLoopStatement(BaseParser::LoopStatementContext *ctx) = 0;
  virtual void exitLoopStatement(BaseParser::LoopStatementContext *ctx) = 0;

  virtual void enterForeachStatement(BaseParser::ForeachStatementContext *ctx) = 0;
  virtual void exitForeachStatement(BaseParser::ForeachStatementContext *ctx) = 0;

  virtual void enterForEachVertex(BaseParser::ForEachVertexContext *ctx) = 0;
  virtual void exitForEachVertex(BaseParser::ForEachVertexContext *ctx) = 0;

  virtual void enterForEachEdge(BaseParser::ForEachEdgeContext *ctx) = 0;
  virtual void exitForEachEdge(BaseParser::ForEachEdgeContext *ctx) = 0;

  virtual void enterForEachAdj(BaseParser::ForEachAdjContext *ctx) = 0;
  virtual void exitForEachAdj(BaseParser::ForEachAdjContext *ctx) = 0;

  virtual void enterWhileStatement(BaseParser::WhileStatementContext *ctx) = 0;
  virtual void exitWhileStatement(BaseParser::WhileStatementContext *ctx) = 0;

  virtual void enterNodeEdgeOperation(BaseParser::NodeEdgeOperationContext *ctx) = 0;
  virtual void exitNodeEdgeOperation(BaseParser::NodeEdgeOperationContext *ctx) = 0;

  virtual void enterAddOperation(BaseParser::AddOperationContext *ctx) = 0;
  virtual void exitAddOperation(BaseParser::AddOperationContext *ctx) = 0;

  virtual void enterRemoveOperation(BaseParser::RemoveOperationContext *ctx) = 0;
  virtual void exitRemoveOperation(BaseParser::RemoveOperationContext *ctx) = 0;

  virtual void enterAddTargets(BaseParser::AddTargetsContext *ctx) = 0;
  virtual void exitAddTargets(BaseParser::AddTargetsContext *ctx) = 0;

  virtual void enterRemoveTargets(BaseParser::RemoveTargetsContext *ctx) = 0;
  virtual void exitRemoveTargets(BaseParser::RemoveTargetsContext *ctx) = 0;

  virtual void enterQueryStatement(BaseParser::QueryStatementContext *ctx) = 0;
  virtual void exitQueryStatement(BaseParser::QueryStatementContext *ctx) = 0;

  virtual void enterShowgraph(BaseParser::ShowgraphContext *ctx) = 0;
  virtual void exitShowgraph(BaseParser::ShowgraphContext *ctx) = 0;

  virtual void enterFunction(BaseParser::FunctionContext *ctx) = 0;
  virtual void exitFunction(BaseParser::FunctionContext *ctx) = 0;

  virtual void enterReturnType(BaseParser::ReturnTypeContext *ctx) = 0;
  virtual void exitReturnType(BaseParser::ReturnTypeContext *ctx) = 0;

  virtual void enterParamList(BaseParser::ParamListContext *ctx) = 0;
  virtual void exitParamList(BaseParser::ParamListContext *ctx) = 0;

  virtual void enterParam(BaseParser::ParamContext *ctx) = 0;
  virtual void exitParam(BaseParser::ParamContext *ctx) = 0;

  virtual void enterType(BaseParser::TypeContext *ctx) = 0;
  virtual void exitType(BaseParser::TypeContext *ctx) = 0;

  virtual void enterFunctionCall(BaseParser::FunctionCallContext *ctx) = 0;
  virtual void exitFunctionCall(BaseParser::FunctionCallContext *ctx) = 0;

  virtual void enterArgumentList(BaseParser::ArgumentListContext *ctx) = 0;
  virtual void exitArgumentList(BaseParser::ArgumentListContext *ctx) = 0;

  virtual void enterBlock(BaseParser::BlockContext *ctx) = 0;
  virtual void exitBlock(BaseParser::BlockContext *ctx) = 0;

  virtual void enterReturnStatement(BaseParser::ReturnStatementContext *ctx) = 0;
  virtual void exitReturnStatement(BaseParser::ReturnStatementContext *ctx) = 0;

  virtual void enterPrintStatement(BaseParser::PrintStatementContext *ctx) = 0;
  virtual void exitPrintStatement(BaseParser::PrintStatementContext *ctx) = 0;

  virtual void enterPrintExpr(BaseParser::PrintExprContext *ctx) = 0;
  virtual void exitPrintExpr(BaseParser::PrintExprContext *ctx) = 0;

  virtual void enterPrintArrayStatement(BaseParser::PrintArrayStatementContext *ctx) = 0;
  virtual void exitPrintArrayStatement(BaseParser::PrintArrayStatementContext *ctx) = 0;

  virtual void enterEdgePrint(BaseParser::EdgePrintContext *ctx) = 0;
  virtual void exitEdgePrint(BaseParser::EdgePrintContext *ctx) = 0;

  virtual void enterNodePrint(BaseParser::NodePrintContext *ctx) = 0;
  virtual void exitNodePrint(BaseParser::NodePrintContext *ctx) = 0;

  virtual void enterGraphPrint(BaseParser::GraphPrintContext *ctx) = 0;
  virtual void exitGraphPrint(BaseParser::GraphPrintContext *ctx) = 0;

  virtual void enterBoolTrueExpr(BaseParser::BoolTrueExprContext *ctx) = 0;
  virtual void exitBoolTrueExpr(BaseParser::BoolTrueExprContext *ctx) = 0;

  virtual void enterMulDivExpr(BaseParser::MulDivExprContext *ctx) = 0;
  virtual void exitMulDivExpr(BaseParser::MulDivExprContext *ctx) = 0;

  virtual void enterIdExpr(BaseParser::IdExprContext *ctx) = 0;
  virtual void exitIdExpr(BaseParser::IdExprContext *ctx) = 0;

  virtual void enterBoolFalseExpr(BaseParser::BoolFalseExprContext *ctx) = 0;
  virtual void exitBoolFalseExpr(BaseParser::BoolFalseExprContext *ctx) = 0;

  virtual void enterArrayAccessExpr(BaseParser::ArrayAccessExprContext *ctx) = 0;
  virtual void exitArrayAccessExpr(BaseParser::ArrayAccessExprContext *ctx) = 0;

  virtual void enterIntExpr(BaseParser::IntExprContext *ctx) = 0;
  virtual void exitIntExpr(BaseParser::IntExprContext *ctx) = 0;

  virtual void enterParenExpr(BaseParser::ParenExprContext *ctx) = 0;
  virtual void exitParenExpr(BaseParser::ParenExprContext *ctx) = 0;

  virtual void enterArrayPrint(BaseParser::ArrayPrintContext *ctx) = 0;
  virtual void exitArrayPrint(BaseParser::ArrayPrintContext *ctx) = 0;

  virtual void enterFuncExpr(BaseParser::FuncExprContext *ctx) = 0;
  virtual void exitFuncExpr(BaseParser::FuncExprContext *ctx) = 0;

  virtual void enterRealExpr(BaseParser::RealExprContext *ctx) = 0;
  virtual void exitRealExpr(BaseParser::RealExprContext *ctx) = 0;

  virtual void enterLogicalExpr(BaseParser::LogicalExprContext *ctx) = 0;
  virtual void exitLogicalExpr(BaseParser::LogicalExprContext *ctx) = 0;

  virtual void enterAddSubExpr(BaseParser::AddSubExprContext *ctx) = 0;
  virtual void exitAddSubExpr(BaseParser::AddSubExprContext *ctx) = 0;

  virtual void enterSizedArray(BaseParser::SizedArrayContext *ctx) = 0;
  virtual void exitSizedArray(BaseParser::SizedArrayContext *ctx) = 0;

  virtual void enterUnsizedArray(BaseParser::UnsizedArrayContext *ctx) = 0;
  virtual void exitUnsizedArray(BaseParser::UnsizedArrayContext *ctx) = 0;

  virtual void enterArrayInitializer(BaseParser::ArrayInitializerContext *ctx) = 0;
  virtual void exitArrayInitializer(BaseParser::ArrayInitializerContext *ctx) = 0;

  virtual void enterAssignmentStatement(BaseParser::AssignmentStatementContext *ctx) = 0;
  virtual void exitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx) = 0;

  virtual void enterArrayAssignStmt(BaseParser::ArrayAssignStmtContext *ctx) = 0;
  virtual void exitArrayAssignStmt(BaseParser::ArrayAssignStmtContext *ctx) = 0;

  virtual void enterWeights(BaseParser::WeightsContext *ctx) = 0;
  virtual void exitWeights(BaseParser::WeightsContext *ctx) = 0;


};

