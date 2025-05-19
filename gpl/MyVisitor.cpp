#include "MyVisitor.h"
#include <iostream>
#include <optional>
#include <queue>
#include "utilities.h"

antlrcpp::Any MyVisitor::visitProgram(BaseParser::ProgramContext *ctx)
{
    // std::cout << "Visiting program" << std::endl;

    BaseParser::FunctionContext *mainFunction = nullptr;

    for (auto function : ctx->function())
    {
        auto functionName = function->ID()->getText();
        if (functionName == "main")
        {
            mainFunction = function; // hold main function
        }
        visitFunction(function); // Visiting all functions
    }
    for (auto statement : ctx->statement())
    {
        visitStatement(statement); // executing statements which are not part of functions
    }

    // first visit everything and hold main function, then execute (global) statements, finally execute main function

    if (mainFunction)
    {
        // std::cout << "Main function found. Executing main..." << std::endl;
        visitBlock(mainFunction->block()); // Execute the main function
    }

    return nullptr;
}

// Visit individual statements
antlrcpp::Any MyVisitor::visitStatement(BaseParser::StatementContext *ctx)
{
    // std::cout << "Visiting statement" << std::endl;

    if (ctx->graphDef())
    {
        // std::cout << "Visiting graph definition" << std::endl;
        return visitGraphDef(ctx->graphDef());
    }
    else if (ctx->conditionalStatement())
    {
        // std::cout << "Visiting conditional statement" << std::endl;
        return visitConditionalStatement(ctx->conditionalStatement());
    }
    else if (ctx->printStatement())
    {
        // std::cout << "Visiting print statement" << std::endl;
        return visitPrintStatement(ctx->printStatement());
    }
    else if (ctx->loopStatement())
    {
        // std::cout << "Visiting loop statement" << std::endl;
        return visitLoopStatement(ctx->loopStatement());
    }
    else if (ctx->varDecl())
    {
        // std::cout << " visting assignment " << "\n";
        return visitVarDecl(ctx->varDecl());
    }
    else if (ctx->functionCall())
    {
        // std::cout << "visiting functions " << "\n";
        return visitFunctionCall(ctx->functionCall());
    }
    else if (ctx->graphComprehension())
    {
        // std::cout<<"visitng compre";
        return visitGraphComprehension(ctx->graphComprehension());
    }
    else if (ctx->queryStatement())
    {
        // std:: cout<<" visiitng query st";
        return visitQueryStatement(ctx->queryStatement());
    }
    else if (ctx->showgraph())
    {
        // std::cout<<"visiting show graph";
        return visitShowgraph(ctx->showgraph());
    }

    return visitChildren(ctx);
}

// Visit Graph Definition
antlrcpp::Any MyVisitor::visitGraphDef(BaseParser::GraphDefContext *ctx)
{
    graphName = ctx->graphID()->getText();
    // std::cout << "Graph Name: " << graphName << std::endl;

    visit(ctx->nodes());
    visit(ctx->edges());
    return nullptr;
}

// Visit Nodes
antlrcpp::Any MyVisitor::visitNodes(BaseParser::NodesContext *ctx)
{
    auto parentCtx = dynamic_cast<BaseParser::GraphDefContext *>(ctx->parent);
    std::string gName = parentCtx->graphID()->getText();
    // std::cout << "Graph Name: " << gName << std::endl;

    for (auto Node : ctx->nodeList()->nodeID())
    {
        node newNode(Node->getText());
        addNode(gName, newNode); // Store node IDs
    }

    return nullptr;
}

// Visit Edges
antlrcpp::Any MyVisitor::visitEdges(BaseParser::EdgesContext *ctx)
{
    auto parentCtx = dynamic_cast<BaseParser::GraphDefContext *>(ctx->parent);
    std::string gName = parentCtx->graphID()->getText();
    if (ctx->fileEdgeList())
    {
        // Extract the filename
        std::string filename = ctx->fileEdgeList()->STRING()->getText();

        // Remove surrounding quotes
        if (!filename.empty() && (filename.front() == '"' || filename.front() == '\''))
        {
            filename = filename.substr(1, filename.size() - 2);
        }

        // Open and read the file
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::string line;
        while (std::getline(file, line))
        {
            // Trim leading/trailing whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            // Skip empty lines
            if (line.empty())
            {
                continue;
            }

            std::istringstream iss(line);
            std::string fromToken, toToken;

            // Parse two integers from the line
            if (iss >> fromToken >> toToken)
            {
                node from(fromToken);
                node to(toToken);
                addEdge(gName, from, to);
            }
            else
            {
                std::cerr << "Invalid edge format in file: " << line << std::endl;
            }
        }

        file.close();
    }
    else
    {
        for (auto edge : ctx->edgeList()->edge())
        {
            node from(edge->nodeID(0)->getText());
            node to(edge->nodeID(1)->getText());
            addEdge(gName, from, to);
        }
    }

    return nullptr;
}

// Visit conditional statement
antlrcpp::Any MyVisitor::visitConditionalStatement(BaseParser::ConditionalStatementContext *ctx)
{
    // std::cout << "h1\n";
    auto conditionResult = visitCondition(ctx->condition());
    // std::cout << "Condition result type1: " << conditionResult.type().name() << "\n";
    bool condition = std::any_cast<bool>(conditionResult);
    // std::cout << "condition = " << condition;
    // exit(0);
    // bool condition = std::any_cast<bool>(visit(ctx->condition()));
    //  bool condition1 = false;
    //  bool condition2 = true;
    if (condition)
    {
        return visitBlock(ctx->block(0)); // Execute the first block if the condition is true
    }
    else if (ctx->conditionalStatement())
    {
        // Visit the 'else if' block recursively
        return visitConditionalStatement(ctx->conditionalStatement());
    }
    else if (ctx->block(1))
    {
        return visitBlock(ctx->block(1)); // Execute the second block (else block) if present
    }
    return nullptr; // No action if the condition is false and there's no else block
}

// visit graph comprehension
antlrcpp::Any MyVisitor::visitGraphComprehension(BaseParser::GraphComprehensionContext *ctx)
{
    std::string originalGraphName = ctx->graphID()->getText();
    std::string newGraphName = ctx->ID()->getText();
    BaseParser::GraphConditionContext *condition = ctx->graphCondition();

    // checking if original graph exists
    if (graphs.find(originalGraphName) == graphs.end())
    {
        throw std::runtime_error("Graph '" + originalGraphName + "' does not exist.");
        return nullptr;
    }

    // Create a new graph
    graph newGraph;

    // Iterate through the original graph's nodes and apply the condition
    for (const auto &[node, neighbors] : graphs[originalGraphName].adjacencyList)
    {
        if (evaluateGraphCondition(node, originalGraphName, condition))
        {
            newGraph.nodes = graphs[originalGraphName].nodes;         // Add node to the new graph
            newGraph.edges = graphs[originalGraphName].edges;         // Copy edges from the original graph
            newGraph.adjacencyList[node] = neighbors;                 // Add node and its edges to the new graph
            newGraph.nodeId = graphs[originalGraphName].nodeId;       // Copy node ID mapping
            newGraph.nodeIndex = graphs[originalGraphName].nodeIndex; // Copy node index mapping
            if (graphs[originalGraphName].adjacencyMatrixMode)
            {
                newGraph.adjacencyMatrixMode = true;
                newGraph.adjacencyMatrix = graphs[originalGraphName].adjacencyMatrix; // Copy adjacency matrix
            }
            if (graphs[originalGraphName].isDirected())
            {
                newGraph.setDirected(true);
            }
            if (graphs[originalGraphName].isWeighted())
            {
                newGraph.setWeighted(true);
            }
            if (graphs[originalGraphName].isStochastic())
            {
                newGraph.setStochastic(true);
            }
        }

        // Store the new graph
    }
    graphs[newGraphName] = newGraph;
    return nullptr; // No specific return required
}

// Evaluate GraphCondition
bool MyVisitor::evaluateGraphCondition(node Node, const std::string &gName, BaseParser::GraphConditionContext *ctx)
{
    if (auto andContext = dynamic_cast<BaseParser::GraphLogicalAndContext *>(ctx))
    {
        // Logical AND condition
        return evaluateGraphCondition(Node, gName, andContext->graphCondition(0)) &&
               evaluateGraphCondition(Node, gName, andContext->graphCondition(1));
    }
    if (auto orContext = dynamic_cast<BaseParser::GraphLogicalOrContext *>(ctx))
    {
        // Logical OR condition
        return evaluateGraphCondition(Node, gName, orContext->graphCondition(0)) ||
               evaluateGraphCondition(Node, gName, orContext->graphCondition(1));
    }
    if (auto degreeCondition = dynamic_cast<BaseParser::DegreeConditionContext *>(ctx))
    {
        // Degree condition
        std::string operatorStr = degreeCondition->EQUAL() ? degreeCondition->EQUAL()->getText() : degreeCondition->NOTEQUAL()   ? degreeCondition->NOTEQUAL()->getText()
                                                                                               : degreeCondition->LESSEQUAL()    ? degreeCondition->LESSEQUAL()->getText()
                                                                                               : degreeCondition->GREATEREQUAL() ? degreeCondition->GREATEREQUAL()->getText()
                                                                                               : degreeCondition->LESSTHAN()     ? degreeCondition->LESSTHAN()->getText()
                                                                                                                                 : degreeCondition->GREATERTHAN()->getText();
        int value = std::stoi(degreeCondition->INT()->getText());
        return evaluateDegreeCondition(Node, gName, operatorStr, value);
    }
    if (auto connectedCondition = dynamic_cast<BaseParser::ConnectedConditionContext *>(ctx))
    {
        // Connected condition
        node targetNode;
        targetNode.id = connectedCondition->nodeID()->getText();
        return evaluateConnectedCondition(gName, Node, targetNode);
    }
    if (auto parenCondition = dynamic_cast<BaseParser::ParenGraphConditionContext *>(ctx))
    {
        // Parenthesized condition
        return evaluateGraphCondition(Node, gName, parenCondition->graphCondition());
    }

    throw std::runtime_error("Invalid graph condition encountered.");
}

// Evaluate degree condition
bool MyVisitor::evaluateDegreeCondition(node node, const std::string &gName, const std::string &operatorStr, int value)
{
    auto &graphNodes = graphs[gName];
    int degree = graphNodes.adjacencyList[node].size();

    if (operatorStr == "==")
        return degree == value;
    if (operatorStr == "!=")
        return degree != value;
    if (operatorStr == "<")
        return degree < value;
    if (operatorStr == ">")
        return degree > value;
    if (operatorStr == "<=")
        return degree <= value;
    if (operatorStr == ">=")
        return degree >= value;

    throw std::runtime_error("Unknown comparison operator in degree condition.");
}

// Evaluate connected condition
bool MyVisitor::evaluateConnectedCondition(const std::string &gName, node Node, node targetNode)
{
    auto &graphNodes = graphs[gName];
    if (find(graphNodes.nodes.begin(), graphNodes.nodes.end(), Node) == graphNodes.nodes.end())
    {
        return false; // Node does not exist.
    }
    else
    {
        return edgeExists(gName, Node, targetNode);
    }
}

// query
antlrcpp::Any MyVisitor::visitQueryStatement(BaseParser::QueryStatementContext *ctx)
{
    // Extract the query variable name, function name (dfs, detect cycle) and graph ID
    std::string queryVariable = ctx->ID()->getText();
    std::string queryType = ctx->STRING()->getText();
    queryType = queryType.substr(1, queryType.size() - 2); // Remove quotes
    std::string graphID = ctx->graphID()->getText();

    // Ensure the graph exists in the symbol table
    if (graphs.find(graphID) == graphs.end())
    {
        throw std::runtime_error("Graph '" + graphID + "' not found.");
    }

    // Get the adjacency list of the specified graph
    auto adjList = graphs[graphID].adjacencyList;
    graph &bfsgraph = graphs[graphID];

    // Execute the query based on its type
    std::string result;
    // result = "hh";
    if (queryType == "bfs")
    {
        result = bfs(bfsgraph);
    }
    // if (queryType == "bfs") {
    //     //result = BFS(bfsgraph); // Assuming bfs returns a string result
    // }
    // else if (queryType == "detectCycle"){
    //     result = detectCycle(adjList);
    // }
    // else if (queryType == "bfs") {
    //     result = dfs(adjList); // Assuming dfs returns a string result
    // }
    // } else if (queryType == "connected_components") {
    //     result = findConnectedComponents(adjList); // Assuming this returns a string result
    // } else {
    //     throw std::runtime_error("Unknown query type: " + queryType);
    // }

    // Store the result in the symbol table under the query variable
    symbolTable[queryVariable] = result;

    return nullptr;
}

std::string MyVisitor::bfs(const graph &graph)
{
    std::string result;
    std::unordered_set<node> visited;
    for (const auto &Node : graph.adjacencyList)
    {
        if (visited.count(Node.first))
            continue;
        std::queue<node> q;
        q.push(Node.first);
        visited.insert(Node.first);

        while (!q.empty())
        {
            node current = q.front();
            q.pop();
            result += utilities::nodeTypeToString(current.id) + " ";

            for (node neighbor : graph.adjacencyList.at(current))
            {
                if (!visited.count(neighbor))
                {
                    q.push(neighbor);
                    visited.insert(neighbor);
                }
            }
        }
    }
    return result;
}

antlrcpp::Any MyVisitor::visitShowgraph(BaseParser::ShowgraphContext *ctx)
{
    std::string graphID = ctx->graphID()->getText(); // Get the graph ID from the context

    // Check if the graph exists in the symbol table
    if (graphs.find(graphID) == graphs.end())
    {
        std::cerr << "Error: Graph '" << graphID << "' not found." << std::endl;
        return nullptr;
    }

    // Get the graph from the symbol table
    auto mm = graphs[graphID];

    // Generate DOT file and visualize the graph
    generateDotFile(mm, graphID + ".dot");
    showGraph(graphID);

    return nullptr;
}

std::string nodeTypeToString(const NodeType &value)
{
    return std::visit([](auto &&v) -> std::string
                      {
        using U = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<U, std::string>) {
            return v;
        } else if constexpr (std::is_same_v<U, char>) {
            return std::string(1, v);
        } else if constexpr (std::is_same_v<U, bool>) {
            return v ? "true" : "false";
        } else if constexpr (std::is_arithmetic_v<U>) {
            return std::to_string(v);
        } else {
            return {}; // Fallback for unknown types
        } }, value);
}

void MyVisitor::generateDotFile(graph &graph, const std::string &filename)
{
    std::ofstream outFile(filename);
    outFile << "graph G {\n";

    // Loop over the graph and write the edges to the DOT file
    for (const auto &edge : graph.edges)
    {
        outFile << "    " << nodeTypeToString(edge.from.id) << " -- " << nodeTypeToString(edge.to.id) << ";\n";
    }

    outFile << "}\n";
    outFile.close();
    std::cout << "DOT file generated: " << filename << std::endl;
}

void MyVisitor::showGraph(const std::string &graphID)
{
    // Use Graphviz (dot) to render the DOT file to an image
    std::string command = "dot -Tpng " + graphID + ".dot -o " + graphID + ".png";
    if (system(command.c_str()) == 0)
    {
        std::cout << "Graph visualization saved as " << graphID << ".png" << std::endl;
        system(("open " + graphID + ".png").c_str());
    }
    else
    {
        std::cerr << "Error: Failed to render the graph using Graphviz." << std::endl;
    }
}

// function

antlrcpp::Any MyVisitor::visitFunction(BaseParser::FunctionContext *ctx)
{

    std::string functionName = ctx->ID()->getText();
    std::string returnType = ctx->returnType()->getText();
    // Corrected to use visitParamList correctly
    std::vector<std::pair<std::string, std::string>> paramList =
        std::any_cast<std::vector<std::pair<std::string, std::string>>>(visitParamList(ctx->paramList()));

    if (functions.find(functionName) != functions.end())
    {
        throw std::runtime_error("Function " + functionName + " already defined.");
    }

    FunctionDefinition funcDef{returnType, paramList, ctx->block()};
    functions[functionName] = funcDef;

    std::cout << "Function " << functionName << " with return type " << returnType << " defined.\n";
    return nullptr;
}

antlrcpp::Any MyVisitor::visitParamList(BaseParser::ParamListContext *ctx)
{
    std::vector<std::pair<std::string, std::string>> params;

    // Check if there are any parameters
    if (ctx->param().empty())
    {
        return params; // Return empty parameter list if no parameters exist
    }

    // Process each parameter if they exist
    for (auto paramCtx : ctx->param())
    {
        auto param = std::any_cast<std::pair<std::string, std::string>>(visitParam(paramCtx));
        params.push_back(param);
    }

    return params;
}

antlrcpp::Any MyVisitor::visitParam(BaseParser::ParamContext *ctx)
{
    std::string type = ctx->type()->getText();
    std::string name = ctx->ID()->getText();
    return std::make_pair(type, name);
}

// 33
antlrcpp::Any MyVisitor::visitFunctionCall(BaseParser::FunctionCallContext *ctx)
{
    // std::cout << "Visiting function call: \n";

    std::string functionName;
    if (ctx->ID() != nullptr)
    {
        functionName = ctx->ID()->getText();
        std::cout << "Function name: " << functionName << std::endl;
    }
    else
    {
        std::cerr << "Error: Function call ID is missing!" << std::endl;
        throw std::runtime_error("Function call ID is missing!");
    }

    // Built-in print function (if not in user-defined functions)
    if (functionName == "print")
    {
        std::vector<std::any> args;
        if (ctx->argumentList())
        {
            for (auto expr : ctx->argumentList()->expr())
            {
                args.push_back(visitExpr(expr));
            }
        }

        for (const auto &arg : args)
        {
            if (arg.type() == typeid(int))
                std::cout << std::any_cast<int>(arg);
            else if (arg.type() == typeid(double))
                std::cout << std::any_cast<double>(arg);
            else if (arg.type() == typeid(std::string))
                std::cout << std::any_cast<std::string>(arg);
            else if (arg.type() == typeid(bool))
                std::cout << (std::any_cast<bool>(arg) ? "true" : "false");
            else if (arg.type() == typeid(std::vector<int>))
            {
                const auto &vec = std::any_cast<std::vector<int>>(arg);
                std::cout << "[";
                for (size_t i = 0; i < vec.size(); ++i)
                {
                    std::cout << vec[i];
                    if (i < vec.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "]";
            }
            else if (arg.type() == typeid(std::vector<double>))
            {
                const auto &vec = std::any_cast<std::vector<double>>(arg);
                std::cout << "[";
                for (size_t i = 0; i < vec.size(); ++i)
                {
                    std::cout << vec[i];
                    if (i < vec.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "]";
            }
            else
            {
                std::cerr << "Unsupported print type: " << arg.type().name() << std::endl;
            }

            std::cout << " "; // Separate arguments
        }
        std::cout << std::endl;

        return nullptr;
    }

    // Check if the function exists
    if (functions.find(functionName) == functions.end())
    {
        throw std::runtime_error("Function '" + functionName + "' is not defined.");
    }

    // Retrieve function definition
    auto functionDef = functions[functionName];
    const auto &paramList = functionDef.parameters;
    auto *functionBody = functionDef.block;

    // Evaluate arguments if present
    std::vector<std::any> args;
    if (ctx->argumentList())
    {
        auto arguments = ctx->argumentList()->expr();
        std::cout << "Number of arguments: " << arguments.size() << std::endl;

        if (arguments.size() != paramList.size())
        {
            throw std::runtime_error("Function '" + functionName + "' expects " +
                                     std::to_string(paramList.size()) + " arguments, but got " +
                                     std::to_string(arguments.size()) + ".");
        }

        for (size_t i = 0; i < arguments.size(); ++i)
        {
            // std::cout << "Processing argument " << i << std::endl;
            auto argumentValue = visitExpr(arguments[i]);
            args.push_back(argumentValue);

            // Debugging the argument value and type
            if (argumentValue.type() == typeid(int))
            {
                // std::cout << "Argument " << i << " is of type int with value: " << std::any_cast<int>(argumentValue) << std::endl;
            }
            else if (argumentValue.type() == typeid(std::string))
            {
                // std::cout << "Argument " << i << " is of type string with value: " << std::any_cast<std::string>(argumentValue) << std::endl;
            }
            else if (argumentValue.type() == typeid(double))
            {
                // std::cout << "Argument " << i << " is of type int with value: " << std::any_cast<int>(argumentValue) << std::endl;
            }
            else
            {
                std::cout << "Argument " << i << " has an unknown type!" << std::endl;
            }
        }
    }

    // Store current symbol table and replace with function's local scope
    std::unordered_map<std::string, std::any> originalSymbolTable = symbolTable;

    // Map parameters to arguments if both exist
    if (!paramList.empty() && !args.empty())
    {
        for (size_t i = 0; i < paramList.size(); ++i)
        {
            const auto &param = paramList[i];
            const std::string &paramName = param.second;
            std::cout << "Mapping argument " << i << " of type " << param.first << " to parameter " << paramName << std::endl;

            // Cast arguments based on the parameter type
            try
            {
                if (param.first == "int")
                {
                    try
                    {
                        std::cout << "Trying to cast argument " << i << " to int\n";
                        int val = std::any_cast<int>(args[i]);
                        symbolTable[paramName] = val;
                        std::cout << "Mapped int argument: " << val << std::endl;
                    }
                    catch (const std::bad_any_cast &e)
                    {
                        // std::cout << "3333333\n";
                        std::cerr << "Failed to cast argument " << i << " to int: " << e.what() << "\n";
                        std::cerr << "Actual type was: " << args[i].type().name() << "\n";
                        throw;
                    }
                }
                else if (param.first == "string")
                {
                    try
                    {
                        std::cout << "Trying to cast argument " << i << " to string\n";
                        std::string val = std::any_cast<std::string>(args[i]);
                        symbolTable[paramName] = val;
                        std::cout << "Mapped string argument: " << val << std::endl;
                    }
                    catch (const std::bad_any_cast &e)
                    {
                        // std::cout << "222222\n";
                        std::cerr << "Failed to cast argument " << i << " to string: " << e.what() << "\n";
                        std::cerr << "Actual type was: " << args[i].type().name() << "\n";
                        throw;
                    }
                }
                else if (param.first == "real")
                {

                    try
                    {
                        std::cout << "Trying to cast argument " << i << " to double\n";
                        double val = std::any_cast<double>(args[i]);
                        symbolTable[paramName] = val;
                        std::cout << "Mapped real argument: " << val << std::endl;
                    }
                    catch (const std::bad_any_cast &e)
                    {
                        // std::cout << "1111111\n";
                        std::cerr << "Failed to cast argument " << i << " to double: " << e.what() << "\n";
                        std::cerr << "Actual type was: " << args[i].type().name() << "\n";
                        throw;
                    }
                }
                else
                {
                    throw std::runtime_error("Unsupported parameter type: " + param.first);
                }
            }
            catch (const std::bad_any_cast &e)
            {
                // std::cout << "4444444\n";
                std::cerr << "Error casting argument " << i << " of type " << param.first << ": " << e.what() << std::endl;
                throw;
            }
        }
    }
    else
    {
        // std::cout << "No parameters or arguments to map for function: " << functionName << std::endl;
    }

    // Execute the function body
    antlrcpp::Any returnValue = nullptr;
    try
    {
        returnValue = visitBlock(functionBody);
    }
    catch (const std::string &returnSignal)
    {
        // Handle return statement signal
        returnValue = symbolTable["__return"];
    }

    // Restore the original symbol table
    symbolTable = originalSymbolTable;

    return returnValue;
}

antlrcpp::Any MyVisitor::visitBlock(BaseParser::BlockContext *ctx)
{
    for (auto stmt : ctx->statement())
    {
        visitStatement(stmt);
    }
    if (!ctx->returnStatement().empty())
    {
        return visitReturnStatement(ctx->returnStatement()[0]);
    }
    return nullptr;
}

// Visit return statement
antlrcpp::Any MyVisitor::visitReturnStatement(BaseParser::ReturnStatementContext *ctx)
{
    return visitExpr(ctx->expr());
}

antlrcpp::Any MyVisitor::visitExpr(BaseParser::ExprContext *ctx)
{
    if (auto mulDiv = dynamic_cast<BaseParser::MulDivExprContext *>(ctx))
    {
        auto left = visitExpr(mulDiv->expr(0));
        auto right = visitExpr(mulDiv->expr(1));

        if (left.type() == typeid(double) || right.type() == typeid(double))
        {
            double l = (left.type() == typeid(int)) ? static_cast<double>(std::any_cast<int>(left)) : std::any_cast<double>(left);
            double r = (right.type() == typeid(int)) ? static_cast<double>(std::any_cast<int>(right)) : std::any_cast<double>(right);
            return mulDiv->TIMES() ? l * r : l / r;
        }
        else
        {
            int l = std::any_cast<int>(left);
            int r = std::any_cast<int>(right);
            return mulDiv->TIMES() ? l * r : l / r;
        }
    }

    else if (auto addSub = dynamic_cast<BaseParser::AddSubExprContext *>(ctx))
    {
        auto left = visitExpr(addSub->expr(0));
        auto right = visitExpr(addSub->expr(1));

        if (left.type() == typeid(double) || right.type() == typeid(double))
        {
            double l = (left.type() == typeid(int)) ? static_cast<double>(std::any_cast<int>(left)) : std::any_cast<double>(left);
            double r = (right.type() == typeid(int)) ? static_cast<double>(std::any_cast<int>(right)) : std::any_cast<double>(right);
            return addSub->PLUS() ? l + r : l - r;
        }
        else
        {
            int l = std::any_cast<int>(left);
            int r = std::any_cast<int>(right);
            return addSub->PLUS() ? l + r : l - r;
        }
    }

    else if (auto funcCall = dynamic_cast<BaseParser::FuncExprContext *>(ctx))
    {
        return visitFunctionCall(funcCall->functionCall());
    }

    else if (auto intLit = dynamic_cast<BaseParser::IntExprContext *>(ctx))
    {
        return std::stoi(intLit->getText());
    }

    else if (auto realLit = dynamic_cast<BaseParser::RealExprContext *>(ctx))
    {
        return std::stod(realLit->getText());
    }

    else if (auto boolTrue = dynamic_cast<BaseParser::BoolTrueExprContext *>(ctx))
    {
        return true;
    }

    else if (auto boolFalse = dynamic_cast<BaseParser::BoolFalseExprContext *>(ctx))
    {
        return false;
    }

    else if (auto idExpr = dynamic_cast<BaseParser::IdExprContext *>(ctx))
    {
        std::string id = idExpr->getText();

        if (symbolTable.find(id) == symbolTable.end())
            throw std::runtime_error("Undefined variable: " + id);

        auto value = symbolTable[id];

        if (value.type() == typeid(int))
            return std::any_cast<int>(value);
        if (value.type() == typeid(double))
            return std::any_cast<double>(value);
        if (value.type() == typeid(bool))
            return std::any_cast<bool>(value);
        if (value.type() == typeid(std::string))
            return std::any_cast<std::string>(value);

        if (value.type() == typeid(std::vector<int>) ||
            value.type() == typeid(std::vector<double>) ||
            value.type() == typeid(std::vector<bool>) ||
            value.type() == typeid(std::vector<std::string>))
        {
            return value; // Return array reference to be used by print or access
        }

        std::cerr << "Unhandled type for variable '" << id << "': " << value.type().name() << std::endl;
        throw std::runtime_error("Unsupported variable type for '" + id + "'");
    }

    else if (auto paren = dynamic_cast<BaseParser::ParenExprContext *>(ctx))
    {
        return visitExpr(paren->expr());
    }

    else if (auto access = dynamic_cast<BaseParser::ArrayAccessExprContext *>(ctx))
    {
        std::string arrayName = access->ID()->getText();
        int index = std::any_cast<int>(visitExpr(access->expr()));

        if (symbolTable.find(arrayName) == symbolTable.end())
            throw std::runtime_error("Undefined array: " + arrayName);

        auto &arr = symbolTable[arrayName];

        if (arr.type() == typeid(std::vector<int>))
        {
            auto &vec = std::any_cast<std::vector<int> &>(arr);
            if (index < 0 || index >= vec.size())
                throw std::runtime_error("Index out of bounds");
            return vec[index];
        }
        else if (arr.type() == typeid(std::vector<double>))
        {
            auto &vec = std::any_cast<std::vector<double> &>(arr);
            if (index < 0 || index >= vec.size())
                throw std::runtime_error("Index out of bounds");
            return vec[index];
        }
        else if (arr.type() == typeid(std::vector<std::string>))
        {
            auto &vec = std::any_cast<std::vector<std::string> &>(arr);
            if (index < 0 || index >= vec.size())
                throw std::runtime_error("Index out of bounds");
            return vec[index];
        }
        else if (arr.type() == typeid(std::vector<bool>))
        {
            auto &vec = std::any_cast<std::vector<bool> &>(arr);
            if (index < 0 || index >= vec.size())
                throw std::runtime_error("Index out of bounds");
            return vec[index];
        }
        else
        {
            throw std::runtime_error("Variable is not an array: " + arrayName);
        }
    }
    else if (auto access2d = dynamic_cast<BaseParser::Array2DAccessExprContext *>(ctx))
    {
        std::string arrayName = access2d->ID()->getText();
        int row = std::any_cast<int>(visitExpr(access2d->expr(0)));
        int col = std::any_cast<int>(visitExpr(access2d->expr(1)));

        if (symbolTable.find(arrayName) == symbolTable.end())
            throw std::runtime_error("Undefined array: " + arrayName);

        auto &arr = symbolTable[arrayName];

        if (arr.type() == typeid(std::vector<std::vector<int>>))
        {
            auto &vec2d = std::any_cast<std::vector<std::vector<int>> &>(arr);
            if (row < 0 || row >= vec2d.size() || col < 0 || col >= vec2d[row].size())
                throw std::runtime_error("2D Index out of bounds");
            std::cout << "[visitExpr] Returning 2D array element of type: " << typeid(vec2d[row][col]).name() << std::endl;

            return vec2d[row][col];
        }
        else if (arr.type() == typeid(std::vector<std::vector<double>>))
        {
            auto &vec2d = std::any_cast<std::vector<std::vector<double>> &>(arr);
            if (row < 0 || row >= vec2d.size() || col < 0 || col >= vec2d[row].size())
                throw std::runtime_error("2D Index out of bounds");
            std::cout << "[visitExpr] Returning 2D array element of type: " << typeid(vec2d[row][col]).name() << std::endl;

            return vec2d[row][col];
        }
        else if (arr.type() == typeid(std::vector<std::vector<std::string>>))
        {
            auto &vec2d = std::any_cast<std::vector<std::vector<std::string>> &>(arr);
            if (row < 0 || row >= vec2d.size() || col < 0 || col >= vec2d[row].size())
                throw std::runtime_error("2D Index out of bounds");
            std::cout << "[visitExpr] Returning 2D array element of type: " << typeid(vec2d[row][col]).name() << std::endl;

            return vec2d[row][col];
        }
        else if (arr.type() == typeid(std::vector<std::vector<bool>>))
        {
            auto &vec2d = std::any_cast<std::vector<std::vector<bool>> &>(arr);
            if (row < 0 || row >= vec2d.size() || col < 0 || col >= vec2d[row].size())
                throw std::runtime_error("2D Index out of bounds");
            std::cout << "[visitExpr] Returning 2D array element of type: " << typeid(vec2d[row][col]).name() << std::endl;

            return vec2d[row][col];
        }
        else
        {
            throw std::runtime_error("Variable is not a 2D array: " + arrayName);
        }
    }
    else if (auto assign2d = dynamic_cast<BaseParser::Array2DAssignStmtContext *>(ctx))
    {
        std::string arrayName = assign2d->ID()->getText();
        int row = std::stoi(assign2d->INT(0)->getText());
        int col = std::stoi(assign2d->INT(1)->getText());
        std::any value = visitExpr(assign2d->expr());

        if (symbolTable.find(arrayName) == symbolTable.end())
            throw std::runtime_error("Undefined array: " + arrayName);

        auto &arr = symbolTable[arrayName];

        if (arr.type() == typeid(std::vector<std::vector<int>>))
        {
            auto &vec2d = std::any_cast<std::vector<std::vector<int>> &>(arr);
            if (row < 0 || row >= vec2d.size() || col < 0 || col >= vec2d[row].size())
                throw std::runtime_error("2D Index out of bounds");

            if (value.type() == typeid(int))
                vec2d[row][col] = std::any_cast<int>(value);
            else if (value.type() == typeid(double))
                vec2d[row][col] = static_cast<int>(std::any_cast<double>(value));
            else
                throw std::runtime_error("Invalid value for int 2D array assignment: " + arrayName);
        }
        // Optional: Add support for double, string, bool 2D arrays here

        else
        {
            throw std::runtime_error("Variable is not a 2D array: " + arrayName);
        }

        return nullptr;
    }

    throw std::runtime_error("Visit to expr() returned void or no value! Unrecognized expression node: " + std::string(ctx->getText()));

    // return 0; // Fallback default
}

// Check if node exists
antlrcpp::Any MyVisitor::visitVarDecl(BaseParser::VarDeclContext *ctx)
{
    // Case 1: Scalar variable declaration
    if (auto simpleDeclarationContext = dynamic_cast<BaseParser::SimpleDeclarationContext *>(ctx))
    {
        std::string type = simpleDeclarationContext->type()->getText();
        std::string name = simpleDeclarationContext->ID()->getText();

        if (symbolTable.find(name) != symbolTable.end())
        {
            throw std::runtime_error("Variable '" + name + "' is already declared.");
        }

        std::any value;
        if (simpleDeclarationContext->expr())
        {
            value = visitExpr(simpleDeclarationContext->expr());

            if (type == "int" && value.type() != typeid(int))
                throw std::runtime_error("Type mismatch: Expected 'int' for '" + name + "'");
            if (type == "real" && value.type() != typeid(double))
                throw std::runtime_error("Type mismatch: Expected 'real' for '" + name + "'");
            if (type == "string" && value.type() != typeid(std::string))
                throw std::runtime_error("Type mismatch: Expected 'string' for '" + name + "'");
            if (type == "bool" && value.type() != typeid(bool))
                throw std::runtime_error("Type mismatch: Expected 'bool' for '" + name + "'");
        }
        else
        {
            // Default initialization
            if (type == "int")
                value = 0;
            else if (type == "real")
                value = 0.0;
            else if (type == "string")
                value = std::string("");
            else if (type == "bool")
                value = false;
            else
                throw std::runtime_error("Unsupported type: " + type);
        }

        symbolTable[name] = value;
        return nullptr;
    }

    // Case 2: Array declarations
    else if (auto arrayDeclarationContext = dynamic_cast<BaseParser::ArrayDeclarationContext *>(ctx))
    {
        std::string type = arrayDeclarationContext->type()->getText();
        auto declarator = arrayDeclarationContext->arrayDeclarator();
        std::string name;

        int size1 = -1, size2 = -1;
        bool is2D = false;

        if (auto sized = dynamic_cast<BaseParser::SizedArrayContext *>(declarator))
        {
            name = sized->ID()->getText();
            size1 = std::stoi(sized->INT()->getText());
        }
        else if (auto sized2d = dynamic_cast<BaseParser::Sized2DArrayContext *>(declarator))
        {
            name = sized2d->ID()->getText();
            size1 = std::stoi(sized2d->INT(0)->getText());
            size2 = std::stoi(sized2d->INT(1)->getText());
            is2D = true;
        }
        else if (auto unsized = dynamic_cast<BaseParser::UnsizedArrayContext *>(declarator))
        {
            name = unsized->ID()->getText();
        }

        if (symbolTable.find(name) != symbolTable.end())
        {
            throw std::runtime_error("Array '" + name + "' is already declared.");
        }

        auto initializer = arrayDeclarationContext->arrayInitializer();

        // 1D array
        if (!is2D)
        {
            std::vector<antlrcpp::Any> values;

            if (auto init1D = dynamic_cast<BaseParser::ArrayInit1DContext *>(initializer))
            {
                for (auto exprCtx : init1D->expr())
                {
                    values.push_back(visitExpr(exprCtx));
                }
            }

            if (type == "int")
            {
                std::vector<int> vec;
                for (auto &val : values)
                {
                    if (val.type() == typeid(int))
                        vec.push_back(std::any_cast<int>(val));
                    else if (val.type() == typeid(double))
                        vec.push_back(static_cast<int>(std::any_cast<double>(val)));
                    else
                        throw std::runtime_error("Invalid value in int array '" + name + "'");
                }
                symbolTable[name] = vec;
            }
            else if (type == "real")
            {
                std::vector<double> vec;
                for (auto &val : values)
                {
                    if (val.type() == typeid(double))
                        vec.push_back(std::any_cast<double>(val));
                    else if (val.type() == typeid(int))
                        vec.push_back(static_cast<double>(std::any_cast<int>(val)));
                    else
                        throw std::runtime_error("Invalid value in real array '" + name + "'");
                }
                symbolTable[name] = vec;
            }
            else if (type == "string")
            {
                std::vector<std::string> vec;
                for (auto &val : values)
                {
                    if (val.type() == typeid(std::string))
                        vec.push_back(std::any_cast<std::string>(val));
                    else
                        throw std::runtime_error("Invalid value in string array '" + name + "'");
                }
                symbolTable[name] = vec;
            }
            else if (type == "bool")
            {
                std::vector<bool> vec;
                for (auto &val : values)
                {
                    if (val.type() == typeid(bool))
                        vec.push_back(std::any_cast<bool>(val));
                    else
                        throw std::runtime_error("Invalid value in bool array '" + name + "'");
                }
                symbolTable[name] = vec;
            }
            else
            {
                throw std::runtime_error("Unsupported 1D array type: " + type);
            }
        }

        // 2D array
        else
        {
            auto info = std::any_cast<std::tuple<std::string, int, int, bool>>(symbolTable[name]);
            std::string type = std::get<0>(info);
            int rows = std::get<1>(info);
            int cols = std::get<2>(info);
            bool is2D = std::get<3>(info);
            if (initializer == nullptr)
            {
                // No initializer: create default initialized array based on declared size
                if (rows <= 0 || cols <= 0)
                {
                    throw std::runtime_error("Array size must be specified if no initializer is given for: " + name);
                }

                if (type == "int")
                {
                    symbolTable[name] = std::vector<std::vector<int>>(rows, std::vector<int>(cols, 0));
                }
                else if (type == "real")
                {
                    symbolTable[name] = std::vector<std::vector<double>>(rows, std::vector<double>(cols, 0.0));
                }
                else if (type == "string")
                {
                    symbolTable[name] = std::vector<std::vector<std::string>>(rows, std::vector<std::string>(cols, ""));
                }
                else if (type == "bool")
                {
                    symbolTable[name] = std::vector<std::vector<bool>>(rows, std::vector<bool>(cols, false));
                }
                else
                {
                    throw std::runtime_error("Unsupported type for default-initialized 2D array: " + name);
                }
                return nullptr;
            }

            if (auto init2D = dynamic_cast<BaseParser::ArrayInit2DContext *>(initializer))
            {
                if (init2D->arrayInitializer().empty())
                {
                    // Empty initializer: create empty 2D array
                    if (type == "int")
                        symbolTable[name] = std::vector<std::vector<int>>();
                    else if (type == "real")
                        symbolTable[name] = std::vector<std::vector<double>>();
                    else if (type == "string")
                        symbolTable[name] = std::vector<std::vector<std::string>>();
                    else if (type == "bool")
                        symbolTable[name] = std::vector<std::vector<bool>>();
                    else
                        throw std::runtime_error("Unsupported type for empty 2D array: " + name);

                    return nullptr;
                }

                if (type == "int")
                {
                    std::vector<std::vector<int>> mat;

                    for (auto rowInit : init2D->arrayInitializer())
                    {
                        auto row1D = dynamic_cast<BaseParser::ArrayInit1DContext *>(rowInit);
                        if (!row1D)
                            throw std::runtime_error("Invalid row in 2D initializer for '" + name + "'");

                        std::vector<int> row;
                        for (auto exprCtx : row1D->expr())
                        {
                            auto val = visitExpr(exprCtx);
                            if (val.type() == typeid(int))
                                row.push_back(std::any_cast<int>(val));
                            else if (val.type() == typeid(double))
                                row.push_back(static_cast<int>(std::any_cast<double>(val)));
                            else
                                throw std::runtime_error("Invalid value in int 2D array '" + name + "'");
                        }
                        mat.push_back(row);
                    }

                    symbolTable[name] = mat;
                }
                else if (type == "real")
                {
                    std::vector<std::vector<double>> mat;

                    for (auto rowInit : init2D->arrayInitializer())
                    {
                        auto row1D = dynamic_cast<BaseParser::ArrayInit1DContext *>(rowInit);
                        if (!row1D)
                            throw std::runtime_error("Invalid row in 2D initializer for '" + name + "'");

                        std::vector<double> row;
                        for (auto exprCtx : row1D->expr())
                        {
                            auto val = visitExpr(exprCtx);
                            if (val.type() == typeid(double))
                                row.push_back(std::any_cast<double>(val));
                            else if (val.type() == typeid(int))
                                row.push_back(static_cast<double>(std::any_cast<int>(val)));
                            else
                                throw std::runtime_error("Invalid value in real 2D array '" + name + "'");
                        }
                        mat.push_back(row);
                    }

                    symbolTable[name] = mat;
                }
                else if (type == "string")
                {
                    std::vector<std::vector<std::string>> mat;

                    for (auto rowInit : init2D->arrayInitializer())
                    {
                        auto row1D = dynamic_cast<BaseParser::ArrayInit1DContext *>(rowInit);
                        if (!row1D)
                            throw std::runtime_error("Invalid row in 2D initializer for '" + name + "'");

                        std::vector<std::string> row;
                        for (auto exprCtx : row1D->expr())
                        {
                            auto val = visitExpr(exprCtx);
                            if (val.type() == typeid(std::string))
                                row.push_back(std::any_cast<std::string>(val));
                            else
                                throw std::runtime_error("Invalid value in string 2D array '" + name + "'");
                        }
                        mat.push_back(row);
                    }

                    symbolTable[name] = mat;
                }
                else if (type == "bool")
                {
                    std::vector<std::vector<bool>> mat;

                    for (auto rowInit : init2D->arrayInitializer())
                    {
                        auto row1D = dynamic_cast<BaseParser::ArrayInit1DContext *>(rowInit);
                        if (!row1D)
                            throw std::runtime_error("Invalid row in 2D initializer for '" + name + "'");

                        std::vector<bool> row;
                        for (auto exprCtx : row1D->expr())
                        {
                            auto val = visitExpr(exprCtx);
                            if (val.type() == typeid(bool))
                                row.push_back(std::any_cast<bool>(val));
                            else
                                throw std::runtime_error("Invalid value in bool 2D array '" + name + "'");
                        }
                        mat.push_back(row);
                    }

                    symbolTable[name] = mat;
                }
                else
                {
                    throw std::runtime_error("Only int, real, string, bool 2D arrays supported for now: " + name);
                }
            }
            else
            {
                if (auto init1D = dynamic_cast<BaseParser::ArrayInit1DContext *>(initializer))
                {
                    throw std::runtime_error("Got 1D initializer, but 2D expected: " + name);
                }
                else
                {
                    throw std::runtime_error("Expected 2D initializer for: " + name);
                }
            }
        }

        return nullptr;
    }

    return nullptr;
}

antlrcpp::Any MyVisitor::visitCondition(BaseParser::ConditionContext *ctx)
{
    // std::cout<<"condition working ";
    // return true;
    if (auto andContext = dynamic_cast<BaseParser::LogicalAndContext *>(ctx))
    {
        // Handle "&&"
        bool left = std::any_cast<bool>(visitCondition(andContext->condition(0)));
        bool right = std::any_cast<bool>(visitCondition(andContext->condition(1)));
        return left && right;
    }
    else if (auto orContext = dynamic_cast<BaseParser::LogicalOrContext *>(ctx))
    {
        // Handle "||"
        bool left = std::any_cast<bool>(visitCondition(orContext->condition(0)));
        bool right = std::any_cast<bool>(visitCondition(orContext->condition(1)));
        return left || right;
    }
    else if (auto relationalContext = dynamic_cast<BaseParser::RelationalContext *>(ctx))
    {
        // Handle relational expressions
        // return true;
        auto leftResult = visitExpr(relationalContext->expr(0));
        std::cout << "left result type: " << leftResult.type().name() << "\n";
        // return true;
        auto rightResult = visitExpr(relationalContext->expr(1));
        std::cout << "right result type: " << rightResult.type().name() << "\n";

        // return true;
        int left = std::any_cast<int>(leftResult);
        int right = std::any_cast<int>(rightResult);

        std::cout << "left = " << left << " right= " << right << "\n";

        if (relationalContext->EQUAL())
            return left == right;
        if (relationalContext->NOTEQUAL())
            return left != right;
        if (relationalContext->LESSTHAN())
            return left < right;
        if (relationalContext->GREATERTHAN())
            return left > right;
        if (relationalContext->LESSEQUAL())
            return left <= right;
        if (relationalContext->GREATEREQUAL())
            return left >= right;
    }
    else if (auto nodeInGraphContext = dynamic_cast<BaseParser::NodeCheckContext *>(ctx))
    {
        // Handle "nodeID in graphID"
        node node;
        node.id = nodeInGraphContext->nodeID()->getText();
        std::string gName = nodeInGraphContext->graphID()->getText();
        return nodeExists(gName, node);
    }
    else if (auto edgeInGraphContext = dynamic_cast<BaseParser::EdgeCheckContext *>(ctx))
    {
        // Handle "edge in graphID"
        node from, to;
        from.id = edgeInGraphContext->edge()->nodeID(0)->getText();
        to.id = edgeInGraphContext->edge()->nodeID(1)->getText();
        std::string gName = edgeInGraphContext->graphID()->getText();
        // if (graph == graphName) return edgeExists(from, to);
        return edgeExists(gName, from, to);
    }

    throw std::runtime_error("Invalid condition.");
}

antlrcpp::Any MyVisitor::visitLoopStatement(BaseParser::LoopStatementContext *ctx)
{
    // std::cout<<"nopee";
    // exit(0);
    if (ctx->foreachStatement())
    {
        return visitForeachStatement(ctx->foreachStatement());
    }
    else if (ctx->whileStatement())
    {
        return visitWhileStatement(ctx->whileStatement());
    }
    return nullptr;
}

antlrcpp::Any MyVisitor::visitForeachStatement(BaseParser::ForeachStatementContext *ctx)
{
    std::string graphName = ctx->graphID()->getText();

    // Check if the graph exists
    if (graphs.find(graphName) == graphs.end())
    {
        throw std::runtime_error("Graph '" + graphName + "' not found.");
    }

    const auto &graphData = graphs[graphName];
    auto loopTarget = ctx->loopTarget();

    // Handle "for each vertex"
    if (auto vertexContext = dynamic_cast<BaseParser::ForEachVertexContext *>(loopTarget))
    {
        std::string vertexVar = vertexContext->ID()->getText();
        std::optional<std::any> previousValue = symbolTable.count(vertexVar) ? std::optional<std::any>(symbolTable[vertexVar]) : std::nullopt;

        for (const auto &node : graphData.nodes)
        {
            symbolTable[vertexVar] = node; // Set vertex variable
            visitBlock(ctx->block());      // Execute loop block
        }

        // Restore previous value or remove the variable
        if (previousValue.has_value())
        {
            symbolTable[vertexVar] = previousValue.value();
        }
        else
        {
            symbolTable.erase(vertexVar);
        }

        // Handle "for each edge"
    }
    else if (auto edgeContext = dynamic_cast<BaseParser::ForEachEdgeContext *>(loopTarget))
    {
        std::string fromVar = edgeContext->ID(0)->getText();
        std::string toVar = edgeContext->ID(1)->getText();

        std::optional<std::any> previousFrom = symbolTable.count(fromVar) ? std::optional<std::any>(symbolTable[fromVar]) : std::nullopt;
        std::optional<std::any> previousTo = symbolTable.count(toVar) ? std::optional<std::any>(symbolTable[toVar]) : std::nullopt;

        for (const auto &edge : graphData.edges)
        {

            symbolTable[fromVar] = edge.from; // Set source vertex
            symbolTable[toVar] = edge.to;     // Set target vertex
            visitBlock(ctx->block());         // Execute loop block
        }

        // Restore previous values or remove the variables
        if (previousFrom.has_value())
        {
            symbolTable[fromVar] = previousFrom.value();
        }
        else
        {
            symbolTable.erase(fromVar);
        }

        if (previousTo.has_value())
        {
            symbolTable[toVar] = previousTo.value();
        }
        else
        {
            symbolTable.erase(toVar);
        }

        // Handle "for each neighbor"
    }
    else if (auto adjContext = dynamic_cast<BaseParser::ForEachAdjContext *>(loopTarget))
    {
        // UNDERSTAND THIS!!!!!!!!!!!!!!!!!!!!!!!!
        node node;
        node.id = adjContext->nodeID()->getText();
        std::string neighborVar = adjContext->ID()->getText();

        if (find(graphData.nodes.begin(), graphData.nodes.end(), node) == graphData.nodes.end())
        {
            throw std::runtime_error("Node '" + nodeTypeToString(node.id) + "' not found in graph '" + graphName + "'.");
        }

        std::optional<std::any> previousNeighbor = symbolTable.count(neighborVar) ? std::optional<std::any>(symbolTable[neighborVar]) : std::nullopt;

        for (const auto &neighbor : graphData.adjacencyList.at(node))
        {
            symbolTable[neighborVar] = neighbor; // Set neighbor variable
            visitBlock(ctx->block());            // Execute loop block
        }

        // Restore previous value or remove the variable
        if (previousNeighbor.has_value())
        {
            symbolTable[neighborVar] = previousNeighbor.value();
        }
        else
        {
            symbolTable.erase(neighborVar);
        }
    }
    else
    {
        throw std::runtime_error("Unknown loop target type.");
    }

    return nullptr;
}

antlrcpp::Any MyVisitor::visitWhileStatement(BaseParser::WhileStatementContext *ctx)
{
    // Evaluate and loop while the condition is true
    while (std::any_cast<bool>(visitCondition(ctx->condition())))
    {
        visitBlock(ctx->block()); // Execute the loop block
    }
    return nullptr;
}

void MyVisitor::addNode(const std::string &gName, node node)
{

    // adjacencyList[node]; // Ensures the node exists

    // if (graph.find(gName) == graph.end())
    // {
    //     graph[gName] = std::unordered_map<int, std::unordered_set<int>>();
    // }
    graphs[gName];
    graphs[gName].addNode(node);
}

void MyVisitor::removeNode(const std::string &gName, node node)
{
    graphs[gName].removeNode(node);
}

void MyVisitor::addEdge(const std::string &gName, node from, node to, std::optional<EdgeType> weight)
{
    // if (graph.find(gName) == graph.end())
    // {
    //
    //     graph[gName] = std::unordered_map<int, std::unordered_set<int>>();
    // }
    // if (graph[gName].find(node) == graph[gName].end())
    // {
    //     graph[gName][node] = std::unordered_set<int>();
    //     std::cout << "Node " << node << " added to graph " << graphName << std::endl;
    // }
    // else
    // {
    //     std::cout << "Node " << node << " already exists in graph " << graphName << std::endl;
    // }
    graphs[gName];
    if (weight.has_value())
    {
        edge e(from, to, weight.value());
        graphs[gName].addEdge(e);
    }
    else
    {
        edge e(from, to);
        graphs[gName].addEdge(e);
    }

    // adjacencyList[from].insert(to);
    // adjacencyList[to].insert(from);
}

bool MyVisitor::nodeExists(const std::string &gName, node node) const
{
    auto it = graphs.find(gName);
    if (it != graphs.end())
    {
        return find(it->second.nodes.begin(), it->second.nodes.end(), node) != it->second.nodes.end();
    }
    return false;
}

// Visit addOperation
antlrcpp::Any MyVisitor::visitAddOperation(BaseParser::AddOperationContext *ctx)
{
    std::string gName = ctx->graphID()->getText();
    if (ctx->addTargets()->nodeID())
    {
        node node;
        node.id = ctx->addTargets()->nodeID()->getText();
        addNode(gName, node);
    }
    else if (ctx->addTargets()->edge())
    {
        node from, to;
        from.id = ctx->addTargets()->edge()->nodeID(0)->getText();
        to.id = ctx->addTargets()->edge()->nodeID(1)->getText();
        addEdge(gName, from, to);
    }
    // Handle nodeList and edgeList if needed
    return nullptr;
}

// Visit removeOperation
antlrcpp::Any MyVisitor::visitRemoveOperation(BaseParser::RemoveOperationContext *ctx)
{
    std::string gName = ctx->graphID()->getText();
    if (ctx->removeTargets()->nodeID())
    {
        node node;
        node.id = ctx->removeTargets()->nodeID()->getText();
        removeNode(gName, node);
    }
    else if (ctx->removeTargets()->edge())
    {
        node from, to;
        from.id = ctx->removeTargets()->edge()->nodeID(0)->getText();
        to.id = ctx->removeTargets()->edge()->nodeID(1)->getText();
        removeEdge(gName, from, to);
    }
    // Handle nodeList and edgeList if needed
    return nullptr;
}

// Remove an edge from the graph
void MyVisitor::removeEdge(const std::string &gName, node from, node to, std::optional<EdgeType> weight)
{
    if (weight.has_value())
    {
        edge e(from, to, weight.value());
        graphs[gName].removeEdge(e);
    }
    else
    {
        edge e(from, to);
        graphs[gName].removeEdge(e);
    }
}

bool MyVisitor::edgeExists(const std::string &gName, node from, node to) const
{
    bool final = false;
    auto it1 = graphs.find(gName);
    if (it1 != graphs.end())
    {
        for (const auto &edge : it1->second.edges)
        {
            if (edge.from.id == from.id && edge.to.id == to.id)
            {
                final = true;
                break;
            }
        }
        return final;
    }
    return false;
}

antlrcpp::Any MyVisitor::visitPrintStatement(BaseParser::PrintStatementContext *ctx)
{
    if (ctx->printExpr())
    {
        // Visit and handle print expressions
        auto result = visitPrintExpr(ctx->printExpr());
        // Print the result if it's not nullptr
        if (result.has_value())
        {
            if (result.type() == typeid(int))
            {
                std::cout << std::any_cast<int>(result) << std::endl;
            }
            else if (result.type() == typeid(double))
            {
                std::cout << std::any_cast<double>(result) << std::endl;
            }
            else if (result.type() == typeid(std::string))
            {
                std::cout << std::any_cast<std::string>(result) << std::endl;
            }
            if (result.type() == typeid(bool))
            {
                std::cout << (std::any_cast<bool>(result) ? "TRUE" : "FALSE") << std::endl;
            }
            else if (result.type() == typeid(std::vector<int>))
            {
                const auto &vec = std::any_cast<const std::vector<int> &>(result);
                std::cout << "[";
                for (size_t i = 0; i < vec.size(); ++i)
                {
                    std::cout << vec[i];
                    if (i < vec.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }
            else if (result.type() == typeid(std::vector<double>))
            {
                const auto &vec = std::any_cast<const std::vector<double> &>(result);
                std::cout << "[";
                for (size_t i = 0; i < vec.size(); ++i)
                {
                    std::cout << vec[i];
                    if (i < vec.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }
            else if (result.type() == typeid(std::vector<std::string>))
            {
                const auto &vec = std::any_cast<const std::vector<std::string> &>(result);
                std::cout << "[";
                for (size_t i = 0; i < vec.size(); ++i)
                {
                    std::cout << vec[i];
                    if (i < vec.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }
            else if (result.type() == typeid(std::vector<bool>))
            {
                const auto &vec = std::any_cast<const std::vector<bool> &>(result);
                std::cout << "[";
                for (size_t i = 0; i < vec.size(); ++i)
                {
                    std::cout << (vec[i] ? "TRUE" : "FALSE");
                    if (i < vec.size() - 1)
                        std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }
        }
        return nullptr;
    }
    else if (ctx->printgraph())
    {
        // Visit and handle graph-related print statements
        return visitPrintgraph(ctx->printgraph());
    }
    return nullptr;
}

antlrcpp::Any MyVisitor::visitPrintExpr(BaseParser::PrintExprContext *ctx)
{
    if (ctx->STRING())
    {
        // Return string literal value
        std::string str = ctx->STRING()->getText();
        // Remove quotes from the string if present
        str = str.substr(1, str.size() - 2);
        return str;
    }
    else if (ctx->expr())
    {
        // Visit and return the evaluated expression result
        return visitExpr(ctx->expr());
    }
    else if (ctx->printExpr(0) && ctx->printExpr(1))
    {
        // Handle concatenation
        auto left = visitPrintExpr(ctx->printExpr(0));
        auto right = visitPrintExpr(ctx->printExpr(1));

        // Convert both to strings and concatenate
        std::string leftStr, rightStr;

        if (left.type() == typeid(int))
        {
            leftStr = std::to_string(std::any_cast<int>(left));
        }
        else if (left.type() == typeid(std::string))
        {
            leftStr = std::any_cast<std::string>(left);
        }
        else if (left.type() == typeid(double))
        {
            leftStr = std::to_string(std::any_cast<double>(left));
        }

        if (right.type() == typeid(int))
        {
            rightStr = std::to_string(std::any_cast<int>(right));
        }
        else if (right.type() == typeid(std::string))
        {
            rightStr = std::any_cast<std::string>(right);
        }
        else if (right.type() == typeid(double))
        {
            rightStr = std::to_string(std::any_cast<double>(right));
        }

        return leftStr + rightStr;
    }
    return nullptr;
}

antlrcpp::Any MyVisitor::visitPrintgraph(BaseParser::PrintgraphContext *ctx)
{
    // std::cout << "hee";
    if (auto edgePrintCtx = dynamic_cast<BaseParser::EdgePrintContext *>(ctx))
    {
        // Handle 'print edges of graphID'
        std::string gName = edgePrintCtx->graphID()->getText();
        std::cout << "Edges of graph: " << gName << std::endl;
        printEdges(gName);
        return nullptr;
    }
    else if (auto nodePrintCtx = dynamic_cast<BaseParser::NodePrintContext *>(ctx))
    {
        // Handle 'print nodes of graphID'
        std::string gName = nodePrintCtx->graphID()->getText();
        std::cout << "Nodes of graph: " << gName << std::endl;
        printNodes(gName);
        return nullptr;
    }
    else if (auto graphPrintCtx = dynamic_cast<BaseParser::GraphPrintContext *>(ctx))
    {
        // Handle 'print graph of graphID'
        std::string gName = graphPrintCtx->graphID()->getText();
        std::cout << "Graph: " << gName << std::endl;
        printGraph(gName);
        return nullptr;
    }

    throw std::runtime_error("Invalid printgraph statement.");
}

void MyVisitor::printNodes(const std::string &gName) const
{
    if (graphs.find(gName) != graphs.end())
    {
        graph g = graphs.at(gName);
        // FIX THIS
        g.printNodes();
    }
}

void MyVisitor::printEdges(const std::string &gName) const
{
    auto it = graphs.find(gName);
    if (it != graphs.end())
    {
        graph g = it->second;
        g.printEdges();
    }
}

void MyVisitor::printGraph(const std::string &gName) const
{
    auto it = graphs.find(gName);
    if (it != graphs.end())
    {
        graph g = it->second;
        g.printGraph();
    }
}

const std::unordered_map<int, std::unordered_set<int>> &MyVisitor::getGraph() const
{
    return adjacencyList;
}

antlrcpp::Any MyVisitor::visitArrayAssignStmt(BaseParser::ArrayAssignStmtContext *ctx)
{
    std::string arrayName = ctx->ID()->getText();
    int index = std::stoi(ctx->INT()->getText()); // index from INT token
    antlrcpp::Any value = visitExpr(ctx->expr()); // value is the expr
    // std::cout << "Visiting expr: " << ctx->expr()->getText() << std::endl;
    if (value.has_value() == false)
        throw std::runtime_error("Visit to expr() returned void or no value!");

    if (symbolTable.find(arrayName) == symbolTable.end())
        throw std::runtime_error("Undefined array: " + arrayName);

    auto &arr = symbolTable[arrayName];

    if (arr.type() == typeid(std::vector<int>))
    {
        auto &vec = std::any_cast<std::vector<int> &>(arr);
        if (index < 0 || index >= vec.size())
            throw std::runtime_error("Array index out of bounds: " + std::to_string(index));

        if (value.type() == typeid(double))
        {
            vec[index] = static_cast<int>(std::any_cast<double>(value));
        }
        else if (value.type() == typeid(int))
        {
            vec[index] = std::any_cast<int>(value);
        }
        else
        {
            throw std::runtime_error(std::string("Type mismatch in array assignment, expected int, got ") + value.type().name());
        }

        symbolTable[arrayName] = vec;
    }
    else if (arr.type() == typeid(std::vector<double>))
    {
        auto &vec = std::any_cast<std::vector<double> &>(arr);
        if (index < 0 || index >= vec.size())
            throw std::runtime_error("Array index out of bounds: " + std::to_string(index));

        if (value.type() == typeid(double))
        {
            vec[index] = std::any_cast<double>(value);
        }
        else if (value.type() == typeid(int))
        {
            vec[index] = static_cast<double>(std::any_cast<int>(value));
        }
        else
        {
            throw std::runtime_error(std::string("Type mismatch in array assignment, expected real/double, got ") + value.type().name());
        }

        symbolTable[arrayName] = vec;
    }
    else if (arr.type() == typeid(std::vector<bool>))
    {
        auto &vec = std::any_cast<std::vector<bool> &>(arr);
        if (index < 0 || index >= vec.size())
            throw std::runtime_error("Array index out of bounds: " + std::to_string(index));

        if (value.type() == typeid(bool))
        {
            vec[index] = std::any_cast<bool>(value);
        }
        else
        {
            throw std::runtime_error(std::string("Type mismatch in array assignment, expected bool, got ") + value.type().name());
        }

        symbolTable[arrayName] = vec;
    }
    else if (arr.type() == typeid(std::vector<std::string>))
    {
        auto &vec = std::any_cast<std::vector<std::string> &>(arr);
        if (index < 0 || index >= vec.size())
            throw std::runtime_error("Array index out of bounds: " + std::to_string(index));

        if (value.type() == typeid(std::string))
        {
            vec[index] = std::any_cast<std::string>(value);
        }
        else
        {
            throw std::runtime_error(std::string("Type mismatch in array assignment, expected string, got ") + value.type().name());
        }

        symbolTable[arrayName] = vec;
    }
    else
    {
        throw std::runtime_error("Variable is not an array: " + arrayName);
    }

    return nullptr;
}

antlrcpp::Any MyVisitor::visitArray2DAssignStmt(BaseParser::Array2DAssignStmtContext *ctx)
{
    std::string name = ctx->ID()->getText();

    int row = std::stoi(ctx->INT(0)->getText());
    int col = std::stoi(ctx->INT(1)->getText());

    std::any value = visit(ctx->expr()); // only one expr — the RHS

    std::string key = name + "[" + std::to_string(row) + "][" + std::to_string(col) + "]";
    symbolTable[key] = value;

    return nullptr;
}

antlrcpp::Any MyVisitor::visitArray2DAccessExpr(BaseParser::Array2DAccessExprContext *ctx)
{
    std::string name = ctx->ID()->getText();
    auto exprs = ctx->expr(); // Returns std::vector<ExprContext*>

    if (exprs.size() != 2)
        throw std::runtime_error("2D array access requires exactly two indices.");

    int row = std::any_cast<int>(visit(exprs[0]));
    int col = std::any_cast<int>(visit(exprs[1]));

    std::string key = name + "[" + std::to_string(row) + "][" + std::to_string(col) + "]";
    if (symbolTable.count(key))
        return symbolTable[key];

    throw std::runtime_error("2D array access out of bounds: " + key);
}

antlrcpp::Any MyVisitor::visitArrayInit2D(BaseParser::ArrayInit2DContext *ctx)
{
    // TODO: Implement logic
    return visitChildren(ctx);
}

antlrcpp::Any MyVisitor::visitSized2DArray(BaseParser::Sized2DArrayContext *ctx)
{
    // TODO: Implement logic
    return visitChildren(ctx);
}

antlrcpp::Any MyVisitor::visitUnsized2DArray(BaseParser::Unsized2DArrayContext *ctx)
{
    // TODO: Implement logic
    return visitChildren(ctx);
}

antlrcpp::Any MyVisitor::visitAssignmentStatement(BaseParser::AssignmentStatementContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    std::any varType = symbolTable[varName];
    // antlrcpp::Any v = visit(ctx->expr());

    std::any value;
    if (ctx->expr())
    {
        value = visitExpr(ctx->expr());
        // std::cout << "Value type: " << value.type().name() << std::endl;
        // std::cout << "Variable type: " << varType.type().name() << std::endl;

        // Type checking directly with typeid
        if (varType.type() == typeid(int) && value.type() != typeid(int))
        {
            throw std::runtime_error("Type mismatch: Expected 'int' initializer for variable '" + varName + "'.");
        }
        else if (varType.type() == typeid(std::string) && value.type() != typeid(std::string))
        {
            throw std::runtime_error("Type mismatch: Expected 'string' initializer for variable '" + varName + "'.");
        }
        else if (varType.type() == typeid(double) && value.type() != typeid(double))
        {
            throw std::runtime_error("Type mismatch: Expected 'real' initializer for variable '" + varName + "'.");
        }
        else if (varType.type() == typeid(bool) && value.type() != typeid(bool))
        {
            throw std::runtime_error("Type mismatch: Expected 'bool' initializer for variable '" + varName + "'.");
        }
    }
    else
    {
        value = symbolTable[varName];
    }

    // Assuming you have some symbol table like:
    symbolTable[varName] = value;

    return nullptr;
}

antlrcpp::Any MyVisitor::visitDeclaration(BaseParser::DeclarationContext *ctx)
{
    std::string varName;
    int rows = -1, cols = -1;
    bool is2D = false;
    std::string type = ctx->type()->getText();

    if (ctx->arrayDeclarator())
    {
        auto arrDec = ctx->arrayDeclarator();
        if (auto sized2D = dynamic_cast<BaseParser::Sized2DArrayContext *>(arrDec))
        {
            varName = sized2D->ID()->getText();
            rows = std::stoi(sized2D->INT(0)->getText());
            cols = std::stoi(sized2D->INT(1)->getText());
            is2D = true;
        }
        else if (auto sized1D = dynamic_cast<BaseParser::SizedArrayContext *>(arrDec))
        {
            varName = sized1D->ID()->getText();
            rows = std::stoi(sized1D->INT()->getText());
            cols = 1;
            is2D = false;
        }
        else if (auto unsized2D = dynamic_cast<BaseParser::Unsized2DArrayContext *>(arrDec))
        {
            varName = unsized2D->ID()->getText();
            is2D = true;
        }
        else if (auto unsized1D = dynamic_cast<BaseParser::UnsizedArrayContext *>(arrDec))
        {
            varName = unsized1D->ID()->getText();
            is2D = false;
        }

        // varName = arrDec->ID()->getText();

        // if (auto sized2D = dynamic_cast<MyParser::Sized2DArrayContext *>(arrDec))
        // {
        //     rows = std::stoi(sized2D->INT(0)->getText());
        //     cols = std::stoi(sized2D->INT(1)->getText());
        //     is2D = true;
        // }
        // else if (auto sized1D = dynamic_cast<MyParser::SizedArrayContext *>(arrDec))
        // {
        //     rows = std::stoi(sized1D->INT()->getText());
        //     // 1D array, cols = -1 or 1
        //     cols = 1;
        //     is2D = false;
        // }
        // else
        // {
        //     // unsized arrays: rows and cols stay -1
        // }

        // Store in symbol table (example)
        // symbolTable[varName] = VariableInfo{type, rows, cols, ...};
        symbolTable[varName] = std::make_any<std::tuple<std::string, int, int, bool>>(type, rows, cols, is2D);
    }
    else
    {
        varName = ctx->ID()->getText();
        // scalar variable handling...
    }

    // Then you can pass these sizes to your initializer handler:
    // if (ctx->arrayInitializer())
    // {
    //     visitArrayInitializer(varName, type, rows, cols, ctx->arrayInitializer());
    // }
    // else
    // {
    //     // No initializer, can create default array using rows and cols
    //     if (is2D && rows > 0 && cols > 0)
    //     {
    //         createDefault2DArray(varName, type, rows, cols);
    //     }
    //     else
    //     {
    //         // handle other cases or throw error
    //     }
    // }

    return nullptr;
}
