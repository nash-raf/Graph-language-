// Generated from /Users/washioferdousrubai/Documents/VS code/cpp/graph language/Graph-language-/gpl/Base.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast", "CheckReturnValue"})
public class BaseParser extends Parser {
	static { RuntimeMetaData.checkVersion("4.13.1", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, T__7=8, T__8=9, 
		T__9=10, T__10=11, T__11=12, T__12=13, T__13=14, T__14=15, T__15=16, T__16=17, 
		T__17=18, T__18=19, T__19=20, T__20=21, T__21=22, T__22=23, T__23=24, 
		T__24=25, T__25=26, T__26=27, T__27=28, T__28=29, T__29=30, T__30=31, 
		T__31=32, T__32=33, T__33=34, T__34=35, T__35=36, T__36=37, T__37=38, 
		T__38=39, T__39=40, GRAPH=41, EDGE=42, NODE=43, OF=44, PLUS=45, MINUS=46, 
		TIMES=47, DIVIDE=48, AND=49, OR=50, EQUAL=51, NOTEQUAL=52, LESSTHAN=53, 
		GREATERTHAN=54, LESSEQUAL=55, GREATEREQUAL=56, ID=57, INT=58, REAL=59, 
		STRING=60, Comment=61, WS=62;
	public static final int
		RULE_program = 0, RULE_statement = 1, RULE_graphDef = 2, RULE_nodes = 3, 
		RULE_edges = 4, RULE_nodeList = 5, RULE_edgeList = 6, RULE_graphID = 7, 
		RULE_nodeID = 8, RULE_fileEdgeList = 9, RULE_edge = 10, RULE_varDecl = 11, 
		RULE_conditionalStatement = 12, RULE_condition = 13, RULE_graphComprehension = 14, 
		RULE_graphCondition = 15, RULE_loopStatement = 16, RULE_foreachStatement = 17, 
		RULE_loopTarget = 18, RULE_whileStatement = 19, RULE_nodeEdgeOperation = 20, 
		RULE_addOperation = 21, RULE_removeOperation = 22, RULE_addTargets = 23, 
		RULE_removeTargets = 24, RULE_queryStatement = 25, RULE_showgraph = 26, 
		RULE_function = 27, RULE_returnType = 28, RULE_paramList = 29, RULE_param = 30, 
		RULE_type = 31, RULE_functionCall = 32, RULE_argumentList = 33, RULE_block = 34, 
		RULE_returnStatement = 35, RULE_printStatement = 36, RULE_printExpr = 37, 
		RULE_printgraph = 38, RULE_expr = 39, RULE_arrayDeclarator = 40, RULE_arrayInitializer = 41;
	private static String[] makeRuleNames() {
		return new String[] {
			"program", "statement", "graphDef", "nodes", "edges", "nodeList", "edgeList", 
			"graphID", "nodeID", "fileEdgeList", "edge", "varDecl", "conditionalStatement", 
			"condition", "graphComprehension", "graphCondition", "loopStatement", 
			"foreachStatement", "loopTarget", "whileStatement", "nodeEdgeOperation", 
			"addOperation", "removeOperation", "addTargets", "removeTargets", "queryStatement", 
			"showgraph", "function", "returnType", "paramList", "param", "type", 
			"functionCall", "argumentList", "block", "returnStatement", "printStatement", 
			"printExpr", "printgraph", "expr", "arrayDeclarator", "arrayInitializer"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "';'", "'{'", "'}'", "'nodes:'", "'edges:'", "','", "'file'", "'->'", 
			"'='", "'if'", "'('", "')'", "'else'", "'in'", "'['", "'where'", "']'", 
			"'degree'", "'connected'", "'with'", "'for'", "'each'", "'vertex'", "'edge'", 
			"'neighbor'", "'while'", "'add'", "'to'", "'remove'", "'from'", "'query'", 
			"':'", "'show'", "'fn'", "'int'", "'void'", "'string'", "'real'", "'return'", 
			"'print'", "'graph'", "'edges'", "'nodes'", "'of'", "'+'", "'-'", "'*'", 
			"'/'", "'&&'", "'||'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, null, "GRAPH", "EDGE", "NODE", "OF", "PLUS", 
			"MINUS", "TIMES", "DIVIDE", "AND", "OR", "EQUAL", "NOTEQUAL", "LESSTHAN", 
			"GREATERTHAN", "LESSEQUAL", "GREATEREQUAL", "ID", "INT", "REAL", "STRING", 
			"Comment", "WS"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}

	@Override
	public String getGrammarFileName() { return "Base.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public ATN getATN() { return _ATN; }

	public BaseParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ProgramContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(BaseParser.EOF, 0); }
		public List<StatementContext> statement() {
			return getRuleContexts(StatementContext.class);
		}
		public StatementContext statement(int i) {
			return getRuleContext(StatementContext.class,i);
		}
		public List<FunctionContext> function() {
			return getRuleContexts(FunctionContext.class);
		}
		public FunctionContext function(int i) {
			return getRuleContext(FunctionContext.class,i);
		}
		public ProgramContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_program; }
	}

	public final ProgramContext program() throws RecognitionException {
		ProgramContext _localctx = new ProgramContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_program);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(88);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & 144118961970086914L) != 0)) {
				{
				setState(86);
				_errHandler.sync(this);
				switch (_input.LA(1)) {
				case T__0:
				case T__9:
				case T__20:
				case T__22:
				case T__23:
				case T__25:
				case T__26:
				case T__28:
				case T__30:
				case T__32:
				case T__34:
				case T__36:
				case T__37:
				case T__39:
				case GRAPH:
				case ID:
					{
					setState(84);
					statement();
					}
					break;
				case T__33:
					{
					setState(85);
					function();
					}
					break;
				default:
					throw new NoViableAltException(this);
				}
				}
				setState(90);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(91);
			match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class StatementContext extends ParserRuleContext {
		public GraphDefContext graphDef() {
			return getRuleContext(GraphDefContext.class,0);
		}
		public ConditionalStatementContext conditionalStatement() {
			return getRuleContext(ConditionalStatementContext.class,0);
		}
		public PrintStatementContext printStatement() {
			return getRuleContext(PrintStatementContext.class,0);
		}
		public LoopStatementContext loopStatement() {
			return getRuleContext(LoopStatementContext.class,0);
		}
		public VarDeclContext varDecl() {
			return getRuleContext(VarDeclContext.class,0);
		}
		public FunctionCallContext functionCall() {
			return getRuleContext(FunctionCallContext.class,0);
		}
		public GraphComprehensionContext graphComprehension() {
			return getRuleContext(GraphComprehensionContext.class,0);
		}
		public QueryStatementContext queryStatement() {
			return getRuleContext(QueryStatementContext.class,0);
		}
		public ShowgraphContext showgraph() {
			return getRuleContext(ShowgraphContext.class,0);
		}
		public NodeEdgeOperationContext nodeEdgeOperation() {
			return getRuleContext(NodeEdgeOperationContext.class,0);
		}
		public StatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_statement; }
	}

	public final StatementContext statement() throws RecognitionException {
		StatementContext _localctx = new StatementContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_statement);
		try {
			setState(104);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,2,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(93);
				graphDef();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(94);
				conditionalStatement();
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(95);
				printStatement();
				}
				break;
			case 4:
				enterOuterAlt(_localctx, 4);
				{
				setState(96);
				loopStatement();
				}
				break;
			case 5:
				enterOuterAlt(_localctx, 5);
				{
				setState(97);
				varDecl();
				}
				break;
			case 6:
				enterOuterAlt(_localctx, 6);
				{
				setState(98);
				functionCall();
				}
				break;
			case 7:
				enterOuterAlt(_localctx, 7);
				{
				setState(99);
				graphComprehension();
				}
				break;
			case 8:
				enterOuterAlt(_localctx, 8);
				{
				setState(100);
				queryStatement();
				}
				break;
			case 9:
				enterOuterAlt(_localctx, 9);
				{
				setState(101);
				showgraph();
				}
				break;
			case 10:
				enterOuterAlt(_localctx, 10);
				{
				setState(102);
				nodeEdgeOperation();
				}
				break;
			case 11:
				enterOuterAlt(_localctx, 11);
				{
				setState(103);
				match(T__0);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class GraphDefContext extends ParserRuleContext {
		public TerminalNode GRAPH() { return getToken(BaseParser.GRAPH, 0); }
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public NodesContext nodes() {
			return getRuleContext(NodesContext.class,0);
		}
		public EdgesContext edges() {
			return getRuleContext(EdgesContext.class,0);
		}
		public GraphDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_graphDef; }
	}

	public final GraphDefContext graphDef() throws RecognitionException {
		GraphDefContext _localctx = new GraphDefContext(_ctx, getState());
		enterRule(_localctx, 4, RULE_graphDef);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(106);
			match(GRAPH);
			setState(107);
			graphID();
			setState(108);
			match(T__1);
			setState(110);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==T__3) {
				{
				setState(109);
				nodes();
				}
			}

			setState(113);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==T__4) {
				{
				setState(112);
				edges();
				}
			}

			setState(115);
			match(T__2);
			setState(116);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class NodesContext extends ParserRuleContext {
		public NodeListContext nodeList() {
			return getRuleContext(NodeListContext.class,0);
		}
		public NodesContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_nodes; }
	}

	public final NodesContext nodes() throws RecognitionException {
		NodesContext _localctx = new NodesContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_nodes);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(118);
			match(T__3);
			setState(119);
			nodeList();
			setState(120);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class EdgesContext extends ParserRuleContext {
		public EdgeListContext edgeList() {
			return getRuleContext(EdgeListContext.class,0);
		}
		public FileEdgeListContext fileEdgeList() {
			return getRuleContext(FileEdgeListContext.class,0);
		}
		public EdgesContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_edges; }
	}

	public final EdgesContext edges() throws RecognitionException {
		EdgesContext _localctx = new EdgesContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_edges);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(122);
			match(T__4);
			setState(125);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case INT:
				{
				setState(123);
				edgeList();
				}
				break;
			case T__6:
				{
				setState(124);
				fileEdgeList();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			setState(127);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class NodeListContext extends ParserRuleContext {
		public List<NodeIDContext> nodeID() {
			return getRuleContexts(NodeIDContext.class);
		}
		public NodeIDContext nodeID(int i) {
			return getRuleContext(NodeIDContext.class,i);
		}
		public NodeListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_nodeList; }
	}

	public final NodeListContext nodeList() throws RecognitionException {
		NodeListContext _localctx = new NodeListContext(_ctx, getState());
		enterRule(_localctx, 10, RULE_nodeList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(129);
			nodeID();
			setState(134);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__5) {
				{
				{
				setState(130);
				match(T__5);
				setState(131);
				nodeID();
				}
				}
				setState(136);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class EdgeListContext extends ParserRuleContext {
		public List<EdgeContext> edge() {
			return getRuleContexts(EdgeContext.class);
		}
		public EdgeContext edge(int i) {
			return getRuleContext(EdgeContext.class,i);
		}
		public EdgeListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_edgeList; }
	}

	public final EdgeListContext edgeList() throws RecognitionException {
		EdgeListContext _localctx = new EdgeListContext(_ctx, getState());
		enterRule(_localctx, 12, RULE_edgeList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(137);
			edge();
			setState(142);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__5) {
				{
				{
				setState(138);
				match(T__5);
				setState(139);
				edge();
				}
				}
				setState(144);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class GraphIDContext extends ParserRuleContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public GraphIDContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_graphID; }
	}

	public final GraphIDContext graphID() throws RecognitionException {
		GraphIDContext _localctx = new GraphIDContext(_ctx, getState());
		enterRule(_localctx, 14, RULE_graphID);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(145);
			match(ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class NodeIDContext extends ParserRuleContext {
		public TerminalNode INT() { return getToken(BaseParser.INT, 0); }
		public NodeIDContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_nodeID; }
	}

	public final NodeIDContext nodeID() throws RecognitionException {
		NodeIDContext _localctx = new NodeIDContext(_ctx, getState());
		enterRule(_localctx, 16, RULE_nodeID);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(147);
			match(INT);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class FileEdgeListContext extends ParserRuleContext {
		public TerminalNode STRING() { return getToken(BaseParser.STRING, 0); }
		public FileEdgeListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_fileEdgeList; }
	}

	public final FileEdgeListContext fileEdgeList() throws RecognitionException {
		FileEdgeListContext _localctx = new FileEdgeListContext(_ctx, getState());
		enterRule(_localctx, 18, RULE_fileEdgeList);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(149);
			match(T__6);
			setState(150);
			match(STRING);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class EdgeContext extends ParserRuleContext {
		public List<NodeIDContext> nodeID() {
			return getRuleContexts(NodeIDContext.class);
		}
		public NodeIDContext nodeID(int i) {
			return getRuleContext(NodeIDContext.class,i);
		}
		public EdgeContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_edge; }
	}

	public final EdgeContext edge() throws RecognitionException {
		EdgeContext _localctx = new EdgeContext(_ctx, getState());
		enterRule(_localctx, 20, RULE_edge);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(152);
			nodeID();
			setState(153);
			match(T__7);
			setState(154);
			nodeID();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class VarDeclContext extends ParserRuleContext {
		public VarDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_varDecl; }
	 
		public VarDeclContext() { }
		public void copyFrom(VarDeclContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class SimpleDeclarationContext extends VarDeclContext {
		public TypeContext type() {
			return getRuleContext(TypeContext.class,0);
		}
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public SimpleDeclarationContext(VarDeclContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrayDeclarationContext extends VarDeclContext {
		public TypeContext type() {
			return getRuleContext(TypeContext.class,0);
		}
		public ArrayDeclaratorContext arrayDeclarator() {
			return getRuleContext(ArrayDeclaratorContext.class,0);
		}
		public ArrayInitializerContext arrayInitializer() {
			return getRuleContext(ArrayInitializerContext.class,0);
		}
		public ArrayDeclarationContext(VarDeclContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class AssignedDeclarationContext extends VarDeclContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public VarDeclContext varDecl() {
			return getRuleContext(VarDeclContext.class,0);
		}
		public AssignedDeclarationContext(VarDeclContext ctx) { copyFrom(ctx); }
	}

	public final VarDeclContext varDecl() throws RecognitionException {
		VarDeclContext _localctx = new VarDeclContext(_ctx, getState());
		enterRule(_localctx, 22, RULE_varDecl);
		int _la;
		try {
			setState(177);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,10,_ctx) ) {
			case 1:
				_localctx = new SimpleDeclarationContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(156);
				type();
				setState(157);
				match(ID);
				setState(160);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==T__8) {
					{
					setState(158);
					match(T__8);
					setState(159);
					expr(0);
					}
				}

				setState(162);
				match(T__0);
				}
				break;
			case 2:
				_localctx = new ArrayDeclarationContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(164);
				type();
				setState(165);
				arrayDeclarator();
				setState(168);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==T__8) {
					{
					setState(166);
					match(T__8);
					setState(167);
					arrayInitializer();
					}
				}

				setState(170);
				match(T__0);
				}
				break;
			case 3:
				_localctx = new AssignedDeclarationContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(172);
				match(ID);
				setState(173);
				match(T__8);
				setState(174);
				varDecl();
				setState(175);
				match(T__0);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ConditionalStatementContext extends ParserRuleContext {
		public ConditionContext condition() {
			return getRuleContext(ConditionContext.class,0);
		}
		public List<BlockContext> block() {
			return getRuleContexts(BlockContext.class);
		}
		public BlockContext block(int i) {
			return getRuleContext(BlockContext.class,i);
		}
		public ConditionalStatementContext conditionalStatement() {
			return getRuleContext(ConditionalStatementContext.class,0);
		}
		public ConditionalStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_conditionalStatement; }
	}

	public final ConditionalStatementContext conditionalStatement() throws RecognitionException {
		ConditionalStatementContext _localctx = new ConditionalStatementContext(_ctx, getState());
		enterRule(_localctx, 24, RULE_conditionalStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(179);
			match(T__9);
			setState(180);
			match(T__10);
			setState(181);
			condition(0);
			setState(182);
			match(T__11);
			setState(183);
			block();
			setState(188);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,11,_ctx) ) {
			case 1:
				{
				setState(184);
				match(T__12);
				setState(185);
				conditionalStatement();
				}
				break;
			case 2:
				{
				setState(186);
				match(T__12);
				setState(187);
				block();
				}
				break;
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ConditionContext extends ParserRuleContext {
		public ConditionContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_condition; }
	 
		public ConditionContext() { }
		public void copyFrom(ConditionContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class NodeCheckContext extends ConditionContext {
		public NodeIDContext nodeID() {
			return getRuleContext(NodeIDContext.class,0);
		}
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public NodeCheckContext(ConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class LogicalAndContext extends ConditionContext {
		public List<ConditionContext> condition() {
			return getRuleContexts(ConditionContext.class);
		}
		public ConditionContext condition(int i) {
			return getRuleContext(ConditionContext.class,i);
		}
		public TerminalNode AND() { return getToken(BaseParser.AND, 0); }
		public LogicalAndContext(ConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class RelationalContext extends ConditionContext {
		public List<ExprContext> expr() {
			return getRuleContexts(ExprContext.class);
		}
		public ExprContext expr(int i) {
			return getRuleContext(ExprContext.class,i);
		}
		public TerminalNode EQUAL() { return getToken(BaseParser.EQUAL, 0); }
		public TerminalNode NOTEQUAL() { return getToken(BaseParser.NOTEQUAL, 0); }
		public TerminalNode LESSEQUAL() { return getToken(BaseParser.LESSEQUAL, 0); }
		public TerminalNode GREATEREQUAL() { return getToken(BaseParser.GREATEREQUAL, 0); }
		public TerminalNode LESSTHAN() { return getToken(BaseParser.LESSTHAN, 0); }
		public TerminalNode GREATERTHAN() { return getToken(BaseParser.GREATERTHAN, 0); }
		public RelationalContext(ConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class EdgeCheckContext extends ConditionContext {
		public EdgeContext edge() {
			return getRuleContext(EdgeContext.class,0);
		}
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public EdgeCheckContext(ConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class LogicalOrContext extends ConditionContext {
		public List<ConditionContext> condition() {
			return getRuleContexts(ConditionContext.class);
		}
		public ConditionContext condition(int i) {
			return getRuleContext(ConditionContext.class,i);
		}
		public TerminalNode OR() { return getToken(BaseParser.OR, 0); }
		public LogicalOrContext(ConditionContext ctx) { copyFrom(ctx); }
	}

	public final ConditionContext condition() throws RecognitionException {
		return condition(0);
	}

	private ConditionContext condition(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		ConditionContext _localctx = new ConditionContext(_ctx, _parentState);
		ConditionContext _prevctx = _localctx;
		int _startState = 26;
		enterRecursionRule(_localctx, 26, RULE_condition, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(203);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,12,_ctx) ) {
			case 1:
				{
				_localctx = new RelationalContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(191);
				expr(0);
				setState(192);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 141863388262170624L) != 0)) ) {
				_errHandler.recoverInline(this);
				}
				else {
					if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
					_errHandler.reportMatch(this);
					consume();
				}
				setState(193);
				expr(0);
				}
				break;
			case 2:
				{
				_localctx = new NodeCheckContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(195);
				nodeID();
				setState(196);
				match(T__13);
				setState(197);
				graphID();
				}
				break;
			case 3:
				{
				_localctx = new EdgeCheckContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(199);
				edge();
				setState(200);
				match(T__13);
				setState(201);
				graphID();
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(213);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,14,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(211);
					_errHandler.sync(this);
					switch ( getInterpreter().adaptivePredict(_input,13,_ctx) ) {
					case 1:
						{
						_localctx = new LogicalAndContext(new ConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_condition);
						setState(205);
						if (!(precpred(_ctx, 5))) throw new FailedPredicateException(this, "precpred(_ctx, 5)");
						setState(206);
						match(AND);
						setState(207);
						condition(6);
						}
						break;
					case 2:
						{
						_localctx = new LogicalOrContext(new ConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_condition);
						setState(208);
						if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
						setState(209);
						match(OR);
						setState(210);
						condition(5);
						}
						break;
					}
					} 
				}
				setState(215);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,14,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class GraphComprehensionContext extends ParserRuleContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public GraphConditionContext graphCondition() {
			return getRuleContext(GraphConditionContext.class,0);
		}
		public GraphComprehensionContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_graphComprehension; }
	}

	public final GraphComprehensionContext graphComprehension() throws RecognitionException {
		GraphComprehensionContext _localctx = new GraphComprehensionContext(_ctx, getState());
		enterRule(_localctx, 28, RULE_graphComprehension);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(216);
			match(ID);
			setState(217);
			match(T__8);
			setState(218);
			match(T__14);
			setState(219);
			graphID();
			setState(220);
			match(T__15);
			setState(221);
			graphCondition(0);
			setState(222);
			match(T__16);
			setState(223);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class GraphConditionContext extends ParserRuleContext {
		public GraphConditionContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_graphCondition; }
	 
		public GraphConditionContext() { }
		public void copyFrom(GraphConditionContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class GraphLogicalAndContext extends GraphConditionContext {
		public List<GraphConditionContext> graphCondition() {
			return getRuleContexts(GraphConditionContext.class);
		}
		public GraphConditionContext graphCondition(int i) {
			return getRuleContext(GraphConditionContext.class,i);
		}
		public TerminalNode AND() { return getToken(BaseParser.AND, 0); }
		public GraphLogicalAndContext(GraphConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class DegreeConditionContext extends GraphConditionContext {
		public TerminalNode INT() { return getToken(BaseParser.INT, 0); }
		public TerminalNode EQUAL() { return getToken(BaseParser.EQUAL, 0); }
		public TerminalNode NOTEQUAL() { return getToken(BaseParser.NOTEQUAL, 0); }
		public TerminalNode LESSEQUAL() { return getToken(BaseParser.LESSEQUAL, 0); }
		public TerminalNode GREATEREQUAL() { return getToken(BaseParser.GREATEREQUAL, 0); }
		public TerminalNode LESSTHAN() { return getToken(BaseParser.LESSTHAN, 0); }
		public TerminalNode GREATERTHAN() { return getToken(BaseParser.GREATERTHAN, 0); }
		public DegreeConditionContext(GraphConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ConnectedConditionContext extends GraphConditionContext {
		public NodeIDContext nodeID() {
			return getRuleContext(NodeIDContext.class,0);
		}
		public ConnectedConditionContext(GraphConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class GraphLogicalOrContext extends GraphConditionContext {
		public List<GraphConditionContext> graphCondition() {
			return getRuleContexts(GraphConditionContext.class);
		}
		public GraphConditionContext graphCondition(int i) {
			return getRuleContext(GraphConditionContext.class,i);
		}
		public TerminalNode OR() { return getToken(BaseParser.OR, 0); }
		public GraphLogicalOrContext(GraphConditionContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ParenGraphConditionContext extends GraphConditionContext {
		public GraphConditionContext graphCondition() {
			return getRuleContext(GraphConditionContext.class,0);
		}
		public ParenGraphConditionContext(GraphConditionContext ctx) { copyFrom(ctx); }
	}

	public final GraphConditionContext graphCondition() throws RecognitionException {
		return graphCondition(0);
	}

	private GraphConditionContext graphCondition(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		GraphConditionContext _localctx = new GraphConditionContext(_ctx, _parentState);
		GraphConditionContext _prevctx = _localctx;
		int _startState = 30;
		enterRecursionRule(_localctx, 30, RULE_graphCondition, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(236);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__17:
				{
				_localctx = new DegreeConditionContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(226);
				match(T__17);
				setState(227);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 141863388262170624L) != 0)) ) {
				_errHandler.recoverInline(this);
				}
				else {
					if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
					_errHandler.reportMatch(this);
					consume();
				}
				setState(228);
				match(INT);
				}
				break;
			case T__18:
				{
				_localctx = new ConnectedConditionContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(229);
				match(T__18);
				setState(230);
				match(T__19);
				setState(231);
				nodeID();
				}
				break;
			case T__10:
				{
				_localctx = new ParenGraphConditionContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(232);
				match(T__10);
				setState(233);
				graphCondition(0);
				setState(234);
				match(T__11);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			_ctx.stop = _input.LT(-1);
			setState(246);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,17,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(244);
					_errHandler.sync(this);
					switch ( getInterpreter().adaptivePredict(_input,16,_ctx) ) {
					case 1:
						{
						_localctx = new GraphLogicalAndContext(new GraphConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_graphCondition);
						setState(238);
						if (!(precpred(_ctx, 5))) throw new FailedPredicateException(this, "precpred(_ctx, 5)");
						setState(239);
						match(AND);
						setState(240);
						graphCondition(6);
						}
						break;
					case 2:
						{
						_localctx = new GraphLogicalOrContext(new GraphConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_graphCondition);
						setState(241);
						if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
						setState(242);
						match(OR);
						setState(243);
						graphCondition(5);
						}
						break;
					}
					} 
				}
				setState(248);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,17,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class LoopStatementContext extends ParserRuleContext {
		public ForeachStatementContext foreachStatement() {
			return getRuleContext(ForeachStatementContext.class,0);
		}
		public WhileStatementContext whileStatement() {
			return getRuleContext(WhileStatementContext.class,0);
		}
		public LoopStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_loopStatement; }
	}

	public final LoopStatementContext loopStatement() throws RecognitionException {
		LoopStatementContext _localctx = new LoopStatementContext(_ctx, getState());
		enterRule(_localctx, 32, RULE_loopStatement);
		try {
			setState(251);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__20:
				enterOuterAlt(_localctx, 1);
				{
				setState(249);
				foreachStatement();
				}
				break;
			case T__25:
				enterOuterAlt(_localctx, 2);
				{
				setState(250);
				whileStatement();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ForeachStatementContext extends ParserRuleContext {
		public LoopTargetContext loopTarget() {
			return getRuleContext(LoopTargetContext.class,0);
		}
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public ForeachStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_foreachStatement; }
	}

	public final ForeachStatementContext foreachStatement() throws RecognitionException {
		ForeachStatementContext _localctx = new ForeachStatementContext(_ctx, getState());
		enterRule(_localctx, 34, RULE_foreachStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(253);
			match(T__20);
			setState(254);
			match(T__21);
			setState(255);
			loopTarget();
			setState(256);
			match(T__13);
			setState(257);
			graphID();
			setState(258);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class LoopTargetContext extends ParserRuleContext {
		public LoopTargetContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_loopTarget; }
	 
		public LoopTargetContext() { }
		public void copyFrom(LoopTargetContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ForEachVertexContext extends LoopTargetContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ForEachVertexContext(LoopTargetContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ForEachAdjContext extends LoopTargetContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public TerminalNode OF() { return getToken(BaseParser.OF, 0); }
		public NodeIDContext nodeID() {
			return getRuleContext(NodeIDContext.class,0);
		}
		public ForEachAdjContext(LoopTargetContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ForEachEdgeContext extends LoopTargetContext {
		public List<TerminalNode> ID() { return getTokens(BaseParser.ID); }
		public TerminalNode ID(int i) {
			return getToken(BaseParser.ID, i);
		}
		public ForEachEdgeContext(LoopTargetContext ctx) { copyFrom(ctx); }
	}

	public final LoopTargetContext loopTarget() throws RecognitionException {
		LoopTargetContext _localctx = new LoopTargetContext(_ctx, getState());
		enterRule(_localctx, 36, RULE_loopTarget);
		try {
			setState(270);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__22:
				_localctx = new ForEachVertexContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(260);
				match(T__22);
				setState(261);
				match(ID);
				}
				break;
			case T__23:
				_localctx = new ForEachEdgeContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(262);
				match(T__23);
				setState(263);
				match(ID);
				setState(264);
				match(T__5);
				setState(265);
				match(ID);
				}
				break;
			case T__24:
				_localctx = new ForEachAdjContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(266);
				match(T__24);
				setState(267);
				match(ID);
				setState(268);
				match(OF);
				setState(269);
				nodeID();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class WhileStatementContext extends ParserRuleContext {
		public ConditionContext condition() {
			return getRuleContext(ConditionContext.class,0);
		}
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public WhileStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_whileStatement; }
	}

	public final WhileStatementContext whileStatement() throws RecognitionException {
		WhileStatementContext _localctx = new WhileStatementContext(_ctx, getState());
		enterRule(_localctx, 38, RULE_whileStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(272);
			match(T__25);
			setState(273);
			match(T__10);
			setState(274);
			condition(0);
			setState(275);
			match(T__11);
			setState(276);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class NodeEdgeOperationContext extends ParserRuleContext {
		public AddOperationContext addOperation() {
			return getRuleContext(AddOperationContext.class,0);
		}
		public RemoveOperationContext removeOperation() {
			return getRuleContext(RemoveOperationContext.class,0);
		}
		public NodeEdgeOperationContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_nodeEdgeOperation; }
	}

	public final NodeEdgeOperationContext nodeEdgeOperation() throws RecognitionException {
		NodeEdgeOperationContext _localctx = new NodeEdgeOperationContext(_ctx, getState());
		enterRule(_localctx, 40, RULE_nodeEdgeOperation);
		try {
			setState(280);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__26:
				enterOuterAlt(_localctx, 1);
				{
				setState(278);
				addOperation();
				}
				break;
			case T__28:
				enterOuterAlt(_localctx, 2);
				{
				setState(279);
				removeOperation();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class AddOperationContext extends ParserRuleContext {
		public AddTargetsContext addTargets() {
			return getRuleContext(AddTargetsContext.class,0);
		}
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public AddOperationContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_addOperation; }
	}

	public final AddOperationContext addOperation() throws RecognitionException {
		AddOperationContext _localctx = new AddOperationContext(_ctx, getState());
		enterRule(_localctx, 42, RULE_addOperation);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(282);
			match(T__26);
			setState(283);
			addTargets();
			setState(284);
			match(T__27);
			setState(285);
			graphID();
			setState(286);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class RemoveOperationContext extends ParserRuleContext {
		public RemoveTargetsContext removeTargets() {
			return getRuleContext(RemoveTargetsContext.class,0);
		}
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public RemoveOperationContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_removeOperation; }
	}

	public final RemoveOperationContext removeOperation() throws RecognitionException {
		RemoveOperationContext _localctx = new RemoveOperationContext(_ctx, getState());
		enterRule(_localctx, 44, RULE_removeOperation);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(288);
			match(T__28);
			setState(289);
			removeTargets();
			setState(290);
			match(T__29);
			setState(291);
			graphID();
			setState(292);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class AddTargetsContext extends ParserRuleContext {
		public NodeIDContext nodeID() {
			return getRuleContext(NodeIDContext.class,0);
		}
		public EdgeContext edge() {
			return getRuleContext(EdgeContext.class,0);
		}
		public NodeListContext nodeList() {
			return getRuleContext(NodeListContext.class,0);
		}
		public EdgeListContext edgeList() {
			return getRuleContext(EdgeListContext.class,0);
		}
		public AddTargetsContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_addTargets; }
	}

	public final AddTargetsContext addTargets() throws RecognitionException {
		AddTargetsContext _localctx = new AddTargetsContext(_ctx, getState());
		enterRule(_localctx, 46, RULE_addTargets);
		try {
			setState(298);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,21,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(294);
				nodeID();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(295);
				edge();
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(296);
				nodeList();
				}
				break;
			case 4:
				enterOuterAlt(_localctx, 4);
				{
				setState(297);
				edgeList();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class RemoveTargetsContext extends ParserRuleContext {
		public NodeIDContext nodeID() {
			return getRuleContext(NodeIDContext.class,0);
		}
		public EdgeContext edge() {
			return getRuleContext(EdgeContext.class,0);
		}
		public NodeListContext nodeList() {
			return getRuleContext(NodeListContext.class,0);
		}
		public EdgeListContext edgeList() {
			return getRuleContext(EdgeListContext.class,0);
		}
		public RemoveTargetsContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_removeTargets; }
	}

	public final RemoveTargetsContext removeTargets() throws RecognitionException {
		RemoveTargetsContext _localctx = new RemoveTargetsContext(_ctx, getState());
		enterRule(_localctx, 48, RULE_removeTargets);
		try {
			setState(304);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,22,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(300);
				nodeID();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(301);
				edge();
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(302);
				nodeList();
				}
				break;
			case 4:
				enterOuterAlt(_localctx, 4);
				{
				setState(303);
				edgeList();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class QueryStatementContext extends ParserRuleContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public TerminalNode STRING() { return getToken(BaseParser.STRING, 0); }
		public TerminalNode OF() { return getToken(BaseParser.OF, 0); }
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public QueryStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_queryStatement; }
	}

	public final QueryStatementContext queryStatement() throws RecognitionException {
		QueryStatementContext _localctx = new QueryStatementContext(_ctx, getState());
		enterRule(_localctx, 50, RULE_queryStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(306);
			match(T__30);
			setState(307);
			match(ID);
			setState(308);
			match(T__31);
			setState(309);
			match(STRING);
			setState(310);
			match(OF);
			setState(311);
			graphID();
			setState(312);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ShowgraphContext extends ParserRuleContext {
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public ShowgraphContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_showgraph; }
	}

	public final ShowgraphContext showgraph() throws RecognitionException {
		ShowgraphContext _localctx = new ShowgraphContext(_ctx, getState());
		enterRule(_localctx, 52, RULE_showgraph);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(314);
			match(T__32);
			setState(315);
			graphID();
			setState(316);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class FunctionContext extends ParserRuleContext {
		public ReturnTypeContext returnType() {
			return getRuleContext(ReturnTypeContext.class,0);
		}
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ParamListContext paramList() {
			return getRuleContext(ParamListContext.class,0);
		}
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public FunctionContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_function; }
	}

	public final FunctionContext function() throws RecognitionException {
		FunctionContext _localctx = new FunctionContext(_ctx, getState());
		enterRule(_localctx, 54, RULE_function);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(318);
			match(T__33);
			setState(319);
			returnType();
			setState(320);
			match(ID);
			setState(321);
			paramList();
			setState(322);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ReturnTypeContext extends ParserRuleContext {
		public TerminalNode GRAPH() { return getToken(BaseParser.GRAPH, 0); }
		public ReturnTypeContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_returnType; }
	}

	public final ReturnTypeContext returnType() throws RecognitionException {
		ReturnTypeContext _localctx = new ReturnTypeContext(_ctx, getState());
		enterRule(_localctx, 56, RULE_returnType);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(324);
			_la = _input.LA(1);
			if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 2439566589952L) != 0)) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ParamListContext extends ParserRuleContext {
		public List<ParamContext> param() {
			return getRuleContexts(ParamContext.class);
		}
		public ParamContext param(int i) {
			return getRuleContext(ParamContext.class,i);
		}
		public ParamListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_paramList; }
	}

	public final ParamListContext paramList() throws RecognitionException {
		ParamListContext _localctx = new ParamListContext(_ctx, getState());
		enterRule(_localctx, 58, RULE_paramList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(326);
			match(T__10);
			setState(335);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 2645725020160L) != 0)) {
				{
				setState(327);
				param();
				setState(332);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==T__5) {
					{
					{
					setState(328);
					match(T__5);
					setState(329);
					param();
					}
					}
					setState(334);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				}
			}

			setState(337);
			match(T__11);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ParamContext extends ParserRuleContext {
		public TypeContext type() {
			return getRuleContext(TypeContext.class,0);
		}
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ParamContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_param; }
	}

	public final ParamContext param() throws RecognitionException {
		ParamContext _localctx = new ParamContext(_ctx, getState());
		enterRule(_localctx, 60, RULE_param);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(339);
			type();
			setState(340);
			match(ID);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class TypeContext extends ParserRuleContext {
		public TerminalNode GRAPH() { return getToken(BaseParser.GRAPH, 0); }
		public TypeContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_type; }
	}

	public final TypeContext type() throws RecognitionException {
		TypeContext _localctx = new TypeContext(_ctx, getState());
		enterRule(_localctx, 62, RULE_type);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(342);
			_la = _input.LA(1);
			if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 2645725020160L) != 0)) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class FunctionCallContext extends ParserRuleContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ArgumentListContext argumentList() {
			return getRuleContext(ArgumentListContext.class,0);
		}
		public FunctionCallContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_functionCall; }
	}

	public final FunctionCallContext functionCall() throws RecognitionException {
		FunctionCallContext _localctx = new FunctionCallContext(_ctx, getState());
		enterRule(_localctx, 64, RULE_functionCall);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(344);
			match(ID);
			setState(345);
			match(T__10);
			setState(347);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 1008806316530993152L) != 0)) {
				{
				setState(346);
				argumentList();
				}
			}

			setState(349);
			match(T__11);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ArgumentListContext extends ParserRuleContext {
		public List<ExprContext> expr() {
			return getRuleContexts(ExprContext.class);
		}
		public ExprContext expr(int i) {
			return getRuleContext(ExprContext.class,i);
		}
		public ArgumentListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_argumentList; }
	}

	public final ArgumentListContext argumentList() throws RecognitionException {
		ArgumentListContext _localctx = new ArgumentListContext(_ctx, getState());
		enterRule(_localctx, 66, RULE_argumentList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(351);
			expr(0);
			setState(356);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__5) {
				{
				{
				setState(352);
				match(T__5);
				setState(353);
				expr(0);
				}
				}
				setState(358);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class BlockContext extends ParserRuleContext {
		public List<StatementContext> statement() {
			return getRuleContexts(StatementContext.class);
		}
		public StatementContext statement(int i) {
			return getRuleContext(StatementContext.class,i);
		}
		public List<ReturnStatementContext> returnStatement() {
			return getRuleContexts(ReturnStatementContext.class);
		}
		public ReturnStatementContext returnStatement(int i) {
			return getRuleContext(ReturnStatementContext.class,i);
		}
		public BlockContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_block; }
	}

	public final BlockContext block() throws RecognitionException {
		BlockContext _localctx = new BlockContext(_ctx, getState());
		enterRule(_localctx, 68, RULE_block);
		int _la;
		try {
			setState(370);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,29,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(359);
				match(T__1);
				setState(364);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while ((((_la) & ~0x3f) == 0 && ((1L << _la) & 144119494546031618L) != 0)) {
					{
					setState(362);
					_errHandler.sync(this);
					switch (_input.LA(1)) {
					case T__0:
					case T__9:
					case T__20:
					case T__22:
					case T__23:
					case T__25:
					case T__26:
					case T__28:
					case T__30:
					case T__32:
					case T__34:
					case T__36:
					case T__37:
					case T__39:
					case GRAPH:
					case ID:
						{
						setState(360);
						statement();
						}
						break;
					case T__38:
						{
						setState(361);
						returnStatement();
						}
						break;
					default:
						throw new NoViableAltException(this);
					}
					}
					setState(366);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(367);
				match(T__2);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(368);
				match(T__1);
				setState(369);
				match(T__2);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ReturnStatementContext extends ParserRuleContext {
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public ReturnStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_returnStatement; }
	}

	public final ReturnStatementContext returnStatement() throws RecognitionException {
		ReturnStatementContext _localctx = new ReturnStatementContext(_ctx, getState());
		enterRule(_localctx, 70, RULE_returnStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(372);
			match(T__38);
			setState(373);
			expr(0);
			setState(374);
			match(T__0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class PrintStatementContext extends ParserRuleContext {
		public PrintExprContext printExpr() {
			return getRuleContext(PrintExprContext.class,0);
		}
		public PrintgraphContext printgraph() {
			return getRuleContext(PrintgraphContext.class,0);
		}
		public PrintStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_printStatement; }
	}

	public final PrintStatementContext printStatement() throws RecognitionException {
		PrintStatementContext _localctx = new PrintStatementContext(_ctx, getState());
		enterRule(_localctx, 72, RULE_printStatement);
		try {
			setState(381);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,30,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(376);
				match(T__39);
				setState(377);
				printExpr(0);
				setState(378);
				match(T__0);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(380);
				printgraph();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class PrintExprContext extends ParserRuleContext {
		public TerminalNode STRING() { return getToken(BaseParser.STRING, 0); }
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public List<PrintExprContext> printExpr() {
			return getRuleContexts(PrintExprContext.class);
		}
		public PrintExprContext printExpr(int i) {
			return getRuleContext(PrintExprContext.class,i);
		}
		public TerminalNode PLUS() { return getToken(BaseParser.PLUS, 0); }
		public PrintExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_printExpr; }
	}

	public final PrintExprContext printExpr() throws RecognitionException {
		return printExpr(0);
	}

	private PrintExprContext printExpr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		PrintExprContext _localctx = new PrintExprContext(_ctx, _parentState);
		PrintExprContext _prevctx = _localctx;
		int _startState = 74;
		enterRecursionRule(_localctx, 74, RULE_printExpr, _p);
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(386);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case STRING:
				{
				setState(384);
				match(STRING);
				}
				break;
			case T__10:
			case ID:
			case INT:
			case REAL:
				{
				setState(385);
				expr(0);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			_ctx.stop = _input.LT(-1);
			setState(393);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,32,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new PrintExprContext(_parentctx, _parentState);
					pushNewRecursionContext(_localctx, _startState, RULE_printExpr);
					setState(388);
					if (!(precpred(_ctx, 1))) throw new FailedPredicateException(this, "precpred(_ctx, 1)");
					setState(389);
					match(PLUS);
					setState(390);
					printExpr(2);
					}
					} 
				}
				setState(395);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,32,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class PrintgraphContext extends ParserRuleContext {
		public PrintgraphContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_printgraph; }
	 
		public PrintgraphContext() { }
		public void copyFrom(PrintgraphContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class NodePrintContext extends PrintgraphContext {
		public TerminalNode NODE() { return getToken(BaseParser.NODE, 0); }
		public TerminalNode OF() { return getToken(BaseParser.OF, 0); }
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public NodePrintContext(PrintgraphContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class GraphPrintContext extends PrintgraphContext {
		public TerminalNode GRAPH() { return getToken(BaseParser.GRAPH, 0); }
		public TerminalNode OF() { return getToken(BaseParser.OF, 0); }
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public GraphPrintContext(PrintgraphContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class EdgePrintContext extends PrintgraphContext {
		public TerminalNode EDGE() { return getToken(BaseParser.EDGE, 0); }
		public TerminalNode OF() { return getToken(BaseParser.OF, 0); }
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public EdgePrintContext(PrintgraphContext ctx) { copyFrom(ctx); }
	}

	public final PrintgraphContext printgraph() throws RecognitionException {
		PrintgraphContext _localctx = new PrintgraphContext(_ctx, getState());
		enterRule(_localctx, 76, RULE_printgraph);
		try {
			setState(414);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,33,_ctx) ) {
			case 1:
				_localctx = new EdgePrintContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(396);
				match(T__39);
				setState(397);
				match(EDGE);
				setState(398);
				match(OF);
				setState(399);
				graphID();
				setState(400);
				match(T__0);
				}
				break;
			case 2:
				_localctx = new NodePrintContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(402);
				match(T__39);
				setState(403);
				match(NODE);
				setState(404);
				match(OF);
				setState(405);
				graphID();
				setState(406);
				match(T__0);
				}
				break;
			case 3:
				_localctx = new GraphPrintContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(408);
				match(T__39);
				setState(409);
				match(GRAPH);
				setState(410);
				match(OF);
				setState(411);
				graphID();
				setState(412);
				match(T__0);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExprContext extends ParserRuleContext {
		public ExprContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_expr; }
	 
		public ExprContext() { }
		public void copyFrom(ExprContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class MulDivExprContext extends ExprContext {
		public List<ExprContext> expr() {
			return getRuleContexts(ExprContext.class);
		}
		public ExprContext expr(int i) {
			return getRuleContext(ExprContext.class,i);
		}
		public TerminalNode TIMES() { return getToken(BaseParser.TIMES, 0); }
		public TerminalNode DIVIDE() { return getToken(BaseParser.DIVIDE, 0); }
		public MulDivExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class IdExprContext extends ExprContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public IdExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class IntExprContext extends ExprContext {
		public TerminalNode INT() { return getToken(BaseParser.INT, 0); }
		public IntExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ParenExprContext extends ExprContext {
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public ParenExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class FuncExprContext extends ExprContext {
		public FunctionCallContext functionCall() {
			return getRuleContext(FunctionCallContext.class,0);
		}
		public FuncExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class RealExprContext extends ExprContext {
		public TerminalNode REAL() { return getToken(BaseParser.REAL, 0); }
		public RealExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class AddSubExprContext extends ExprContext {
		public List<ExprContext> expr() {
			return getRuleContexts(ExprContext.class);
		}
		public ExprContext expr(int i) {
			return getRuleContext(ExprContext.class,i);
		}
		public TerminalNode PLUS() { return getToken(BaseParser.PLUS, 0); }
		public TerminalNode MINUS() { return getToken(BaseParser.MINUS, 0); }
		public AddSubExprContext(ExprContext ctx) { copyFrom(ctx); }
	}

	public final ExprContext expr() throws RecognitionException {
		return expr(0);
	}

	private ExprContext expr(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		ExprContext _localctx = new ExprContext(_ctx, _parentState);
		ExprContext _prevctx = _localctx;
		int _startState = 78;
		enterRecursionRule(_localctx, 78, RULE_expr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(425);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,34,_ctx) ) {
			case 1:
				{
				_localctx = new FuncExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(417);
				functionCall();
				}
				break;
			case 2:
				{
				_localctx = new IntExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(418);
				match(INT);
				}
				break;
			case 3:
				{
				_localctx = new IdExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(419);
				match(ID);
				}
				break;
			case 4:
				{
				_localctx = new ParenExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(420);
				match(T__10);
				setState(421);
				expr(0);
				setState(422);
				match(T__11);
				}
				break;
			case 5:
				{
				_localctx = new RealExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(424);
				match(REAL);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(435);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,36,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(433);
					_errHandler.sync(this);
					switch ( getInterpreter().adaptivePredict(_input,35,_ctx) ) {
					case 1:
						{
						_localctx = new MulDivExprContext(new ExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expr);
						setState(427);
						if (!(precpred(_ctx, 7))) throw new FailedPredicateException(this, "precpred(_ctx, 7)");
						setState(428);
						_la = _input.LA(1);
						if ( !(_la==TIMES || _la==DIVIDE) ) {
						_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(429);
						expr(8);
						}
						break;
					case 2:
						{
						_localctx = new AddSubExprContext(new ExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expr);
						setState(430);
						if (!(precpred(_ctx, 6))) throw new FailedPredicateException(this, "precpred(_ctx, 6)");
						setState(431);
						_la = _input.LA(1);
						if ( !(_la==PLUS || _la==MINUS) ) {
						_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(432);
						expr(7);
						}
						break;
					}
					} 
				}
				setState(437);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,36,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ArrayDeclaratorContext extends ParserRuleContext {
		public ArrayDeclaratorContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrayDeclarator; }
	 
		public ArrayDeclaratorContext() { }
		public void copyFrom(ArrayDeclaratorContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class SizedArrayContext extends ArrayDeclaratorContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public TerminalNode INT() { return getToken(BaseParser.INT, 0); }
		public SizedArrayContext(ArrayDeclaratorContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class UnsizedArrayContext extends ArrayDeclaratorContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public UnsizedArrayContext(ArrayDeclaratorContext ctx) { copyFrom(ctx); }
	}

	public final ArrayDeclaratorContext arrayDeclarator() throws RecognitionException {
		ArrayDeclaratorContext _localctx = new ArrayDeclaratorContext(_ctx, getState());
		enterRule(_localctx, 80, RULE_arrayDeclarator);
		try {
			setState(445);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,37,_ctx) ) {
			case 1:
				_localctx = new SizedArrayContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(438);
				match(ID);
				setState(439);
				match(T__14);
				setState(440);
				match(INT);
				setState(441);
				match(T__16);
				}
				break;
			case 2:
				_localctx = new UnsizedArrayContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(442);
				match(ID);
				setState(443);
				match(T__14);
				setState(444);
				match(T__16);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ArrayInitializerContext extends ParserRuleContext {
		public List<ExprContext> expr() {
			return getRuleContexts(ExprContext.class);
		}
		public ExprContext expr(int i) {
			return getRuleContext(ExprContext.class,i);
		}
		public ArrayInitializerContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrayInitializer; }
	}

	public final ArrayInitializerContext arrayInitializer() throws RecognitionException {
		ArrayInitializerContext _localctx = new ArrayInitializerContext(_ctx, getState());
		enterRule(_localctx, 82, RULE_arrayInitializer);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(447);
			match(T__14);
			setState(448);
			expr(0);
			setState(453);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__5) {
				{
				{
				setState(449);
				match(T__5);
				setState(450);
				expr(0);
				}
				}
				setState(455);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(456);
			match(T__16);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public boolean sempred(RuleContext _localctx, int ruleIndex, int predIndex) {
		switch (ruleIndex) {
		case 13:
			return condition_sempred((ConditionContext)_localctx, predIndex);
		case 15:
			return graphCondition_sempred((GraphConditionContext)_localctx, predIndex);
		case 37:
			return printExpr_sempred((PrintExprContext)_localctx, predIndex);
		case 39:
			return expr_sempred((ExprContext)_localctx, predIndex);
		}
		return true;
	}
	private boolean condition_sempred(ConditionContext _localctx, int predIndex) {
		switch (predIndex) {
		case 0:
			return precpred(_ctx, 5);
		case 1:
			return precpred(_ctx, 4);
		}
		return true;
	}
	private boolean graphCondition_sempred(GraphConditionContext _localctx, int predIndex) {
		switch (predIndex) {
		case 2:
			return precpred(_ctx, 5);
		case 3:
			return precpred(_ctx, 4);
		}
		return true;
	}
	private boolean printExpr_sempred(PrintExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 4:
			return precpred(_ctx, 1);
		}
		return true;
	}
	private boolean expr_sempred(ExprContext _localctx, int predIndex) {
		switch (predIndex) {
		case 5:
			return precpred(_ctx, 7);
		case 6:
			return precpred(_ctx, 6);
		}
		return true;
	}

	public static final String _serializedATN =
		"\u0004\u0001>\u01cb\u0002\u0000\u0007\u0000\u0002\u0001\u0007\u0001\u0002"+
		"\u0002\u0007\u0002\u0002\u0003\u0007\u0003\u0002\u0004\u0007\u0004\u0002"+
		"\u0005\u0007\u0005\u0002\u0006\u0007\u0006\u0002\u0007\u0007\u0007\u0002"+
		"\b\u0007\b\u0002\t\u0007\t\u0002\n\u0007\n\u0002\u000b\u0007\u000b\u0002"+
		"\f\u0007\f\u0002\r\u0007\r\u0002\u000e\u0007\u000e\u0002\u000f\u0007\u000f"+
		"\u0002\u0010\u0007\u0010\u0002\u0011\u0007\u0011\u0002\u0012\u0007\u0012"+
		"\u0002\u0013\u0007\u0013\u0002\u0014\u0007\u0014\u0002\u0015\u0007\u0015"+
		"\u0002\u0016\u0007\u0016\u0002\u0017\u0007\u0017\u0002\u0018\u0007\u0018"+
		"\u0002\u0019\u0007\u0019\u0002\u001a\u0007\u001a\u0002\u001b\u0007\u001b"+
		"\u0002\u001c\u0007\u001c\u0002\u001d\u0007\u001d\u0002\u001e\u0007\u001e"+
		"\u0002\u001f\u0007\u001f\u0002 \u0007 \u0002!\u0007!\u0002\"\u0007\"\u0002"+
		"#\u0007#\u0002$\u0007$\u0002%\u0007%\u0002&\u0007&\u0002\'\u0007\'\u0002"+
		"(\u0007(\u0002)\u0007)\u0001\u0000\u0001\u0000\u0005\u0000W\b\u0000\n"+
		"\u0000\f\u0000Z\t\u0000\u0001\u0000\u0001\u0000\u0001\u0001\u0001\u0001"+
		"\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001"+
		"\u0001\u0001\u0001\u0001\u0001\u0001\u0003\u0001i\b\u0001\u0001\u0002"+
		"\u0001\u0002\u0001\u0002\u0001\u0002\u0003\u0002o\b\u0002\u0001\u0002"+
		"\u0003\u0002r\b\u0002\u0001\u0002\u0001\u0002\u0001\u0002\u0001\u0003"+
		"\u0001\u0003\u0001\u0003\u0001\u0003\u0001\u0004\u0001\u0004\u0001\u0004"+
		"\u0003\u0004~\b\u0004\u0001\u0004\u0001\u0004\u0001\u0005\u0001\u0005"+
		"\u0001\u0005\u0005\u0005\u0085\b\u0005\n\u0005\f\u0005\u0088\t\u0005\u0001"+
		"\u0006\u0001\u0006\u0001\u0006\u0005\u0006\u008d\b\u0006\n\u0006\f\u0006"+
		"\u0090\t\u0006\u0001\u0007\u0001\u0007\u0001\b\u0001\b\u0001\t\u0001\t"+
		"\u0001\t\u0001\n\u0001\n\u0001\n\u0001\n\u0001\u000b\u0001\u000b\u0001"+
		"\u000b\u0001\u000b\u0003\u000b\u00a1\b\u000b\u0001\u000b\u0001\u000b\u0001"+
		"\u000b\u0001\u000b\u0001\u000b\u0001\u000b\u0003\u000b\u00a9\b\u000b\u0001"+
		"\u000b\u0001\u000b\u0001\u000b\u0001\u000b\u0001\u000b\u0001\u000b\u0001"+
		"\u000b\u0003\u000b\u00b2\b\u000b\u0001\f\u0001\f\u0001\f\u0001\f\u0001"+
		"\f\u0001\f\u0001\f\u0001\f\u0001\f\u0003\f\u00bd\b\f\u0001\r\u0001\r\u0001"+
		"\r\u0001\r\u0001\r\u0001\r\u0001\r\u0001\r\u0001\r\u0001\r\u0001\r\u0001"+
		"\r\u0001\r\u0003\r\u00cc\b\r\u0001\r\u0001\r\u0001\r\u0001\r\u0001\r\u0001"+
		"\r\u0005\r\u00d4\b\r\n\r\f\r\u00d7\t\r\u0001\u000e\u0001\u000e\u0001\u000e"+
		"\u0001\u000e\u0001\u000e\u0001\u000e\u0001\u000e\u0001\u000e\u0001\u000e"+
		"\u0001\u000f\u0001\u000f\u0001\u000f\u0001\u000f\u0001\u000f\u0001\u000f"+
		"\u0001\u000f\u0001\u000f\u0001\u000f\u0001\u000f\u0001\u000f\u0003\u000f"+
		"\u00ed\b\u000f\u0001\u000f\u0001\u000f\u0001\u000f\u0001\u000f\u0001\u000f"+
		"\u0001\u000f\u0005\u000f\u00f5\b\u000f\n\u000f\f\u000f\u00f8\t\u000f\u0001"+
		"\u0010\u0001\u0010\u0003\u0010\u00fc\b\u0010\u0001\u0011\u0001\u0011\u0001"+
		"\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0001\u0012\u0001"+
		"\u0012\u0001\u0012\u0001\u0012\u0001\u0012\u0001\u0012\u0001\u0012\u0001"+
		"\u0012\u0001\u0012\u0001\u0012\u0003\u0012\u010f\b\u0012\u0001\u0013\u0001"+
		"\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0014\u0001"+
		"\u0014\u0003\u0014\u0119\b\u0014\u0001\u0015\u0001\u0015\u0001\u0015\u0001"+
		"\u0015\u0001\u0015\u0001\u0015\u0001\u0016\u0001\u0016\u0001\u0016\u0001"+
		"\u0016\u0001\u0016\u0001\u0016\u0001\u0017\u0001\u0017\u0001\u0017\u0001"+
		"\u0017\u0003\u0017\u012b\b\u0017\u0001\u0018\u0001\u0018\u0001\u0018\u0001"+
		"\u0018\u0003\u0018\u0131\b\u0018\u0001\u0019\u0001\u0019\u0001\u0019\u0001"+
		"\u0019\u0001\u0019\u0001\u0019\u0001\u0019\u0001\u0019\u0001\u001a\u0001"+
		"\u001a\u0001\u001a\u0001\u001a\u0001\u001b\u0001\u001b\u0001\u001b\u0001"+
		"\u001b\u0001\u001b\u0001\u001b\u0001\u001c\u0001\u001c\u0001\u001d\u0001"+
		"\u001d\u0001\u001d\u0001\u001d\u0005\u001d\u014b\b\u001d\n\u001d\f\u001d"+
		"\u014e\t\u001d\u0003\u001d\u0150\b\u001d\u0001\u001d\u0001\u001d\u0001"+
		"\u001e\u0001\u001e\u0001\u001e\u0001\u001f\u0001\u001f\u0001 \u0001 \u0001"+
		" \u0003 \u015c\b \u0001 \u0001 \u0001!\u0001!\u0001!\u0005!\u0163\b!\n"+
		"!\f!\u0166\t!\u0001\"\u0001\"\u0001\"\u0005\"\u016b\b\"\n\"\f\"\u016e"+
		"\t\"\u0001\"\u0001\"\u0001\"\u0003\"\u0173\b\"\u0001#\u0001#\u0001#\u0001"+
		"#\u0001$\u0001$\u0001$\u0001$\u0001$\u0003$\u017e\b$\u0001%\u0001%\u0001"+
		"%\u0003%\u0183\b%\u0001%\u0001%\u0001%\u0005%\u0188\b%\n%\f%\u018b\t%"+
		"\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001"+
		"&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0003&\u019f"+
		"\b&\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001"+
		"\'\u0003\'\u01aa\b\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0005"+
		"\'\u01b2\b\'\n\'\f\'\u01b5\t\'\u0001(\u0001(\u0001(\u0001(\u0001(\u0001"+
		"(\u0001(\u0003(\u01be\b(\u0001)\u0001)\u0001)\u0001)\u0005)\u01c4\b)\n"+
		")\f)\u01c7\t)\u0001)\u0001)\u0001)\u0000\u0004\u001a\u001eJN*\u0000\u0002"+
		"\u0004\u0006\b\n\f\u000e\u0010\u0012\u0014\u0016\u0018\u001a\u001c\u001e"+
		" \"$&(*,.02468:<>@BDFHJLNPR\u0000\u0005\u0001\u000038\u0003\u0000\u0017"+
		"\u0018#%))\u0004\u0000\u0017\u0018##%&))\u0001\u0000/0\u0001\u0000-.\u01dd"+
		"\u0000X\u0001\u0000\u0000\u0000\u0002h\u0001\u0000\u0000\u0000\u0004j"+
		"\u0001\u0000\u0000\u0000\u0006v\u0001\u0000\u0000\u0000\bz\u0001\u0000"+
		"\u0000\u0000\n\u0081\u0001\u0000\u0000\u0000\f\u0089\u0001\u0000\u0000"+
		"\u0000\u000e\u0091\u0001\u0000\u0000\u0000\u0010\u0093\u0001\u0000\u0000"+
		"\u0000\u0012\u0095\u0001\u0000\u0000\u0000\u0014\u0098\u0001\u0000\u0000"+
		"\u0000\u0016\u00b1\u0001\u0000\u0000\u0000\u0018\u00b3\u0001\u0000\u0000"+
		"\u0000\u001a\u00cb\u0001\u0000\u0000\u0000\u001c\u00d8\u0001\u0000\u0000"+
		"\u0000\u001e\u00ec\u0001\u0000\u0000\u0000 \u00fb\u0001\u0000\u0000\u0000"+
		"\"\u00fd\u0001\u0000\u0000\u0000$\u010e\u0001\u0000\u0000\u0000&\u0110"+
		"\u0001\u0000\u0000\u0000(\u0118\u0001\u0000\u0000\u0000*\u011a\u0001\u0000"+
		"\u0000\u0000,\u0120\u0001\u0000\u0000\u0000.\u012a\u0001\u0000\u0000\u0000"+
		"0\u0130\u0001\u0000\u0000\u00002\u0132\u0001\u0000\u0000\u00004\u013a"+
		"\u0001\u0000\u0000\u00006\u013e\u0001\u0000\u0000\u00008\u0144\u0001\u0000"+
		"\u0000\u0000:\u0146\u0001\u0000\u0000\u0000<\u0153\u0001\u0000\u0000\u0000"+
		">\u0156\u0001\u0000\u0000\u0000@\u0158\u0001\u0000\u0000\u0000B\u015f"+
		"\u0001\u0000\u0000\u0000D\u0172\u0001\u0000\u0000\u0000F\u0174\u0001\u0000"+
		"\u0000\u0000H\u017d\u0001\u0000\u0000\u0000J\u0182\u0001\u0000\u0000\u0000"+
		"L\u019e\u0001\u0000\u0000\u0000N\u01a9\u0001\u0000\u0000\u0000P\u01bd"+
		"\u0001\u0000\u0000\u0000R\u01bf\u0001\u0000\u0000\u0000TW\u0003\u0002"+
		"\u0001\u0000UW\u00036\u001b\u0000VT\u0001\u0000\u0000\u0000VU\u0001\u0000"+
		"\u0000\u0000WZ\u0001\u0000\u0000\u0000XV\u0001\u0000\u0000\u0000XY\u0001"+
		"\u0000\u0000\u0000Y[\u0001\u0000\u0000\u0000ZX\u0001\u0000\u0000\u0000"+
		"[\\\u0005\u0000\u0000\u0001\\\u0001\u0001\u0000\u0000\u0000]i\u0003\u0004"+
		"\u0002\u0000^i\u0003\u0018\f\u0000_i\u0003H$\u0000`i\u0003 \u0010\u0000"+
		"ai\u0003\u0016\u000b\u0000bi\u0003@ \u0000ci\u0003\u001c\u000e\u0000d"+
		"i\u00032\u0019\u0000ei\u00034\u001a\u0000fi\u0003(\u0014\u0000gi\u0005"+
		"\u0001\u0000\u0000h]\u0001\u0000\u0000\u0000h^\u0001\u0000\u0000\u0000"+
		"h_\u0001\u0000\u0000\u0000h`\u0001\u0000\u0000\u0000ha\u0001\u0000\u0000"+
		"\u0000hb\u0001\u0000\u0000\u0000hc\u0001\u0000\u0000\u0000hd\u0001\u0000"+
		"\u0000\u0000he\u0001\u0000\u0000\u0000hf\u0001\u0000\u0000\u0000hg\u0001"+
		"\u0000\u0000\u0000i\u0003\u0001\u0000\u0000\u0000jk\u0005)\u0000\u0000"+
		"kl\u0003\u000e\u0007\u0000ln\u0005\u0002\u0000\u0000mo\u0003\u0006\u0003"+
		"\u0000nm\u0001\u0000\u0000\u0000no\u0001\u0000\u0000\u0000oq\u0001\u0000"+
		"\u0000\u0000pr\u0003\b\u0004\u0000qp\u0001\u0000\u0000\u0000qr\u0001\u0000"+
		"\u0000\u0000rs\u0001\u0000\u0000\u0000st\u0005\u0003\u0000\u0000tu\u0005"+
		"\u0001\u0000\u0000u\u0005\u0001\u0000\u0000\u0000vw\u0005\u0004\u0000"+
		"\u0000wx\u0003\n\u0005\u0000xy\u0005\u0001\u0000\u0000y\u0007\u0001\u0000"+
		"\u0000\u0000z}\u0005\u0005\u0000\u0000{~\u0003\f\u0006\u0000|~\u0003\u0012"+
		"\t\u0000}{\u0001\u0000\u0000\u0000}|\u0001\u0000\u0000\u0000~\u007f\u0001"+
		"\u0000\u0000\u0000\u007f\u0080\u0005\u0001\u0000\u0000\u0080\t\u0001\u0000"+
		"\u0000\u0000\u0081\u0086\u0003\u0010\b\u0000\u0082\u0083\u0005\u0006\u0000"+
		"\u0000\u0083\u0085\u0003\u0010\b\u0000\u0084\u0082\u0001\u0000\u0000\u0000"+
		"\u0085\u0088\u0001\u0000\u0000\u0000\u0086\u0084\u0001\u0000\u0000\u0000"+
		"\u0086\u0087\u0001\u0000\u0000\u0000\u0087\u000b\u0001\u0000\u0000\u0000"+
		"\u0088\u0086\u0001\u0000\u0000\u0000\u0089\u008e\u0003\u0014\n\u0000\u008a"+
		"\u008b\u0005\u0006\u0000\u0000\u008b\u008d\u0003\u0014\n\u0000\u008c\u008a"+
		"\u0001\u0000\u0000\u0000\u008d\u0090\u0001\u0000\u0000\u0000\u008e\u008c"+
		"\u0001\u0000\u0000\u0000\u008e\u008f\u0001\u0000\u0000\u0000\u008f\r\u0001"+
		"\u0000\u0000\u0000\u0090\u008e\u0001\u0000\u0000\u0000\u0091\u0092\u0005"+
		"9\u0000\u0000\u0092\u000f\u0001\u0000\u0000\u0000\u0093\u0094\u0005:\u0000"+
		"\u0000\u0094\u0011\u0001\u0000\u0000\u0000\u0095\u0096\u0005\u0007\u0000"+
		"\u0000\u0096\u0097\u0005<\u0000\u0000\u0097\u0013\u0001\u0000\u0000\u0000"+
		"\u0098\u0099\u0003\u0010\b\u0000\u0099\u009a\u0005\b\u0000\u0000\u009a"+
		"\u009b\u0003\u0010\b\u0000\u009b\u0015\u0001\u0000\u0000\u0000\u009c\u009d"+
		"\u0003>\u001f\u0000\u009d\u00a0\u00059\u0000\u0000\u009e\u009f\u0005\t"+
		"\u0000\u0000\u009f\u00a1\u0003N\'\u0000\u00a0\u009e\u0001\u0000\u0000"+
		"\u0000\u00a0\u00a1\u0001\u0000\u0000\u0000\u00a1\u00a2\u0001\u0000\u0000"+
		"\u0000\u00a2\u00a3\u0005\u0001\u0000\u0000\u00a3\u00b2\u0001\u0000\u0000"+
		"\u0000\u00a4\u00a5\u0003>\u001f\u0000\u00a5\u00a8\u0003P(\u0000\u00a6"+
		"\u00a7\u0005\t\u0000\u0000\u00a7\u00a9\u0003R)\u0000\u00a8\u00a6\u0001"+
		"\u0000\u0000\u0000\u00a8\u00a9\u0001\u0000\u0000\u0000\u00a9\u00aa\u0001"+
		"\u0000\u0000\u0000\u00aa\u00ab\u0005\u0001\u0000\u0000\u00ab\u00b2\u0001"+
		"\u0000\u0000\u0000\u00ac\u00ad\u00059\u0000\u0000\u00ad\u00ae\u0005\t"+
		"\u0000\u0000\u00ae\u00af\u0003\u0016\u000b\u0000\u00af\u00b0\u0005\u0001"+
		"\u0000\u0000\u00b0\u00b2\u0001\u0000\u0000\u0000\u00b1\u009c\u0001\u0000"+
		"\u0000\u0000\u00b1\u00a4\u0001\u0000\u0000\u0000\u00b1\u00ac\u0001\u0000"+
		"\u0000\u0000\u00b2\u0017\u0001\u0000\u0000\u0000\u00b3\u00b4\u0005\n\u0000"+
		"\u0000\u00b4\u00b5\u0005\u000b\u0000\u0000\u00b5\u00b6\u0003\u001a\r\u0000"+
		"\u00b6\u00b7\u0005\f\u0000\u0000\u00b7\u00bc\u0003D\"\u0000\u00b8\u00b9"+
		"\u0005\r\u0000\u0000\u00b9\u00bd\u0003\u0018\f\u0000\u00ba\u00bb\u0005"+
		"\r\u0000\u0000\u00bb\u00bd\u0003D\"\u0000\u00bc\u00b8\u0001\u0000\u0000"+
		"\u0000\u00bc\u00ba\u0001\u0000\u0000\u0000\u00bc\u00bd\u0001\u0000\u0000"+
		"\u0000\u00bd\u0019\u0001\u0000\u0000\u0000\u00be\u00bf\u0006\r\uffff\uffff"+
		"\u0000\u00bf\u00c0\u0003N\'\u0000\u00c0\u00c1\u0007\u0000\u0000\u0000"+
		"\u00c1\u00c2\u0003N\'\u0000\u00c2\u00cc\u0001\u0000\u0000\u0000\u00c3"+
		"\u00c4\u0003\u0010\b\u0000\u00c4\u00c5\u0005\u000e\u0000\u0000\u00c5\u00c6"+
		"\u0003\u000e\u0007\u0000\u00c6\u00cc\u0001\u0000\u0000\u0000\u00c7\u00c8"+
		"\u0003\u0014\n\u0000\u00c8\u00c9\u0005\u000e\u0000\u0000\u00c9\u00ca\u0003"+
		"\u000e\u0007\u0000\u00ca\u00cc\u0001\u0000\u0000\u0000\u00cb\u00be\u0001"+
		"\u0000\u0000\u0000\u00cb\u00c3\u0001\u0000\u0000\u0000\u00cb\u00c7\u0001"+
		"\u0000\u0000\u0000\u00cc\u00d5\u0001\u0000\u0000\u0000\u00cd\u00ce\n\u0005"+
		"\u0000\u0000\u00ce\u00cf\u00051\u0000\u0000\u00cf\u00d4\u0003\u001a\r"+
		"\u0006\u00d0\u00d1\n\u0004\u0000\u0000\u00d1\u00d2\u00052\u0000\u0000"+
		"\u00d2\u00d4\u0003\u001a\r\u0005\u00d3\u00cd\u0001\u0000\u0000\u0000\u00d3"+
		"\u00d0\u0001\u0000\u0000\u0000\u00d4\u00d7\u0001\u0000\u0000\u0000\u00d5"+
		"\u00d3\u0001\u0000\u0000\u0000\u00d5\u00d6\u0001\u0000\u0000\u0000\u00d6"+
		"\u001b\u0001\u0000\u0000\u0000\u00d7\u00d5\u0001\u0000\u0000\u0000\u00d8"+
		"\u00d9\u00059\u0000\u0000\u00d9\u00da\u0005\t\u0000\u0000\u00da\u00db"+
		"\u0005\u000f\u0000\u0000\u00db\u00dc\u0003\u000e\u0007\u0000\u00dc\u00dd"+
		"\u0005\u0010\u0000\u0000\u00dd\u00de\u0003\u001e\u000f\u0000\u00de\u00df"+
		"\u0005\u0011\u0000\u0000\u00df\u00e0\u0005\u0001\u0000\u0000\u00e0\u001d"+
		"\u0001\u0000\u0000\u0000\u00e1\u00e2\u0006\u000f\uffff\uffff\u0000\u00e2"+
		"\u00e3\u0005\u0012\u0000\u0000\u00e3\u00e4\u0007\u0000\u0000\u0000\u00e4"+
		"\u00ed\u0005:\u0000\u0000\u00e5\u00e6\u0005\u0013\u0000\u0000\u00e6\u00e7"+
		"\u0005\u0014\u0000\u0000\u00e7\u00ed\u0003\u0010\b\u0000\u00e8\u00e9\u0005"+
		"\u000b\u0000\u0000\u00e9\u00ea\u0003\u001e\u000f\u0000\u00ea\u00eb\u0005"+
		"\f\u0000\u0000\u00eb\u00ed\u0001\u0000\u0000\u0000\u00ec\u00e1\u0001\u0000"+
		"\u0000\u0000\u00ec\u00e5\u0001\u0000\u0000\u0000\u00ec\u00e8\u0001\u0000"+
		"\u0000\u0000\u00ed\u00f6\u0001\u0000\u0000\u0000\u00ee\u00ef\n\u0005\u0000"+
		"\u0000\u00ef\u00f0\u00051\u0000\u0000\u00f0\u00f5\u0003\u001e\u000f\u0006"+
		"\u00f1\u00f2\n\u0004\u0000\u0000\u00f2\u00f3\u00052\u0000\u0000\u00f3"+
		"\u00f5\u0003\u001e\u000f\u0005\u00f4\u00ee\u0001\u0000\u0000\u0000\u00f4"+
		"\u00f1\u0001\u0000\u0000\u0000\u00f5\u00f8\u0001\u0000\u0000\u0000\u00f6"+
		"\u00f4\u0001\u0000\u0000\u0000\u00f6\u00f7\u0001\u0000\u0000\u0000\u00f7"+
		"\u001f\u0001\u0000\u0000\u0000\u00f8\u00f6\u0001\u0000\u0000\u0000\u00f9"+
		"\u00fc\u0003\"\u0011\u0000\u00fa\u00fc\u0003&\u0013\u0000\u00fb\u00f9"+
		"\u0001\u0000\u0000\u0000\u00fb\u00fa\u0001\u0000\u0000\u0000\u00fc!\u0001"+
		"\u0000\u0000\u0000\u00fd\u00fe\u0005\u0015\u0000\u0000\u00fe\u00ff\u0005"+
		"\u0016\u0000\u0000\u00ff\u0100\u0003$\u0012\u0000\u0100\u0101\u0005\u000e"+
		"\u0000\u0000\u0101\u0102\u0003\u000e\u0007\u0000\u0102\u0103\u0003D\""+
		"\u0000\u0103#\u0001\u0000\u0000\u0000\u0104\u0105\u0005\u0017\u0000\u0000"+
		"\u0105\u010f\u00059\u0000\u0000\u0106\u0107\u0005\u0018\u0000\u0000\u0107"+
		"\u0108\u00059\u0000\u0000\u0108\u0109\u0005\u0006\u0000\u0000\u0109\u010f"+
		"\u00059\u0000\u0000\u010a\u010b\u0005\u0019\u0000\u0000\u010b\u010c\u0005"+
		"9\u0000\u0000\u010c\u010d\u0005,\u0000\u0000\u010d\u010f\u0003\u0010\b"+
		"\u0000\u010e\u0104\u0001\u0000\u0000\u0000\u010e\u0106\u0001\u0000\u0000"+
		"\u0000\u010e\u010a\u0001\u0000\u0000\u0000\u010f%\u0001\u0000\u0000\u0000"+
		"\u0110\u0111\u0005\u001a\u0000\u0000\u0111\u0112\u0005\u000b\u0000\u0000"+
		"\u0112\u0113\u0003\u001a\r\u0000\u0113\u0114\u0005\f\u0000\u0000\u0114"+
		"\u0115\u0003D\"\u0000\u0115\'\u0001\u0000\u0000\u0000\u0116\u0119\u0003"+
		"*\u0015\u0000\u0117\u0119\u0003,\u0016\u0000\u0118\u0116\u0001\u0000\u0000"+
		"\u0000\u0118\u0117\u0001\u0000\u0000\u0000\u0119)\u0001\u0000\u0000\u0000"+
		"\u011a\u011b\u0005\u001b\u0000\u0000\u011b\u011c\u0003.\u0017\u0000\u011c"+
		"\u011d\u0005\u001c\u0000\u0000\u011d\u011e\u0003\u000e\u0007\u0000\u011e"+
		"\u011f\u0005\u0001\u0000\u0000\u011f+\u0001\u0000\u0000\u0000\u0120\u0121"+
		"\u0005\u001d\u0000\u0000\u0121\u0122\u00030\u0018\u0000\u0122\u0123\u0005"+
		"\u001e\u0000\u0000\u0123\u0124\u0003\u000e\u0007\u0000\u0124\u0125\u0005"+
		"\u0001\u0000\u0000\u0125-\u0001\u0000\u0000\u0000\u0126\u012b\u0003\u0010"+
		"\b\u0000\u0127\u012b\u0003\u0014\n\u0000\u0128\u012b\u0003\n\u0005\u0000"+
		"\u0129\u012b\u0003\f\u0006\u0000\u012a\u0126\u0001\u0000\u0000\u0000\u012a"+
		"\u0127\u0001\u0000\u0000\u0000\u012a\u0128\u0001\u0000\u0000\u0000\u012a"+
		"\u0129\u0001\u0000\u0000\u0000\u012b/\u0001\u0000\u0000\u0000\u012c\u0131"+
		"\u0003\u0010\b\u0000\u012d\u0131\u0003\u0014\n\u0000\u012e\u0131\u0003"+
		"\n\u0005\u0000\u012f\u0131\u0003\f\u0006\u0000\u0130\u012c\u0001\u0000"+
		"\u0000\u0000\u0130\u012d\u0001\u0000\u0000\u0000\u0130\u012e\u0001\u0000"+
		"\u0000\u0000\u0130\u012f\u0001\u0000\u0000\u0000\u01311\u0001\u0000\u0000"+
		"\u0000\u0132\u0133\u0005\u001f\u0000\u0000\u0133\u0134\u00059\u0000\u0000"+
		"\u0134\u0135\u0005 \u0000\u0000\u0135\u0136\u0005<\u0000\u0000\u0136\u0137"+
		"\u0005,\u0000\u0000\u0137\u0138\u0003\u000e\u0007\u0000\u0138\u0139\u0005"+
		"\u0001\u0000\u0000\u01393\u0001\u0000\u0000\u0000\u013a\u013b\u0005!\u0000"+
		"\u0000\u013b\u013c\u0003\u000e\u0007\u0000\u013c\u013d\u0005\u0001\u0000"+
		"\u0000\u013d5\u0001\u0000\u0000\u0000\u013e\u013f\u0005\"\u0000\u0000"+
		"\u013f\u0140\u00038\u001c\u0000\u0140\u0141\u00059\u0000\u0000\u0141\u0142"+
		"\u0003:\u001d\u0000\u0142\u0143\u0003D\"\u0000\u01437\u0001\u0000\u0000"+
		"\u0000\u0144\u0145\u0007\u0001\u0000\u0000\u01459\u0001\u0000\u0000\u0000"+
		"\u0146\u014f\u0005\u000b\u0000\u0000\u0147\u014c\u0003<\u001e\u0000\u0148"+
		"\u0149\u0005\u0006\u0000\u0000\u0149\u014b\u0003<\u001e\u0000\u014a\u0148"+
		"\u0001\u0000\u0000\u0000\u014b\u014e\u0001\u0000\u0000\u0000\u014c\u014a"+
		"\u0001\u0000\u0000\u0000\u014c\u014d\u0001\u0000\u0000\u0000\u014d\u0150"+
		"\u0001\u0000\u0000\u0000\u014e\u014c\u0001\u0000\u0000\u0000\u014f\u0147"+
		"\u0001\u0000\u0000\u0000\u014f\u0150\u0001\u0000\u0000\u0000\u0150\u0151"+
		"\u0001\u0000\u0000\u0000\u0151\u0152\u0005\f\u0000\u0000\u0152;\u0001"+
		"\u0000\u0000\u0000\u0153\u0154\u0003>\u001f\u0000\u0154\u0155\u00059\u0000"+
		"\u0000\u0155=\u0001\u0000\u0000\u0000\u0156\u0157\u0007\u0002\u0000\u0000"+
		"\u0157?\u0001\u0000\u0000\u0000\u0158\u0159\u00059\u0000\u0000\u0159\u015b"+
		"\u0005\u000b\u0000\u0000\u015a\u015c\u0003B!\u0000\u015b\u015a\u0001\u0000"+
		"\u0000\u0000\u015b\u015c\u0001\u0000\u0000\u0000\u015c\u015d\u0001\u0000"+
		"\u0000\u0000\u015d\u015e\u0005\f\u0000\u0000\u015eA\u0001\u0000\u0000"+
		"\u0000\u015f\u0164\u0003N\'\u0000\u0160\u0161\u0005\u0006\u0000\u0000"+
		"\u0161\u0163\u0003N\'\u0000\u0162\u0160\u0001\u0000\u0000\u0000\u0163"+
		"\u0166\u0001\u0000\u0000\u0000\u0164\u0162\u0001\u0000\u0000\u0000\u0164"+
		"\u0165\u0001\u0000\u0000\u0000\u0165C\u0001\u0000\u0000\u0000\u0166\u0164"+
		"\u0001\u0000\u0000\u0000\u0167\u016c\u0005\u0002\u0000\u0000\u0168\u016b"+
		"\u0003\u0002\u0001\u0000\u0169\u016b\u0003F#\u0000\u016a\u0168\u0001\u0000"+
		"\u0000\u0000\u016a\u0169\u0001\u0000\u0000\u0000\u016b\u016e\u0001\u0000"+
		"\u0000\u0000\u016c\u016a\u0001\u0000\u0000\u0000\u016c\u016d\u0001\u0000"+
		"\u0000\u0000\u016d\u016f\u0001\u0000\u0000\u0000\u016e\u016c\u0001\u0000"+
		"\u0000\u0000\u016f\u0173\u0005\u0003\u0000\u0000\u0170\u0171\u0005\u0002"+
		"\u0000\u0000\u0171\u0173\u0005\u0003\u0000\u0000\u0172\u0167\u0001\u0000"+
		"\u0000\u0000\u0172\u0170\u0001\u0000\u0000\u0000\u0173E\u0001\u0000\u0000"+
		"\u0000\u0174\u0175\u0005\'\u0000\u0000\u0175\u0176\u0003N\'\u0000\u0176"+
		"\u0177\u0005\u0001\u0000\u0000\u0177G\u0001\u0000\u0000\u0000\u0178\u0179"+
		"\u0005(\u0000\u0000\u0179\u017a\u0003J%\u0000\u017a\u017b\u0005\u0001"+
		"\u0000\u0000\u017b\u017e\u0001\u0000\u0000\u0000\u017c\u017e\u0003L&\u0000"+
		"\u017d\u0178\u0001\u0000\u0000\u0000\u017d\u017c\u0001\u0000\u0000\u0000"+
		"\u017eI\u0001\u0000\u0000\u0000\u017f\u0180\u0006%\uffff\uffff\u0000\u0180"+
		"\u0183\u0005<\u0000\u0000\u0181\u0183\u0003N\'\u0000\u0182\u017f\u0001"+
		"\u0000\u0000\u0000\u0182\u0181\u0001\u0000\u0000\u0000\u0183\u0189\u0001"+
		"\u0000\u0000\u0000\u0184\u0185\n\u0001\u0000\u0000\u0185\u0186\u0005-"+
		"\u0000\u0000\u0186\u0188\u0003J%\u0002\u0187\u0184\u0001\u0000\u0000\u0000"+
		"\u0188\u018b\u0001\u0000\u0000\u0000\u0189\u0187\u0001\u0000\u0000\u0000"+
		"\u0189\u018a\u0001\u0000\u0000\u0000\u018aK\u0001\u0000\u0000\u0000\u018b"+
		"\u0189\u0001\u0000\u0000\u0000\u018c\u018d\u0005(\u0000\u0000\u018d\u018e"+
		"\u0005*\u0000\u0000\u018e\u018f\u0005,\u0000\u0000\u018f\u0190\u0003\u000e"+
		"\u0007\u0000\u0190\u0191\u0005\u0001\u0000\u0000\u0191\u019f\u0001\u0000"+
		"\u0000\u0000\u0192\u0193\u0005(\u0000\u0000\u0193\u0194\u0005+\u0000\u0000"+
		"\u0194\u0195\u0005,\u0000\u0000\u0195\u0196\u0003\u000e\u0007\u0000\u0196"+
		"\u0197\u0005\u0001\u0000\u0000\u0197\u019f\u0001\u0000\u0000\u0000\u0198"+
		"\u0199\u0005(\u0000\u0000\u0199\u019a\u0005)\u0000\u0000\u019a\u019b\u0005"+
		",\u0000\u0000\u019b\u019c\u0003\u000e\u0007\u0000\u019c\u019d\u0005\u0001"+
		"\u0000\u0000\u019d\u019f\u0001\u0000\u0000\u0000\u019e\u018c\u0001\u0000"+
		"\u0000\u0000\u019e\u0192\u0001\u0000\u0000\u0000\u019e\u0198\u0001\u0000"+
		"\u0000\u0000\u019fM\u0001\u0000\u0000\u0000\u01a0\u01a1\u0006\'\uffff"+
		"\uffff\u0000\u01a1\u01aa\u0003@ \u0000\u01a2\u01aa\u0005:\u0000\u0000"+
		"\u01a3\u01aa\u00059\u0000\u0000\u01a4\u01a5\u0005\u000b\u0000\u0000\u01a5"+
		"\u01a6\u0003N\'\u0000\u01a6\u01a7\u0005\f\u0000\u0000\u01a7\u01aa\u0001"+
		"\u0000\u0000\u0000\u01a8\u01aa\u0005;\u0000\u0000\u01a9\u01a0\u0001\u0000"+
		"\u0000\u0000\u01a9\u01a2\u0001\u0000\u0000\u0000\u01a9\u01a3\u0001\u0000"+
		"\u0000\u0000\u01a9\u01a4\u0001\u0000\u0000\u0000\u01a9\u01a8\u0001\u0000"+
		"\u0000\u0000\u01aa\u01b3\u0001\u0000\u0000\u0000\u01ab\u01ac\n\u0007\u0000"+
		"\u0000\u01ac\u01ad\u0007\u0003\u0000\u0000\u01ad\u01b2\u0003N\'\b\u01ae"+
		"\u01af\n\u0006\u0000\u0000\u01af\u01b0\u0007\u0004\u0000\u0000\u01b0\u01b2"+
		"\u0003N\'\u0007\u01b1\u01ab\u0001\u0000\u0000\u0000\u01b1\u01ae\u0001"+
		"\u0000\u0000\u0000\u01b2\u01b5\u0001\u0000\u0000\u0000\u01b3\u01b1\u0001"+
		"\u0000\u0000\u0000\u01b3\u01b4\u0001\u0000\u0000\u0000\u01b4O\u0001\u0000"+
		"\u0000\u0000\u01b5\u01b3\u0001\u0000\u0000\u0000\u01b6\u01b7\u00059\u0000"+
		"\u0000\u01b7\u01b8\u0005\u000f\u0000\u0000\u01b8\u01b9\u0005:\u0000\u0000"+
		"\u01b9\u01be\u0005\u0011\u0000\u0000\u01ba\u01bb\u00059\u0000\u0000\u01bb"+
		"\u01bc\u0005\u000f\u0000\u0000\u01bc\u01be\u0005\u0011\u0000\u0000\u01bd"+
		"\u01b6\u0001\u0000\u0000\u0000\u01bd\u01ba\u0001\u0000\u0000\u0000\u01be"+
		"Q\u0001\u0000\u0000\u0000\u01bf\u01c0\u0005\u000f\u0000\u0000\u01c0\u01c5"+
		"\u0003N\'\u0000\u01c1\u01c2\u0005\u0006\u0000\u0000\u01c2\u01c4\u0003"+
		"N\'\u0000\u01c3\u01c1\u0001\u0000\u0000\u0000\u01c4\u01c7\u0001\u0000"+
		"\u0000\u0000\u01c5\u01c3\u0001\u0000\u0000\u0000\u01c5\u01c6\u0001\u0000"+
		"\u0000\u0000\u01c6\u01c8\u0001\u0000\u0000\u0000\u01c7\u01c5\u0001\u0000"+
		"\u0000\u0000\u01c8\u01c9\u0005\u0011\u0000\u0000\u01c9S\u0001\u0000\u0000"+
		"\u0000\'VXhnq}\u0086\u008e\u00a0\u00a8\u00b1\u00bc\u00cb\u00d3\u00d5\u00ec"+
		"\u00f4\u00f6\u00fb\u010e\u0118\u012a\u0130\u014c\u014f\u015b\u0164\u016a"+
		"\u016c\u0172\u017d\u0182\u0189\u019e\u01a9\u01b1\u01b3\u01bd\u01c5";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}