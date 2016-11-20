/*
 * GLSLConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLConverter.h"
#include "AST.h"
#include "ASTFactory.h"
#include "Exception.h"
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
    if (ast->intrinsic == Intrinsic::Saturate)
    {
        /* Convert "saturate(x)" to "clamp(x, genType(0), genType(1))" */
        if (ast->arguments.size() == 1)
        {
            auto argTypeDen = ast->arguments.front()->GetTypeDenoter()->Get();
            if (argTypeDen->IsBase())
            {
                ast->intrinsic = Intrinsic::Clamp;
                ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, Token::Types::IntLiteral, "0"));
                ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, Token::Types::IntLiteral, "1"));
            }
            else
                RuntimeErr("invalid argument type denoter in intrinsic 'saturate'", ast->arguments.front().get());
        }
        else
            RuntimeErr("invalid number of arguments in intrinsic 'saturate'", ast);
    }
    else if (ast->intrinsic == Intrinsic::Undefined)
    {
        /* Remove arguments which contain a sampler state object, since GLSL does not support sampler states */
        EraseIf(ast->arguments,
            [&](const ExprPtr& expr)
            {
                return ExprContainsSampler(*expr);
            }
        );
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

    /* Default visitor */
    Visitor::VisitFunctionDecl(ast, args);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    if (auto funcCall = ASTFactory::FindSingleFunctionCall(ast->expr.get()))
    {
        /* Is this a special intrinsic function call? */
        if (funcCall->intrinsic == Intrinsic::SinCos)
            ast->expr = ASTFactory::MakeSeparatedSinCosFunctionCalls(*funcCall);
    }

    /* Default visitor */
    Visitor::VisitExprStmnt(ast, args);
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for conversion --- */

bool GLSLConverter::ExprContainsSampler(Expr& ast)
{
    return ast.GetTypeDenoter()->Get()->IsSampler();
}

bool GLSLConverter::VarTypeIsSampler(VarType& ast)
{
    return ast.typeDenoter->IsSampler();
}


} // /namespace Xsc



// ================================================================================
