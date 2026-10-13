#include "ASTBuilder.h"
#include <iostream>
#include "ASTNode.h"
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <cstring> // for std::memcpy
#include "llvm/Support/Allocator.h"


namespace
{
// `a -> b;` is a required POSITIVE edge, `a -| b;` a required NEGATIVE one.
// Every pair with no motif edge is required to be ABSENT, which is what makes
// matching induced rather than merely monomorphic.
MotifEdgeSpec parseMotifEdge(BaseParser::MotifEdgeContext *edge)
{
    MotifEdgeSpec result;
    result.source = edge->ID(0)->getText();
    result.target = edge->ID(1)->getText();
    result.sign = edge->children[1]->getText() == "-|"
                      ? MotifEdgeSign::Negative
                      : MotifEdgeSign::Positive;
    return result;
}
} // namespace

antlrcpp::Any ASTBuilder::visitProgram(BaseParser::ProgramContext *ctx)
{
    std::vector<ASTNodePtr> items;

    // Preserve top-level source order from: (statement | function)* EOF
    for (auto *child : ctx->children)
    {
        if (auto *funcCtx = dynamic_cast<BaseParser::FunctionContext *>(child))
        {
            auto fnAny = visitFunction(funcCtx);
            items.push_back(safe_any_cast<ASTNodePtr>(fnAny, "visitProgram"));
            continue;
        }
        if (auto *stmtCtx = dynamic_cast<BaseParser::StatementContext *>(child))
        {
            auto stAny = visitStatement(stmtCtx);
            if (stAny.has_value() && stAny.type() == typeid(ASTNodePtr))
            {
                items.push_back(safe_any_cast<ASTNodePtr>(stAny, "visitProgram"));
            }
        }
    }

    // Package top-level nodes into program root.
    auto prog = std::make_shared<ProgramNode>(std::move(items));
    return prog;
}

antlrcpp::Any ASTBuilder::visitStatement(BaseParser::StatementContext *ctx)
{
    // std::cerr << " visiting statement " << "\n";
    if (ctx->printStatement())
    {
        // std::cout<<"before entering printxpr\n";
        return visitPrintStatement(ctx->printStatement());
    }
    else if (ctx->varDecl())
    {
        // std::cerr << "before visting varDeclr " << "\n";
        return visitVarDecl(ctx->varDecl());
    }
    else if (ctx->assignmentStatement())
    {
        // std::cerr << " befoe entering assignment " << "\n";
        return visitAssignmentStatement(ctx->assignmentStatement());
    }
    else if (ctx->conditionalStatement())
    {
        return visitConditionalStatement(ctx->conditionalStatement());
    }
    // else if(ctx->loopStatement()){
    //     return visitLoopStatement(ctx->loopStatement());
    // }
    else if (ctx->whileStatement())
    {
        return visitWhileStatement(ctx->whileStatement());
    }
    else if (ctx->foreachStatement())
    {
        return visitForeachStatement(ctx->foreachStatement());
    }
    else if (ctx->arrayAssignStatement())
    {
        // Check if it's a 2D array assignment first
        if (auto *arr2DAlt = dynamic_cast<BaseParser::Array2DAssignStmtContext *>(ctx->arrayAssignStatement()))
        {
            return visitArray2DAssignStmt(arr2DAlt);
        }
        auto *arrAlt = dynamic_cast<BaseParser::ArrayAssignStmtContext *>(ctx->arrayAssignStatement());
        if (!arrAlt)
        {
            throw std::runtime_error("internal: expected ArrayAssignStmtContext (grammar alternative mismatch)");
        }
        return visitArrayAssignStmt(arrAlt);
    }

    else if (ctx->functionCall())
    {
        return visitFunctionCall(ctx->functionCall());
    }
    else if (ctx->graphDef())
    {
        if (auto wg = dynamic_cast<BaseParser::WeightedGraphDefContext *>(ctx->graphDef()))
            return visitWeightedGraphDef(wg);
        else if (auto ug = dynamic_cast<BaseParser::UnweightedGraphDefContext *>(ctx->graphDef()))
            return visitUnweightedGraphDef(ug);
        else if (auto pg = dynamic_cast<BaseParser::PlainGraphDefContext *>(ctx->graphDef()))
            return visitPlainGraphDef(pg);
        else if (auto lg = dynamic_cast<BaseParser::LoadGraphDefContext *>(ctx->graphDef()))
            return visitLoadGraphDef(lg);
        else
            throw std::runtime_error("internal: unknown graphDef alternative");
        // produce a GraphDeclNode
        // return visitGraphDef(ctx->graphDef());
    }
    else if (ctx->queryStatement())
    {
        return visitQueryStatement(ctx->queryStatement());
    }
    else if (ctx->graphUpdateCall())
    {
        if (auto *an = dynamic_cast<BaseParser::GraphAddNodesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphAddNodesCall(an);
        if (auto *ae = dynamic_cast<BaseParser::GraphAddEdgesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphAddEdgesCall(ae);
        if (auto *rn = dynamic_cast<BaseParser::GraphRemoveNodesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphRemoveNodesCall(rn);
        if (auto *re = dynamic_cast<BaseParser::GraphRemoveEdgesCallContext *>(ctx->graphUpdateCall()))
            return visitGraphRemoveEdgesCall(re);
        throw std::runtime_error("internal: unknown graphUpdateCall alternative");
    }
    else if (ctx->graphComprehension())
    {
        auto *gc = ctx->graphComprehension();
        std::string target = gc->ID()->getText();

        auto *gexpr = gc->graphExpr();
        std::string gname = gexpr->graphID(0)->getText();
        std::vector<GraphExprOp> ops;
        std::vector<std::string> operands;

        // Parse graphExpr children: graphID (AND/OR graphID)*
        if (gexpr->children.size() > 1)
        {
            for (size_t i = 1; i + 1 < gexpr->children.size(); i += 2)
            {
                std::string opText = gexpr->children[i]->getText();
                GraphExprOp op = (opText == "&&") ? GraphExprOp::And : GraphExprOp::Or;
                ops.push_back(op);
                operands.push_back(gexpr->children[i + 1]->getText());
            }
        }

        std::shared_ptr<GraphConditionNode> cond = nullptr;
        if (gc->graphCondition())
            cond = buildGraphCondition(gc->graphCondition());

    // Unified comprehension: a `motif` condition yields the bindings
    // (MotifMatchesDeclNode), exactly like the explicit `motifs h = [...]` form.
    if (cond && !cond->motifEdges.empty())
    {
        std::vector<std::string> variables;
        std::unordered_set<std::string> seen;
        for (const MotifEdgeSpec &e : cond->motifEdges)
        {
            if (seen.insert(e.source).second) variables.push_back(e.source);
            if (seen.insert(e.target).second) variables.push_back(e.target);
        }
        auto mnode = std::make_shared<MotifMatchesDeclNode>(
            target, gname, cond->motifEdges, std::move(variables));
        return std::static_pointer_cast<ASTNode>(mnode);
    }
        auto node = std::make_shared<GraphComprehensionNode>(
            target, gname, std::move(ops), std::move(operands), cond);
        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->motifMatchesDecl())
    {
        auto *decl = ctx->motifMatchesDecl();
        std::vector<MotifEdgeSpec> edges;
        std::vector<std::string> variables;
        std::unordered_set<std::string> seen;
        for (auto *edgeCtx : decl->motifEdge())
        {
            MotifEdgeSpec edge = parseMotifEdge(edgeCtx);
            if (seen.insert(edge.source).second)
                variables.push_back(edge.source);
            if (seen.insert(edge.target).second)
                variables.push_back(edge.target);
            edges.push_back(std::move(edge));
        }
        auto node = std::make_shared<MotifMatchesDeclNode>(
            decl->ID()->getText(), decl->graphID()->getText(),
            std::move(edges), std::move(variables));
        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->motifSearchDecl())
    {
        return visitMotifSearchDecl(ctx->motifSearchDecl());
    }
    else if (ctx->graphListDecl())
    {
        auto *decl = ctx->graphListDecl();
        std::vector<MotifEdgeSpec> edges;
        std::vector<std::string> variables;
        std::unordered_set<std::string> seen;
        for (auto *edgeCtx : decl->motifEdge())
        {
            MotifEdgeSpec edge = parseMotifEdge(edgeCtx);
            if (seen.insert(edge.source).second)
                variables.push_back(edge.source);
            if (seen.insert(edge.target).second)
                variables.push_back(edge.target);
            edges.push_back(std::move(edge));
        }
        auto node = std::make_shared<GraphListDeclNode>(
            decl->ID()->getText(), decl->graphID()->getText(),
            std::move(edges), std::move(variables));
        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->showgraph())
    {
        std::string gname = ctx->showgraph()->graphID()->getText();
        auto node = std::make_shared<ShowGraphNode>(gname);
        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->drawgraph())
    {
        auto *draw = ctx->drawgraph();
        std::string output = draw->STRING()->getText();
        output = output.substr(1, output.size() - 2);

        auto node = std::make_shared<DrawGraphNode>(
            draw->graphID()->getText(), output);

        bool seenLayout = false;
        bool seenVertexLabel = false;
        bool seenVertexColor = false;
        bool seenVertexSize = false;
        bool seenEdgeLabel = false;

        for (auto *option : draw->drawOption())
        {
            if (option->STRING())
            {
                if (seenLayout)
                    throw std::runtime_error("draw: duplicate layout option");
                seenLayout = true;
                std::string value = option->STRING()->getText();
                value = value.substr(1, value.size() - 2);
                if (value == "hierarchical") node->layout = DrawLayout::Hierarchical;
                else if (value == "force") node->layout = DrawLayout::Force;
                else if (value == "radial") node->layout = DrawLayout::Radial;
                else if (value == "circular") node->layout = DrawLayout::Circular;
                else if (value == "clustered") node->layout = DrawLayout::Clustered;
                else if (value == "auto") node->layout = DrawLayout::Auto;
                else throw std::runtime_error("draw: unsupported layout: " + value);
                continue;
            }

            for (auto *vertexOption : option->vertexDrawOption())
            {
                if (auto *mapping = vertexOption->colorMapping())
                {
                    if (seenVertexColor)
                        throw std::runtime_error("draw: duplicate vertex color option");
                    seenVertexColor = true;
                    if (mapping->continuousMapping())
                    {
                        node->colorMode = DrawColorMode::Continuous;
                        node->colorArray = mapping->continuousMapping()->ID()->getText();
                    }
                    else
                    {
                        node->colorMode = DrawColorMode::Categorical;
                        node->colorArray = mapping->ID()->getText();
                    }
                }
                else if (auto *mapping = vertexOption->continuousMapping())
                {
                    if (seenVertexSize)
                        throw std::runtime_error("draw: duplicate vertex size option");
                    seenVertexSize = true;
                    node->sizeArray = mapping->ID()->getText();
                }
                else if (vertexOption->getStart()->getText() == "size" &&
                         vertexOption->ID())
                {
                    if (seenVertexSize)
                        throw std::runtime_error("draw: duplicate vertex size option");
                    seenVertexSize = true;
                    node->sizeArray = vertexOption->ID()->getText();
                }
                else
                {
                    if (seenVertexLabel)
                        throw std::runtime_error("draw: duplicate vertex label option");
                    seenVertexLabel = true;
                    const std::string value = vertexOption->drawBoolLiteral()->getText();
                    node->vertexLabels = value == "true" || value == "TRUE";
                }
            }

            for (auto *edgeOption : option->edgeDrawOption())
            {
                if (seenEdgeLabel)
                    throw std::runtime_error("draw: duplicate edge label option");
                seenEdgeLabel = true;
                const std::string value = edgeOption->drawBoolLiteral()->getText();
                node->edgeWeightLabels = value == "true" || value == "TRUE";
            }
        }

        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->drawmotifs())
    {
        auto *draw = ctx->drawmotifs();
        std::string output = draw->STRING()->getText();
        output = output.substr(1, output.size() - 2);

        auto node = std::make_shared<DrawMotifsNode>(
            draw->graphID()->getText(), output);
        node->combinedImage = draw->getText().rfind("drawmotifsof", 0) == 0;

        bool seenLayout = false;
        bool seenVertexLabel = false;
        bool seenEdgeLabel = false;

        for (auto *option : draw->drawMotifOption())
        {
            if (auto *edge = option->motifEdge())
            {
                node->motifEdges.push_back(parseMotifEdge(edge));
                continue;
            }

            if (option->STRING())
            {
                if (seenLayout)
                    throw std::runtime_error("draw motif: duplicate layout option");
                seenLayout = true;
                std::string value = option->STRING()->getText();
                value = value.substr(1, value.size() - 2);
                if (value == "hierarchical") node->layout = DrawLayout::Hierarchical;
                else if (value == "force") node->layout = DrawLayout::Force;
                else if (value == "radial") node->layout = DrawLayout::Radial;
                else if (value == "circular") node->layout = DrawLayout::Circular;
                else if (value == "clustered") node->layout = DrawLayout::Clustered;
                else if (value == "auto") node->layout = DrawLayout::Auto;
                else throw std::runtime_error("draw motif: unsupported layout: " + value);
                continue;
            }

            for (auto *vertexOption : option->vertexDrawOption())
            {
                if (!vertexOption->drawBoolLiteral())
                    throw std::runtime_error("draw motif: vertex color/size is not supported for per-match drawings yet");
                if (seenVertexLabel)
                    throw std::runtime_error("draw motif: duplicate vertex label option");
                seenVertexLabel = true;
                const std::string value = vertexOption->drawBoolLiteral()->getText();
                node->vertexLabels = value == "true" || value == "TRUE";
            }

            for (auto *edgeOption : option->edgeDrawOption())
            {
                if (seenEdgeLabel)
                    throw std::runtime_error("draw motif: duplicate edge label option");
                seenEdgeLabel = true;
                const std::string value = edgeOption->drawBoolLiteral()->getText();
                node->edgeLabels = value == "true" || value == "TRUE";
            }
        }

        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->sleepStatement())
    {
        return visitSleepStatement(ctx->sleepStatement());
    }

    else if (ctx->setDecl())
        {
            return visitSetDecl(ctx->setDecl());
        }
        else if (ctx->setOperation())
        {
            return visitSetOperation(ctx->setOperation());
        }
        else if (ctx->setMethodCall())
        {
            return visitSetMethodCall(ctx->setMethodCall());
        }
        else if (ctx->swapStatement())
        {
            return visitSwapStatement(ctx->swapStatement());
        }
    // std::cerr << " ending statement " << "\n";
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitBlock(BaseParser::BlockContext *ctx)
{
    std::vector<ASTNodePtr> stmts;
    for (size_t i = 0; i < ctx->children.size(); ++i)
    {
        if (auto stmtCtx = dynamic_cast<BaseParser::StatementContext *>(ctx->children[i]))
        {
            auto anyStmt = visitStatement(stmtCtx);
            if (anyStmt.has_value() && anyStmt.type() == typeid(ASTNodePtr))
            {
                stmts.push_back(safe_any_cast<ASTNodePtr>(anyStmt));
            }
        }
        else if (auto retStmtCtx = dynamic_cast<BaseParser::ReturnStatementContext *>(ctx->children[i]))
        {
            ASTNodePtr exprNode = safe_any_cast<ASTNodePtr>(visitExpr(retStmtCtx->expr()));
            auto retStmtNode = std::make_shared<ReturnStmtNode>(exprNode);
            stmts.push_back(retStmtNode);
        }
        else if (dynamic_cast<BaseParser::BreakStatementContext *>(ctx->children[i]))
        {
            stmts.push_back(std::make_shared<BreakStmtNode>());
        }
        else if (dynamic_cast<BaseParser::ContinueStatementContext *>(ctx->children[i]))
        {
            stmts.push_back(std::make_shared<ContinueStmtNode>());
        }
        // Ignore tokens like '{' and '}'
    }
    auto blockNode = std::make_shared<BlockStmtNode>(std::move(stmts));
    return std::static_pointer_cast<ASTNode>(blockNode);
}

antlrcpp::Any ASTBuilder::visitConditionalStatement(BaseParser::ConditionalStatementContext *ctx)
{
    ASTNodePtr condition = safe_any_cast<ASTNodePtr>(visitCondition(ctx->condition()));

    ASTNodePtr thenBlock = safe_any_cast<ASTNodePtr>(visitBlock(ctx->block(0)));
    ASTNodePtr elseBlock = nullptr;
    if (ctx->block().size() > 1)
    {
        elseBlock = safe_any_cast<ASTNodePtr>(visitBlock(ctx->block(1)));
    }

    auto condNode = std::make_shared<ConditionalNode>(condition, thenBlock, elseBlock);
    return std::static_pointer_cast<ASTNode>(condNode);
}

// Recursive eval of boolean conditions
antlrcpp::Any ASTBuilder::visitCondition(BaseParser::ConditionContext *ctx)
{
    using Ctx = BaseParser;
    if (auto a = dynamic_cast<Ctx::LogicalAndContext *>(ctx))
    {
        ASTNodePtr left = safe_any_cast<ASTNodePtr>(visitCondition(a->condition(0)));
        ASTNodePtr right = safe_any_cast<ASTNodePtr>(visitCondition(a->condition(1)));
        return ASTNodePtr(std::make_shared<BinaryExprNode>("&&", left, right));
    }
    if (auto o = dynamic_cast<Ctx::LogicalOrContext *>(ctx))
    {
        ASTNodePtr left = safe_any_cast<ASTNodePtr>(visitCondition(o->condition(0)));
        ASTNodePtr right = safe_any_cast<ASTNodePtr>(visitCondition(o->condition(1)));
        return ASTNodePtr(std::make_shared<BinaryExprNode>("||", left, right));
    }
    if (auto r = dynamic_cast<Ctx::RelationalContext *>(ctx))
    {
        ASTNodePtr left = safe_any_cast<ASTNodePtr>(visitExpr(r->expr(0)));
        ASTNodePtr right = safe_any_cast<ASTNodePtr>(visitExpr(r->expr(1)));
        std::string op = r->EQUAL() ? "==" : r->NOTEQUAL()   ? "!="
                                         : r->LESSEQUAL()    ? "<="
                                         : r->GREATEREQUAL() ? ">="
                                         : r->LESSTHAN()     ? "<"
                                         : r->GREATERTHAN()  ? ">"
                                                             : "";
        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, left, right));
    }
    if (auto hn = dynamic_cast<Ctx::HasNodeCheckContext *>(ctx))
    {
        // G.hasNode(x)  ==  x in G.nodes  (the backend already lowers set membership)
        ASTNodePtr val = safe_any_cast<ASTNodePtr>(visitExpr(hn->expr()));
        return ASTNodePtr(std::make_shared<SetContainsExprNode>(
            SetTargetKind::GraphNodes, hn->graphID()->getText(), val));
    }
    if (auto hnc = dynamic_cast<Ctx::HasNodeCallContext *>(ctx))
    {
        // G.hasNode(x)  ==  x in G.nodes  (the backend already lowers set membership)
        ASTNodePtr val = safe_any_cast<ASTNodePtr>(visitExpr(hnc->expr()));
        return ASTNodePtr(std::make_shared<SetContainsExprNode>(
            SetTargetKind::GraphNodes, hnc->graphID()->getText(), val));
    }
    if (auto he = dynamic_cast<Ctx::HasEdgeCheckContext *>(ctx))
    {
        // G.hasEdge(u, v): lowered by IRGen as the "hasEdge" builtin (runtime pair lookup)
        std::vector<ASTNodePtr> args;
        args.push_back(std::make_shared<VariableNode>(he->graphID()->getText()));
        args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(he->expr(0))));
        args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(he->expr(1))));
        return ASTNodePtr(std::make_shared<FunctionCallNode>("hasEdge", args));
    }
    if (auto hec = dynamic_cast<Ctx::HasEdgeCallContext *>(ctx))
    {
        // G.hasEdge(u, v): lowered by IRGen as the "hasEdge" builtin (runtime pair lookup)
        std::vector<ASTNodePtr> args;
        args.push_back(std::make_shared<VariableNode>(hec->graphID()->getText()));
        args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(hec->expr(0))));
        args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(hec->expr(1))));
        return ASTNodePtr(std::make_shared<FunctionCallNode>("hasEdge", args));
    }
    if (auto e = dynamic_cast<Ctx::ExprConditionContext *>(ctx))
    {
        return safe_any_cast<ASTNodePtr>(visitExpr(e->expr()));
    }
    throw std::runtime_error("Unsupported condition: " + ctx->getText());
}

// antlrcpp::Any ASTBuilder::visitLoopStatement(BaseParser::LoopStatementContext* ctx) {
//     if (ctx->whileStatement()) {
//         return visitWhileStatement(ctx->whileStatement());
//     }

//     return nullptr;
// }

antlrcpp::Any ASTBuilder::visitWhileStatement(BaseParser::WhileStatementContext *ctx)
{
    // build the sub‐trees
    ASTNodePtr cond = safe_any_cast<ASTNodePtr>(visitCondition(ctx->condition()));
    ASTNodePtr blk = safe_any_cast<ASTNodePtr>(visitBlock(ctx->block()));
    // return a WhileStmtNode
    auto whileNode = std::make_shared<WhileStmtNode>(cond, blk);
    return std::static_pointer_cast<ASTNode>(whileNode);
}

antlrcpp::Any ASTBuilder::visitExpr(BaseParser::ExprContext *ctx)
{
    // std::cout << " expr text: " << ctx->getText() << "\n";
    // std::cout << " Runtime type: " << typeid(*ctx).name() << "\n";

    if (auto logCtx = dynamic_cast<BaseParser::LogicalExprContext *>(ctx))
    {
        auto lhs = safe_any_cast<ASTNodePtr>(visitExpr(logCtx->expr(0)));
        auto rhs = safe_any_cast<ASTNodePtr>(visitExpr(logCtx->expr(1)));
        std::string op = logCtx->AND() ? "&&" : "||";
        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, lhs, rhs));
    }
    else if (auto mulDivContext = dynamic_cast<BaseParser::MulDivExprContext *>(ctx))
    {
        auto lhs = safe_any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(0)));
        auto rhs = safe_any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(1)));
        std::string op = mulDivContext->TIMES() ? "*" : (mulDivContext->DIVIDE() ? "/" : "%");
        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, lhs, rhs));
    }
    else if (auto fctx = dynamic_cast<BaseParser::FuncExprContext *>(ctx))
    {
        return visitFunctionCall(fctx->functionCall());
    }
    else if (auto addSubContext = dynamic_cast<BaseParser::AddSubExprContext *>(ctx))
    {
        // std::cout<<"we have tone till here\n";

        auto lhs = safe_any_cast<ASTNodePtr>(visitExpr(addSubContext->expr(0)));
        // std::cout<<"we have tone till here\n";
        auto rhs = safe_any_cast<ASTNodePtr>(visitExpr(addSubContext->expr(1)));
        std::string op = addSubContext->PLUS() ? "+" : "-";
        // std::cout << "ASTBuilder AddSubExpr: " <<lhs<< op <<rhs << "\n";
        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, lhs, rhs));
    }

    else if (auto intExprContext = dynamic_cast<BaseParser::IntExprContext *>(ctx))
    {
        int val = std::stoi(intExprContext->getText());
        // std::cout << "astbuilder intExpr : " << val << "\n";
        return ASTNodePtr(std::make_shared<IntLiteralNode>(val));
    }
    else if (auto realExprContext = dynamic_cast<BaseParser::RealExprContext *>(ctx))
    {
        double val = std::stod(realExprContext->getText());
        return ASTNodePtr(std::make_shared<RealLiteralNode>(val));
    }
    else if (dynamic_cast<BaseParser::BoolTrueExprContext *>(ctx))
    {
        return ASTNodePtr(std::make_shared<BoolLiteralNode>(true));
    }
    else if (dynamic_cast<BaseParser::BoolFalseExprContext *>(ctx))
    {
        return ASTNodePtr(std::make_shared<BoolLiteralNode>(false));
    }
    else if (auto idExprContext = dynamic_cast<BaseParser::IdExprContext *>(ctx))
    {
        std::string name = idExprContext->getText();

        // std::cout <<"astbuilder idExpr : " <<name<<"\n";
        return ASTNodePtr(std::make_shared<VariableNode>(name));
    }
    else if (auto parenContext = dynamic_cast<BaseParser::ParenExprContext *>(ctx))
    {
        // std::cout <<"astbuilder parenExpr : "<<"\n";

        return visitExpr(parenContext->expr());
    }
    else if (auto arrCtx = dynamic_cast<BaseParser::ArrayAccessExprContext *>(ctx))
    {
        std::string name = arrCtx->ID()->getText();
        ASTNodePtr indexNode = safe_any_cast<ASTNodePtr>(visitExpr(arrCtx->expr()));

        // Produce a runtime ArrayAccessNode (array variable + index expression)
        ASTNodePtr base = std::make_shared<VariableNode>(name);
        return ASTNodePtr(std::make_shared<ArrayAccessNode>(base, indexNode));
    }
    else if (dynamic_cast<BaseParser::TimerExprContext *>(ctx))
    {
        // timer() returns current time as real - treat as function call
        std::vector<ASTNodePtr> emptyArgs;
        return ASTNodePtr(std::make_shared<FunctionCallNode>("timer", emptyArgs));
    }
    else if (auto containsCtx = dynamic_cast<BaseParser::SetContainsExprContext *>(ctx))
    {
        // Delegate to the dedicated visitor which handles setTarget properly
        return visitSetContainsExpr(containsCtx);
    }
    else if (auto setLitCtx = dynamic_cast<BaseParser::SetLitExprContext *>(ctx))
    {
        // Set literal used inline in expression, e.g. {1,2,3}
        return visitSetInitializer(setLitCtx->setInitializer());
    }
    else if (auto sizeCtx = dynamic_cast<BaseParser::SetSizeExprContext *>(ctx))
    {
        // ID.size() → treat as setSize(ID) built-in function call
        std::string setName = sizeCtx->ID()->getText();
        std::vector<ASTNodePtr> args;
        args.push_back(std::make_shared<VariableNode>(setName));
        return ASTNodePtr(std::make_shared<FunctionCallNode>("setSize", args));
    }
    else if (auto popCtx = dynamic_cast<BaseParser::SetPopExprContext *>(ctx))
    {
        std::string setName = popCtx->ID()->getText();
        return ASTNodePtr(std::make_shared<SetPopExprNode>(setName));
    }
    else if (auto notCtx = dynamic_cast<BaseParser::NotExprContext *>(ctx))
    {
        ASTNodePtr operand = safe_any_cast<ASTNodePtr>(visitExpr(notCtx->expr()));
        return ASTNodePtr(std::make_shared<NotExprNode>(operand));
    }
    else if (auto unaryMinusCtx = dynamic_cast<BaseParser::UnaryMinusExprContext *>(ctx))
    {
        ASTNodePtr operand = safe_any_cast<ASTNodePtr>(visitExpr(unaryMinusCtx->expr()));
        return ASTNodePtr(std::make_shared<UnaryMinusExprNode>(operand));
    }
    else if (dynamic_cast<BaseParser::InfExprContext *>(ctx))
    {
        // INF → max int value (2147483647)
        return ASTNodePtr(std::make_shared<IntLiteralNode>(2147483647));
    }
    else if (auto arr2DCtx = dynamic_cast<BaseParser::Array2DAccessExprContext *>(ctx))
    {
        std::string name = arr2DCtx->ID()->getText();
        ASTNodePtr rowIdx = safe_any_cast<ASTNodePtr>(visitExpr(arr2DCtx->expr(0)));
        ASTNodePtr colIdx = safe_any_cast<ASTNodePtr>(visitExpr(arr2DCtx->expr(1)));
        ASTNodePtr base = std::make_shared<VariableNode>(name);
        return ASTNodePtr(std::make_shared<Array2DAccessNode>(base, rowIdx, colIdx));
    }

    throw std::runtime_error("ASTBuilder Unsupported expr: " + ctx->getText());

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitVarDecl(BaseParser::VarDeclContext *ctx)
{
    // only SimpleDeclaration (#SimpleDeclaration)
    if (auto sd = dynamic_cast<BaseParser::SimpleDeclarationContext *>(ctx))
    {
        std::string typeName = sd->type()->getText();
        std::string name = sd->ID()->getText();
        ASTNodePtr init = nullptr;
        if (sd->expr())
        {
            init = safe_any_cast<ASTNodePtr>(visitExpr(sd->expr()));
        }
        if (init && init->type == ASTNodeType::ArrayLiteral)
        {
            // initializer is an array literal -> infer size from initializer
            auto *arrLit = static_cast<ArrayLiteralNode *>(init.get());
            return std::static_pointer_cast<ASTNode>(
                std::make_shared<VarDeclNode>(typeName, name, init, /*isArr=*/true, arrLit->elements.size()));
        }
        else
        {
            return std::static_pointer_cast<ASTNode>(
                std::make_shared<VarDeclNode>(typeName, name, init, /*isArr=*/false, 0));
        }
    }
    else if (auto arrayDecl = dynamic_cast<BaseParser::ArrayDeclarationContext *>(ctx))
    {
        std::string typeName = arrayDecl->type()->getText();
        // Extract the name and (static) size
        auto declarator = arrayDecl->arrayDeclarator();
        if (!declarator)
            throw std::runtime_error("Missing arrayDeclarator in array declaration");

        std::string name;
        int size = 0;
        ASTNodePtr sizeExpr = nullptr;
        if (auto sized2D = dynamic_cast<BaseParser::Sized2DArrayContext *>(declarator))
        {
            // 2D array: int arr[rows][cols];
            name = sized2D->ID()->getText();
            ASTNodePtr rowNode = safe_any_cast<ASTNodePtr>(visitExpr(sized2D->expr(0)));
            ASTNodePtr colNode = safe_any_cast<ASTNodePtr>(visitExpr(sized2D->expr(1)));

            auto node = std::make_shared<VarDeclNode>(typeName, name, nullptr, true, 0, nullptr);
            node->isArray2D = true;

            if (auto *rowLit = dynamic_cast<IntLiteralNode *>(rowNode.get()))
                node->array2DRows = rowLit->value;
            else
                node->array2DRowsExpr = rowNode;

            if (auto *colLit = dynamic_cast<IntLiteralNode *>(colNode.get()))
                node->array2DCols = colLit->value;
            else
                node->array2DColsExpr = colNode;

            return std::static_pointer_cast<ASTNode>(node);
        }
        else if (auto sized = dynamic_cast<BaseParser::SizedArrayContext *>(declarator))
        {
            name = sized->ID()->getText();
            ASTNodePtr sizeNode = safe_any_cast<ASTNodePtr>(visitExpr(sized->expr()));
            if (auto *intLit = dynamic_cast<IntLiteralNode *>(sizeNode.get()))
                size = intLit->value;
            else
                sizeExpr = sizeNode;
        }
        else if (auto unsized = dynamic_cast<BaseParser::UnsizedArrayContext *>(declarator))
        {
            name = unsized->ID()->getText();
        }
        else
        {
            throw std::runtime_error("Unknown arrayDeclarator type");
        }

        // Build the vector of initializer AST nodes
        std::vector<ASTNodePtr> elems;
        if (auto initCtx = arrayDecl->arrayInitializer())
        {
            for (auto *eCtx : initCtx->expr())
            {
                elems.push_back(safe_any_cast<ASTNodePtr>(visitExpr(eCtx)));
            }
        }

        // If no explicit size was given and no dynamic expr, infer from initializer
        if (size == 0 && !sizeExpr)
        {
            size = static_cast<int>(elems.size());
        }

        // Wrap the element list in an ArrayLiteralNode only when explicitly provided.
        ASTNodePtr initNode = nullptr;
        if (auto initCtx = arrayDecl->arrayInitializer())
        {
            (void)initCtx;
            initNode = std::make_shared<ArrayLiteralNode>(elems);
        }

        // Return VarDeclNode with optional dynamic size expression
        return std::static_pointer_cast<ASTNode>(
            std::make_shared<VarDeclNode>(typeName, name, initNode, /*isArr=*/true,
                                          static_cast<size_t>(size), sizeExpr));
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx)
{
    // If it's "ID = expr;"
    if (ctx->expr())
    {
        std::string name = ctx->ID()->getText();
        ASTNodePtr lhs = std::make_shared<VariableNode>(name);
        ASTNodePtr rhs = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
        auto assignNode = std::make_shared<AssignmentStmtNode>(lhs, rhs);
        return std::static_pointer_cast<ASTNode>(assignNode);
    }

    // "ID;" as a bare statement — treat as no-op (or change to error / other semantics)
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitFunction(BaseParser::FunctionContext *ctx)
{
    std::string name = ctx->ID()->getText();
    std::string returnType = ctx->returnType()->getText();

    std::vector<ParamNodePtr> params;
    for (auto paramCtx : ctx->paramList()->param())
    {
        std::string paramName = paramCtx->ID()->getText();
        std::string paramType = paramCtx->type()->getText();
        params.push_back(std::make_shared<ParamNode>(paramType, paramName));
        // std::cerr << "[Param] " << paramType << " " << paramName << "\n";
    }
    // std::cerr << "[FunctionDecl] " << returnType << " " << name << "(";
    //  for (const auto &param : params)
    //  {
    //      //std::cerr << param->typeName << " " << param->paramName;
    //      if (&param != &params.back())
    //          //std::cerr << ", ";
    //  }
    // std::cerr << ")\n";

    ASTNodePtr body = safe_any_cast<ASTNodePtr>(visitBlock(ctx->block()));

    auto funcNode = std::make_shared<FunctionDeclNode>(
        std::move(returnType),
        std::move(name),
        std::move(params),
        std::move(body));

    // Debug: Verify parameters in funcNode
    // std::cerr << "[FunctionNode Parameters] ";
    for (const auto &param : funcNode->parameters)
    {
        // std::cerr << param->paramName << " ";
    }
    // std::cerr << "\n";

    return std::static_pointer_cast<ASTNode>(funcNode);
}

antlrcpp::Any ASTBuilder::visitArrayAssignStmt(BaseParser::ArrayAssignStmtContext *ctx)
{
    std::string arrayName = ctx->ID()->getText();
    ASTNodePtr base = std::make_shared<VariableNode>(arrayName);
    ASTNodePtr indexNode = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(0)));
    ASTNodePtr valueNode = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(1)));

    ASTNodePtr arrAccess = std::make_shared<ArrayAccessNode>(base, indexNode);
    ASTNodePtr assignNode = std::make_shared<AssignmentStmtNode>(arrAccess, valueNode);
    return std::static_pointer_cast<ASTNode>(assignNode);
}

antlrcpp::Any ASTBuilder::visitArray2DAssignStmt(BaseParser::Array2DAssignStmtContext *ctx)
{
    std::string arrayName = ctx->ID()->getText();
    ASTNodePtr base = std::make_shared<VariableNode>(arrayName);
    ASTNodePtr rowIdx = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(0)));
    ASTNodePtr colIdx = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(1)));
    ASTNodePtr valueNode = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(2)));

    ASTNodePtr arr2DAccess = std::make_shared<Array2DAccessNode>(base, rowIdx, colIdx);
    ASTNodePtr assignNode = std::make_shared<AssignmentStmtNode>(arr2DAccess, valueNode);
    return std::static_pointer_cast<ASTNode>(assignNode);
}

antlrcpp::Any ASTBuilder::visitPrintExpr(BaseParser::PrintExprContext *ctx)
{
    // std::cout << "Entering PrintExpr\n";

    if (ctx->STRING())
    {
        std::string str = ctx->STRING()->getText();
        str = str.substr(1, str.size() - 2);
        // std::cout << "ASTBuilder STRING: " << str << "\n";
        return str;
    }
    else if (ctx->expr())
    {
        // std::cout << "ASTBuilder Visiting inner expr\n";
        auto node = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
        // std::cout << "ASTBuilder exitinng inner expr\n";
        return node;
    }
    else if (ctx->printExpr(0) && ctx->printExpr(1))
    {
        // std::cout << "ASTBuilder Binary PrintExpr +\n";
        auto left = visitPrintExpr(ctx->printExpr(0));
        auto right = visitPrintExpr(ctx->printExpr(1));

        std::string leftStr, rightStr;

        if (left.type() == typeid(std::string))
        {
            leftStr = safe_any_cast<std::string>(left);
        }
        else if (left.type() == typeid(ASTNodePtr))
        {
            auto node = safe_any_cast<ASTNodePtr>(left);
            if (auto lit = dynamic_cast<IntLiteralNode *>(node.get()))
            {
                leftStr = std::to_string(lit->value);
            }
            else if (auto bl = dynamic_cast<BoolLiteralNode *>(node.get()))
            {
                leftStr = bl->value ? "true" : "false";
            }
            else if (auto rl = dynamic_cast<RealLiteralNode *>(node.get()))
            {
                leftStr = std::to_string(rl->value);
            }
            else if (auto var = dynamic_cast<VariableNode *>(node.get()))
            {
                leftStr = var->name;
            }
            else
            {
                leftStr = "UnknownExpr";
            }
        }

        if (right.type() == typeid(std::string))
        {
            rightStr = safe_any_cast<std::string>(right);
        }
        else if (right.type() == typeid(ASTNodePtr))
        {
            auto node = safe_any_cast<ASTNodePtr>(right);
            if (auto lit = dynamic_cast<IntLiteralNode *>(node.get()))
            {
                rightStr = std::to_string(lit->value);
            }
            else if (auto bl = dynamic_cast<BoolLiteralNode *>(node.get()))
            {
                rightStr = bl->value ? "true" : "false";
            }
            else if (auto rl = dynamic_cast<RealLiteralNode *>(node.get()))
            {
                rightStr = std::to_string(rl->value);
            }
            else if (auto var = dynamic_cast<VariableNode *>(node.get()))
            {
                rightStr = var->name;
            }
            else
            {
                rightStr = "[UnknownExpr]";
            }
        }

        return leftStr + rightStr;
    }

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitFunctionCall(BaseParser::FunctionCallContext *ctx)
{
    // Builtins also have keyword tokens; they do not populate ID().
    std::string callee = ctx->getStart()->getText();

    // collect arguments
    std::vector<ASTNodePtr> args;
    if (auto argList = ctx->argumentList())
    {
        for (auto exprCtx : argList->expr())
        {
            args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(exprCtx)));
        }
    }

    if (callee == "hasNode")
    {
        if (args.size() != 2)
            throw std::runtime_error("hasNode requires a graph and a vertex");
        auto graph = std::dynamic_pointer_cast<VariableNode>(args[0]);
        if (!graph)
            throw std::runtime_error("hasNode first argument must be a graph name");
        return ASTNodePtr(std::make_shared<SetContainsExprNode>(
            SetTargetKind::GraphNodes, graph->name, args[1]));
    }
    if (callee == "hasEdge" && args.size() != 3)
        throw std::runtime_error("hasEdge requires a graph and two vertices");

    return ASTNodePtr{
        std::make_shared<FunctionCallNode>(callee, std::move(args))};
}


// `directed: true;` may appear anywhere in the graph body; the grammar allows
// graphProperty* between each section, so scan all of them.
template <typename Ctx>
static bool parseDirectedProperty(Ctx *ctx)
{
    bool directed = false;
    for (auto *prop : ctx->graphProperty())
    {
        const std::string value = prop->drawBoolLiteral()->getText();
        directed = value == "true" || value == "TRUE";
    }
    return directed;
}

antlrcpp::Any ASTBuilder::visitUnweightedGraphDef(BaseParser::UnweightedGraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();
    // std::cerr << "[ASTBuilder] Declaring graph: " << nm << std::endl;

    if (!ctx->edges())
        throw std::runtime_error("graph must have edges (inline list or file):");

    // File-based graph: defer loading to runtime for performance
    if (auto *fe = ctx->edges()->fileEdgeList())
    {
        std::string s = fe->STRING()->getText();
        s = s.substr(1, s.size() - 2);
        auto gnode = std::make_shared<GraphDeclNode>(std::move(nm), std::move(s));
    gnode->directed = parseDirectedProperty(ctx);
        return std::static_pointer_cast<ASTNode>(gnode);
    }

    // Inline graph: build CSR at compile time (small graphs defined in DSL)
    std::vector<std::pair<int, int>> edgesVec;
    if (auto *el = ctx->edges()->edgeList())
    {
        for (auto *eCtx : el->edge())
        {
            int u = std::stoi(eCtx->nodeID(0)->getText());
            int v = std::stoi(eCtx->nodeID(1)->getText());
            edgesVec.emplace_back(u, v);
        }
    }

    llvm::DenseSet<int> idset;
    if (ctx->nodes())
    {
        auto *nl = ctx->nodes()->nodeList();
        if (nl)
        {
            for (auto *idT : nl->nodeID())
                idset.insert(std::stoi(idT->getText()));
        }
    }
    for (auto &e : edgesVec)
    {
        idset.insert(e.first);
        idset.insert(e.second);
    }

    std::vector<int> nodeIds;
    nodeIds.reserve(idset.size());
    for (int x : idset)
        nodeIds.push_back(x);
    std::sort(nodeIds.begin(), nodeIds.end());

    auto nd = std::make_unique<InlineNodeList>(std::move(nodeIds));
    auto ed = std::make_unique<InlineEdgeList>(std::move(edgesVec));

    auto gnode = std::make_shared<GraphDeclNode>(
        std::move(nm),
        std::move(nd),
        std::move(ed));
    gnode->directed = parseDirectedProperty(ctx);

    return std::static_pointer_cast<ASTNode>(gnode);
}

antlrcpp::Any ASTBuilder::visitWeightedGraphDef(BaseParser::WeightedGraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();

    if (!ctx->edges())
        throw std::runtime_error("graph must have edges (inline list or file):");

    if (auto *fe = ctx->edges()->fileEdgeList())
    {
        std::string s = fe->STRING()->getText();
        s = s.substr(1, s.size() - 2);
        auto gnode = std::make_shared<WeightedGraphDeclNode>(std::move(nm), std::move(s));
    gnode->directed = parseDirectedProperty(ctx);
        return std::static_pointer_cast<ASTNode>(gnode);
    }

    std::vector<std::pair<int, int>> edgesVec;
    llvm::DenseMap<std::pair<int, int>, int> weightMap;
    if (auto *el = ctx->edges()->edgeList())
    {
        throw std::runtime_error("inline weighted edges not yet supported");
    }

    // Build node id set: explicit nodes (if any) U nodes from edges
    llvm::DenseSet<int> idset;
    if (ctx->nodes())
    {
        auto *nl = ctx->nodes()->nodeList();
        if (nl)
        {
            for (auto *idT : nl->nodeID())
                idset.insert(std::stoi(idT->getText()));
        }
    }
    for (auto &e : edgesVec)
    {
        idset.insert(e.first);
        idset.insert(e.second);
    }

    std::vector<int> nodeIds;
    nodeIds.reserve(idset.size());
    for (int x : idset)
        nodeIds.push_back(x);
    std::sort(nodeIds.begin(), nodeIds.end());

    // Construct InlineNodeList / InlineEdgeList and pass them to GraphDeclNode
    auto nd = std::make_unique<InlineNodeList>(std::move(nodeIds));
    auto ed = std::make_unique<InlineWeightedEdgeList>(std::move(edgesVec), std::move(weightMap));

    auto gnode = std::make_shared<WeightedGraphDeclNode>(
        std::move(nm),
        std::move(nd),
        std::move(ed));
    gnode->directed = parseDirectedProperty(ctx);

    return std::static_pointer_cast<ASTNode>(gnode);
}

antlrcpp::Any ASTBuilder::visitQueryStatement(BaseParser::QueryStatementContext *ctx)
{
    // std::cerr << " entered visit query statement \n ";

    std::string name = ctx->ID()->getText();
    std::string desc = ctx->STRING()->getText();
    desc = desc.substr(1, desc.size() - 2); // strip quotes
    std::string gname = ctx->graphID()->getText();

    QueryType qtype = QueryType::UNKNOWN;
    if (desc == "chromaticity" || desc == "min_cut")
    {
        qtype = QueryType::INT;
    }
    // else if (desc == "bfs" || desc == "dfs")
    // {
    //     qtype = QueryType::ARRAY;
    // }
    else if (desc == "bfs" || desc == "dfs" || desc == "bfs_src" || desc == "dfs_src" || desc == "dijkstra")
    {
        qtype = QueryType::ARRAY;
    }
    

    auto node = std::make_shared<QueryNode>(name, desc, gname, qtype);

    if (ctx->INT())
    {
        node->source = std::stoi(ctx->INT()->getText());
    }

    return std::static_pointer_cast<ASTNode>(node);
}

// antlrcpp::Any ASTBuilder::visitPrintStatement(BaseParser::PrintStatementContext *ctx)
// {
//     // 1) build the ASTNode for whatever was inside the print
//     ASTNodePtr inner;
//     if (ctx->printExpr())
//     {
//         inner = safe_any_cast<ASTNodePtr>(visitPrintExpr(ctx->printExpr()));
//     }
//     else
//     {
//         // handle printgraph / edge/node cases if you want…
//         throw std::runtime_error("printgraph not yet supported in ASTBuilder");
//     }

//     // 2) wrap in a PrintStmtNode
//     return ASTNodePtr{std::make_shared<PrintStmtNode>(inner)};
// }

// antlrcpp::Any ASTBuilder::visitPrintStatement(BaseParser::PrintStatementContext *ctx)
// {
//     // 1) scalar expression print
//     if (auto exprCtx = ctx->printExpr()) {
//         auto inner = safe_any_cast<ASTNodePtr>(visitPrintExpr(exprCtx));
//         return ASTNodePtr{std::make_shared<PrintStmtNode>(inner)};
//     }
//     // 2) array print
//     else if (auto arrCtx = dynamic_cast<BaseParser::PrintArrayStatementContext*>(ctx)) {
//         auto name = arrCtx->ID()->getText();
//         auto index = safe_any_cast<ASTNodePtr>(visitExpr(arrCtx->expr()));
//         return ASTNodePtr(std::make_shared<PrintArrayNode>(name, index));
//     }
//     // else if (ctx->ID()) {
//     //     std::string varName = ctx->ID()->getText();
//     //     return ASTNodePtr{std::make_shared<PrintStmtNode>(
//     //         std::make_shared<VarExprNode>(varName))};
//     // }
//     else if (ctx->ID()) {
//         std::string varName = ctx->ID()->getText();
//         auto varExpr = std::make_shared<VarExprNode>(varName);
//         return ASTNodePtr{std::make_shared<PrintStmtNode>(varExpr)};
//     }
//     // 3) graph/node/edge print
//     else {
//         throw std::runtime_error("printgraph not yet supported in ASTBuilder");
//     }
// }

antlrcpp::Any ASTBuilder::visitPrintStatement(BaseParser::PrintStatementContext *ctx)
{
    // 1) print <printExpr>;
    if (auto exprCtx = ctx->printExpr())
    {
        antlrcpp::Any any = visitPrintExpr(exprCtx);

        // If printExpr returned an AST node (expression), use it directly
        if (any.type() == typeid(ASTNodePtr))
        {
            ASTNodePtr inner = safe_any_cast<ASTNodePtr>(any);
            return ASTNodePtr{std::make_shared<PrintStmtNode>(inner)};
        }

        // If printExpr returned a std::string (string literal or concatenation),
        // wrap it in a StringLiteralNode so PrintStmtNode can carry it.
        if (any.type() == typeid(std::string))
        {
            std::string s = safe_any_cast<std::string>(any);
            ASTNodePtr strNode = std::make_shared<StringLiteralNode>(s);
            return ASTNodePtr{std::make_shared<PrintStmtNode>(strNode)};
        }

    throw std::runtime_error("visitPrintStatement: unexpected type from visitPrintExpr");
    }

    // 2) print array: 'print' ID '[' expr ']' ';'
    if (auto arrCtx = ctx->printArrayStatement())
    {
        auto name = arrCtx->ID()->getText();
        auto index = safe_any_cast<ASTNodePtr>(visitExpr(arrCtx->expr()));
        return ASTNodePtr(std::make_shared<PrintArrayNode>(name, index));
    }

    // 3) printgraph / node / edge (not implemented)
    if (ctx->printgraph())
    {
        throw std::runtime_error("printgraph not yet supported in ASTBuilder");
    }

    throw std::runtime_error("Unsupported print statement form");
}

antlrcpp::Any ASTBuilder::visitSleepStatement(BaseParser::SleepStatementContext *ctx)
{
    ASTNodePtr durationExpr = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
    auto sleepNode = std::make_shared<SleepStmtNode>(durationExpr);
    return std::static_pointer_cast<ASTNode>(sleepNode);
}


antlrcpp::Any ASTBuilder::visitForeachStatement(BaseParser::ForeachStatementContext *ctx)
{
    if (auto *member = ctx->memberLoopStatement())
    {
        if (auto *n = dynamic_cast<BaseParser::ForeachNodesContext *>(member))
            return visitForeachNodes(n);
        if (auto *e = dynamic_cast<BaseParser::ForeachEdgesContext *>(member))
            return visitForeachEdges(e);
        if (auto *nb = dynamic_cast<BaseParser::ForeachNeighborsContext *>(member))
            return visitForeachNeighbors(nb);
        throw std::runtime_error("internal: unknown memberLoopStatement alternative");
    }
    ForEachTargetType tgt;
    std::string var1, var2;
    ASTNodePtr adjNodeExpr = nullptr;

    // Determine loop type
    auto *loopCtx = ctx->loopTarget();
    if (auto elemCtx = dynamic_cast<BaseParser::ForEachElementContext *>(loopCtx))
    {
        tgt = ForEachTargetType::Element;
        var1 = elemCtx->ID()->getText();
    }
    else if (auto graphCtx = dynamic_cast<BaseParser::ForEachGraphContext *>(loopCtx))
    {
        tgt = ForEachTargetType::Graph;
        var1 = graphCtx->ID()->getText();
    }
    else if (auto motifCtx = dynamic_cast<BaseParser::ForEachMotifContext *>(loopCtx))
    {
        tgt = ForEachTargetType::Motif;
        const auto ids = motifCtx->ID();
        if (!ids.empty())
            var1 = ids[0]->getText();
        if (ids.size() > 1)
            var2 = ids[1]->getText();
    }
    else if (auto plainCtx = dynamic_cast<BaseParser::ForEachPlainContext *>(loopCtx))
    {
        // "for each v in setOrGraph" — plain variable iteration
        tgt = ForEachTargetType::Element; // treat as set element iteration
        var1 = plainCtx->ID()->getText();
    }
    else
    {
        throw std::runtime_error("Unknown loop target in foreachStatement");
    }

    // Create the ForEachStmtNode with proper constructor
    auto fsNode = std::make_shared<ForEachStmtNode>(
        tgt,
        var1,
        var2,
        ctx->graphID()->getText(),
        std::move(adjNodeExpr),
        std::any_cast<ASTNodePtr>(visitBlock(ctx->block())));

    // `for each motif (a, b, c)` may bind more names than var1/var2 hold.
    if (auto motifCtx = dynamic_cast<BaseParser::ForEachMotifContext *>(loopCtx))
        for (auto *id : motifCtx->ID())
            fsNode->motifVars.push_back(id->getText());

    // Return as ASTNodePtr
    return std::static_pointer_cast<ASTNode>(fsNode);
}

// Helper to build GraphConditionNode from grammar
std::shared_ptr<GraphConditionNode> ASTBuilder::buildGraphCondition(BaseParser::GraphConditionContext *ctx)
{
    if (auto *andCtx = dynamic_cast<BaseParser::GraphLogicalAndContext*>(ctx))
    {
        auto left = buildGraphCondition(andCtx->graphCondition(0));
        auto right = buildGraphCondition(andCtx->graphCondition(1));
        return std::make_shared<GraphConditionNode>(GraphConditionOp::And, left, right);
    }
    else if (auto *orCtx = dynamic_cast<BaseParser::GraphLogicalOrContext*>(ctx))
    {
        auto left = buildGraphCondition(orCtx->graphCondition(0));
        auto right = buildGraphCondition(orCtx->graphCondition(1));
        return std::make_shared<GraphConditionNode>(GraphConditionOp::Or, left, right);
    }
    else if (auto *connCtx = dynamic_cast<BaseParser::ConnectedConditionContext*>(ctx))
    {
        int nid = std::stoi(connCtx->nodeID()->getText());
        return std::make_shared<GraphConditionNode>(nid);
    }
    else if (auto *motifCtx = dynamic_cast<BaseParser::MotifConditionContext *>(ctx))
    {
        std::vector<MotifEdgeSpec> edges;
        for (auto *edgeCtx : motifCtx->motifEdge())
            edges.push_back(parseMotifEdge(edgeCtx));
        return std::make_shared<GraphConditionNode>(std::move(edges));
    }
    else if (auto *degCtx = dynamic_cast<BaseParser::DegreeConditionContext*>(ctx))
    {
        int val = std::stoi(degCtx->INT()->getText());
        GraphDegreeOp dop = GraphDegreeOp::None;
        if (degCtx->EQUAL()) dop = GraphDegreeOp::Eq;
        else if (degCtx->NOTEQUAL()) dop = GraphDegreeOp::Ne;
        else if (degCtx->LESSEQUAL()) dop = GraphDegreeOp::Le;
        else if (degCtx->GREATEREQUAL()) dop = GraphDegreeOp::Ge;
        else if (degCtx->LESSTHAN()) dop = GraphDegreeOp::Lt;
        else if (degCtx->GREATERTHAN()) dop = GraphDegreeOp::Gt;
        return std::make_shared<GraphConditionNode>(dop, val);
    }
    else if (auto *cycleCtx = dynamic_cast<BaseParser::CycleConditionContext*>(ctx))
    {
        (void)cycleCtx;
        return std::make_shared<GraphConditionNode>();
    }
    else if (auto *parenCtx = dynamic_cast<BaseParser::ParenGraphConditionContext*>(ctx))
    {
        return buildGraphCondition(parenCtx->graphCondition());
    }
    else
    {
        throw std::runtime_error("buildGraphCondition: unsupported condition type");
    }
}



antlrcpp::Any ASTBuilder::visitSetDecl(
    BaseParser::SetDeclContext *ctx)
{
    std::string name = ctx->ID()->getText();

    ASTNodePtr initNode = nullptr;

    // Check if this declaration has an initializer
    if (ctx->setInitializer())
    {
        // Visit the setInitializer to get the SetLiteralNode
        initNode = safe_any_cast<ASTNodePtr>(
            visitSetInitializer(ctx->setInitializer()),
            "visitSetDecl");
    }

    else if (ctx->setExpr())
    {
        initNode = safe_any_cast<ASTNodePtr>(
            visitSetExpr(ctx->setExpr()),
            "visitSetDecl");
    }

    auto node = std::make_shared<SetDeclNode>(name, initNode);
    //SetKinds[node->name] = inferSetKind(node->initializer.get());
    //SetKinds[setDecl->name] = inferSetKind(setDecl->initializer.get());
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetInitializer(
    BaseParser::SetInitializerContext *ctx)
{
    std::vector<ASTNodePtr> elements;

    // Get all expressions in the set literal
    if (ctx->expr().size() > 0)
    {
        elements.reserve(ctx->expr().size());
        for (auto *e : ctx->expr())
        {
            elements.push_back(
                safe_any_cast<ASTNodePtr>(
                    visitExpr(e),
                    "visitSetInitializer"));
        }
    }

    auto node = std::make_shared<SetLiteralNode>(elements);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetOperation(BaseParser::SetOperationContext *ctx)
{
    std::string targetName = ctx->ID()->getText();

    ASTNodePtr setExpr = safe_any_cast<ASTNodePtr>(
        visitSetExpr(ctx->setExpr()),
        "visitSetOperation");

    auto node = std::make_shared<SetOperationNode>(targetName, setExpr);
    //SetKinds[node->targetName] = inferSetKind(node->expr.get());
    //SetKinds[setOp->targetName] = inferSetKind(setOp->expr.get());
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetExpr(BaseParser::SetExprContext *ctx)
{
    if (auto unionCtx = dynamic_cast<BaseParser::SetUnionContext *>(ctx))
    {
        return visitSetUnion(unionCtx);
    }
    else if (auto intersectCtx = dynamic_cast<BaseParser::SetIntersectContext *>(ctx))
    {
        return visitSetIntersect(intersectCtx);
    }
    else if (auto nodesCtx = dynamic_cast<BaseParser::GraphNodesSetContext *>(ctx))
    {
        std::string gname = nodesCtx->graphID()->getText();
        return ASTNodePtr(std::make_shared<GraphMemberSetNode>(gname, GraphMemberKind::Nodes));
    }
    else if (auto edgesCtx = dynamic_cast<BaseParser::GraphEdgesSetContext *>(ctx))
    {
        std::string gname = edgesCtx->graphID()->getText();
        return ASTNodePtr(std::make_shared<GraphMemberSetNode>(gname, GraphMemberKind::Edges));
    }
    else if (auto idCtx = dynamic_cast<BaseParser::SetIdContext *>(ctx))
    {
        return visitSetId(idCtx);
    }
    else if (auto literalCtx = dynamic_cast<BaseParser::SetLiteralContext *>(ctx))
    {
        return visitSetLiteral(literalCtx);
    }
    else if (auto parenCtx = dynamic_cast<BaseParser::ParenSetContext *>(ctx))
    {
        return visitParenSet(parenCtx);
    }

    throw std::runtime_error("Unsupported setExpr: " + ctx->getText());
}

antlrcpp::Any ASTBuilder::visitSetUnion(BaseParser::SetUnionContext *ctx)
{
    ASTNodePtr lhs = safe_any_cast<ASTNodePtr>(
        visitSetExpr(ctx->setExpr(0)),
        "visitSetUnion left");

    ASTNodePtr rhs = safe_any_cast<ASTNodePtr>(
        visitSetExpr(ctx->setExpr(1)),
        "visitSetUnion right");

    auto node = std::make_shared<SetBinaryExprNode>("union", lhs, rhs);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetIntersect(BaseParser::SetIntersectContext *ctx)
{
    ASTNodePtr lhs = safe_any_cast<ASTNodePtr>(
        visitSetExpr(ctx->setExpr(0)),
        "visitSetIntersect left");

    ASTNodePtr rhs = safe_any_cast<ASTNodePtr>(
        visitSetExpr(ctx->setExpr(1)),
        "visitSetIntersect right");

    auto node = std::make_shared<SetBinaryExprNode>("intersect", lhs, rhs);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetId(BaseParser::SetIdContext *ctx)
{
    std::string name = ctx->ID()->getText();

    // Reuse VariableNode or create a dedicated SetIdNode
    auto node = std::make_shared<VariableNode>(name);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetLiteral(BaseParser::SetLiteralContext *ctx)
{
    // SetLiteral in setExpr refers to setInitializer
    return visitSetInitializer(ctx->setInitializer());
}

antlrcpp::Any ASTBuilder::visitParenSet(BaseParser::ParenSetContext *ctx)
{
    // Just visit the inner expression
    return visitSetExpr(ctx->setExpr());
}

antlrcpp::Any ASTBuilder::visitSetMethodCall(BaseParser::SetMethodCallContext *ctx)
{
    if (auto addCtx = dynamic_cast<BaseParser::SetAddMethodContext *>(ctx))
    {
        return visitSetAddMethod(addCtx);
    }
    else if (auto removeCtx = dynamic_cast<BaseParser::SetRemoveMethodContext *>(ctx))
    {
        return visitSetRemoveMethod(removeCtx);
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitSetAddMethod(BaseParser::SetAddMethodContext *ctx)
{
    auto kind = parseSetTarget(ctx->setTarget());
    std::string name;
    if (kind == SetTargetKind::Variable)
        name = ctx->setTarget()->ID()->getText();
    else
        name = ctx->setTarget()->graphID()->getText();
    ASTNodePtr arg = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()), "visitSetAddMethod");

    auto node = std::make_shared<SetMethodCallNode>(kind, name, "add", arg);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetRemoveMethod(BaseParser::SetRemoveMethodContext *ctx)
{
    auto kind = parseSetTarget(ctx->setTarget());
    std::string name;
    if (kind == SetTargetKind::Variable)
        name = ctx->setTarget()->ID()->getText();
    else
        name = ctx->setTarget()->graphID()->getText();
    ASTNodePtr arg = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()), "visitSetRemoveMethod");

    auto node = std::make_shared<SetMethodCallNode>(kind, name, "remove", arg);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitSetContainsExpr(BaseParser::SetContainsExprContext *ctx)
{
    auto kind = parseSetTarget(ctx->setTarget());
    // Extract the proper name (graph name for G.nodes/G.edges, variable name for ID)
    std::string name;
    if (kind == SetTargetKind::Variable)
        name = ctx->setTarget()->ID()->getText();
    else
        name = ctx->setTarget()->graphID()->getText();
    ASTNodePtr arg = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()), "visitSetContainsExpr");
    auto node = std::make_shared<SetContainsExprNode>(kind, name, arg);
    return std::static_pointer_cast<ASTNode>(node);
}

SetTargetKind ASTBuilder::parseSetTarget(BaseParser::SetTargetContext *ctx)
{
    if (ctx->ID() && !ctx->graphID())
        return SetTargetKind::Variable;
    if (ctx->NODE()) // 'nodes'
        return SetTargetKind::GraphNodes;
    return SetTargetKind::GraphEdges; // falls through to 'edges'
}

antlrcpp::Any ASTBuilder::visitSwapStatement(BaseParser::SwapStatementContext *ctx)
{
    std::string a = ctx->ID(0)->getText();
    std::string b = ctx->ID(1)->getText();
    auto node = std::make_shared<SwapStmtNode>(a, b);
    return std::static_pointer_cast<ASTNode>(node);
}

// ── plan rows 1-4: new declaration / mutation surface ───────────────────────
// Every function here builds the SAME AST node the old spelling builds
// (GraphDeclNode / WeightedGraphDeclNode / GraphUpdateNode), so IRGen,
// SemanticAnalyzer and the mid-end passes are untouched: only the surface
// spelling of the program changes.

namespace {
// The declaration modifiers are an inlined ('directed' | 'weighted')* block, so
// they appear as terminal children of the declaration context.
void readGraphMods(antlr4::ParserRuleContext *ctx, bool &directed, bool &weighted)
{
    for (auto *child : ctx->children)
        if (auto *t = dynamic_cast<antlr4::tree::TerminalNode *>(child))
        {
            const std::string s = t->getText();
            if (s == "directed")
                directed = true;
            else if (s == "weighted")
                weighted = true;
        }
}

// Literal node/edge targets for the call forms (literals only, per the agreed
// surface).  A `G.addNodes(0,1,2)` is one GraphUpdateNode with three Node
// targets, exactly like `add 0, 1, 2 to G;`.
std::vector<GraphUpdateTarget> litNodeTargets(BaseParser::NodeListContext *nl)
{
    std::vector<GraphUpdateTarget> out;
    if (!nl)
        return out;
    for (auto *n : nl->nodeID())
        out.emplace_back(std::make_shared<IntLiteralNode>(std::stoi(n->getText())));
    return out;
}

std::vector<GraphUpdateTarget> litEdgeTargets(BaseParser::EdgeListContext *el)
{
    std::vector<GraphUpdateTarget> out;
    if (!el)
        return out;
    for (auto *e : el->edge())
    {
        ASTNodePtr s = std::make_shared<IntLiteralNode>(std::stoi(e->nodeID(0)->getText()));
        ASTNodePtr d = std::make_shared<IntLiteralNode>(std::stoi(e->nodeID(1)->getText()));
        out.emplace_back(std::move(s), std::move(d));
    }
    return out;
}

std::shared_ptr<ASTNode> makeUpdateNode(const std::string &gname,
                                       GraphUpdateKind kind,
                                       std::vector<GraphUpdateTarget> targets)
{
    return std::static_pointer_cast<ASTNode>(
        std::make_shared<GraphUpdateNode>(kind, gname, std::move(targets)));
}
} // namespace

antlrcpp::Any ASTBuilder::visitPlainGraphDef(BaseParser::PlainGraphDefContext *ctx)
{
    bool directed = false, weighted = false;
    readGraphMods(ctx, directed, weighted);
    const std::string name = ctx->graphID()->getText();
    std::shared_ptr<ASTNode> g;
    if (weighted)
        g = std::make_shared<WeightedGraphDeclNode>(
            name, std::make_unique<InlineNodeList>(std::vector<int>{}),
            std::make_unique<InlineWeightedEdgeList>(
                std::vector<std::pair<int, int>>{},
                llvm::DenseMap<std::pair<int, int>, int>{}));
    else
        g = std::make_shared<GraphDeclNode>(
            name, std::make_unique<InlineNodeList>(std::vector<int>{}),
            std::make_unique<InlineEdgeList>(std::vector<std::pair<int, int>>{}));
    if (directed) // `directed graph G;` -- the flag the braced form sets too
    {
        if (auto *wd = dynamic_cast<WeightedGraphDeclNode *>(g.get()))
            wd->directed = true;
        else
            static_cast<GraphDeclNode *>(g.get())->directed = true;
    }
    return std::static_pointer_cast<ASTNode>(g);
}

antlrcpp::Any ASTBuilder::visitLoadGraphDef(BaseParser::LoadGraphDefContext *ctx)
{
    bool directed = false, weighted = false;
    readGraphMods(ctx, directed, weighted);
    const std::string name = ctx->graphID()->getText();
    std::string file = ctx->STRING()->getText();
    if (file.size() >= 2 && file.front() == '"' && file.back() == '"')
        file = file.substr(1, file.size() - 2);
    std::shared_ptr<ASTNode> g;
    if (weighted)
    {
        auto wd = std::make_shared<WeightedGraphDeclNode>(name, file);
        wd->directed = directed;
        g = wd;
    }
    else
    {
        auto d = std::make_shared<GraphDeclNode>(name, file);
        d->directed = directed;
        g = d;
    }
    return std::static_pointer_cast<ASTNode>(g);
}

antlrcpp::Any ASTBuilder::visitGraphAddNodesCall(BaseParser::GraphAddNodesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Add,
                          litNodeTargets(ctx->nodeList()));
}

antlrcpp::Any ASTBuilder::visitGraphAddEdgesCall(BaseParser::GraphAddEdgesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Add,
                          litEdgeTargets(ctx->edgeList()));
}

antlrcpp::Any ASTBuilder::visitGraphRemoveNodesCall(BaseParser::GraphRemoveNodesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Remove,
                          litNodeTargets(ctx->nodeList()));
}

antlrcpp::Any ASTBuilder::visitGraphRemoveEdgesCall(BaseParser::GraphRemoveEdgesCallContext *ctx)
{
    return makeUpdateNode(ctx->graphID()->getText(), GraphUpdateKind::Remove,
                          litEdgeTargets(ctx->edgeList()));
}

// Rows 5-7: member-style loop headers.  Each builds the same ForEachStmtNode the
// classic spelling builds -- Vertex = CSR vertex scan, Edge = edge scan,
// Neighbor = neighbour scan -- so the effect analysis sees the same loops.
antlrcpp::Any ASTBuilder::visitForeachNodes(BaseParser::ForeachNodesContext *ctx)
{
    auto fs = std::make_shared<ForEachStmtNode>(
        ForEachTargetType::Vertex, ctx->ID()->getText(), std::string(),
        ctx->graphID()->getText(), nullptr,
        std::any_cast<ASTNodePtr>(visitBlock(ctx->block())));
    return std::static_pointer_cast<ASTNode>(fs);
}

antlrcpp::Any ASTBuilder::visitForeachEdges(BaseParser::ForeachEdgesContext *ctx)
{
    auto fs = std::make_shared<ForEachStmtNode>(
        ForEachTargetType::Edge, ctx->ID(0)->getText(), ctx->ID(1)->getText(),
        ctx->graphID()->getText(), nullptr,
        std::any_cast<ASTNodePtr>(visitBlock(ctx->block())));
    return std::static_pointer_cast<ASTNode>(fs);
}

antlrcpp::Any ASTBuilder::visitForeachNeighbors(BaseParser::ForeachNeighborsContext *ctx)
{
    ASTNodePtr adj = std::any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
    auto fs = std::make_shared<ForEachStmtNode>(
        ForEachTargetType::Neighbor, ctx->ID()->getText(), std::string(),
        ctx->graphID()->getText(), std::move(adj),
        std::any_cast<ASTNodePtr>(visitBlock(ctx->block())));
    return std::static_pointer_cast<ASTNode>(fs);
}

// Row 8: membership checks (the dispatch in visitCondition calls these overrides).
antlrcpp::Any ASTBuilder::visitHasNodeCheck(BaseParser::HasNodeCheckContext *ctx)
{
    ASTNodePtr val = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
    return ASTNodePtr(std::make_shared<SetContainsExprNode>(
        SetTargetKind::GraphNodes, ctx->graphID()->getText(), val));
}

antlrcpp::Any ASTBuilder::visitHasEdgeCheck(BaseParser::HasEdgeCheckContext *ctx)
{
    std::vector<ASTNodePtr> args;
    args.push_back(std::make_shared<VariableNode>(ctx->graphID()->getText()));
    args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(0))));
    args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(1))));
    return ASTNodePtr(std::make_shared<FunctionCallNode>("hasEdge", args));
}

// Row 8 (call spellings): `hasNode(G, x)` / `hasEdge(G, u, v)` build exactly the
// same AST as the member forms, so both spellings stay interchangeable.
antlrcpp::Any ASTBuilder::visitHasNodeCall(BaseParser::HasNodeCallContext *ctx)
{
    ASTNodePtr val = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
    return ASTNodePtr(std::make_shared<SetContainsExprNode>(
        SetTargetKind::GraphNodes, ctx->graphID()->getText(), val));
}

antlrcpp::Any ASTBuilder::visitHasEdgeCall(BaseParser::HasEdgeCallContext *ctx)
{
    std::vector<ASTNodePtr> args;
    args.push_back(std::make_shared<VariableNode>(ctx->graphID()->getText()));
    args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(0))));
    args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(1))));
    return ASTNodePtr(std::make_shared<FunctionCallNode>("hasEdge", args));
}

// Row 13: `M = in G search motif { ... };` -> the same MotifMatchesDeclNode the
// `motifs` spelling builds (bindings, first-appearance variable order).
antlrcpp::Any ASTBuilder::visitMotifSearchDecl(BaseParser::MotifSearchDeclContext *ctx)
{
    std::vector<MotifEdgeSpec> edges;
    std::vector<std::string> variables;
    std::unordered_set<std::string> seen;
    for (auto *edgeCtx : ctx->motifEdge())
    {
        MotifEdgeSpec edge = parseMotifEdge(edgeCtx);
        if (seen.insert(edge.source).second)
            variables.push_back(edge.source);
        if (seen.insert(edge.target).second)
            variables.push_back(edge.target);
        edges.push_back(std::move(edge));
    }
    auto node = std::make_shared<MotifMatchesDeclNode>(
        ctx->ID()->getText(), ctx->graphID()->getText(), std::move(edges), std::move(variables));
    return std::static_pointer_cast<ASTNode>(node);
}
