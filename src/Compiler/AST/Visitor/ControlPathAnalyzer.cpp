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

void ControlPathAnalyzer::VisitStmntList(const std::vector<StmntPtr>& stmnts)
{
    /* Search for return statement */
    bool hasReturnPath = false;

    for (auto& ast : stmnts)
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
    VisitStmntList(ast->stmnts);
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

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(BasicDeclStmnt)
{
    PushReturnPath(false);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Visit(ast->bodyStmnt);
    auto thenPath = PopReturnPath();

    Visit(ast->elseStmnt);
    auto elsePath = PopReturnPath();

    PushReturnPath(thenPath && elsePath);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    bool hasDefaultCase = false;

    for (auto& switchCase : ast->cases)
    {
        /* Has the switch statement a default case? */
        if (switchCase->IsDefaultCase())
            hasDefaultCase = true;

        /* Has this case a non-return-path? */
        VisitStmntList(switchCase->stmnts);
        if (!PopReturnPath())
        {
            PushReturnPath(false);
            return;
        }
    }

    /* All cases have a return path, but has the switch statement a default case? */
    PushReturnPath(hasDefaultCase);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    PushReturnPath(false);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    /* Found return statement */
    PushReturnPath(true);
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    PushReturnPath(false);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================