/*
 * ExprEvaluator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CONST_EXPR_EVALUATOR_H
#define XSC_CONST_EXPR_EVALUATOR_H


#include "Visitor.h"
#include "Variant.h"
#include <stack>
#include <functional>


namespace Xsc
{


// Constant expression evaluator AST visitor.
class ExprEvaluator : private Visitor
{
    
    public:
        
        using OnObjectExprCallback = std::function<Variant(ObjectExpr* expr)>;

        /*
        Evaluates the specified expression and returns the result as variante.
        Throws an std::runtime_error if the expression could not be evaluated.
        */
        Variant Evaluate(Expr& ast, const OnObjectExprCallback& onObjectExprCallback = nullptr);

    private:
        
        /* === Functions === */

        void Push(const Variant& v);
        Variant Pop();

        /* --- Visitor implementation --- */

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
        DECL_VISIT_PROC( AssignExpr        );
        DECL_VISIT_PROC( ObjectExpr        );
        DECL_VISIT_PROC( ArrayExpr         );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( InitializerExpr   );

        /* === Members === */

        std::stack<Variant>     variantStack_;

        OnObjectExprCallback    onObjectExprCallback_;

};


} // /namespace Xsc


#endif



// ================================================================================