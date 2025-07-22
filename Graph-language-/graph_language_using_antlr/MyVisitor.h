#ifndef MYVISITOR_H
#define MYVISITOR_H

#include "generated/BaseBaseVisitor.h"
#include "generated/BaseParser.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <stdexcept>
#include <any>


class MyVisitor : public BaseBaseVisitor {
public:

    

    antlrcpp::Any visitProgram(BaseParser::ProgramContext *ctx) override;
    antlrcpp::Any visitStatement(BaseParser::StatementContext *ctx) override;
    // Graph-related methods
    antlrcpp::Any visitGraphDef(BaseParser::GraphDefContext *ctx) override;
    antlrcpp::Any visitNodes(BaseParser::NodesContext *ctx) override;
    antlrcpp::Any visitEdges(BaseParser::EdgesContext *ctx) override;

    // Conditional-related methods
    
    antlrcpp::Any visitConditionalStatement(BaseParser::ConditionalStatementContext *ctx) override;
    antlrcpp::Any visitCondition(BaseParser::ConditionContext *ctx);
    antlrcpp::Any visitBlock(BaseParser::BlockContext *ctx) override;
    antlrcpp::Any visitReturnStatement(BaseParser::ReturnStatementContext *ctx) override;

    // graph condition
    //antlrcpp::Any visitGraphCondition(BaseParser::GraphConditionContext *ctx);
    antlrcpp::Any visitGraphComprehension(BaseParser::GraphComprehensionContext *ctx) override;

    bool evaluateGraphCondition(int node, const std::string& gName, BaseParser::GraphConditionContext* ctx);

    bool evaluateDegreeCondition(int node, const std::string& gName, const std::string& operatorStr, int value);

    bool evaluateConnectedCondition(const std::string& gName, int node, int targetNode);

    //query
    antlrcpp::Any visitQueryStatement(BaseParser::QueryStatementContext *ctx) override;

    // printing
    antlrcpp::Any visitPrintStatement(BaseParser::PrintStatementContext *ctx) override;
    antlrcpp::Any visitPrintgraph(BaseParser::PrintgraphContext *ctx);
    antlrcpp::Any visitPrintExpr(BaseParser::PrintExprContext *ctx) override;
    // antlrcpp::Any visitLogicalAnd(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitLogicaLOr(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitRelational(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitNodeCheck(BaseParser::ParenExprContext *ctx) override;
    // antlrcpp::Any visitEdgeCheck(BaseParser::ParenExprContext *ctx) override;

    // Expression-related methods
    antlrcpp::Any visitExpr(BaseParser::ExprContext *ctx);
    // antlrcpp::Any visitMulDivExpr(BaseParser::MulDivExprContext *ctx) override;
    // antlrcpp::Any visitIdExpr(BaseParser::IdExprContext *ctx) override;
    // antlrcpp::Any visitIntExpr(BaseParser::IntExprContext *ctx) override;
    // antlrcpp::Any visitParenExpr(BaseParser::ParenExprContext *ctx) override;
    std::string bfs(const std::unordered_map<int, std::unordered_set<int>>& graph);
    std::string detectCycle(const std::unordered_map<int, std::unordered_set<int>>& graph);
    bool detectCycleHelper(int node, int parent, const std::unordered_map<int, std::unordered_set<int>>& graph, 
                       std::unordered_set<int>& visited);

    antlrcpp::Any visitShowgraph(BaseParser::ShowgraphContext *ctx) override;


    void generateDotFile(const std::unordered_map<int, std::unordered_set<int>>& graph, const std::string& filename);


    void showGraph(const std::string& graphID);
    //  funcitons
    antlrcpp::Any visitFunction(BaseParser::FunctionContext *ctx) override;
    antlrcpp::Any visitParamList(BaseParser::ParamListContext *ctx) override;
    antlrcpp::Any visitParam(BaseParser::ParamContext *ctx) override;
    antlrcpp::Any visitFunctionCall(BaseParser::FunctionCallContext *ctx) override;


    // loops
    antlrcpp::Any visitLoopStatement(BaseParser::LoopStatementContext *ctx) override;
    antlrcpp::Any visitForeachStatement(BaseParser::ForeachStatementContext *ctx) override;
    //antlrcpp::Any visitLoopTarget(BaseParser::LoopTargetContext *ctx);
    antlrcpp::Any visitWhileStatement(BaseParser::WhileStatementContext *ctx) override;

    //assignment
    antlrcpp::Any visitVarDecl(BaseParser::VarDeclContext *ctx) override;

    // Utility methods for graph management
    void addNode(const std::string &gName,int node);
    void addEdge(const std::string &gName,int from, int to);
    bool nodeExists(const std::string &gName,int node) const;
    bool edgeExists(const std::string &gName,int from, int to) const;

    // Get the graph as an adjacency list
    const std::unordered_map<int, std::unordered_set<int>>& getGraph() const;

    // Print graph information
    void printNodes(const std::string &gName) const;
    void printEdges(const std::string &gName) const;
    void printGraph(const std::string &gName) const;

private:
    std::unordered_map<std::string, std::any> symbolTable;
    std::unordered_map<int, std::unordered_set<int>> adjacencyList;
    std::unordered_map<std::string,std::unordered_map<int, std::unordered_set<int>>> graph;


    struct FunctionDefinition {
        std::string returnType;
        std::vector<std::pair<std::string, std::string>> parameters; // Pair of type and name
        BaseParser::BlockContext *block; // AST block for the function body
    };

    std::unordered_map<std::string, FunctionDefinition> functions;



    std::string graphName;
};

#endif // MYVISITOR_H
