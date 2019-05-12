/*
 * ExprEvaluator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CONST_EXPR_EVALUATOR_H
#define XSC_CONST_EXPR_EVALUATOR_H


#include "Visitor.h"
#include "Variant.h"
#include "Flags.h"
#include <stack>
#include <functional>


namespace Xsc
{


// Constant expression evaluator AST visitor.
class ExprEvaluator : private Visitor
{

    public:

        // Evaluation flags enumeration.
        enum : unsigned int
        {
            // Evaluates only the sub expressions of a BinaryExpr that are necessary (especially for logical AND and OR).
            EvaluateReducedBinaryExpr = (1 << 0),
        };

        ExprEvaluator(Flags flags = 0);

        using OnObjectExprCallback = std::function<Variant(IdentExpr* expr)>;

        // Evaluates the specified expression and returns the result as variant, or throws a runtime error on failure.
        Variant Evaluate(Expr& expr, const OnObjectExprCallback& onObjectExprCallback = nullptr);

        // Evaluates the specified expression and returns the result as variant, or returns the default value on failure.
        Variant EvaluateOrDefault(Expr& expr, const Variant& defaultValue = {}, const OnObjectExprCallback& onObjectExprCallback = nullptr);

        // Abort expression evaluation process.
        void Abort();

    private:

        void Push(const Variant& v);
        Variant Pop();

        void SetObjectExprCallback(const OnObjectExprCallback& callback);

        Variant EvaluateBinaryOp(const BinaryExpr* ast, Variant lhs, Variant rhs);
        Variant EvaluateUnaryOp(const UnaryExpr* ast, Variant rhs);

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( NullExpr        );
        DECL_VISIT_PROC( SequenceExpr    );
        DECL_VISIT_PROC( LiteralExpr     );
        DECL_VISIT_PROC( TernaryExpr     );
        DECL_VISIT_PROC( BinaryExpr      );
        DECL_VISIT_PROC( UnaryExpr       );
        DECL_VISIT_PROC( CallExpr        );
        DECL_VISIT_PROC( BracketExpr     );
        DECL_VISIT_PROC( AssignExpr      );
        DECL_VISIT_PROC( IdentExpr       );
        DECL_VISIT_PROC( SubscriptExpr   );
        DECL_VISIT_PROC( CastExpr        );
        DECL_VISIT_PROC( InitializerExpr );

    private:

        std::stack<Variant>     variantStack_;

        OnObjectExprCallback    onObjectExprCallback_;

        bool                    throwOnFailure_         = true;
        bool                    abort_                  = false;

        Flags                   flags_;

};


} // /namespace Xsc


#endif



// ================================================================================