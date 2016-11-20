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


void GLSLConverter::Convert(
    Program& program, const ShaderTarget shaderTarget, const std::string& nameManglingPrefix)
{
    /* Store settings */
    shaderTarget_       = shaderTarget;
    nameManglingPrefix_ = nameManglingPrefix;

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

IMPLEMENT_VISIT_PROC(VarIdent)
{
    /* Has the variable identifier a next identifier? */
    if (ast->next && ast->symbolRef)
    {
        /* Does this identifier refer to a variable declaration? */
        if (auto varDecl = ast->symbolRef->As<VarDecl>())
        {
            /* Is its type denoter a structure? */
            auto& typeDenoter = *varDecl->declStmntRef->varType->typeDenoter;
            if (typeDenoter.IsStruct())
            {
                /* Must the structure be resolved? */
                auto& structTypeDenoter = static_cast<StructTypeDenoter&>(typeDenoter);
                if (MustResolveStruct(structTypeDenoter.structDeclRef))
                {
                    /* Remove first identifier */
                    ast->PopFront();
                }
            }
        }
    }
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    /* Must this variable be renamed with name mangling? */
    if (MustRenameVarDecl(ast))
        RenameVarDecl(ast);

    /* Default visitor */
    Visitor::VisitVarDecl(ast, args);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (MustResolveStruct(ast))
    {
        /* Append all members of this resolved structure to the list of reserved identifiers for local variables */
        for (auto& member : ast->members)
        {
            for (auto& memberVar : member->varDecls)
                reservedLocalVarIdents_.push_back(memberVar->ident);
        }
    }
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
    localScope_ = true;
    {
        Visitor::VisitFunctionDecl(ast, args);
    }
    localScope_ = false;
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

bool GLSLConverter::ExprContainsSampler(Expr& ast) const
{
    return ast.GetTypeDenoter()->Get()->IsSampler();
}

bool GLSLConverter::VarTypeIsSampler(VarType& ast) const
{
    return ast.typeDenoter->IsSampler();
}

bool GLSLConverter::MustResolveStruct(StructDecl* ast) const
{
    return
    (
        ( shaderTarget_ == ShaderTarget::VertexShader && ast->flags(StructDecl::isShaderInput) ) ||
        ( shaderTarget_ == ShaderTarget::FragmentShader && ast->flags(StructDecl::isShaderOutput) ) ||
        ( shaderTarget_ == ShaderTarget::ComputeShader && ( ast->flags(StructDecl::isShaderInput) || ast->flags(StructDecl::isShaderOutput) ) )
    );
}

bool GLSLConverter::MustRenameVarDecl(VarDecl* ast) const
{
    /* Variable must be renamed if it's inside local scope and its name is reserved */
    return (localScope_ && std::find(reservedLocalVarIdents_.begin(), reservedLocalVarIdents_.end(), ast->ident) != reservedLocalVarIdents_.end());
}

void GLSLConverter::RenameVarDecl(VarDecl* ast)
{
    /* Set new identifier for this variable */
    ast->ident = nameManglingPrefix_ + ast->ident;
}


} // /namespace Xsc



// ================================================================================
