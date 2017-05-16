/*
 * ReportIdentsEN.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_IDENTS_EN_H
#define XSC_REPORT_IDENTS_EN_H


/* ----- Common ----- */

DECL_REPORT( Message,                           "message"                                                                                                       );
DECL_REPORT( Warning,                           "warning"                                                                                                       );
DECL_REPORT( Error,                             "error"                                                                                                         );
DECL_REPORT( CodeGenerationError,               "code generation error"                                                                                         );
DECL_REPORT( CodeReflection,                    "code reflection"                                                                                               );
DECL_REPORT( SyntaxError,                       "syntax error"                                                                                                  );
DECL_REPORT( ContextError,                      "context error"                                                                                                 );
DECL_REPORT( InternalError,                     "internal error"                                                                                                );
DECL_REPORT( In,                                "in"                                                                                                            ); // e.g. "error in 'context'"
DECL_REPORT( Input,                             "input"                                                                                                         ); // e.g. "entry point input"
DECL_REPORT( Output,                            "output"                                                                                                        ); // e.g. "entry point output"
DECL_REPORT( Anonymous,                         "anonymous"                                                                                                     );
DECL_REPORT( Unspecified,                       "unspecified"                                                                                                   );
DECL_REPORT( CandidatesAre,                     "candidates are"                                                                                                );
DECL_REPORT( StackUnderflow,                    "stack underflow[ in {0}]"                                                                                      );
DECL_REPORT( VertexShader,                      "vertex shader"                                                                                                 );
DECL_REPORT( TessControlShader,                 "tessellation-control shader"                                                                                   );
DECL_REPORT( TessEvaluationShader,              "tessellation-evaluation shader"                                                                                );
DECL_REPORT( GeometryShader,                    "geometry shader"                                                                                               );
DECL_REPORT( FragmentShader,                    "fragment shader"                                                                                               );
DECL_REPORT( ComputeShader,                     "compute shader"                                                                                                );
DECL_REPORT( InvalidOutputStream,               "invalid output stream"                                                                                         );
DECL_REPORT( Implicitly,                        "implicitly"                                                                                                    );
DECL_REPORT( ButGot,                            "[, but got {0}]"                                                                                               );
DECL_REPORT( NotImplementedYet,                 "[{0} ]not implemented yet[ (in '{1}')]"                                                                        );
DECL_REPORT( DeclaredAt,                        "declared at ({0})"                                                                                             );
DECL_REPORT( PrevDefinitionAt,                  "previous definition at ({0})"                                                                                  );
DECL_REPORT( ExceptionThrown,                   "exception thrown: "                                                                                            );
DECL_REPORT( ZeroBased,                         "zero-based"                                                                                                    );
DECL_REPORT( OneBased,                          "one-based"                                                                                                     );

/* ----- Token ----- */

DECL_REPORT( Identifier,                        "identifier"                                                                                                    );
DECL_REPORT( BoolLiteral,                       "boolean literal"                                                                                               );
DECL_REPORT( IntLiteral,                        "integer literal"                                                                                               );
DECL_REPORT( FloatLiteral,                      "floating-point literal"                                                                                        );
DECL_REPORT( StringLiteral,                     "string literal"                                                                                                );
DECL_REPORT( CharLiteral,                       "character literal"                                                                                             );
DECL_REPORT( NullLiteral,                       "null literal"                                                                                                  );
DECL_REPORT( AssignOp,                          "assign operator[ '{0}']"                                                                                       );
DECL_REPORT( BinaryOp,                          "binary operator[ '{0}']"                                                                                       );
DECL_REPORT( UnaryOp,                           "unary operator[ '{0}']"                                                                                        );
DECL_REPORT( TernaryOp,                         "ternary operator"                                                                                              );
DECL_REPORT( StringTypeDen,                     "string type denoter"                                                                                           );
DECL_REPORT( ScalarTypeDen,                     "scalar type denoter"                                                                                           );
DECL_REPORT( VectorTypeDen,                     "vector type denoter"                                                                                           );
DECL_REPORT( MatrixTypeDen,                     "matrix type denoter"                                                                                           );
DECL_REPORT( VoidTypeDen,                       "void type denoter"                                                                                             );
DECL_REPORT( PrimitiveTypeDen,                  "primitive type denoter"                                                                                        );
DECL_REPORT( ReservedWord,                      "reserved keyword"                                                                                              );
DECL_REPORT( VectorGenericTypeDen,              "vector generic type denoter"                                                                                   );
DECL_REPORT( MatrixGenericTypeDen,              "matrix generic type denoter"                                                                                   );
DECL_REPORT( SamplerTypeDen,                    "sampler type denoter"                                                                                          );
DECL_REPORT( SamplerState,                      "sampler state"                                                                                                 );
DECL_REPORT( BufferTypeDen,                     "buffer type denoter"                                                                                           );
DECL_REPORT( UniformBufferTypeDen,              "uniform buffer type denoter"                                                                                   );
DECL_REPORT( KeywordDo,                         "'do' keyword"                                                                                                  );
DECL_REPORT( KeywordWhile,                      "'while' keyword"                                                                                               );
DECL_REPORT( KeywordFor,                        "'for' keyword"                                                                                                 );
DECL_REPORT( KeywordIf,                         "'if' keyword"                                                                                                  );
DECL_REPORT( KeywordElse,                       "'else' keyword"                                                                                                );
DECL_REPORT( KeywordSwitch,                     "'switch' keyword"                                                                                              );
DECL_REPORT( KeywordCase,                       "'case' keyword"                                                                                                );
DECL_REPORT( KeywordDefault,                    "'default' keyword"                                                                                             );
DECL_REPORT( KeywordTypedef,                    "'typedef' keyword"                                                                                             );
DECL_REPORT( KeywordStruct,                     "'struct' keyword"                                                                                              );
DECL_REPORT( KeywordRegister,                   "'register' keyword"                                                                                            );
DECL_REPORT( KeywordPackOffset,                 "'packoffset' keyword"                                                                                          );
DECL_REPORT( KeywordReturn,                     "'return' keyword"                                                                                              );
DECL_REPORT( KeywordInline,                     "'inline' keyword"                                                                                              );
DECL_REPORT( KeywordTechnique,                  "'technique' keyword"                                                                                           );
DECL_REPORT( KeywordPass,                       "'pass' keyword"                                                                                                );
DECL_REPORT( KeywordCompile,                    "'compile' keyword"                                                                                             );
DECL_REPORT( CtrlTransfer,                      "control transfer"                                                                                              );
DECL_REPORT( InputModifier,                     "input modifier"                                                                                                );
DECL_REPORT( InterpModifier,                    "interpolation modifier"                                                                                        );
DECL_REPORT( TypeModifier,                      "type modifier"                                                                                                 );
DECL_REPORT( StorageClass,                      "storage class"                                                                                                 );
DECL_REPORT( Comment,                           "comment"                                                                                                       );
DECL_REPORT( WhiteSpaces,                       "white spaces"                                                                                                  );
DECL_REPORT( NewLineChars,                      "new-line characters"                                                                                           );
DECL_REPORT( VarArgSpecifier,                   "variadic argument specifier"                                                                                   );
DECL_REPORT( Misc,                              "miscellaneous"                                                                                                 );
DECL_REPORT( PPDirective,                       "preprocessor directive"                                                                                        );
DECL_REPORT( PPDirectiveConcat,                 "preprocessor directive concatenation"                                                                          );
DECL_REPORT( PPLineBreak,                       "preprocessor line break"                                                                                       );
DECL_REPORT( EndOfStream,                       "end-of-stream"                                                                                                 );

/* ----- AST ----- */

DECL_REPORT( IllegalTypeOfFuncObj,              "illegal type denoter of function object '{0}'"                                                                 );
DECL_REPORT( CantDirectlyAccessMembersOf,       "can not directly access members of '{0}'"                                                                      );
DECL_REPORT( CantDirectlyAccessArrayOf,         "can not directly access array of '{0}'"                                                                        );
DECL_REPORT( UnknownTypeOfObjectIdentSymbolRef, "unknown type of symbol reference to derive type denoter of object identifier '{0}'"                            );
DECL_REPORT( InvalidSubscriptBaseType,          "invalid base type denoter for vector subscript"                                                                );
DECL_REPORT( IllegalStaticAccessForSubscript,   "illegal static access for subscript '{0}'"                                                                     );
DECL_REPORT( MissingVarIdentSymbolRef,          "missing symbol reference to derive type denoter of variable identifier '{0}'"                                  );
DECL_REPORT( MissingFuncRefToDeriveExprType,    "missing function reference to derive expression type"                                                          );
DECL_REPORT( MissingDeclStmntRefToDeriveType,   "missing reference to declaration statement to derive type denoter of variable identifier '{0}'"                );
DECL_REPORT( MissingObjectExprSymbolRef,        "missing symbol reference in object expression[ '{0}']"                                                         );
DECL_REPORT( FuncDoesntTake1Param,              "function '{0}' does not take {1} parameter"                                                                    );
DECL_REPORT( FuncDoesntTakeNParams,             "function '{0}' does not take {1} parameters"                                                                   );
DECL_REPORT( TernaryExpr,                       "ternary expression"                                                                                            );
DECL_REPORT( BinaryExpr,                        "binary expression '{0}'"                                                                                       );
DECL_REPORT( OnlyBaseTypeAllowed,               "only scalar, vector, and matrix type allowed[ in {0}][, but got '{1}']"                                        );
DECL_REPORT( CastExpr,                          "cast expression"                                                                                               );
DECL_REPORT( CantDeriveTypeOfInitializer,       "can not derive type of initializer list without type parameter"                                                );
DECL_REPORT( CantDeriveTypeOfEmptyInitializer,  "can not derive type of initializer list with no elements"                                                      );
DECL_REPORT( ConditionOfTernaryExpr,            "condition of ternary expression"                                                                               );
DECL_REPORT( ExpectedInitializerForArrayAccess, "initializer expression expected for array access"                                                              );
DECL_REPORT( InvalidNumElementsInInitializer,   "invalid number of elements in initializer expression for type '{0}' (expected {1}, but got {2})"               );
DECL_REPORT( NotEnoughElementsInInitializer,    "not enough elements in initializer expression"                                                                 );
DECL_REPORT( NotEnoughIndicesForInitializer,    "not enough array indices specified for initializer expression"                                                 );
DECL_REPORT( ArrayIndexOutOfBounds,             "array index out of bounds[: {0} is not in range \\[0, {1})]"                                                   );

/* ----- ASTEnums ----- */

DECL_REPORT( DataType,                          "data type"                                                                                                     );
DECL_REPORT( SamplerType,                       "sampler type"                                                                                                  );
DECL_REPORT( BufferType,                        "buffer type"                                                                                                   );
DECL_REPORT( Intrinsic,                         "intrinsic[ '{0}']"                                                                                             );
DECL_REPORT( DomainType,                        "domain type"                                                                                                   );
DECL_REPORT( PrimitiveType,                     "primitive type"                                                                                                );
DECL_REPORT( Partitioning,                      "partitioning"                                                                                                  );
DECL_REPORT( OutputToplogy,                     "output toplogy"                                                                                                );
DECL_REPORT( Attributes,                        "attributes"                                                                                                    );
DECL_REPORT( Undefined,                         "<undefined>"                                                                                                   );
DECL_REPORT( UserDefined,                       "<user-defined>"                                                                                                );
DECL_REPORT( FailedToMap,                       "failed to map {0} to {1}"                                                                                      );
DECL_REPORT( VectorSubscriptCantHaveNComps,     "vector subscript can not have {0} components"                                                                  );
DECL_REPORT( IncompleteMatrixSubscript,         "incomplete matrix subscript: '{0}'"                                                                            );
DECL_REPORT( InvalidVectorDimension,            "invalid vector dimension (must be in the range \\[1, 4\\], but got {0})"                                       );
DECL_REPORT( InvalidVectorSubscript,            "invalid vector subscript '{0}' for {1}"                                                                        );
DECL_REPORT( InvalidMatrixDimension,            "invalid matrix dimension (must be in the range \\[1, 4\\] x \\[1, 4\\], but got {0} x {1})"                    );
DECL_REPORT( InvalidMatrixSubscriptMixture,     "invalid mixture of zero-based and one-based matrix subscripts[: '{0}']"                                        );
DECL_REPORT( InvalidCharInMatrixSubscript,      "invalid character '{0}' in [{2} ]matrix subscript: '{1}'"                                                      );
DECL_REPORT( InvalidIntrinsicArgType,           "invalid argument type denoter for intrinsic[ '{0}']"                                                           );
DECL_REPORT( InvalidIntrinsicArgCount,          "invalid number of arguments for intrinsic[ '{0}'][ (expected {1}, but got {2})]"                               );
DECL_REPORT( InvalidIntrinsicArgs,              "invalid arguments for intrinsic[ '{0}']"                                                                       );

/* ----- TypeDenoter ------ */

DECL_REPORT( VarIdentCantBeResolved,            "variable identifier can not be resolved"                                                                       );
DECL_REPORT( TypeHasNoSuchObject,               "type '{0}' has no object named '{1}'"                                                                          );
DECL_REPORT( IllegalArrayAccess,                "array access not allowed[ for '{0}']"                                                                          );
DECL_REPORT( TooManyArrayDimensions,            "too many array dimensions[ for '{0}']"                                                                         );
DECL_REPORT( MissingRefToStructDecl,            "missing reference to structure declaration[ '{0}']"                                                            );
DECL_REPORT( MissingRefToAliasDecl,             "missing reference to alias declaration[ '{0}']"                                                                );
DECL_REPORT( MissingBaseTypeInArray,            "missing base type in array type denoter"                                                                       );
DECL_REPORT( MissingRefInTypeDen,               "missing reference to declaration[ in {0}]"                                                                     );
DECL_REPORT( InvalidExprForSubTypeDen,          "invalid expression to derive sub type denoter[ for '{0}']"                                                     );

/* ----- SymbolTable ----- */

DECL_REPORT( UndefinedSymbol,                   "undefined symbol '{0}'"                                                                                        );
DECL_REPORT( AmbiguousSymbol,                   "symbol '{0}' is ambiguous"                                                                                     );
DECL_REPORT( AmbiguousFuncCall,                 "ambiguous function call '{0}({1})'"                                                                            );
DECL_REPORT( AmbiguousIntrinsicCall,            "ambiguous intrinsic call[ '{0}']"                                                                              );
DECL_REPORT( IdentIsNotFunc,                    "identifier '{0}' does not name a function"                                                                     );
DECL_REPORT( IdentIsNotVar,                     "identifier '{0}' does not name a variable"                                                                     );
DECL_REPORT( IdentIsNotType,                    "identifier '{0}' does not name a type"                                                                         );
DECL_REPORT( IdentIsNotVarOrBufferOrSampler,    "identifier '{0}' does not name a variable, buffer, or sampler"                                                 );
DECL_REPORT( IdentIsNotDecl,                    "identifier '{0}' does not name a variable, buffer, sampler, structure, or alias"                               );
DECL_REPORT( IdentIsNotBaseOf,                  "identifier '{0}' does not name a base of '{1}'"                                                                );
DECL_REPORT( IdentAlreadyDeclared,              "identifier '{0}' already declared in this scope"                                                               );
DECL_REPORT( NoActiveScopeToRegisterSymbol,     "no active scope to register symbol"                                                                            );

/* ----- Scanner ----- */

DECL_REPORT( LexicalError,                      "lexical error"                                                                                                 );
DECL_REPORT( UnexpectedChar,                    "unexpected character '{0}'[ (expected '{1}')]"                                                                 );
DECL_REPORT( MissingDigitSequenceAfterExpr,     "missing digit-sequence after exponent part"                                                                    );
DECL_REPORT( MissingDecimalPartInFloat,         "missing decimal part in floating-point number"                                                                 );

/* ----- Parser ----- */

DECL_REPORT( UnexpectedToken,                   "unexpected token[: {0}[ ({1})]]"                                                                               );
DECL_REPORT( UnexpectedEndOfStream,             "unexpected end-of-stream"                                                                                      );
DECL_REPORT( UnexpectedTokenSpell,              "unexpected token spelling '{0}'[ (expected '{1}')]"                                                            );
DECL_REPORT( Expected,                          "expected {0}"                                                                                                  );
DECL_REPORT( ExpectedPrimaryExpr,               "expected primary expression"                                                                                   );
DECL_REPORT( ExpectedLiteralExpr,               "expected literal expression"                                                                                   );
DECL_REPORT( ExpectedTypeDen,                   "expected type denoter"                                                                                         );
DECL_REPORT( ExpectedBaseTypeDen,               "expected base type denoter[, but got '{0}']"                                                                   );
DECL_REPORT( ExpectedIntLiteral,                "expected integer literal[, but got '{0}']"                                                                     );
DECL_REPORT( InFunction,                        " (in function: {0})"                                                                                           );
DECL_REPORT( FailedToCreateScanner,             "failed to create token scanner"                                                                                );
DECL_REPORT( FailedToScanSource,                "failed to scan source code"                                                                                    );
DECL_REPORT( MissingScanner,                    "missing token scanner"                                                                                         );
DECL_REPORT( SubExprMustNotBeEmpty,             "sub-expressions must not be empty"                                                                             );
DECL_REPORT( SubExprAndOpsUncorrelated,         "sub-expressions and operators have uncorrelated number of elements"                                            );
DECL_REPORT( TooManySyntaxErrors,               "too many syntax errors"                                                                                        );
DECL_REPORT( IdentNameManglingConflict,         "identifier '{0}' conflicts with reserved name mangling prefix '{1}'"                                           );
DECL_REPORT( NotAllowedInThisContext,           "{0} not allowed in this context"                                                                               );
DECL_REPORT( IntLiteralOutOfRange,              "integer literal[ '{0}'] is out of range"                                                                       );

/* ----- PreProcessor ----- */

DECL_REPORT( UnknownPPDirective,                "unknown preprocessor directive: \"{0}\""                                                                       );
DECL_REPORT( UnknownMatrixPackAlignment,        "unknown matrix pack alignment: \"{0}\" (must be \"row_major\" or \"column_major\")"                            );
DECL_REPORT( UnknownPragma,                     "unknown pragma: \"{0}\""                                                                                       );
DECL_REPORT( InvalidMacroIdentTokenArg,         "invalid argument for macro identifier token"                                                                   );
DECL_REPORT( FailedToUndefMacro,                "failed to undefine macro \"{0}\""                                                                              );
DECL_REPORT( MacroRedef,                        "redefinition of macro \"{0}\"[ {1}]"                                                                           );
DECL_REPORT( WithMismatchInParamListAndBody,    "with mismatch in parameter list and body definition"                                                           );
DECL_REPORT( WithMismatchInParamList,           "with mismatch in parameter list"                                                                               );
DECL_REPORT( WithMismatchInBody,                "with mismatch in body definition"                                                                              );
DECL_REPORT( MissingIfDirective,                "missing '#if'-directive to closing '#endif', '#else', or '#elif'"                                              );
DECL_REPORT( MissingEndIfDirective,             "missing '#endif'-directive for open '#if', '#ifdef', or '#ifndef'"                                             );
DECL_REPORT( TooManyArgsForMacro,               "too many arguments for macro \"{0}\"[ (expected {1}, but got {2})]"                                            );
DECL_REPORT( TooFewArgsForMacro,                "too few arguments for macro \"{0}\"[ (expected {1}, but got {2})]"                                             );
DECL_REPORT( ExpectedEndIfDirective,            "expected '#endif'-directive after previous '#else'[, but got '{0}']"                                           );
DECL_REPORT( PragmaCantBeHandled,               "pragma \"{0}\" can currently not be handled"                                                                   );
DECL_REPORT( UnexpectedTokenInPragma,           "unexpected token in '#pragam'-directive"                                                                       );
DECL_REPORT( UnexpectedEndOfTokenString,        "unexpected end of token string"                                                                                );
DECL_REPORT( RemainingTokensInPragma,           "remaining unhandled tokens in '#pragma'-directive"                                                             );
DECL_REPORT( EmptyPragma,                       "empty '#pragma'-directive"                                                                                     );

/* ----- VisitorTracker ----- */

DECL_REPORT( FuncDeclStackUnderflow,            "function declaration stack underflow"                                                                          ); // internal error
DECL_REPORT( CallExprStackUnderflow,            "call expression stack underflow"                                                                               ); // internal error
DECL_REPORT( LValueExprStackUnderflow,          "l-value expression stack underflow"                                                                            ); // internal error
DECL_REPORT( StructDeclStackUnderflow,          "structure declaration stack underflow"                                                                         ); // internal error
DECL_REPORT( UniformBufferDeclStackUnderflow,   "uniform buffer declaration stack underflow"                                                                    ); // internal error
DECL_REPORT( WritePrefixStackUnderflow,         "write prefix stack underflow"                                                                                  ); // internal error

/* ----- Analyzer ----- */

DECL_REPORT( UndeclaredIdent,                   "undeclared identifier \"{0}\"[ in '{1}'][; did you mean \"{2}\"?]"                                             );
DECL_REPORT( StatementWithEmptyBody,            "<{0}> statement with empty body"                                                                               );
DECL_REPORT( MissingReferenceToStructInType,    "missing reference to structure declaration in type denoter '{0}'"                                              );
DECL_REPORT( MissingVariableType,               "missing variable type"                                                                                         );
DECL_REPORT( ParameterCantBeUniformAndOut,      "type attributes 'out' and 'inout' can not be used together with 'uniform' for a parameter"                     );
DECL_REPORT( IllegalCast,                       "can not cast '{0}' to '{1}'[ in {2}]"                                                                          );
DECL_REPORT( NullPointerArgument,               "null pointer passed to {0}"                                                                                    );
DECL_REPORT( ConditionalExprNotScalar,          "conditional expression must evaluate to scalar, but got '{0}'"                                                 );
DECL_REPORT( ExpectedConstExpr,                 "expected constant expression"                                                                                  );
DECL_REPORT( ExpectedConstIntExpr,              "expected constant integer expression"                                                                          );
DECL_REPORT( ExpectedConstFloatExpr,            "expected constant floating-point expression"                                                                   );
DECL_REPORT( VarDeclaredButNeverUsed,           "variable '{0}' is declared but never used"                                                                     );
DECL_REPORT( ImplicitVectorTruncation,          "implicit truncation of vector type[ (from {0} to {1} dimensions)]"                                             );

/* ----- ExprEvaluator ----- */

DECL_REPORT( ExprEvaluator,                     "expression evaluator"                                                                                          );
DECL_REPORT( IllegalExprInConstExpr,            "illegal {0} in constant expression"                                                                            );
DECL_REPORT( DynamicArrayDim,                   "dynamic array dimension"                                                                                       );
DECL_REPORT( BoolLiteralValue,                  "boolean literal value '{0}'"                                                                                   );
DECL_REPORT( LiteralType,                       "literal type '{0}'"                                                                                            );
DECL_REPORT( TypeSpecifier,                     "type specifier"                                                                                                );
DECL_REPORT( DivisionByZero,                    "division by zero"                                                                                              );
DECL_REPORT( TypeCast,                          "type cast '{0}'"                                                                                               );
DECL_REPORT( InitializerList,                   "initializer list"                                                                                              );
DECL_REPORT( FunctionCall,                      "function call"                                                                                                 );

/* ----- ExprConverter ----- */

DECL_REPORT( MissingArrayIndexInOp,             "missing array index in operator of '{0}'"                                                                      );

/* ----- ReferenceAnalyzer ----- */

DECL_REPORT( CallStack,                         "call stack"                                                                                                    );
DECL_REPORT( IllegalRecursiveCall,              "illegal recursive call[ of function '{0}']"                                                                    );
DECL_REPORT( MissingFuncImpl,                   "missing function implementation[ for '{0}']"                                                                   );

/* ----- ReflectionAnalyzer ----- */

DECL_REPORT( InvalidTypeOrArgCount,             "invalid type or invalid number of arguments"                                                                   );
DECL_REPORT( InvalidArgCount,                   "invalid number of arguments[ for {0}]"                                                                         );
DECL_REPORT( FailedToInitializeSamplerValue,    "{0} to initialize sampler value '{1}'"                                                                         );

/* ----- Converter ----- */

DECL_REPORT( SelfParamStackUnderflow,           "'self'-parameter stack underflow"                                                                              );
DECL_REPORT( NoActiveStmntScopeHandler,         "no active statement scope handler"                                                                             );
DECL_REPORT( MissingScopedStmntRef,             "missing reference to scoped statement"                                                                         );

/* ----- GLSLConverter ----- */

DECL_REPORT( MissingSelfParamForMemberFunc,     "missing 'self'-parameter for member function[ '{0}']"                                                          );
DECL_REPORT( FailedToGetTextureDim,             "failed to determine dimension of texture object[ '{0}']"                                                       );
DECL_REPORT( FailedToMapClassIntrinsicOverload, "failed to map overload of class intrinsic '{0}' for type '{1}'"                                                );

/* ----- GLSLExtensionAgent ----- */

DECL_REPORT( GLSLExtensionOrVersionRequired,    "GLSL extension '{0}' or shader output version '{1}' required[ (for {2})]"                                      );
DECL_REPORT( GLSLExtensionAcquired,             "GLSL extension '{0}' acquired due to shader output version below '{1}'[ (for {2})]"                            );
DECL_REPORT( NoGLSLExtensionVersionRegisterd,   "no GLSL version is registered for the extension '{0}'"                                                         );
DECL_REPORT( FragmentCoordinate,                "fragment coordinate"                                                                                           );
DECL_REPORT( EarlyDepthStencil,                 "early depth stencil test"                                                                                      );
DECL_REPORT( MultiDimArray,                     "multi-dimensional array"                                                                                       );
DECL_REPORT( TextureCubeArray,                  "texture cube array"                                                                                            );
DECL_REPORT( PackOffsetLayout,                  "pack offset layout"                                                                                            );
DECL_REPORT( ConstantBuffer,                    "constant buffer"                                                                                               );
DECL_REPORT( ExplicitBindingSlot,               "explicit binding slot"                                                                                         );
DECL_REPORT( MultiSampledTexture,               "multi-sampled texture"                                                                                         );
DECL_REPORT( BitwiseOperator,                   "bitwise operator"                                                                                              );

/* ----- GLSLGenerator ----- */

DECL_REPORT( EntryPointNotFound,                "entry point \"{0}\" not found"                                                                                 );
DECL_REPORT( FailedToMapToGLSLKeyword,          "failed to map {0} to GLSL keyword[ ({1})]"                                                                     );
DECL_REPORT( FailedToMapGLSLImageDataType,      "failed to map data type to image format GLSL keyword"                                                          );
DECL_REPORT( FailedToWriteLiteralType,          "failed to write type denoter for literal[ '{0}']"                                                              );
DECL_REPORT( FailedToDetermineGLSLDataType,     "failed to determine GLSL data type"                                                                            );
DECL_REPORT( TessAbstractPatchType,             "tessellation abstract patch type"                                                                              );
DECL_REPORT( TessSpacing,                       "tessellation spacing"                                                                                          );
DECL_REPORT( TessPrimitiveOrdering,             "tessellation primitive ordering"                                                                               );
DECL_REPORT( InputGeometryPrimitive,            "input geometry primitive"                                                                                      );
DECL_REPORT( OutputGeometryPrimitive,           "output geometry primitive"                                                                                     );
DECL_REPORT( OutputSemantic,                    "output semantic"                                                                                               );
DECL_REPORT( VertexSemanticNotFound,            "vertex semantic '{0}' specified but not found"                                                                 );
DECL_REPORT( MultiUseOfVertexSemanticLocation,  "multiple usage of vertex semantic location ({0})[ (used {1} times)]"                                           );
DECL_REPORT( InvalidControlPathInUnrefFunc,     "not all control paths in unreferenced function '{0}' return a value"                                           );
DECL_REPORT( InvalidControlPathInFunc,          "not all control paths in function '{0}' return a value"                                                        );
DECL_REPORT( MissingInputPrimitiveType,         "missing input primitive type[ for {0}]"                                                                        );
DECL_REPORT( MissingOutputPrimitiveType,        "missing output primitive type[ for {0}]"                                                                       );
DECL_REPORT( MissingFuncName,                   "missing function name"                                                                                         );
DECL_REPORT( TooManyIndicesForShaderInputParam, "too many array indices for shader input parameter"                                                             );
DECL_REPORT( InterpModNotSupportedForGLSL120,   "interpolation modifiers not supported for GLSL version 120 or below"                                           );
DECL_REPORT( InvalidParamVarCount,              "invalid number of variables in function parameter"                                                             ); // internal error
DECL_REPORT( NotAllStorageClassesMappedToGLSL,  "not all storage classes can be mapped to GLSL keywords"                                                        );
DECL_REPORT( NotAllInterpModMappedToGLSL,       "not all interpolation modifiers can be mapped to GLSL keywords"                                                );
DECL_REPORT( CantTranslateSamplerToGLSL,        "can not translate sampler state object to GLSL sampler"                                                        );
DECL_REPORT( MissingArrayPrefixForIOSemantic,   "missing array prefix expression for input/output semantic[ '{0}']"                                             );

/* ----- GLSLPreProcessor ----- */

DECL_REPORT( MacrosBeginWithGLReserved,         "macros beginning with 'GL_' are reserved[: {0}]"                                                               );
DECL_REPORT( MacrosWithTwoUnderscoresReserved,  "macros containing consecutive underscores '__' are reserved[: {0}]"                                            );
DECL_REPORT( IllegalRedefOfStdMacro,            "illegal redefinition of standard macro[: {0}]"                                                                 );
DECL_REPORT( IllegalUndefOfStdMacro,            "illegal undefinition of standard macro[: {0}]"                                                                 );
DECL_REPORT( VersionMustBeFirstDirective,       "'#version'-directive must be the first directive"                                                              );
DECL_REPORT( UnknwonGLSLVersion,                "unknown GLSL version: '{0}'"                                                                                   );
DECL_REPORT( NoProfileForGLSLVersionBefore150,  "versions before 150 do not allow a profile token"                                                              );
DECL_REPORT( InvalidGLSLVersionProfile,         "invalid version profile '{0}' (must be 'core' or 'compatibility')"                                             );
DECL_REPORT( ExtensionNotSupported,             "extension not supported[: {0}]"                                                                                );
DECL_REPORT( InvalidGLSLExtensionBehavior,      "invalid extension behavior '{0}' (must be 'enable', 'require', 'warn', or 'disable')"                          );

/* ----- HLSLIntrinsics ----- */

DECL_REPORT( FailedToDeriveIntrinsicType,       "failed to derive type denoter for intrinsic[ '{0}']"                                                           );
DECL_REPORT( FailedToDeriveIntrinsicParamType,  "failed to derive parameter type denoter for intrinsic[ '{0}']"                                                 );
DECL_REPORT( MissingTypeInTextureIntrinsic,     "missing buffer type in texture intrinsic[ '{0}']"                                                              );

/* ----- HLSLKeywords ----- */

DECL_REPORT( FailedToMapFromHLSLKeyword,        "failed to map HLSL keyword '{0}' to {1}"                                                                       );
DECL_REPORT( FailedToMapFromCgKeyword,          "failed to map Cg keyword '{0}' to {1}"                                                                         );
DECL_REPORT( InvalidSystemValueSemantic,        "invalid system value semantic \"{0}\""                                                                         );

/* ----- HLSLScanner ----- */

DECL_REPORT( KeywordReservedForFutureUse,       "keyword[ '{0}'] is reserved for future use"                                                                    );
DECL_REPORT( KeywordNotSupportedYet,            "keyword[ '{0}'] is currently not supported"                                                                    );

/* ----- HLSLParser ----- */

DECL_REPORT( UnknownAttribute,                  "unknown attribute: '{0}'"                                                                                      );
DECL_REPORT( UnknownSlotRegister,               "unknown slot register: '{0}'"                                                                                  );
DECL_REPORT( ExpectedExplicitArrayDim,          "explicit array dimension expected"                                                                             );
DECL_REPORT( ExpectedVarOrAssignOrFuncCall,     "expected variable declaration, assignment, or function call statement"                                         );
DECL_REPORT( ExpectedTypeNameOrFuncCall,        "expected type name or function call expression"                                                                );
DECL_REPORT( ExpectedUnaryOp,                   "expected unary operator"                                                                                       );
DECL_REPORT( ExpectedIdentPrefix,               "expected '::' or '.' identifier prefix"                                                                        );
DECL_REPORT( UnexpectedTokenInPackMatrixPragma, "unexpected token in '#pragma pack_matrix'-directive"                                                           );
DECL_REPORT( UnexpectedPreParsedAST,            "unexpected pre-parsed AST node"                                                                                );
DECL_REPORT( InvalidHLSLDirectiveAfterPP,       "only '#line' and '#pragma' directives are allowed after pre-processing"                                        );
DECL_REPORT( InvalidHLSLPragmaAfterPP,          "only 'pack_matrix' pragma directive is allowed after pre-processing"                                           );
DECL_REPORT( IllegalRecursiveInheritance,       "recursive inheritance is not allowed"                                                                          );
DECL_REPORT( IllegalMultipleInheritance,        "multiple inheritance is not allowed"                                                                           );
DECL_REPORT( IllegalDeclStmntInsideDeclOf,      "illegal declaration statement inside declaration of '{0}'"                                                     );
DECL_REPORT( IllegalBufferTypeGenericSize,      "illegal usage of generic size in texture, buffer, or stream object"                                            );
DECL_REPORT( IllegalPackOffset,                 "packoffset is only allowed in a constant buffer"                                                               );
DECL_REPORT( TextureSampleCountLimitIs128,      "number of samples in texture must be in the range [1, 128), but got {0}"                                       );
DECL_REPORT( PatchCtrlPointLimitIs64,           "number of control points in patch must be in the range [1, 64], but got {0}"                                   );
DECL_REPORT( VectorAndMatrixDimOutOfRange,      "vector and matrix dimensions must be in the range [1, 4], but got {0}"                                         );
DECL_REPORT( TechniquesAreIgnored,              "techniques are ignored"                                                                                        );
DECL_REPORT( MissingClosingBrace,               "missing closing brace '}' for open code block"                                                                 );
DECL_REPORT( RegisterIgnoredForVarDecls,        "register is ignored for variable declarations"                                                                 );
DECL_REPORT( RegisterIgnoredForFuncDecls,       "register is ignored for function declarations"                                                                 );
DECL_REPORT( ExpectedSamplerOrSamplerState,     "expected sampler type denoter or sampler state"                                                                );
DECL_REPORT( ExpectedOpenBracketOrAngleBracket, "expected '<' or '('"                                                                                           );
DECL_REPORT( DuplicatedPrimitiveType,           "duplicate primitive type specified"                                                                            );
DECL_REPORT( ConflictingPrimitiveTypes,         "conflicting primitive types"                                                                                   );

/* ----- HLSLAnalyzer ----- */

DECL_REPORT( SecondEntryPointNotFound,          "secondary entry point \"{0}\" not found"                                                                       );
DECL_REPORT( NestedStructsMustBeAnonymous,      "nested structures must be anonymous"                                                                           );
DECL_REPORT( TypeHasNoMemberVariables,          "'{0}' has no member variables"                                                                                 );
DECL_REPORT( BufferCanOnlyHaveOneSlot,          "buffers can only be bound to one slot"                                                                         );
DECL_REPORT( UserCBuffersCantBeTargetSpecific,  "user-defined constant buffer slots can not be target specific"                                                 );
DECL_REPORT( DeclShadowsPreviousLocal,          "declaration of '{0}' shadows a previous local at ({1})"                                                        );
DECL_REPORT( DeclShadowsMemberOfBase,           "declaration of '{0}' shadows member of base '{1}'"                                                             );
DECL_REPORT( ReturnOutsideFuncDecl,             "return statement outside function declaration"                                                                 );
DECL_REPORT( ReturnExpression,                  "return expression"                                                                                             );
DECL_REPORT( ArrayIndex,                        "array index"                                                                                                   );
DECL_REPORT( ArrayIndexMustHaveBaseType,        "array index must have scalar or vector type with correct number of dimensions[ (but got '{0}')]"               );
DECL_REPORT( VarAssignment,                     "variable assignment"                                                                                           );
DECL_REPORT( VarInitialization,                 "variable initialization"                                                                                       );
DECL_REPORT( IntrinsicNotDeclaredInObject,      "intrinsic '{0}' not declared in object '{1}'"                                                                  );
DECL_REPORT( InvalidShaderModelForIntrinsic,    "intrinsic '{0}' requires shader model {1}, but only {2} is specified"                                          );
DECL_REPORT( InvalidIntrinsicForTexture,        "invalid intrinsic '{0}' for texture object"                                                                    );
DECL_REPORT( InvalidIntrinsicForRWTexture,      "invalid intrinsic '{0}' for RW-texture object"                                                                 );
DECL_REPORT( InvalidIntrinsicForStreamOutput,   "invalid intrinsic '{0}' for stream-output object"                                                              );
DECL_REPORT( InvalidGlobalIntrinsicForType,     "invalid global intrinsic '{0}' for type '{1}'"                                                                 );
DECL_REPORT( InvalidClassIntrinsicForType,      "invalid class intrinsic '{0}' for type '{1}'"                                                                  );
DECL_REPORT( InvalidClassIntrinsic,             "invalid class intrinsic '{0}' for global function call"                                                        );
DECL_REPORT( InvalidMemberFuncForType,          "invalid member function '{0}' for type '{1}'"                                                                  );
DECL_REPORT( InvalidSymbolRefToVarIdent,        "invalid symbol reference to variable identifier '{0}'"                                                         );
DECL_REPORT( InvalidVarDeclCountInParam,        "invalid number of variable declarations in function parameter"                                                 ); // internal error
DECL_REPORT( InvalidInputSemanticInEntryPoint,  "invalid input semantic '{0}' in entry point '{1}'"                                                             );
DECL_REPORT( InvalidOutputSemanticInEntryPoint, "invalid output semantic '{0}' in entry point '{1}'"                                                            );
DECL_REPORT( MissingInputSemanticInEntryPoint,  "missing input semantic '{0}' in entry point '{1}'"                                                             );
DECL_REPORT( MissingOutputSemanticInEntryPoint, "missing output semantic '{0}' in entry point '{1}'"                                                            );
DECL_REPORT( MissingAttributeForEntryPoint,     "missing '{0}' attribute for entry point"                                                                       );
DECL_REPORT( MissingExprInReturnForFunc,        "missing expression in return statement for function with '{0}' return type"                                    );
DECL_REPORT( MissingInitializerForDefaultParam, "missing initializer expression for default parameter '{0}'"                                                    ); // internal error
DECL_REPORT( MissingSemanticInEntryPointParam,  "missing semantic in parameter '{0}' of entry point"                                                            );
DECL_REPORT( MissingGenericTypeDen,             "missing generic type denoter[ in '{0}']"                                                                       );
DECL_REPORT( IllegalUseOfNormModifiers,         "'snorm' and 'unorm' type modifiers can only be used for floating-point types"                                  );
DECL_REPORT( IllegalExprInReturnForVoidFunc,    "illegal expression in return statement for function with 'void' return type"                                   );
DECL_REPORT( IllegalBufferTypeForEntryPoint,    "illegal buffer type for entry point[ {0}]"                                                                     );
DECL_REPORT( IllegalLValueAssignmentToConst,    "illegal assignment to l-value '{0}' that is[ {1}] declared as constant"                                        );
DECL_REPORT( IllegalRValueAssignment,           "illegal assignment to r-value expression"                                                                      );
DECL_REPORT( IllegalNonStaticAccessToMember,    "illegal non-static access to static structure member[ '{0}']"                                                  );
DECL_REPORT( IllegalStaticAccessToMember,       "illegal static access to non-static structure member[ '{0}']"                                                  );
DECL_REPORT( IllegalNonStaticAccessToType,      "illegal non-static access to type[ '{0}']"                                                                     );
DECL_REPORT( IllegalStaticAccessToNonType,      "illegal static access to non-typename expression"                                                              );
DECL_REPORT( IllegalStaticIntrinsicCall,        "illegal static call to intrinsic[ '{0}']"                                                                      );
DECL_REPORT( IllegalStaticFuncCall,             "illegal static call to function[ '{0}']"                                                                       );
DECL_REPORT( IllegalNonStaticFuncCall,          "illegal call to static function[ '{0}']"                                                                       );
DECL_REPORT( IllegalDefOfNonStaticMemberVar,    "illegal definition of non-static member variable[ '{0}']"                                                      );
DECL_REPORT( DuplicateUseOfOutputSemantic,      "duplicate use of output semantic '{0}'"                                                                        );
DECL_REPORT( UniformCantBeOutput,               "uniforms can not be defined as output"                                                                         );
DECL_REPORT( TooManyArgsForAttribute,           "too many arguments for attribute[ '{0}'][ (expected {1}, but got {2})]"                                        );
DECL_REPORT( TooFewArgsForAttribute,            "too few arguments for attribute[ '{0}'][ (expected {1}, but got {2})]"                                         );
DECL_REPORT( ExpectedIdentArgInAttribute,       "expected identifier as argument for attribute[ '{0}']"                                                         );
DECL_REPORT( InvalidIdentArgInAttribute,        "invalid identifier '{0}' used as argument for attribute[ '{1}']"                                               );
DECL_REPORT( ExpectedDomainTypeParamToBe,       "expected domain type parameter to be \"tri\", \"quad\", or \"isoline\""                                        );
DECL_REPORT( ExpectedOutputTopologyParamToBe,   "expected output topology parameter to be \"point\", \"line\", \"triangle_cw\", or \"triangle_ccw\""            );
DECL_REPORT( ExpectedPartitioningModeParamToBe, "expected partitioning mode parameter to be \"integer\", \"pow2\", \"fractional_even\", or \"fractional_odd\""  );
DECL_REPORT( ExpectedOutputCtrlPointParamToBe,  "expected output control point parameter to be an unsigned integer"                                             );
DECL_REPORT( ExpectedPatchFuncParamToBe,        "expected patch constant function parameter to be a string literal"                                             );
DECL_REPORT( EntryPointForPatchFuncNotFound,    "entry point \"{0}\" for patch constant function not found"                                                     );
DECL_REPORT( MaxVertexCountMustBeGreaterZero,   "maximal vertex count must be greater than zero"                                                                );
DECL_REPORT( NumThreadsMustBeGreaterZero,       "number of threads must be greater than zero"                                                                   );
DECL_REPORT( SecondaryArrayDimMustBeExplicit,   "secondary array dimensions must be explicit"                                                                   );
DECL_REPORT( StructsCantBeDefinedInParam,       "structures can not be defined in a parameter type[: '{0}']"                                                    );
DECL_REPORT( StaticMembersCantBeDefinedInGlob,  "static members can only be defined in global scope[: '{0}']"                                                   );
DECL_REPORT( StaticMemberVarRedef,              "redefinition of static member variable[ '{0}']"                                                                );
DECL_REPORT( MemberVarsCantHaveDefaultValues,   "member variables can not have default values[: '{0}']"                                                         );
DECL_REPORT( DeclTypeDiffersFromDefType,        "declaration type '{0}' differs from definition type '{1}'"                                                     );
DECL_REPORT( ArrayTypeCanOnlyAppearInDef,       "array type can only appear in definition of static member variables[: '{0}']"                                  );

/* ----- Xsc ----- */

DECL_REPORT( InputStreamCantBeNull,             "input stream must not be null"                                                                                 );
DECL_REPORT( OutputStreamCantBeNull,            "output stream must not be null"                                                                                );
DECL_REPORT( NameManglingPrefixResCantBeEmpty,  "name mangling prefix for reserved words must not be empty"                                                     );
DECL_REPORT( NameManglingPrefixTmpCantBeEmpty,  "name mangling prefix for temporary variables must not be empty"                                                );
DECL_REPORT( OverlappingNameManglingPrefixes,   "overlapping name mangling prefixes"                                                                            );
DECL_REPORT( LangExtensionsNotSupported,        "compiler was not build with language extensions"                                                               );
DECL_REPORT( PreProcessingSourceFailed,         "preprocessing input code failed"                                                                               );
DECL_REPORT( ParsingSourceFailed,               "parsing input code failed"                                                                                     );
DECL_REPORT( AnalyzingSourceFailed,             "analyzing input code failed"                                                                                   );
DECL_REPORT( GeneratingOutputCodeFailed,        "generating output code failed"                                                                                 );
DECL_REPORT( OnlyPreProcessingForNonHLSL,       "only pre-processing supported for shaders other than HLSL or Cg"                                               );

/* ----- Shell ----- */

DECL_REPORT( UnexpectedEndOfCmdLine,            "unexpected end of command line arguments"                                                                      );
DECL_REPORT( ExpectedCmdLineBoolean,            "expected '{0}' or '{1}', but got '{2}'"                                                                        );
DECL_REPORT( MissingValueInShellCmd,            "missing value in command '{0}'"                                                                                );
DECL_REPORT( PressAnyKeyToContinue,             "press any key to continue ..."                                                                                 );
DECL_REPORT( FailedToReadFile,                  "failed to read file: \"{0}\""                                                                                  );
DECL_REPORT( FailedToWriteFile,                 "failed to write file: \"{0}\""                                                                                 );
DECL_REPORT( ValidateShader,                    "validate \"{0}\""                                                                                              );
DECL_REPORT( ValidationSuccessful,              "validation successful"                                                                                         );
DECL_REPORT( ValidationFailed,                  "validation failed"                                                                                             );
DECL_REPORT( CompileShader,                     "compile \"{0}\" to \"{1}\""                                                                                    );
DECL_REPORT( CompilationSuccessful,             "compilation successful"                                                                                        );
DECL_REPORT( CompilationFailed,                 "compilation failed"                                                                                            );

/* ----- Extensions ----- */

#ifdef XSC_ENABLE_LANGUAGE_EXT

DECL_REPORT( InvalidImageFormatForType,         "invalid image format '{0}' used for buffer of type '{1}'"                                                      );
DECL_REPORT( AttributeRequiresExtension,        "attribute '{0}' requires language extension '{1}'"                                                             );
DECL_REPORT( IllegalVectorSpaceAssignment,      "illegal assignment of '{0}' vector-space to '{1}' vector-space"                                                );
DECL_REPORT( InconsistVectorSpacesInTypes,      "inconsistent vector-spaces between type denoters[ (found '{0}' and '{1}')]"                                    );
DECL_REPORT( ExpectedIdentInSpaceAttr,          "expected identifier as argument in 'space' attribute"                                                          );

#endif


#endif



// ================================================================================