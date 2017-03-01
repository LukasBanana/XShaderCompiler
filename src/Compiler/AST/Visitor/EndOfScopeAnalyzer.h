/*
 * EndOfScopeAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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

        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( ReturnStmnt       );
        DECL_VISIT_PROC( CtrlTransferStmnt );

};


} // /namespace Xsc


#endif



// ================================================================================