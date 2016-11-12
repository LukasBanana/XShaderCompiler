/*
 * ASTPrinter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_PRINTER_H
#define XSC_AST_PRINTER_H


#include <Xsc/Xsc.h>
#include "CodeWriter.h"
#include "Visitor.h"
#include "Token.h"

#include <map>
#include <vector>


namespace Xsc
{


// AST debug printer.
class ASTPrinter : private Visitor
{
    
    public:
        
        void DumpAST(Program* program, Log& log);

    private:
        
        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( BufferDeclIdent   );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( Structure         );
        DECL_VISIT_PROC( SwitchCase        );

        DECL_VISIT_PROC( PackOffset        );
        DECL_VISIT_PROC( VarSemantic       );
        DECL_VISIT_PROC( VarType           );
        DECL_VISIT_PROC( VarIdent          );
        DECL_VISIT_PROC( VarDecl           );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( BufferDeclStmnt        );
        DECL_VISIT_PROC( TextureDecl       );
        DECL_VISIT_PROC( SamplerDecl       );
        DECL_VISIT_PROC( StructDeclStmnt   );

        DECL_VISIT_PROC( NullStmnt         );
        DECL_VISIT_PROC( CodeBlockStmnt    );
        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( VarDeclStmnt      );
        DECL_VISIT_PROC( AssignStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( FunctionCallStmnt );
        DECL_VISIT_PROC( ReturnStmnt       );
        DECL_VISIT_PROC( CtrlTransferStmnt );

        DECL_VISIT_PROC( ListExpr          );
        DECL_VISIT_PROC( LiteralExpr       );
        DECL_VISIT_PROC( TypeNameExpr      );
        DECL_VISIT_PROC( TernaryExpr       );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( FunctionCallExpr  );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( VarAccessExpr     );
        DECL_VISIT_PROC( InitializerExpr   );

        /* --- Helper functions --- */

        void Print(AST* ast, const std::string& astName, const std::string& info = "");

        void IncIndent();
        void DecIndent();

        /* === Members === */

        Log* log_ = nullptr;

};


} // /namespace Xsc


#endif



// ================================================================================