/*
 * ExprEvaluator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ExprEvaluator.h"
#include "AST.h"


namespace Xsc
{


Variant ExprEvaluator::EvaluateExpr(Expr& ast)
{
    Visit(&ast);
    return Pop();
}


/*
 * ======= Private: =======
 */

void ExprEvaluator::Push(const Variant& v)
{
    variantStack_.push(v);
}

Variant ExprEvaluator::Pop()
{
    if (variantStack_.empty())
        throw std::runtime_error("stack underflow in expression evaluator");
    auto v = variantStack_.top();
    variantStack_.pop();
    return v;
}

/* --- Expressions --- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ExprEvaluator::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Visit(ast->firstExpr);
    Visit(ast->nextExpr);
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
}

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Visit(ast->condition);
    Visit(ast->ifExpr);
    Visit(ast->elseExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Visit(ast->lhsExpr);
    Visit(ast->rhsExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    Visit(ast->call);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->typeExpr);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
