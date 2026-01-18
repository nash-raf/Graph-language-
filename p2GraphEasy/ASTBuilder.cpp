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
        // ctx->arrayAssignStatement() returns BaseParser::ArrayAssignStatementContext*
        // but our visitArrayAssignStmt expects BaseParser::ArrayAssignStmtContext* (the labeled alt).
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
        std::string gname  = gc->graphID()->getText();
        auto cond = buildGraphCondition(gc->graphCondition());
        auto node = std::make_shared<GraphComprehensionNode>(target, gname, cond);
        return std::static_pointer_cast<ASTNode>(node);
    }
    else if (ctx->showgraph())
    {
        std::string gname = ctx->showgraph()->graphID()->getText();
        auto node = std::make_shared<ShowGraphNode>(gname);
        return std::static_pointer_cast<ASTNode>(node);
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

    if (auto mulDivContext = dynamic_cast<BaseParser::MulDivExprContext *>(ctx))
    {
        auto lhs = safe_any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(0)));
        auto rhs = safe_any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(1)));
        std::string op = mulDivContext->TIMES() ? "*" : "/";
        // std::cout << "ASTBuilder MulDivExpr: " << op << "\n";
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
        if (auto sized = dynamic_cast<BaseParser::SizedArrayContext *>(declarator))
        {
            name = sized->ID()->getText();
            size = std::stoi(sized->INT()->getText());
        }
        else if (auto unsized = dynamic_cast<BaseParser::UnsizedArrayContext *>(declarator))
        {
            name = unsized->ID()->getText();
            // For now, we’ll treat unsized like sized=0 (must have initializer to infer size)
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

        // If no explicit size was given, infer it from the initializer list
        if (size == 0)
        {
            size = static_cast<int>(elems.size());
        }

        // Wrap the element list in an ArrayLiteralNode
        auto arrayLit = std::make_shared<ArrayLiteralNode>(elems);

        // Finally, return a VarDeclNode carrying that literal
        return std::static_pointer_cast<ASTNode>(
            std::make_shared<VarDeclNode>(typeName, name, arrayLit, /*isArr=*/true, static_cast<size_t>(size)));
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
    // Example body you already have:
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

// antlrcpp::Any ASTBuilder::visitGraphDef(BaseParser::GraphDefContext *ctx)
// {
//     std::string nm = ctx->graphID()->getText();

//     // nodesChild: optional. Build InlineNodeList only if nodes: present and non-empty.
//     std::unique_ptr<NodeListNode> nd;
//     if (ctx->nodes())
//     {
//         // if nodes: exists, might be empty or contain nodeIDs
//         auto *nl = ctx->nodes()->nodeList();
//         if (nl)
//         {
//             std::vector<int> ids;
//             for (auto *idT : nl->nodeID())
//                 ids.push_back(std::stoi(idT->getText()));
//             // We still create InlineNodeList even if ids is empty;
//             // GraphDeclNode constructor will union with edges.
//             nd = std::make_unique<InlineNodeList>(std::move(ids));
//         }
//         else
//         {
//             // nodes: exists but no nodeList (shouldn't happen with grammar), leave nd empty
//             nd = nullptr;
//         }
//     }
//     // else nd remains nullptr -> GraphDeclNode will infer nodes from edges

//     // edgesChild: either fileEdgeList or inline edgeList
//     std::unique_ptr<EdgeListNode> ed;
//     std::string filePath; // non-empty if edges came from a file
//     if (ctx->edges())
//     {
//         if (auto *fe = ctx->edges()->fileEdgeList())
//         {
//             std::string s = fe->STRING()->getText();
//             s = s.substr(1, s.size() - 2); // strip quotes
//             filePath = s;
//             ed = std::make_unique<FileEdgeList>(std::move(s));
//         }
//         else if (auto *el = ctx->edges()->edgeList())
//         {
//             std::vector<std::pair<int, int>> edgesVec;
//             for (auto *eCtx : el->edge())
//             {
//                 int u = std::stoi(eCtx->nodeID(0)->getText());
//                 int v = std::stoi(eCtx->nodeID(1)->getText());
//                 edgesVec.emplace_back(u, v);
//             }
//             ed = std::make_unique<InlineEdgeList>(std::move(edgesVec));
//         }
//     }

//     if (!ed)
//     {
//         throw std::runtime_error("graph must have edges (inline list or file):");
//     }

//     // Construct GraphDeclNode — its constructor will (for file-backed graphs)
//     // materialize (read) the edge list. We measure that operation when filePath is set.
//     std::shared_ptr<GraphDeclNode> gnode;
//     if (!filePath.empty())
//     {
//         auto t0 = std::chrono::high_resolution_clock::now();
//         gnode = std::make_shared<GraphDeclNode>(
//             std::move(nm),
//             std::move(nd),
//             std::move(ed));
//         auto t1 = std::chrono::high_resolution_clock::now();
//         auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
//         std::cerr << "[ASTBuilder] loaded edge-list file '" << filePath
//                   << "' in " << dur.count() << " ms\n";
//     }
//     else
//     {
//         // inline edges: just construct without timing the file read
//         gnode = std::make_shared<GraphDeclNode>(
//             std::move(nm),
//             std::move(nd),
//             std::move(ed));
//     }

//     return std::static_pointer_cast<ASTNode>(gnode);
// }

antlrcpp::Any ASTBuilder::visitUnweightedGraphDef(BaseParser::UnweightedGraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();
    std::cerr << "[ASTBuilder] Declaring graph: " << nm << std::endl;

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

antlrcpp::Any ASTBuilder::visitWeightedGraphDef(BaseParser::WeightedGraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();
    std::cerr << "[ASTBuilder] Declaring Weighted graph: " << nm << std::endl;

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

// antlrcpp::Any ASTBuilder::visitForeachStatement(BaseParser::ForeachStatementContext *ctx) {
//     std::string var1, var2;
//     std::string graphName = ctx->graphID()->getText();

//     // Get the loop target context
//     auto *loopCtx = ctx->loopTarget();

//     // Lookup graph from your stored graphs
//     auto it = declaredGraphs.find(graphName);
//     if (it == declaredGraphs.end()) {
//         throw std::runtime_error("Graph " + graphName + " not declared");
//     }
//     auto gnode = it->second; // GraphDeclNode*

//     // ====== foreach vertex v in g ======
//     if (auto vertexCtx = dynamic_cast<BaseParser::ForEachVertexContext*>(loopCtx)) {
//         var1 = vertexCtx->ID()->getText();

//         for (int node : gnode->nodes->materializeNodeIds()) {
//             runtimeVariables[var1] = node;
//             visit(ctx->block());  // execute loop body
//         }
//     }

//     // ====== foreach edge (u,v) in g ======
//     else if (auto edgeCtx = dynamic_cast<BaseParser::ForEachEdgeContext*>(loopCtx)) {
//         auto ids = edgeCtx->ID();
//         var1 = ids[0]->getText();
//         var2 = ids[1]->getText();

//         for (auto &e : gnode->edges->materializeEdges()) {
//             runtimeVariables[var1] = e.first;
//             runtimeVariables[var2] = e.second;
//             visit(ctx->block());
//         }
//     }

//     // ====== foreach neighbor n of x in g ======
//     else if (auto adjCtx = dynamic_cast<BaseParser::ForEachAdjContext*>(loopCtx)) {
//         var1 = adjCtx->ID()->getText();
//         int nodeId = std::stoi(adjCtx->nodeID()->getText());

//         // Build adjacency map
//         std::unordered_map<int, std::vector<int>> adj;
//         for (auto &e : gnode->edges->materializeEdges()) {
//             adj[e.first].push_back(e.second);
//             adj[e.second].push_back(e.first); // assuming undirected graph
//         }

//         for (int neighbor : adj[nodeId]) {
//             runtimeVariables[var1] = neighbor;
//             visit(ctx->block());
//         }
//     }

//     else {
//         throw std::runtime_error("Unknown loop target in foreachStatement");
//     }

//     return nullptr;
// }

antlrcpp::Any ASTBuilder::visitForeachStatement(BaseParser::ForeachStatementContext *ctx)
{
    ForEachTargetType tgt;
    std::string var1, var2;
    int adjNodeId = -1;

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
        adjNodeId = std::stoi(adjCtx->nodeID()->getText());
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
        adjNodeId,
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
    else if (auto *parenCtx = dynamic_cast<BaseParser::ParenGraphConditionContext*>(ctx))
    {
        return buildGraphCondition(parenCtx->graphCondition());
    }
    else
    {
        throw std::runtime_error("buildGraphCondition: unsupported condition type");
    }
}
