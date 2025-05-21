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
		T__38=39, T__39=40, T__40=41, GRAPH=42, DIRGRAPH=43, EDGE=44, NODE=45, 
		TRUE=46, FALSE=47, OF=48, PLUS=49, MINUS=50, TIMES=51, DIVIDE=52, AND=53, 
		OR=54, EQUAL=55, NOTEQUAL=56, LESSTHAN=57, GREATERTHAN=58, LESSEQUAL=59, 
		GREATEREQUAL=60, ID=61, INT=62, REAL=63, STRING=64, Comment=65, WS=66;
	public static final int
		RULE_program = 0, RULE_statement = 1, RULE_graphDef = 2, RULE_undirectedEdge = 3, 
		RULE_undirectedEdgeList = 4, RULE_undirectedGraphDef = 5, RULE_undirectedEdges = 6, 
		RULE_nodes = 7, RULE_edges = 8, RULE_nodeList = 9, RULE_edgeList = 10, 
		RULE_graphID = 11, RULE_nodeID = 12, RULE_fileEdgeList = 13, RULE_edge = 14, 
		RULE_varDecl = 15, RULE_conditionalStatement = 16, RULE_condition = 17, 
		RULE_graphComprehension = 18, RULE_graphCondition = 19, RULE_loopStatement = 20, 
		RULE_foreachStatement = 21, RULE_loopTarget = 22, RULE_whileStatement = 23, 
		RULE_nodeEdgeOperation = 24, RULE_addOperation = 25, RULE_removeOperation = 26, 
		RULE_addTargets = 27, RULE_removeTargets = 28, RULE_queryStatement = 29, 
		RULE_showgraph = 30, RULE_function = 31, RULE_returnType = 32, RULE_paramList = 33, 
		RULE_param = 34, RULE_type = 35, RULE_functionCall = 36, RULE_argumentList = 37, 
		RULE_block = 38, RULE_returnStatement = 39, RULE_printStatement = 40, 
		RULE_printExpr = 41, RULE_printgraph = 42, RULE_expr = 43, RULE_arrayDeclarator = 44, 
		RULE_declaration = 45, RULE_arrayInitializer = 46, RULE_assignmentStatement = 47, 
		RULE_arrayAssignStatement = 48;
	private static String[] makeRuleNames() {
		return new String[] {
			"program", "statement", "graphDef", "undirectedEdge", "undirectedEdgeList", 
			"undirectedGraphDef", "undirectedEdges", "nodes", "edges", "nodeList", 
			"edgeList", "graphID", "nodeID", "fileEdgeList", "edge", "varDecl", "conditionalStatement", 
			"condition", "graphComprehension", "graphCondition", "loopStatement", 
			"foreachStatement", "loopTarget", "whileStatement", "nodeEdgeOperation", 
			"addOperation", "removeOperation", "addTargets", "removeTargets", "queryStatement", 
			"showgraph", "function", "returnType", "paramList", "param", "type", 
			"functionCall", "argumentList", "block", "returnStatement", "printStatement", 
			"printExpr", "printgraph", "expr", "arrayDeclarator", "declaration", 
			"arrayInitializer", "assignmentStatement", "arrayAssignStatement"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "';'", "'{'", "'}'", "','", "'edges:'", "'nodes:'", "'file'", "'->'", 
			"'='", "'if'", "'('", "')'", "'else'", "'in'", "'['", "'where'", "']'", 
			"'degree'", "'connected'", "'with'", "'for'", "'each'", "'vertex'", "'edge'", 
			"'neighbor'", "'while'", "'add'", "'to'", "'remove'", "'from'", "'query'", 
			"':'", "'show'", "'fn'", "'int'", "'void'", "'string'", "'real'", "'bool'", 
			"'return'", "'print'", "'graph'", "'dirgraph'", "'edges'", "'nodes'", 
			"'TRUE'", "'FALSE'", "'of'", "'+'", "'-'", "'*'", "'/'", "'&&'", "'||'", 
			"'=='", "'!='", "'<'", "'>'", "'<='", "'>='"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, null, null, "GRAPH", "DIRGRAPH", "EDGE", "NODE", 
			"TRUE", "FALSE", "OF", "PLUS", "MINUS", "TIMES", "DIVIDE", "AND", "OR", 
			"EQUAL", "NOTEQUAL", "LESSTHAN", "GREATERTHAN", "LESSEQUAL", "GREATEREQUAL", 
			"ID", "INT", "REAL", "STRING", "Comment", "WS"
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
			setState(102);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & 2305859427491644418L) != 0)) {
				{
				setState(100);
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
				case T__38:
				case T__40:
				case GRAPH:
				case DIRGRAPH:
				case ID:
					{
					setState(98);
					statement();
					}
					break;
				case T__33:
					{
					setState(99);
					function();
					}
					break;
				default:
					throw new NoViableAltException(this);
				}
				}
				setState(104);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(105);
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
		public UndirectedGraphDefContext undirectedGraphDef() {
			return getRuleContext(UndirectedGraphDefContext.class,0);
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
		public ArrayAssignStatementContext arrayAssignStatement() {
			return getRuleContext(ArrayAssignStatementContext.class,0);
		}
		public AssignmentStatementContext assignmentStatement() {
			return getRuleContext(AssignmentStatementContext.class,0);
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
			setState(121);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,2,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(107);
				graphDef();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(108);
				undirectedGraphDef();
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(109);
				conditionalStatement();
				}
				break;
			case 4:
				enterOuterAlt(_localctx, 4);
				{
				setState(110);
				printStatement();
				}
				break;
			case 5:
				enterOuterAlt(_localctx, 5);
				{
				setState(111);
				loopStatement();
				}
				break;
			case 6:
				enterOuterAlt(_localctx, 6);
				{
				setState(112);
				varDecl();
				}
				break;
			case 7:
				enterOuterAlt(_localctx, 7);
				{
				setState(113);
				functionCall();
				}
				break;
			case 8:
				enterOuterAlt(_localctx, 8);
				{
				setState(114);
				graphComprehension();
				}
				break;
			case 9:
				enterOuterAlt(_localctx, 9);
				{
				setState(115);
				arrayAssignStatement();
				}
				break;
			case 10:
				enterOuterAlt(_localctx, 10);
				{
				setState(116);
				assignmentStatement();
				}
				break;
			case 11:
				enterOuterAlt(_localctx, 11);
				{
				setState(117);
				queryStatement();
				}
				break;
			case 12:
				enterOuterAlt(_localctx, 12);
				{
				setState(118);
				showgraph();
				}
				break;
			case 13:
				enterOuterAlt(_localctx, 13);
				{
				setState(119);
				nodeEdgeOperation();
				}
				break;
			case 14:
				enterOuterAlt(_localctx, 14);
				{
				setState(120);
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
		public TerminalNode DIRGRAPH() { return getToken(BaseParser.DIRGRAPH, 0); }
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
			setState(123);
			match(DIRGRAPH);
			setState(124);
			graphID();
			setState(125);
			match(T__1);
			setState(127);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==T__5) {
				{
				setState(126);
				nodes();
				}
			}

			setState(130);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==T__4) {
				{
				setState(129);
				edges();
				}
			}

			setState(132);
			match(T__2);
			setState(133);
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
	public static class UndirectedEdgeContext extends ParserRuleContext {
		public List<NodeIDContext> nodeID() {
			return getRuleContexts(NodeIDContext.class);
		}
		public NodeIDContext nodeID(int i) {
			return getRuleContext(NodeIDContext.class,i);
		}
		public UndirectedEdgeContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_undirectedEdge; }
	}

	public final UndirectedEdgeContext undirectedEdge() throws RecognitionException {
		UndirectedEdgeContext _localctx = new UndirectedEdgeContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_undirectedEdge);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(135);
			nodeID();
			setState(136);
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
	public static class UndirectedEdgeListContext extends ParserRuleContext {
		public List<UndirectedEdgeContext> undirectedEdge() {
			return getRuleContexts(UndirectedEdgeContext.class);
		}
		public UndirectedEdgeContext undirectedEdge(int i) {
			return getRuleContext(UndirectedEdgeContext.class,i);
		}
		public UndirectedEdgeListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_undirectedEdgeList; }
	}

	public final UndirectedEdgeListContext undirectedEdgeList() throws RecognitionException {
		UndirectedEdgeListContext _localctx = new UndirectedEdgeListContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_undirectedEdgeList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(138);
			undirectedEdge();
			setState(143);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__3) {
				{
				{
				setState(139);
				match(T__3);
				setState(140);
				undirectedEdge();
				}
				}
				setState(145);
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
	public static class UndirectedGraphDefContext extends ParserRuleContext {
		public TerminalNode GRAPH() { return getToken(BaseParser.GRAPH, 0); }
		public GraphIDContext graphID() {
			return getRuleContext(GraphIDContext.class,0);
		}
		public NodesContext nodes() {
			return getRuleContext(NodesContext.class,0);
		}
		public UndirectedEdgesContext undirectedEdges() {
			return getRuleContext(UndirectedEdgesContext.class,0);
		}
		public UndirectedGraphDefContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_undirectedGraphDef; }
	}

	public final UndirectedGraphDefContext undirectedGraphDef() throws RecognitionException {
		UndirectedGraphDefContext _localctx = new UndirectedGraphDefContext(_ctx, getState());
		enterRule(_localctx, 10, RULE_undirectedGraphDef);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(146);
			match(GRAPH);
			setState(147);
			graphID();
			setState(148);
			match(T__1);
			setState(150);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==T__5) {
				{
				setState(149);
				nodes();
				}
			}

			setState(153);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==T__4) {
				{
				setState(152);
				undirectedEdges();
				}
			}

			setState(155);
			match(T__2);
			setState(156);
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
	public static class UndirectedEdgesContext extends ParserRuleContext {
		public UndirectedEdgeListContext undirectedEdgeList() {
			return getRuleContext(UndirectedEdgeListContext.class,0);
		}
		public FileEdgeListContext fileEdgeList() {
			return getRuleContext(FileEdgeListContext.class,0);
		}
		public UndirectedEdgesContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_undirectedEdges; }
	}

	public final UndirectedEdgesContext undirectedEdges() throws RecognitionException {
		UndirectedEdgesContext _localctx = new UndirectedEdgesContext(_ctx, getState());
		enterRule(_localctx, 12, RULE_undirectedEdges);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(158);
			match(T__4);
			setState(161);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case INT:
				{
				setState(159);
				undirectedEdgeList();
				}
				break;
			case T__6:
				{
				setState(160);
				fileEdgeList();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			setState(163);
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
		enterRule(_localctx, 14, RULE_nodes);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(165);
			match(T__5);
			setState(166);
			nodeList();
			setState(167);
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
		enterRule(_localctx, 16, RULE_edges);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(169);
			match(T__4);
			setState(172);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case INT:
				{
				setState(170);
				edgeList();
				}
				break;
			case T__6:
				{
				setState(171);
				fileEdgeList();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			setState(174);
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
		enterRule(_localctx, 18, RULE_nodeList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(176);
			nodeID();
			setState(181);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__3) {
				{
				{
				setState(177);
				match(T__3);
				setState(178);
				nodeID();
				}
				}
				setState(183);
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
		enterRule(_localctx, 20, RULE_edgeList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(184);
			edge();
			setState(189);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__3) {
				{
				{
				setState(185);
				match(T__3);
				setState(186);
				edge();
				}
				}
				setState(191);
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
		enterRule(_localctx, 22, RULE_graphID);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(192);
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
		enterRule(_localctx, 24, RULE_nodeID);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(194);
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
		enterRule(_localctx, 26, RULE_fileEdgeList);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(196);
			match(T__6);
			setState(197);
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
		enterRule(_localctx, 28, RULE_edge);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(199);
			nodeID();
			setState(200);
			match(T__7);
			setState(201);
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

	public final VarDeclContext varDecl() throws RecognitionException {
		VarDeclContext _localctx = new VarDeclContext(_ctx, getState());
		enterRule(_localctx, 30, RULE_varDecl);
		int _la;
		try {
			setState(219);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,14,_ctx) ) {
			case 1:
				_localctx = new SimpleDeclarationContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(203);
				type();
				setState(204);
				match(ID);
				setState(207);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==T__8) {
					{
					setState(205);
					match(T__8);
					setState(206);
					expr(0);
					}
				}

				setState(209);
				match(T__0);
				}
				break;
			case 2:
				_localctx = new ArrayDeclarationContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(211);
				type();
				setState(212);
				arrayDeclarator();
				setState(215);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==T__8) {
					{
					setState(213);
					match(T__8);
					setState(214);
					arrayInitializer();
					}
				}

				setState(217);
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
		enterRule(_localctx, 32, RULE_conditionalStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(221);
			match(T__9);
			setState(222);
			match(T__10);
			setState(223);
			condition(0);
			setState(224);
			match(T__11);
			setState(225);
			block();
			setState(230);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,15,_ctx) ) {
			case 1:
				{
				setState(226);
				match(T__12);
				setState(227);
				conditionalStatement();
				}
				break;
			case 2:
				{
				setState(228);
				match(T__12);
				setState(229);
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
		int _startState = 34;
		enterRecursionRule(_localctx, 34, RULE_condition, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(245);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,16,_ctx) ) {
			case 1:
				{
				_localctx = new RelationalContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(233);
				expr(0);
				setState(234);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 2269814212194729984L) != 0)) ) {
				_errHandler.recoverInline(this);
				}
				else {
					if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
					_errHandler.reportMatch(this);
					consume();
				}
				setState(235);
				expr(0);
				}
				break;
			case 2:
				{
				_localctx = new NodeCheckContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(237);
				nodeID();
				setState(238);
				match(T__13);
				setState(239);
				graphID();
				}
				break;
			case 3:
				{
				_localctx = new EdgeCheckContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(241);
				edge();
				setState(242);
				match(T__13);
				setState(243);
				graphID();
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(255);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,18,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(253);
					_errHandler.sync(this);
					switch ( getInterpreter().adaptivePredict(_input,17,_ctx) ) {
					case 1:
						{
						_localctx = new LogicalAndContext(new ConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_condition);
						setState(247);
						if (!(precpred(_ctx, 5))) throw new FailedPredicateException(this, "precpred(_ctx, 5)");
						setState(248);
						match(AND);
						setState(249);
						condition(6);
						}
						break;
					case 2:
						{
						_localctx = new LogicalOrContext(new ConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_condition);
						setState(250);
						if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
						setState(251);
						match(OR);
						setState(252);
						condition(5);
						}
						break;
					}
					} 
				}
				setState(257);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,18,_ctx);
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
		enterRule(_localctx, 36, RULE_graphComprehension);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(258);
			match(ID);
			setState(259);
			match(T__8);
			setState(260);
			match(T__14);
			setState(261);
			graphID();
			setState(262);
			match(T__15);
			setState(263);
			graphCondition(0);
			setState(264);
			match(T__16);
			setState(265);
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
		int _startState = 38;
		enterRecursionRule(_localctx, 38, RULE_graphCondition, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(278);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__17:
				{
				_localctx = new DegreeConditionContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(268);
				match(T__17);
				setState(269);
				_la = _input.LA(1);
				if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 2269814212194729984L) != 0)) ) {
				_errHandler.recoverInline(this);
				}
				else {
					if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
					_errHandler.reportMatch(this);
					consume();
				}
				setState(270);
				match(INT);
				}
				break;
			case T__18:
				{
				_localctx = new ConnectedConditionContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(271);
				match(T__18);
				setState(272);
				match(T__19);
				setState(273);
				nodeID();
				}
				break;
			case T__10:
				{
				_localctx = new ParenGraphConditionContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(274);
				match(T__10);
				setState(275);
				graphCondition(0);
				setState(276);
				match(T__11);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
			_ctx.stop = _input.LT(-1);
			setState(288);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,21,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(286);
					_errHandler.sync(this);
					switch ( getInterpreter().adaptivePredict(_input,20,_ctx) ) {
					case 1:
						{
						_localctx = new GraphLogicalAndContext(new GraphConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_graphCondition);
						setState(280);
						if (!(precpred(_ctx, 5))) throw new FailedPredicateException(this, "precpred(_ctx, 5)");
						setState(281);
						match(AND);
						setState(282);
						graphCondition(6);
						}
						break;
					case 2:
						{
						_localctx = new GraphLogicalOrContext(new GraphConditionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_graphCondition);
						setState(283);
						if (!(precpred(_ctx, 4))) throw new FailedPredicateException(this, "precpred(_ctx, 4)");
						setState(284);
						match(OR);
						setState(285);
						graphCondition(5);
						}
						break;
					}
					} 
				}
				setState(290);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,21,_ctx);
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
		enterRule(_localctx, 40, RULE_loopStatement);
		try {
			setState(293);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__20:
				enterOuterAlt(_localctx, 1);
				{
				setState(291);
				foreachStatement();
				}
				break;
			case T__25:
				enterOuterAlt(_localctx, 2);
				{
				setState(292);
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
		enterRule(_localctx, 42, RULE_foreachStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(295);
			match(T__20);
			setState(296);
			match(T__21);
			setState(297);
			loopTarget();
			setState(298);
			match(T__13);
			setState(299);
			graphID();
			setState(300);
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
		enterRule(_localctx, 44, RULE_loopTarget);
		try {
			setState(312);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__22:
				_localctx = new ForEachVertexContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(302);
				match(T__22);
				setState(303);
				match(ID);
				}
				break;
			case T__23:
				_localctx = new ForEachEdgeContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(304);
				match(T__23);
				setState(305);
				match(ID);
				setState(306);
				match(T__3);
				setState(307);
				match(ID);
				}
				break;
			case T__24:
				_localctx = new ForEachAdjContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(308);
				match(T__24);
				setState(309);
				match(ID);
				setState(310);
				match(OF);
				setState(311);
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
		enterRule(_localctx, 46, RULE_whileStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(314);
			match(T__25);
			setState(315);
			match(T__10);
			setState(316);
			condition(0);
			setState(317);
			match(T__11);
			setState(318);
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
		enterRule(_localctx, 48, RULE_nodeEdgeOperation);
		try {
			setState(322);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case T__26:
				enterOuterAlt(_localctx, 1);
				{
				setState(320);
				addOperation();
				}
				break;
			case T__28:
				enterOuterAlt(_localctx, 2);
				{
				setState(321);
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
		enterRule(_localctx, 50, RULE_addOperation);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(324);
			match(T__26);
			setState(325);
			addTargets();
			setState(326);
			match(T__27);
			setState(327);
			graphID();
			setState(328);
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
		enterRule(_localctx, 52, RULE_removeOperation);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(330);
			match(T__28);
			setState(331);
			removeTargets();
			setState(332);
			match(T__29);
			setState(333);
			graphID();
			setState(334);
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
		enterRule(_localctx, 54, RULE_addTargets);
		try {
			setState(340);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,25,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(336);
				nodeID();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(337);
				edge();
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(338);
				nodeList();
				}
				break;
			case 4:
				enterOuterAlt(_localctx, 4);
				{
				setState(339);
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
		enterRule(_localctx, 56, RULE_removeTargets);
		try {
			setState(346);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,26,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(342);
				nodeID();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(343);
				edge();
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(344);
				nodeList();
				}
				break;
			case 4:
				enterOuterAlt(_localctx, 4);
				{
				setState(345);
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
		enterRule(_localctx, 58, RULE_queryStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(348);
			match(T__30);
			setState(349);
			match(ID);
			setState(350);
			match(T__31);
			setState(351);
			match(STRING);
			setState(352);
			match(OF);
			setState(353);
			graphID();
			setState(354);
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
		enterRule(_localctx, 60, RULE_showgraph);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(356);
			match(T__32);
			setState(357);
			graphID();
			setState(358);
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
		enterRule(_localctx, 62, RULE_function);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(360);
			match(T__33);
			setState(361);
			returnType();
			setState(362);
			match(ID);
			setState(363);
			paramList();
			setState(364);
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
		public TerminalNode DIRGRAPH() { return getToken(BaseParser.DIRGRAPH, 0); }
		public ReturnTypeContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_returnType; }
	}

	public final ReturnTypeContext returnType() throws RecognitionException {
		ReturnTypeContext _localctx = new ReturnTypeContext(_ctx, getState());
		enterRule(_localctx, 64, RULE_returnType);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(366);
			_la = _input.LA(1);
			if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 13434682867712L) != 0)) ) {
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
		enterRule(_localctx, 66, RULE_paramList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(368);
			match(T__10);
			setState(377);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 14190597111808L) != 0)) {
				{
				setState(369);
				param();
				setState(374);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==T__3) {
					{
					{
					setState(370);
					match(T__3);
					setState(371);
					param();
					}
					}
					setState(376);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				}
			}

			setState(379);
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
		enterRule(_localctx, 68, RULE_param);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(381);
			type();
			setState(382);
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
		public TerminalNode DIRGRAPH() { return getToken(BaseParser.DIRGRAPH, 0); }
		public TypeContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_type; }
	}

	public final TypeContext type() throws RecognitionException {
		TypeContext _localctx = new TypeContext(_ctx, getState());
		enterRule(_localctx, 70, RULE_type);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(384);
			_la = _input.LA(1);
			if ( !((((_la) & ~0x3f) == 0 && ((1L << _la) & 14190597111808L) != 0)) ) {
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
		enterRule(_localctx, 72, RULE_functionCall);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(386);
			match(ID);
			setState(387);
			match(T__10);
			setState(389);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & -2305631902981158912L) != 0)) {
				{
				setState(388);
				argumentList();
				}
			}

			setState(391);
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
		enterRule(_localctx, 74, RULE_argumentList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(393);
			expr(0);
			setState(398);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__3) {
				{
				{
				setState(394);
				match(T__3);
				setState(395);
				expr(0);
				}
				}
				setState(400);
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
		enterRule(_localctx, 76, RULE_block);
		int _la;
		try {
			setState(412);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,33,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(401);
				match(T__1);
				setState(406);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while ((((_la) & ~0x3f) == 0 && ((1L << _la) & 2305860509823403010L) != 0)) {
					{
					setState(404);
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
					case T__38:
					case T__40:
					case GRAPH:
					case DIRGRAPH:
					case ID:
						{
						setState(402);
						statement();
						}
						break;
					case T__39:
						{
						setState(403);
						returnStatement();
						}
						break;
					default:
						throw new NoViableAltException(this);
					}
					}
					setState(408);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(409);
				match(T__2);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(410);
				match(T__1);
				setState(411);
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
		enterRule(_localctx, 78, RULE_returnStatement);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(414);
			match(T__39);
			setState(415);
			expr(0);
			setState(416);
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
		enterRule(_localctx, 80, RULE_printStatement);
		try {
			setState(423);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,34,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(418);
				match(T__40);
				setState(419);
				printExpr(0);
				setState(420);
				match(T__0);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(422);
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
		int _startState = 82;
		enterRecursionRule(_localctx, 82, RULE_printExpr, _p);
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(428);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case STRING:
				{
				setState(426);
				match(STRING);
				}
				break;
			case T__10:
			case TRUE:
			case FALSE:
			case ID:
			case INT:
			case REAL:
				{
				setState(427);
				expr(0);
				}
				break;
			default:
				throw new NoViableAltException(this);
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
					{
					_localctx = new PrintExprContext(_parentctx, _parentState);
					pushNewRecursionContext(_localctx, _startState, RULE_printExpr);
					setState(430);
					if (!(precpred(_ctx, 1))) throw new FailedPredicateException(this, "precpred(_ctx, 1)");
					setState(431);
					match(PLUS);
					setState(432);
					printExpr(2);
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
		enterRule(_localctx, 84, RULE_printgraph);
		try {
			setState(456);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,37,_ctx) ) {
			case 1:
				_localctx = new EdgePrintContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(438);
				match(T__40);
				setState(439);
				match(EDGE);
				setState(440);
				match(OF);
				setState(441);
				graphID();
				setState(442);
				match(T__0);
				}
				break;
			case 2:
				_localctx = new NodePrintContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(444);
				match(T__40);
				setState(445);
				match(NODE);
				setState(446);
				match(OF);
				setState(447);
				graphID();
				setState(448);
				match(T__0);
				}
				break;
			case 3:
				_localctx = new GraphPrintContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(450);
				match(T__40);
				setState(451);
				match(GRAPH);
				setState(452);
				match(OF);
				setState(453);
				graphID();
				setState(454);
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
	public static class BoolTrueExprContext extends ExprContext {
		public TerminalNode TRUE() { return getToken(BaseParser.TRUE, 0); }
		public BoolTrueExprContext(ExprContext ctx) { copyFrom(ctx); }
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
	public static class BoolFalseExprContext extends ExprContext {
		public TerminalNode FALSE() { return getToken(BaseParser.FALSE, 0); }
		public BoolFalseExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrayAccessExprContext extends ExprContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public ArrayAccessExprContext(ExprContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class Array2DAccessExprContext extends ExprContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public List<ExprContext> expr() {
			return getRuleContexts(ExprContext.class);
		}
		public ExprContext expr(int i) {
			return getRuleContext(ExprContext.class,i);
		}
		public Array2DAccessExprContext(ExprContext ctx) { copyFrom(ctx); }
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
		int _startState = 86;
		enterRecursionRule(_localctx, 86, RULE_expr, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(482);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,38,_ctx) ) {
			case 1:
				{
				_localctx = new FuncExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(459);
				functionCall();
				}
				break;
			case 2:
				{
				_localctx = new IntExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(460);
				match(INT);
				}
				break;
			case 3:
				{
				_localctx = new IdExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(461);
				match(ID);
				}
				break;
			case 4:
				{
				_localctx = new ParenExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(462);
				match(T__10);
				setState(463);
				expr(0);
				setState(464);
				match(T__11);
				}
				break;
			case 5:
				{
				_localctx = new ArrayAccessExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(466);
				match(ID);
				setState(467);
				match(T__14);
				setState(468);
				expr(0);
				setState(469);
				match(T__16);
				}
				break;
			case 6:
				{
				_localctx = new Array2DAccessExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(471);
				match(ID);
				setState(472);
				match(T__14);
				setState(473);
				expr(0);
				setState(474);
				match(T__16);
				setState(475);
				match(T__14);
				setState(476);
				expr(0);
				setState(477);
				match(T__16);
				}
				break;
			case 7:
				{
				_localctx = new BoolTrueExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(479);
				match(TRUE);
				}
				break;
			case 8:
				{
				_localctx = new BoolFalseExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(480);
				match(FALSE);
				}
				break;
			case 9:
				{
				_localctx = new RealExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(481);
				match(REAL);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(492);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,40,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(490);
					_errHandler.sync(this);
					switch ( getInterpreter().adaptivePredict(_input,39,_ctx) ) {
					case 1:
						{
						_localctx = new MulDivExprContext(new ExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expr);
						setState(484);
						if (!(precpred(_ctx, 11))) throw new FailedPredicateException(this, "precpred(_ctx, 11)");
						setState(485);
						_la = _input.LA(1);
						if ( !(_la==TIMES || _la==DIVIDE) ) {
						_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(486);
						expr(12);
						}
						break;
					case 2:
						{
						_localctx = new AddSubExprContext(new ExprContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expr);
						setState(487);
						if (!(precpred(_ctx, 10))) throw new FailedPredicateException(this, "precpred(_ctx, 10)");
						setState(488);
						_la = _input.LA(1);
						if ( !(_la==PLUS || _la==MINUS) ) {
						_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(489);
						expr(11);
						}
						break;
					}
					} 
				}
				setState(494);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,40,_ctx);
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
	public static class Sized2DArrayContext extends ArrayDeclaratorContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public List<TerminalNode> INT() { return getTokens(BaseParser.INT); }
		public TerminalNode INT(int i) {
			return getToken(BaseParser.INT, i);
		}
		public Sized2DArrayContext(ArrayDeclaratorContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class SizedArrayContext extends ArrayDeclaratorContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public TerminalNode INT() { return getToken(BaseParser.INT, 0); }
		public SizedArrayContext(ArrayDeclaratorContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class Unsized2DArrayContext extends ArrayDeclaratorContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public Unsized2DArrayContext(ArrayDeclaratorContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class UnsizedArrayContext extends ArrayDeclaratorContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public UnsizedArrayContext(ArrayDeclaratorContext ctx) { copyFrom(ctx); }
	}

	public final ArrayDeclaratorContext arrayDeclarator() throws RecognitionException {
		ArrayDeclaratorContext _localctx = new ArrayDeclaratorContext(_ctx, getState());
		enterRule(_localctx, 88, RULE_arrayDeclarator);
		try {
			setState(514);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,41,_ctx) ) {
			case 1:
				_localctx = new Sized2DArrayContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(495);
				match(ID);
				setState(496);
				match(T__14);
				setState(497);
				match(INT);
				setState(498);
				match(T__16);
				setState(499);
				match(T__14);
				setState(500);
				match(INT);
				setState(501);
				match(T__16);
				}
				break;
			case 2:
				_localctx = new Unsized2DArrayContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(502);
				match(ID);
				setState(503);
				match(T__14);
				setState(504);
				match(T__16);
				setState(505);
				match(T__14);
				setState(506);
				match(T__16);
				}
				break;
			case 3:
				_localctx = new SizedArrayContext(_localctx);
				enterOuterAlt(_localctx, 3);
				{
				setState(507);
				match(ID);
				setState(508);
				match(T__14);
				setState(509);
				match(INT);
				setState(510);
				match(T__16);
				}
				break;
			case 4:
				_localctx = new UnsizedArrayContext(_localctx);
				enterOuterAlt(_localctx, 4);
				{
				setState(511);
				match(ID);
				setState(512);
				match(T__14);
				setState(513);
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
	public static class DeclarationContext extends ParserRuleContext {
		public TypeContext type() {
			return getRuleContext(TypeContext.class,0);
		}
		public ArrayDeclaratorContext arrayDeclarator() {
			return getRuleContext(ArrayDeclaratorContext.class,0);
		}
		public ArrayInitializerContext arrayInitializer() {
			return getRuleContext(ArrayInitializerContext.class,0);
		}
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public DeclarationContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_declaration; }
	}

	public final DeclarationContext declaration() throws RecognitionException {
		DeclarationContext _localctx = new DeclarationContext(_ctx, getState());
		enterRule(_localctx, 90, RULE_declaration);
		int _la;
		try {
			setState(532);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,44,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(516);
				type();
				setState(517);
				arrayDeclarator();
				setState(520);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==T__8) {
					{
					setState(518);
					match(T__8);
					setState(519);
					arrayInitializer();
					}
				}

				setState(522);
				match(T__0);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(524);
				type();
				setState(525);
				match(ID);
				setState(528);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==T__8) {
					{
					setState(526);
					match(T__8);
					setState(527);
					expr(0);
					}
				}

				setState(530);
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
	public static class ArrayInitializerContext extends ParserRuleContext {
		public ArrayInitializerContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrayInitializer; }
	 
		public ArrayInitializerContext() { }
		public void copyFrom(ArrayInitializerContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrayInit1DContext extends ArrayInitializerContext {
		public List<ExprContext> expr() {
			return getRuleContexts(ExprContext.class);
		}
		public ExprContext expr(int i) {
			return getRuleContext(ExprContext.class,i);
		}
		public ArrayInit1DContext(ArrayInitializerContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrayInit2DContext extends ArrayInitializerContext {
		public List<ArrayInitializerContext> arrayInitializer() {
			return getRuleContexts(ArrayInitializerContext.class);
		}
		public ArrayInitializerContext arrayInitializer(int i) {
			return getRuleContext(ArrayInitializerContext.class,i);
		}
		public ArrayInit2DContext(ArrayInitializerContext ctx) { copyFrom(ctx); }
	}

	public final ArrayInitializerContext arrayInitializer() throws RecognitionException {
		ArrayInitializerContext _localctx = new ArrayInitializerContext(_ctx, getState());
		enterRule(_localctx, 92, RULE_arrayInitializer);
		int _la;
		try {
			setState(557);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,48,_ctx) ) {
			case 1:
				_localctx = new ArrayInit1DContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(534);
				match(T__14);
				setState(535);
				expr(0);
				setState(540);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==T__3) {
					{
					{
					setState(536);
					match(T__3);
					setState(537);
					expr(0);
					}
					}
					setState(542);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(543);
				match(T__16);
				}
				break;
			case 2:
				_localctx = new ArrayInit2DContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(545);
				match(T__14);
				setState(554);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==T__14) {
					{
					setState(546);
					arrayInitializer();
					setState(551);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==T__3) {
						{
						{
						setState(547);
						match(T__3);
						setState(548);
						arrayInitializer();
						}
						}
						setState(553);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(556);
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
	public static class AssignmentStatementContext extends ParserRuleContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public AssignmentStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_assignmentStatement; }
	}

	public final AssignmentStatementContext assignmentStatement() throws RecognitionException {
		AssignmentStatementContext _localctx = new AssignmentStatementContext(_ctx, getState());
		enterRule(_localctx, 94, RULE_assignmentStatement);
		try {
			setState(566);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,49,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(559);
				match(ID);
				setState(560);
				match(T__8);
				setState(561);
				expr(0);
				setState(562);
				match(T__0);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(564);
				match(ID);
				setState(565);
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
	public static class ArrayAssignStatementContext extends ParserRuleContext {
		public ArrayAssignStatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrayAssignStatement; }
	 
		public ArrayAssignStatementContext() { }
		public void copyFrom(ArrayAssignStatementContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class Array2DAssignStmtContext extends ArrayAssignStatementContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public List<TerminalNode> INT() { return getTokens(BaseParser.INT); }
		public TerminalNode INT(int i) {
			return getToken(BaseParser.INT, i);
		}
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public Array2DAssignStmtContext(ArrayAssignStatementContext ctx) { copyFrom(ctx); }
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrayAssignStmtContext extends ArrayAssignStatementContext {
		public TerminalNode ID() { return getToken(BaseParser.ID, 0); }
		public TerminalNode INT() { return getToken(BaseParser.INT, 0); }
		public ExprContext expr() {
			return getRuleContext(ExprContext.class,0);
		}
		public ArrayAssignStmtContext(ArrayAssignStatementContext ctx) { copyFrom(ctx); }
	}

	public final ArrayAssignStatementContext arrayAssignStatement() throws RecognitionException {
		ArrayAssignStatementContext _localctx = new ArrayAssignStatementContext(_ctx, getState());
		enterRule(_localctx, 96, RULE_arrayAssignStatement);
		try {
			setState(587);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,50,_ctx) ) {
			case 1:
				_localctx = new ArrayAssignStmtContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(568);
				match(ID);
				setState(569);
				match(T__14);
				setState(570);
				match(INT);
				setState(571);
				match(T__16);
				setState(572);
				match(T__8);
				setState(573);
				expr(0);
				setState(574);
				match(T__0);
				}
				break;
			case 2:
				_localctx = new Array2DAssignStmtContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(576);
				match(ID);
				setState(577);
				match(T__14);
				setState(578);
				match(INT);
				setState(579);
				match(T__16);
				setState(580);
				match(T__14);
				setState(581);
				match(INT);
				setState(582);
				match(T__16);
				setState(583);
				match(T__8);
				setState(584);
				expr(0);
				setState(585);
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

	public boolean sempred(RuleContext _localctx, int ruleIndex, int predIndex) {
		switch (ruleIndex) {
		case 17:
			return condition_sempred((ConditionContext)_localctx, predIndex);
		case 19:
			return graphCondition_sempred((GraphConditionContext)_localctx, predIndex);
		case 41:
			return printExpr_sempred((PrintExprContext)_localctx, predIndex);
		case 43:
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
			return precpred(_ctx, 11);
		case 6:
			return precpred(_ctx, 10);
		}
		return true;
	}

	public static final String _serializedATN =
		"\u0004\u0001B\u024e\u0002\u0000\u0007\u0000\u0002\u0001\u0007\u0001\u0002"+
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
		"(\u0007(\u0002)\u0007)\u0002*\u0007*\u0002+\u0007+\u0002,\u0007,\u0002"+
		"-\u0007-\u0002.\u0007.\u0002/\u0007/\u00020\u00070\u0001\u0000\u0001\u0000"+
		"\u0005\u0000e\b\u0000\n\u0000\f\u0000h\t\u0000\u0001\u0000\u0001\u0000"+
		"\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001"+
		"\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001"+
		"\u0001\u0001\u0001\u0001\u0003\u0001z\b\u0001\u0001\u0002\u0001\u0002"+
		"\u0001\u0002\u0001\u0002\u0003\u0002\u0080\b\u0002\u0001\u0002\u0003\u0002"+
		"\u0083\b\u0002\u0001\u0002\u0001\u0002\u0001\u0002\u0001\u0003\u0001\u0003"+
		"\u0001\u0003\u0001\u0004\u0001\u0004\u0001\u0004\u0005\u0004\u008e\b\u0004"+
		"\n\u0004\f\u0004\u0091\t\u0004\u0001\u0005\u0001\u0005\u0001\u0005\u0001"+
		"\u0005\u0003\u0005\u0097\b\u0005\u0001\u0005\u0003\u0005\u009a\b\u0005"+
		"\u0001\u0005\u0001\u0005\u0001\u0005\u0001\u0006\u0001\u0006\u0001\u0006"+
		"\u0003\u0006\u00a2\b\u0006\u0001\u0006\u0001\u0006\u0001\u0007\u0001\u0007"+
		"\u0001\u0007\u0001\u0007\u0001\b\u0001\b\u0001\b\u0003\b\u00ad\b\b\u0001"+
		"\b\u0001\b\u0001\t\u0001\t\u0001\t\u0005\t\u00b4\b\t\n\t\f\t\u00b7\t\t"+
		"\u0001\n\u0001\n\u0001\n\u0005\n\u00bc\b\n\n\n\f\n\u00bf\t\n\u0001\u000b"+
		"\u0001\u000b\u0001\f\u0001\f\u0001\r\u0001\r\u0001\r\u0001\u000e\u0001"+
		"\u000e\u0001\u000e\u0001\u000e\u0001\u000f\u0001\u000f\u0001\u000f\u0001"+
		"\u000f\u0003\u000f\u00d0\b\u000f\u0001\u000f\u0001\u000f\u0001\u000f\u0001"+
		"\u000f\u0001\u000f\u0001\u000f\u0003\u000f\u00d8\b\u000f\u0001\u000f\u0001"+
		"\u000f\u0003\u000f\u00dc\b\u000f\u0001\u0010\u0001\u0010\u0001\u0010\u0001"+
		"\u0010\u0001\u0010\u0001\u0010\u0001\u0010\u0001\u0010\u0001\u0010\u0003"+
		"\u0010\u00e7\b\u0010\u0001\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0001"+
		"\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0001"+
		"\u0011\u0001\u0011\u0001\u0011\u0003\u0011\u00f6\b\u0011\u0001\u0011\u0001"+
		"\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0001\u0011\u0005\u0011\u00fe"+
		"\b\u0011\n\u0011\f\u0011\u0101\t\u0011\u0001\u0012\u0001\u0012\u0001\u0012"+
		"\u0001\u0012\u0001\u0012\u0001\u0012\u0001\u0012\u0001\u0012\u0001\u0012"+
		"\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013"+
		"\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0003\u0013"+
		"\u0117\b\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013\u0001\u0013"+
		"\u0001\u0013\u0005\u0013\u011f\b\u0013\n\u0013\f\u0013\u0122\t\u0013\u0001"+
		"\u0014\u0001\u0014\u0003\u0014\u0126\b\u0014\u0001\u0015\u0001\u0015\u0001"+
		"\u0015\u0001\u0015\u0001\u0015\u0001\u0015\u0001\u0015\u0001\u0016\u0001"+
		"\u0016\u0001\u0016\u0001\u0016\u0001\u0016\u0001\u0016\u0001\u0016\u0001"+
		"\u0016\u0001\u0016\u0001\u0016\u0003\u0016\u0139\b\u0016\u0001\u0017\u0001"+
		"\u0017\u0001\u0017\u0001\u0017\u0001\u0017\u0001\u0017\u0001\u0018\u0001"+
		"\u0018\u0003\u0018\u0143\b\u0018\u0001\u0019\u0001\u0019\u0001\u0019\u0001"+
		"\u0019\u0001\u0019\u0001\u0019\u0001\u001a\u0001\u001a\u0001\u001a\u0001"+
		"\u001a\u0001\u001a\u0001\u001a\u0001\u001b\u0001\u001b\u0001\u001b\u0001"+
		"\u001b\u0003\u001b\u0155\b\u001b\u0001\u001c\u0001\u001c\u0001\u001c\u0001"+
		"\u001c\u0003\u001c\u015b\b\u001c\u0001\u001d\u0001\u001d\u0001\u001d\u0001"+
		"\u001d\u0001\u001d\u0001\u001d\u0001\u001d\u0001\u001d\u0001\u001e\u0001"+
		"\u001e\u0001\u001e\u0001\u001e\u0001\u001f\u0001\u001f\u0001\u001f\u0001"+
		"\u001f\u0001\u001f\u0001\u001f\u0001 \u0001 \u0001!\u0001!\u0001!\u0001"+
		"!\u0005!\u0175\b!\n!\f!\u0178\t!\u0003!\u017a\b!\u0001!\u0001!\u0001\""+
		"\u0001\"\u0001\"\u0001#\u0001#\u0001$\u0001$\u0001$\u0003$\u0186\b$\u0001"+
		"$\u0001$\u0001%\u0001%\u0001%\u0005%\u018d\b%\n%\f%\u0190\t%\u0001&\u0001"+
		"&\u0001&\u0005&\u0195\b&\n&\f&\u0198\t&\u0001&\u0001&\u0001&\u0003&\u019d"+
		"\b&\u0001\'\u0001\'\u0001\'\u0001\'\u0001(\u0001(\u0001(\u0001(\u0001"+
		"(\u0003(\u01a8\b(\u0001)\u0001)\u0001)\u0003)\u01ad\b)\u0001)\u0001)\u0001"+
		")\u0005)\u01b2\b)\n)\f)\u01b5\t)\u0001*\u0001*\u0001*\u0001*\u0001*\u0001"+
		"*\u0001*\u0001*\u0001*\u0001*\u0001*\u0001*\u0001*\u0001*\u0001*\u0001"+
		"*\u0001*\u0001*\u0003*\u01c9\b*\u0001+\u0001+\u0001+\u0001+\u0001+\u0001"+
		"+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001"+
		"+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0003+\u01e3"+
		"\b+\u0001+\u0001+\u0001+\u0001+\u0001+\u0001+\u0005+\u01eb\b+\n+\f+\u01ee"+
		"\t+\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0001"+
		",\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0001,\u0003"+
		",\u0203\b,\u0001-\u0001-\u0001-\u0001-\u0003-\u0209\b-\u0001-\u0001-\u0001"+
		"-\u0001-\u0001-\u0001-\u0003-\u0211\b-\u0001-\u0001-\u0003-\u0215\b-\u0001"+
		".\u0001.\u0001.\u0001.\u0005.\u021b\b.\n.\f.\u021e\t.\u0001.\u0001.\u0001"+
		".\u0001.\u0001.\u0001.\u0005.\u0226\b.\n.\f.\u0229\t.\u0003.\u022b\b."+
		"\u0001.\u0003.\u022e\b.\u0001/\u0001/\u0001/\u0001/\u0001/\u0001/\u0001"+
		"/\u0003/\u0237\b/\u00010\u00010\u00010\u00010\u00010\u00010\u00010\u0001"+
		"0\u00010\u00010\u00010\u00010\u00010\u00010\u00010\u00010\u00010\u0001"+
		"0\u00010\u00030\u024c\b0\u00010\u0000\u0004\"&RV1\u0000\u0002\u0004\u0006"+
		"\b\n\f\u000e\u0010\u0012\u0014\u0016\u0018\u001a\u001c\u001e \"$&(*,."+
		"02468:<>@BDFHJLNPRTVXZ\\^`\u0000\u0005\u0001\u00007<\u0003\u0000\u0017"+
		"\u0018#%*+\u0004\u0000\u0017\u0018##%\'*+\u0001\u000034\u0001\u000012"+
		"\u026d\u0000f\u0001\u0000\u0000\u0000\u0002y\u0001\u0000\u0000\u0000\u0004"+
		"{\u0001\u0000\u0000\u0000\u0006\u0087\u0001\u0000\u0000\u0000\b\u008a"+
		"\u0001\u0000\u0000\u0000\n\u0092\u0001\u0000\u0000\u0000\f\u009e\u0001"+
		"\u0000\u0000\u0000\u000e\u00a5\u0001\u0000\u0000\u0000\u0010\u00a9\u0001"+
		"\u0000\u0000\u0000\u0012\u00b0\u0001\u0000\u0000\u0000\u0014\u00b8\u0001"+
		"\u0000\u0000\u0000\u0016\u00c0\u0001\u0000\u0000\u0000\u0018\u00c2\u0001"+
		"\u0000\u0000\u0000\u001a\u00c4\u0001\u0000\u0000\u0000\u001c\u00c7\u0001"+
		"\u0000\u0000\u0000\u001e\u00db\u0001\u0000\u0000\u0000 \u00dd\u0001\u0000"+
		"\u0000\u0000\"\u00f5\u0001\u0000\u0000\u0000$\u0102\u0001\u0000\u0000"+
		"\u0000&\u0116\u0001\u0000\u0000\u0000(\u0125\u0001\u0000\u0000\u0000*"+
		"\u0127\u0001\u0000\u0000\u0000,\u0138\u0001\u0000\u0000\u0000.\u013a\u0001"+
		"\u0000\u0000\u00000\u0142\u0001\u0000\u0000\u00002\u0144\u0001\u0000\u0000"+
		"\u00004\u014a\u0001\u0000\u0000\u00006\u0154\u0001\u0000\u0000\u00008"+
		"\u015a\u0001\u0000\u0000\u0000:\u015c\u0001\u0000\u0000\u0000<\u0164\u0001"+
		"\u0000\u0000\u0000>\u0168\u0001\u0000\u0000\u0000@\u016e\u0001\u0000\u0000"+
		"\u0000B\u0170\u0001\u0000\u0000\u0000D\u017d\u0001\u0000\u0000\u0000F"+
		"\u0180\u0001\u0000\u0000\u0000H\u0182\u0001\u0000\u0000\u0000J\u0189\u0001"+
		"\u0000\u0000\u0000L\u019c\u0001\u0000\u0000\u0000N\u019e\u0001\u0000\u0000"+
		"\u0000P\u01a7\u0001\u0000\u0000\u0000R\u01ac\u0001\u0000\u0000\u0000T"+
		"\u01c8\u0001\u0000\u0000\u0000V\u01e2\u0001\u0000\u0000\u0000X\u0202\u0001"+
		"\u0000\u0000\u0000Z\u0214\u0001\u0000\u0000\u0000\\\u022d\u0001\u0000"+
		"\u0000\u0000^\u0236\u0001\u0000\u0000\u0000`\u024b\u0001\u0000\u0000\u0000"+
		"be\u0003\u0002\u0001\u0000ce\u0003>\u001f\u0000db\u0001\u0000\u0000\u0000"+
		"dc\u0001\u0000\u0000\u0000eh\u0001\u0000\u0000\u0000fd\u0001\u0000\u0000"+
		"\u0000fg\u0001\u0000\u0000\u0000gi\u0001\u0000\u0000\u0000hf\u0001\u0000"+
		"\u0000\u0000ij\u0005\u0000\u0000\u0001j\u0001\u0001\u0000\u0000\u0000"+
		"kz\u0003\u0004\u0002\u0000lz\u0003\n\u0005\u0000mz\u0003 \u0010\u0000"+
		"nz\u0003P(\u0000oz\u0003(\u0014\u0000pz\u0003\u001e\u000f\u0000qz\u0003"+
		"H$\u0000rz\u0003$\u0012\u0000sz\u0003`0\u0000tz\u0003^/\u0000uz\u0003"+
		":\u001d\u0000vz\u0003<\u001e\u0000wz\u00030\u0018\u0000xz\u0005\u0001"+
		"\u0000\u0000yk\u0001\u0000\u0000\u0000yl\u0001\u0000\u0000\u0000ym\u0001"+
		"\u0000\u0000\u0000yn\u0001\u0000\u0000\u0000yo\u0001\u0000\u0000\u0000"+
		"yp\u0001\u0000\u0000\u0000yq\u0001\u0000\u0000\u0000yr\u0001\u0000\u0000"+
		"\u0000ys\u0001\u0000\u0000\u0000yt\u0001\u0000\u0000\u0000yu\u0001\u0000"+
		"\u0000\u0000yv\u0001\u0000\u0000\u0000yw\u0001\u0000\u0000\u0000yx\u0001"+
		"\u0000\u0000\u0000z\u0003\u0001\u0000\u0000\u0000{|\u0005+\u0000\u0000"+
		"|}\u0003\u0016\u000b\u0000}\u007f\u0005\u0002\u0000\u0000~\u0080\u0003"+
		"\u000e\u0007\u0000\u007f~\u0001\u0000\u0000\u0000\u007f\u0080\u0001\u0000"+
		"\u0000\u0000\u0080\u0082\u0001\u0000\u0000\u0000\u0081\u0083\u0003\u0010"+
		"\b\u0000\u0082\u0081\u0001\u0000\u0000\u0000\u0082\u0083\u0001\u0000\u0000"+
		"\u0000\u0083\u0084\u0001\u0000\u0000\u0000\u0084\u0085\u0005\u0003\u0000"+
		"\u0000\u0085\u0086\u0005\u0001\u0000\u0000\u0086\u0005\u0001\u0000\u0000"+
		"\u0000\u0087\u0088\u0003\u0018\f\u0000\u0088\u0089\u0003\u0018\f\u0000"+
		"\u0089\u0007\u0001\u0000\u0000\u0000\u008a\u008f\u0003\u0006\u0003\u0000"+
		"\u008b\u008c\u0005\u0004\u0000\u0000\u008c\u008e\u0003\u0006\u0003\u0000"+
		"\u008d\u008b\u0001\u0000\u0000\u0000\u008e\u0091\u0001\u0000\u0000\u0000"+
		"\u008f\u008d\u0001\u0000\u0000\u0000\u008f\u0090\u0001\u0000\u0000\u0000"+
		"\u0090\t\u0001\u0000\u0000\u0000\u0091\u008f\u0001\u0000\u0000\u0000\u0092"+
		"\u0093\u0005*\u0000\u0000\u0093\u0094\u0003\u0016\u000b\u0000\u0094\u0096"+
		"\u0005\u0002\u0000\u0000\u0095\u0097\u0003\u000e\u0007\u0000\u0096\u0095"+
		"\u0001\u0000\u0000\u0000\u0096\u0097\u0001\u0000\u0000\u0000\u0097\u0099"+
		"\u0001\u0000\u0000\u0000\u0098\u009a\u0003\f\u0006\u0000\u0099\u0098\u0001"+
		"\u0000\u0000\u0000\u0099\u009a\u0001\u0000\u0000\u0000\u009a\u009b\u0001"+
		"\u0000\u0000\u0000\u009b\u009c\u0005\u0003\u0000\u0000\u009c\u009d\u0005"+
		"\u0001\u0000\u0000\u009d\u000b\u0001\u0000\u0000\u0000\u009e\u00a1\u0005"+
		"\u0005\u0000\u0000\u009f\u00a2\u0003\b\u0004\u0000\u00a0\u00a2\u0003\u001a"+
		"\r\u0000\u00a1\u009f\u0001\u0000\u0000\u0000\u00a1\u00a0\u0001\u0000\u0000"+
		"\u0000\u00a2\u00a3\u0001\u0000\u0000\u0000\u00a3\u00a4\u0005\u0001\u0000"+
		"\u0000\u00a4\r\u0001\u0000\u0000\u0000\u00a5\u00a6\u0005\u0006\u0000\u0000"+
		"\u00a6\u00a7\u0003\u0012\t\u0000\u00a7\u00a8\u0005\u0001\u0000\u0000\u00a8"+
		"\u000f\u0001\u0000\u0000\u0000\u00a9\u00ac\u0005\u0005\u0000\u0000\u00aa"+
		"\u00ad\u0003\u0014\n\u0000\u00ab\u00ad\u0003\u001a\r\u0000\u00ac\u00aa"+
		"\u0001\u0000\u0000\u0000\u00ac\u00ab\u0001\u0000\u0000\u0000\u00ad\u00ae"+
		"\u0001\u0000\u0000\u0000\u00ae\u00af\u0005\u0001\u0000\u0000\u00af\u0011"+
		"\u0001\u0000\u0000\u0000\u00b0\u00b5\u0003\u0018\f\u0000\u00b1\u00b2\u0005"+
		"\u0004\u0000\u0000\u00b2\u00b4\u0003\u0018\f\u0000\u00b3\u00b1\u0001\u0000"+
		"\u0000\u0000\u00b4\u00b7\u0001\u0000\u0000\u0000\u00b5\u00b3\u0001\u0000"+
		"\u0000\u0000\u00b5\u00b6\u0001\u0000\u0000\u0000\u00b6\u0013\u0001\u0000"+
		"\u0000\u0000\u00b7\u00b5\u0001\u0000\u0000\u0000\u00b8\u00bd\u0003\u001c"+
		"\u000e\u0000\u00b9\u00ba\u0005\u0004\u0000\u0000\u00ba\u00bc\u0003\u001c"+
		"\u000e\u0000\u00bb\u00b9\u0001\u0000\u0000\u0000\u00bc\u00bf\u0001\u0000"+
		"\u0000\u0000\u00bd\u00bb\u0001\u0000\u0000\u0000\u00bd\u00be\u0001\u0000"+
		"\u0000\u0000\u00be\u0015\u0001\u0000\u0000\u0000\u00bf\u00bd\u0001\u0000"+
		"\u0000\u0000\u00c0\u00c1\u0005=\u0000\u0000\u00c1\u0017\u0001\u0000\u0000"+
		"\u0000\u00c2\u00c3\u0005>\u0000\u0000\u00c3\u0019\u0001\u0000\u0000\u0000"+
		"\u00c4\u00c5\u0005\u0007\u0000\u0000\u00c5\u00c6\u0005@\u0000\u0000\u00c6"+
		"\u001b\u0001\u0000\u0000\u0000\u00c7\u00c8\u0003\u0018\f\u0000\u00c8\u00c9"+
		"\u0005\b\u0000\u0000\u00c9\u00ca\u0003\u0018\f\u0000\u00ca\u001d\u0001"+
		"\u0000\u0000\u0000\u00cb\u00cc\u0003F#\u0000\u00cc\u00cf\u0005=\u0000"+
		"\u0000\u00cd\u00ce\u0005\t\u0000\u0000\u00ce\u00d0\u0003V+\u0000\u00cf"+
		"\u00cd\u0001\u0000\u0000\u0000\u00cf\u00d0\u0001\u0000\u0000\u0000\u00d0"+
		"\u00d1\u0001\u0000\u0000\u0000\u00d1\u00d2\u0005\u0001\u0000\u0000\u00d2"+
		"\u00dc\u0001\u0000\u0000\u0000\u00d3\u00d4\u0003F#\u0000\u00d4\u00d7\u0003"+
		"X,\u0000\u00d5\u00d6\u0005\t\u0000\u0000\u00d6\u00d8\u0003\\.\u0000\u00d7"+
		"\u00d5\u0001\u0000\u0000\u0000\u00d7\u00d8\u0001\u0000\u0000\u0000\u00d8"+
		"\u00d9\u0001\u0000\u0000\u0000\u00d9\u00da\u0005\u0001\u0000\u0000\u00da"+
		"\u00dc\u0001\u0000\u0000\u0000\u00db\u00cb\u0001\u0000\u0000\u0000\u00db"+
		"\u00d3\u0001\u0000\u0000\u0000\u00dc\u001f\u0001\u0000\u0000\u0000\u00dd"+
		"\u00de\u0005\n\u0000\u0000\u00de\u00df\u0005\u000b\u0000\u0000\u00df\u00e0"+
		"\u0003\"\u0011\u0000\u00e0\u00e1\u0005\f\u0000\u0000\u00e1\u00e6\u0003"+
		"L&\u0000\u00e2\u00e3\u0005\r\u0000\u0000\u00e3\u00e7\u0003 \u0010\u0000"+
		"\u00e4\u00e5\u0005\r\u0000\u0000\u00e5\u00e7\u0003L&\u0000\u00e6\u00e2"+
		"\u0001\u0000\u0000\u0000\u00e6\u00e4\u0001\u0000\u0000\u0000\u00e6\u00e7"+
		"\u0001\u0000\u0000\u0000\u00e7!\u0001\u0000\u0000\u0000\u00e8\u00e9\u0006"+
		"\u0011\uffff\uffff\u0000\u00e9\u00ea\u0003V+\u0000\u00ea\u00eb\u0007\u0000"+
		"\u0000\u0000\u00eb\u00ec\u0003V+\u0000\u00ec\u00f6\u0001\u0000\u0000\u0000"+
		"\u00ed\u00ee\u0003\u0018\f\u0000\u00ee\u00ef\u0005\u000e\u0000\u0000\u00ef"+
		"\u00f0\u0003\u0016\u000b\u0000\u00f0\u00f6\u0001\u0000\u0000\u0000\u00f1"+
		"\u00f2\u0003\u001c\u000e\u0000\u00f2\u00f3\u0005\u000e\u0000\u0000\u00f3"+
		"\u00f4\u0003\u0016\u000b\u0000\u00f4\u00f6\u0001\u0000\u0000\u0000\u00f5"+
		"\u00e8\u0001\u0000\u0000\u0000\u00f5\u00ed\u0001\u0000\u0000\u0000\u00f5"+
		"\u00f1\u0001\u0000\u0000\u0000\u00f6\u00ff\u0001\u0000\u0000\u0000\u00f7"+
		"\u00f8\n\u0005\u0000\u0000\u00f8\u00f9\u00055\u0000\u0000\u00f9\u00fe"+
		"\u0003\"\u0011\u0006\u00fa\u00fb\n\u0004\u0000\u0000\u00fb\u00fc\u0005"+
		"6\u0000\u0000\u00fc\u00fe\u0003\"\u0011\u0005\u00fd\u00f7\u0001\u0000"+
		"\u0000\u0000\u00fd\u00fa\u0001\u0000\u0000\u0000\u00fe\u0101\u0001\u0000"+
		"\u0000\u0000\u00ff\u00fd\u0001\u0000\u0000\u0000\u00ff\u0100\u0001\u0000"+
		"\u0000\u0000\u0100#\u0001\u0000\u0000\u0000\u0101\u00ff\u0001\u0000\u0000"+
		"\u0000\u0102\u0103\u0005=\u0000\u0000\u0103\u0104\u0005\t\u0000\u0000"+
		"\u0104\u0105\u0005\u000f\u0000\u0000\u0105\u0106\u0003\u0016\u000b\u0000"+
		"\u0106\u0107\u0005\u0010\u0000\u0000\u0107\u0108\u0003&\u0013\u0000\u0108"+
		"\u0109\u0005\u0011\u0000\u0000\u0109\u010a\u0005\u0001\u0000\u0000\u010a"+
		"%\u0001\u0000\u0000\u0000\u010b\u010c\u0006\u0013\uffff\uffff\u0000\u010c"+
		"\u010d\u0005\u0012\u0000\u0000\u010d\u010e\u0007\u0000\u0000\u0000\u010e"+
		"\u0117\u0005>\u0000\u0000\u010f\u0110\u0005\u0013\u0000\u0000\u0110\u0111"+
		"\u0005\u0014\u0000\u0000\u0111\u0117\u0003\u0018\f\u0000\u0112\u0113\u0005"+
		"\u000b\u0000\u0000\u0113\u0114\u0003&\u0013\u0000\u0114\u0115\u0005\f"+
		"\u0000\u0000\u0115\u0117\u0001\u0000\u0000\u0000\u0116\u010b\u0001\u0000"+
		"\u0000\u0000\u0116\u010f\u0001\u0000\u0000\u0000\u0116\u0112\u0001\u0000"+
		"\u0000\u0000\u0117\u0120\u0001\u0000\u0000\u0000\u0118\u0119\n\u0005\u0000"+
		"\u0000\u0119\u011a\u00055\u0000\u0000\u011a\u011f\u0003&\u0013\u0006\u011b"+
		"\u011c\n\u0004\u0000\u0000\u011c\u011d\u00056\u0000\u0000\u011d\u011f"+
		"\u0003&\u0013\u0005\u011e\u0118\u0001\u0000\u0000\u0000\u011e\u011b\u0001"+
		"\u0000\u0000\u0000\u011f\u0122\u0001\u0000\u0000\u0000\u0120\u011e\u0001"+
		"\u0000\u0000\u0000\u0120\u0121\u0001\u0000\u0000\u0000\u0121\'\u0001\u0000"+
		"\u0000\u0000\u0122\u0120\u0001\u0000\u0000\u0000\u0123\u0126\u0003*\u0015"+
		"\u0000\u0124\u0126\u0003.\u0017\u0000\u0125\u0123\u0001\u0000\u0000\u0000"+
		"\u0125\u0124\u0001\u0000\u0000\u0000\u0126)\u0001\u0000\u0000\u0000\u0127"+
		"\u0128\u0005\u0015\u0000\u0000\u0128\u0129\u0005\u0016\u0000\u0000\u0129"+
		"\u012a\u0003,\u0016\u0000\u012a\u012b\u0005\u000e\u0000\u0000\u012b\u012c"+
		"\u0003\u0016\u000b\u0000\u012c\u012d\u0003L&\u0000\u012d+\u0001\u0000"+
		"\u0000\u0000\u012e\u012f\u0005\u0017\u0000\u0000\u012f\u0139\u0005=\u0000"+
		"\u0000\u0130\u0131\u0005\u0018\u0000\u0000\u0131\u0132\u0005=\u0000\u0000"+
		"\u0132\u0133\u0005\u0004\u0000\u0000\u0133\u0139\u0005=\u0000\u0000\u0134"+
		"\u0135\u0005\u0019\u0000\u0000\u0135\u0136\u0005=\u0000\u0000\u0136\u0137"+
		"\u00050\u0000\u0000\u0137\u0139\u0003\u0018\f\u0000\u0138\u012e\u0001"+
		"\u0000\u0000\u0000\u0138\u0130\u0001\u0000\u0000\u0000\u0138\u0134\u0001"+
		"\u0000\u0000\u0000\u0139-\u0001\u0000\u0000\u0000\u013a\u013b\u0005\u001a"+
		"\u0000\u0000\u013b\u013c\u0005\u000b\u0000\u0000\u013c\u013d\u0003\"\u0011"+
		"\u0000\u013d\u013e\u0005\f\u0000\u0000\u013e\u013f\u0003L&\u0000\u013f"+
		"/\u0001\u0000\u0000\u0000\u0140\u0143\u00032\u0019\u0000\u0141\u0143\u0003"+
		"4\u001a\u0000\u0142\u0140\u0001\u0000\u0000\u0000\u0142\u0141\u0001\u0000"+
		"\u0000\u0000\u01431\u0001\u0000\u0000\u0000\u0144\u0145\u0005\u001b\u0000"+
		"\u0000\u0145\u0146\u00036\u001b\u0000\u0146\u0147\u0005\u001c\u0000\u0000"+
		"\u0147\u0148\u0003\u0016\u000b\u0000\u0148\u0149\u0005\u0001\u0000\u0000"+
		"\u01493\u0001\u0000\u0000\u0000\u014a\u014b\u0005\u001d\u0000\u0000\u014b"+
		"\u014c\u00038\u001c\u0000\u014c\u014d\u0005\u001e\u0000\u0000\u014d\u014e"+
		"\u0003\u0016\u000b\u0000\u014e\u014f\u0005\u0001\u0000\u0000\u014f5\u0001"+
		"\u0000\u0000\u0000\u0150\u0155\u0003\u0018\f\u0000\u0151\u0155\u0003\u001c"+
		"\u000e\u0000\u0152\u0155\u0003\u0012\t\u0000\u0153\u0155\u0003\u0014\n"+
		"\u0000\u0154\u0150\u0001\u0000\u0000\u0000\u0154\u0151\u0001\u0000\u0000"+
		"\u0000\u0154\u0152\u0001\u0000\u0000\u0000\u0154\u0153\u0001\u0000\u0000"+
		"\u0000\u01557\u0001\u0000\u0000\u0000\u0156\u015b\u0003\u0018\f\u0000"+
		"\u0157\u015b\u0003\u001c\u000e\u0000\u0158\u015b\u0003\u0012\t\u0000\u0159"+
		"\u015b\u0003\u0014\n\u0000\u015a\u0156\u0001\u0000\u0000\u0000\u015a\u0157"+
		"\u0001\u0000\u0000\u0000\u015a\u0158\u0001\u0000\u0000\u0000\u015a\u0159"+
		"\u0001\u0000\u0000\u0000\u015b9\u0001\u0000\u0000\u0000\u015c\u015d\u0005"+
		"\u001f\u0000\u0000\u015d\u015e\u0005=\u0000\u0000\u015e\u015f\u0005 \u0000"+
		"\u0000\u015f\u0160\u0005@\u0000\u0000\u0160\u0161\u00050\u0000\u0000\u0161"+
		"\u0162\u0003\u0016\u000b\u0000\u0162\u0163\u0005\u0001\u0000\u0000\u0163"+
		";\u0001\u0000\u0000\u0000\u0164\u0165\u0005!\u0000\u0000\u0165\u0166\u0003"+
		"\u0016\u000b\u0000\u0166\u0167\u0005\u0001\u0000\u0000\u0167=\u0001\u0000"+
		"\u0000\u0000\u0168\u0169\u0005\"\u0000\u0000\u0169\u016a\u0003@ \u0000"+
		"\u016a\u016b\u0005=\u0000\u0000\u016b\u016c\u0003B!\u0000\u016c\u016d"+
		"\u0003L&\u0000\u016d?\u0001\u0000\u0000\u0000\u016e\u016f\u0007\u0001"+
		"\u0000\u0000\u016fA\u0001\u0000\u0000\u0000\u0170\u0179\u0005\u000b\u0000"+
		"\u0000\u0171\u0176\u0003D\"\u0000\u0172\u0173\u0005\u0004\u0000\u0000"+
		"\u0173\u0175\u0003D\"\u0000\u0174\u0172\u0001\u0000\u0000\u0000\u0175"+
		"\u0178\u0001\u0000\u0000\u0000\u0176\u0174\u0001\u0000\u0000\u0000\u0176"+
		"\u0177\u0001\u0000\u0000\u0000\u0177\u017a\u0001\u0000\u0000\u0000\u0178"+
		"\u0176\u0001\u0000\u0000\u0000\u0179\u0171\u0001\u0000\u0000\u0000\u0179"+
		"\u017a\u0001\u0000\u0000\u0000\u017a\u017b\u0001\u0000\u0000\u0000\u017b"+
		"\u017c\u0005\f\u0000\u0000\u017cC\u0001\u0000\u0000\u0000\u017d\u017e"+
		"\u0003F#\u0000\u017e\u017f\u0005=\u0000\u0000\u017fE\u0001\u0000\u0000"+
		"\u0000\u0180\u0181\u0007\u0002\u0000\u0000\u0181G\u0001\u0000\u0000\u0000"+
		"\u0182\u0183\u0005=\u0000\u0000\u0183\u0185\u0005\u000b\u0000\u0000\u0184"+
		"\u0186\u0003J%\u0000\u0185\u0184\u0001\u0000\u0000\u0000\u0185\u0186\u0001"+
		"\u0000\u0000\u0000\u0186\u0187\u0001\u0000\u0000\u0000\u0187\u0188\u0005"+
		"\f\u0000\u0000\u0188I\u0001\u0000\u0000\u0000\u0189\u018e\u0003V+\u0000"+
		"\u018a\u018b\u0005\u0004\u0000\u0000\u018b\u018d\u0003V+\u0000\u018c\u018a"+
		"\u0001\u0000\u0000\u0000\u018d\u0190\u0001\u0000\u0000\u0000\u018e\u018c"+
		"\u0001\u0000\u0000\u0000\u018e\u018f\u0001\u0000\u0000\u0000\u018fK\u0001"+
		"\u0000\u0000\u0000\u0190\u018e\u0001\u0000\u0000\u0000\u0191\u0196\u0005"+
		"\u0002\u0000\u0000\u0192\u0195\u0003\u0002\u0001\u0000\u0193\u0195\u0003"+
		"N\'\u0000\u0194\u0192\u0001\u0000\u0000\u0000\u0194\u0193\u0001\u0000"+
		"\u0000\u0000\u0195\u0198\u0001\u0000\u0000\u0000\u0196\u0194\u0001\u0000"+
		"\u0000\u0000\u0196\u0197\u0001\u0000\u0000\u0000\u0197\u0199\u0001\u0000"+
		"\u0000\u0000\u0198\u0196\u0001\u0000\u0000\u0000\u0199\u019d\u0005\u0003"+
		"\u0000\u0000\u019a\u019b\u0005\u0002\u0000\u0000\u019b\u019d\u0005\u0003"+
		"\u0000\u0000\u019c\u0191\u0001\u0000\u0000\u0000\u019c\u019a\u0001\u0000"+
		"\u0000\u0000\u019dM\u0001\u0000\u0000\u0000\u019e\u019f\u0005(\u0000\u0000"+
		"\u019f\u01a0\u0003V+\u0000\u01a0\u01a1\u0005\u0001\u0000\u0000\u01a1O"+
		"\u0001\u0000\u0000\u0000\u01a2\u01a3\u0005)\u0000\u0000\u01a3\u01a4\u0003"+
		"R)\u0000\u01a4\u01a5\u0005\u0001\u0000\u0000\u01a5\u01a8\u0001\u0000\u0000"+
		"\u0000\u01a6\u01a8\u0003T*\u0000\u01a7\u01a2\u0001\u0000\u0000\u0000\u01a7"+
		"\u01a6\u0001\u0000\u0000\u0000\u01a8Q\u0001\u0000\u0000\u0000\u01a9\u01aa"+
		"\u0006)\uffff\uffff\u0000\u01aa\u01ad\u0005@\u0000\u0000\u01ab\u01ad\u0003"+
		"V+\u0000\u01ac\u01a9\u0001\u0000\u0000\u0000\u01ac\u01ab\u0001\u0000\u0000"+
		"\u0000\u01ad\u01b3\u0001\u0000\u0000\u0000\u01ae\u01af\n\u0001\u0000\u0000"+
		"\u01af\u01b0\u00051\u0000\u0000\u01b0\u01b2\u0003R)\u0002\u01b1\u01ae"+
		"\u0001\u0000\u0000\u0000\u01b2\u01b5\u0001\u0000\u0000\u0000\u01b3\u01b1"+
		"\u0001\u0000\u0000\u0000\u01b3\u01b4\u0001\u0000\u0000\u0000\u01b4S\u0001"+
		"\u0000\u0000\u0000\u01b5\u01b3\u0001\u0000\u0000\u0000\u01b6\u01b7\u0005"+
		")\u0000\u0000\u01b7\u01b8\u0005,\u0000\u0000\u01b8\u01b9\u00050\u0000"+
		"\u0000\u01b9\u01ba\u0003\u0016\u000b\u0000\u01ba\u01bb\u0005\u0001\u0000"+
		"\u0000\u01bb\u01c9\u0001\u0000\u0000\u0000\u01bc\u01bd\u0005)\u0000\u0000"+
		"\u01bd\u01be\u0005-\u0000\u0000\u01be\u01bf\u00050\u0000\u0000\u01bf\u01c0"+
		"\u0003\u0016\u000b\u0000\u01c0\u01c1\u0005\u0001\u0000\u0000\u01c1\u01c9"+
		"\u0001\u0000\u0000\u0000\u01c2\u01c3\u0005)\u0000\u0000\u01c3\u01c4\u0005"+
		"*\u0000\u0000\u01c4\u01c5\u00050\u0000\u0000\u01c5\u01c6\u0003\u0016\u000b"+
		"\u0000\u01c6\u01c7\u0005\u0001\u0000\u0000\u01c7\u01c9\u0001\u0000\u0000"+
		"\u0000\u01c8\u01b6\u0001\u0000\u0000\u0000\u01c8\u01bc\u0001\u0000\u0000"+
		"\u0000\u01c8\u01c2\u0001\u0000\u0000\u0000\u01c9U\u0001\u0000\u0000\u0000"+
		"\u01ca\u01cb\u0006+\uffff\uffff\u0000\u01cb\u01e3\u0003H$\u0000\u01cc"+
		"\u01e3\u0005>\u0000\u0000\u01cd\u01e3\u0005=\u0000\u0000\u01ce\u01cf\u0005"+
		"\u000b\u0000\u0000\u01cf\u01d0\u0003V+\u0000\u01d0\u01d1\u0005\f\u0000"+
		"\u0000\u01d1\u01e3\u0001\u0000\u0000\u0000\u01d2\u01d3\u0005=\u0000\u0000"+
		"\u01d3\u01d4\u0005\u000f\u0000\u0000\u01d4\u01d5\u0003V+\u0000\u01d5\u01d6"+
		"\u0005\u0011\u0000\u0000\u01d6\u01e3\u0001\u0000\u0000\u0000\u01d7\u01d8"+
		"\u0005=\u0000\u0000\u01d8\u01d9\u0005\u000f\u0000\u0000\u01d9\u01da\u0003"+
		"V+\u0000\u01da\u01db\u0005\u0011\u0000\u0000\u01db\u01dc\u0005\u000f\u0000"+
		"\u0000\u01dc\u01dd\u0003V+\u0000\u01dd\u01de\u0005\u0011\u0000\u0000\u01de"+
		"\u01e3\u0001\u0000\u0000\u0000\u01df\u01e3\u0005.\u0000\u0000\u01e0\u01e3"+
		"\u0005/\u0000\u0000\u01e1\u01e3\u0005?\u0000\u0000\u01e2\u01ca\u0001\u0000"+
		"\u0000\u0000\u01e2\u01cc\u0001\u0000\u0000\u0000\u01e2\u01cd\u0001\u0000"+
		"\u0000\u0000\u01e2\u01ce\u0001\u0000\u0000\u0000\u01e2\u01d2\u0001\u0000"+
		"\u0000\u0000\u01e2\u01d7\u0001\u0000\u0000\u0000\u01e2\u01df\u0001\u0000"+
		"\u0000\u0000\u01e2\u01e0\u0001\u0000\u0000\u0000\u01e2\u01e1\u0001\u0000"+
		"\u0000\u0000\u01e3\u01ec\u0001\u0000\u0000\u0000\u01e4\u01e5\n\u000b\u0000"+
		"\u0000\u01e5\u01e6\u0007\u0003\u0000\u0000\u01e6\u01eb\u0003V+\f\u01e7"+
		"\u01e8\n\n\u0000\u0000\u01e8\u01e9\u0007\u0004\u0000\u0000\u01e9\u01eb"+
		"\u0003V+\u000b\u01ea\u01e4\u0001\u0000\u0000\u0000\u01ea\u01e7\u0001\u0000"+
		"\u0000\u0000\u01eb\u01ee\u0001\u0000\u0000\u0000\u01ec\u01ea\u0001\u0000"+
		"\u0000\u0000\u01ec\u01ed\u0001\u0000\u0000\u0000\u01edW\u0001\u0000\u0000"+
		"\u0000\u01ee\u01ec\u0001\u0000\u0000\u0000\u01ef\u01f0\u0005=\u0000\u0000"+
		"\u01f0\u01f1\u0005\u000f\u0000\u0000\u01f1\u01f2\u0005>\u0000\u0000\u01f2"+
		"\u01f3\u0005\u0011\u0000\u0000\u01f3\u01f4\u0005\u000f\u0000\u0000\u01f4"+
		"\u01f5\u0005>\u0000\u0000\u01f5\u0203\u0005\u0011\u0000\u0000\u01f6\u01f7"+
		"\u0005=\u0000\u0000\u01f7\u01f8\u0005\u000f\u0000\u0000\u01f8\u01f9\u0005"+
		"\u0011\u0000\u0000\u01f9\u01fa\u0005\u000f\u0000\u0000\u01fa\u0203\u0005"+
		"\u0011\u0000\u0000\u01fb\u01fc\u0005=\u0000\u0000\u01fc\u01fd\u0005\u000f"+
		"\u0000\u0000\u01fd\u01fe\u0005>\u0000\u0000\u01fe\u0203\u0005\u0011\u0000"+
		"\u0000\u01ff\u0200\u0005=\u0000\u0000\u0200\u0201\u0005\u000f\u0000\u0000"+
		"\u0201\u0203\u0005\u0011\u0000\u0000\u0202\u01ef\u0001\u0000\u0000\u0000"+
		"\u0202\u01f6\u0001\u0000\u0000\u0000\u0202\u01fb\u0001\u0000\u0000\u0000"+
		"\u0202\u01ff\u0001\u0000\u0000\u0000\u0203Y\u0001\u0000\u0000\u0000\u0204"+
		"\u0205\u0003F#\u0000\u0205\u0208\u0003X,\u0000\u0206\u0207\u0005\t\u0000"+
		"\u0000\u0207\u0209\u0003\\.\u0000\u0208\u0206\u0001\u0000\u0000\u0000"+
		"\u0208\u0209\u0001\u0000\u0000\u0000\u0209\u020a\u0001\u0000\u0000\u0000"+
		"\u020a\u020b\u0005\u0001\u0000\u0000\u020b\u0215\u0001\u0000\u0000\u0000"+
		"\u020c\u020d\u0003F#\u0000\u020d\u0210\u0005=\u0000\u0000\u020e\u020f"+
		"\u0005\t\u0000\u0000\u020f\u0211\u0003V+\u0000\u0210\u020e\u0001\u0000"+
		"\u0000\u0000\u0210\u0211\u0001\u0000\u0000\u0000\u0211\u0212\u0001\u0000"+
		"\u0000\u0000\u0212\u0213\u0005\u0001\u0000\u0000\u0213\u0215\u0001\u0000"+
		"\u0000\u0000\u0214\u0204\u0001\u0000\u0000\u0000\u0214\u020c\u0001\u0000"+
		"\u0000\u0000\u0215[\u0001\u0000\u0000\u0000\u0216\u0217\u0005\u000f\u0000"+
		"\u0000\u0217\u021c\u0003V+\u0000\u0218\u0219\u0005\u0004\u0000\u0000\u0219"+
		"\u021b\u0003V+\u0000\u021a\u0218\u0001\u0000\u0000\u0000\u021b\u021e\u0001"+
		"\u0000\u0000\u0000\u021c\u021a\u0001\u0000\u0000\u0000\u021c\u021d\u0001"+
		"\u0000\u0000\u0000\u021d\u021f\u0001\u0000\u0000\u0000\u021e\u021c\u0001"+
		"\u0000\u0000\u0000\u021f\u0220\u0005\u0011\u0000\u0000\u0220\u022e\u0001"+
		"\u0000\u0000\u0000\u0221\u022a\u0005\u000f\u0000\u0000\u0222\u0227\u0003"+
		"\\.\u0000\u0223\u0224\u0005\u0004\u0000\u0000\u0224\u0226\u0003\\.\u0000"+
		"\u0225\u0223\u0001\u0000\u0000\u0000\u0226\u0229\u0001\u0000\u0000\u0000"+
		"\u0227\u0225\u0001\u0000\u0000\u0000\u0227\u0228\u0001\u0000\u0000\u0000"+
		"\u0228\u022b\u0001\u0000\u0000\u0000\u0229\u0227\u0001\u0000\u0000\u0000"+
		"\u022a\u0222\u0001\u0000\u0000\u0000\u022a\u022b\u0001\u0000\u0000\u0000"+
		"\u022b\u022c\u0001\u0000\u0000\u0000\u022c\u022e\u0005\u0011\u0000\u0000"+
		"\u022d\u0216\u0001\u0000\u0000\u0000\u022d\u0221\u0001\u0000\u0000\u0000"+
		"\u022e]\u0001\u0000\u0000\u0000\u022f\u0230\u0005=\u0000\u0000\u0230\u0231"+
		"\u0005\t\u0000\u0000\u0231\u0232\u0003V+\u0000\u0232\u0233\u0005\u0001"+
		"\u0000\u0000\u0233\u0237\u0001\u0000\u0000\u0000\u0234\u0235\u0005=\u0000"+
		"\u0000\u0235\u0237\u0005\u0001\u0000\u0000\u0236\u022f\u0001\u0000\u0000"+
		"\u0000\u0236\u0234\u0001\u0000\u0000\u0000\u0237_\u0001\u0000\u0000\u0000"+
		"\u0238\u0239\u0005=\u0000\u0000\u0239\u023a\u0005\u000f\u0000\u0000\u023a"+
		"\u023b\u0005>\u0000\u0000\u023b\u023c\u0005\u0011\u0000\u0000\u023c\u023d"+
		"\u0005\t\u0000\u0000\u023d\u023e\u0003V+\u0000\u023e\u023f\u0005\u0001"+
		"\u0000\u0000\u023f\u024c\u0001\u0000\u0000\u0000\u0240\u0241\u0005=\u0000"+
		"\u0000\u0241\u0242\u0005\u000f\u0000\u0000\u0242\u0243\u0005>\u0000\u0000"+
		"\u0243\u0244\u0005\u0011\u0000\u0000\u0244\u0245\u0005\u000f\u0000\u0000"+
		"\u0245\u0246\u0005>\u0000\u0000\u0246\u0247\u0005\u0011\u0000\u0000\u0247"+
		"\u0248\u0005\t\u0000\u0000\u0248\u0249\u0003V+\u0000\u0249\u024a\u0005"+
		"\u0001\u0000\u0000\u024a\u024c\u0001\u0000\u0000\u0000\u024b\u0238\u0001"+
		"\u0000\u0000\u0000\u024b\u0240\u0001\u0000\u0000\u0000\u024ca\u0001\u0000"+
		"\u0000\u00003dfy\u007f\u0082\u008f\u0096\u0099\u00a1\u00ac\u00b5\u00bd"+
		"\u00cf\u00d7\u00db\u00e6\u00f5\u00fd\u00ff\u0116\u011e\u0120\u0125\u0138"+
		"\u0142\u0154\u015a\u0176\u0179\u0185\u018e\u0194\u0196\u019c\u01a7\u01ac"+
		"\u01b3\u01c8\u01e2\u01ea\u01ec\u0202\u0208\u0210\u0214\u021c\u0227\u022a"+
		"\u022d\u0236\u024b";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}