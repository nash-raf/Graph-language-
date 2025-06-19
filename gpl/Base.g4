grammar Base;

// Entry point
program: (statement | function)* EOF;

GRAPH: 'graph';

// Statements
statement:
	graphDef
	| conditionalStatement
	| printStatement
	| loopStatement
	| varDecl
	| functionCall
	| graphComprehension
	| arrayAssignStatement
	| assignmentStatement
	| queryStatement
	| showgraph
	| nodeEdgeOperation
	| ';';

// Graph Definition
graphDef: GRAPH graphID '{' nodes? edges? '}' ';';

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
	| edge 'in' graphID		# EdgeCheck;

//graphcondition
graphComprehension:
	ID '=' '[' graphID 'where' graphCondition ']' ';';

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
	| '(' graphCondition ')'	# ParenGraphCondition;

//loop
loopStatement: foreachStatement | whileStatement;
foreachStatement: 'for' 'each' loopTarget 'in' graphID block;
loopTarget:
	'vertex' ID					# forEachVertex
	| 'edge' ID ',' ID			# forEachEdge
	| 'neighbor' ID 'of' nodeID	# forEachAdj;
whileStatement: 'while' '(' condition ')' block;

nodeEdgeOperation: addOperation | removeOperation;

addOperation: 'add' addTargets 'to' graphID ';';
removeOperation: 'remove' removeTargets 'from' graphID ';';
addTargets: nodeID | edge | nodeList | edgeList;
removeTargets: nodeID | edge | nodeList | edgeList;

queryStatement: 'query' ID ':' STRING 'of' graphID ';';

showgraph: 'show' graphID ';';
//functions

function: 'fn' returnType ID paramList block;
returnType:
	'graph'
	| 'vertex'
	| 'edge'
	| 'int'
	| 'void'
	| 'string';
paramList: '(' (param (',' param)*)? ')';
param: type ID;
type:
	'graph'
	| 'vertex'
	| 'edge'
	| 'int'
	| 'string'
	| 'real'
	| 'bool';

functionCall: ID '(' argumentList? ')';
argumentList: expr (',' expr)*;

block: '{' (statement | returnStatement)* '}' | '{' '}';
returnStatement: 'return' expr ';';

// Print
printStatement: 'print' printExpr ';' | printgraph;
printExpr: STRING | expr | printExpr '+' printExpr;

printgraph:
	'print' EDGE OF graphID ';'		# edgePrint
	| 'print' NODE OF graphID ';'	# nodePrint
	| 'print' GRAPH OF graphID ';'	# graphPrint;

// Expressions
expr:
	expr (TIMES | DIVIDE) expr	# MulDivExpr
	| expr (PLUS | MINUS) expr	# AddSubExpr
	| functionCall				# FuncExpr
	| INT						# IntExpr
	| ID						# IdExpr
	| '(' expr ')'				# ParenExpr
	| ID '[' expr ']'			# ArrayAccessExpr
	| TRUE						# BoolTrueExpr
	| FALSE						# BoolFalseExpr
	| REAL						# RealExpr;
// | nodeID                	# nodeExpr

// Array 
arrayDeclarator:
	ID '[' INT ']'	# SizedArray
	| ID '[' ']'	# UnsizedArray;

arrayInitializer: '[' expr (',' expr)* ']'; // Array literal

// assignment
assignmentStatement: ID '=' expr ';' | ID ';';

arrayAssignStatement:
	ID '[' expr ']' '=' expr ';' ;

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