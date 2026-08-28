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
    if (typeName == "set")
        return TypeKind::Set;
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
        // Annotate the variable node with its resolved type
        var->resolvedType = sym->type;
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
        if (sym->type != TypeKind::IntArray && sym->type != TypeKind::RealArray)
            error("subscripted value is not an array: " + baseVar->name);
        TypeKind idxTy = analyzeExpr(acc->indexExpr.get());
        if (idxTy != TypeKind::Int)
            error("array index must be int");
        TypeKind elemTy = (sym->type == TypeKind::RealArray) ? TypeKind::Real : TypeKind::Int;
        acc->resolvedType = elemTy;
        return elemTy;
    }
    case ASTNodeType::BinaryExpr:
    {
        auto *bin = static_cast<BinaryExprNode *>(expr);
        TypeKind lt = analyzeExpr(bin->lhs.get());
        TypeKind rt = analyzeExpr(bin->rhs.get());
        const std::string &op = bin->op;
        TypeKind resultType = TypeKind::Unknown;
        if (op == "&&" || op == "||")
        {
            if (lt != TypeKind::Bool || rt != TypeKind::Bool)
                error("logical operator requires bool operands: " + op);
            resultType = TypeKind::Bool;
        }
        else if (op == "==" || op == "!=")
        {
            if (lt == rt && (lt == TypeKind::Int || lt == TypeKind::Real || lt == TypeKind::Bool))
                resultType = TypeKind::Bool;
            else if ((lt == TypeKind::Int || lt == TypeKind::Real) &&
                (rt == TypeKind::Int || rt == TypeKind::Real))
                resultType = TypeKind::Bool;
            else
                error("equality requires compatible operands: " + op);
        }
        else if (op == "<" || op == "<=" || op == ">" || op == ">=")
        {
            if ((lt == TypeKind::Int || lt == TypeKind::Real) &&
                (rt == TypeKind::Int || rt == TypeKind::Real))
                resultType = TypeKind::Bool;
            else
                error("ordering comparison requires numeric operands: " + op);
        }
        else if (op == "%")
        {
            if (lt != TypeKind::Int || rt != TypeKind::Int)
                error("modulo requires int operands");
            resultType = TypeKind::Int;
        }
        else if ((lt == TypeKind::Int || lt == TypeKind::Real) &&
            (rt == TypeKind::Int || rt == TypeKind::Real))
        {
            if (lt == TypeKind::Real || rt == TypeKind::Real)
                resultType = TypeKind::Real;
            else
                resultType = TypeKind::Int;
        }
        else
        {
            error("binary operator requires numeric operands: " + op);
        }
        // Annotate the binary expression node with its result type
        bin->resolvedType = resultType;
        return resultType;
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
    case ASTNodeType::FunctionCall:
    {
        auto *call = static_cast<FunctionCallNode *>(expr);
        // Handle built-in timer() function
        if (call->name == "timer")
        {
            if (!call->arguments.empty())
                error("timer() takes no arguments");
            call->resolvedType = TypeKind::Real;
            return TypeKind::Real;
        }
        // Built-in: numVertices(G) -> int
        if (call->name == "numVertices" || call->name == "numEdges")
        {
            if (call->arguments.size() != 1)
                error(call->name + " requires exactly 1 argument (graph)");
            // Don't fully analyze the graph arg (it's looked up by name in IRGen)
            call->resolvedType = TypeKind::Int;
            return TypeKind::Int;
        }
        // Built-in: degree(G, v) -> int
        if (call->name == "degree")
        {
            if (call->arguments.size() != 2)
                error("degree requires exactly 2 arguments (graph, vertex)");
            call->resolvedType = TypeKind::Int;
            return TypeKind::Int;
        }
        // Built-in: hasEdge(G, u, v) -> bool
        if (call->name == "hasEdge")
        {
            if (call->arguments.size() != 3)
                error("hasEdge requires exactly 3 arguments (graph, u, v)");
            call->resolvedType = TypeKind::Bool;
            return TypeKind::Bool;
        }
        if (call->name == "weight")
        {
            if (call->arguments.size() != 3)
                error("weight requires exactly 3 arguments (graph, u, v)");
            call->resolvedType = TypeKind::Int;
            return TypeKind::Int;
        }
        if (call->name == "edgeWeight" || call->name == "edgeSrc" || call->name == "edgeDst")
        {
            if (call->arguments.size() != 2)
                error(call->name + " requires exactly 2 arguments (graph, edge_id)");
            call->resolvedType = TypeKind::Int;
            return TypeKind::Int;
        }
        // Built-in: setSize(s) -> int
        if (call->name == "setSize")
        {
            if (call->arguments.size() != 1)
                error("setSize requires exactly 1 argument (set)");
            call->resolvedType = TypeKind::Int;
            return TypeKind::Int;
        }
        // Built-in: min(a, b) -> int or real
        if (call->name == "min" || call->name == "max")
        {
            if (call->arguments.size() != 2)
                error(call->name + " requires exactly 2 arguments");
            TypeKind lt = analyzeExpr(call->arguments[0].get());
            TypeKind rt = analyzeExpr(call->arguments[1].get());
            if ((lt != TypeKind::Int && lt != TypeKind::Real) ||
                (rt != TypeKind::Int && rt != TypeKind::Real))
                error(call->name + " requires numeric arguments");
            TypeKind result = (lt == TypeKind::Real || rt == TypeKind::Real) ? TypeKind::Real : TypeKind::Int;
            call->resolvedType = result;
            return result;
        }
        
        // Built-in: neighbors(G, v) -> set (roaring bitmap of v's neighbors)
        if (call->name == "neighbors")
        {
            if (call->arguments.size() != 2)
                error("neighbors requires exactly 2 arguments (graph, vertex)");
            call->resolvedType = TypeKind::Set;
            return TypeKind::Set;
        }
        // Built-in: subgraph(G, vertexSet) -> graph (induced subgraph)
        if (call->name == "subgraph")
        {
            if (call->arguments.size() != 2)
                error("subgraph requires exactly 2 arguments (graph, vertexSet)");
            call->resolvedType = TypeKind::Graph;
            return TypeKind::Graph;
        }
        // Built-in: degreeOrder(G) -> int array (ordering by degree)
        if (call->name == "degreeOrder")
        {
            if (call->arguments.size() != 1)
                error("degreeOrder requires exactly 1 argument (graph)");
            call->resolvedType = TypeKind::IntArray;
            return TypeKind::IntArray;
        }
        // Built-in: orient(G, order) -> graph (DAG oriented by ordering)
        if (call->name == "orient")
        {
            if (call->arguments.size() != 2)
                error("orient requires exactly 2 arguments (graph, order)");
            call->resolvedType = TypeKind::Graph;
            return TypeKind::Graph;
        }
        // Fall through to normal function call handling
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
        // Annotate the function call node with its return type
        call->resolvedType = sym->func.returnType;
        return sym->func.returnType;
    }
    case ASTNodeType::SetContainsExpr:
    {
        auto *sc = static_cast<SetContainsExprNode *>(expr);
        // Validate that the target set exists
        if (sc->targetKind == SetTargetKind::Variable)
        {
            Symbol *sym = lookupSymbol(sc->targetName);
            if (!sym)
                error("set contains on undeclared variable: " + sc->targetName);
        }
        else
        {
            Symbol *gSym = lookupSymbol(sc->targetName);
            if (!gSym || (gSym->type != TypeKind::Graph && gSym->type != TypeKind::WeightedGraph))
                error("set contains on undeclared graph: " + sc->targetName);
        }
        if (sc->argument)
            analyzeExpr(sc->argument.get());
        return TypeKind::Bool;
    }
    case ASTNodeType::SetPopExpr:
    {
        auto *sp = static_cast<SetPopExprNode *>(expr);
        Symbol *sym = lookupSymbol(sp->setName);
        if (!sym)
            error("set pop on undeclared variable: " + sp->setName);
        return TypeKind::Int;
    }
    case ASTNodeType::NotExpr:
    {
        auto *ne = static_cast<NotExprNode *>(expr);
        TypeKind opTy = analyzeExpr(ne->operand.get());
        if (opTy != TypeKind::Bool)
            error("! (NOT) operator requires bool operand");
        ne->resolvedType = TypeKind::Bool;
        return TypeKind::Bool;
    }
    case ASTNodeType::UnaryMinusExpr:
    {
        auto *um = static_cast<UnaryMinusExprNode *>(expr);
        TypeKind opTy = analyzeExpr(um->operand.get());
        if (opTy != TypeKind::Int && opTy != TypeKind::Real)
            error("unary minus requires numeric operand");
        um->resolvedType = opTy;
        return opTy;
    }
    case ASTNodeType::Array2DAccess:
    {
        auto *a2d = static_cast<Array2DAccessNode *>(expr);
        auto *baseVar = dynamic_cast<VariableNode *>(a2d->arrayExpr.get());
        if (!baseVar)
            error("2D array access base must be a variable");
        Symbol *sym = lookupSymbol(baseVar->name);
        if (!sym)
            error("use of undeclared 2D array: " + baseVar->name);
        TypeKind rowTy = analyzeExpr(a2d->rowExpr.get());
        TypeKind colTy = analyzeExpr(a2d->colExpr.get());
        if (rowTy != TypeKind::Int || colTy != TypeKind::Int)
            error("2D array indices must be int");
        a2d->resolvedType = TypeKind::Int;
        return TypeKind::Int;
    }
    case ASTNodeType::SetLiteral:
        return TypeKind::Set;
    case ASTNodeType::SetBinaryExpr:
        return TypeKind::Set;
    case ASTNodeType::GraphMemberSet:
        return TypeKind::Set;
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
            TypeKind returnType = typeFromString(FD->returnType);
            fnSym.func.returnType = returnType;
            // Annotate function declaration with return type (may be updated later in full analysis)
            FD->resolvedReturnType = returnType;
            for (auto &p : FD->parameters)
            {
                TypeKind paramType = typeFromString(p->typeName);
                fnSym.func.paramTypes.push_back(paramType);
                // Annotate parameter with its type
                p->resolvedType = paramType;
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
    case ASTNodeType::SleepStmt:
    {
        auto *sleep = static_cast<SleepStmtNode *>(node);
        TypeKind durTy = analyzeExpr(sleep->duration.get());
        if (durTy != TypeKind::Int)
            error("sleep() requires int argument (seconds)");
        break;
    }
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
    case ASTNodeType::SetDecl:
        analyzeSetDecl(static_cast<SetDeclNode *>(node));
        break;
    case ASTNodeType::SetOperation:
        analyzeSetOperation(static_cast<SetOperationNode *>(node));
        break;
    case ASTNodeType::SetMethodCall:
        analyzeSetMethodCall(static_cast<SetMethodCallNode *>(node));
        break;
    case ASTNodeType::BreakStmt:
        if (loopDepth <= 0)
            error("break statement outside of loop");
        break;
    case ASTNodeType::ContinueStmt:
        if (loopDepth <= 0)
            error("continue statement outside of loop");
        break;
    case ASTNodeType::SwapStmt:
    {
        auto *sw = static_cast<SwapStmtNode *>(node);
        Symbol *s1 = lookupSymbol(sw->name1);
        if (!s1)
            error("swap: undeclared variable: " + sw->name1);
        Symbol *s2 = lookupSymbol(sw->name2);
        if (!s2)
            error("swap: undeclared variable: " + sw->name2);
        if (s1->type != TypeKind::IntArray || s2->type != TypeKind::IntArray)
            error("swap() requires two array arguments");
        break;
    }
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
    if (decl->isArray2D)
    {
        if (declared != TypeKind::Int)
            error("only int 2D arrays are supported: " + decl->name);
        sym.type = TypeKind::IntArray;
    }
    else if (decl->isArray)
    {
        if (declared == TypeKind::Int)
            sym.type = TypeKind::IntArray;
        else if (declared == TypeKind::Real)
            sym.type = TypeKind::RealArray;
        else
            error("only int or real arrays are supported: " + decl->name);
    }
    else
    {
        sym.type = declared;
        if (sym.type == TypeKind::Unknown)
            error("unknown type in declaration: " + decl->typeName);
    }
    // Annotate the variable declaration node with its resolved type
    decl->resolvedType = sym.type;
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
        if (sym.type == TypeKind::RealArray &&
            initTy != TypeKind::RealArray &&
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
        if (lhsTy == TypeKind::Real)
        {
            if (rhsTy != TypeKind::Real)
                error("type mismatch in real array element assignment");
            return;
        }
        if (lhsTy != TypeKind::Int)
            error("array element assignment requires int value");
        if (rhsTy != TypeKind::Int)
            error("type mismatch in array element assignment");
        return;
    }

    if (assign->lhs->type == ASTNodeType::Array2DAccess)
    {
        TypeKind lhsTy = analyzeExpr(assign->lhs.get());
        if (rhsTy != TypeKind::Int)
            error("type mismatch in 2D array element assignment");
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
    loopDepth++;
    analyzeBlock(static_cast<BlockStmtNode *>(ws->body.get()));
    loopDepth--;
}

void SemanticAnalyzer::analyzeForEach(ForEachStmtNode *fs)
{
    if (fs->targetType == ForEachTargetType::Element)
    {
        Symbol *sSym = lookupSymbol(fs->graphName);
        if (!sSym || sSym->type != TypeKind::Set)
            error("foreach element over non-set variable: " + fs->graphName);

        enterScope();
        Symbol varSym;
        varSym.isFunction = false;
        varSym.type = TypeKind::Int;
        declareSymbol(fs->var1, varSym);
        loopDepth++;
        analyzeBlock(static_cast<BlockStmtNode *>(fs->body.get()));
        loopDepth--;
        exitScope();
        return;
    }

    Symbol *gSym = lookupSymbol(fs->graphName);
    if (!gSym || (gSym->type != TypeKind::Graph && gSym->type != TypeKind::WeightedGraph))
        error("foreach over undeclared graph: " + fs->graphName);

    enterScope();
    Symbol varSym;
    varSym.isFunction = false;
    varSym.type = TypeKind::Int;

    if (fs->targetType == ForEachTargetType::Vertex)
    {
        declareSymbol(fs->var1, varSym);
    }
    else if (fs->targetType == ForEachTargetType::Neighbor)
    {
        declareSymbol(fs->var1, varSym);
        if (fs->adjNodeExpr)
            analyzeExpr(fs->adjNodeExpr.get());
    }
    else if (fs->targetType == ForEachTargetType::Edge)
    {
        declareSymbol(fs->var1, varSym);
        declareSymbol(fs->var2, varSym);
    }
    loopDepth++;
    analyzeBlock(static_cast<BlockStmtNode *>(fs->body.get()));
    loopDepth--;
    exitScope();
}

void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclNode *func)
{
    enterScope();
    TypeKind returnType = typeFromString(func->returnType);
    // Annotate the function declaration node with its return type
    func->resolvedReturnType = returnType;
    returnTypeStack.push_back(returnType);
    for (auto &p : func->parameters)
    {
        Symbol sym;
        sym.isFunction = false;
        TypeKind paramType = typeFromString(p->typeName);
        sym.type = paramType;
        // Annotate the parameter node with its resolved type
        p->resolvedType = paramType;
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
    if (t == TypeKind::Void)
        error("print expects int, string, array, or set");
    // Allow Unknown (e.g. query arrays) and Set types through
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

    for (const auto &target : upd->targets)
    {
        if (target.kind == GraphUpdateTargetKind::Node)
        {
            TypeKind t = analyzeExpr(target.value.get());
            if (t != TypeKind::Int)
                error("graph update node target must be int");
            continue;
        }

        TypeKind srcType = analyzeExpr(target.src.get());
        TypeKind dstType = analyzeExpr(target.dst.get());
        if (srcType != TypeKind::Int || dstType != TypeKind::Int)
            error("graph update edge endpoints must be int");
    }
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
    if (gSym->type != TypeKind::Graph)
        error("graph comprehension only supported on unweighted graphs: " + GC->graphName);

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

    auto countDegree = [](auto *node, auto &self) -> int {
        if (!node) return 0;
        int c = (node->op == GraphConditionOp::Degree) ? 1 : 0;
        if (node->left) c += self(node->left.get(), self);
        if (node->right) c += self(node->right.get(), self);
        return c;
    };
    if (countDegree(GC->condition.get(), countDegree) > 1)
        error("graph comprehension supports at most one degree condition");

    if (!GC->graphOperands.empty())
    {
        auto *base = dynamic_cast<GraphDeclNode *>(it->second);
        if (!base)
            error("graph comprehension base graph not found: " + GC->graphName);

        for (const auto &rhsName : GC->graphOperands)
        {
            Symbol *rhsSym = lookupSymbol(rhsName);
            if (!rhsSym || (rhsSym->type != TypeKind::Graph && rhsSym->type != TypeKind::WeightedGraph))
                error("graph comprehension uses undeclared graph: " + rhsName);
            if (rhsSym->type != TypeKind::Graph)
                error("graph comprehension only supports unweighted graphs: " + rhsName);

            auto itR = graphDecls.find(rhsName);
            if (itR == graphDecls.end())
                continue;

            auto *rhs = dynamic_cast<GraphDeclNode *>(itR->second);
            if (!rhs)
                error("graph comprehension only supports unweighted graphs: " + rhsName);

            if (!base->isFileGraph && !rhs->isFileGraph && base->node_ids != rhs->node_ids)
                error("graph comprehension requires graphs with identical node sets/order");
        }
    }
}

void SemanticAnalyzer::validateGraphCondition(GraphConditionNode *cond, GraphDeclNode *G)
{
    if (!cond || !G)
        return;

    if (cond->op == GraphConditionOp::Cycle)
    {
        return;
    }
    if (cond->op == GraphConditionOp::Degree)
    {
        return;
    }
    if (cond->op == GraphConditionOp::Connected)
    {
        if (!G->isFileGraph)
        {
            auto &nodes = G->node_ids;
            if (std::find(nodes.begin(), nodes.end(), cond->nodeId) == nodes.end())
            {
                error("graph comprehension references missing node id: " + std::to_string(cond->nodeId));
            }
        }
        return;
    }
    if (cond->left)
        validateGraphCondition(cond->left.get(), G);
    if (cond->right)
        validateGraphCondition(cond->right.get(), G);
}

void SemanticAnalyzer::analyzeSetDecl(SetDeclNode *SD)
{
    Symbol sym;
    sym.isFunction = false;
    sym.type = TypeKind::Set;
    declareSymbol(SD->name, sym);
    // Initializer is a set literal or set binary expr — no deep validation needed
}

void SemanticAnalyzer::analyzeSetOperation(SetOperationNode *setOp)
{
    Symbol *sym = lookupSymbol(setOp->targetName);
    if (!sym)
        error("set operation on undeclared variable: " + setOp->targetName);
    if (sym->type != TypeKind::Set)
        error("set operation on non-set variable: " + setOp->targetName);
}

void SemanticAnalyzer::analyzeSetMethodCall(SetMethodCallNode *node)
{
    if (node->targetKind == SetTargetKind::Variable)
    {
        Symbol *sym = lookupSymbol(node->targetName);
        if (!sym)
            error("set method call on undeclared variable: " + node->targetName);
    }
    else
    {
        // GraphNodes or GraphEdges — verify graph exists
        Symbol *gSym = lookupSymbol(node->targetName);
        if (!gSym || (gSym->type != TypeKind::Graph && gSym->type != TypeKind::WeightedGraph))
            error("set method on undeclared graph: " + node->targetName);
    }
    if (node->argument)
        analyzeExpr(node->argument.get());
}

[[noreturn]] void SemanticAnalyzer::error(const std::string &msg) const
{
    throw std::runtime_error("Semantic error: " + msg);
}
