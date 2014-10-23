/*
 * FuncUseAnalyzer.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_FUNC_USE_ANALYZER_H__
#define __HT_FUNC_USE_ANALYZER_H__


#include "Visitor.h"
#include "Token.h"
#include "SymbolTable.h"


namespace HTLib
{


//! AST symbol table type.
typedef SymbolTable<AST> ASTSymbolTable;

/**
Function usage analyzer.
This helper class for the context analyzer marks all functions
which are used from the beginning of the shader entry point.
All other functions will be removed from the code generation.
*/
class FuncUseAnalyzer : private Visitor
{
    
    public:
        
        FuncUseAnalyzer(const ASTSymbolTable& symTable);

        void MarkFunctionsFromEntryPoint(FunctionDecl* ast);

    private:
        
        typedef SymbolTable<AST>::OnOverrideProc OnOverrideProc;

        /* === Visitor implementation === */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( SwitchCase        );

        DECL_VISIT_PROC( FunctionDecl      );

        DECL_VISIT_PROC( CodeBlockStmnt    );
        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( VarDeclStmnt      );
        DECL_VISIT_PROC( AssignSmnt        );
        DECL_VISIT_PROC( FunctionCallStmnt );
        DECL_VISIT_PROC( ReturnStmnt       );

        DECL_VISIT_PROC( ListExpr          );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( FunctionCallExpr  );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( VarAccessExpr     );
        DECL_VISIT_PROC( InitializerExpr   );

        DECL_VISIT_PROC( VarIdent          );
        DECL_VISIT_PROC( VarDecl           );

        /* === Members === */

        const ASTSymbolTable* symTable_ = nullptr;

};


} // /namespace HTLib


#endif



// ================================================================================