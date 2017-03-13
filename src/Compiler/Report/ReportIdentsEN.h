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
DECL_REPORT( ContextError,                      "context error"                                                                             );
DECL_REPORT( InternalError,                     "internal error"                                                                            );
DECL_REPORT( In,                                "in"                                                                                        ); // e.g. "error in 'context'"


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