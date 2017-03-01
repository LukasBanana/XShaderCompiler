/*
 * EndOfScopeAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "EndOfScopeAnalyzer.h"
#include "AST.h"


namespace Xsc
{


void EndOfScopeAnalyzer::MarkEndOfScopesFromFunction(FunctionDecl& funcDecl)
{
    Visit(funcDecl.codeBlock);
}


/*
 * ======= Private: =======
 */

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void EndOfScopeAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    if (!ast->stmnts.empty())
        Visit(ast->stmnts.back());
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Visit(ast->bodyStmnt);
    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    ast->flags << ReturnStmnt::isEndOfFunction;
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    // do nothing
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================