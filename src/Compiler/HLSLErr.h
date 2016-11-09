/*
 * HLSLErr.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_ERR_H
#define XSC_HLSL_ERR_H


#include <string>


namespace Xsc
{


/*
HLSL error code enumeration.
This enumeration is auto-generated code.
see https://msdn.microsoft.com/en-us/library/windows/desktop/dn508414(v=vs.85).aspx
*/
enum class HLSLErr
{
    Unknown                                 = 0,

    // A comment continues past the end of file.
    ERR_COMMENTEOF                          = 1001,

    // A hex value was truncated to 32 bits.
    ERR_HEXTRUNCATED                        = 1002,

    // An octal value was truncated to 32 bits.
    ERR_OCTTRUNCATED                        = 1003,

    // A decimal value was truncated to 32 bits.
    ERR_DECTRUNCATED                        = 1004,

    // A string continues past the end of line.
    ERR_STRINGEOL                           = 1005,

    // A string continues past the end of file.
    ERR_STRINGEOF                           = 1006,

    // A character continues past the end of file.
    ERR_CHAREOF                             = 1007,

    // An error in the token version.
    ERR_TOK_VERSION                         = 1008,

    // An invalid preprocessor syntax.
    ERR_PP_SYNTAX                           = 1500,

    // There were unexpected tokens following the preprocessor directive.
    ERR_UNEXPECTEDTOKENS                    = 1501,

    // The end of file was reached unexpectedly.
    ERR_UNEXPECTEDEOF                       = 1502,

    // A division by zero in the preprocessor expression occurred.
    ERR_DIVZERO                             = 1503,

    // An invalid preprocessor command.
    ERR_INVALIDCOMMAND                      = 1504,

    // The include interface that is required to support #include from resource or memory doesn't work.
    ERR_INCLUDEFROMFILE                     = 1505,

    // There are too many nested #includes.
    ERR_TOOMANYINCLUDES                     = 1506,

    // The specified source file failed to open.
    ERR_FILE_OPEN                           = 1507,

    // An unexpected #elif directive occurred.
    ERR_ELIF                                = 1508,

    // An unexpected #else directive occurred.
    ERR_ELSE                                = 1509,

    // An unexpected #endif directive occurred.
    ERR_ENDIF                               = 1510,

    // A duplicate parameter was supplied to the specified macro.
    ERR_DUPLICATEPARAMATER                  = 1511,

    // A resource failed to open.
    ERR_RESOURCE_OPEN                       = 1512,

    // An unexpected #elif directive followed a #else directive.
    ERR_ELIF_ELSE                           = 1513,

    // An unexpected #else directive followed a #else directive.
    ERR_ELSE_ELSE                           = 1514,

    // An unexpected end of file occurred in a macro expansion.
    ERR_UNEXPECTEDEOF_MACRO                 = 1515,

    // Not enough actual parameters were supplied to the specified macro.
    ERR_PARAMETERS_MACRO                    = 1516,

    // Functional defines in preprocessor expressions are not yet implemented.
    ERR_PP_NOT_YET_IMPLEMENTED              = 1517,

    // An integer constant expression is invalid or unsupported.
    ERR_INVALID_INT_EXPR                    = 1518,

    // The specified macro requires redefining.
    ERR_MACRO_REDEFINITION                  = 1519,

    // The #hlsl_full_path directive must be the first content in a source file.
    ERR_LATE_FULL_PATH                      = 1520,

    // The #hlsl_full_path directive was malformed.
    ERR_INVALID_FULL_PATH                   = 1521,

    // A syntax error was found while parsing a shader file.
    ERR_PARSE_SYNTAX                        = 3000,

    // The specified function requires redefining.
    ERR_REDEFINITION                        = 3003,

    // An undeclared identifier was found while parsing a shader file.
    ERR_UNDECLARED_IDENTIFIER               = 3004,

    // The invalid use of a type was found while parsing a shader file.
    ERR_INVALID_USE                         = 3005,

    // The specified variable can't be declared extern.
    ERR_EXTERN                              = 3006,

    // The specified variable can't be declared static.
    ERR_STATIC                              = 3007,

    // The specified variable can't be declared volatile.
    ERR_VOLATILE                            = 3008,

    // The specified variable can't have initializers.
    ERR_INITIALIZERS                        = 3009,

    // The specified variable can't be declared as group shared or the group-shared variable can't perform a specific task.
    ERR_GROUPSHARED                         = 3010,

    // The specified variable must be a literal expression.
    ERR_NONLITERAL_INITIALIZER              = 3011,

    // The specified variable is missing initializers.
    ERR_MISSING_INITIALIZERS                = 3012,

    // The function doesn't take the specified number of parameters.
    ERR_ARGUMENTS                           = 3013,

    // An incorrect number of arguments was passed to the numeric-type constructor.
    ERR_ARGUMENTS_BASETYPE                  = 3014,

    // An incorrect number of arguments was passed to the intrinsic function.
    ERR_ARGUMENTS_INTRINSIC                 = 3015,

    // The conversion from one type to another type is unsupported.
    ERR_UNSUPPORTED_CAST                    = 3017,

    // The subscript is invalid.
    ERR_SUBSCRIPT                           = 3018,

    // A numeric value, like a float, was expected.
    ERR_NUMERIC_EXPECTED                    = 3019,

    // A type mismatch occurred. For example, this error is returned if all template type components must have the same type, but they don't.
    ERR_TYPE_MISMATCH                       = 3020,

    // An array was expected.
    ERR_PARSE_ARRAY_EXPECTED                = 3021,

    // A scalar, vector, or matrix was expected.
    ERR_BASETYPE_EXPECTED                   = 3022,

    // A determinant error, such as a faulty observation, occurred.
    ERR_DETERMINANT                         = 3023,

    // A vector was expected.
    ERR_VECTOR_EXPECTED                     = 3024,

    // An l-value, which specifies a const object, was expected.
    ERR_LVALUE_EXPECTED                     = 3025,

    // An error in matrix multiplication occurred.
    ERR_MATRIX_MULTIPLICATION               = 3026,

    // An index for an array is out of bounds.
    ERR_PARSE_ARRAY_INDEX_OUT_OF_BOUNDS     = 3030,

    // A imaginary square root error was found while parsing a shader file.
    ERR_PARSE_IMAGINARY_SQUARE_ROOT         = 3031,

    // A indefinite log error was found while parsing a shader file.
    ERR_PARSE_INDEFINITE_LOG                = 3032,

    // A division-by-zero error was found while parsing a shader file.
    ERR_PARSE_DIVISION_BY_ZERO              = 3033,

    // The specified variable can't be declared const.
    ERR_CONST                               = 3035,

    // An error occurred with the redefinition of the specified formal parameter.
    ERR_REDEFINITION_FORMAL_PARAMETER       = 3036,

    // Constructors only defined for numeric base types.
    ERR_UNSUPPORTED_TYPE_EXPR               = 3037,

    // The specified variable must be numeric.
    ERR_NUMERIC                             = 3038,

    // Can't be specific to the target.
    ERR_PARSE_VERSION                       = 3039,

    // Can't have annotations.
    ERR_ANNOTATIONS                         = 3040,

    // The compiler target is unsupported.
    ERR_SHADER_VERSION                      = 3041,

    // A not-yet-implemented error was found while parsing a shader file.
    ERR_PARSE_NOT_YET_IMPLEMENTED           = 3042,

    // Can't have semantics.
    ERR_SEMANTICS                           = 3043,

    // A default value for the specified parameter is missing.
    ERR_MISSING_DEFAULT_PARAMETER           = 3044,

    // Output only and can't be initialized.
    ERR_OUTPUT_INITIALIZER                  = 3045,

    // Output parameters can't be declared const.
    ERR_CONST_OUTPUT                        = 3046,

    // The specified variable can't be declared uniform.
    ERR_UNIFORM                             = 3047,

    // Duplicate usages are specified.
    ERR_USAGE                               = 3048,

    // Can't be specific to the usage.
    ERR_USAGE_VERSION                       = 3049,

    // A matrix was expected.
    ERR_MATRIX_EXPECTED                     = 3050,

    // A scalar was expected.
    ERR_SCALAR_EXPECTED                     = 3051,

    // The vector dimension must be between 1 and MAX_VECTOR_SIZE.
    ERR_VECTOR_SIZE                         = 3052,

    // The matrix dimensions must be between 1 and MAX_VECTOR_SIZE.
    ERR_MATRIX_SIZE                         = 3053,

    // The specified variable can't be declared as shared.
    ERR_SHARED                              = 3054,

    // The specified variable can't be declared as inline.
    ERR_INLINE                              = 3055,

    // The specified variable is a literal expression.
    ERR_LITERAL_VARIABLE                    = 3057,

    // Array dimensions must be literal scalar expressions.
    ERR_ARRAY_LITERAL                       = 3058,

    // Array dimension must be between 1 and 65536.
    ERR_ARRAY_SIZE                          = 3059,

    // The vector dimension must be a literal scalar expression.
    ERR_VECTOR_LITERAL                      = 3060,

    // Matrix dimensions must be a literal scalar expressions.
    ERR_MATRIX_LITERAL                      = 3061,

    // The specified variable can't be declared 'uniform out'.
    ERR_UNIFORM_OUT                         = 3062,

    // The specified variable can't be a sampler.
    ERR_SAMPLER                             = 3063,

    // Object literal expressions aren't allowed inside functions.
    ERR_OBJECT_LITERALS                     = 3064,

    // Object assignments aren't allowed inside functions.
    ERR_OBJECT_ASSIGNMENTS                  = 3065,

    // A sampler was expected.
    ERR_SAMPLER_EXPECTED                    = 3066,

    // The function call is ambiguous.
    ERR_AMBIGUOUS_FUNCTION_CALL             = 3067,

    // The return value of a function differs from the return value of the prototype of the function.
    ERR_PROTOTYPE                           = 3068,

    // The function already has a body.
    ERR_FUNCTION_HAS_BODY                   = 3069,

    // A syntax error was found while parsing an indefinite arccosine.
    ERR_PARSE_INDEFINITE_ACOS               = 3070,

    // A syntax error was found while parsing an indefinite arcsine.
    ERR_PARSE_INDEFINITE_ASIN               = 3071,

    // Array dimensions for this type must be explicit.
    ERR_ARRAY_IMPLICIT                      = 3072,

    // Secondary array dimensions must be explicit.
    ERR_ARRAY_IMPLICIT_ORDER                = 3073,

    // The implicit array is missing a value.
    ERR_ARRAY_IMPLICIT_VALUE                = 3074,

    // The implicit array type does not match.
    ERR_ARRAY_IMPLICIT_SIZE                 = 3075,

    // A void function can't have a semantic attached to it.
    ERR_VOID_SEMANTIC                       = 3076,

    // Non-matrix types can't be declared as row_major or column_major.
    ERR_USAGE_MATRIX                        = 3077,

    // The loop control variable that is used outside the for-loop scope conflicts with a previous declaration in the outer scope; the most recent definition was used.
    ERR_REDEFINITION_LOOP_CONTROL           = 3078,

    // Void functions can't return a value.
    ERR_RETURN_VOID                         = 3079,

    // The function must return a value.
    ERR_RETURN_VALUE                        = 3080,

    // A comma expression was used where an initializer list may have been intended.
    ERR_COMMA_EXPRESSION                    = 3081,

    // An int or unsigned int type is required for bitwise operators.
    ERR_BINARYTYPE_EXPECTED                 = 3082,

    // There are conflicting geometry types.
    ERR_GEOMETRY_CONFLICT                   = 3083,

    // Error with the attribute due to errors with its parameters.
    ERR_ATTRIBUTE                           = 3084,

    // The unsigned type can't be used with this variable type.
    ERR_UNSIGNED_TYPE                       = 3085,

    // The particular syntax (DirectX 9 syntax) or keyword (pixelshader) is deprecated in strict mode.
    ERR_DEPRECATED_IN_STRICT_MODE           = 3086,

    // The object doesn't have methods.
    ERR_NO_METHODS                          = 3087,

    // The object doesn't have the specified method.
    ERR_UNKNOWN_METHOD                      = 3088,

    // The shader target or usage is invalid.
    ERR_TARGETUSAGE_INVALID                 = 3089,

    // No writable textures, samplers, or UAVs can be members of compound types with interface inheritance.
    ERR_NO_OBJECTS_IN_STRUCTS               = 3090,

    // Packoffset is only allowed in a constant buffer.
    ERR_PACK_OFFSET_IN_INVALID_SCOPE        = 3091,

    // Unary negate of unsigned value is still unsigned.
    ERR_UNARY_NEGATE_OF_UNSIGNED            = 3092,

    // Ran out of memory will performing the operation.
    ERR_OUT_OF_MEMORY                       = 3093,

    // The base type is not a structure, class, or interface.
    ERR_NON_COMPOUND_BASE                   = 3094,

    // Multiple concrete base types were specified.
    ERR_MULTI_CONCRETE_BASE                 = 3095,

    // The specified variable isn't a template type.
    ERR_NOT_TEMPLATE_TYPE                   = 3096,

    // The specified static method can't refer to instance members.
    ERR_STATIC_METHOD_MEMBER_USE            = 3097,

    // The method isn't found in the class.
    ERR_NO_METHOD_PROTOTYPE                 = 3098,

    // The specified static method can't be called on objects.
    ERR_STATIC_METHOD_INSTANCE_CALL         = 3099,

    // The specified static member isn't found in the class or isn't a static variable.
    ERR_NO_STATIC_MEMBER_DECL               = 3100,

    // The declaration type differs from the definition type.
    ERR_STATIC_MEMBER_TYPE_MISMATCH         = 3101,

    // Static members can only be defined in global scopes.
    ERR_INVALID_STATIC_MEMBER_SCOPE         = 3102,

    // The specified variable was declared but not defined.
    ERR_MISSING_VARIABLE_DEFINITION         = 3103,

    // Interfaces can't contain data.
    ERR_NO_DATA_IN_INTERFACES               = 3104,

    // Interface methods can't be static.
    ERR_NO_STATIC_METHODS_IN_INTERFACES     = 3105,

    // Interface methods can't be declared outside of an interface.
    ERR_NO_INTERFACE_METHOD_BODIES          = 3106,

    // Interfaces can't inherit from other types.
    ERR_NO_INTERFACE_INHERITANCE            = 3107,

    // The class doesn't implement the specified method.
    ERR_CLASS_MISSING_INTERFACE_METHOD      = 3108,

    // The return type doesn't match the overridden method.
    ERR_MISMATCHED_OVERRIDE_RETTYPE         = 3109,

    // Interfaces can't be members.
    ERR_NO_INTERFACES_AS_MEMBERS            = 3110,

    // Types can't contain members of their own type.
    ERR_RECURSIVE_CONTAINMENT               = 3111,

    // Can't use call or forcecase attributes on switch statements in the specified programs.
    ERR_NO_SWITCH                           = 3112,

    // Default parameters can only be provided in the first prototype.
    ERR_NO_OUT_DEFAULTS                     = 3113,

    // The specified register is used more than once.
    ERR_CONFLICTING_REGISTER_SEMANTICS      = 3115,

    // The API call is invalid.
    ERR_INVALID_API_CALL                    = 3116,

    // The debug info flag can only be set globally.
    ERR_INVALID_COMPILE_EXPR_FLAG           = 3117,

    // Interfaces can only be inputs.
    ERR_INTERFACE_OUT                       = 3118,

    // Interface arrays can't be multi-dimensional.
    ERR_MULTI_DIM_POINTER_ARRAY             = 3119,

    // Invalid type for index was specified. Index must be a scalar or a vector with the correct number of dimensions.
    ERR_INVALID_INDEX                       = 3120,

    // An array, matrix, vector, or indexable object type was expected in the index expression.
    ERR_INDEXABLE_TYPE_EXPECTED             = 3121,

    // The vector element type must be a scalar type.
    ERR_NON_SCALAR_VECTOR_ELEMENT           = 3122,

    // The matrix element type must be a scalar type.
    ERR_NON_SCALAR_MATRIX_ELEMENT           = 3123,

    // The object element type can't be an object type.
    ERR_OBJECT_HAS_OBJECT_ELEMENT           = 3124,

    // The .mips type can only be used in a two-element indexing expression, such as, .mips[mip][element].
    ERR_INVALID_DOT_MIPS_USAGE              = 3125,

    // The specified method didn't match any prototype in the class.
    ERR_METHOD_IMPL_PROTO_MISMATCH          = 3126,

    // The specified method can't be re-declared.
    ERR_METHOD_IMPL_BODY_MISSING            = 3127,

    // Stream parameters can only be single-element types.
    ERR_NON_SIMPLE_STREAM                   = 3128,

    // A warning was treated as an error.
    ERR_WARNING_AS_ERROR                    = 3129,

    // The specified variable can't be declared 'single'.
    ERR_FX_SINGLE                           = 3130,

    // Static interfaces can't have initializers.
    ERR_NO_STATIC_INTERFACE_INIT            = 3131,

    // Interfaces can't be declared in buffers.
    ERR_NO_INTERFACES_IN_BUFFERS            = 3132,

    // Type mismatches aren't recommended.
    WAR_TYPE_MISMATCH                       = 3200,

    // Fragments aren't recommended.
    WAR_NOFRAGMENTS                         = 3201,

    // The semantic doesn't apply and is ignored.
    WAR_INVALID_SEMANTIC                    = 3202,

    // A signed versus unsigned mismatch occurred between destination and value and unsigned is assumed.
    WAR_SIGNED_UNSIGNED_COMPARE             = 3203,

    // Unsigned integer literal is too large so is truncated.
    WAR_INT_TOO_LARGE                       = 3204,

    // In the conversion from larger type to smaller, a loss of data might occur.
    WAR_PRECISION_LOSS                      = 3205,

    // The implicit truncation of a vector type occurred.
    WAR_ELT_TRUNCATION                      = 3206,

    // Initializer was used on a global 'const' variable. This requires setting an external constant. If a literal is wanted, use 'static const' instead.
    WAR_CONST_INITIALIZER                   = 3207,

    // Failed compiling the 10_level_9 (9_x feature levels) vertex shader version of the library function.
    WAR_FAILED_COMPILING_10L9VS             = 3208,

    // Failed compiling the 10_level_9 (9_x feature levels) pixel shader version of the library function.
    WAR_FAILED_COMPILING_10L9PS             = 3209,

    // The particular expressions are not yet implemented.
    ERR_COMP_NOT_YET_IMPLEMENTED            = 3500,

    // The entry-point function is not found.
    ERR_ENTRYPOINT_NOT_FOUND                = 3501,

    // The specified input parameter is missing semantics.
    ERR_MISSING_INPUT_SEMANTICS             = 3502,

    // The specified output parameter or function return value is missing semantics.
    ERR_MISSING_OUTPUT_SEMANTICS            = 3503,

    // The index of the array is out of bounds.
    ERR_COMP_ARRAY_INDEX_OUT_OF_BOUNDS      = 3504,

    // The version being used is no longer supported; instead use a current version.
    ERR_OLD_VERSION                         = 3505,

    // The compiler target isn't recognized.
    ERR_UNRECOGNIZED_VERSION                = 3506,

    // The type can't return a value.
    ERR_RETURN                              = 3507,

    // The output parameter or return value was never assigned a value.
    ERR_OUT_UNINITIALIZED                   = 3508,

    // Texture sample is considered dependent since texcoord wasn't declared as at least a float.
    ERR_DEPENDENT_TEX1D                     = 3509,

    // The function is missing an implementation.
    ERR_FUNCTION_MISSING_BODY               = 3510,

    // The loop is unable to unroll, the loop doesn't appear to terminate in a timely manner (in the specified number of iterations), or the unrolled loop is too large. Use the [unroll(n)] attribute to force an exact higher number.
    ERR_CANT_UNROLL                         = 3511,

    // The index of the sampler array must be a literal expression.
    ERR_ARRAY_INDEX_MUST_BE_LITERAL         = 3512,

    // An array or a particular array dimension was expected.
    ERR_COMP_ARRAY_EXPECTED                 = 3513,

    // The specified input semantic is invalid for geometry shader primitives, it must be its own parameter.
    ERR_GEOMETRY_INVALID                    = 3514,

    // The target is invalid. For example, user-defined buffers can't be target specific, and the register specification expected a particular binding.
    ERR_TARGET_INVALID                      = 3515,

    // Texcube instructions can't have integer offsets.
    ERR_TEXCUBE_OFFSET_INVALID              = 3516,

    // The variable is undefined.
    ERR_UNDEFINED_VARIABLE                  = 3517,

    // A break must be inside a loop.
    ERR_BREAK_OUTSIDE_LOOP                  = 3518,

    // A continue must be inside a loop.
    ERR_CONTINUE_OUTSIDE_LOOP               = 3519,

    // Texture projection can't have texcoord instructions.
    ERR_TEXPROJ_INVALID_TEXCOORD            = 3520,

    // The return type of the texture is too large. It can't exceed four components.
    ERR_TEXTURE_TYPE                        = 3521,

    // Texture objects or streams aren't supported on legacy targets.
    ERR_TEXTURE_OBJECTS_UNSUPPORTED         = 3522,

    // DirectX 9-style intrinsic functions are disabled when not running in DirectX 9 compatibility mode.
    ERR_COMPAT_MAKETEXTURE                  = 3523,

    // Specific attributes can't be used together, like loop and unroll, or a duplicate attribute was supplied.
    ERR_DUPLICATE_ATTRIBUTE                 = 3524,

    // The loop can't be mapped to a shader target because the target doesn't support breaks.
    ERR_NOT_SIMPLE_LOOP                     = 3525,

    // Gradient instructions can't be used in loops with breaks.
    ERR_GRADIENT_WITH_BREAK                 = 3526,

    // Texture access requires literal offset and multisample index.
    ERR_TEXTURE_OFFSET                      = 3527,

    // Flow control (branching) can't be used on this profile.
    ERR_CANT_BRANCH                         = 3528,

    // Flattening with flow control in this specific situation can't be done.
    ERR_MUST_BRANCH                         = 3529,

    // Invalid binding operation was performed. For example, buffers can only be bound to one slot or one constant offset; invalid register specification because a particular binding was expected but didn't occur; can't mix packoffset elements with nonpackoffset elements in a cbuffer.
    ERR_BIND_INVALID                        = 3530,

    // Loops that are marked with the loop attribute can't be unrolled.
    ERR_NEED_UNROLL_FORCED_LOOP             = 3531,

    // A duplicate default or case statement occurred in a switch statement.
    ERR_DUPLICATE_CASE                      = 3532,

    // Non-empty case statements must have a break or return.
    ERR_MUST_HAVE_BREAK                     = 3533,

    // Partial precision isn't supported for the specified target. Min-precision types might offer similar functionality.
    ERR_LOW_PRECISION                       = 3534,

    // An unsupported operation was performed. For example, bitwise operations aren't supported on legacy targets; CheckAccessFullyMapped requires shader model 5 or higher; TextureXxx methods for tiled resources require shader model 5 or higher.
    ERR_UNSUPPORTED_OPERATION               = 3535,

    // SV_ClipDistance semantics can't be used when using the clipplanes attribute, or duplicated input semantics can't change type, size, or layout.
    ERR_INCOMPATIBLE_DUP_SEMANTICS          = 3536,

    // Fall-through cases in switch statements aren't supported. case/default statements that fall through to the next case/default without a break can't have any code in them.
    ERR_NO_FALLTHROUGH                      = 3537,

    // Sampler parameter must come from a literal expression.
    ERR_NON_LITERAL_SAMPLER                 = 3538,

    // A particular shader version, such as, ps_1_x, is no longer supported; use /Gec in the fxc.exe HLSL code compiler to automatically upgrade to the next shader version, such as, ps_2_0; alternately, fxc's /LD option allows use of a previous compiler DLL.
    ERR_OLDVERSION                          = 3539,

    // Global packoffset variables aren't supported.
    ERR_NO_GLOBAL_PACK_OFFSETS              = 3540,

    // Invalid packoffset location was specified.
    ERR_INVALID_PACK_OFFSET_NAME            = 3541,

    // A packoffset variable can't have a target qualifier.
    ERR_PACK_OFFSET_CANT_HAVE_TARGET        = 3542,

    // The operation can't reinterpret the supplied datatype.
    ERR_REINTERPRET_UNSUPPORTED             = 3543,

    // Abstract interfaces aren't supported on the specified target; interface references must resolve to specific instances.
    ERR_NO_INTERFACE_SUPPORT                = 3544,

    // No classes implement the specified method.
    ERR_NO_IFACE_METHOD_IMPLS               = 3545,

    // Reading from texture buffers is unsupported on the specified target.
    ERR_TBUFFER_UNSUPPORTED                 = 3546,

    // Global structs and classes can't be changed.
    ERR_NO_GLOBAL_COMPOUND_WRITES           = 3547,

    // The specified uints can only be used with known-positive values, use int if possible.
    ERR_NO_NEGATIVE_EMULATED_UINTS          = 3548,

    // Interlocked targets must be groupshared or UAV elements. Or, the specified target doesn't support interlocked operations, for example, IncrementCounter/DecrementCounter are only valid on RWStructuredBuffer objects.
    ERR_INTERLOCKED_TARGET                  = 3549,

    // The index of the sampler array must be a literal expression, so the loop is forced to unroll.
    WAR_ARRAY_INDEX_MUST_BE_LITERAL         = 3550,

    // An infinite loop was detected so the loop writes no values.
    WAR_INFINITE_LOOP                       = 3551,

    // The loop can't be mapped to a shader target because the target doesn't support breaks.
    WAR_NOT_SIMPLE_LOOP                     = 3552,

    // Can't use gradient instructions in loops with break.
    WAR_GRADIENT_WITH_BREAK                 = 3553,

    // The attribute is unknown or invalid for the specified statement.
    WAR_UNKNOWN_ATTRIBUTE                   = 3554,

    // Flags aren't compatible with the operation.
    WAR_INCOMPATIBLE_FLAGS                  = 3555,

    // Integer divides might be much slower, try using uints if possible.
    WAR_INT_DIVIDE_SLOW                     = 3556,

    // The loop only executes for a limited number of iterations or doesn't seem to do anything so consider removing it or forcing it to unroll.
    WAR_TOO_SIMPLE_LOOP                     = 3557,

    // The #endif directive is uninitialized.
    WAR_ENDIF_UNINITIALIZED                 = 3558,

    // The loop returns asymmetrically.
    WAR_LOOP_ASYMMETRIC_RETURN              = 3559,

    // If statements that contain out of bounds array accesses can't be flattened.
    WAR_MUST_BRANCH                         = 3560,

    // A particular shader version, such as, ps_1_x, is no longer supported; use the next shader version, such as, ps_2_0.
    WAR_OLDVERSION                          = 3561,

    // The loop simulation goes out of bounds.
    WAR_OUTOFBOUNDS_LOOPSIM                 = 3562,

    // The loop unrolls out of bounds.
    WAR_OUTOFBOUNDS_LOOPUNROLL              = 3563,

    // For better compilation results, consider re-enabling the specified rule.
    WAR_PRAGMA_RULEDISABLE                  = 3564,

    // Loop simulation finished early, use /O1 or higher for potentially better codegen.
    WAR_DID_NOT_SIMULATE                    = 3565,

    // Loop won't exit early, try to make sure the loop condition is as tight as possible.
    WAR_NO_EARLY_BREAK                      = 3566,

    // The register semantic is ignored.
    WAR_IGNORING_REGISTER_SEMANTIC          = 3567,

    // The unknown pragma directive is ignored.
    WAR_UNKNOWN_PRAGMA                      = 3568,

    // The loop executes for more than the maximum number of iterations for the specified shader target, which forces the loop to unroll.
    WAR_LOOP_TOO_LONG                       = 3569,

    // A gradient instruction is used in a loop with varying iteration, which forces the loop to unroll.
    WAR_GRADIENT_MUST_UNROLL                = 3570,

    // The pow(f, e) intrinsic function won't work for negative f, use abs(f) or conditionally handle negative values if you expect them.
    WAR_POW_NOT_KNOWN_TO_BE_POSITIVE        = 3571,

    // Interface references must resolve to non-varying objects.
    WAR_VARYING_INTERFACE                   = 3572,

    // Tessellation factor scale is clamped to the range [0, 1].
    WAR_TESSFACTORSCALE_OUTOFRANGE          = 3573,

    // Thread synchronization operations can't be used in varying flow control.
    WAR_SYNC_IN_VARYING_FLOW                = 3574,

    // Automatic unrolling has been disabled for the loop, consider using the [unroll] attribute or manual unrolling. Or, loop termination conditions in varying flow control so can't depend on data read from a UAV.
    WAR_BREAK_FROM_UAV                      = 3575,

    // Patch semantics must live in the enclosed type so the outer semantic is ignored. Or, semantics in type are overridden by variable/function or enclosing type.
    WAR_OVERRIDDEN_SEMANTIC                 = 3576,

    // The value can't be infinity, A call to isfinite might not be necessary. /Gis might force isfinite to be performed. Or, The value can't be NaN, A call to isnan might not be necessary. /Gis might force isnan to be performed.
    WAR_KNOWN_NON_SPECIAL                   = 3577,

    // The output value isn't completely initialized.
    WAR_TLOUT_UNINITIALIZED                 = 3578,

    // The specified variable doesn't support groupshared so groupshared is ignored.
    WAR_GROUPSHARED_UNSUPPORTED             = 3579,

    // Both sides of the &&, ||, or ?: operator are always evaluated so the side effect on the specified side won't be conditional.
    WAR_CONDITIONAL_SIDE_EFFECT             = 3580,

    // The abs operation on unsigned values is not meaningful so it's ignored.
    WAR_NO_UNSIGNED_ABS                     = 3581,

    // Texture access must have literal offset and multisample index.
    WAR_TEXTURE_OFFSET                      = 3582,

    // A race condition writing to a shared resource was detected, note that threads are writing the same value, but performance might be diminished due to contention.
    WAR_POTENTIAL_RACE_CONDITION_UAV        = 3583,

    // A race condition writing to shared memory was detected, note that threads are writing the same value, but performance might be diminished due to contention.
    WAR_POTENTIAL_RACE_CONDITION_GSM        = 3584,

    // Source_mark is most useful in /Od builds. Without /Od source_mark, can be moved around in the final shader by optimizations.
    WAR_UNRELIABLE_SOURCE_MARK              = 3585,

    // Abstract interfaces aren't supported on the specified target so interface references must resolve to specific instances.
    WAR_NO_INTERFACE_SUPPORT                = 3586,

    // The target emulates A / B with A * reciprocal(B). If the reciprocal of B is not representable in your min-precision type, the result might not be mathematically correct.
    WAR_MIN10_RCP                           = 3587,

    // The clipplanes attribute is ignored in library functions.
    WAR_NO_CLIPPLANES_IN_LIBRARY            = 3588,

    // The '#pragma def' directive is no longer supported on DirectX 10+ and 10_level_9 (9_x feature levels) targets. Use compatibility mode to allow compilation.
    ERR_PRAGMA_DEF_OBSOLETE                 = 3589,

    // Global variables can't use the 'half' type in the specified target. To treat this variable as a float, use the backwards compatibility flag.
    ERR_NO_32_BIT_HALF                      = 3650,

    // The specified target doesn't support double data type values.
    ERR_NO_32_BIT_DOUBLE                    = 3651,

    // The specified target doesn't support 8-bit or 16-bit integers.
    ERR_NO_SMALL_INT                        = 3652,

    // The specified target doesn't support 64-bit integers.
    ERR_NO_64_BIT_INT                       = 3653,

    // The abs operation on unsigned values isn't supported.
    ERR_NO_UNSIGNED_ABS                     = 3654,

    // The thread group size is invalid.
    ERR_THREAD_GROUP_SIZE_INVALID           = 3655,

    // The size of the thread group is missing.
    ERR_THREAD_GROUP_SIZE_MISSING           = 3656,

    // Expected the specified parameter to be a certain value but got the specified value. Or, line or triangle output topologies are only available with isoline domains. Or, the maximum tesselation factor must be in the range [1,64].
    ERR_HSATTRIBUTE_INVALID                 = 3657,

    // Only one InputPatch or OutputPatch parameter is allowed. Or, InputPatch inputs can only be used in hull and geometry (5_0+) shaders. Or, OutputPatch inputs can only be used in the domain shaders and a hull shader's patch constant function.
    ERR_HS_PATCH_INVALID                    = 3658,

    // The patch constant function must use the same input control point type that is declared in the control point phase. Or, the patch constant function must use the same output control point type that is returned from the control point phase. Or, the patch constant function's output patch input should have a certain number of elements, but has the specified amount.
    ERR_HS_TYPE_MISMATCH                    = 3659,

    // The specified target doesn't support interlocked operations.
    ERR_INTERLOCKED_UNSUPPORTED             = 3660,

    // The specified variable doesn't support groupshared.
    ERR_GROUPSHARED_UNSUPPORTED             = 3661,

    // The gradient operation uses a value that might not be defined for all pixels (in the specified target, UAV loads can't participate in gradient operations).
    ERR_INDETERMINATE_DERIVATIVE            = 3662,

    // Thread synchronization operations can't be used in varying flow control.
    ERR_SYNC_IN_VARYING_FLOW                = 3663,

    // The specified target doesn't support synchronization operations.
    ERR_SYNC_UNSUPPORTED                    = 3664,

    // The specified target doesn't support Append/Consume buffers.
    ERR_NO_APPEND_CONSUME                   = 3665,

    // The specified target doesn't support typed UAVs.
    ERR_NO_TYPED_UAVS                       = 3666,

    // The specified target doesn't support UAVs.
    ERR_NO_UAVS                             = 3667,

    // Stores to group shared memory for specified targets must be indexed by an SV_GroupIndex only.
    ERR_INDEX_IS_NOT_GROUP_INDEX            = 3668,

    // Resources being indexed can't come from conditional expressions, they must come from literal expressions.
    ERR_NON_LITERAL_RESOURCE                = 3669,

    // The stream parameter must come from a literal expression.
    ERR_NON_LITERAL_STREAM                  = 3670,

    // Loop termination conditions in varying flow control so can't depend on data read from a UAV.
    ERR_BREAK_FROM_UAV                      = 3671,

    // The specified target doesn't support pull-model attribute evaluation.
    ERR_NO_PULL_MODEL                       = 3672,

    // The specified target doesn't support pull-model evaluation of position.
    ERR_CANT_PULL_POSITION                  = 3673,

    // Attribute evaluation can only be done on values that are taken directly from inputs.
    ERR_PULL_MUST_BE_INPUT                  = 3674,

    // Can't unroll loop with an out-of-bounds array reference in the condition.
    ERR_LOOP_CONDITION_OUT_OF_BOUNDS        = 3675,

    // Typed UAV loads are only supported for single-component 32-bit element types.
    ERR_TYPED_UAV_LOAD_MULTI_COMP           = 3676,

    // The specified target only allows one depth output.
    ERR_MULTIPLE_DEPTH_OUT                  = 3677,

    // Interface-reachable members containing UAVs or group shared variables aren't implemented yet.
    ERR_NO_ORDERED_ACCESS_IN_INTERFACE      = 3678,

    // The storage class globallycoherent can only be used with Unordered Access View (UAV) buffers and can't be used with append/consume buffers.
    ERR_COMP_GLC_INVALID                    = 3679,

    // When you define a pass-through control-point shader, you must declare an InputPatch object, and the number of output control points must be zero or must match the input patch size.
    ERR_HS_UNKNOWN_OUTPUT_TYPE              = 3680,

    // The specified target only supports interlocked operations on scalar int or uint data.
    ERR_ATOMIC_REQUIRES_INT                 = 3681,

    // Groupshared variables can't contain resources such as textures, samplers or UAVs. Or, resources such as textures, samplers or UAVs can't contain other resources.
    ERR_ATTRIBUTE_PARAM_SIDE_EFFECT         = 3682,

    // The specified target doesn't support double-precision floating-point. Or, the operation can't be used directly on resources. Or, the operation can't be used with doubles, cast to float first. Or, the operation isn't supported on the given type.
    ERR_UNSUPPORTED_DOUBLE_OPERATION        = 3684,

    // The tessfactor semantic is out of order. Or, conflicting quad/tri/isoline tessfactor semantic. Or, tessfactor semantics must be in the same component.
    ERR_INVALID_TESS_FACTOR_SEMANTIC        = 3685,

    // The specified object isn't supported.
    ERR_UNSUPPORTED_THIS_OBJECT             = 3686,

    // Double types can't be used as shader inputs or outputs. If you need to pass a double between shader stages, you must pass it as two uints and use asuint and asdouble to convert between forms.
    ERR_INVALID_SHADER_IO                   = 3687,

    // Derivatives of indexed variables aren't implemented yet.
    ERR_INDEXED_DERIV                       = 3688,

    // The left-hand side of an assignment can't be cast to an indexable object so consider using asuint, asfloat, or asdouble on the right-hand side.
    ERR_ORDERED_ACCESS_CAST                 = 3689,

    // The resource being indexed is uninitialized.
    ERR_RESOURCE_UNINITIALIZED              = 3690,

    // Invalid variable reference in static variable initializer. Locals can't be used to initialize static variables.
    ERR_INVALID_STATIC_VAR_INIT             = 3691,

    // The specified target doesn't support aborts.
    ERR_NO_ABORT                            = 3692,

    // The specified target doesn't support messages.
    ERR_NO_MESSAGES                         = 3693,

    // A race condition writing to a shared resource was detected so consider making this operation write conditional.
    ERR_GUARANTEED_RACE_CONDITION_UAV       = 3694,

    // A race condition writing to shared memory was detected so consider making this operation write conditional.
    ERR_GUARANTEED_RACE_CONDITION_GSM       = 3695,

    // An infinite loop was detected so the loop never exits.
    ERR_INFINITE_LOOP                       = 3696,

    // The specified variable matches a variable in the template shader but the type layout doesn't match.
    ERR_TEMPLATE_VAR_CONFLICT               = 3697,

    // The specified resource had binding conflicts with the template shader.
    ERR_RESOURCE_BIND_CONFLICT              = 3698,

    // Place-holder template resources can only be simple resources so structs and arrays aren't supported.
    ERR_COMPLEX_TEMPLATE_RESOURCE           = 3699,

    // For the specified resource, binding isn't present in the template shader.
    ERR_RESOURCE_NOT_IN_TEMPLATE            = 3700,

    // The specified target doesn't support indexing resources.
    ERR_RESINDEX_UNSUPPORTED                = 3701,

    // The fma intrinsic function can only be used with double arguments.
    ERR_FMA_ONLY_DOUBLE                     = 3702,

    // The specified target doesn't support minimum-precision data.
    ERR_NO_MIN_PRECISION                    = 3703,

    // The specified target doesn't support 16-bit float conversions.
    ERR_NO_F32_F16                          = 3704,

    // If statements that contain side effects can't be flattened.
    ERR_NOT_ABLE_TO_FLATTEN                 = 3705,

    // Signed integer division isn't supported on minimum-precision types. Cast to int to use 32-bit division.
    ERR_INVALID_MININT                      = 3706,

    // A minimum 8-bit floating point value is invalid or unsupported.
    ERR_INVALID_MIN8FLOAT                   = 3707,

    // A continue statement can't be used in a switch statement.
    ERR_CONTINUE_INSIDE_SWITCH              = 3708,

    // Debug isn't supported.
    ERR_DEBUG_NOT_SUPPORTED_FOR_MODERN      = 3709,

    // The specified function parameters are unsupported.
    ERR_UNSUPPORTED_PARAM_TYPE              = 3710,

    // Library function parameters and return values can't have duplicate semantic.
    ERR_DUPLICATE_FUNC_PARAM_SEMANTICS      = 3711,

    // Library functions are supported only for pixel shaders and vertex shaders.
    ERR_LIBRARY_FUNC_UNSUPPORTED            = 3712,

    // An entry point can't be specified for a library. Mark library entry points with the export keyword.
    ERR_ENTRYPOINT_MUST_BE_EMPTY            = 3713,

    // The specified variable is declared as static, which isn't supported for libraries yet.
    ERR_NO_STATIC_IN_LIBRARY                = 3714,

    // The specified variable is declared as tbuffer, which is not supported for libraries yet.
    ERR_NO_TBUFFER_IN_LIBRARY               = 3715,

    // Classes and interfaces aren't supported in libraries.
    ERR_NO_INTERFACES_IN_LIBRARY            = 3716,

    // Double data types can't be used as library function inputs or outputs. If you need to pass a double to a library function, you must pass it as two uints and use asuint and asdouble to convert between forms.
    ERR_NO_DOUBLE_IN_LIBRARY                = 3717,

    // Library entry points can't be overloaded.
    ERR_NO_OVERLOADING_FOR_LIB_FUNC         = 3718,

    // The 'resources_may_alias' option is only valid for cs_5_0+ targets.
    ERR_RES_MAY_ALIAS_ONLY_IN_CS_5          = 3719,

    // The specified variable is used without having been completely initialized.
    ERR_READ_BEFORE_WRITE                   = 4000,

    // A division by zero in the mid-level preprocessor expression occurred.
    ERR_MID_DIVISION_BY_ZERO                = 4001,

    // An indefinite logarithm occurred.
    ERR_MID_INDEFINITE_LOG                  = 4002,

    // An imaginary square root occurred.
    ERR_MID_IMAGINARY_SQUARE_ROOT           = 4003,

    // The program is too complex because there are more active values than registers.
    ERR_TOO_COMPLEX                         = 4004,

    // An indefinite arcsine occurred.
    ERR_INDEFINITE_ASIN                     = 4005,

    // An indefinite arccosine occurred.
    ERR_INDEFINITE_ACOS                     = 4006,

    // The array index is out of bounds.
    ERR_ARRAY_INDEX_OUT_OF_BOUNDS           = 4007,

    // A floating point division by zero occurred.
    WARN_FLOAT_DIVISION_BY_ZERO             = 4008,

    // An integer division by zero occurred.
    ERR_IDIV_DIVISION_BY_ZERO               = 4009,

    // An unsigned integer division by zero occurred.
    ERR_UDIV_DIVISION_BY_ZERO               = 4010,

    // The floating-point value out of integer range for a conversion.
    ERR_FTOI_OUTOFRANGE                     = 4011,

    // The floating-point value out of unsigned integer range for a conversion.
    ERR_FTOU_OUTOFRANGE                     = 4012,

    // An indefinite derivative calculation occurred.
    ERR_INDEFINITE_DSXY                     = 4013,

    // Gradient operations can't occur inside loops with divergent flow control.
    ERR_GRADIENT_FLOW                       = 4014,

    // The semantic length is too long.
    ERR_MID_SEMANTIC_TOO_LONG               = 4015,

    // The semantic is invalid. For example, the SV_InstanceID semantic can't be used with 10_level_9 (9_x feature levels) targets, or zero-character semantics aren't supported.
    ERR_INVALID_SEMANTIC                    = 4016,

    // The same variable can't be bound to multiple constants in the same constant bank.
    ERR_MID_INVALID_REGISTER_SEMANTIC       = 4017,

    // The shader uses texture addressing operations in a dependency chain that is too complex for the specific target shader model to handle.
    ERR_TOO_MANY_PHASES                     = 4018,

    // Multiple variables were found with the same user-specified location.
    ERR_CONSTANT_REG_COLLISION              = 4019,

    // Multiple variables were found with the same user-specified location.
    ERR_TBUFFER_REG_COLLISION               = 4020,

    // Derivative is being used before it was defined so consider moving the derivative assignment earlier in the program.
    ERR_DERIV_READ_BEFORE_WRITE             = 4021,

    // Derivative isn't defined in a different branch of flow-control so consider moving the derivative assignment before any flow control statements.
    ERR_DERIV_INVALID_PREDICATE             = 4022,

    // A redefinition of a derivative occurred, and derivatives can only be assigned once.
    ERR_DERIV_REDEFINITION                  = 4023,

    // Derivatives of known values are unimplemented.
    ERR_DERIV_KNOWN_VALUE                   = 4024,

    // Unable to calculate the derivative of the specified value.
    ERR_DERIV_UNKNOWN                       = 4025,

    // A thread sync operation must be in non-varying flow control. Because of a potential race condition, this sync is invalid so consider adding a sync after reading any values that control shader execution at this point.
    ERR_RACE_CONDITION_INDUCED_INV_SYNC     = 4026,

    // The array index is out of bounds.
    ERR_ALIAS_ARRAY_INDEX_OUT_OF_BOUNDS     = 4027,

    // The specified variable has a minimum precision type and can't be marked precise.
    ERR_MINPRECISION_PRECISE                = 4028,

    // An infinite loop was detected so the loop never exits.
    ERR_LOOP_NEVER_BREAKS                   = 4029,

    // The literal floating-point value is out of integer range for the conversion.
    WARN_FTOI_OUTOFRANGE                    = 4114,

    // The literal floating-point value is out of unsigned integer range for the conversion.
    WARN_FTOU_OUTOFRANGE                    = 4115,

    // A possible integer divide by zero occurred.
    WARN_IDIV_DIVISION_BY_ZERO              = 4116,

    // A possible unsigned integer divide by zero occurred.
    WARN_UDIV_DIVISION_BY_ZERO              = 4117,

    // An imaginary square root operation occurred.
    WARN_IMAGINARY_SQUARE_ROOT              = 4118,

    // An indefinite logarithm operation occurred.
    WARN_INDEFINITE_LOG                     = 4119,

    // Optimizations aren't converging.
    WARN_REPLACE_NOT_CONVERGE               = 4120,

    // Gradient-based operations must be moved out of flow control to prevent divergence. Performance might improve by using a non-gradient operation.
    WARN_HOISTING_GRADIENT                  = 4121,

    // The sum of two floating point values can't be represented accurately in double precision.
    WARN_FLOAT_PRECISION_LOSS               = 4122,

    // Floating-point operations flush denorm float literals to zero so the specified floating point value is losing precision (this warning will only be shown once per compile).
    WARN_FLOAT_CLAMP                        = 4123,

    // A feature like clipping from a swizzled vector is not yet implemented.
    ERR_GEN_NOT_YET_IMPLEMENTED             = 4500,

    // An inconsistent semantic definition occurred.
    ERR_DUPLICATE_INPUT_SEMANTIC            = 4501,

    // The specified input semantic is invalid.
    ERR_INVALID_INPUT_SEMANTIC              = 4502,

    // The specified output semantic is invalid.
    ERR_INVALID_OUTPUT_SEMANTIC             = 4503,

    // Overlapping output semantics occurred.
    ERR_DUPLICATE_OUTPUT_SEMANTIC           = 4504,

    // The maximum temp register index was exceeded.
    ERR_MAX_TEMP_EXCEEDED                   = 4505,

    // The maximum number of inputs was exceeded.
    ERR_MAX_INPUT_EXCEEDED                  = 4506,

    // The maximum constant register index was exceeded. Try to reduce the number of constants that are referenced.
    ERR_MAX_CONST_EXCEEDED                  = 4507,

    // The maximum address register index was exceeded.
    ERR_MAX_ADDR_EXCEEDED                   = 4508,

    // An invalid register semantic was used, or a variable must be bound to multiple register banks.
    ERR_GEN_INVALID_REGISTER_SEMANTIC       = 4509,

    // The maximum number of samplers was exceeded.
    ERR_MAX_SAMPLER_EXCEEDED                = 4510,

    // The target doesn't support relative addressing.
    ERR_REL_ADDRESS_NOT_SUP                 = 4511,

    // The texture coordinate w-component can't be accessed.
    ERR_NO_W_ACCESS                         = 4512,

    // Dependent texture read operations that in any way are based on color inputs can't be performed.
    ERR_NO_DEP_FROM_COL                     = 4513,

    // The program is too big.
    ERR_PROGRAM_TOO_BIG                     = 4514,

    // The sampler can't be bound to the user specified stage or sampler array.
    ERR_CANNOT_BIND_SAMPLER                 = 4515,

    // A texcoord that was used as input in a sampler can't be read from.
    ERR_CANNOT_READ_SAME_TEX                = 4516,

    // User defined sampler or sampler array bindings are conflicting. If two samplers have the same user binding, they can't both be used in the same shader.
    ERR_CONFLICT_SAMP_BIND                  = 4517,

    // Texture lookup can't be performed twice from a user bound or similar array access sampler.
    ERR_MULTI_READ_SAMP_BIND                = 4518,

    // Too many texture loads and reads occurred from texcoords.
    ERR_TOO_MANY_TEXREADS                   = 4519,

    // texcoord can be read from and used for texlookup only in ps_1_4 and higher.
    ERR_NO_TEXCRD_SHARE                     = 4520,

    // The program is too complex and is out of temporary registers.
    ERR_OUT_OF_TEMP                         = 4521,

    // Replicate swizzles are only supported in ps_1_4.
    ERR_NO_REP_SWIZZLE                      = 4522,

    // This dependent texture read can't be mapped to ps_1_x, or the shader can't compile to a ps_1_x shader because this model can't match all the dependent texture reads this shader requires.
    ERR_NO_DEP_MATCH                        = 4523,

    // texm can't be matched because computed texcoord is used in shader.
    ERR_TEXM_NO_SHARE                       = 4524,

    // texm* can't be matched because source inputs aren't in the appropriate texture coordinates. For more info, see the ps_1_x assembly reference.
    ERR_TEXM_NOT_COR_STAGE                  = 4525,

    // texm* can't be matched to because texm* can't have source modifiers on input texcoord.
    ERR_TEXM_NO_SOURCE_MOD                  = 4526,

    // texm* can't be matched to because texm* can only have bx2 modifier on input texload.
    ERR_TEXM_BX2_ONLY                       = 4527,

    // DEPTH must be a scalar.
    ERR_DEPTH_SCALAR                        = 4528,

    // The semantic (SV_Target or COLOR) value must be a four-component vector.
    ERR_COLOR_4COMP                         = 4529,

    // The pixel shader must minimally write all four components of the semantic (SV_Target0 or COLOR0) value.
    ERR_WRITE_TO_COLOR0                     = 4530,

    // DP4 isn't supported.
    ERR_DP4_NOT_SUP                         = 4531,

    // The expression can't be mapped to the shader instruction set.
    ERR_NO_MATCH                            = 4532,

    // Swizzle can't be mapped to ps_1_x.
    ERR_NO_SWIZZLE_MATCH                    = 4533,

    // Double dependent texture reads can't be performed in ps_1_x.
    ERR_NO_DOUBLE_DEP                       = 4534,

    // The texreg2ar or texreg2gb instruction can't be matched to because you can't have input modifiers.
    ERR_NO_TEX_SOURCE                       = 4535,

    // The expression can only be mapped to texreg2rgb, but this instruction isn't supported on 1_x.
    ERR_TEXRGB_NOT_SUPPORTED                = 4536,

    // Write masks can't be emulated for the ps_1_x shader model.
    ERR_CANT_EMMULLATE_WRITE                = 4537,

    // SV_Target outputs must be contiguous from SV_Target0 to SV_TargetN, or COLOR outputs must be contiguous from COLOR0 to COLORn.
    ERR_COLOR_CONT                          = 4538,

    // A sampler mismatch occurred because the sampler was used inconsistently.
    ERR_SAMPLER_MISMATCH                    = 4539,

    // PSIZE or FOG must be a scalar.
    ERR_SEMANTIC_SCALER                     = 4540,

    // The vertex shader must minimally write all four components of SV_Position or POSITION.
    ERR_WRITE_ALL_POS                       = 4541,

    // Texcoord outputs must be contiguous from texcoord0 to texcoordn.
    ERR_TEXCOORD_CONT                       = 4542,

    // Multi-register semantics aren't supported in fragments.
    ERR_NO_MULTI_SEM                        = 4543,

    // The clip must be from a 3 vector in ps_1_x.
    ERR_NO_4COMP_CLIP                       = 4544,

    // An unsupported texture type for the specified target was encountered.
    ERR_TEXTURE_NOT_SUPPORTED               = 4545,

    // The maximum sampler register index was exceeded.
    ERR_MAX_SAMP_EXCEEDED                   = 4546,

    // The debug info exceeds the maximum comment size so no debug info was emitted.
    ERR_DEBUG_SIZE                          = 4547,

    // The constant table info exceeds the maximum comment size.
    ERR_CONSTANTTABLE_SIZE                  = 4548,

    // The maximum predicate register index was exceeded.
    ERR_MAX_PRED_EXCEEDED                   = 4549,

    // Try reducing the number of constant branches, take bools out of structs/arrays, or move them to the start of the struct.
    ERR_MAX_BOOL_EXCEEDED                   = 4550,

    // Try reducing the number of loops, take loop counters out of structs/arrays, or move them to the start of the struct.
    ERR_MAX_LOOP_EXCEEDED                   = 4551,

    // The general loop can't be mapped to this instruction set.
    ERR_NOT_SIMPLE_FOR                      = 4552,

    // Relative address references are too deep.
    ERR_ADDRESS_TOO_DEEP                    = 4553,

    // Vector conditionals can't be emulated in ps_1_x shader model.
    ERR_CND_SCALAR                          = 4554,

    // An invalid type used for the specified semantics.
    ERR_INVALID_TYPE                        = 4555,

    // The maximum number of texture slots is exceeded for a library.
    ERR_MAX_TEXTURE_EXCEEDED                = 4565,

    // Offset texture instructions must take an offset, which can resolve to integer literal in the range -8 to 7.
    ERR_REQUIRE_INT_OFFSET                  = 4566,

    // The maximum number of constant buffer slots is exceeded for a library.
    ERR_MAX_CBUFFER_EXCEEDED                = 4567,

    // The usage is unsupported on the target. For example, the sample interpolation, nointerpolation, noperspective, or integer inputs usages might be unsupported.
    ERR_INCORRECT_USAGE                     = 4568,

    // An incorrect type was specified for the POSITION value.
    ERR_POSITION_INCORRECTTYPE              = 4569,

    // The target can only emit to a specific amount of streams.
    ERR_MULTIPLE_STREAMS                    = 4570,

    // The output limit was exceeded.
    ERR_MAX_OUTPUT_EXCEEDED                 = 4571,

    // The geometry shader didn't emit anything.
    ERR_NO_STREAMS_USED                     = 4572,

    // The semantic length is too long and is limited to the specified number of characters.
    ERR_GEN_SEMANTIC_TOO_LONG               = 4573,

    // A duplicate system value semantic definition was encountered.
    ERR_DUPLICATE_SYSVAL_SEMANTIC           = 4574,

    // An uninitialized value was read.
    ERR_READING_UNINITIALIZED               = 4575,

    // An error occurred during signature validation.
    ERR_SIGNATURE_VALIDATION                = 4576,

    // Not all elements of SV_Position were written.
    ERR_INCOMPLETE_POSITION                 = 4577,

    // The specified cbuffer register was used more than once.
    ERR_DUPLICATE_CBUFFER_BANK              = 4578,

    // An invalid floating point literal occurred.
    ERR_INVALID_FP_LITERAL                  = 4579,

    // The specified output contains a system-interpreted value that must be written in every execution path of the shader. Unconditional initialization might help.
    ERR_UNWRITTEN_SI_VALUE                  = 4580,

    // Using sampler arrays with texture objects on 10_level_9 (9_x feature level) targets isn't implemented yet.
    ERR_AUTOSAMPLER_ARRAY_UNIMPL            = 4581,

    // Sampling from non-floating point texture formats can't be done.
    ERR_INVALID_TEXTURE_FORMAT              = 4582,

    // The specified semantic isn't supported on the 10_level_9 (9_x feature level) target.
    ERR_INVALID_10L9_SEMANTIC               = 4583,

    // The maximum number of interface pointers was exceeded.
    ERR_MAX_IFACE_EXCEEDED                  = 4584,

    // The maximum number of UAV slots was exceeded for a library.
    ERR_MAX_UAV_EXCEEDED                    = 4585,

    // The total amount, in bytes, of group shared memory exceeded the target's limit.
    ERR_MAX_GROUP_SHARED_MEMORY_EXCEEDED    = 4586,

    // Shaders compiled for the specified target can only have a single group shared data item.
    ERR_TOO_MANY_GROUP_SHARED_DATA          = 4587,

    // Group shared data for the specified target must have a count of elements that is equal to the number of threads in the thread group.
    ERR_INCORRECT_NUM_GROUP_SHARED_ELEMENTS = 4588,

    // Group shared data for the specified target is too large and must have an element size of at most the specified amount of bytes when compiling for the specified number of threads.
    ERR_CONTROL_POINT_COUNT_EXCEEDED        = 4589,

    // Group shared data for the specified target must be an array of elements.
    ERR_GROUP_SHARED_DATA_NOT_AN_ARRAY      = 4591,

    // When multiple geometry shader output streams are used they must be point lists.
    ERR_MULTI_SO_NOT_POINT                  = 4592,

    // The target's snap offset must be in the range -8 to 7.
    ERR_INVALID_SNAP_OFFSET                 = 4593,

    // Clip planes can't be addressed in the specified target; or, clip planes must be non-literal constants with identity swizzles in the specified target.
    ERR_CLIPPLANE_TOO_COMPLICATED           = 4594,

    // RWStructuredBuffer objects can increment or decrement their counters, but not both.
    ERR_ONLY_ONE_ALLOC_CONSUME              = 4595,

    // Typed UAV stores must write all declared components.
    ERR_TYPED_UAV_WRITE_MASK_MISMATCH       = 4596,

    // Texture1D types are unsupported on the specified target.
    ERR_TEX1D_UNSUPPORTED                   = 4597,

    // The array element count of GetDimensions on TextureCubeArray objects is unavailable on the specified target.
    ERR_RESINFO_Z_UNDEFINED_CUBEARRAY       = 4598,

    // The structured buffer element size is invalid. It must be a multiple of specified bytes in the specified target, or it can't be larger than the specified bytes in in the specified target.
    ERR_INVALID_STRUCTURED_ELEMENT_SIZE     = 4599,

    // The shader's indexable literal values were exceeded. The shader uses too many indexable literal values so consider using less constant arrays.
    ERR_MAX_ICB_REG_EXCEEDED                = 4600,

    // The size of the specified constant buffer is the specified number 16-byte entries, which exceeds maximum allowed size of entries.
    ERR_MAX_CBUFFER_SIZE_EXCEEDED           = 4601,

    // Debug instructions are unsupported in shader libraries.
    ERR_LIB_DEBUG_INST_UNSUPPORTED          = 4602,

    // Interface calls can't be indexed with varying values.
    ERR_VARYING_INDEXED_INTERFACE           = 4603,

    // A feature isn't implemented yet.
    WAR_GEN_NOT_YET_IMPLEMENTED             = 4700,

    // A _bias opportunity was missed because the source wasn't clamped 0 to 1.
    WAR_BIAS_MISSED                         = 4701,

    // A complement opportunity was missed because the input result was clamped from 0 to 1.
    WAR_COMP_MISSED                         = 4702,

    // Lerp can't be matched because the lerp factor is not _sat'd.
    WAR_LRP_MISSED                          = 4703,

    // Literal values outside range -1 to 1 are clamped on all ps_1_x shading models.
    WAR_MAX_CONST_RANGE                     = 4704,

    // The specified input semantic has been deprecated; use the specified semantic instead.
    WAR_DEPRECATED_INPUT_SEMANTIC           = 4705,

    // The specified output semantic has been deprecated; use the specified semantic instead.
    WAR_DEPRECATED_OUTPUT_SEMANTIC          = 4706,

    // The texcoord inputs used directly (that is, other than sampling from textures) in shader body in ps_1_x are always clamped from 0 to 1.
    WAR_TEXCOORD_CLAMP                      = 4707,

    // The mid-level var was not found.
    WAR_MIDLEVEL_VARNOTFOUND                = 4708,

    // The semantic is no longer in use.
    WAR_OLD_SEMANTIC                        = 4710,

    // A duplicate non-system value semantic definition was encountered.
    WAR_DUPLICATE_SEMANTIC                  = 4711,

    // The loop can't be matched because the loop count isn't from an integer type.
    WAR_CANT_MATCH_LOOP                     = 4712,

    // The sample bias value is limited to the range [-16.00, 15.99] so use the specified value instead of this value.
    WAR_BIAS_CLAMPED                        = 4713,

    // The sum of temp registers and indexable temp registers times the specified number of threads exceeds the recommended total number of threads so performance might be reduced.
    WAR_CS_TEMP_EXCEEDED                    = 4714,

    // A system-interpreted value is emitted that can't be written in every execution path of the shader.
    WAR_UNWRITTEN_SI_VALUE                  = 4715,

    // The specified semantic has no special meaning on 10_level_9 (9_x feature levels) targets.
    WAR_PSIZE_HAS_NO_SPECIAL_MEANING        = 4716,

    // Effects are deprecated for the D3DCompiler_47.dll or later.
    WAR_DEPRECATED_FEATURE                  = 4717,
};


// Converts the specified error code to a string.
std::string ErrToString(const HLSLErr errorCode);


} // /namespace Xsc


#endif



// ================================================================================