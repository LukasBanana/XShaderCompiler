
// HLSL (Shader Model 5) Grammar
// 09/10/2014

grammar HLSL;

program : global_decl_list;

global_decl_list	: global_decl*;
global_decl			: function_decl
					| buffer_decl
					| texture_decl
					| samplerstate_decl
					| struct_decl;

code_block	: '{' stmnt_list '}';
code_body	: stmnt
			| code_block

stmnt_list	: stmnt*;
stmnt		: ';'
			| for_loop_stmnt
			| while_loop_stmnt
			| do_while_loop_stmnt
			| if_stmnt
			| switch_stmnt
			| var_decl_stmnt
			| assign_stmnt
			| function_call_stmnt
			| return_stmnt
			| CTRL_TRANSFER_STMNT;

expr			: literal
				| binary_expr
				| unary_expr
				| post_unary_expr
				| function_call;
binary_expr		: expr BINARY_OP expr;
unary_expr		: UNARY_OP expr;
post_unary_expr	: expr POST_UNARY_OP;
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

for_loop_stmnt	: 'for' '(' for_init? ';' expr? ';' expr? ')' code_block;
for_init		: assign_stmnt
				| var_decl_stmnt;

while_loop_stmnt	: 'while' '(' expr ')' code_body;
do_while_loop_stmnt	: 'do' code_block 'while' '(' expr ')' ';'

if_stmnt	: 'if' '(' expr ')' code_body else_stmnt?;
else_stmnt	: 'else' code_body;

switch_stmnt	: 'switch' '(' expr ')' '{' case_stmnt_list '}';
case_stmnt_list	: case_stmnt* default_stmnt?;
case_stmnt		: 'case' expr ':' stmnt_list;
default_stmnt	: 'default' ':' stmnt_list;

var_decl_stmnt	: ;
assign_stmnt	: ;

function_call		: IDENT '(' argument_list ')';
function_call_stmnt	: function_call ';';
argument_list		: ( epsilion | expr ( ',' expr )* );

return_stmnt	: 'return' expr? ';';

CTRL_TRANSFER_STMNT	: 'break'
					| 'continue'
					| 'discard';

function_decl	: return_type IDENT '(' parameter_list ')' semantic_modifier? code_block;

var_type		: SCALAR_TYPE
				| VECTOR_TYPE
				| MATRIX_TYPE
				| TEXTURE_TYPE
				| BUFFER_TYPE
				| SAMPLER_STATE_TYPE
				| IDENT;
return_type		: var_type
				| VOID;

semantic_modifier	: ':' IDENT;
initializer			: '=' expr;

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
parameter		: INPUT_MODIFIER? var_type IDENT semantic_modifier? INTERP_MODIFIER? initializer?;


