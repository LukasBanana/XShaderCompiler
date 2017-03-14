/*
 * ReportIdentsDE.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REPORT_IDENTS_DE_H
#define XSC_REPORT_IDENTS_DE_H


/* ----- Common ----- */

DECL_REPORT( Message,                           "Nachricht"                                                                                                     );
DECL_REPORT( Warning,                           "Warnung"                                                                                                       );
DECL_REPORT( Error,                             "Fehler"                                                                                                        );
DECL_REPORT( Syntax,                            "Syntax"                                                                                                        );
DECL_REPORT( Context,                           "Kontext"                                                                                                       );
DECL_REPORT( Reflection,                        "Reflektion"                                                                                                    );
DECL_REPORT( CodeGeneration,                    "Code Generation"                                                                                               );
DECL_REPORT( CodeReflection,                    "Code Reflektion"                                                                                               );
DECL_REPORT( ContextError,                      "Kontext Fehler"                                                                                                );
DECL_REPORT( InternalError,                     "Interner Fehler"                                                                                               );
DECL_REPORT( In,                                "in"                                                                                                            ); // e.g. "error in 'context'"
DECL_REPORT( Input,                             "Eingabe"                                                                                                       ); // e.g. "entry point input"
DECL_REPORT( Output,                            "Ausgabe"                                                                                                       ); // e.g. "entry point output"
DECL_REPORT( Anonymous,                         "anonym"                                                                                                        );
DECL_REPORT( CandidatesAre,                     "Kandidaten sind"                                                                                               );
DECL_REPORT( StackUnderflow,                    "Stack Unterlauf[ in {0}]"                                                                                      );
DECL_REPORT( VertexShader,                      "Vertex Shader"                                                                                                 );
DECL_REPORT( TessControlShader,                 "Tessellation-Control Shader"                                                                                   );
DECL_REPORT( TessEvaluationShader,              "Tessellation-Evaluation Shader"                                                                                );
DECL_REPORT( GeometryShader,                    "Geometry Shader"                                                                                               );
DECL_REPORT( FragmentShader,                    "Fragment Shader"                                                                                               );
DECL_REPORT( ComputeShader,                     "Compute Shader"                                                                                                );
DECL_REPORT( InvalidOutputStream,               "ungültiger Ausgabestrom"                                                                                       );
DECL_REPORT( Implicitly,                        "implizit"                                                                                                      );
DECL_REPORT( ButGot,                            "[, aber erhielt {0}]"                                                                                          );

/* ----- Token ----- */

DECL_REPORT( Identifier,                        "Identifizierer"                                                                                                );
DECL_REPORT( BoolLiteral,                       "boolsches Literal"                                                                                             );
DECL_REPORT( IntLiteral,                        "ganzzahliges Literal"                                                                                          );
DECL_REPORT( FloatLiteral,                      "fließkomma Literal"                                                                                            );
DECL_REPORT( StringLiteral,                     "String Literal"                                                                                                );
DECL_REPORT( CharLiteral,                       "Zeichen Literal"                                                                                               );
DECL_REPORT( NullLiteral,                       "Null Literal"                                                                                                  );
DECL_REPORT( AssignOp,                          "Zuweisungsoperator[ '{0}']"                                                                                    );
DECL_REPORT( BinaryOp,                          "Binäroperator[ '{0}']"                                                                                         );
DECL_REPORT( UnaryOp,                           "Unäroperator[ '{0}']"                                                                                          );
DECL_REPORT( TernaryOp,                         "Dreifachoperator"                                                                                              );
DECL_REPORT( StringTypeDen,                     "String Typbezeichner"                                                                                          );
DECL_REPORT( ScalarTypeDen,                     "Skalar Typbezeichner"                                                                                          );
DECL_REPORT( VectorTypeDen,                     "Vektor Typbezeichner"                                                                                          );
DECL_REPORT( MatrixTypeDen,                     "Matrix Typbezeichner"                                                                                          );
DECL_REPORT( VoidTypeDen,                       "void Typbezeichner"                                                                                            );
DECL_REPORT( PrimitiveTypeDen,                  "primitive Typbezeichner"                                                                                       );
DECL_REPORT( ReservedWord,                      "reserviertes Schlüsselwort"                                                                                    );
DECL_REPORT( VectorGenericTypeDen,              "generischer Vektor Typbezeichner"                                                                              );
DECL_REPORT( MatrixGenericTypeDen,              "generischer Matrix Typbezeichner"                                                                              );
DECL_REPORT( SamplerTypeDen,                    "sampler Typbezeichner"                                                                                         );
DECL_REPORT( SamplerState,                      "sampler state"                                                                                                 );
DECL_REPORT( BufferTypeDen,                     "buffer Typbezeichner"                                                                                          );
DECL_REPORT( UniformBufferTypeDen,              "uniform buffer Typbezeichner"                                                                                  );
DECL_REPORT( KeywordDo,                         "'do' Schlüsselwort"                                                                                            );
DECL_REPORT( KeywordWhile,                      "'while' Schlüsselwort"                                                                                         );
DECL_REPORT( KeywordFor,                        "'for' Schlüsselwort"                                                                                           );
DECL_REPORT( KeywordIf,                         "'if' Schlüsselwort"                                                                                            );
DECL_REPORT( KeywordElse,                       "'else' Schlüsselwort"                                                                                          );
DECL_REPORT( KeywordSwitch,                     "'switch' Schlüsselwort"                                                                                        );
DECL_REPORT( KeywordCase,                       "'case' Schlüsselwort"                                                                                          );
DECL_REPORT( KeywordDefault,                    "'default' Schlüsselwort"                                                                                       );
DECL_REPORT( KeywordTypedef,                    "'typedef' Schlüsselwort"                                                                                       );
DECL_REPORT( KeywordStruct,                     "'struct' Schlüsselwort"                                                                                        );
DECL_REPORT( KeywordRegister,                   "'register' Schlüsselwort"                                                                                      );
DECL_REPORT( KeywordPackOffset,                 "'packoffset' Schlüsselwort"                                                                                    );
DECL_REPORT( KeywordReturn,                     "'return' Schlüsselwort"                                                                                        );
DECL_REPORT( KeywordInline,                     "'inline' Schlüsselwort"                                                                                        );
DECL_REPORT( KeywordTechnique,                  "'technique' Schlüsselwort"                                                                                     );
DECL_REPORT( KeywordPass,                       "'pass' Schlüsselwort"                                                                                          );
DECL_REPORT( KeywordCompile,                    "'compile' Schlüsselwort"                                                                                       );
DECL_REPORT( CtrlTransfer,                      "Kontrollflußübergang"                                                                                          );
DECL_REPORT( InputModifier,                     "Eingabemodifizierer"                                                                                           );
DECL_REPORT( InterpModifier,                    "Interpolationsmodifizierer"                                                                                    );
DECL_REPORT( TypeModifier,                      "Typmodifizierer"                                                                                               );
DECL_REPORT( StorageClass,                      "Speicherklasse"                                                                                                );
DECL_REPORT( Comment,                           "Kommentar"                                                                                                     );
DECL_REPORT( WhiteSpaces,                       "Leerzeichen"                                                                                                   );
DECL_REPORT( NewLineChars,                      "Neue-Zeile Zeichen"                                                                                            );
DECL_REPORT( VarArgSpecifier,                   "variadisches-Argument Spezifikationssymbol"                                                                    );
DECL_REPORT( Misc,                              "Sonstiges"                                                                                                     );
DECL_REPORT( PPDirective,                       "Präprozessor Direktive"                                                                                        );
DECL_REPORT( PPDirectiveConcat,                 "Präprozessor Direktivenkonkatenation"                                                                          );
DECL_REPORT( PPLineBreak,                       "Präprozessor Zeilenumbruch"                                                                                    );
DECL_REPORT( EndOfStream,                       "Stromende"                                                                                                     );

/* ----- AST ----- */

DECL_REPORT( IllegalTypeOfFuncObj,              "unerlaubter Typbezeichner des Funktionsobjekts '{0}'"                                                          );
DECL_REPORT( CantDirectlyAccessMembersOf,       "es kann nicht direkt auf Elemente von '{0}' zugegriffen werden"                                                );
DECL_REPORT( CantDirectlyAccessArrayOf,         "es kann nicht direkt auf Felder von '{0}' zugegriffen werden"                                                  );
DECL_REPORT( UnknownTypeOfVarIdentSymbolRef,    "unbekannter Typ der Symbolreferenz des Namens '{0}' um Typbezeichner abzuleiten"                               );
DECL_REPORT( InvalidSubscriptBaseType,          "ungültiger basis Typbezeichner für Vektor Index"                                                               );
DECL_REPORT( MissingVarIdentSymbolRef,          "fehlende Symbolreferenz des Namens '{0}' um Typbezeichner abzuleiten"                                          );
DECL_REPORT( MissingFuncRefToDeriveExprType,    "fehlende Funktionsreferenz um Ausdrucks-Typbezeichner abzuleiten"                                              );
DECL_REPORT( MissingDeclStmntRefToDeriveType,   "fehlende Referenz zur Deklarationsangabe des Names '{0}' um Typbezeichner abzuleiten"                          );
DECL_REPORT( FuncDoesntTake1Param,              "Funktion '{0}' nimmt keine {1} Parameter"                                                                      );
DECL_REPORT( FuncDoesntTakeNParams,             "Funktion '{0}' nimmt keine {1} Parameter"                                                                      );
DECL_REPORT( TernaryExpr,                       "Ternärausdruck"                                                                                                );
DECL_REPORT( BinaryExpr,                        "Binärausdruck '{0}'"                                                                                           );
DECL_REPORT( CastExpr,                          "Besetzungsausdruck"                                                                                            );
DECL_REPORT( InitializerExpr,                   "Initialisierungsausdruck"                                                                                      );
DECL_REPORT( ConditionOfTernaryExpr,            "Bedingung des Ternärausdrucks"                                                                                 );
DECL_REPORT( CantDeriveTypeOfEmptyInitializer,  "es kann kein Typ für Initialisierungsliste ohne Elemente ermittelnt werden"                                    );
DECL_REPORT( ArrayDimMismatchInInitializer,     "Diskrepanz zwischen Felddimensionen in Initialisierungsausdruck ({0} Dimension(en) erwartet, aber erhielt {1})"    );
DECL_REPORT( ArrayDimSizeMismatchInInitializer, "Diskrepanz zwischen Felddimensionsgrößen in Initialisierungsausdruck ({0} Element(e) erwartet, aber erhielt {1})"  );
DECL_REPORT( TypeMismatchInInitializer,         "Diskrepanz zwischen Typ in Initialisierungsausdruck (Feld '{0}' erwartet, aber erhielt '{1}')"                 );
DECL_REPORT( ExpectedInitializerForArrayAccess, "Initialisierungsausdruck für Feldzugriff erwartet"                                                             );
DECL_REPORT( NotEnoughElementsInInitializer,    "nicht genug Elemente in Initialisierungsausdruck"                                                              );
DECL_REPORT( NotEnoughIndicesForInitializer,    "nicht genug Feldindizes für Initialisierungsausdruck angegeben"                                                );

/* ----- ASTEnums ----- */

DECL_REPORT( DataType,                          "Datentyp"                                                                                                      );
DECL_REPORT( SamplerType,                       "Sampler-Typ"                                                                                                   );
DECL_REPORT( BufferType,                        "Buffer-Typ"                                                                                                    );
DECL_REPORT( Intrinsic,                         "Intrinsic[ '{0}']"                                                                                             );
DECL_REPORT( DomainType,                        "Domain-Typ"                                                                                                    );
DECL_REPORT( PrimitiveType,                     "Primitiv-Typ"                                                                                                  );
DECL_REPORT( Partitioning,                      "Partitionierungs-Typ"                                                                                          );
DECL_REPORT( OutputToplogy,                     "Ausgabetopologie-Typ"                                                                                          );
DECL_REPORT( Attributes,                        "Attribute"                                                                                                     );
DECL_REPORT( Undefined,                         "<undefiniert>"                                                                                                 );
DECL_REPORT( UserDefined,                       "<Nutzer-definiert>"                                                                                            );
DECL_REPORT( FailedToMap,                       "Abbildung von {0} auf {1} fehlgeschlagen"                                                                      );
DECL_REPORT( VectorSubscriptCantHaveNComps,     "Vektor Index kann keine {0} Komponenten haben"                                                                 );
DECL_REPORT( IncompleteMatrixSubscript,         "unvollständiger Matrix Index: '{0}'"                                                                           );
DECL_REPORT( InvalidVectorDimension,            "ungültige Vektor Dimension (muss im Intervall \\[1, 4\\] liegen, aber erhielt {0})"                            );
DECL_REPORT( InvalidVectorSubscript,            "ungültiger Vektor Index '{0}' für {1}"                                                                         );
DECL_REPORT( InvalidMatrixDimension,            "ungültige Matrix Dimension (muss im Intervall \\[1, 4\\] x \\[1, 4\\] liegen, aber erhielt {0} x {1})"         );
DECL_REPORT( InvalidCharInMatrixSubscript,      "ungültiges Zeichen '{0}' in [{2}-basiertem ]Matrix Index: '{1}'"                                               );
DECL_REPORT( InvalidIntrinsicArgType,           "ungültiger Argument Typbezeichner für Intrinsic[ '{0}']"                                                       );
DECL_REPORT( InvalidIntrinsicArgCount,          "ungültige Anzahl Argumente für Intrinsic[ '{0}'][ ({1} erwartet, aber erhielt {2})]"                           );
DECL_REPORT( InvalidIntrinsicArgs,              "ungültige Arguments für Intrinsic[ '{0}']"                                                                     );

/* ----- TypeDenoter ------ */

DECL_REPORT( VarIdentCantBeResolved,            "Variablenname kann nicht aufgelöst werden"                                                                     );
DECL_REPORT( IllegalArrayAccess,                "Feldzugriff nicht erlaubt[ für '{0}']"                                                                         );
DECL_REPORT( TooManyArrayDimensions,            "Zu viele Felddimensionen[ für '{0}']"                                                                          );
DECL_REPORT( MissingRefToStructDecl,            "fehlende Referenz zu Struktur Deklaration[ '{0}']"                                                             );
DECL_REPORT( MissingRefToAliasDecl,             "fehlende Referenz zu Pseudonym Deklaration[ '{0}']"                                                            );
DECL_REPORT( MissingBaseTypeInArray,            "fehlender basis Typ in Feld Typbezeichner"                                                                     );
DECL_REPORT( MissingRefInTypeDen,               "fehlende Referenz zu Deklaration[ in {0}]"                                                                     );

/* ----- SymbolTable ----- */

DECL_REPORT( UndefinedSymbol,                   "undefiniertese Symbol '{0}'"                                                                                   );
DECL_REPORT( AmbiguousSymbol,                   "Symbol '{0}' ist mehrdeutig"                                                                                   );
DECL_REPORT( AmbiguousFuncCall,                 "mehrdeutiger Funktionsaufruf '{0}({1})'"                                                                       );
DECL_REPORT( AmbiguousIntrinsicCall,            "mehrdeutiger Intrinsic Aufruf[ '{0}']"                                                                         );
DECL_REPORT( IdentIsNotFunc,                    "Name '{0}' bezeichnet keine Funktion"                                                                          );
DECL_REPORT( IdentIsNotVar,                     "Name '{0}' bezeichnet keine Variable"                                                                          );
DECL_REPORT( IdentIsNotType,                    "Name '{0}' bezeichnet keinen Typ"                                                                              );
DECL_REPORT( IdentIsNotVarOrBufferOrSampler,    "Name '{0}' bezeichnet keine Variable, Buffer oder Sampler"                                                     );
DECL_REPORT( IdentAlreadyDeclared,              "Name '{0}' ist in diesem Bereich bereits deklariert"                                                           );
DECL_REPORT( NoActiveScopeToRegisterSymbol,     "kein aktiver Bereich um Symbol zu registrieren"                                                                );

/* ----- Scanner ----- */

DECL_REPORT( LexicalError,                      "lexikalischer Fehler"                                                                                          );
DECL_REPORT( UnexpectedChar,                    "unerwartetes Zeichen '{0}'[ ('{1}' erwartet)]"                                                                 );
DECL_REPORT( MissingDigitSequenceAfterExpr,     "fehlende Ziffersequenz nach Exponentanteil"                                                                    );
DECL_REPORT( MissingDecimalPartInFloat,         "fehlender Dezimalanteil in fließkomma Zahl"                                                                    );

/* ----- Parser ----- */

DECL_REPORT( UnexpectedToken,                   "unerwartetes Zeichen[: {0}[ ({1})]]"                                                                           );
DECL_REPORT( UnexpectedEndOfStream,             "unerwarteter Stromende"                                                                                        );
DECL_REPORT( UnexpectedTokenSpell,              "unerwartete Zeichenschreibweise '{0}'[ ('{1}' erwartet)]"                                                      );
DECL_REPORT( Expected,                          "erwartet"                                                                                                      );
DECL_REPORT( ExpectedPrimaryExpr,               "Primärausdruck erwartet"                                                                                       );
DECL_REPORT( ExpectedLiteralExpr,               "Literalausdruck erwartet"                                                                                      );
DECL_REPORT( ExpectedTypeDen,                   "Typbezeichner erwartet"                                                                                        );
DECL_REPORT( ExpectedBaseTypeDen,               "basis Typbezeichner erwartet"                                                                                  );
DECL_REPORT( InFunction,                        " (in Funktion: {0})"                                                                                           );
DECL_REPORT( FailedToCreateScanner,             "Erstellen des Zeichenlesers fehlgeschlagen"                                                                    );
DECL_REPORT( FailedToScanSource,                "Einlesen des Quellcodes fehlgeschlagen"                                                                        );
DECL_REPORT( MissingScanner,                    "Zeichenleser fehlt"                                                                                            );
DECL_REPORT( SubExprMustNotBeEmpty,             "Teilausdrücke dürfen nicht leer sein"                                                                          );
DECL_REPORT( SubExprAndOpsUncorrelated,         "Teilausdrücke und Operatoren haven unkorrelierte Anzahl an Elementen"                                          );
DECL_REPORT( TooManySyntaxErrors,               "zu viele Syntaxfehler"                                                                                         );
DECL_REPORT( IdentNameManglingConflict,         "Name '{0}' konfligiert mit reserviertem Name-Mangling-Vorzeichens '{1}'"                                       );
DECL_REPORT( NotAllowedInThisContext,           "{0} in diesem Kontext nicht erlaubt"                                                                           );

/* ----- PreProcessor ----- */

DECL_REPORT( UnknownPPDirective,                "unbekannte Präprozessor Direktive: \"{0}\""                                                                    );
DECL_REPORT( UnknownMatrixPackAlignment,        "unbekannte Matrix Packungsanordnung: \"{0}\" (muss \"row_major\" oder \"column_major\" sein)"                  );
DECL_REPORT( UnknownPragma,                     "unbekanntes Pragma: \"{0}\""                                                                                   );
DECL_REPORT( InvalidMacroIdentTokenArg,         "ungültige Arguments für Makro Namenszeichen"                                                                   );
DECL_REPORT( FailedToUndefMacro,                "Auflösen der Makrodefinition \"{0}\" fehlgeschlagen"                                                           );
DECL_REPORT( MacroRedef,                        "Neudefinition des Makros \"{0}\"[ {1}]"                                                                        );
DECL_REPORT( PrevDefinitionAt,                  "vorherige Definition bei ({0})"                                                                                );
DECL_REPORT( WithMismatchInParamListAndBody,    "mit Diskrepanz in der Parameterlist und Rumpfdefinition"                                                       );
DECL_REPORT( WithMismatchInParamList,           "mit Diskrepanz in der Parameterlist"                                                                           );
DECL_REPORT( WithMismatchInBody,                "mit Diskrepanz in der Rumpfdefinition"                                                                         );
DECL_REPORT( MissingIfDirective,                "fehlende '#if'-Direktive zu schließendem '#endif', '#else' oder '#elif'"                                       );
DECL_REPORT( MissingEndIfDirective,             "missing '#endif'-directive for open '#if', '#ifdef', or '#ifndef'"                                             );
DECL_REPORT( TooManyArgsForMacro,               "zu viele Argumente für Makro \"{0}\"[ ({1} erwartet, aber erhielt {2})]"                                       );
DECL_REPORT( TooFewArgsForMacro,                "zu wenig Argumente für Makro \"{0}\"[ ({1} erwartet, aber erhielt {2})]"                                       );
DECL_REPORT( ExpectedEndIfDirective,            "'#endif'-Direktive nach vorherigem '#else' erwartet[, aber erhielt '{0}']"                                     );
DECL_REPORT( PragmaCantBeHandled,               "Pragma \"{0}\" kann im Moment nicht behandelt werden"                                                          );
DECL_REPORT( UnexpectedTokenInPragma,           "unerwartetes Zeichen in '#pragam'-Direktive"                                                                   );
DECL_REPORT( UnexpectedEndOfTokenString,        "unerwartetes Ende der Zeichenkette"                                                                            );
DECL_REPORT( RemainingTokensInPragma,           "verbleibende unbehandelte Zeichen in '#pragma'-Direktive"                                                      );
DECL_REPORT( EmptyPragma,                       "leere '#pragma'-Direktive"                                                                                     );

/* ----- Visitor ----- */

DECL_REPORT( FuncDeclStackUnderflow,            "function declaration stack underflow"                                                                          ); // internal error
DECL_REPORT( FuncCallStackUnderflow,            "function call stack underflow"                                                                                 ); // internal error
DECL_REPORT( StructDeclStackUnderflow,          "structure declaration stack underflow"                                                                         );
DECL_REPORT( UniformBufferDeclStackUnderflow,   "uniform buffer declaration stack underflow" );

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

/* ----- ConstExprEvaluator ----- */

DECL_REPORT( ExprEvaluator,                     "expression evaluator"                                                                                          );
DECL_REPORT( IllegalExprInConstExpr,            "illegal {0} in constant expression"                                                                            );
DECL_REPORT( DynamicArrayDim,                   "dynamic array dimension"                                                                                       );
DECL_REPORT( BoolLiteralValue,                  "boolean literal value '{0}'"                                                                                   );
DECL_REPORT( LiteralType,                       "literal type '{0}'"                                                                                            );
DECL_REPORT( TypeSpecifier,                     "type specifier"                                                                                                );
DECL_REPORT( DivisionByZero,                    "division by zero"                                                                                              );
DECL_REPORT( TypeCast,                          "type cast '{0}'"                                                                                               );
DECL_REPORT( InitializerList,                   "initializer list"                                                                                              );

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

/* ----- GLSLConverter ----- */

DECL_REPORT( SelfParamLevelUnderflow,           "'self'-parameter level underflow"                                                                              );
DECL_REPORT( MissingSelfParamForMemberFunc,     "missing 'self'-parameter for member function: {0}"                                                             );

/* ----- GLSLExtensionAgent ----- */

DECL_REPORT( GLSLExtensionOrVersionRequired,    "GLSL extension '{0}' or shader output version '{1}' required"                                                  );
DECL_REPORT( NoGLSLExtensionVersionRegisterd,   "no GLSL version is registered for the extension '{0}'"                                                         );

/* ----- GLSLGenerator ----- */

DECL_REPORT( EntryPointNotFound,                "entry point \"{0}\" not found"                                                                                 );
DECL_REPORT( FailedToMapToGLSLKeyword,          "failed to map {0} to GLSL keyword[ ({1})]"                                                                     );
DECL_REPORT( FailedToWriteLiteralType,          "failed to write type denoter for literal[ '{0}']"                                                              );
DECL_REPORT( FailedToDetermineGLSLDataType,     "failed to determine GLSL data type" );
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

/* ----- GLSLPreProcessor ----- */

DECL_REPORT( MacrosBeginWithGLReserved,         "macros beginning with 'GL_' are reserved[: {0}]"                                                               );
DECL_REPORT( MacrosWithTwoUnderscoresReserved,  "macros containing consecutive underscores '__' are reserved[: {0}]"                                            );
DECL_REPORT( IllegalRedefOfStdMacro,            "illegal redefinition of standard macro[: {0}]"                                                                 );
DECL_REPORT( IllegalUndefOfStdMacro,            "illegal undefinition of standard macro[: {0}]"                                                                 );
DECL_REPORT( VersionMustBeFirstDirective,       "'#version'-directive must be the first directive"                                                              );
DECL_REPORT( UnknwonGLSLVersion,                "unknown GLSL version: '{0}'"                                                                                    );
DECL_REPORT( NoProfileForGLSLVersionBefore150,  "versions before 150 do not allow a profile token"                                                              );
DECL_REPORT( InvalidGLSLVersionProfile,         "invalid version profile '{0}' (must be 'core' or 'compatibility')"                                             );
DECL_REPORT( ExtensionNotSupported,             "extension not supported[: {0}]"                                                                                );
DECL_REPORT( InvalidGLSLExtensionBehavior,      "invalid extension behavior '{0}' (must be 'enable', 'require', 'warn', or 'disable')"                          );

/* ----- HLSLIntrinsics ----- */

DECL_REPORT( FailedToDeriveIntrinsicType,       "failed to derive type denoter for intrinsic[ '{0}']"                                                           );
DECL_REPORT( FailedToDeriveIntrinsicParamType,  "failed to derive parameter type denoter for intrinsic[ '{0}']"                                                 );

/* ----- HLSLKeywords ----- */

DECL_REPORT( FailedToMapFromHLSLKeyword,        "failed to map HLSL keyword '{0}' to {1}"                                                                       );
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
DECL_REPORT( ExpectedUnaryExprOp,               "expected unary expression operator"                                                                            );
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
DECL_REPORT( RegisterIgnoredForVarDecls,        "register is ignore for variable declarations"                                                                  );
DECL_REPORT( RegisterIgnoredForFuncDecls,       "register is ignore for function declarations"                                                                  );
DECL_REPORT( ExpectedSamplerOrSamplerState,     "expected sampler type denoter or sampler state"                                                                );
DECL_REPORT( ExpectedOpenBracketOrAngleBracket, "expected '<' or '('"                                                                                           );
DECL_REPORT( DuplicatedPrimitiveType,           "duplicate primitive type specified"                                                                            );
DECL_REPORT( ConflictingPrimitiveTypes,         "conflicting primitive types"                                                                                   );

/* ----- HLSLAnalyzer ----- */

DECL_REPORT( SecondEntryPointNotFound,          "secondary entry point \"{0}\" not found"                                                                       );
DECL_REPORT( VariableOverridesMemberOfBase,     "member variable '{0}' overrides member of base '{1}'"                                                          );
DECL_REPORT( NestedStructsMustBeAnonymous,      "nested structures must be anonymous"                                                                           );
DECL_REPORT( IsCompletelyEmpty,                 "'{0}' is completely empty"                                                                                     );
DECL_REPORT( BufferCanOnlyHaveOneSlot,          "buffers can only be bound to one slot"                                                                         );
DECL_REPORT( UserCBuffersCantBeTargetSpecific,  "user-defined constant buffer slots can not be target specific"                                                 );
DECL_REPORT( DeclShadowsPreviousLocal,          "declaration of '{0}' shadows a previous local at ({1})"                                                        );
DECL_REPORT( ReturnOutsideFuncDecl,             "return statement outside function declaration"                                                                 );
DECL_REPORT( VarAssignment,                     "variable assignment"                                                                                           );
DECL_REPORT( VarInitialization,                 "variable initialization"                                                                                       );
DECL_REPORT( IntrinsicNotDeclaredInObject,      "intrinsic '{0}' not declared in object '{1}'"                                                                  );
DECL_REPORT( InvalidShaderModelForIntrinsic,    "intrinsic '{0}' requires shader model {1}, but only {2} is specified"                                          );
DECL_REPORT( InvalidIntrinsicForTexture,        "invalid intrinsic '{0}' for texture object"                                                                    );
DECL_REPORT( InvalidIntrinsicForStorageBuffer,  "invalid intrinsic '{0}' for storage-buffer object"                                                             );
DECL_REPORT( InvalidIntrinsicForStreamOutput,   "invalid intrinsic '{0}' for stream-output object"                                                              );
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
DECL_REPORT( DuplicateUseOfOutputSemantic,      "duplicate use of output semantic '{0}'"                                                                        );
DECL_REPORT( UniformCantBeOutput,               "uniforms can not be defined as output"                                                                         );
DECL_REPORT( TooManyArgsForAttribute,           "too many arguments for attribute[ '{0}'][ (expected {1}, but got {2})]"                                        );
DECL_REPORT( TooFewArgsForAttribute,            "too few arguments for attribute[ '{0}'][ (expected {1}, but got {2})]"                                         );
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

/* ----- Xsc ----- */

DECL_REPORT( InputStreamCantBeNull,             "input stream must not be null"                                                                                 );
DECL_REPORT( OutputStreamCantBeNull,            "output stream must not be null"                                                                                );
DECL_REPORT( NameManglingPrefixResCantBeEmpty,  "name mangling prefix for reserved words must not be empty"                                                     );
DECL_REPORT( NameManglingPrefixTmpCantBeEmpty,  "name mangling prefix for temporary variables must not be empty"                                                );
DECL_REPORT( NameManglingPrefixOverlap,         "name mangling prefix for reserved words and temporary variables must not be equal to any other prefix"         );
DECL_REPORT( PreProcessingSourceFailed,         "preprocessing input code failed"                                                                               );
DECL_REPORT( ParsingSourceFailed,               "parsing input code failed"                                                                                     );
DECL_REPORT( AnalyzingSourceFailed,             "analyzing input code failed"                                                                                   );
DECL_REPORT( GeneratingOutputCodeFailed,        "generating output code failed"                                                                                 );


#endif



// ================================================================================