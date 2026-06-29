
// Generated from Base.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "BaseListener.h"


/**
 * This class provides an empty implementation of BaseListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  BaseBaseListener : public BaseListener {
public:

  virtual void enterProgram(BaseParser::ProgramContext * /*ctx*/) override { }
  virtual void exitProgram(BaseParser::ProgramContext * /*ctx*/) override { }

  virtual void enterStatement(BaseParser::StatementContext * /*ctx*/) override { }
  virtual void exitStatement(BaseParser::StatementContext * /*ctx*/) override { }

  virtual void enterSwapStatement(BaseParser::SwapStatementContext * /*ctx*/) override { }
  virtual void exitSwapStatement(BaseParser::SwapStatementContext * /*ctx*/) override { }

  virtual void enterWeightedGraphDef(BaseParser::WeightedGraphDefContext * /*ctx*/) override { }
  virtual void exitWeightedGraphDef(BaseParser::WeightedGraphDefContext * /*ctx*/) override { }

  virtual void enterUnweightedGraphDef(BaseParser::UnweightedGraphDefContext * /*ctx*/) override { }
  virtual void exitUnweightedGraphDef(BaseParser::UnweightedGraphDefContext * /*ctx*/) override { }

  virtual void enterGraphProperty(BaseParser::GraphPropertyContext * /*ctx*/) override { }
  virtual void exitGraphProperty(BaseParser::GraphPropertyContext * /*ctx*/) override { }

  virtual void enterBoolLiteral(BaseParser::BoolLiteralContext * /*ctx*/) override { }
  virtual void exitBoolLiteral(BaseParser::BoolLiteralContext * /*ctx*/) override { }

  virtual void enterNodes(BaseParser::NodesContext * /*ctx*/) override { }
  virtual void exitNodes(BaseParser::NodesContext * /*ctx*/) override { }

  virtual void enterEdges(BaseParser::EdgesContext * /*ctx*/) override { }
  virtual void exitEdges(BaseParser::EdgesContext * /*ctx*/) override { }

  virtual void enterNodeList(BaseParser::NodeListContext * /*ctx*/) override { }
  virtual void exitNodeList(BaseParser::NodeListContext * /*ctx*/) override { }

  virtual void enterEdgeList(BaseParser::EdgeListContext * /*ctx*/) override { }
  virtual void exitEdgeList(BaseParser::EdgeListContext * /*ctx*/) override { }

  virtual void enterGraphID(BaseParser::GraphIDContext * /*ctx*/) override { }
  virtual void exitGraphID(BaseParser::GraphIDContext * /*ctx*/) override { }

  virtual void enterNodeID(BaseParser::NodeIDContext * /*ctx*/) override { }
  virtual void exitNodeID(BaseParser::NodeIDContext * /*ctx*/) override { }

  virtual void enterFileEdgeList(BaseParser::FileEdgeListContext * /*ctx*/) override { }
  virtual void exitFileEdgeList(BaseParser::FileEdgeListContext * /*ctx*/) override { }

  virtual void enterEdge(BaseParser::EdgeContext * /*ctx*/) override { }
  virtual void exitEdge(BaseParser::EdgeContext * /*ctx*/) override { }

  virtual void enterSimpleDeclaration(BaseParser::SimpleDeclarationContext * /*ctx*/) override { }
  virtual void exitSimpleDeclaration(BaseParser::SimpleDeclarationContext * /*ctx*/) override { }

  virtual void enterArrayDeclaration(BaseParser::ArrayDeclarationContext * /*ctx*/) override { }
  virtual void exitArrayDeclaration(BaseParser::ArrayDeclarationContext * /*ctx*/) override { }

  virtual void enterSetDecl(BaseParser::SetDeclContext * /*ctx*/) override { }
  virtual void exitSetDecl(BaseParser::SetDeclContext * /*ctx*/) override { }

  virtual void enterSetInitializer(BaseParser::SetInitializerContext * /*ctx*/) override { }
  virtual void exitSetInitializer(BaseParser::SetInitializerContext * /*ctx*/) override { }

  virtual void enterSetOperation(BaseParser::SetOperationContext * /*ctx*/) override { }
  virtual void exitSetOperation(BaseParser::SetOperationContext * /*ctx*/) override { }

  virtual void enterSetTarget(BaseParser::SetTargetContext * /*ctx*/) override { }
  virtual void exitSetTarget(BaseParser::SetTargetContext * /*ctx*/) override { }

  virtual void enterSetUnion(BaseParser::SetUnionContext * /*ctx*/) override { }
  virtual void exitSetUnion(BaseParser::SetUnionContext * /*ctx*/) override { }

  virtual void enterGraphEdgesSet(BaseParser::GraphEdgesSetContext * /*ctx*/) override { }
  virtual void exitGraphEdgesSet(BaseParser::GraphEdgesSetContext * /*ctx*/) override { }

  virtual void enterSetIntersect(BaseParser::SetIntersectContext * /*ctx*/) override { }
  virtual void exitSetIntersect(BaseParser::SetIntersectContext * /*ctx*/) override { }

  virtual void enterSetId(BaseParser::SetIdContext * /*ctx*/) override { }
  virtual void exitSetId(BaseParser::SetIdContext * /*ctx*/) override { }

  virtual void enterSetLiteral(BaseParser::SetLiteralContext * /*ctx*/) override { }
  virtual void exitSetLiteral(BaseParser::SetLiteralContext * /*ctx*/) override { }

  virtual void enterParenSet(BaseParser::ParenSetContext * /*ctx*/) override { }
  virtual void exitParenSet(BaseParser::ParenSetContext * /*ctx*/) override { }

  virtual void enterGraphNodesSet(BaseParser::GraphNodesSetContext * /*ctx*/) override { }
  virtual void exitGraphNodesSet(BaseParser::GraphNodesSetContext * /*ctx*/) override { }

  virtual void enterSetAddMethod(BaseParser::SetAddMethodContext * /*ctx*/) override { }
  virtual void exitSetAddMethod(BaseParser::SetAddMethodContext * /*ctx*/) override { }

  virtual void enterSetRemoveMethod(BaseParser::SetRemoveMethodContext * /*ctx*/) override { }
  virtual void exitSetRemoveMethod(BaseParser::SetRemoveMethodContext * /*ctx*/) override { }

  virtual void enterConditionalStatement(BaseParser::ConditionalStatementContext * /*ctx*/) override { }
  virtual void exitConditionalStatement(BaseParser::ConditionalStatementContext * /*ctx*/) override { }

  virtual void enterExprCondition(BaseParser::ExprConditionContext * /*ctx*/) override { }
  virtual void exitExprCondition(BaseParser::ExprConditionContext * /*ctx*/) override { }

  virtual void enterNodeCheck(BaseParser::NodeCheckContext * /*ctx*/) override { }
  virtual void exitNodeCheck(BaseParser::NodeCheckContext * /*ctx*/) override { }

  virtual void enterLogicalAnd(BaseParser::LogicalAndContext * /*ctx*/) override { }
  virtual void exitLogicalAnd(BaseParser::LogicalAndContext * /*ctx*/) override { }

  virtual void enterRelational(BaseParser::RelationalContext * /*ctx*/) override { }
  virtual void exitRelational(BaseParser::RelationalContext * /*ctx*/) override { }

  virtual void enterEdgeCheck(BaseParser::EdgeCheckContext * /*ctx*/) override { }
  virtual void exitEdgeCheck(BaseParser::EdgeCheckContext * /*ctx*/) override { }

  virtual void enterLogicalOr(BaseParser::LogicalOrContext * /*ctx*/) override { }
  virtual void exitLogicalOr(BaseParser::LogicalOrContext * /*ctx*/) override { }

  virtual void enterGraphComprehension(BaseParser::GraphComprehensionContext * /*ctx*/) override { }
  virtual void exitGraphComprehension(BaseParser::GraphComprehensionContext * /*ctx*/) override { }

  virtual void enterGraphExpr(BaseParser::GraphExprContext * /*ctx*/) override { }
  virtual void exitGraphExpr(BaseParser::GraphExprContext * /*ctx*/) override { }

  virtual void enterGraphLogicalAnd(BaseParser::GraphLogicalAndContext * /*ctx*/) override { }
  virtual void exitGraphLogicalAnd(BaseParser::GraphLogicalAndContext * /*ctx*/) override { }

  virtual void enterDegreeCondition(BaseParser::DegreeConditionContext * /*ctx*/) override { }
  virtual void exitDegreeCondition(BaseParser::DegreeConditionContext * /*ctx*/) override { }

  virtual void enterEdgeHasCondition(BaseParser::EdgeHasConditionContext * /*ctx*/) override { }
  virtual void exitEdgeHasCondition(BaseParser::EdgeHasConditionContext * /*ctx*/) override { }

  virtual void enterCycleCondition(BaseParser::CycleConditionContext * /*ctx*/) override { }
  virtual void exitCycleCondition(BaseParser::CycleConditionContext * /*ctx*/) override { }

  virtual void enterConnectedCondition(BaseParser::ConnectedConditionContext * /*ctx*/) override { }
  virtual void exitConnectedCondition(BaseParser::ConnectedConditionContext * /*ctx*/) override { }

  virtual void enterGraphLogicalOr(BaseParser::GraphLogicalOrContext * /*ctx*/) override { }
  virtual void exitGraphLogicalOr(BaseParser::GraphLogicalOrContext * /*ctx*/) override { }

  virtual void enterParenGraphCondition(BaseParser::ParenGraphConditionContext * /*ctx*/) override { }
  virtual void exitParenGraphCondition(BaseParser::ParenGraphConditionContext * /*ctx*/) override { }

  virtual void enterVertexInSetCondition(BaseParser::VertexInSetConditionContext * /*ctx*/) override { }
  virtual void exitVertexInSetCondition(BaseParser::VertexInSetConditionContext * /*ctx*/) override { }

  virtual void enterLoopStatement(BaseParser::LoopStatementContext * /*ctx*/) override { }
  virtual void exitLoopStatement(BaseParser::LoopStatementContext * /*ctx*/) override { }

  virtual void enterForeachStatement(BaseParser::ForeachStatementContext * /*ctx*/) override { }
  virtual void exitForeachStatement(BaseParser::ForeachStatementContext * /*ctx*/) override { }

  virtual void enterForEachVertex(BaseParser::ForEachVertexContext * /*ctx*/) override { }
  virtual void exitForEachVertex(BaseParser::ForEachVertexContext * /*ctx*/) override { }

  virtual void enterForEachEdge(BaseParser::ForEachEdgeContext * /*ctx*/) override { }
  virtual void exitForEachEdge(BaseParser::ForEachEdgeContext * /*ctx*/) override { }

  virtual void enterForEachOutAdj(BaseParser::ForEachOutAdjContext * /*ctx*/) override { }
  virtual void exitForEachOutAdj(BaseParser::ForEachOutAdjContext * /*ctx*/) override { }

  virtual void enterForEachInAdj(BaseParser::ForEachInAdjContext * /*ctx*/) override { }
  virtual void exitForEachInAdj(BaseParser::ForEachInAdjContext * /*ctx*/) override { }

  virtual void enterForEachAdj(BaseParser::ForEachAdjContext * /*ctx*/) override { }
  virtual void exitForEachAdj(BaseParser::ForEachAdjContext * /*ctx*/) override { }

  virtual void enterForEachElement(BaseParser::ForEachElementContext * /*ctx*/) override { }
  virtual void exitForEachElement(BaseParser::ForEachElementContext * /*ctx*/) override { }

  virtual void enterForEachPlain(BaseParser::ForEachPlainContext * /*ctx*/) override { }
  virtual void exitForEachPlain(BaseParser::ForEachPlainContext * /*ctx*/) override { }

  virtual void enterWhileStatement(BaseParser::WhileStatementContext * /*ctx*/) override { }
  virtual void exitWhileStatement(BaseParser::WhileStatementContext * /*ctx*/) override { }

  virtual void enterNodeEdgeOperation(BaseParser::NodeEdgeOperationContext * /*ctx*/) override { }
  virtual void exitNodeEdgeOperation(BaseParser::NodeEdgeOperationContext * /*ctx*/) override { }

  virtual void enterAddOperation(BaseParser::AddOperationContext * /*ctx*/) override { }
  virtual void exitAddOperation(BaseParser::AddOperationContext * /*ctx*/) override { }

  virtual void enterRemoveOperation(BaseParser::RemoveOperationContext * /*ctx*/) override { }
  virtual void exitRemoveOperation(BaseParser::RemoveOperationContext * /*ctx*/) override { }

  virtual void enterAddTargets(BaseParser::AddTargetsContext * /*ctx*/) override { }
  virtual void exitAddTargets(BaseParser::AddTargetsContext * /*ctx*/) override { }

  virtual void enterRemoveTargets(BaseParser::RemoveTargetsContext * /*ctx*/) override { }
  virtual void exitRemoveTargets(BaseParser::RemoveTargetsContext * /*ctx*/) override { }

  virtual void enterQueryStatement(BaseParser::QueryStatementContext * /*ctx*/) override { }
  virtual void exitQueryStatement(BaseParser::QueryStatementContext * /*ctx*/) override { }

  virtual void enterShowgraph(BaseParser::ShowgraphContext * /*ctx*/) override { }
  virtual void exitShowgraph(BaseParser::ShowgraphContext * /*ctx*/) override { }

  virtual void enterFunction(BaseParser::FunctionContext * /*ctx*/) override { }
  virtual void exitFunction(BaseParser::FunctionContext * /*ctx*/) override { }

  virtual void enterReturnType(BaseParser::ReturnTypeContext * /*ctx*/) override { }
  virtual void exitReturnType(BaseParser::ReturnTypeContext * /*ctx*/) override { }

  virtual void enterParamList(BaseParser::ParamListContext * /*ctx*/) override { }
  virtual void exitParamList(BaseParser::ParamListContext * /*ctx*/) override { }

  virtual void enterParam(BaseParser::ParamContext * /*ctx*/) override { }
  virtual void exitParam(BaseParser::ParamContext * /*ctx*/) override { }

  virtual void enterType(BaseParser::TypeContext * /*ctx*/) override { }
  virtual void exitType(BaseParser::TypeContext * /*ctx*/) override { }

  virtual void enterFunctionCall(BaseParser::FunctionCallContext * /*ctx*/) override { }
  virtual void exitFunctionCall(BaseParser::FunctionCallContext * /*ctx*/) override { }

  virtual void enterFunctionName(BaseParser::FunctionNameContext * /*ctx*/) override { }
  virtual void exitFunctionName(BaseParser::FunctionNameContext * /*ctx*/) override { }

  virtual void enterArgumentList(BaseParser::ArgumentListContext * /*ctx*/) override { }
  virtual void exitArgumentList(BaseParser::ArgumentListContext * /*ctx*/) override { }

  virtual void enterSleepStatement(BaseParser::SleepStatementContext * /*ctx*/) override { }
  virtual void exitSleepStatement(BaseParser::SleepStatementContext * /*ctx*/) override { }

  virtual void enterBlock(BaseParser::BlockContext * /*ctx*/) override { }
  virtual void exitBlock(BaseParser::BlockContext * /*ctx*/) override { }

  virtual void enterReturnStatement(BaseParser::ReturnStatementContext * /*ctx*/) override { }
  virtual void exitReturnStatement(BaseParser::ReturnStatementContext * /*ctx*/) override { }

  virtual void enterBreakStatement(BaseParser::BreakStatementContext * /*ctx*/) override { }
  virtual void exitBreakStatement(BaseParser::BreakStatementContext * /*ctx*/) override { }

  virtual void enterContinueStatement(BaseParser::ContinueStatementContext * /*ctx*/) override { }
  virtual void exitContinueStatement(BaseParser::ContinueStatementContext * /*ctx*/) override { }

  virtual void enterPrintStatement(BaseParser::PrintStatementContext * /*ctx*/) override { }
  virtual void exitPrintStatement(BaseParser::PrintStatementContext * /*ctx*/) override { }

  virtual void enterPrintExpr(BaseParser::PrintExprContext * /*ctx*/) override { }
  virtual void exitPrintExpr(BaseParser::PrintExprContext * /*ctx*/) override { }

  virtual void enterPrintArrayStatement(BaseParser::PrintArrayStatementContext * /*ctx*/) override { }
  virtual void exitPrintArrayStatement(BaseParser::PrintArrayStatementContext * /*ctx*/) override { }

  virtual void enterEdgePrint(BaseParser::EdgePrintContext * /*ctx*/) override { }
  virtual void exitEdgePrint(BaseParser::EdgePrintContext * /*ctx*/) override { }

  virtual void enterNodePrint(BaseParser::NodePrintContext * /*ctx*/) override { }
  virtual void exitNodePrint(BaseParser::NodePrintContext * /*ctx*/) override { }

  virtual void enterGraphPrint(BaseParser::GraphPrintContext * /*ctx*/) override { }
  virtual void exitGraphPrint(BaseParser::GraphPrintContext * /*ctx*/) override { }

  virtual void enterBoolTrueExpr(BaseParser::BoolTrueExprContext * /*ctx*/) override { }
  virtual void exitBoolTrueExpr(BaseParser::BoolTrueExprContext * /*ctx*/) override { }

  virtual void enterTimerExpr(BaseParser::TimerExprContext * /*ctx*/) override { }
  virtual void exitTimerExpr(BaseParser::TimerExprContext * /*ctx*/) override { }

  virtual void enterIdExpr(BaseParser::IdExprContext * /*ctx*/) override { }
  virtual void exitIdExpr(BaseParser::IdExprContext * /*ctx*/) override { }

  virtual void enterBoolFalseExpr(BaseParser::BoolFalseExprContext * /*ctx*/) override { }
  virtual void exitBoolFalseExpr(BaseParser::BoolFalseExprContext * /*ctx*/) override { }

  virtual void enterArrayAccessExpr(BaseParser::ArrayAccessExprContext * /*ctx*/) override { }
  virtual void exitArrayAccessExpr(BaseParser::ArrayAccessExprContext * /*ctx*/) override { }

  virtual void enterSetLitExpr(BaseParser::SetLitExprContext * /*ctx*/) override { }
  virtual void exitSetLitExpr(BaseParser::SetLitExprContext * /*ctx*/) override { }

  virtual void enterSetSizeExpr(BaseParser::SetSizeExprContext * /*ctx*/) override { }
  virtual void exitSetSizeExpr(BaseParser::SetSizeExprContext * /*ctx*/) override { }

  virtual void enterArray2DAccessExpr(BaseParser::Array2DAccessExprContext * /*ctx*/) override { }
  virtual void exitArray2DAccessExpr(BaseParser::Array2DAccessExprContext * /*ctx*/) override { }

  virtual void enterArrayPrint(BaseParser::ArrayPrintContext * /*ctx*/) override { }
  virtual void exitArrayPrint(BaseParser::ArrayPrintContext * /*ctx*/) override { }

  virtual void enterInfExpr(BaseParser::InfExprContext * /*ctx*/) override { }
  virtual void exitInfExpr(BaseParser::InfExprContext * /*ctx*/) override { }

  virtual void enterFuncExpr(BaseParser::FuncExprContext * /*ctx*/) override { }
  virtual void exitFuncExpr(BaseParser::FuncExprContext * /*ctx*/) override { }

  virtual void enterSetContainsExpr(BaseParser::SetContainsExprContext * /*ctx*/) override { }
  virtual void exitSetContainsExpr(BaseParser::SetContainsExprContext * /*ctx*/) override { }

  virtual void enterMulDivExpr(BaseParser::MulDivExprContext * /*ctx*/) override { }
  virtual void exitMulDivExpr(BaseParser::MulDivExprContext * /*ctx*/) override { }

  virtual void enterNotExpr(BaseParser::NotExprContext * /*ctx*/) override { }
  virtual void exitNotExpr(BaseParser::NotExprContext * /*ctx*/) override { }

  virtual void enterIntExpr(BaseParser::IntExprContext * /*ctx*/) override { }
  virtual void exitIntExpr(BaseParser::IntExprContext * /*ctx*/) override { }

  virtual void enterSetPopExpr(BaseParser::SetPopExprContext * /*ctx*/) override { }
  virtual void exitSetPopExpr(BaseParser::SetPopExprContext * /*ctx*/) override { }

  virtual void enterParenExpr(BaseParser::ParenExprContext * /*ctx*/) override { }
  virtual void exitParenExpr(BaseParser::ParenExprContext * /*ctx*/) override { }

  virtual void enterRealExpr(BaseParser::RealExprContext * /*ctx*/) override { }
  virtual void exitRealExpr(BaseParser::RealExprContext * /*ctx*/) override { }

  virtual void enterLogicalExpr(BaseParser::LogicalExprContext * /*ctx*/) override { }
  virtual void exitLogicalExpr(BaseParser::LogicalExprContext * /*ctx*/) override { }

  virtual void enterAddSubExpr(BaseParser::AddSubExprContext * /*ctx*/) override { }
  virtual void exitAddSubExpr(BaseParser::AddSubExprContext * /*ctx*/) override { }

  virtual void enterUnaryMinusExpr(BaseParser::UnaryMinusExprContext * /*ctx*/) override { }
  virtual void exitUnaryMinusExpr(BaseParser::UnaryMinusExprContext * /*ctx*/) override { }

  virtual void enterSized2DArray(BaseParser::Sized2DArrayContext * /*ctx*/) override { }
  virtual void exitSized2DArray(BaseParser::Sized2DArrayContext * /*ctx*/) override { }

  virtual void enterSizedArray(BaseParser::SizedArrayContext * /*ctx*/) override { }
  virtual void exitSizedArray(BaseParser::SizedArrayContext * /*ctx*/) override { }

  virtual void enterUnsizedArray(BaseParser::UnsizedArrayContext * /*ctx*/) override { }
  virtual void exitUnsizedArray(BaseParser::UnsizedArrayContext * /*ctx*/) override { }

  virtual void enterArrayInitializer(BaseParser::ArrayInitializerContext * /*ctx*/) override { }
  virtual void exitArrayInitializer(BaseParser::ArrayInitializerContext * /*ctx*/) override { }

  virtual void enterAssignmentStatement(BaseParser::AssignmentStatementContext * /*ctx*/) override { }
  virtual void exitAssignmentStatement(BaseParser::AssignmentStatementContext * /*ctx*/) override { }

  virtual void enterArray2DAssignStmt(BaseParser::Array2DAssignStmtContext * /*ctx*/) override { }
  virtual void exitArray2DAssignStmt(BaseParser::Array2DAssignStmtContext * /*ctx*/) override { }

  virtual void enterArrayAssignStmt(BaseParser::ArrayAssignStmtContext * /*ctx*/) override { }
  virtual void exitArrayAssignStmt(BaseParser::ArrayAssignStmtContext * /*ctx*/) override { }

  virtual void enterWeights(BaseParser::WeightsContext * /*ctx*/) override { }
  virtual void exitWeights(BaseParser::WeightsContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

