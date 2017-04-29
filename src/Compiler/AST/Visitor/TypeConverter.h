/*
 * TypeConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_TYPE_CONVERTER_H
#define XSC_TYPE_CONVERTER_H


#include "Visitor.h"
#include <functional>
#include <set>


namespace Xsc
{


/*
~~~~~~~~~~~~~ TODO: ~~~~~~~~~~~~~
Similar to "ExprConverter", all AST nodes that have sub expressions must be traversed manually,
to ensure the type denoters are reset all up the tree hierarchy, if required (i.e. as soon as a type has changed).
*/

// Helper class to update the type denoters of all 'TypedAST' nodes, whose type denoters have been reset.
class TypeConverter : public Visitor
{
    
    public:
        
        // Callback interface for each variable declaration, which returns true if its type has changed (i.e. type denoter has been reset).
        using OnVisitVarDecl = std::function<bool(VarDecl& varDecl)>;

        // Converts the type denoters in the specified AST.
        void Convert(Program& program, const OnVisitVarDecl& onVisitVarDecl);

    private:
        
        void ConvertExprType(Expr* expr);
        void ConvertExpr(const ExprPtr& expr);

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( VarDecl          );

        DECL_VISIT_PROC( ForLoopStmnt     );
        DECL_VISIT_PROC( WhileLoopStmnt   );
        DECL_VISIT_PROC( DoWhileLoopStmnt );
        DECL_VISIT_PROC( IfStmnt          );
        DECL_VISIT_PROC( SwitchStmnt      );
        DECL_VISIT_PROC( ExprStmnt        );
        DECL_VISIT_PROC( ReturnStmnt      );

        DECL_VISIT_PROC( SequenceExpr     );
        DECL_VISIT_PROC( TernaryExpr      );
        DECL_VISIT_PROC( BinaryExpr       );
        DECL_VISIT_PROC( UnaryExpr        );
        DECL_VISIT_PROC( PostUnaryExpr    );
        DECL_VISIT_PROC( CallExpr         );
        DECL_VISIT_PROC( BracketExpr      );
        DECL_VISIT_PROC( CastExpr         );
        DECL_VISIT_PROC( ObjectExpr       );
        DECL_VISIT_PROC( AssignExpr       );
        DECL_VISIT_PROC( ArrayExpr        );
        DECL_VISIT_PROC( InitializerExpr  );

        /* === Members === */

        OnVisitVarDecl  onVisitVarDecl_;

        bool            resetExprTypes_     = false;    // If true, all expression types must be reset.
        std::set<AST*>  convertedSymbols_;              // List of all symbols, whose type denoters have been reset.

};


} // /namespace Xsc


#endif



// ================================================================================