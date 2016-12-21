/*
 * Optimizer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Optimizer.h"
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

void Optimizer::OptimizeStmntList(std::vector<StmntPtr>& stmnts)
{
    /* Remove null statements */
    for (auto it = stmnts.begin(); it != stmnts.end();)
    {
        if (CanRemoveStmnt(**it))
            it = stmnts.erase(it);
        else
            ++it;
    }
}

bool Optimizer::CanRemoveStmnt(const Stmnt& ast) const
{
    /* Remove if node is null-statement */
    if (ast.Type() == AST::Types::NullStmnt)
        return true;

    /* Remove if node is empty code block statement */
    if (ast.Type() == AST::Types::CodeBlockStmnt)
    {
        auto& codeBlockStmnt = static_cast<const CodeBlockStmnt&>(ast);
        if (codeBlockStmnt.codeBlock->stmnts.empty())
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
    VISIT_DEFAULT(CodeBlock);

    /* Optimize statement list */
    OptimizeStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    VISIT_DEFAULT(SwitchCase);

    /* Optimize statement list */
    OptimizeStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    VISIT_DEFAULT(BracketExpr);

    /* Reduce inner brackets */
    if (auto subBracketExpr = ast->expr->As<BracketExpr>())
        ast->expr = subBracketExpr->expr;
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================