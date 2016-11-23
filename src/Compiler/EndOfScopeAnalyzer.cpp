/*
 * EndOfScopeAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "EndOfScopeAnalyzer.h"
#include "AST.h"


namespace Xsc
{


void EndOfScopeAnalyzer::MarkEndOfScopesFromFunction(FunctionDecl& ast)
{
    Visit(ast.codeBlock);
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

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Visit(ast->bodyStmnt);
    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    ast->flags << ReturnStmnt::isEndOfFunction;
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================