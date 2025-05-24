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
	| ';';

// Graph Definition

// assignment varDecl: type ID ('=' expr)? ';' | type ID '=' functionCall ';' ;
varDecl:
	type ID ('=' expr)? ';' ;
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
