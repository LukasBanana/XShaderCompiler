/*
 * ConstExprEvaluator.h
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
class ConstExprEvaluator : private Visitor
{
    
    public:
        
        using OnVarAccessCallback = std::function<Variant(VarAccessExpr* ast)>;

        /*
        Evaluates the specified expression and returns the result as variante.
        Throws an std::runtime_error if the expression could not be evaluated.
        */
        Variant EvaluateExpr(Expr& ast, const OnVarAccessCallback& onVarAccessCallback = nullptr);

    private:
        
        /* === Functions === */

        void Push(const Variant& v);
        Variant Pop();

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( NullExpr         );
        DECL_VISIT_PROC( ListExpr         );
        DECL_VISIT_PROC( LiteralExpr      );
        DECL_VISIT_PROC( TypeNameExpr     );
        DECL_VISIT_PROC( TernaryExpr      );
        DECL_VISIT_PROC( BinaryExpr       );
        DECL_VISIT_PROC( UnaryExpr        );
        DECL_VISIT_PROC( PostUnaryExpr    );
        DECL_VISIT_PROC( FunctionCallExpr );
        DECL_VISIT_PROC( BracketExpr      );
        DECL_VISIT_PROC( SuffixExpr       );
        DECL_VISIT_PROC( ArrayAccessExpr  );
        DECL_VISIT_PROC( CastExpr         );
        DECL_VISIT_PROC( VarAccessExpr    );
        DECL_VISIT_PROC( InitializerExpr  );

        /* === Members === */

        std::stack<Variant> variantStack_;

        OnVarAccessCallback onVarAccessCallback_;

};


} // /namespace Xsc


#endif



// ================================================================================