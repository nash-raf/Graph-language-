#include "SemanticAnalyzer.h"
#include <algorithm>
#include <stdexcept>

SemanticAnalyzer::SemanticAnalyzer(ProgramNodePtr rootNode)
    : root(std::move(rootNode))
{
}

void SemanticAnalyzer::analyze()
{
    analyzeProgram(root);
}

void SemanticAnalyzer::enterScope()
{
    scopes.emplace_back();
}

void SemanticAnalyzer::exitScope()
{
    if (!scopes.empty())
        scopes.pop_back();
}

void SemanticAnalyzer::declareSymbol(const std::string &name, const Symbol &sym)
{
    if (scopes.empty())
        enterScope();

    auto &current = scopes.back();
    if (current.find(name) != current.end())
    {
        error("redeclaration of symbol: " + name);
    }
    current[name] = sym;
}

Symbol *SemanticAnalyzer::lookupSymbol(const std::string &name)
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        auto found = it->find(name);
        if (found != it->end())
            return &found->second;
    }
    return nullptr;
}

TypeKind SemanticAnalyzer::typeFromString(const std::string &typeName) const
{
    if (typeName == "int")
        return TypeKind::Int;
    if (typeName == "void")
        return TypeKind::Void;
    if (typeName == "string")
        return TypeKind::String;
    if (typeName == "graph")
        return TypeKind::Graph;
    if (typeName == "vertex" || typeName == "edge")
        return TypeKind::Int;
    if (typeName == "bool")
        return TypeKind::Bool;
    if (typeName == "real")
        return TypeKind::Real;
    return TypeKind::Unknown;
}

TypeKind SemanticAnalyzer::analyzeExpr(ASTNode *expr)
{
    switch (expr->type)
    {
    case ASTNodeType::IntLiteral:
        return TypeKind::Int;
    case ASTNodeType::BoolLiteral:
        return TypeKind::Bool;
    case ASTNodeType::RealLiteral:
        return TypeKind::Real;
    case ASTNodeType::StringLiteral:
        return TypeKind::String;
    case ASTNodeType::Variable:
    {
        auto *var = static_cast<VariableNode *>(expr);
        Symbol *sym = lookupSymbol(var->name);
        if (!sym)
            error("use of undeclared variable: " + var->name);
        if (sym->isFunction)
            error("function used as variable: " + var->name);
        return sym->type;
    }
    case ASTNodeType::ArrayLiteral:
    {
        auto *arr = static_cast<ArrayLiteralNode *>(expr);
        for (auto &elem : arr->elements)
        {
            TypeKind t = analyzeExpr(elem.get());
            if (t != TypeKind::Int)
                error("array literal elements must be int");
        }
        return TypeKind::IntArray;
    }
    case ASTNodeType::ArrayAccess:
    {
        auto *acc = static_cast<ArrayAccessNode *>(expr);
        auto *baseVar = dynamic_cast<VariableNode *>(acc->arrayExpr.get());
        if (!baseVar)
            error("array access base must be a variable");
        Symbol *sym = lookupSymbol(baseVar->name);
        if (!sym)
            error("use of undeclared array: " + baseVar->name);
        if (sym->type != TypeKind::IntArray)
            error("subscripted value is not an array: " + baseVar->name);
        TypeKind idxTy = analyzeExpr(acc->indexExpr.get());
        if (idxTy != TypeKind::Int)
            error("array index must be int");
        return TypeKind::Int;
    }
    case ASTNodeType::BinaryExpr:
    {
        auto *bin = static_cast<BinaryExprNode *>(expr);
        TypeKind lt = analyzeExpr(bin->lhs.get());
        TypeKind rt = analyzeExpr(bin->rhs.get());
        const std::string &op = bin->op;
        if (op == "&&" || op == "||")
        {
            if (lt != TypeKind::Bool || rt != TypeKind::Bool)
                error("logical operator requires bool operands: " + op);
            return TypeKind::Bool;
        }
        if (op == "==" || op == "!=")
        {
            if (lt == rt && (lt == TypeKind::Int || lt == TypeKind::Real || lt == TypeKind::Bool))
                return TypeKind::Bool;
            if ((lt == TypeKind::Int || lt == TypeKind::Real) &&
                (rt == TypeKind::Int || rt == TypeKind::Real))
                return TypeKind::Bool;
            error("equality requires compatible operands: " + op);
        }
        if (op == "<" || op == "<=" || op == ">" || op == ">=")
        {
            if ((lt == TypeKind::Int || lt == TypeKind::Real) &&
                (rt == TypeKind::Int || rt == TypeKind::Real))
                return TypeKind::Bool;
            error("ordering comparison requires numeric operands: " + op);
        }
        if (op == "%")
        {
            if (lt != TypeKind::Int || rt != TypeKind::Int)
                error("modulo requires int operands");
            return TypeKind::Int;
        }
        if ((lt == TypeKind::Int || lt == TypeKind::Real) &&
            (rt == TypeKind::Int || rt == TypeKind::Real))
        {
            if (lt == TypeKind::Real || rt == TypeKind::Real)
                return TypeKind::Real;
            return TypeKind::Int;
        }
        error("binary operator requires numeric operands: " + op);
    }
    case ASTNodeType::FunctionCall:
    {
        auto *call = static_cast<FunctionCallNode *>(expr);
        Symbol *sym = lookupSymbol(call->name);
        if (!sym || !sym->isFunction)
            error("call to undeclared function: " + call->name);
        if (call->arguments.size() != sym->func.paramTypes.size())
            error("argument count mismatch for function: " + call->name);
        for (size_t i = 0; i < call->arguments.size(); ++i)
        {
            TypeKind argTy = analyzeExpr(call->arguments[i].get());
            if (sym->func.paramTypes[i] != TypeKind::Unknown &&
                argTy != sym->func.paramTypes[i])
                error("argument type mismatch for function: " + call->name);
        }
        return sym->func.returnType;
    }
    case ASTNodeType::QueryNode:
    {
        auto *q = static_cast<QueryNode *>(expr);
        if (q->type == QueryType::INT)
            return TypeKind::Int;
        if (q->type == QueryType::ARRAY)
            return TypeKind::IntArray;
        return TypeKind::Unknown;
    }
    default:
        return TypeKind::Unknown;
    }
}

void SemanticAnalyzer::analyzeProgram(ProgramNodePtr prog)
{
    enterScope();

    // Predeclare all function signatures for forward references
    for (auto &node : prog->topLevel)
    {
        if (node->type == ASTNodeType::FunctionDecl)
        {
            auto *FD = static_cast<FunctionDeclNode *>(node.get());
            Symbol fnSym;
            fnSym.isFunction = true;
            fnSym.type = TypeKind::Unknown;
            fnSym.func.returnType = typeFromString(FD->returnType);
            for (auto &p : FD->parameters)
            {
                fnSym.func.paramTypes.push_back(typeFromString(p->typeName));
            }
            declareSymbol(FD->name, fnSym);
        }
    }

    for (auto &node : prog->topLevel)
    {
        analyzeStatement(node.get());
    }

    exitScope();
}

void SemanticAnalyzer::analyzeStatement(ASTNode *node)
{
    switch (node->type)
    {
    case ASTNodeType::VarDecl:
        analyzeVarDecl(static_cast<VarDeclNode *>(node));
        break;
    case ASTNodeType::AssignmentStmt:
        analyzeAssignment(static_cast<AssignmentStmtNode *>(node));
        break;
    case ASTNodeType::ReturnStmt:
        analyzeReturnStmt(static_cast<ReturnStmtNode *>(node));
        break;
    case ASTNodeType::Conditional:
        analyzeConditional(static_cast<ConditionalNode *>(node));
        break;
    case ASTNodeType::WhileStmt:
        analyzeWhile(static_cast<WhileStmtNode *>(node));
        break;
    case ASTNodeType::ForEachStmt:
        analyzeForEach(static_cast<ForEachStmtNode *>(node));
        break;
    case ASTNodeType::FunctionDecl:
        analyzeFunctionDecl(static_cast<FunctionDeclNode *>(node));
        break;
    case ASTNodeType::GraphDecl:
        analyzeGraphDecl(static_cast<GraphDeclNode *>(node));
        break;
    case ASTNodeType::WeightedGraphDecl:
        analyzeWeightedGraphDecl(static_cast<WeightedGraphDeclNode *>(node));
        break;
    case ASTNodeType::QueryNode:
        analyzeQuery(static_cast<QueryNode *>(node));
        break;
    case ASTNodeType::PrintStmt:
        analyzePrintStmt(static_cast<PrintStmtNode *>(node));
        break;
    case ASTNodeType::GraphUpdate:
        analyzeGraphUpdate(static_cast<GraphUpdateNode *>(node));
        break;
    case ASTNodeType::ShowGraph:
        analyzeShowGraph(static_cast<ShowGraphNode *>(node));
        break;
    case ASTNodeType::GraphComprehension:
        analyzeGraphComprehension(static_cast<GraphComprehensionNode *>(node));
        break;
    case ASTNodeType::FunctionCall:
        (void)analyzeExpr(node);
        break;
    default:
        break;
    }
}

void SemanticAnalyzer::analyzeBlock(BlockStmtNode *block)
{
    enterScope();
    for (auto &stmt : block->statements)
    {
        analyzeStatement(stmt.get());
    }
    exitScope();
}

void SemanticAnalyzer::analyzeVarDecl(VarDeclNode *decl)
{
    Symbol sym;
    sym.isFunction = false;
    TypeKind declared = typeFromString(decl->typeName);
    if (decl->isArray)
    {
        if (declared != TypeKind::Int)
            error("only int arrays are supported: " + decl->name);
        sym.type = TypeKind::IntArray;
    }
    else
    {
        sym.type = declared;
        if (sym.type == TypeKind::Unknown)
            error("unknown type in declaration: " + decl->typeName);
    }
    declareSymbol(decl->name, sym);

    if (decl->initializer)
    {
        TypeKind initTy = analyzeExpr(decl->initializer.get());
        if (sym.type == TypeKind::Int && initTy != TypeKind::Int)
            error("type mismatch in variable initialization: " + decl->name);
        if (sym.type == TypeKind::Bool && initTy != TypeKind::Bool)
            error("type mismatch in variable initialization: " + decl->name);
        if (sym.type == TypeKind::Real && initTy != TypeKind::Real)
            error("type mismatch in variable initialization: " + decl->name);
        if (sym.type == TypeKind::IntArray &&
            initTy != TypeKind::IntArray &&
            initTy != TypeKind::Unknown)
            error("type mismatch in array initialization: " + decl->name);
    }
}

void SemanticAnalyzer::analyzeAssignment(AssignmentStmtNode *assign)
{
    TypeKind rhsTy = analyzeExpr(assign->rhs.get());

    if (assign->lhs->type == ASTNodeType::Variable)
    {
        auto *var = static_cast<VariableNode *>(assign->lhs.get());
        Symbol *sym = lookupSymbol(var->name);
        if (!sym)
            error("assignment to undeclared variable: " + var->name);
        if (sym->type != rhsTy)
            error("type mismatch in assignment to " + var->name);
        return;
    }

    if (assign->lhs->type == ASTNodeType::ArrayAccess)
    {
        TypeKind lhsTy = analyzeExpr(assign->lhs.get());
        if (lhsTy != TypeKind::Int)
            error("array element assignment requires int value");
        if (rhsTy != TypeKind::Int)
            error("type mismatch in array element assignment");
        return;
    }

    error("unsupported assignment target");
}

void SemanticAnalyzer::analyzeConditional(ConditionalNode *ifs)
{
    TypeKind condTy = analyzeExpr(ifs->condition.get());
    if (condTy != TypeKind::Bool)
        error("if condition must be bool");
    analyzeBlock(static_cast<BlockStmtNode *>(ifs->thenBlock.get()));
    if (ifs->elseBlock)
        analyzeBlock(static_cast<BlockStmtNode *>(ifs->elseBlock.get()));
}

void SemanticAnalyzer::analyzeWhile(WhileStmtNode *ws)
{
    TypeKind condTy = analyzeExpr(ws->condition.get());
    if (condTy != TypeKind::Bool)
        error("while condition must be bool");
    analyzeBlock(static_cast<BlockStmtNode *>(ws->body.get()));
}

void SemanticAnalyzer::analyzeForEach(ForEachStmtNode *fs)
{
    Symbol *gSym = lookupSymbol(fs->graphName);
    if (!gSym || (gSym->type != TypeKind::Graph && gSym->type != TypeKind::WeightedGraph))
        error("foreach over undeclared graph: " + fs->graphName);

    enterScope();
    Symbol varSym;
    varSym.isFunction = false;
    varSym.type = TypeKind::Int;

    if (fs->targetType == ForEachTargetType::Vertex || fs->targetType == ForEachTargetType::Neighbor)
    {
        declareSymbol(fs->var1, varSym);
    }
    else if (fs->targetType == ForEachTargetType::Edge)
    {
        declareSymbol(fs->var1, varSym);
        declareSymbol(fs->var2, varSym);
    }
    analyzeBlock(static_cast<BlockStmtNode *>(fs->body.get()));
    exitScope();
}

void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclNode *func)
{
    enterScope();
    returnTypeStack.push_back(typeFromString(func->returnType));
    for (auto &p : func->parameters)
    {
        Symbol sym;
        sym.isFunction = false;
        sym.type = typeFromString(p->typeName);
        declareSymbol(p->paramName, sym);
    }
    analyzeBlock(static_cast<BlockStmtNode *>(func->body.get()));
    returnTypeStack.pop_back();
    exitScope();
}

void SemanticAnalyzer::analyzeGraphDecl(GraphDeclNode *G)
{
    Symbol sym;
    sym.isFunction = false;
    sym.type = TypeKind::Graph;
    declareSymbol(G->name, sym);
    graphDecls[G->name] = G;
}

void SemanticAnalyzer::analyzeWeightedGraphDecl(WeightedGraphDeclNode *G)
{
    Symbol sym;
    sym.isFunction = false;
    sym.type = TypeKind::WeightedGraph;
    declareSymbol(G->name, sym);
    graphDecls[G->name] = G;
}

void SemanticAnalyzer::analyzeQuery(QueryNode *Q)
{
    Symbol *gSym = lookupSymbol(Q->graphName);
    if (!gSym || (gSym->type != TypeKind::Graph && gSym->type != TypeKind::WeightedGraph))
        error("query on undeclared graph: " + Q->graphName);

    if (Q->queryDesc == "dijkstra" && gSym->type != TypeKind::WeightedGraph)
        error("dijkstra requires weighted graph: " + Q->graphName);

    Symbol sym;
    sym.isFunction = false;
    if (Q->type == QueryType::INT)
        sym.type = TypeKind::Int;
    else if (Q->type == QueryType::ARRAY)
        sym.type = TypeKind::IntArray;
    else
        sym.type = TypeKind::Unknown;

    declareSymbol(Q->queryName, sym);
}

void SemanticAnalyzer::analyzePrintStmt(PrintStmtNode *PS)
{
    TypeKind t = analyzeExpr(PS->expr.get());
    if (t == TypeKind::Void || t == TypeKind::Unknown)
        error("print expects int, string, or array");
}

void SemanticAnalyzer::analyzeReturnStmt(ReturnStmtNode *ret)
{
    if (returnTypeStack.empty())
        error("return statement outside of function");

    TypeKind expected = returnTypeStack.back();
    if (expected == TypeKind::Void)
    {
        if (ret->returnValue)
            error("void function should not return a value");
        return;
    }

    if (!ret->returnValue)
        error("non-void function must return a value");

    TypeKind actual = analyzeExpr(ret->returnValue.get());
    if (actual != expected)
        error("return type mismatch");
}

void SemanticAnalyzer::analyzeGraphUpdate(GraphUpdateNode *upd)
{
    Symbol *gSym = lookupSymbol(upd->graphName);
    if (!gSym)
        error("graph update on undeclared graph: " + upd->graphName);
    if (gSym->type != TypeKind::Graph)
        error("graph update only supported on unweighted graphs: " + upd->graphName);
}

void SemanticAnalyzer::analyzeShowGraph(ShowGraphNode *S)
{
    Symbol *gSym = lookupSymbol(S->graphName);
    if (!gSym || (gSym->type != TypeKind::Graph && gSym->type != TypeKind::WeightedGraph))
        error("showgraph on undeclared graph: " + S->graphName);
}

void SemanticAnalyzer::analyzeGraphComprehension(GraphComprehensionNode *GC)
{
    Symbol *gSym = lookupSymbol(GC->graphName);
    if (!gSym || (gSym->type != TypeKind::Graph && gSym->type != TypeKind::WeightedGraph))
        error("graph comprehension on undeclared graph: " + GC->graphName);

    Symbol sym;
    sym.isFunction = false;
    sym.type = gSym->type;
    declareSymbol(GC->targetName, sym);

    auto it = graphDecls.find(GC->graphName);
    if (it != graphDecls.end())
    {
        auto *G = dynamic_cast<GraphDeclNode *>(it->second);
        if (G)
            validateGraphCondition(GC->condition.get(), G);
    }
}

void SemanticAnalyzer::validateGraphCondition(GraphConditionNode *cond, GraphDeclNode *G)
{
    if (!cond || !G)
        return;

    if (cond->op == GraphConditionOp::Connected)
    {
        auto &nodes = G->materializedNodes;
        if (std::find(nodes.begin(), nodes.end(), cond->nodeId) == nodes.end())
        {
            error("graph comprehension references missing node id: " + std::to_string(cond->nodeId));
        }
        return;
    }
    if (cond->left)
        validateGraphCondition(cond->left.get(), G);
    if (cond->right)
        validateGraphCondition(cond->right.get(), G);
}

[[noreturn]] void SemanticAnalyzer::error(const std::string &msg) const
{
    throw std::runtime_error("Semantic error: " + msg);
}

