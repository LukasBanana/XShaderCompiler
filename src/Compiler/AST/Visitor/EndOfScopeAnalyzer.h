/*
 * EndOfScopeAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_END_OF_SCOPE_ANALYZER_H
#define XSC_END_OF_SCOPE_ANALYZER_H


#include "Visitor.h"
#include "Token.h"
#include "SymbolTable.h"


namespace Xsc
{


/*
End-of-scope analyzer.
This helper class for the context analyzer marks all return statements
which are at the end of a scope (e.g. the 'isEndOfFunction' flag for the return statement).
*/
class EndOfScopeAnalyzer : private Visitor
{

    public:

        void MarkEndOfScopesFromFunction(FunctionDecl& funcDecl);

    private:

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock         );

        DECL_VISIT_PROC( ForLoopStmt      );
        DECL_VISIT_PROC( WhileLoopStmt    );
        DECL_VISIT_PROC( DoWhileLoopStmt  );
        DECL_VISIT_PROC( IfStmt           );
        DECL_VISIT_PROC( SwitchStmt       );
        DECL_VISIT_PROC( ExprStmt         );
        DECL_VISIT_PROC( ReturnStmt       );
        DECL_VISIT_PROC( CtrlTransferStmt );

};


} // /namespace Xsc


#endif



// ================================================================================