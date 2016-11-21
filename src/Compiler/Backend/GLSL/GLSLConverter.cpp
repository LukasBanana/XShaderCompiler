/*
 * GLSLConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLConverter.h"
#include "GLSLHelper.h"
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

IMPLEMENT_VISIT_PROC(Program)
{
    /* Register all input semantic variables are reserved identifiers */
    auto& varDeclRefs = ast->entryPointRef->inputSemantics.varDeclRefs;
    for (auto& varDecl : varDeclRefs)
        reservedVarIdents_.push_back(varDecl->ident);

    /* Default visitor */
    Visitor::VisitProgram(ast, args);
}

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
                ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, DataType::Int, "0"));
                ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, DataType::Int, "1"));
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

IMPLEMENT_VISIT_PROC(StructDecl)
{
    PushStructDeclLevel();
    {
        /* Default visitor */
        Visitor::VisitStructDecl(ast, args);
    }
    PopStructDeclLevel();
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

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    /* Replace 'h' and 'H' suffix with 'f' suffix */
    auto& s = ast->value;

    if (!s.empty())
    {
        if (s.back() == 'h' || s.back() == 'H')
        {
            s.back() = 'f';
            ast->dataType = DataType::Float;
        }
    }

    /* Default visitor */
    Visitor::VisitLiteralExpr(ast, args);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    /* Is the next sub expression again an unary expression? */
    if (ast->expr->Type() == AST::Types::UnaryExpr)
    {
        /* Insert bracket expression */
        auto bracketExpr = MakeShared<BracketExpr>(ast->area);
        
        bracketExpr->expr = ast->expr;

        ast->expr = bracketExpr;
    }

    /* Default visitor */
    Visitor::VisitUnaryExpr(ast, args);
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
    return MustResolveStructForTarget(shaderTarget_, ast);
}

bool GLSLConverter::MustRenameVarDecl(VarDecl* ast) const
{
    /* Variable must be renamed if it's not inside a structure declaration and its name is reserved */
    return (
        !IsInsideStructDecl() &&
        !ast->flags(VarDecl::isShaderInput) &&
        std::find(reservedVarIdents_.begin(), reservedVarIdents_.end(), ast->ident) != reservedVarIdents_.end()
    );
}

void GLSLConverter::RenameVarDecl(VarDecl* ast)
{
    /* Set new identifier for this variable */
    ast->ident = nameManglingPrefix_ + ast->ident;
}

void GLSLConverter::PushStructDeclLevel()
{
    ++structDeclLevel_;
}

void GLSLConverter::PopStructDeclLevel()
{
    --structDeclLevel_;
}

bool GLSLConverter::IsInsideStructDecl() const
{
    return (structDeclLevel_ > 0);
}


} // /namespace Xsc



// ================================================================================
