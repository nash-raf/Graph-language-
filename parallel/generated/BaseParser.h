
// Generated from Base.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  BaseParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, GRAPH = 42, SET = 43, UNION = 44, 
    INTERSECT = 45, EDGE = 46, NODE = 47, TRUE = 48, FALSE = 49, OF = 50, 
    PLUS = 51, MINUS = 52, TIMES = 53, DIVIDE = 54, AND = 55, OR = 56, EQUAL = 57, 
    NOTEQUAL = 58, LESSTHAN = 59, GREATERTHAN = 60, LESSEQUAL = 61, GREATEREQUAL = 62, 
    ID = 63, INT = 64, REAL = 65, STRING = 66, Comment = 67, WS = 68
  };

  enum {
    RuleProgram = 0, RuleStatement = 1, RuleGraphDef = 2, RuleNodes = 3, 
    RuleEdges = 4, RuleNodeList = 5, RuleEdgeList = 6, RuleGraphID = 7, 
    RuleNodeID = 8, RuleFileEdgeList = 9, RuleEdge = 10, RuleVarDecl = 11, 
    RuleSetDecl = 12, RuleSetInitializer = 13, RuleSetOperation = 14, RuleSetExpr = 15, 
    RuleGraphComprehension = 16, RuleGraphCondition = 17, RuleConditionalStatement = 18, 
    RuleCondition = 19, RuleLoopStatement = 20, RuleForeachStatement = 21, 
    RuleLoopTarget = 22, RuleWhileStatement = 23, RuleNodeEdgeOperation = 24, 
    RuleAddOperation = 25, RuleRemoveOperation = 26, RuleAddTargets = 27, 
    RuleRemoveTargets = 28, RuleQueryStatement = 29, RuleShowgraph = 30, 
    RuleFunction = 31, RuleReturnType = 32, RuleParamList = 33, RuleParam = 34, 
    RuleType = 35, RuleFunctionCall = 36, RuleArgumentList = 37, RuleBlock = 38, 
    RuleReturnStatement = 39, RulePrintStatement = 40, RulePrintExpr = 41, 
    RulePrintgraph = 42, RuleExpr = 43, RuleArrayDeclarator = 44, RuleArrayInitializer = 45, 
    RuleAssignmentStatement = 46, RuleArrayAssignStatement = 47
  };

  explicit BaseParser(antlr4::TokenStream *input);

  BaseParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~BaseParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class ProgramContext;
  class StatementContext;
  class GraphDefContext;
  class NodesContext;
  class EdgesContext;
  class NodeListContext;
  class EdgeListContext;
  class GraphIDContext;
  class NodeIDContext;
  class FileEdgeListContext;
  class EdgeContext;
  class VarDeclContext;
  class SetDeclContext;
  class SetInitializerContext;
  class SetOperationContext;
  class SetExprContext;
  class GraphComprehensionContext;
  class GraphConditionContext;
  class ConditionalStatementContext;
  class ConditionContext;
  class LoopStatementContext;
  class ForeachStatementContext;
  class LoopTargetContext;
  class WhileStatementContext;
  class NodeEdgeOperationContext;
  class AddOperationContext;
  class RemoveOperationContext;
  class AddTargetsContext;
  class RemoveTargetsContext;
  class QueryStatementContext;
  class ShowgraphContext;
  class FunctionContext;
  class ReturnTypeContext;
  class ParamListContext;
  class ParamContext;
  class TypeContext;
  class FunctionCallContext;
  class ArgumentListContext;
  class BlockContext;
  class ReturnStatementContext;
  class PrintStatementContext;
  class PrintExprContext;
  class PrintgraphContext;
  class ExprContext;
  class ArrayDeclaratorContext;
  class ArrayInitializerContext;
  class AssignmentStatementContext;
  class ArrayAssignStatementContext; 

  class  ProgramContext : public antlr4::ParserRuleContext {
  public:
    ProgramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);
    std::vector<FunctionContext *> function();
    FunctionContext* function(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramContext* program();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GraphDefContext *graphDef();
    ConditionalStatementContext *conditionalStatement();
    PrintStatementContext *printStatement();
    WhileStatementContext *whileStatement();
    VarDeclContext *varDecl();
    SetDeclContext *setDecl();
    FunctionCallContext *functionCall();
    GraphComprehensionContext *graphComprehension();
    ArrayAssignStatementContext *arrayAssignStatement();
    AssignmentStatementContext *assignmentStatement();
    QueryStatementContext *queryStatement();
    ShowgraphContext *showgraph();
    NodeEdgeOperationContext *nodeEdgeOperation();
    SetOperationContext *setOperation();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  GraphDefContext : public antlr4::ParserRuleContext {
  public:
    GraphDefContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GRAPH();
    GraphIDContext *graphID();
    NodesContext *nodes();
    EdgesContext *edges();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphDefContext* graphDef();

  class  NodesContext : public antlr4::ParserRuleContext {
  public:
    NodesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NodeListContext *nodeList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NodesContext* nodes();

  class  EdgesContext : public antlr4::ParserRuleContext {
  public:
    EdgesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    EdgeListContext *edgeList();
    FileEdgeListContext *fileEdgeList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EdgesContext* edges();

  class  NodeListContext : public antlr4::ParserRuleContext {
  public:
    NodeListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<NodeIDContext *> nodeID();
    NodeIDContext* nodeID(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NodeListContext* nodeList();

  class  EdgeListContext : public antlr4::ParserRuleContext {
  public:
    EdgeListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<EdgeContext *> edge();
    EdgeContext* edge(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EdgeListContext* edgeList();

  class  GraphIDContext : public antlr4::ParserRuleContext {
  public:
    GraphIDContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphIDContext* graphID();

  class  NodeIDContext : public antlr4::ParserRuleContext {
  public:
    NodeIDContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NodeIDContext* nodeID();

  class  FileEdgeListContext : public antlr4::ParserRuleContext {
  public:
    FileEdgeListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FileEdgeListContext* fileEdgeList();

  class  EdgeContext : public antlr4::ParserRuleContext {
  public:
    EdgeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<NodeIDContext *> nodeID();
    NodeIDContext* nodeID(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EdgeContext* edge();

  class  VarDeclContext : public antlr4::ParserRuleContext {
  public:
    VarDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    VarDeclContext() = default;
    void copyFrom(VarDeclContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  SimpleDeclarationContext : public VarDeclContext {
  public:
    SimpleDeclarationContext(VarDeclContext *ctx);

    TypeContext *type();
    antlr4::tree::TerminalNode *ID();
    ExprContext *expr();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ArrayDeclarationContext : public VarDeclContext {
  public:
    ArrayDeclarationContext(VarDeclContext *ctx);

    TypeContext *type();
    ArrayDeclaratorContext *arrayDeclarator();
    ArrayInitializerContext *arrayInitializer();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  VarDeclContext* varDecl();

  class  SetDeclContext : public antlr4::ParserRuleContext {
  public:
    SetDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SET();
    antlr4::tree::TerminalNode *ID();
    SetInitializerContext *setInitializer();
    SetExprContext *setExpr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SetDeclContext* setDecl();

  class  SetInitializerContext : public antlr4::ParserRuleContext {
  public:
    SetInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SetInitializerContext* setInitializer();

  class  SetOperationContext : public antlr4::ParserRuleContext {
  public:
    SetOperationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    SetExprContext *setExpr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SetOperationContext* setOperation();

  class  SetExprContext : public antlr4::ParserRuleContext {
  public:
    SetExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    SetExprContext() = default;
    void copyFrom(SetExprContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  SetUnionContext : public SetExprContext {
  public:
    SetUnionContext(SetExprContext *ctx);

    std::vector<SetExprContext *> setExpr();
    SetExprContext* setExpr(size_t i);
    antlr4::tree::TerminalNode *UNION();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SetIntersectContext : public SetExprContext {
  public:
    SetIntersectContext(SetExprContext *ctx);

    std::vector<SetExprContext *> setExpr();
    SetExprContext* setExpr(size_t i);
    antlr4::tree::TerminalNode *INTERSECT();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SetIdContext : public SetExprContext {
  public:
    SetIdContext(SetExprContext *ctx);

    antlr4::tree::TerminalNode *ID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SetLiteralContext : public SetExprContext {
  public:
    SetLiteralContext(SetExprContext *ctx);

    SetInitializerContext *setInitializer();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ParenSetContext : public SetExprContext {
  public:
    ParenSetContext(SetExprContext *ctx);

    SetExprContext *setExpr();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  SetExprContext* setExpr();
  SetExprContext* setExpr(int precedence);
  class  GraphComprehensionContext : public antlr4::ParserRuleContext {
  public:
    GraphComprehensionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    GraphIDContext *graphID();
    GraphConditionContext *graphCondition();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GraphComprehensionContext* graphComprehension();

  class  GraphConditionContext : public antlr4::ParserRuleContext {
  public:
    GraphConditionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    GraphConditionContext() = default;
    void copyFrom(GraphConditionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  GraphLogicalAndContext : public GraphConditionContext {
  public:
    GraphLogicalAndContext(GraphConditionContext *ctx);

    std::vector<GraphConditionContext *> graphCondition();
    GraphConditionContext* graphCondition(size_t i);
    antlr4::tree::TerminalNode *AND();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  DegreeConditionContext : public GraphConditionContext {
  public:
    DegreeConditionContext(GraphConditionContext *ctx);

    antlr4::tree::TerminalNode *INT();
    antlr4::tree::TerminalNode *EQUAL();
    antlr4::tree::TerminalNode *NOTEQUAL();
    antlr4::tree::TerminalNode *LESSEQUAL();
    antlr4::tree::TerminalNode *GREATEREQUAL();
    antlr4::tree::TerminalNode *LESSTHAN();
    antlr4::tree::TerminalNode *GREATERTHAN();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ConnectedConditionContext : public GraphConditionContext {
  public:
    ConnectedConditionContext(GraphConditionContext *ctx);

    NodeIDContext *nodeID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  GraphLogicalOrContext : public GraphConditionContext {
  public:
    GraphLogicalOrContext(GraphConditionContext *ctx);

    std::vector<GraphConditionContext *> graphCondition();
    GraphConditionContext* graphCondition(size_t i);
    antlr4::tree::TerminalNode *OR();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ParenGraphConditionContext : public GraphConditionContext {
  public:
    ParenGraphConditionContext(GraphConditionContext *ctx);

    GraphConditionContext *graphCondition();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  GraphConditionContext* graphCondition();
  GraphConditionContext* graphCondition(int precedence);
  class  ConditionalStatementContext : public antlr4::ParserRuleContext {
  public:
    ConditionalStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConditionContext *condition();
    std::vector<BlockContext *> block();
    BlockContext* block(size_t i);
    ConditionalStatementContext *conditionalStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConditionalStatementContext* conditionalStatement();

  class  ConditionContext : public antlr4::ParserRuleContext {
  public:
    ConditionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ConditionContext() = default;
    void copyFrom(ConditionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  NodeCheckContext : public ConditionContext {
  public:
    NodeCheckContext(ConditionContext *ctx);

    NodeIDContext *nodeID();
    GraphIDContext *graphID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LogicalAndContext : public ConditionContext {
  public:
    LogicalAndContext(ConditionContext *ctx);

    std::vector<ConditionContext *> condition();
    ConditionContext* condition(size_t i);
    antlr4::tree::TerminalNode *AND();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  RelationalContext : public ConditionContext {
  public:
    RelationalContext(ConditionContext *ctx);

    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *EQUAL();
    antlr4::tree::TerminalNode *NOTEQUAL();
    antlr4::tree::TerminalNode *LESSEQUAL();
    antlr4::tree::TerminalNode *GREATEREQUAL();
    antlr4::tree::TerminalNode *LESSTHAN();
    antlr4::tree::TerminalNode *GREATERTHAN();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EdgeCheckContext : public ConditionContext {
  public:
    EdgeCheckContext(ConditionContext *ctx);

    EdgeContext *edge();
    GraphIDContext *graphID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LogicalOrContext : public ConditionContext {
  public:
    LogicalOrContext(ConditionContext *ctx);

    std::vector<ConditionContext *> condition();
    ConditionContext* condition(size_t i);
    antlr4::tree::TerminalNode *OR();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ConditionContext* condition();
  ConditionContext* condition(int precedence);
  class  LoopStatementContext : public antlr4::ParserRuleContext {
  public:
    LoopStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ForeachStatementContext *foreachStatement();
    WhileStatementContext *whileStatement();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LoopStatementContext* loopStatement();

  class  ForeachStatementContext : public antlr4::ParserRuleContext {
  public:
    ForeachStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LoopTargetContext *loopTarget();
    GraphIDContext *graphID();
    BlockContext *block();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForeachStatementContext* foreachStatement();

  class  LoopTargetContext : public antlr4::ParserRuleContext {
  public:
    LoopTargetContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    LoopTargetContext() = default;
    void copyFrom(LoopTargetContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ForEachVertexContext : public LoopTargetContext {
  public:
    ForEachVertexContext(LoopTargetContext *ctx);

    antlr4::tree::TerminalNode *ID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ForEachAdjContext : public LoopTargetContext {
  public:
    ForEachAdjContext(LoopTargetContext *ctx);

    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *OF();
    NodeIDContext *nodeID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ForEachEdgeContext : public LoopTargetContext {
  public:
    ForEachEdgeContext(LoopTargetContext *ctx);

    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  LoopTargetContext* loopTarget();

  class  WhileStatementContext : public antlr4::ParserRuleContext {
  public:
    WhileStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConditionContext *condition();
    BlockContext *block();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  WhileStatementContext* whileStatement();

  class  NodeEdgeOperationContext : public antlr4::ParserRuleContext {
  public:
    NodeEdgeOperationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AddOperationContext *addOperation();
    RemoveOperationContext *removeOperation();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NodeEdgeOperationContext* nodeEdgeOperation();

  class  AddOperationContext : public antlr4::ParserRuleContext {
  public:
    AddOperationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AddTargetsContext *addTargets();
    GraphIDContext *graphID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AddOperationContext* addOperation();

  class  RemoveOperationContext : public antlr4::ParserRuleContext {
  public:
    RemoveOperationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    RemoveTargetsContext *removeTargets();
    GraphIDContext *graphID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RemoveOperationContext* removeOperation();

  class  AddTargetsContext : public antlr4::ParserRuleContext {
  public:
    AddTargetsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NodeIDContext *nodeID();
    EdgeContext *edge();
    NodeListContext *nodeList();
    EdgeListContext *edgeList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AddTargetsContext* addTargets();

  class  RemoveTargetsContext : public antlr4::ParserRuleContext {
  public:
    RemoveTargetsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NodeIDContext *nodeID();
    EdgeContext *edge();
    NodeListContext *nodeList();
    EdgeListContext *edgeList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  RemoveTargetsContext* removeTargets();

  class  QueryStatementContext : public antlr4::ParserRuleContext {
  public:
    QueryStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *OF();
    GraphIDContext *graphID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  QueryStatementContext* queryStatement();

  class  ShowgraphContext : public antlr4::ParserRuleContext {
  public:
    ShowgraphContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    GraphIDContext *graphID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ShowgraphContext* showgraph();

  class  FunctionContext : public antlr4::ParserRuleContext {
  public:
    FunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ReturnTypeContext *returnType();
    antlr4::tree::TerminalNode *ID();
    ParamListContext *paramList();
    BlockContext *block();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionContext* function();

  class  ReturnTypeContext : public antlr4::ParserRuleContext {
  public:
    ReturnTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GRAPH();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReturnTypeContext* returnType();

  class  ParamListContext : public antlr4::ParserRuleContext {
  public:
    ParamListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ParamContext *> param();
    ParamContext* param(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParamListContext* paramList();

  class  ParamContext : public antlr4::ParserRuleContext {
  public:
    ParamContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParamContext* param();

  class  TypeContext : public antlr4::ParserRuleContext {
  public:
    TypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GRAPH();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeContext* type();

  class  FunctionCallContext : public antlr4::ParserRuleContext {
  public:
    FunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    ArgumentListContext *argumentList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionCallContext* functionCall();

  class  ArgumentListContext : public antlr4::ParserRuleContext {
  public:
    ArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArgumentListContext* argumentList();

  class  BlockContext : public antlr4::ParserRuleContext {
  public:
    BlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);
    std::vector<ReturnStatementContext *> returnStatement();
    ReturnStatementContext* returnStatement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockContext* block();

  class  ReturnStatementContext : public antlr4::ParserRuleContext {
  public:
    ReturnStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExprContext *expr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReturnStatementContext* returnStatement();

  class  PrintStatementContext : public antlr4::ParserRuleContext {
  public:
    PrintStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PrintExprContext *printExpr();
    PrintgraphContext *printgraph();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrintStatementContext* printStatement();

  class  PrintExprContext : public antlr4::ParserRuleContext {
  public:
    PrintExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING();
    ExprContext *expr();
    std::vector<PrintExprContext *> printExpr();
    PrintExprContext* printExpr(size_t i);
    antlr4::tree::TerminalNode *PLUS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrintExprContext* printExpr();
  PrintExprContext* printExpr(int precedence);
  class  PrintgraphContext : public antlr4::ParserRuleContext {
  public:
    PrintgraphContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    PrintgraphContext() = default;
    void copyFrom(PrintgraphContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  NodePrintContext : public PrintgraphContext {
  public:
    NodePrintContext(PrintgraphContext *ctx);

    antlr4::tree::TerminalNode *NODE();
    antlr4::tree::TerminalNode *OF();
    GraphIDContext *graphID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  GraphPrintContext : public PrintgraphContext {
  public:
    GraphPrintContext(PrintgraphContext *ctx);

    antlr4::tree::TerminalNode *GRAPH();
    antlr4::tree::TerminalNode *OF();
    GraphIDContext *graphID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EdgePrintContext : public PrintgraphContext {
  public:
    EdgePrintContext(PrintgraphContext *ctx);

    antlr4::tree::TerminalNode *EDGE();
    antlr4::tree::TerminalNode *OF();
    GraphIDContext *graphID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  PrintgraphContext* printgraph();

  class  ExprContext : public antlr4::ParserRuleContext {
  public:
    ExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExprContext() = default;
    void copyFrom(ExprContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  BoolTrueExprContext : public ExprContext {
  public:
    BoolTrueExprContext(ExprContext *ctx);

    antlr4::tree::TerminalNode *TRUE();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MulDivExprContext : public ExprContext {
  public:
    MulDivExprContext(ExprContext *ctx);

    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *TIMES();
    antlr4::tree::TerminalNode *DIVIDE();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IdExprContext : public ExprContext {
  public:
    IdExprContext(ExprContext *ctx);

    antlr4::tree::TerminalNode *ID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BoolFalseExprContext : public ExprContext {
  public:
    BoolFalseExprContext(ExprContext *ctx);

    antlr4::tree::TerminalNode *FALSE();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ArrayAccessExprContext : public ExprContext {
  public:
    ArrayAccessExprContext(ExprContext *ctx);

    antlr4::tree::TerminalNode *ID();
    ExprContext *expr();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SetLitExprContext : public ExprContext {
  public:
    SetLitExprContext(ExprContext *ctx);

    SetInitializerContext *setInitializer();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IntExprContext : public ExprContext {
  public:
    IntExprContext(ExprContext *ctx);

    antlr4::tree::TerminalNode *INT();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ParenExprContext : public ExprContext {
  public:
    ParenExprContext(ExprContext *ctx);

    ExprContext *expr();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FuncExprContext : public ExprContext {
  public:
    FuncExprContext(ExprContext *ctx);

    FunctionCallContext *functionCall();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  RealExprContext : public ExprContext {
  public:
    RealExprContext(ExprContext *ctx);

    antlr4::tree::TerminalNode *REAL();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AddSubExprContext : public ExprContext {
  public:
    AddSubExprContext(ExprContext *ctx);

    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    antlr4::tree::TerminalNode *PLUS();
    antlr4::tree::TerminalNode *MINUS();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ExprContext* expr();
  ExprContext* expr(int precedence);
  class  ArrayDeclaratorContext : public antlr4::ParserRuleContext {
  public:
    ArrayDeclaratorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ArrayDeclaratorContext() = default;
    void copyFrom(ArrayDeclaratorContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  SizedArrayContext : public ArrayDeclaratorContext {
  public:
    SizedArrayContext(ArrayDeclaratorContext *ctx);

    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *INT();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  UnsizedArrayContext : public ArrayDeclaratorContext {
  public:
    UnsizedArrayContext(ArrayDeclaratorContext *ctx);

    antlr4::tree::TerminalNode *ID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ArrayDeclaratorContext* arrayDeclarator();

  class  ArrayInitializerContext : public antlr4::ParserRuleContext {
  public:
    ArrayInitializerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArrayInitializerContext* arrayInitializer();

  class  AssignmentStatementContext : public antlr4::ParserRuleContext {
  public:
    AssignmentStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    ExprContext *expr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignmentStatementContext* assignmentStatement();

  class  ArrayAssignStatementContext : public antlr4::ParserRuleContext {
  public:
    ArrayAssignStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ArrayAssignStatementContext() = default;
    void copyFrom(ArrayAssignStatementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ArrayAssignStmtContext : public ArrayAssignStatementContext {
  public:
    ArrayAssignStmtContext(ArrayAssignStatementContext *ctx);

    antlr4::tree::TerminalNode *ID();
    std::vector<ExprContext *> expr();
    ExprContext* expr(size_t i);
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ArrayAssignStatementContext* arrayAssignStatement();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool setExprSempred(SetExprContext *_localctx, size_t predicateIndex);
  bool graphConditionSempred(GraphConditionContext *_localctx, size_t predicateIndex);
  bool conditionSempred(ConditionContext *_localctx, size_t predicateIndex);
  bool printExprSempred(PrintExprContext *_localctx, size_t predicateIndex);
  bool exprSempred(ExprContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

