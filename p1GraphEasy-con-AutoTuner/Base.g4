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
	| graphListDecl
	| nodeEdgeOperation
	| setOperation
	| setMethodCall
	| swapStatement
	| ';';

swapStatement: 'swap' '(' ID ',' ID ')' ';';

// Graph Definition
// One rule, not two labelled alternatives: weightedness is just the optional
// `TRUE` marker, so `weighted?` says it directly instead of duplicating the
// whole production.  Sections are in fixed order, which also removes the four
// interleaved graphProperty* repetitions the previous form needed.
graphDef
    : GRAPH graphID '{' graphProperty? nodes? edges? weighted? '}' ';' ;

// `directed` is a flag that can only be true, so it carries no value -- present
// means directed, absent means undirected.  It is what makes a motif's `->`
// meaningful: on an undirected graph every edge is stored both ways, so a -> b
// and b -> a are the same edge and directed motifs collapse.
graphProperty : 'directed' ';' ;

weighted      : 'TRUE' ;

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
	| expr					# ExprCondition;

//graphcondition
graphComprehension:
	ID '=' '[' graphExpr ('where' graphCondition)? ']' ';';

graphExpr:
	graphID ((AND | OR) graphID)*;

// A motif is a fixed, compile-time-known subgraph pattern.  `motifs` collects
// the vertex bindings, `graphs` collects one induced subgraph per match.
motifMatchesDecl:
	'motifs' ID '=' '[' graphID 'where' 'motif' '{' motifEdge+ '}' ']' ';';

graphListDecl:
	'graphs' ID '=' '[' graphID 'where' 'motif' '{' motifEdge+ '}' ']' ';';

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
foreachStatement: 'for' 'each' loopTarget 'in' graphID block;
loopTarget:
	'vertex' ID					# forEachVertex
	| 'edge' ID ',' ID			# forEachEdge
	| 'neighbor' ID 'of' expr	# forEachAdj
	| 'element' ID				# forEachElement
	| 'graph' ID				# ForEachGraph
	| 'motif' '(' ID (',' ID)+ ')'	# ForEachMotif
	| ID						# forEachPlain;
whileStatement: 'while' '(' condition ')' block;

nodeEdgeOperation: addOperation | removeOperation;

addOperation: 'add' graphUpdateTargets 'to' graphID ';';
removeOperation: 'remove' graphUpdateTargets 'from' graphID ';';
graphUpdateTargets: updateNodeTargetList | updateEdgeTargetList;
updateNodeTargetList: expr (',' expr)*;
updateEdgeTargetList: updateEdgeTarget (',' updateEdgeTarget)*;
updateEdgeTarget: expr '->' expr;



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
functionCall: functionName '(' argumentList? ')';
functionName: ID | 'degree';
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
