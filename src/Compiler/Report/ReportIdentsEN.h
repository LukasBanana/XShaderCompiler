/*
 * ReportIdentsEN.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_IDENTS_EN_H
#define XSC_REPORT_IDENTS_EN_H


/* ----- Common ----- */

DECL_REPORT( Warning,                           "warning"                                                                                   );
DECL_REPORT( Error,                             "error"                                                                                     );
DECL_REPORT( Context,                           "context"                                                                                   );
DECL_REPORT( Syntax,                            "syntax"                                                                                    );
DECL_REPORT( ContextError,                      "context error"                                                                             );
DECL_REPORT( InternalError,                     "internal error"                                                                            );
DECL_REPORT( In,                                "in"                                                                                        ); // e.g. "error in 'context'"


/* ----- Token ----- */

DECL_REPORT( Identifier,                        "identifier"                                                                                );
DECL_REPORT( BoolLiteral,                       "boolean literal"                                                                           );
DECL_REPORT( IntLiteral,                        "integer literal"                                                                           );
DECL_REPORT( FloatLiteral,                      "floating-point literal"                                                                    );
DECL_REPORT( StringLiteral,                     "string literal"                                                                            );
DECL_REPORT( CharLiteral,                       "character literal"                                                                         );
DECL_REPORT( NullLiteral,                       "null literal"                                                                              );
DECL_REPORT( AssignOp,                          "assign operator"                                                                           );
DECL_REPORT( BinaryOp,                          "binary operator"                                                                           );
DECL_REPORT( UnaryOp,                           "unary operator"                                                                            );
DECL_REPORT( TernaryOp,                         "ternary operator"                                                                          );
DECL_REPORT( StringType,                        "string type denoter"                                                                       );
DECL_REPORT( ScalarType,                        "scalar type denoter"                                                                       );
DECL_REPORT( VectorType,                        "vector type denoter"                                                                       );
DECL_REPORT( MatrixType,                        "matrix type denoter"                                                                       );
DECL_REPORT( VoidType,                          "void type denoter"                                                                         );
DECL_REPORT( PrimitiveType,                     "primitive type denoter"                                                                    );
DECL_REPORT( ReservedWord,                      "reserved keyword"                                                                          );
DECL_REPORT( VectorGenericType,                 "vector generic type denoter"                                                               );
DECL_REPORT( MatrixGenericType,                 "matrix generic type denoter"                                                               );
DECL_REPORT( SamplerType,                       "sampler type denoter"                                                                      );
DECL_REPORT( SamplerState,                      "sampler state"                                                                             );
DECL_REPORT( BufferType,                        "buffer type denoter"                                                                       );
DECL_REPORT( UniformBufferType,                 "uniform buffer type denoter"                                                               );
DECL_REPORT( KeywordDo,                         "'do' keyword"                                                                              );
DECL_REPORT( KeywordWhile,                      "'while' keyword"                                                                           );
DECL_REPORT( KeywordFor,                        "'for' keyword"                                                                             );
DECL_REPORT( KeywordIf,                         "'if' keyword"                                                                              );
DECL_REPORT( KeywordElse,                       "'else' keyword"                                                                            );
DECL_REPORT( KeywordSwitch,                     "'switch' keyword"                                                                          );
DECL_REPORT( KeywordCase,                       "'case' keyword"                                                                            );
DECL_REPORT( KeywordDefault,                    "'default' keyword"                                                                         );
DECL_REPORT( KeywordTypedef,                    "'typedef' keyword"                                                                         );
DECL_REPORT( KeywordStruct,                     "'struct' keyword"                                                                          );
DECL_REPORT( KeywordRegister,                   "'register' keyword"                                                                        );
DECL_REPORT( KeywordPackOffset,                 "'packoffset' keyword"                                                                      );
DECL_REPORT( KeywordReturn,                     "'return' keyword"                                                                          );
DECL_REPORT( KeywordInline,                     "'inline' keyword"                                                                          );
DECL_REPORT( KeywordTechnique,                  "'technique' keyword"                                                                       );
DECL_REPORT( KeywordPass,                       "'pass' keyword"                                                                            );
DECL_REPORT( KeywordCompile,                    "'compile' keyword"                                                                         );
DECL_REPORT( CtrlTransfer,                      "control transfer"                                                                          );
DECL_REPORT( InputModifier,                     "input modifier"                                                                            );
DECL_REPORT( InterpModifier,                    "interpolation modifier"                                                                    );
DECL_REPORT( TypeModifier,                      "type modifier"                                                                             );
DECL_REPORT( StorageClass,                      "storage class"                                                                             );
DECL_REPORT( Comment,                           "comment"                                                                                   );
DECL_REPORT( WhiteSpaces,                       "white spaces"                                                                              );
DECL_REPORT( NewLineChars,                      "new-line characters"                                                                       );
DECL_REPORT( VarArgSpecifier,                   "variadic argument specifier"                                                               );
DECL_REPORT( Misc,                              "miscellaneous"                                                                             );
DECL_REPORT( PPDirective,                       "preprocessor directive"                                                                    );
DECL_REPORT( PPDirectiveConcat,                 "preprocessor directive concatenation"                                                      );
DECL_REPORT( PPLineBreak,                       "preprocessor line break"                                                                   );
DECL_REPORT( EndOfStream,                       "end-of-stream"                                                                             );


/* ----- Scanner ----- */

DECL_REPORT( LexicalError,                      "lexical error"                                                                             );
DECL_REPORT( UnexpectedChar,                    "unexpected character '{0}'[ (expected '{1}')]"                                             );
DECL_REPORT( MissingDigitSequenceAfterExpr,     "missing digit-sequence after exponent part"                                                );


/* ----- Parser ----- */

DECL_REPORT( UnexpectedToken,                   "unexpected token: {0}[ ({1})]"                                                             );
DECL_REPORT( UnexpectedEndOfStream,             "unexpected end-of-stream"                                                                  );
DECL_REPORT( UnexpectedTokenSpell,              "unexpected token spelling '{0}'[ (expected '{1}')]"                                        );
DECL_REPORT( Expected,                          "expected"                                                                                  );
DECL_REPORT( InFunction,                        " (in function: {0})"                                                                       );
DECL_REPORT( FailedToCreateScanner,             "failed to create token scanner"                                                            );
DECL_REPORT( FailedToScanSource,                "failed to scan source code"                                                                );
DECL_REPORT( MissingScanner,                    "missing token scanner"                                                                     );
DECL_REPORT( SubExprMustNotBeEmpty,             "sub-expressions must not be empty"                                                         );
DECL_REPORT( SubExprAndOpsUncorrelated,         "sub-expressions and operators have uncorrelated number of elements"                        );
DECL_REPORT( TooManySyntaxErrors,               "too many syntax errors"                                                                    );


/* ----- Analyzer ----- */

DECL_REPORT( UndeclaredIdent,                   "undeclared identifier \"{0}\"[ in '{1}'][; did you mean \"{2}\"?]"                         );
DECL_REPORT( StatementWithEmptyBody,            "<{0}> statement with empty body"                                                           );
DECL_REPORT( IdentDoesNotNameAFunction,         "identifier '{0}' does not name a function"                                                 );
DECL_REPORT( MissingReferenceToStructInType,    "missing reference to structure declaration in type denoter '{0}'"                          );
DECL_REPORT( MissingVariableType,               "missing variable type"                                                                     );
DECL_REPORT( ParameterCantBeUniformAndOut,      "type attributes 'out' and 'inout' can not be used together with 'uniform' for a parameter" );
DECL_REPORT( IllegalCast,                       "can not cast '{0}' to '{1}'[ in {2}]"                                                      );
DECL_REPORT( NullPointerArgument,               "null pointer passed to {0}"                                                                );
DECL_REPORT( ConditionalExprNotScalar,          "conditional expression must evaluate to scalar, but got '{0}'"                             );
DECL_REPORT( ExpectedConstExpr,                 "expected constant expression"                                                              );
DECL_REPORT( ExpectedConstIntExpr,              "expected constant integer expression"                                                      );
DECL_REPORT( ExpectedConstFloatExpr,            "expected constant floating-point expression"                                               );


#endif



// ================================================================================