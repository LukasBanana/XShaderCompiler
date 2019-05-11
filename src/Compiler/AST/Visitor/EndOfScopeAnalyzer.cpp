/*
 * EndOfScopeAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
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
    if (!ast->stmts.empty())
        Visit(ast->stmts.back());
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(ForLoopStmt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(WhileLoopStmt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(IfStmt)
{
    Visit(ast->bodyStmt);
    Visit(ast->elseStmt);
}

IMPLEMENT_VISIT_PROC(SwitchStmt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(ExprStmt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(ReturnStmt)
{
    ast->flags << ReturnStmt::isEndOfFunction;
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmt)
{
    // do nothing
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================