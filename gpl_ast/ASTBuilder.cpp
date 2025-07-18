#include "ASTBuilder.h"
#include <iostream>
#include "ASTNode.h"
#include <stdexcept>


int ASTBuilder::evaluate(ASTNodePtr node) {
    if (auto lit = dynamic_cast<IntLiteralNode*>(node.get())) {
        return lit->value;
    }
    if (auto var = dynamic_cast<VariableNode*>(node.get())) {
        auto it = symbolTable.find(var->name);
        if (it == symbolTable.end())
            throw std::runtime_error("Undefined variable: " + var->name);
        return it->second;
    }
    if (auto bin = dynamic_cast<BinaryExprNode*>(node.get())) {
        int L = evaluate(bin->lhs);
        int R = evaluate(bin->rhs);
        if (bin->op == "+") return L + R;
        if (bin->op == "-") return L - R;
        if (bin->op == "*") return L * R;
        if (bin->op == "/") return R != 0 ? L / R : 0;
    }
    throw std::runtime_error("wrongnode");
}



antlrcpp::Any ASTBuilder::visitProgram(BaseParser::ProgramContext* ctx) {
    std::cout << "Visited program" << std::endl;
    for(auto statement: ctx->statement()){
        visitStatement(statement);
    }
    std::cout << "Visited program" << std::endl;


    return nullptr; 
}

antlrcpp::Any ASTBuilder::visitStatement(BaseParser::StatementContext* ctx){
    std::cout << " visiting statement " << "\n";
    if(ctx->printStatement()){
        // std::cout<<"before entering printxpr\n";
        return visitPrintStatement(ctx->printStatement());
    }
     else if (ctx->varDecl())
    {
        std::cout << "before visting varDeclr " << "\n";
        return visitVarDecl(ctx->varDecl());
    }
    else if (ctx->assignmentStatement())
    {
        std::cout <<" befoe entering assignment " <<"\n";
        return visitAssignmentStatement(ctx->assignmentStatement());
    }
    else if (ctx->conditionalStatement()){
        return visitConditionalStatement(ctx->conditionalStatement());
    }
    // else if(ctx->loopStatement()){
    //     return visitLoopStatement(ctx->loopStatement());
    // }
    else if(ctx->whileStatement()){
        return visitWhileStatement(ctx->whileStatement());
    }
                           

    std::cout << " ending statement " << "\n";
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitBlock(BaseParser::BlockContext* ctx) {
  
    for (auto stmt : ctx->statement()) {
        visitStatement(stmt);
    }
    return nullptr;
}


antlrcpp::Any ASTBuilder::visitConditionalStatement(BaseParser::ConditionalStatementContext* ctx) {
    bool cond = std::any_cast<bool>(visitCondition(ctx->condition()));

    if (cond) {
        return visitBlock(ctx->block(0));
    }
    // else if (...)
    if (ctx->conditionalStatement()) {
        return visitConditionalStatement(ctx->conditionalStatement());
    }
    // else { … }
    if (ctx->block(1)) {
        return visitBlock(ctx->block(1));
    }
    return nullptr;
}

// Recursive eval of boolean conditions
antlrcpp::Any ASTBuilder::visitCondition(BaseParser::ConditionContext* ctx) {
    using Ctx = BaseParser;
    if (auto a = dynamic_cast<Ctx::LogicalAndContext*>(ctx)) {
        bool L = std::any_cast<bool>(visitCondition(a->condition(0)));
        bool R = std::any_cast<bool>(visitCondition(a->condition(1)));
        return L && R;
    }
    if (auto o = dynamic_cast<Ctx::LogicalOrContext*>(ctx)) {
        bool L = std::any_cast<bool>(visitCondition(o->condition(0)));
        bool R = std::any_cast<bool>(visitCondition(o->condition(1)));
        return L || R;
    }
    if (auto r = dynamic_cast<Ctx::RelationalContext*>(ctx)) {
        ASTNodePtr Lnode = std::any_cast<ASTNodePtr>(visitExpr(r->expr(0)));
        ASTNodePtr Rnode = std::any_cast<ASTNodePtr>(visitExpr(r->expr(1)));
        int L = evaluate(Lnode);
        int R = evaluate(Rnode);
        std::string op = r->EQUAL()      ? "==" :
                         r->NOTEQUAL()   ? "!=" :
                         r->LESSEQUAL()  ? "<=" :
                         r->GREATEREQUAL()? ">=" :
                         r->LESSTHAN()   ? "<"  :
                         r->GREATERTHAN()? ">"  : "";
        if (op == "==") return L == R;
        if (op == "!=") return L != R;
        if (op == "<")  return L <  R;
        if (op == ">")  return L >  R;
        if (op == "<=") return L <= R;
        if (op == ">=") return L >= R;
    }
    throw std::runtime_error("Unsupported condition: " + ctx->getText());
}

// antlrcpp::Any ASTBuilder::visitLoopStatement(BaseParser::LoopStatementContext* ctx) {
//     if (ctx->whileStatement()) {
//         return visitWhileStatement(ctx->whileStatement());
//     }
    
//     return nullptr;
// }


antlrcpp::Any ASTBuilder::visitWhileStatement(BaseParser::WhileStatementContext* ctx) {
    while ( true ) {
        bool cond = std::any_cast<bool>( visitCondition(ctx->condition()) );
        if (!cond) break;
        visitBlock(ctx->block());
    }
    return nullptr;
}




antlrcpp::Any ASTBuilder::visitExpr(BaseParser::ExprContext* ctx){
    // std::cout << " expr text: " << ctx->getText() << "\n";   
    // std::cout << " Runtime type: " << typeid(*ctx).name() << "\n";


    if(auto mulDivContext = dynamic_cast<BaseParser::MulDivExprContext *>(ctx)){
        auto lhs = std::any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(0)));
        auto rhs = std::any_cast<ASTNodePtr>(visitExpr(mulDivContext->expr(1)));
        std::string op = mulDivContext->TIMES() ? "*" : "/";
        // std::cout << "ASTBuilder MulDivExpr: " << op << "\n";
        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, lhs, rhs));
    }
    else if(auto addSubContext = dynamic_cast<BaseParser::AddSubExprContext *>(ctx)){
        // std::cout<<"we have tone till here\n";

        auto lhs = std::any_cast<ASTNodePtr>(visitExpr(addSubContext->expr(0)));
       // std::cout<<"we have tone till here\n";
        auto rhs = std::any_cast<ASTNodePtr>(visitExpr(addSubContext->expr(1)));
        std::string op = addSubContext->PLUS() ? "+" : "-";
        // std::cout << "ASTBuilder AddSubExpr: " <<lhs<< op <<rhs << "\n";
        return ASTNodePtr(std::make_shared<BinaryExprNode>(op, lhs, rhs));
    }
    
    else if(auto intExprContext = dynamic_cast<BaseParser::IntExprContext *>(ctx)){
        int val = std::stoi(intExprContext->getText());
        // std::cout << "astbuilder intExpr : " << val << "\n";
        return ASTNodePtr(std::make_shared<IntLiteralNode>(val));
    }
    else if( auto idExprContext = dynamic_cast <BaseParser::IdExprContext *>(ctx)){
        std::string name = idExprContext->getText();
        
        // std::cout <<"astbuilder idExpr : " <<name<<"\n";
        return ASTNodePtr(std::make_shared<VariableNode>(name));
    }
    else if(auto parenContext = dynamic_cast <BaseParser::ParenExprContext*>(ctx)){
        // std::cout <<"astbuilder parenExpr : "<<"\n";

        return visitExpr(parenContext->expr());
    }
    throw std::runtime_error("ASTBuilder Unsupported expr: " + ctx->getText());

    return nullptr;
}




antlrcpp::Any ASTBuilder::visitVarDecl(BaseParser::VarDeclContext* ctx) {
    // only SimpleDeclaration (#SimpleDeclaration)
    if (auto sd = dynamic_cast<BaseParser::SimpleDeclarationContext*>(ctx)) {
        std::string name = sd->ID()->getText();
        int value = 0;
        if (sd->expr()) {
            ASTNodePtr exprNode = std::any_cast<ASTNodePtr>(visitExpr(sd->expr()));
            value = evaluate(exprNode);
        }
        symbolTable[name] = value;
        std::cout << "[VarDecl] " << name << " = " << value << "\n";
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitAssignmentStatement(BaseParser::AssignmentStatementContext* ctx) {
    std::string name = ctx->ID()->getText();

    if (symbolTable.find(name) == symbolTable.end()) {
        throw std::runtime_error("Assign to undefined variable: " + name);
    }

    // Build and evaluate the RHS expression
    ASTNodePtr exprNode = std::any_cast<ASTNodePtr>(visitExpr(ctx->expr()));
    int value = evaluate(exprNode);

    // Update symbol table
    symbolTable[name] = value;
    std::cout << "[Assign] " << name << " = " << value << "\n";

    return nullptr;
}











antlrcpp::Any ASTBuilder::visitPrintStatement(BaseParser::PrintStatementContext *ctx) {
    auto result = visitPrintExpr(ctx->printExpr());
    if (!result.has_value()) return nullptr;

    // integer
    if (result.type() == typeid(int)) {
        std::cout << std::any_cast<int>(result) << "\n";
    }
    // string literal
    else if (result.type() == typeid(std::string)) {
        std::cout << std::any_cast<std::string>(result) << "\n";
    }
    // ASTNodePtr: variable or expression node
    else if (result.type() == typeid(ASTNodePtr)) {
        ASTNodePtr node = std::any_cast<ASTNodePtr>(result);
        // variable lookup
        if (auto var = dynamic_cast<VariableNode*>(node.get())) {
            auto it = symbolTable.find(var->name);
            if (it == symbolTable.end()) throw std::runtime_error("Undefined var: " + var->name);
            std::cout << it->second << "\n";
        }
        else {
            // evaluate any other expr node
            std::cout << evaluate(node) << "\n";
        }
    }
    return nullptr;
}

antlrcpp::Any ASTBuilder::visitPrintExpr(BaseParser::PrintExprContext *ctx) {
    std::cout<< "Entering PrintExpr\n";

    if(ctx->STRING()){
        std::string str = ctx->STRING()->getText();
        str = str.substr(1, str.size() -2 );
        std::cout << "ASTBuilder STRING: " << str << "\n";
        return str;
    }
    else if(ctx->expr()){
        std::cout << "ASTBuilder Visiting inner expr\n";
        auto node = std::any_cast<ASTNodePtr>(visitExpr(ctx->expr()));  
        std::cout << "ASTBuilder exitinng inner expr\n";      
        return node;
    }
    else if(ctx->printExpr(0) && ctx->printExpr(1)){
        // std::cout << "ASTBuilder Binary PrintExpr +\n";
        auto left = visitPrintExpr(ctx->printExpr(0));
        auto right = visitPrintExpr(ctx->printExpr(1));

        std::string leftStr, rightStr;

        if (left.type() == typeid(std::string)) {
            leftStr = std::any_cast<std::string>(left);
        } else if (left.type() == typeid(ASTNodePtr)) {
            auto node = std::any_cast<ASTNodePtr>(left);
            if (auto lit = dynamic_cast<IntLiteralNode*>(node.get())) {
                leftStr = std::to_string(lit->value);
            } else if (auto var = dynamic_cast<VariableNode*>(node.get())) {
                leftStr = var->name;
            } else {
                leftStr = "UnknownExpr";
            }
        }

        if (right.type() == typeid(std::string)) {
            rightStr = std::any_cast<std::string>(right);
        } else if (right.type() == typeid(ASTNodePtr)) {
            auto node = std::any_cast<ASTNodePtr>(right);
            if (auto lit = dynamic_cast<IntLiteralNode*>(node.get())) {
                rightStr = std::to_string(lit->value);
            } else if (auto var = dynamic_cast<VariableNode*>(node.get())) {
                rightStr = var->name;
            } else {
                rightStr = "[UnknownExpr]";
            }
        }

        return leftStr + rightStr;
    }

    return nullptr;
}

