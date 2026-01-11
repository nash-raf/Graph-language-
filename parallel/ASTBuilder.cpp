#include "ASTBuilder.h"
#include <iostream>
#include "ASTNode.h"
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include "llvm/ADT/DenseSet.h"
#include <algorithm>

int ASTBuilder::evaluate(ASTNodePtr node)
{
    if (auto lit = dynamic_cast<IntLiteralNode *>(node.get()))
    {
        return lit->value;
    }
    if (auto var = dynamic_cast<VariableNode *>(node.get()))
    {
        auto it = symbolTable.find(var->name);
        if (it == symbolTable.end())
            throw std::runtime_error("Undefined variable: " + var->name);
        return it->second;
    }
    if (auto bin = dynamic_cast<BinaryExprNode *>(node.get()))
    {
        int L = evaluate(bin->lhs);
        int R = evaluate(bin->rhs);
        if (bin->op == "+")
            return L + R;
        if (bin->op == "-")
            return L - R;
        if (bin->op == "*")
            return L * R;
        if (bin->op == "/")
            return R != 0 ? L / R : 0;
    }
    if (auto call = dynamic_cast<FunctionCallNode *>(node.get()))
    {

        auto it = functionTable.find(call->name);
        if (it == functionTable.end())
            throw std::runtime_error("Undefined function: " + call->name);
        auto decl = it->second;

        std::vector<int> argValues;
        for (const auto &arg : call->arguments)
        {
            argValues.push_back(evaluate(arg));
        }

        std::unordered_map<std::string, int> oldSymbols = symbolTable;
        symbolTable.clear();

        for (size_t i = 0; i < decl->parameters.size(); ++i)
        {
            auto &p = decl->parameters[i];
            symbolTable[p->paramName] = argValues[i];
        }

        int ret = 0;
        auto block = dynamic_cast<BlockStmtNode *>(decl->body.get());
        for (auto &stmt : block->statements)
        {
            if (auto retStmt = dynamic_cast<ReturnStmtNode *>(stmt.get()))
            {
                ret = evaluate(retStmt->returnValue);
                break;
            }
        }

        symbolTable = std::move(oldSymbols);
        return ret;
    }

    throw std::runtime_error("wrongnode");
}

antlrcpp::Any ASTBuilder::visitProgram(BaseParser::ProgramContext *ctx)
{
    std::vector<ASTNodePtr> items;

    for (auto funcCtx : ctx->function())
    {
        auto fnAny = visitFunction(funcCtx);
        auto fnNode = safe_any_cast<ASTNodePtr>(fnAny, "visitProgram");
        items.push_back(fnNode);
    }

    for (auto stmtCtx : ctx->statement())
    {
        auto stAny = visitStatement(stmtCtx);

        if (stAny.has_value() && stAny.type() == typeid(ASTNodePtr))
        {
            items.push_back(safe_any_cast<ASTNodePtr>(stAny, "visitProgram"));
        }
    }

    auto prog = std::make_shared<ProgramNode>(std::move(items));
    return prog;
}

antlrcpp::Any ASTBuilder::visitStatement(BaseParser::StatementContext *ctx)
{

    if (ctx->printStatement())
    {

        return visitPrintStatement(ctx->printStatement());
    }
    else if (ctx->varDecl())
    {

        return visitVarDecl(ctx->varDecl());
    }
    else if (ctx->assignmentStatement())
    {

        return visitAssignmentStatement(ctx->assignmentStatement());
    }
    else if (ctx->conditionalStatement())
    {
        return visitConditionalStatement(ctx->conditionalStatement());
    }

    else if (ctx->whileStatement())
    {
        return visitWhileStatement(ctx->whileStatement());
    }
    else if (ctx->arrayAssignStatement())
    {

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

        return visitGraphDef(ctx->graphDef());
    }
    else if (ctx->queryStatement())
    {
        return visitQueryStatement(ctx->queryStatement());
    }
    else if (ctx->setDecl())
    {
        return visitSetDecl(ctx->setDecl());
    }

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
    throw std::runtime_error("Unsupported condition: " + ctx->getText());
}

antlrcpp::Any ASTBuilder::visitWhileStatement(BaseParser::WhileStatementContext *ctx)
{

    ASTNodePtr cond = safe_any_cast<ASTNodePtr>(visitCondition(ctx->condition()));
    ASTNodePtr blk = safe_any_cast<ASTNodePtr>(visitBlock(ctx->block()));

    auto whileNode = std::make_shared<WhileStmtNode>(cond, blk);
    return std::static_pointer_cast<ASTNode>(whileNode);
}

antlrcpp::Any ASTBuilder::visitExpr(BaseParser::ExprContext *ctx)
{

    if (auto mulDivContext = dynamic_cast<BaseParser::MulDivExprContext *>(ctx))
    {
        auto lhs = safe_any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(0)));
        auto rhs = safe_any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(1)));
        std::string op = mulDivContext->TIMES() ? "*" : "/";

        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, lhs, rhs));
    }
    else if (auto fctx = dynamic_cast<BaseParser::FuncExprContext *>(ctx))
    {
        return visitFunctionCall(fctx->functionCall());
    }

    else if (auto addSubContext = dynamic_cast<BaseParser::AddSubExprContext *>(ctx))
    {

        auto lhs = safe_any_cast<ASTNodePtr>(visitExpr(addSubContext->expr(0)));

        auto rhs = safe_any_cast<ASTNodePtr>(visitExpr(addSubContext->expr(1)));
        std::string op = addSubContext->PLUS() ? "+" : "-";

        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, lhs, rhs));
    }

    else if (auto intExprContext = dynamic_cast<BaseParser::IntExprContext *>(ctx))
    {
        int val = std::stoi(intExprContext->getText());

        return ASTNodePtr(std::make_shared<IntLiteralNode>(val));
    }
    else if (auto idExprContext = dynamic_cast<BaseParser::IdExprContext *>(ctx))
    {
        std::string name = idExprContext->getText();

        return ASTNodePtr(std::make_shared<VariableNode>(name));
    }
    else if (auto parenContext = dynamic_cast<BaseParser::ParenExprContext *>(ctx))
    {

        return visitExpr(parenContext->expr());
    }
    else if (auto arrCtx = dynamic_cast<BaseParser::ArrayAccessExprContext *>(ctx))
    {
        std::string name = arrCtx->ID()->getText();
        ASTNodePtr indexNode = safe_any_cast<ASTNodePtr>(visitExpr(arrCtx->expr()));

        if (arrayTable.find(name) != arrayTable.end())
        {
            if (auto *idxLit = dynamic_cast<IntLiteralNode *>(indexNode.get()))
            {
                int index = idxLit->value;
                auto &arr = arrayTable[name];
                if (index >= 0 && index < (int)arr.size())
                {
                    ASTNodePtr elem = arr[index];

                    if (auto *elemLit = dynamic_cast<IntLiteralNode *>(elem.get()))
                    {
                        return ASTNodePtr(std::make_shared<IntLiteralNode>(elemLit->value));
                    }

                    if (auto *bin = dynamic_cast<BinaryExprNode *>(elem.get()))
                    {

                        auto *L = dynamic_cast<IntLiteralNode *>(bin->lhs.get());
                        auto *R = dynamic_cast<IntLiteralNode *>(bin->rhs.get());
                        if (L && R)
                        {
                            int Lv = L->value;
                            int Rv = R->value;
                            if (bin->op == "+")
                                return ASTNodePtr(std::make_shared<IntLiteralNode>(Lv + Rv));
                            else if (bin->op == "-")
                                return ASTNodePtr(std::make_shared<IntLiteralNode>(Lv - Rv));
                            else if (bin->op == "*")
                                return ASTNodePtr(std::make_shared<IntLiteralNode>(Lv * Rv));
                            else if (bin->op == "/")
                            {

                                return ASTNodePtr(std::make_shared<IntLiteralNode>(Lv / Rv));
                            }
                        }
                    }
                }
            }
        }

        ASTNodePtr base = std::make_shared<VariableNode>(name);
        return ASTNodePtr(std::make_shared<ArrayAccessNode>(base, indexNode));
    }

    throw std::runtime_error("ASTBuilder Unsupported expr: " + ctx->getText());

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitVarDecl(BaseParser::VarDeclContext *ctx)
{

    if (auto sd = dynamic_cast<BaseParser::SimpleDeclarationContext *>(ctx))
    {
        std::string name = sd->ID()->getText();
        ASTNodePtr init = nullptr;
        if (sd->expr())
        {
            init = safe_any_cast<ASTNodePtr>(visitExpr(sd->expr()));
        }
        if (init && init->type == ASTNodeType::ArrayLiteral)
        {

            auto *arrLit = static_cast<ArrayLiteralNode *>(init.get());
            return std::static_pointer_cast<ASTNode>(
                std::make_shared<VarDeclNode>(name, init, /*isArr=*/true, arrLit->elements.size()));
        }
        else
        {
            return std::static_pointer_cast<ASTNode>(
                std::make_shared<VarDeclNode>(name, init, /*isArr=*/false, 0));
        }
    }
    else if (auto arrayDecl = dynamic_cast<BaseParser::ArrayDeclarationContext *>(ctx))
    {

        auto declarator = arrayDecl->arrayDeclarator();
        if (!declarator)
            throw std::runtime_error("Missing arrayDeclarator in array declaration");

        std::string name;
        int size = 0;
        if (auto sized = dynamic_cast<BaseParser::SizedArrayContext *>(declarator))
        {
            name = sized->ID()->getText();
            size = std::stoi(sized->INT()->getText());
        }
        else if (auto unsized = dynamic_cast<BaseParser::UnsizedArrayContext *>(declarator))
        {
            name = unsized->ID()->getText();
        }
        else
        {
            throw std::runtime_error("Unknown arrayDeclarator type");
        }

        std::vector<ASTNodePtr> elems;
        if (auto initCtx = arrayDecl->arrayInitializer())
        {
            for (auto *eCtx : initCtx->expr())
            {
                elems.push_back(safe_any_cast<ASTNodePtr>(visitExpr(eCtx)));
            }
        }

        if (size == 0)
        {
            size = static_cast<int>(elems.size());
        }

        auto arrayLit = std::make_shared<ArrayLiteralNode>(elems);

        return std::static_pointer_cast<ASTNode>(
            std::make_shared<VarDeclNode>(name, arrayLit, /*isArr=*/true, static_cast<size_t>(size)));
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx)
{

    if (ctx->expr())
    {
        std::string name = ctx->ID()->getText();
        ASTNodePtr lhs = std::make_shared<VariableNode>(name);
        ASTNodePtr rhs = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
        auto assignNode = std::make_shared<AssignmentStmtNode>(lhs, rhs);
        return std::static_pointer_cast<ASTNode>(assignNode);
    }

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
    }

    ASTNodePtr body = safe_any_cast<ASTNodePtr>(visitBlock(ctx->block()));

    auto funcNode = std::make_shared<FunctionDeclNode>(
        std::move(returnType),
        std::move(name),
        std::move(params),
        std::move(body));

    for (const auto &param : funcNode->parameters)
    {
    }

    functionTable[funcNode->name] = funcNode;

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

antlrcpp::Any ASTBuilder::visitPrintExpr(BaseParser::PrintExprContext *ctx)
{

    if (ctx->STRING())
    {
        std::string str = ctx->STRING()->getText();
        str = str.substr(1, str.size() - 2);

        return str;
    }
    else if (ctx->expr())
    {

        auto node = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));

        return node;
    }
    else if (ctx->printExpr(0) && ctx->printExpr(1))
    {

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

antlrcpp::Any ASTBuilder::visitGraphDef(BaseParser::GraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();

    if (!ctx->edges())
        throw std::runtime_error("graph must have edges (inline list or file):");

    // Materialize edges exactly once
    std::vector<std::pair<int, int>> edgesVec;
    if (auto *fe = ctx->edges()->fileEdgeList())
    {
        std::string s = fe->STRING()->getText();
        s = s.substr(1, s.size() - 2);
        FileEdgeList tmpFile(std::move(s));
        edgesVec = tmpFile.materializeEdges(); // file read happens here once
    }
    else if (auto *el = ctx->edges()->edgeList())
    {
        for (auto *eCtx : el->edge())
        {
            int u = std::stoi(eCtx->nodeID(0)->getText());
            int v = std::stoi(eCtx->nodeID(1)->getText());
            edgesVec.emplace_back(u, v);
        }
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
    auto ed = std::make_unique<InlineEdgeList>(std::move(edgesVec));

    auto gnode = std::make_shared<GraphDeclNode>(
        std::move(nm),
        std::move(nd),
        std::move(ed));

    return std::static_pointer_cast<ASTNode>(gnode);
}

antlrcpp::Any ASTBuilder::visitQueryStatement(BaseParser::QueryStatementContext *ctx)
{

    std::string name = ctx->ID()->getText();
    std::string desc = ctx->STRING()->getText();
    desc = desc.substr(1, desc.size() - 2); // strip quotes
    std::string gname = ctx->graphID()->getText();
    auto node = std::make_shared<QueryNode>(name, desc, gname);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitPrintStatement(BaseParser::PrintStatementContext *ctx)
{

    ASTNodePtr inner;
    if (ctx->printExpr())
    {
        inner = safe_any_cast<ASTNodePtr>(visitPrintExpr(ctx->printExpr()));
    }
    else
    {

        throw std::runtime_error("printgraph not yet supported in ASTBuilder");
    }

    return ASTNodePtr{std::make_shared<PrintStmtNode>(inner)};
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

    auto node = std::make_shared<SetDeclNode>(name, initNode);
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
