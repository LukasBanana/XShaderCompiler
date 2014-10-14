
// HLSL (Shader Model 5) Grammar
// 09/10/2014

grammar HLSL;

program : global_decl_list;

global_decl_list	: global_decl*;
global_decl			: function_decl
					| buffer_decl
					| texture_decl
					| samplerstate_decl
					| struct_decl ';'
					| directive_decl;

directive_decl	: '#' to-end-of-line

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
			| return_stmnt
			| struct_decl ';'
			| CTRL_TRANSFER_STMNT;

// Attributes

attribute_list	: attribute*;
attribute		: '[' IDENT ('(' argument_list ')')? ']';

// Variables

array_access		: '[' expr ']';
var_single_ident	: IDENT array_access*;
var_ident			: var_single_ident ('.' var_single_ident)*;

// Expressions

expr			: primary_expr
				| binary_expr

primary_expr	: literal
				| unary_expr
				| post_unary_expr
				| function_call_stmnt
				| bracket_expr
				| cast_expr
				| var_access_expr;

binary_expr		: primary_expr BINARY_OP primary_expr;
unary_expr		: UNARY_OP primary_expr;
post_unary_expr	: primary_expr POST_UNARY_OP;
bracket_expr	: '(' expr ')';
cast_expr		: '(' expr ')' primary_expr;
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

for_loop_stmnt		: attribute_list? 'for' '(' stmnt? ';' expr? ';' expr? ')' stmnt;
while_loop_stmnt	: attribute_list? 'while' '(' expr ')' stmnt;
do_while_loop_stmnt	: attribute_list? 'do' code_block 'while' '(' expr ')';

// Conditional statements

if_stmnt	: attribute_list? 'if' '(' expr ')' stmnt else_stmnt?;
else_stmnt	: 'else' stmnt;

switch_stmnt		: attribute_list? 'switch' '(' expr ')' '{' switch_case_list '}';
switch_case_list	: switch_case* switch_default_case?;
switch_case			: 'case' expr ':' stmnt_list;
switch_default_case	: 'default' ':' stmnt_list;

// Variable declaration

STORAGE_CLASS			: 'extern';
						| 'nointerpolation'
						| 'precise'
						| 'shared'
						| 'groupshared'
						| 'static'
						| 'uniform'
						| 'volatile';

INTERP_MODIFIER			: 'linear'
						| 'centroid'
						| 'nointerpolation'
						| 'noperspective'
						| 'sample';

TYPE_MODIFIER			: 'const'
						| 'row_major'
						| 'column_major';

base_var_type			: SCALAR_TYPE
						| VECTOR_TYPE
						| MATRIX_TYPE;

var_type				: base_var_type
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
semantic				: ':' IDENT;
initializer				: '=' expr;
array_dimension			: '[' expr ']';
var_packoffset			: ':' 'packoffset' '(' REGISTER_NAME ('.' VECTOR_COMPONENT)? ')';
var_register			: ':' 'register' '(' REGISTER_NAME ')';

var_semantic			: semantic
						| var_packoffset
						| var_register;

var_decl				: IDENT array_dimension* var_semantic* initializer?;
var_decl_stmnt			: (STORAGE_CLASS | INTERP_MODIFIER)* TYPE_MODIFIER? var_type var_decl (',' var_decl)*;

// Function calls

function_name		: var_ident
					| VECTOR_TYPE
					| MATRIX_TYPE;
function_call_stmnt	: function_name '(' argument_list ')';
argument_list		: ( epsilion | expr ( ',' expr )* );

// Other statements

assign_stmnt	: var_ident ASSIGN_OP expr;

return_stmnt	: 'return' expr? ';';

CTRL_TRANSFER_STMNT	: 'break'
					| 'continue'
					| 'discard';

// Function declaration

return_type		: var_type
				| VOID;
function_decl	: attribute_list? return_type IDENT '(' parameter_list ')' semantic? code_block;

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
parameter		: INPUT_MODIFIER? var_type IDENT semantic? INTERP_MODIFIER? initializer?;

// Buffer declaration

buffer_decl_ident	: IDENT var_register?;

BUFFER_TYPE		: 'cbuffer'
				| 'tbuffer';

buffer_decl		: BUFFER_TYPE IDENT var_register? var_decl_stmnt_list ';';

// Texture declaration

TEXTURE_IDENT	: 'Buffer'
				| 'RWBuffer'
				| 'StructuredBuffer'
				| 'RWStructuredBuffer'
				| 'ByteAddressBuffer'
				| 'RWByteAddressBuffer'
				| 'AppendStructuredBuffer'
				| 'ConsumeStructuredBuffer';
				| 'Texture1D'
				| 'Texture1DArray'
				| 'Texture2D'
				| 'Texture2DArray'
				| 'Texture2DMS'
				| 'Texture2DMSArray'
				| 'Texture3D'
				| 'TextureCube'
				| 'TextureCubeArray'
				| 'RWTexture1D'
				| 'RWTexture1DArray'
				| 'RWTexture2D'
				| 'RWTexture2DArray'
				| 'RWTexture3D';

texture_decl	: TEXTURE_IDENT ('<' base_var_type '>')? buffer_decl_ident (',' buffer_decl_ident)* ;

// Sampler state declaration

samplerstate_decl	: 'SamplerState' buffer_decl_ident (',' buffer_decl_ident)*;

// Struct declaration

struct_decl		: 'struct' IDENT? var_decl_stmnt_list;
var_decl_stmnt_list	: '{' (var_decl_stmnt ';')* '}';


/*
| buffer_decl
| texture_decl
| samplerstate_decl
*/

