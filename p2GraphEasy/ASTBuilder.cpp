#include "ASTBuilder.h"
#include <iostream>
#include "ASTNode.h"
#include <stdexcept>
#include <vector>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <cstring> // for std::memcpy
#include "llvm/Support/Allocator.h"

antlrcpp::Any ASTBuilder::visitProgram(BaseParser::ProgramContext *ctx)
{
    std::vector<ASTNodePtr> items;

    // 1) collect function declarations
    for (auto funcCtx : ctx->function())
    {
        auto fnAny = visitFunction(funcCtx);
        auto fnNode = safe_any_cast<ASTNodePtr>(fnAny, "visitProgram");
        items.push_back(fnNode);
    }

    // 2) collect top‑level statements
    for (auto stmtCtx : ctx->statement())
    {
        auto stAny = visitStatement(stmtCtx);
        
        if (stAny.has_value() && stAny.type() == typeid(ASTNodePtr))
        {
            items.push_back(safe_any_cast<ASTNodePtr>(stAny, "visitProgram"));
        }
    }

    // 3) package into your root
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
        else
            throw std::runtime_error("internal: unknown graphDef alternative");
        // produce a GraphDeclNode
        // return visitGraphDef(ctx->graphDef());
    }
    else if (ctx->queryStatement())
    {
        return visitQueryStatement(ctx->queryStatement());
    }
    else if (ctx->nodeEdgeOperation())
    {
        auto *op = ctx->nodeEdgeOperation();

        std::vector<int> nodes;
        std::vector<std::pair<int,int>> edges;
        std::string gname;
        GraphUpdateKind kind;

        if (auto *add = op->addOperation())
        {
            kind = GraphUpdateKind::Add;
            gname = add->graphID()->getText();
            auto *t = add->addTargets();

            if (t->nodeID())
            {
                nodes.push_back(std::stoi(t->nodeID()->getText()));
            }
            else if (t->edge())
            {
                int u = std::stoi(t->edge()->nodeID(0)->getText());
                int v = std::stoi(t->edge()->nodeID(1)->getText());
                edges.emplace_back(u, v);
            }
            else if (t->nodeList())
            {
                for (auto *idT : t->nodeList()->nodeID())
                    nodes.push_back(std::stoi(idT->getText()));
            }
            else if (t->edgeList())
            {
                for (auto *eCtx : t->edgeList()->edge())
                {
                    int u = std::stoi(eCtx->nodeID(0)->getText());
                    int v = std::stoi(eCtx->nodeID(1)->getText());
                    edges.emplace_back(u, v);
                }
            }
        }
        else if (auto *rem = op->removeOperation())
        {
            kind = GraphUpdateKind::Remove;
            gname = rem->graphID()->getText();
            auto *t = rem->removeTargets();

            if (t->nodeID())
            {
                nodes.push_back(std::stoi(t->nodeID()->getText()));
            }
            else if (t->edge())
            {
                int u = std::stoi(t->edge()->nodeID(0)->getText());
                int v = std::stoi(t->edge()->nodeID(1)->getText());
                edges.emplace_back(u, v);
            }
            else if (t->nodeList())
            {
                for (auto *idT : t->nodeList()->nodeID())
                    nodes.push_back(std::stoi(idT->getText()));
            }
            else if (t->edgeList())
            {
                for (auto *eCtx : t->edgeList()->edge())
                {
                    int u = std::stoi(eCtx->nodeID(0)->getText());
                    int v = std::stoi(eCtx->nodeID(1)->getText());
                    edges.emplace_back(u, v);
                }
            }
        }
        
        else
        {
            throw std::runtime_error("nodeEdgeOperation: unknown alternative");
        }

        auto up = std::make_shared<GraphUpdateNode>(kind, gname, nodes, edges);
        return std::static_pointer_cast<ASTNode>(up);
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

        auto node = std::make_shared<GraphComprehensionNode>(
            target, gname, std::move(ops), std::move(operands), cond);
        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->showgraph())
    {
        std::string gname = ctx->showgraph()->graphID()->getText();
        auto node = std::make_shared<ShowGraphNode>(gname);
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

        // Wrap the element list in an ArrayLiteralNode
        auto arrayLit = std::make_shared<ArrayLiteralNode>(elems);

        // Return VarDeclNode with optional dynamic size expression
        return std::static_pointer_cast<ASTNode>(
            std::make_shared<VarDeclNode>(typeName, name, arrayLit, /*isArr=*/true,
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
    std::string callee = ctx->ID()->getText();

    // collect arguments
    std::vector<ASTNodePtr> args;
    if (auto argList = ctx->argumentList())
    {
        for (auto exprCtx : argList->expr())
        {
            args.push_back(safe_any_cast<ASTNodePtr>(visitExpr(exprCtx)));
        }
    }

    return ASTNodePtr{
        std::make_shared<FunctionCallNode>(callee, std::move(args))};
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

    return std::static_pointer_cast<ASTNode>(gnode);
}

antlrcpp::Any ASTBuilder::visitWeightedGraphDef(BaseParser::WeightedGraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();
    // std::cerr << "[ASTBuilder] Declaring Weighted graph: " << nm << std::endl;

    if (!ctx->edges())
        throw std::runtime_error("graph must have edges (inline list or file):");

    // Materialize edges exactly once
    std::vector<std::pair<int, int>> edgesVec;
    llvm::DenseMap<std::pair<int, int>, int> weightMap;
    if (auto *fe = ctx->edges()->fileEdgeList())
    {
        std::string s = fe->STRING()->getText();
        s = s.substr(1, s.size() - 2);
        WeightedFileEdgeList tmpFile(std::move(s));
        tmpFile.materializeEdges(edgesVec, weightMap); // file read happens here once
    }
    // for inline !!!!NOT SUPPORTED YET!!!!!!
    else if (auto *el = ctx->edges()->edgeList())
    {
        // for (auto *eCtx : el->edge())
        // {
        //     int u = std::stoi(eCtx->nodeID(0)->getText());
        //     int v = std::stoi(eCtx->nodeID(1)->getText());
        //     edgesVec.emplace_back(u, v);
        // }
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
    ForEachTargetType tgt;
    std::string var1, var2;
    ASTNodePtr adjNodeExpr = nullptr;

    // Determine loop type
    auto *loopCtx = ctx->loopTarget();
    if (auto vertexCtx = dynamic_cast<BaseParser::ForEachVertexContext *>(loopCtx))
    {
        tgt = ForEachTargetType::Vertex;
        var1 = vertexCtx->ID()->getText();
    }
    else if (auto edgeCtx = dynamic_cast<BaseParser::ForEachEdgeContext *>(loopCtx))
    {
        tgt = ForEachTargetType::Edge;
        auto ids = edgeCtx->ID();
        var1 = ids[0]->getText();
        var2 = ids[1]->getText();
    }
    else if (auto adjCtx = dynamic_cast<BaseParser::ForEachAdjContext *>(loopCtx))
    {
        tgt = ForEachTargetType::Neighbor;
        var1 = adjCtx->ID()->getText();
        adjNodeExpr = safe_any_cast<ASTNodePtr>(visitExpr(adjCtx->expr()));
    }
    else if (auto elemCtx = dynamic_cast<BaseParser::ForEachElementContext *>(loopCtx))
    {
        tgt = ForEachTargetType::Element;
        var1 = elemCtx->ID()->getText();
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