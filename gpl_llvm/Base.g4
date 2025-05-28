grammar Base;

// Entry point
program: (statement)* EOF;

GRAPH: 'graph';

// Statements
statement:
	loopStatement
	//| printStatement
	| varDecl
    | assignmentStatement
	| conditionalStatement
	| graphDef	
	| ';';

// Graph Definition

graphDef: GRAPH graphID '{' nodes? edges? '}' ';';
nodes: 'nodes:' nodeList ';';
edges: 'edges:' (edgeList | fileEdgeList) ';';
nodeList: nodeID (',' nodeID)*;
edgeList: edge (',' edge)*;
graphID: ID;
nodeID: INT;
fileEdgeList: 'file' STRING;
edge: nodeID '->' nodeID;

varDecl:
	type ID ('=' expr)? ';' ;

conditionalStatement:
	'if' '(' condition ')' block (
		'else' conditionalStatement
		| 'else' block
	)?;

condition:
	condition AND condition		# LogicalAnd
	| condition OR condition	# LogicalOr
	| expr ( EQUAL | NOTEQUAL | LESSEQUAL | GREATEREQUAL | LESSTHAN | GREATERTHAN ) expr # Relational;



loopStatement : whileStatement;
whileStatement: 'while' '(' condition ')' block;


assignmentStatement: ID '=' expr ';' | ID ';';



block: '{' (statement )* '}' | '{' '}';

type: 'graph' | 'vertex' | 'edge' | 'int' | 'string' | 'real' | 'bool';

// Print
// printStatement: 'print' printExpr ';' ;
// printExpr: STRING | expr | printExpr '+' printExpr;

expr:
	expr (TIMES | DIVIDE) expr	# MulDivExpr
	| expr (PLUS | MINUS) expr	# AddSubExpr
	| INT						# IntExpr
	| ID						# IdExpr
	| '(' expr ')'				# ParenExpr
	| TRUE						# BoolTrueExpr
	| FALSE						# BoolFalseExpr
	| REAL						# RealExpr;
// | nodeID                	# nodeExpr


// assignment
// assignmentStatement: ID '=' expr ';' | ID ';';

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
