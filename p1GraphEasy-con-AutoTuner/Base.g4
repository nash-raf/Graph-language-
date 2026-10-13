grammar Base;

// Entry point
program: (statement | function)* EOF;

GRAPH: 'graph';
WEIGHTS : 'weights' ;


// Statements
statement:
	graphDef
	| conditionalStatement
	| printStatement
	//| loopStatement
	| whileStatement
	| foreachStatement
	| varDecl
	| setDecl
	| functionCall ';'
	| sleepStatement
	| graphComprehension
	| arrayAssignStatement
	| assignmentStatement
	| queryStatement
	| showgraph
	| drawgraph
	| drawmotifs
	| motifMatchesDecl
	| motifSearchDecl
	| graphListDecl
	| graphUpdateCall
	| setOperation
	| setMethodCall
	| swapStatement
	| ';';

swapStatement: 'swap' '(' ID ',' ID ')' ';';

// Graph Definition.
// The compiled parser (generated/, 09-07) carries labelled alternatives for the
// two braced forms and no separate `weighted` rule; this text reproduces that
// shape, then adds the loaded/bare forms of plan rows 1-4.  The two braced
// alternatives are deliberately left modifier-free so regenerating them is a
// no-op (gate: the context-class set must not change for them).
graphDef
    : GRAPH graphID '{' graphProperty* nodes? edges? 'TRUE' '}' ';'   # weightedGraphDef
    | GRAPH graphID '{' graphProperty* nodes? edges? '}' ';'          # unweightedGraphDef
    | ('directed' | 'weighted')* GRAPH graphID ';'                    # plainGraphDef
    | ('directed' | 'weighted')* GRAPH graphID '=' 'load' '(' STRING ')' ';'  # loadGraphDef
    ;

// `directed` is a flag that can only be true, so it carries no value -- present
// means directed, absent means undirected.  It is what makes a motif's `->`
// meaningful: on an undirected graph every edge is stored both ways, so a -> b
// and b -> a are the same edge and directed motifs collapse.
// `directed: true;` inside a braced declaration (the compiled parser's shape --
// see the accessor BaseParser::GraphPropertyContext::drawBoolLiteral()).
graphProperty : 'directed' ':' drawBoolLiteral ';' ;

//in graphDef
nodes: 'nodes:' nodeList ';';
edges: 'edges:' (edgeList | fileEdgeList) ';';
nodeList: nodeID (',' nodeID)*;
edgeList: edge (',' edge)*;
graphID: ID;
nodeID: INT;
fileEdgeList: 'file' STRING;
edge: nodeID '->' nodeID;

// assignment varDecl: type ID ('=' expr)? ';' | type ID '=' functionCall ';' ;
varDecl:
	type ID ('=' expr)? ';'								# SimpleDeclaration
	| type arrayDeclarator ('=' arrayInitializer)? ';'	# ArrayDeclaration;

setDecl:
    'set' ID ';'
    | 'set' ID '=' setInitializer ';'
    | 'set' ID '=' setExpr ';'
    ;

setInitializer:
    '{' (expr (',' expr)*)? '}'
    ;

// Set operations
setOperation:
    ID '=' setExpr ';'
    ;

setTarget:
    ID
    | graphID '.' 'nodes'
    | graphID '.' 'edges'
    ;

setExpr:
    setExpr UNION setExpr          # SetUnion
    | setExpr INTERSECT setExpr    # SetIntersect
	| graphID '.' 'nodes'          # GraphNodesSet
    | graphID '.' 'edges'          # GraphEdgesSet
    | ID                           # SetId
    | setInitializer               # SetLiteral
    | '(' setExpr ')'              # ParenSet
    ;

// Set method calls
setMethodCall:
	setTarget '.' 'add' '(' expr ')' ';'      # SetAddMethod
    | setTarget '.' 'remove' '(' expr ')' ';' # SetRemoveMethod
    ;

// if-else
conditionalStatement:
	'if' '(' condition ')' block (
		'else' conditionalStatement
		| 'else' block
	)?;
condition:
	condition AND condition		# LogicalAnd
	| condition OR condition	# LogicalOr
	| expr (
		EQUAL
		| NOTEQUAL
		| LESSEQUAL
		| GREATEREQUAL
		| LESSTHAN
		| GREATERTHAN
	) expr					# Relational
	| nodeID 'in' graphID	# NodeCheck
	| edge 'in' graphID		# EdgeCheck
	| graphID '.' 'hasNode' '(' expr ')'			# HasNodeCheck
	| graphID '.' 'hasEdge' '(' expr ',' expr ')'	# HasEdgeCheck
	| expr					# ExprCondition;

//graphcondition
graphComprehension:
	GRAPH? ID '=' '[' graphExpr ('where' graphCondition)? ']' ';'  ;

graphExpr:
	graphID ((AND | OR) graphID)*;

// A motif is a fixed, compile-time-known subgraph pattern.  `motifs` collects
// the vertex bindings, `graphs` collects one induced subgraph per match.
motifMatchesDecl:
	'motifs' ID '=' '[' graphID 'where' 'motif' '{' motifEdge+ '}' ']' ';';

graphListDecl:
	'graphs' ID '=' '[' graphID 'where' 'motif' '{' motifEdge+ '}' ']' ';';

// Row 13 (adopted style): `M = in G search motif { a->b; b->c; };`
// Builds the same bindings node (MotifMatchesDeclNode) the `motifs M = [...]`
// spelling builds, so the analyzer and IRGen paths are identical.
motifSearchDecl:
	ID '=' 'in' graphID 'search' 'motif' '{' motifEdge+ '}' ';';





// '->' requires a positive edge, '-|' a negative one.
motifEdge: ID ('->' | '-|') ID ';';

graphCondition:
	graphCondition AND graphCondition	# GraphLogicalAnd
	| graphCondition OR graphCondition	# GraphLogicalOr
	| 'degree' (
		EQUAL
		| NOTEQUAL
		| LESSEQUAL
		| GREATEREQUAL
		| LESSTHAN
		| GREATERTHAN
	) INT						# DegreeCondition
	| 'connected' 'with' nodeID	# ConnectedCondition
	| 'cycle'					# CycleCondition
	| 'motif' '{' motifEdge+ '}'	# MotifCondition
	| '(' graphCondition ')'	# ParenGraphCondition;

//loop
loopStatement: foreachStatement | whileStatement;
foreachStatement: 'for' 'each' loopTarget 'in' graphID block | memberLoopStatement;

// Rows 5-7: the member-style loop headers.
//   for v in G.nodes            -> the CSR vertex scan (NOT a materialized set)
//   for (u, v) in G.edges       -> the edge scan
//   for u in G.neighbors(v)     -> the neighbour scan
// Each builds the same foreach AST node the classic spelling builds, so the
// backend is untouched; `G.nodes` in a set context keeps its set meaning.
memberLoopStatement
    : 'for' ID 'in' graphID '.' 'nodes' block                    # foreachNodes
    | 'for' '(' ID ',' ID ')' 'in' graphID '.' 'edges' block     # foreachEdges
    | 'for' ID 'in' graphID '.' 'neighbors' '(' expr ')' block   # foreachNeighbors
    ;
loopTarget:
	'element' ID				# forEachElement
	| 'graph' ID				# ForEachGraph
	| 'motif' '(' ID (',' ID)+ ')'	# ForEachMotif
	| ID						# forEachPlain;
whileStatement: 'while' '(' condition ')' block;

// Plan row 1 + the old `add … to G` removal: graph mutation by method call,
// literals only.  All four desugar in the builder to the existing
// GraphUpdateNode (Add / Remove), so no new AST node kind is needed.
graphUpdateCall
    : graphID '.' 'addNodes' '(' nodeList? ')' ';'      # GraphAddNodesCall
    | graphID '.' 'addEdges' '(' edgeList? ')' ';'      # GraphAddEdgesCall
    | graphID '.' 'removeNodes' '(' nodeList? ')' ';'   # GraphRemoveNodesCall
    | graphID '.' 'removeEdges' '(' edgeList? ')' ';'   # GraphRemoveEdgesCall
    ;

// (the old `add … to G` / `remove … from G` family is deleted: the call form
// `G.addNodes/addEdges/removeNodes/removeEdges` is the only mutation surface.)



queryStatement: 'query' ID ':' STRING INT? 'of' graphID ';';

showgraph: 'show' graphID ';';
//functions

function: 'fn' returnType ID paramList block;
returnType:
	'graph'
	| 'vertex'
	| 'edge'
	| 'int'
	| 'real'
	| 'bool'
	| 'void'
	| 'string'
	| 'set';
paramList: '(' (param (',' param)*)? ')';
param: type ID;
type:
	'graph'
	| 'vertex'
	| 'edge'
	| 'int'
	| 'string'
	| 'real'
	| 'bool'
	| 'set';

drawgraph:
	'draw' graphID 'to' STRING ('{' drawOption* '}')? ';';

drawmotifs:
	'draw' ('motif' | 'motifs') 'of' graphID 'to' STRING '{' drawMotifOption* '}' ';';

drawMotifOption:
	motifEdge
	| 'layout' ':' STRING ';'
	| 'vertices' '{' vertexDrawOption* '}'
	| 'edges' '{' edgeDrawOption* '}';

drawOption:
	'layout' ':' STRING ';'
	| 'vertices' '{' vertexDrawOption* '}'
	| 'edges' '{' edgeDrawOption* '}';

vertexDrawOption:
	'labels' ':' drawBoolLiteral ';'
	| 'color' ':' colorMapping ';'
	| 'size' ':' (continuousMapping | ID) ';';

edgeDrawOption:
	'labels' ':' drawBoolLiteral ';';

colorMapping:
	'categorical' '(' ID ')'
	| continuousMapping
	| ID;

continuousMapping:
	'continuous' '(' ID ')';

drawBoolLiteral: TRUE | FALSE | 'true' | 'false';

// 'degree' is an implicit keyword token (it appears in graphCondition), so it
// cannot match ID.  Without this alternative the degree() builtin -- which
// IRGen already implements as an O(1) row_ptr[v+1]-row_ptr[v] -- is
// unreachable from the surface syntax.
functionCall: (ID | 'degree') '(' argumentList? ')';
argumentList: expr (',' expr)*;

// Sleep statement
sleepStatement: 'sleep' '(' expr ')' ';';

block: '{' (statement | returnStatement | breakStatement | continueStatement)* '}' | '{' '}';
returnStatement: 'return' expr ';';
breakStatement: 'break' ';';
continueStatement: 'continue' ';';

// Print
printStatement: 'print' printExpr ';' | printArrayStatement | printgraph;
printExpr: STRING | expr | printExpr '+' printExpr;
printArrayStatement
    : 'print' ID '[' expr ']' ';'
    ;

printgraph:
	'print' EDGE OF graphID ';'		# edgePrint
	| 'print' NODE OF graphID ';'	# nodePrint
	| 'print' GRAPH OF graphID ';'	# graphPrint;

// Expressions
expr:
	expr (AND | OR) expr		# LogicalExpr
	| expr (TIMES | DIVIDE | MODULO) expr	# MulDivExpr
	| expr (PLUS | MINUS) expr	# AddSubExpr
	| MINUS expr				# UnaryMinusExpr
	| NOT expr					# NotExpr
	| functionCall				# FuncExpr
	| INT						# IntExpr
	| ID						# IdExpr
	| '(' expr ')'				# ParenExpr
	| ID '[' expr ']' '[' expr ']'  # Array2DAccessExpr
	| ID '[' expr ']'			# ArrayAccessExpr
	| setTarget '.' 'contains' '(' expr ')'  # SetContainsExpr
	| ID '.' 'size' '(' ')'	# SetSizeExpr
	| ID '.' 'pop' '(' ')'		# SetPopExpr
	| TRUE						# BoolTrueExpr
	| FALSE						# BoolFalseExpr
	| ID '[]'					# ArrayPrint
	| REAL						# RealExpr
	| setInitializer            # SetLitExpr
	| 'INF'					# InfExpr
	| 'timer' '(' ')'			# TimerExpr;
// | nodeID                	# nodeExpr

SET: 'set';
UNION: 'union';
INTERSECT: 'intersect';
NOT: '!';
MODULO: '%';


// Array 
arrayDeclarator:
	ID '[' expr ']' '[' expr ']'	# Sized2DArray
	| ID '[' expr ']'				# SizedArray
	| ID '[' ']'					# UnsizedArray;

arrayInitializer: '[' expr (',' expr)* ']'; // Array literal

// assignment
assignmentStatement: ID '=' expr ';' | ID ';';

arrayAssignStatement:
	ID '[' expr ']' '[' expr ']' '=' expr ';' # Array2DAssignStmt
	| ID '[' expr ']' '=' expr ';' # ArrayAssignStmt;

// op: '==' | '!=' | '<' | '>' | '<=' | '>=' | '||' | '&&'; // Tokens
EDGE: 'edges';
NODE: 'nodes';
TRUE: 'TRUE';
FALSE: 'FALSE';
OF: 'of';
PLUS: '+';
MINUS: '-';
TIMES: '*';
DIVIDE: '/';
AND: '&&';
OR: '||';
EQUAL: '==';
NOTEQUAL: '!=';
LESSTHAN: '<';
GREATERTHAN: '>';
LESSEQUAL: '<=';
GREATEREQUAL: '>=';
weights: 'TRUE' | 'FALSE';
ID: [a-zA-Z_][a-zA-Z0-9_]*;
INT: [0-9]+;
REAL: [0-9]+ '.' [0-9]+;
//STRING: '"' (~["\r\n])* '"';
STRING:
	["] (~["\r\n\\] | '\\' ~[\r\n])* ["]
	| ['] ( ~['\r\n\\] | '\\' ~[\r\n])* ['];

// Whitespace and Comments COMMENT: '//' ~[\r\n]* -> skip;
Comment: ( '//' ~[\r\n]* | '/*' .*? '*/') -> skip;
WS: [ \t\r\n]+ -> skip;
