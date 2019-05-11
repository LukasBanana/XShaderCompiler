/*
 * Optimizer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Optimizer.h"
#include "ExprEvaluator.h"
#include "ASTFactory.h"
#include "AST.h"


namespace Xsc
{


void Optimizer::Optimize(Program& program)
{
    Visit(&program);
}


/*
 * ======= Private: =======
 */

void Optimizer::OptimizeStmtList(std::vector<StmtPtr>& stmts)
{
    /* Remove null statements */
    for (auto it = stmts.begin(); it != stmts.end();)
    {
        if (CanRemoveStmt(**it))
            it = stmts.erase(it);
        else
            ++it;
    }
}

void Optimizer::OptimizeExpr(ExprPtr& expr)
{
    if (expr)
    {
        /* Try to evaluate expression */
        ExprEvaluator exprEvaluator;
        if (auto value = exprEvaluator.EvaluateOrDefault(*expr))
        {
            /* Convert to literal expression */
            if (auto literalExpr = ASTFactory::MakeLiteralExprOrNull(value))
                expr = literalExpr;
        }
    }
}

bool Optimizer::CanRemoveStmt(const Stmt& ast) const
{
    /* Remove if node is null-statement */
    if (ast.Type() == AST::Types::NullStmt)
        return true;

    /* Remove if node is empty code block statement */
    if (ast.Type() == AST::Types::CodeBlockStmt)
    {
        auto& codeBlockStmt = static_cast<const CodeBlockStmt&>(ast);
        if (codeBlockStmt.codeBlock->stmts.empty())
            return true;
    }

    /* Can not remove statement */
    return false;
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void Optimizer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    OptimizeStmtList(ast->stmts);
    VISIT_DEFAULT(CodeBlock);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    OptimizeStmtList(ast->stmts);
    VISIT_DEFAULT(SwitchCase);
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    OptimizeExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    OptimizeExpr(ast->initializer);
}

IMPLEMENT_VISIT_PROC(ForLoopStmt)
{
    Visit(ast->initStmt);
    OptimizeExpr(ast->condition);
    OptimizeExpr(ast->iteration);
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmt)
{
    OptimizeExpr(ast->condition);
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmt)
{
    OptimizeExpr(ast->condition);
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(IfStmt)
{
    OptimizeExpr(ast->condition);
    Visit(ast->bodyStmt);
    Visit(ast->elseStmt);
}

IMPLEMENT_VISIT_PROC(SwitchStmt)
{
    OptimizeExpr(ast->selector);
    Visit(ast->cases);
}

IMPLEMENT_VISIT_PROC(ExprStmt)
{
    OptimizeExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(ReturnStmt)
{
    OptimizeExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    VISIT_DEFAULT(SequenceExpr);
    for (auto& subExpr : ast->exprs)
        OptimizeExpr(subExpr);
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    VISIT_DEFAULT(TernaryExpr);
    OptimizeExpr(ast->condExpr);
    OptimizeExpr(ast->thenExpr);
    OptimizeExpr(ast->elseExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    VISIT_DEFAULT(BinaryExpr);
    OptimizeExpr(ast->lhsExpr);
    OptimizeExpr(ast->rhsExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    VISIT_DEFAULT(UnaryExpr);
    OptimizeExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    VISIT_DEFAULT(PostUnaryExpr);
    OptimizeExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    VISIT_DEFAULT(BracketExpr);

    /* Reduce inner brackets */
    if (auto subBracketExpr = ast->expr->As<BracketExpr>())
        ast->expr = subBracketExpr->expr;

    OptimizeExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    VISIT_DEFAULT(ObjectExpr);
    OptimizeExpr(ast->prefixExpr);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    VISIT_DEFAULT(AssignExpr);
    OptimizeExpr(ast->lvalueExpr);
    OptimizeExpr(ast->rvalueExpr);
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    VISIT_DEFAULT(ArrayExpr);
    for (auto& subExpr : ast->arrayIndices)
        OptimizeExpr(subExpr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    VISIT_DEFAULT(CastExpr);
    OptimizeExpr(ast->expr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    VISIT_DEFAULT(InitializerExpr);
    for (auto& subExpr : ast->exprs)
        OptimizeExpr(subExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================