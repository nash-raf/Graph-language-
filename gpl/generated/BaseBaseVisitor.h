
// Generated from Base.g4 by ANTLR 4.13.0

#pragma once


#include "antlr4-runtime.h"
#include "BaseVisitor.h"


/**
 * This class provides an empty implementation of BaseVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  BaseBaseVisitor : public BaseVisitor {
public:

  virtual std::any visitProgram(BaseParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(BaseParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphDef(BaseParser::GraphDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUndirectedEdge(BaseParser::UndirectedEdgeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUndirectedEdgeList(BaseParser::UndirectedEdgeListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUndirectedGraphDef(BaseParser::UndirectedGraphDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUndirectedEdges(BaseParser::UndirectedEdgesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNodes(BaseParser::NodesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEdges(BaseParser::EdgesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNodeList(BaseParser::NodeListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEdgeList(BaseParser::EdgeListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphID(BaseParser::GraphIDContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNodeID(BaseParser::NodeIDContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFileEdgeList(BaseParser::FileEdgeListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEdge(BaseParser::EdgeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimpleDeclaration(BaseParser::SimpleDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayDeclaration(BaseParser::ArrayDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalStatement(BaseParser::ConditionalStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNodeCheck(BaseParser::NodeCheckContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAnd(BaseParser::LogicalAndContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelational(BaseParser::RelationalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEdgeCheck(BaseParser::EdgeCheckContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOr(BaseParser::LogicalOrContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphComprehension(BaseParser::GraphComprehensionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphLogicalAnd(BaseParser::GraphLogicalAndContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDegreeCondition(BaseParser::DegreeConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConnectedCondition(BaseParser::ConnectedConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphLogicalOr(BaseParser::GraphLogicalOrContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParenGraphCondition(BaseParser::ParenGraphConditionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLoopStatement(BaseParser::LoopStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeachStatement(BaseParser::ForeachStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForEachVertex(BaseParser::ForEachVertexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForEachEdge(BaseParser::ForEachEdgeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForEachAdj(BaseParser::ForEachAdjContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStatement(BaseParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNodeEdgeOperation(BaseParser::NodeEdgeOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddOperation(BaseParser::AddOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRemoveOperation(BaseParser::RemoveOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddTargets(BaseParser::AddTargetsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRemoveTargets(BaseParser::RemoveTargetsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQueryStatement(BaseParser::QueryStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShowgraph(BaseParser::ShowgraphContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction(BaseParser::FunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnType(BaseParser::ReturnTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParamList(BaseParser::ParamListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParam(BaseParser::ParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType(BaseParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionCall(BaseParser::FunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArgumentList(BaseParser::ArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(BaseParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnStatement(BaseParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrintStatement(BaseParser::PrintStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrintExpr(BaseParser::PrintExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEdgePrint(BaseParser::EdgePrintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNodePrint(BaseParser::NodePrintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphPrint(BaseParser::GraphPrintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBoolTrueExpr(BaseParser::BoolTrueExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulDivExpr(BaseParser::MulDivExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdExpr(BaseParser::IdExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBoolFalseExpr(BaseParser::BoolFalseExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayAccessExpr(BaseParser::ArrayAccessExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray2DAccessExpr(BaseParser::Array2DAccessExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIntExpr(BaseParser::IntExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParenExpr(BaseParser::ParenExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncExpr(BaseParser::FuncExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRealExpr(BaseParser::RealExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddSubExpr(BaseParser::AddSubExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSized2DArray(BaseParser::Sized2DArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnsized2DArray(BaseParser::Unsized2DArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSizedArray(BaseParser::SizedArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnsizedArray(BaseParser::UnsizedArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeclaration(BaseParser::DeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayInit1D(BaseParser::ArrayInit1DContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayInit2D(BaseParser::ArrayInit2DContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayAssignStmt(BaseParser::ArrayAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray2DAssignStmt(BaseParser::Array2DAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }


};

