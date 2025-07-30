#include "ASTBuilder.h"
#include <iostream>
#include "ASTNode.h"
#include <stdexcept>
#include <vector>

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
    std::cerr << " visiting statement " << "\n";
    if (ctx->printStatement())
    {
        // std::cout<<"before entering printxpr\n";
        return visitPrintStatement(ctx->printStatement());
    }
    else if (ctx->varDecl())
    {
        std::cerr << "before visting varDeclr " << "\n";
        return visitVarDecl(ctx->varDecl());
    }
    else if (ctx->assignmentStatement())
    {
        std::cerr << " befoe entering assignment " << "\n";
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

    else if (ctx->functionCall())
    {
        return visitFunctionCall(ctx->functionCall());
    }
    else if (ctx->graphDef())
    {
        // produce a GraphDeclNode
        return visitGraphDef(ctx->graphDef());
    }

    std::cerr << " ending statement " << "\n";
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
        int index = evaluate(indexNode);

        if (arrayTable.find(name) == arrayTable.end())
            throw std::runtime_error("Accessing undefined array: " + name);

        auto &arr = arrayTable[name];
        if (index < 0 || index >= arr.size())
            throw std::runtime_error("Array index out of bounds");

        return ASTNodePtr(std::make_shared<IntLiteralNode>(evaluate(arr[index])));
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
        auto declNode = std::make_shared<VarDeclNode>(name, init);
        // std::cout << "[VarDecl] " << name << " = " << value << "\n";
        return std::static_pointer_cast<ASTNode>(declNode);
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
            std::make_shared<VarDeclNode>(name, arrayLit));
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx)
{
    std::string name = ctx->ID()->getText();
    ASTNodePtr exprNode = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
    auto assignNode = std::make_shared<AssignmentStmtNode>(name, exprNode);
    return std::static_pointer_cast<ASTNode>(assignNode);
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
        std::cerr << "[Param] " << paramType << " " << paramName << "\n";
    }
    std::cerr << "[FunctionDecl] " << returnType << " " << name << "(";
    for (const auto &param : params)
    {
        std::cerr << param->typeName << " " << param->paramName;
        if (&param != &params.back())
            std::cerr << ", ";
    }
    std::cerr << ")\n";

    ASTNodePtr body = safe_any_cast<ASTNodePtr>(visitBlock(ctx->block()));

    auto funcNode = std::make_shared<FunctionDeclNode>(
        std::move(returnType),
        std::move(name),
        std::move(params),
        std::move(body));

    // Debug: Verify parameters in funcNode
    std::cerr << "[FunctionNode Parameters] ";
    for (const auto &param : funcNode->parameters)
    {
        std::cerr << param->paramName << " ";
    }
    std::cerr << "\n";

    functionTable[funcNode->name] = funcNode;

    // Debug: Verify parameters in functionTable
    std::cerr << "[FunctionTable Entry] " << funcNode->name << " parameters: ";
    for (const auto &param : functionTable[funcNode->name]->parameters)
    {
        std::cerr << param->paramName << " ";
    }
    std::cerr << "\n";

    return std::static_pointer_cast<ASTNode>(funcNode);
}

antlrcpp::Any ASTBuilder::visitArrayAssignStmt(BaseParser::ArrayAssignStmtContext *ctx)
{
    std::string arrayName = ctx->ID()->getText();
    if (symbolTable.find(arrayName) == symbolTable.end())
    {
        throw std::runtime_error("Assign to undefined array: " + arrayName);
    }

    // Evaluate the index expression
    ASTNodePtr indexNode = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(0)));
    int index = evaluate(indexNode);

    // Evaluate the value to assign
    ASTNodePtr valueNode = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr(1)));
    int value = evaluate(valueNode);

    // Update the symbol table with the new value at the specified index
    symbolTable[arrayName + "[" + std::to_string(index) + "]"] = value;
    std::cout << "[ArrayAssign] " << arrayName << "[" << index << "] = " << value << "\n";

    return nullptr;
}

antlrcpp::Any ASTBuilder::visitPrintStatement(BaseParser::PrintStatementContext *ctx)
{
    auto result = visitPrintExpr(ctx->printExpr());
    if (!result.has_value())
        return nullptr;

    // integer
    if (result.type() == typeid(int))
    {
        std::cout << safe_any_cast<int>(result) << "\n";
    }
    // string literal
    else if (result.type() == typeid(std::string))
    {
        std::cout << safe_any_cast<std::string>(result) << "\n";
    }
    // ASTNodePtr: variable or expression node
    else if (result.type() == typeid(ASTNodePtr))
    {
        ASTNodePtr node = safe_any_cast<ASTNodePtr>(result);
        // variable lookup
        if (auto var = dynamic_cast<VariableNode *>(node.get()))
        {
            auto it = symbolTable.find(var->name);
            if (it == symbolTable.end())
                throw std::runtime_error("Undefined var: " + var->name);
            std::cout << it->second << "\n";
        }
        else
        {
            // evaluate any other expr node
            std::cout << evaluate(node) << "\n";
        }
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitPrintExpr(BaseParser::PrintExprContext *ctx)
{
    std::cout << "Entering PrintExpr\n";

    if (ctx->STRING())
    {
        std::string str = ctx->STRING()->getText();
        str = str.substr(1, str.size() - 2);
        std::cout << "ASTBuilder STRING: " << str << "\n";
        return str;
    }
    else if (ctx->expr())
    {
        std::cout << "ASTBuilder Visiting inner expr\n";
        auto node = safe_any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
        std::cout << "ASTBuilder exitinng inner expr\n";
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

        std::cerr << "[FunctionCall] " << callee << "(";
        for (size_t i = 0; i < args.size(); ++i)
        {
            ASTNodePtr arg = args[i];
            if (auto lit = dynamic_cast<IntLiteralNode *>(arg.get()))
                std::cerr << lit->value;
            else if (auto var = dynamic_cast<VariableNode *>(arg.get()))
                std::cerr << var->name;
            else if (auto bin = dynamic_cast<BinaryExprNode *>(arg.get()))
                std::cerr << bin->op << " (" << evaluate(bin->lhs)
                          << ", " << evaluate(bin->rhs) << ")";
            else if (auto fc = dynamic_cast<FunctionCallNode *>(arg.get()))
                std::cerr << fc->name << "()";
            else
                std::cerr << "UnknownArg";

            if (i + 1 < args.size())
                std::cerr << ", ";
        }
        std::cerr << ")\n";
    }

    return ASTNodePtr{
        std::make_shared<FunctionCallNode>(callee, std::move(args))};
}

antlrcpp::Any ASTBuilder::visitGraphDef(BaseParser::GraphDefContext *ctx)
{
    std::string nm = ctx->graphID()->getText();

    // nodesChild
    std::unique_ptr<NodeListNode> nd;
    if (ctx->nodes())
    {
        std::vector<int> ids;
        for (auto *idT : ctx->nodes()->nodeList()->nodeID())
            ids.push_back(std::stoi(idT->getText()));
        nd = std::make_unique<InlineNodeList>(std::move(ids));
    }
    else
    {
        throw std::runtime_error("graph must have nodes:");
    }

    // edgesChild
    std::unique_ptr<EdgeListNode> ed;
    auto fe = ctx->edges()->fileEdgeList();
    if (fe)
    {
        std::string s = fe->STRING()->getText();
        s = s.substr(1, s.size() - 2);
        ed = std::make_unique<FileEdgeList>(std::move(s));
    }
    else
    {
        throw std::runtime_error("graph must have edges:");
    }

    auto gnode = std::make_shared<GraphDeclNode>(
        std::move(nm),
        std::move(nd),
        std::move(ed));

    std::cerr << "[visitGraphDef] name=" << nm
              << " nodes=" << ctx->nodes()->getText()
              << " edges=" << ctx->edges()->getText() << "\n";

    return std::static_pointer_cast<ASTNode>(gnode);
}