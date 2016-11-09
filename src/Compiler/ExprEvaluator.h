/*
 * ExprEvaluator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_EXPR_EVALUATOR_H
#define XSC_EXPR_EVALUATOR_H


#include "Visitor.h"
#include "Variant.h"
#include <stack>


namespace Xsc
{


// Expression evaluator AST visitor.
class ExprEvaluator : private Visitor
{
    
    public:
        
        Variant EvaluateExpr(Expr& ast);

    private:
        
        /* === Functions === */

        void Push(const Variant& v);
        Variant Pop();

        /* --- Visitor implementation --- */

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

        /* === Members === */

        std::stack<Variant> variantStack_;

};


} // /namespace Xsc


#endif



// ================================================================================