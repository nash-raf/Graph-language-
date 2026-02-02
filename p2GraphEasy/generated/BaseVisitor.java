// Generated from Base.g4 by ANTLR 4.13.2
import org.antlr.v4.runtime.tree.ParseTreeVisitor;

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link BaseParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
public interface BaseVisitor<T> extends ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link BaseParser#program}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitProgram(BaseParser.ProgramContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#statement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitStatement(BaseParser.StatementContext ctx);
	/**
	 * Visit a parse tree produced by the {@code WeightedGraphDef}
	 * labeled alternative in {@link BaseParser#graphDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitWeightedGraphDef(BaseParser.WeightedGraphDefContext ctx);
	/**
	 * Visit a parse tree produced by the {@code UnweightedGraphDef}
	 * labeled alternative in {@link BaseParser#graphDef}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnweightedGraphDef(BaseParser.UnweightedGraphDefContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#nodes}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNodes(BaseParser.NodesContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#edges}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEdges(BaseParser.EdgesContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#nodeList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNodeList(BaseParser.NodeListContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#edgeList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEdgeList(BaseParser.EdgeListContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#graphID}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGraphID(BaseParser.GraphIDContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#nodeID}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNodeID(BaseParser.NodeIDContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#fileEdgeList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFileEdgeList(BaseParser.FileEdgeListContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#edge}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEdge(BaseParser.EdgeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code SimpleDeclaration}
	 * labeled alternative in {@link BaseParser#varDecl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSimpleDeclaration(BaseParser.SimpleDeclarationContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ArrayDeclaration}
	 * labeled alternative in {@link BaseParser#varDecl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayDeclaration(BaseParser.ArrayDeclarationContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#conditionalStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConditionalStatement(BaseParser.ConditionalStatementContext ctx);
	/**
	 * Visit a parse tree produced by the {@code NodeCheck}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNodeCheck(BaseParser.NodeCheckContext ctx);
	/**
	 * Visit a parse tree produced by the {@code LogicalAnd}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLogicalAnd(BaseParser.LogicalAndContext ctx);
	/**
	 * Visit a parse tree produced by the {@code Relational}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRelational(BaseParser.RelationalContext ctx);
	/**
	 * Visit a parse tree produced by the {@code EdgeCheck}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEdgeCheck(BaseParser.EdgeCheckContext ctx);
	/**
	 * Visit a parse tree produced by the {@code LogicalOr}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLogicalOr(BaseParser.LogicalOrContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#graphComprehension}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGraphComprehension(BaseParser.GraphComprehensionContext ctx);
	/**
	 * Visit a parse tree produced by the {@code GraphLogicalAnd}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGraphLogicalAnd(BaseParser.GraphLogicalAndContext ctx);
	/**
	 * Visit a parse tree produced by the {@code DegreeCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitDegreeCondition(BaseParser.DegreeConditionContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ConnectedCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitConnectedCondition(BaseParser.ConnectedConditionContext ctx);
	/**
	 * Visit a parse tree produced by the {@code GraphLogicalOr}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGraphLogicalOr(BaseParser.GraphLogicalOrContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ParenGraphCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParenGraphCondition(BaseParser.ParenGraphConditionContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#loopStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLoopStatement(BaseParser.LoopStatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#foreachStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForeachStatement(BaseParser.ForeachStatementContext ctx);
	/**
	 * Visit a parse tree produced by the {@code forEachVertex}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForEachVertex(BaseParser.ForEachVertexContext ctx);
	/**
	 * Visit a parse tree produced by the {@code forEachEdge}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForEachEdge(BaseParser.ForEachEdgeContext ctx);
	/**
	 * Visit a parse tree produced by the {@code forEachAdj}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForEachAdj(BaseParser.ForEachAdjContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#whileStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitWhileStatement(BaseParser.WhileStatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#nodeEdgeOperation}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNodeEdgeOperation(BaseParser.NodeEdgeOperationContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#addOperation}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAddOperation(BaseParser.AddOperationContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#removeOperation}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRemoveOperation(BaseParser.RemoveOperationContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#addTargets}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAddTargets(BaseParser.AddTargetsContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#removeTargets}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRemoveTargets(BaseParser.RemoveTargetsContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#queryStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitQueryStatement(BaseParser.QueryStatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#showgraph}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitShowgraph(BaseParser.ShowgraphContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#function}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFunction(BaseParser.FunctionContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#returnType}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitReturnType(BaseParser.ReturnTypeContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#paramList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParamList(BaseParser.ParamListContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#param}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParam(BaseParser.ParamContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#type}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitType(BaseParser.TypeContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#functionCall}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFunctionCall(BaseParser.FunctionCallContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#argumentList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArgumentList(BaseParser.ArgumentListContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#block}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBlock(BaseParser.BlockContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#returnStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitReturnStatement(BaseParser.ReturnStatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#printStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrintStatement(BaseParser.PrintStatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#printExpr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrintExpr(BaseParser.PrintExprContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#printArrayStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrintArrayStatement(BaseParser.PrintArrayStatementContext ctx);
	/**
	 * Visit a parse tree produced by the {@code edgePrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitEdgePrint(BaseParser.EdgePrintContext ctx);
	/**
	 * Visit a parse tree produced by the {@code nodePrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitNodePrint(BaseParser.NodePrintContext ctx);
	/**
	 * Visit a parse tree produced by the {@code graphPrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitGraphPrint(BaseParser.GraphPrintContext ctx);
	/**
	 * Visit a parse tree produced by the {@code BoolTrueExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolTrueExpr(BaseParser.BoolTrueExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code MulDivExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitMulDivExpr(BaseParser.MulDivExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code IdExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIdExpr(BaseParser.IdExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code BoolFalseExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBoolFalseExpr(BaseParser.BoolFalseExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ArrayAccessExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayAccessExpr(BaseParser.ArrayAccessExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code IntExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitIntExpr(BaseParser.IntExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ParenExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitParenExpr(BaseParser.ParenExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ArrayPrint}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayPrint(BaseParser.ArrayPrintContext ctx);
	/**
	 * Visit a parse tree produced by the {@code FuncExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitFuncExpr(BaseParser.FuncExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code RealExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitRealExpr(BaseParser.RealExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code AddSubExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAddSubExpr(BaseParser.AddSubExprContext ctx);
	/**
	 * Visit a parse tree produced by the {@code SizedArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitSizedArray(BaseParser.SizedArrayContext ctx);
	/**
	 * Visit a parse tree produced by the {@code UnsizedArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitUnsizedArray(BaseParser.UnsizedArrayContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#arrayInitializer}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayInitializer(BaseParser.ArrayInitializerContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#assignmentStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitAssignmentStatement(BaseParser.AssignmentStatementContext ctx);
	/**
	 * Visit a parse tree produced by the {@code ArrayAssignStmt}
	 * labeled alternative in {@link BaseParser#arrayAssignStatement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitArrayAssignStmt(BaseParser.ArrayAssignStmtContext ctx);
	/**
	 * Visit a parse tree produced by {@link BaseParser#weights}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitWeights(BaseParser.WeightsContext ctx);
}