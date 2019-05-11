/*
 * ControlPathAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ControlPathAnalyzer.h"
#include "AST.h"


namespace Xsc
{


void ControlPathAnalyzer::MarkControlPathsFromFunction(FunctionDecl& funcDecl)
{
    Visit(&funcDecl);
}


/*
 * ======= Private: =======
 */

void ControlPathAnalyzer::PushReturnPath(bool returnPath)
{
    returnPathStack_.push(returnPath);
}

bool ControlPathAnalyzer::PopReturnPath()
{
    if (!returnPathStack_.empty())
    {
        auto v = returnPathStack_.top();
        returnPathStack_.pop();
        return v;
    }
    return false;
}

void ControlPathAnalyzer::VisitStmtList(const std::vector<StmtPtr>& stmts)
{
    /* Search for return statement */
    bool hasReturnPath = false;

    for (auto& ast : stmts)
    {
        if (hasReturnPath)
        {
            /* Mark all statmenets after return path as dead code */
            ast->flags << AST::isDeadCode;
        }
        else
        {
            /* Search in statement for return path */
            Visit(ast);
            if (PopReturnPath())
                hasReturnPath = true;
        }
    }

    PushReturnPath(hasReturnPath);
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ControlPathAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    VisitStmtList(ast->stmts);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    Visit(ast->codeBlock);

    /* Return statement found in all control paths? */
    if (!PopReturnPath())
    {
        if (!ast->returnType->typeDenoter->IsVoid() && !ast->IsForwardDecl())
        {
            /* Mark function with non-return-path flag */
            ast->flags << FunctionDecl::hasNonReturnControlPath;
        }
    }
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(VarDeclStmt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(AliasDeclStmt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(BasicDeclStmt)
{
    PushReturnPath(false);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(CodeBlockStmt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmt)
{
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmt)
{
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmt)
{
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(IfStmt)
{
    Visit(ast->bodyStmt);
    auto thenPath = PopReturnPath();

    Visit(ast->elseStmt);
    auto elsePath = PopReturnPath();

    PushReturnPath(thenPath && elsePath);
}

IMPLEMENT_VISIT_PROC(SwitchStmt)
{
    bool hasDefaultCase = false;

    for (auto& switchCase : ast->cases)
    {
        /* Has the switch statement a default case? */
        if (switchCase->IsDefaultCase())
            hasDefaultCase = true;

        /* Has this case a non-return-path? */
        VisitStmtList(switchCase->stmts);
        if (!PopReturnPath())
        {
            PushReturnPath(false);
            return;
        }
    }

    /* All cases have a return path, but has the switch statement a default case? */
    PushReturnPath(hasDefaultCase);
}

IMPLEMENT_VISIT_PROC(ExprStmt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(ReturnStmt)
{
    /* Found return statement */
    PushReturnPath(true);
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmt)
{
    PushReturnPath(false);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================