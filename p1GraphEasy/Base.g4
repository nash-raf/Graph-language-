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
	| nodeEdgeOperation
	| setOperation
	| setMethodCall
	| ';';

// Graph Definition
graphDef
    : GRAPH graphID '{' nodes? edges? 'TRUE' '}' ';'   # WeightedGraphDef
    | GRAPH graphID '{' nodes? edges? '}' ';'          # UnweightedGraphDef
;

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
	| '(' graphCondition ')'	# ParenGraphCondition;

//loop
loopStatement: foreachStatement | whileStatement;
foreachStatement: 'for' 'each' loopTarget 'in' graphID block;
loopTarget:
	'vertex' ID					# forEachVertex
	| 'edge' ID ',' ID			# forEachEdge
	| 'neighbor' ID 'of' expr	# forEachAdj
	| 'element' ID				# forEachElement
	| ID						# forEachPlain;
whileStatement: 'while' '(' condition ')' block;

nodeEdgeOperation: addOperation | removeOperation;

addOperation: 'add' addTargets 'to' graphID ';';
removeOperation: 'remove' removeTargets 'from' graphID ';';
addTargets: nodeID | edge | nodeList | edgeList;
removeTargets: nodeID | edge | nodeList | edgeList;



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

functionCall: ID '(' argumentList? ')';
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
