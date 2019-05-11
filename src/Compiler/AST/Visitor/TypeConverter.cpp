/*
 * TypeConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TypeConverter.h"
#include "AST.h"


namespace Xsc
{


void TypeConverter::Convert(Program& program, const OnVisitVarDecl& onVisitVarDecl)
{
    if (onVisitVarDecl)
    {
        onVisitVarDecl_ = onVisitVarDecl;
        Visit(&program);
    }
}


/*
 * ======= Private: =======
 */

void TypeConverter::ConvertExprType(Expr* expr)
{
    if (expr && resetExprTypes_)
        expr->ResetTypeDenoter();
}

void TypeConverter::ConvertExpr(const ExprPtr& expr)
{
    if (expr)
    {
        /* Visit expression */
        Visit(expr);

        /* Check if type must be reset */
        if (resetExprTypes_)
        {
            expr->ResetTypeDenoter();
            resetExprTypes_ = false;
        }
    }
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void TypeConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    VISIT_DEFAULT(VarDecl);
    if (onVisitVarDecl_(*ast))
        convertedSymbols_.insert(ast);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(ForLoopStmt)
{
    VISIT_DEFAULT(ForLoopStmt);
    ConvertExpr(ast->condition);
    ConvertExpr(ast->iteration);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmt)
{
    VISIT_DEFAULT(WhileLoopStmt);
    ConvertExpr(ast->condition);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmt)
{
    VISIT_DEFAULT(DoWhileLoopStmt);
    ConvertExpr(ast->condition);
}

IMPLEMENT_VISIT_PROC(IfStmt)
{
    VISIT_DEFAULT(IfStmt);
    ConvertExpr(ast->condition);
}

IMPLEMENT_VISIT_PROC(SwitchStmt)
{
    VISIT_DEFAULT(SwitchStmt);
    ConvertExpr(ast->selector);
}

IMPLEMENT_VISIT_PROC(ExprStmt)
{
    VISIT_DEFAULT(ExprStmt);
    ConvertExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(ReturnStmt)
{
    if (ast->expr)
    {
        VISIT_DEFAULT(ReturnStmt);
        ConvertExpr(ast->expr);
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    VISIT_DEFAULT(SequenceExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    VISIT_DEFAULT(TernaryExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    VISIT_DEFAULT(BinaryExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    VISIT_DEFAULT(UnaryExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    VISIT_DEFAULT(PostUnaryExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    VISIT_DEFAULT(CallExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    VISIT_DEFAULT(BracketExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    VISIT_DEFAULT(CastExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    VISIT_DEFAULT(ObjectExpr);

    if (auto symbol = ast->symbolRef)
    {
        /* Check if referenced symbol has a converted type */
        if (convertedSymbols_.find(symbol) != convertedSymbols_.end())
            resetExprTypes_ = true;
    }

    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    VISIT_DEFAULT(AssignExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    VISIT_DEFAULT(ArrayExpr);
    ConvertExprType(ast);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    VISIT_DEFAULT(InitializerExpr);
    ConvertExprType(ast);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================