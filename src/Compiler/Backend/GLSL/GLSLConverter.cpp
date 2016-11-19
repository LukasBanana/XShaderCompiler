/*
 * GLSLConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLConverter.h"
#include "AST.h"
#include "Helper.h"


namespace Xsc
{


void GLSLConverter::Convert(Program& program)
{
    /* Visit program AST */
    Visit(&program);
}


/*
 * ======= Private: =======
 */

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    if (ast->intrinsic == Intrinsic::Undefined)
    {
        /* Remove arguments which contain a sampler state object, since GLSL does not support sampler states */
        EraseIf(
            ast->arguments,
            [&](const ExprPtr& expr)
            {
                return ExprContainsSampler(*expr);
            }
        );

        /*for (auto it = ast->arguments.begin(); it != ast->arguments.end();)
        {
            if (ExprContainsSampler(it->get()))
                it = ast->arguments.erase(it);
            else
                ++it;
        }*/
    }

    /* Default visitor */
    Visitor::VisitFunctionCall(ast, args);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    /* Is function reachable? */
    if (!ast->flags(AST::isReachable))
        return;

    /* Remove parameters which contain a sampler state object, since GLSL does not support sampler states */
    EraseIf(
        ast->parameters,
        [&](const VarDeclStmntPtr& varDeclStmnt)
        {
            return VarTypeIsSampler(*varDeclStmnt->varType);
        }
    );
    /*for (auto it = ast->parameters.begin(); it != ast->parameters.end();)
    {
        if (VarTypeIsSampler(*(*it)->varType))
            it = ast->parameters.erase(it);
        else
            ++it;
    }*/

    /* Default visitor */
    Visitor::VisitFunctionDecl(ast, args);
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for conversion --- */

//TODO: refactor this function:
bool GLSLConverter::ExprContainsSampler(Expr& ast)
{
    if (ast.Type() == AST::Types::BracketExpr)
    {
        auto& bracketExpr = static_cast<BracketExpr&>(ast);
        return ExprContainsSampler(*bracketExpr.expr);
    }
    if (ast.Type() == AST::Types::BinaryExpr)
    {
        auto& binaryExpr = static_cast<BinaryExpr&>(ast);
        return
        (
            ExprContainsSampler(*binaryExpr.lhsExpr) ||
            ExprContainsSampler(*binaryExpr.rhsExpr)
        );
    }
    if (ast.Type() == AST::Types::UnaryExpr)
    {
        auto& unaryExpr = static_cast<UnaryExpr&>(ast);
        return ExprContainsSampler(*unaryExpr.expr);
    }
    if (ast.Type() == AST::Types::VarAccessExpr)
    {
        auto symbolRef = static_cast<VarAccessExpr&>(ast).varIdent->symbolRef;
        if (symbolRef && symbolRef->Type() == AST::Types::SamplerDeclStmnt)
            return true;
    }
    return false;
}

bool GLSLConverter::VarTypeIsSampler(VarType& ast)
{
    return ast.typeDenoter->IsSampler();
}


} // /namespace Xsc



// ================================================================================
