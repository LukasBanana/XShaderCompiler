
// HLSL (Shader Model 5) Grammar
// 09/10/2014

grammar HLSL;

program : global_decl_list;

global_decl_list	: global_decl*;
global_decl			: function_decl
					| buffer_decl
					| texture_decl
					| samplerstate_decl
					| struct_decl ';';

code_block	: '{' stmnt_list '}';
code_body	: stmnt
			| code_block

// Statements

stmnt_list	: stmnt*;
stmnt		: ';'
			| code_block
			| for_loop_stmnt
			| while_loop_stmnt
			| do_while_loop_stmnt ';'
			| if_stmnt
			| switch_stmnt
			| var_decl_stmnt ';'
			| assign_stmnt ';'
			| function_call_stmnt ';'
			| return_stmnt ';'
			| struct_decl ';'
			| CTRL_TRANSFER_STMNT;

// Variables

array_access		: '[' expr ']';
var_single_ident	: IDENT array_access*;
var_ident			: var_single_ident ('.' var_single_ident)*;

// Expressions

expr			: literal
				| binary_expr
				| unary_expr
				| post_unary_expr
				| function_call_stmnt
				| bracket_expr
				| cast_expr
				| var_access_expr;

binary_expr		: expr BINARY_OP expr;
unary_expr		: UNARY_OP expr;
post_unary_expr	: expr POST_UNARY_OP;
bracket_expr	: '(' expr ')';
cast_expr		: '(' var_type ')' expr;
var_access_expr	: var_ident (ASSIGN_OP expr)?;

// Operators

ASSIGN_OP		: '='
				| '+='
				| '-='
				| '*='
				| '/='
				| '%='
				| '<<='
				| '>>='
				| '&='
				| '|='
				| '^=';
BINARY_OP		: '+'
				| '-'
				| '*'
				| '/'
				| '%'
				| '<'
				| '>'
				| '<='
				| '>='
				| '=='
				| '&'
				| '&&'
				| '|'
				| '||'
				| '^'
				| ASSIGN_OP;
POST_UNARY_OP	: '++'
				| '--';
UNARY_OP		: POST_UNARY_OP
				| '~'
				| '-';

// Loop statements

for_loop_stmnt	: 'for' '(' for_init? ';' expr? ';' expr? ')' code_block;
for_init		: assign_stmnt
				| var_decl_stmnt;

while_loop_stmnt	: 'while' '(' expr ')' code_body;
do_while_loop_stmnt	: 'do' code_block 'while' '(' expr ')';

// Conditional statements

if_stmnt	: 'if' '(' expr ')' code_body else_stmnt?;
else_stmnt	: 'else' code_body;

switch_stmnt		: 'switch' '(' expr ')' '{' switch_case_list '}';
switch_case_list	: switch_case* switch_default_case?;
switch_case			: 'case' expr ':' stmnt_list;
switch_default_case		: 'default' ':' stmnt_list;

// Variable declaration

INTERP_MODIFIER			: 'linear'
						| 'centroid'
						| 'nointerpolation'
						| 'noperspective'
						| 'sample';

STORAGE_CLASS			: 'extern';
						| 'nointerpolation'
						| 'precise'
						| 'shared'
						| 'groupshared'
						| 'static'
						| 'uniform'
						| 'volatile';

TYPE_MODIFIER			: 'const'
						| 'row_major'
						| 'column_major';

var_type				: SCALAR_TYPE
						| VECTOR_TYPE
						| MATRIX_TYPE
						| TEXTURE_TYPE
						| BUFFER_TYPE
						| SAMPLER_STATE_TYPE
						| IDENT
						| struct_decl;

VECTOR_SINGLE_COMPONENT	: 'x'
						| 'y'
						| 'z'
						| 'w'
						| 'r'
						| 'g'
						| 'b'
						| 'a';

VECTOR_COMPONENT		: VECTOR_SINGLE_COMPONENT (VECTOR_SINGLE_COMPONENT (VECTOR_SINGLE_COMPONENT VECTOR_SINGLE_COMPONENT?)?)?;

REGISTER_IDENT			: 'b'
						| 't'
						| 'c'
						| 's'
						| 'u';

REGISTER_NAME			: REGISTER_IDENT INT_LITERAL;
SEMANTIC				: ':' IDENT;
initializer				: '=' expr;
array_dimension			: '[' expr ']';
var_packoffset			: 'packoffset' '(' REGISTER_NAME ('.' VECTOR_COMPONENT)? ')';
var_register			: 'register' '(' REGISTER_NAME ')';

var_semantic			: SEMANTIC
						| var_packoffset
						| var_register;

var_decl				: IDENT array_dimension* var_semantic* initializer?;
var_decl_stmnt			: (STORAGE_CLASS | INTERP_MODIFIER)* TYPE_MODIFIER? var_type var_decl (',' var_decl)*;

// Function calls

function_name		: IDENT
					| VECTOR_TYPE
					| MATRIX_TYPE;
function_call_stmnt	: function_name '(' argument_list ')';
argument_list		: ( epsilion | expr ( ',' expr )* );

// Other statements

assign_stmnt	: var_ident ASSIGN_OP expr;

return_stmnt	: 'return' expr?;

CTRL_TRANSFER_STMNT	: 'break'
					| 'continue'
					| 'discard';

// Function declaration

return_type		: var_type
				| VOID;
function_decl	: return_type IDENT '(' parameter_list ')' SEMANTIC? code_block;

INPUT_MODIFIER	: 'in'
				| 'out'
				| 'inout'
				| 'uniform';

INTERP_MODIFIER		: 'linear'
					| 'centroid'
					| 'nointerpolation'
					| 'noperspective'
					| 'sample';

parameter_list	: ( epsilion | parameter ( ',' parameter )* );
parameter		: INPUT_MODIFIER? var_type IDENT SEMANTIC? INTERP_MODIFIER? initializer?;

// Struct declaration

struct_decl			: 'struct' IDENT? '{' struct_decl_body '}';
struct_decl_body	: (var_decl_stmnt ';')*;


/*
| buffer_decl
| texture_decl
| samplerstate_decl
*/

