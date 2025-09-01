#include "ASTBuilder.h"
#include <iostream>
#include "ASTNode.h"
#include <stdexcept>
#include <vector>
#include <unordered_set>
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
        // Look up the function declaration
        auto it = functionTable.find(call->name);
        if (it == functionTable.end())
            throw std::runtime_error("Undefined function: " + call->name);
        auto decl = it->second;

        // Evaluate arguments in the current scope
        std::vector<int> argValues;
        for (const auto &arg : call->arguments)
        {
            argValues.push_back(evaluate(arg));
        }

        // Save current symbol table
        std::unordered_map<std::string, int> oldSymbols = symbolTable;
        symbolTable.clear();

        // Bind parameters to evaluated argument values
        for (size_t i = 0; i < decl->parameters.size(); ++i)
        {
            auto &p = decl->parameters[i];
            symbolTable[p->paramName] = argValues[i];
        }

        // Execute the function body
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

        // Restore outer symbol table
        symbolTable = std::move(oldSymbols);
        return ret;
    }

    throw std::runtime_error("wrongnode");
}

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
        // some statements return nullptr (e.g. varDecl returns nullptr after side‑effect),
        // so guard with has_value & type check if you like.
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
        // produce a GraphDeclNode
        return visitGraphDef(ctx->graphDef());
    }
    else if (ctx->queryStatement())
    {
        return visitQueryStatement(ctx->queryStatement());
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

        // Try safe, local constant-fold: only if index is an IntLiteral and array has static initializer
        if (arrayTable.find(name) != arrayTable.end())
        {
            if (auto *idxLit = dynamic_cast<IntLiteralNode *>(indexNode.get()))
            {
                int index = idxLit->value;
                auto &arr = arrayTable[name];
                if (index >= 0 && index < (int)arr.size())
                {
                    ASTNodePtr elem = arr[index];

                    // Case A: element is an IntLiteral -> return it directly
                    if (auto *elemLit = dynamic_cast<IntLiteralNode *>(elem.get()))
                    {
                        return ASTNodePtr(std::make_shared<IntLiteralNode>(elemLit->value));
                    }

                    // Case B: element is a BinaryExpr with literal children (simple fold inline)
                    if (auto *bin = dynamic_cast<BinaryExprNode *>(elem.get()))
                    {
                        // Only fold if both sides are integer literals
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
                                // if (Rv == 0)
                                // {
                                //     // avoid folding divide-by-zero at compile-time; fall through to runtime access
                                // }
                                // else
                                // {
                                return ASTNodePtr(std::make_shared<IntLiteralNode>(Lv / Rv));
                                // }
                            }
                        }
                    }
                }
            }
        }

        // Fallback: produce a runtime ArrayAccessNode (array variable + index expression)
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
            std::make_shared<VarDeclNode>(name, arrayLit, /*isArr=*/true, static_cast<size_t>(size)));
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

    functionTable[funcNode->name] = funcNode;

    // Debug: Verify parameters in functionTable
    // std::cerr << "[FunctionTable Entry] " << funcNode->name << " parameters: ";
    // for (const auto &param : functionTable[funcNode->name]->parameters)
    // {
    //     //std::cerr << param->paramName << " ";
    // }
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

antlrcpp::Any ASTBuilder::visitGraphDef(BaseParser::GraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();

    // nodesChild: optional. Build InlineNodeList only if nodes: present and non-empty.
    std::unique_ptr<NodeListNode> nd;
    if (ctx->nodes())
    {
        // if nodes: exists, might be empty or contain nodeIDs
        auto *nl = ctx->nodes()->nodeList();
        if (nl)
        {
            std::vector<int> ids;
            for (auto *idT : nl->nodeID())
                ids.push_back(std::stoi(idT->getText()));
            // We still create InlineNodeList even if ids is empty;
            // GraphDeclNode constructor will union with edges.
            nd = std::make_unique<InlineNodeList>(std::move(ids));
        }
        else
        {
            // nodes: exists but no nodeList (shouldn't happen with grammar), leave nd empty
            nd = nullptr;
        }
    }
    // else nd remains nullptr -> GraphDeclNode will infer nodes from edges

    // edgesChild: either fileEdgeList or inline edgeList
    std::unique_ptr<EdgeListNode> ed;
    if (ctx->edges())
    {
        if (auto *fe = ctx->edges()->fileEdgeList())
        {
            std::string s = fe->STRING()->getText();
            s = s.substr(1, s.size() - 2); // strip quotes
            ed = std::make_unique<FileEdgeList>(std::move(s));
        }
        else if (auto *el = ctx->edges()->edgeList())
        {
            std::vector<std::pair<int, int>> edgesVec;
            for (auto *eCtx : el->edge())
            {
                int u = std::stoi(eCtx->nodeID(0)->getText());
                int v = std::stoi(eCtx->nodeID(1)->getText());
                edgesVec.emplace_back(u, v);
            }
            ed = std::make_unique<InlineEdgeList>(std::move(edgesVec));
        }
    }

    if (!ed)
    {
        throw std::runtime_error("graph must have edges (inline list or file):");
    }

    // Construct GraphDeclNode — its constructor will merge explicit nodes (if any)
    // with edge endpoints and build CSR.
    auto gnode = std::make_shared<GraphDeclNode>(
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
    auto node = std::make_shared<QueryNode>(name, desc, gname);
    return std::static_pointer_cast<ASTNode>(node);
}

antlrcpp::Any ASTBuilder::visitPrintStatement(BaseParser::PrintStatementContext *ctx)
{
    // 1) build the ASTNode for whatever was inside the print
    ASTNodePtr inner;
    if (ctx->printExpr())
    {
        inner = safe_any_cast<ASTNodePtr>(visitPrintExpr(ctx->printExpr()));
    }
    else
    {
        // handle printgraph / edge/node cases if you want…
        throw std::runtime_error("printgraph not yet supported in ASTBuilder");
    }

    // 2) wrap in a PrintStmtNode
    return ASTNodePtr{std::make_shared<PrintStmtNode>(inner)};
}
