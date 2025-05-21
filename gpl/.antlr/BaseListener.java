// Generated from /Users/washioferdousrubai/Documents/VS code/cpp/graph language/Graph-language-/gpl/Base.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link BaseParser}.
 */
public interface BaseListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link BaseParser#program}.
	 * @param ctx the parse tree
	 */
	void enterProgram(BaseParser.ProgramContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#program}.
	 * @param ctx the parse tree
	 */
	void exitProgram(BaseParser.ProgramContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterStatement(BaseParser.StatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitStatement(BaseParser.StatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#graphDef}.
	 * @param ctx the parse tree
	 */
	void enterGraphDef(BaseParser.GraphDefContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#graphDef}.
	 * @param ctx the parse tree
	 */
	void exitGraphDef(BaseParser.GraphDefContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#nodes}.
	 * @param ctx the parse tree
	 */
	void enterNodes(BaseParser.NodesContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#nodes}.
	 * @param ctx the parse tree
	 */
	void exitNodes(BaseParser.NodesContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#edges}.
	 * @param ctx the parse tree
	 */
	void enterEdges(BaseParser.EdgesContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#edges}.
	 * @param ctx the parse tree
	 */
	void exitEdges(BaseParser.EdgesContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#nodeList}.
	 * @param ctx the parse tree
	 */
	void enterNodeList(BaseParser.NodeListContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#nodeList}.
	 * @param ctx the parse tree
	 */
	void exitNodeList(BaseParser.NodeListContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#edgeList}.
	 * @param ctx the parse tree
	 */
	void enterEdgeList(BaseParser.EdgeListContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#edgeList}.
	 * @param ctx the parse tree
	 */
	void exitEdgeList(BaseParser.EdgeListContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#graphID}.
	 * @param ctx the parse tree
	 */
	void enterGraphID(BaseParser.GraphIDContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#graphID}.
	 * @param ctx the parse tree
	 */
	void exitGraphID(BaseParser.GraphIDContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#nodeID}.
	 * @param ctx the parse tree
	 */
	void enterNodeID(BaseParser.NodeIDContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#nodeID}.
	 * @param ctx the parse tree
	 */
	void exitNodeID(BaseParser.NodeIDContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#fileEdgeList}.
	 * @param ctx the parse tree
	 */
	void enterFileEdgeList(BaseParser.FileEdgeListContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#fileEdgeList}.
	 * @param ctx the parse tree
	 */
	void exitFileEdgeList(BaseParser.FileEdgeListContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#edge}.
	 * @param ctx the parse tree
	 */
	void enterEdge(BaseParser.EdgeContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#edge}.
	 * @param ctx the parse tree
	 */
	void exitEdge(BaseParser.EdgeContext ctx);
	/**
	 * Enter a parse tree produced by the {@code SimpleDeclaration}
	 * labeled alternative in {@link BaseParser#varDecl}.
	 * @param ctx the parse tree
	 */
	void enterSimpleDeclaration(BaseParser.SimpleDeclarationContext ctx);
	/**
	 * Exit a parse tree produced by the {@code SimpleDeclaration}
	 * labeled alternative in {@link BaseParser#varDecl}.
	 * @param ctx the parse tree
	 */
	void exitSimpleDeclaration(BaseParser.SimpleDeclarationContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ArrayDeclaration}
	 * labeled alternative in {@link BaseParser#varDecl}.
	 * @param ctx the parse tree
	 */
	void enterArrayDeclaration(BaseParser.ArrayDeclarationContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ArrayDeclaration}
	 * labeled alternative in {@link BaseParser#varDecl}.
	 * @param ctx the parse tree
	 */
	void exitArrayDeclaration(BaseParser.ArrayDeclarationContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#conditionalStatement}.
	 * @param ctx the parse tree
	 */
	void enterConditionalStatement(BaseParser.ConditionalStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#conditionalStatement}.
	 * @param ctx the parse tree
	 */
	void exitConditionalStatement(BaseParser.ConditionalStatementContext ctx);
	/**
	 * Enter a parse tree produced by the {@code NodeCheck}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void enterNodeCheck(BaseParser.NodeCheckContext ctx);
	/**
	 * Exit a parse tree produced by the {@code NodeCheck}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void exitNodeCheck(BaseParser.NodeCheckContext ctx);
	/**
	 * Enter a parse tree produced by the {@code LogicalAnd}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void enterLogicalAnd(BaseParser.LogicalAndContext ctx);
	/**
	 * Exit a parse tree produced by the {@code LogicalAnd}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void exitLogicalAnd(BaseParser.LogicalAndContext ctx);
	/**
	 * Enter a parse tree produced by the {@code Relational}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void enterRelational(BaseParser.RelationalContext ctx);
	/**
	 * Exit a parse tree produced by the {@code Relational}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void exitRelational(BaseParser.RelationalContext ctx);
	/**
	 * Enter a parse tree produced by the {@code EdgeCheck}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void enterEdgeCheck(BaseParser.EdgeCheckContext ctx);
	/**
	 * Exit a parse tree produced by the {@code EdgeCheck}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void exitEdgeCheck(BaseParser.EdgeCheckContext ctx);
	/**
	 * Enter a parse tree produced by the {@code LogicalOr}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void enterLogicalOr(BaseParser.LogicalOrContext ctx);
	/**
	 * Exit a parse tree produced by the {@code LogicalOr}
	 * labeled alternative in {@link BaseParser#condition}.
	 * @param ctx the parse tree
	 */
	void exitLogicalOr(BaseParser.LogicalOrContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#graphComprehension}.
	 * @param ctx the parse tree
	 */
	void enterGraphComprehension(BaseParser.GraphComprehensionContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#graphComprehension}.
	 * @param ctx the parse tree
	 */
	void exitGraphComprehension(BaseParser.GraphComprehensionContext ctx);
	/**
	 * Enter a parse tree produced by the {@code GraphLogicalAnd}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void enterGraphLogicalAnd(BaseParser.GraphLogicalAndContext ctx);
	/**
	 * Exit a parse tree produced by the {@code GraphLogicalAnd}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void exitGraphLogicalAnd(BaseParser.GraphLogicalAndContext ctx);
	/**
	 * Enter a parse tree produced by the {@code DegreeCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void enterDegreeCondition(BaseParser.DegreeConditionContext ctx);
	/**
	 * Exit a parse tree produced by the {@code DegreeCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void exitDegreeCondition(BaseParser.DegreeConditionContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ConnectedCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void enterConnectedCondition(BaseParser.ConnectedConditionContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ConnectedCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void exitConnectedCondition(BaseParser.ConnectedConditionContext ctx);
	/**
	 * Enter a parse tree produced by the {@code GraphLogicalOr}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void enterGraphLogicalOr(BaseParser.GraphLogicalOrContext ctx);
	/**
	 * Exit a parse tree produced by the {@code GraphLogicalOr}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void exitGraphLogicalOr(BaseParser.GraphLogicalOrContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ParenGraphCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void enterParenGraphCondition(BaseParser.ParenGraphConditionContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ParenGraphCondition}
	 * labeled alternative in {@link BaseParser#graphCondition}.
	 * @param ctx the parse tree
	 */
	void exitParenGraphCondition(BaseParser.ParenGraphConditionContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#loopStatement}.
	 * @param ctx the parse tree
	 */
	void enterLoopStatement(BaseParser.LoopStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#loopStatement}.
	 * @param ctx the parse tree
	 */
	void exitLoopStatement(BaseParser.LoopStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#foreachStatement}.
	 * @param ctx the parse tree
	 */
	void enterForeachStatement(BaseParser.ForeachStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#foreachStatement}.
	 * @param ctx the parse tree
	 */
	void exitForeachStatement(BaseParser.ForeachStatementContext ctx);
	/**
	 * Enter a parse tree produced by the {@code forEachVertex}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 */
	void enterForEachVertex(BaseParser.ForEachVertexContext ctx);
	/**
	 * Exit a parse tree produced by the {@code forEachVertex}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 */
	void exitForEachVertex(BaseParser.ForEachVertexContext ctx);
	/**
	 * Enter a parse tree produced by the {@code forEachEdge}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 */
	void enterForEachEdge(BaseParser.ForEachEdgeContext ctx);
	/**
	 * Exit a parse tree produced by the {@code forEachEdge}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 */
	void exitForEachEdge(BaseParser.ForEachEdgeContext ctx);
	/**
	 * Enter a parse tree produced by the {@code forEachAdj}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 */
	void enterForEachAdj(BaseParser.ForEachAdjContext ctx);
	/**
	 * Exit a parse tree produced by the {@code forEachAdj}
	 * labeled alternative in {@link BaseParser#loopTarget}.
	 * @param ctx the parse tree
	 */
	void exitForEachAdj(BaseParser.ForEachAdjContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#whileStatement}.
	 * @param ctx the parse tree
	 */
	void enterWhileStatement(BaseParser.WhileStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#whileStatement}.
	 * @param ctx the parse tree
	 */
	void exitWhileStatement(BaseParser.WhileStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#nodeEdgeOperation}.
	 * @param ctx the parse tree
	 */
	void enterNodeEdgeOperation(BaseParser.NodeEdgeOperationContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#nodeEdgeOperation}.
	 * @param ctx the parse tree
	 */
	void exitNodeEdgeOperation(BaseParser.NodeEdgeOperationContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#addOperation}.
	 * @param ctx the parse tree
	 */
	void enterAddOperation(BaseParser.AddOperationContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#addOperation}.
	 * @param ctx the parse tree
	 */
	void exitAddOperation(BaseParser.AddOperationContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#removeOperation}.
	 * @param ctx the parse tree
	 */
	void enterRemoveOperation(BaseParser.RemoveOperationContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#removeOperation}.
	 * @param ctx the parse tree
	 */
	void exitRemoveOperation(BaseParser.RemoveOperationContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#addTargets}.
	 * @param ctx the parse tree
	 */
	void enterAddTargets(BaseParser.AddTargetsContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#addTargets}.
	 * @param ctx the parse tree
	 */
	void exitAddTargets(BaseParser.AddTargetsContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#removeTargets}.
	 * @param ctx the parse tree
	 */
	void enterRemoveTargets(BaseParser.RemoveTargetsContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#removeTargets}.
	 * @param ctx the parse tree
	 */
	void exitRemoveTargets(BaseParser.RemoveTargetsContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#queryStatement}.
	 * @param ctx the parse tree
	 */
	void enterQueryStatement(BaseParser.QueryStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#queryStatement}.
	 * @param ctx the parse tree
	 */
	void exitQueryStatement(BaseParser.QueryStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#showgraph}.
	 * @param ctx the parse tree
	 */
	void enterShowgraph(BaseParser.ShowgraphContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#showgraph}.
	 * @param ctx the parse tree
	 */
	void exitShowgraph(BaseParser.ShowgraphContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#function}.
	 * @param ctx the parse tree
	 */
	void enterFunction(BaseParser.FunctionContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#function}.
	 * @param ctx the parse tree
	 */
	void exitFunction(BaseParser.FunctionContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#returnType}.
	 * @param ctx the parse tree
	 */
	void enterReturnType(BaseParser.ReturnTypeContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#returnType}.
	 * @param ctx the parse tree
	 */
	void exitReturnType(BaseParser.ReturnTypeContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#paramList}.
	 * @param ctx the parse tree
	 */
	void enterParamList(BaseParser.ParamListContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#paramList}.
	 * @param ctx the parse tree
	 */
	void exitParamList(BaseParser.ParamListContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#param}.
	 * @param ctx the parse tree
	 */
	void enterParam(BaseParser.ParamContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#param}.
	 * @param ctx the parse tree
	 */
	void exitParam(BaseParser.ParamContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#type}.
	 * @param ctx the parse tree
	 */
	void enterType(BaseParser.TypeContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#type}.
	 * @param ctx the parse tree
	 */
	void exitType(BaseParser.TypeContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#functionCall}.
	 * @param ctx the parse tree
	 */
	void enterFunctionCall(BaseParser.FunctionCallContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#functionCall}.
	 * @param ctx the parse tree
	 */
	void exitFunctionCall(BaseParser.FunctionCallContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#argumentList}.
	 * @param ctx the parse tree
	 */
	void enterArgumentList(BaseParser.ArgumentListContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#argumentList}.
	 * @param ctx the parse tree
	 */
	void exitArgumentList(BaseParser.ArgumentListContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#block}.
	 * @param ctx the parse tree
	 */
	void enterBlock(BaseParser.BlockContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#block}.
	 * @param ctx the parse tree
	 */
	void exitBlock(BaseParser.BlockContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#returnStatement}.
	 * @param ctx the parse tree
	 */
	void enterReturnStatement(BaseParser.ReturnStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#returnStatement}.
	 * @param ctx the parse tree
	 */
	void exitReturnStatement(BaseParser.ReturnStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#printStatement}.
	 * @param ctx the parse tree
	 */
	void enterPrintStatement(BaseParser.PrintStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#printStatement}.
	 * @param ctx the parse tree
	 */
	void exitPrintStatement(BaseParser.PrintStatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#printExpr}.
	 * @param ctx the parse tree
	 */
	void enterPrintExpr(BaseParser.PrintExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#printExpr}.
	 * @param ctx the parse tree
	 */
	void exitPrintExpr(BaseParser.PrintExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code edgePrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 */
	void enterEdgePrint(BaseParser.EdgePrintContext ctx);
	/**
	 * Exit a parse tree produced by the {@code edgePrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 */
	void exitEdgePrint(BaseParser.EdgePrintContext ctx);
	/**
	 * Enter a parse tree produced by the {@code nodePrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 */
	void enterNodePrint(BaseParser.NodePrintContext ctx);
	/**
	 * Exit a parse tree produced by the {@code nodePrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 */
	void exitNodePrint(BaseParser.NodePrintContext ctx);
	/**
	 * Enter a parse tree produced by the {@code graphPrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 */
	void enterGraphPrint(BaseParser.GraphPrintContext ctx);
	/**
	 * Exit a parse tree produced by the {@code graphPrint}
	 * labeled alternative in {@link BaseParser#printgraph}.
	 * @param ctx the parse tree
	 */
	void exitGraphPrint(BaseParser.GraphPrintContext ctx);
	/**
	 * Enter a parse tree produced by the {@code BoolTrueExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterBoolTrueExpr(BaseParser.BoolTrueExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code BoolTrueExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitBoolTrueExpr(BaseParser.BoolTrueExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code MulDivExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterMulDivExpr(BaseParser.MulDivExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code MulDivExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitMulDivExpr(BaseParser.MulDivExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code IdExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterIdExpr(BaseParser.IdExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code IdExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitIdExpr(BaseParser.IdExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code BoolFalseExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterBoolFalseExpr(BaseParser.BoolFalseExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code BoolFalseExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitBoolFalseExpr(BaseParser.BoolFalseExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ArrayAccessExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterArrayAccessExpr(BaseParser.ArrayAccessExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ArrayAccessExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitArrayAccessExpr(BaseParser.ArrayAccessExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code Array2DAccessExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterArray2DAccessExpr(BaseParser.Array2DAccessExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code Array2DAccessExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitArray2DAccessExpr(BaseParser.Array2DAccessExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code IntExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterIntExpr(BaseParser.IntExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code IntExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitIntExpr(BaseParser.IntExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ParenExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterParenExpr(BaseParser.ParenExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ParenExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitParenExpr(BaseParser.ParenExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code FuncExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterFuncExpr(BaseParser.FuncExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code FuncExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitFuncExpr(BaseParser.FuncExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code RealExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterRealExpr(BaseParser.RealExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code RealExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitRealExpr(BaseParser.RealExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code AddSubExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterAddSubExpr(BaseParser.AddSubExprContext ctx);
	/**
	 * Exit a parse tree produced by the {@code AddSubExpr}
	 * labeled alternative in {@link BaseParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitAddSubExpr(BaseParser.AddSubExprContext ctx);
	/**
	 * Enter a parse tree produced by the {@code Sized2DArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void enterSized2DArray(BaseParser.Sized2DArrayContext ctx);
	/**
	 * Exit a parse tree produced by the {@code Sized2DArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void exitSized2DArray(BaseParser.Sized2DArrayContext ctx);
	/**
	 * Enter a parse tree produced by the {@code Unsized2DArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void enterUnsized2DArray(BaseParser.Unsized2DArrayContext ctx);
	/**
	 * Exit a parse tree produced by the {@code Unsized2DArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void exitUnsized2DArray(BaseParser.Unsized2DArrayContext ctx);
	/**
	 * Enter a parse tree produced by the {@code SizedArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void enterSizedArray(BaseParser.SizedArrayContext ctx);
	/**
	 * Exit a parse tree produced by the {@code SizedArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void exitSizedArray(BaseParser.SizedArrayContext ctx);
	/**
	 * Enter a parse tree produced by the {@code UnsizedArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void enterUnsizedArray(BaseParser.UnsizedArrayContext ctx);
	/**
	 * Exit a parse tree produced by the {@code UnsizedArray}
	 * labeled alternative in {@link BaseParser#arrayDeclarator}.
	 * @param ctx the parse tree
	 */
	void exitUnsizedArray(BaseParser.UnsizedArrayContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#declaration}.
	 * @param ctx the parse tree
	 */
	void enterDeclaration(BaseParser.DeclarationContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#declaration}.
	 * @param ctx the parse tree
	 */
	void exitDeclaration(BaseParser.DeclarationContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ArrayInit1D}
	 * labeled alternative in {@link BaseParser#arrayInitializer}.
	 * @param ctx the parse tree
	 */
	void enterArrayInit1D(BaseParser.ArrayInit1DContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ArrayInit1D}
	 * labeled alternative in {@link BaseParser#arrayInitializer}.
	 * @param ctx the parse tree
	 */
	void exitArrayInit1D(BaseParser.ArrayInit1DContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ArrayInit2D}
	 * labeled alternative in {@link BaseParser#arrayInitializer}.
	 * @param ctx the parse tree
	 */
	void enterArrayInit2D(BaseParser.ArrayInit2DContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ArrayInit2D}
	 * labeled alternative in {@link BaseParser#arrayInitializer}.
	 * @param ctx the parse tree
	 */
	void exitArrayInit2D(BaseParser.ArrayInit2DContext ctx);
	/**
	 * Enter a parse tree produced by {@link BaseParser#assignmentStatement}.
	 * @param ctx the parse tree
	 */
	void enterAssignmentStatement(BaseParser.AssignmentStatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link BaseParser#assignmentStatement}.
	 * @param ctx the parse tree
	 */
	void exitAssignmentStatement(BaseParser.AssignmentStatementContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ArrayAssignStmt}
	 * labeled alternative in {@link BaseParser#arrayAssignStatement}.
	 * @param ctx the parse tree
	 */
	void enterArrayAssignStmt(BaseParser.ArrayAssignStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ArrayAssignStmt}
	 * labeled alternative in {@link BaseParser#arrayAssignStatement}.
	 * @param ctx the parse tree
	 */
	void exitArrayAssignStmt(BaseParser.ArrayAssignStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code Array2DAssignStmt}
	 * labeled alternative in {@link BaseParser#arrayAssignStatement}.
	 * @param ctx the parse tree
	 */
	void enterArray2DAssignStmt(BaseParser.Array2DAssignStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code Array2DAssignStmt}
	 * labeled alternative in {@link BaseParser#arrayAssignStatement}.
	 * @param ctx the parse tree
	 */
	void exitArray2DAssignStmt(BaseParser.Array2DAssignStmtContext ctx);
}