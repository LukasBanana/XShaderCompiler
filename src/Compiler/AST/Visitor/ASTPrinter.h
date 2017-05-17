/*
 * ASTPrinter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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
        
        void PrintAST(Program* program, Log& log);

    private:
        
        struct PrintableTree
        {
            std::string                 row;        // Source position row as string.
            std::string                 col;        // Source position column as string.
            std::string                 label;      // AST description label.
            std::vector<PrintableTree>  children;   // Child nodes.
        };

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( Attribute         );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( SamplerValue      );
        DECL_VISIT_PROC( Register          );
        DECL_VISIT_PROC( PackOffset        );
        DECL_VISIT_PROC( ArrayDimension    );
        DECL_VISIT_PROC( TypeSpecifier     );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( BufferDecl        );
        DECL_VISIT_PROC( SamplerDecl       );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( AliasDecl         );
        DECL_VISIT_PROC( FunctionDecl      );

        DECL_VISIT_PROC( VarDeclStmnt      );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( SamplerDeclStmnt  );
        DECL_VISIT_PROC( AliasDeclStmnt    );
        DECL_VISIT_PROC( BasicDeclStmnt    );

        DECL_VISIT_PROC( NullStmnt         );
        DECL_VISIT_PROC( CodeBlockStmnt    );
        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( ReturnStmnt       );
        DECL_VISIT_PROC( CtrlTransferStmnt );

        DECL_VISIT_PROC( NullExpr          );
        DECL_VISIT_PROC( SequenceExpr      );
        DECL_VISIT_PROC( LiteralExpr       );
        DECL_VISIT_PROC( TypeSpecifierExpr );
        DECL_VISIT_PROC( TernaryExpr       );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( CallExpr          );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( ObjectExpr        );
        DECL_VISIT_PROC( AssignExpr        );
        DECL_VISIT_PROC( ArrayExpr         );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( InitializerExpr   );

        /* --- Helper functions --- */

        std::string WriteLabel(AST* ast, const std::string& astName, const std::string& info = "");
        void Print(Log& log, const PrintableTree& tree);

        bool PushPrintable(const SourcePosition& pos, const std::string& label);
        void PopPrintable();

        PrintableTree* TopPrintable();

        /* === Members === */

        PrintableTree               treeRoot_;
        std::stack<PrintableTree*>  parentNodeStack_;

        std::vector<bool>           lastSubNodeStack_;

        std::size_t                 maxRowStrLen_       = 0,
                                    maxColStrLen_       = 0;

};


} // /namespace Xsc


#endif



// ================================================================================