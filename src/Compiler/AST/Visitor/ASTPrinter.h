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
#include <iostream>


namespace Xsc
{


// AST debug printer.
class ASTPrinter : private Visitor
{
    
    public:
        
        void PrintAST(Program* program, std::ostream& output = std::cout);

    private:
        
        struct PrintableTree
        {
            std::string                 row;        // Source position row as string.
            std::string                 col;        // Source position column as string.
            std::string                 label;      // AST description label.
            std::string                 value;      // AST description value.
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
        DECL_VISIT_PROC( LayoutStmnt       );

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
        
        template <typename T>
        void VisitMember(T ast, const std::string& name)
        {
            if (ast)
            {
                PushMemberName(name);
                ast->Visit(this, nullptr);
                PopMemberName();
            }
        }

        template <typename T>
        void VisitMember(const std::vector<T>& astList, const std::string& name)
        {
            for (std::size_t i = 0; i < astList.size(); ++i)
                VisitMember(astList[i], name + "[" + std::to_string(i) + "]");
        }

        std::string WriteLabel(const std::string& astName, TypedAST* ast = nullptr);

        void Print(const PrintableTree& tree, std::ostream& output);

        bool PushPrintable(const AST* ast, const std::string& label, const std::string& value = "");
        void PopPrintable();

        void Printable(const AST* ast, const std::string& label, const std::string& value = "");

        PrintableTree* TopPrintable();

        void PushMemberName(const std::string& name);
        void PopMemberName();

        const std::string& TopMemberName() const;

        /* === Members === */

        PrintableTree               treeRoot_;
        std::stack<PrintableTree*>  parentNodeStack_;
        std::vector<bool>           lastSubNodeStack_;
        std::stack<std::string>     memberNameStack_;

        std::size_t                 maxRowStrLen_       = 0,
                                    maxColStrLen_       = 0;

};


} // /namespace Xsc


#endif



// ================================================================================