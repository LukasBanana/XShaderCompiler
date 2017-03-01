/*
 * ReferenceAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReferenceAnalyzer.h"
#include "Exception.h"
#include "ReportHandler.h"
#include "AST.h"
#include <algorithm>


namespace Xsc
{


void ReferenceAnalyzer::MarkReferencesFromEntryPoint(Program& program, const ShaderTarget shaderTarget)
{
    program_        = (&program);
    shaderTarget_   = shaderTarget;

    /* Visit all entry points */
    Visit(program.entryPointRef);
    Visit(program.layoutTessControl.patchConstFunctionRef);
}


/*
 * ======= Private: =======
 */

bool ReferenceAnalyzer::Reachable(AST* ast)
{
    return ast->flags.SetOnce(AST::isReachable);
}

void ReferenceAnalyzer::VisitStmntList(const std::vector<StmntPtr>& stmnts)
{
    for (auto& stmnt : stmnts)
    {
        if (!stmnt->flags(AST::isDeadCode))
            Visit(stmnt);
    }
}

void ReferenceAnalyzer::MarkLValueVarIdent(VarIdent* varIdent)
{
    while (varIdent)
    {
        if (auto varDecl = varIdent->FetchVarDecl())
        {
            /* Mark variable as l-value */
            varDecl->flags << VarDecl::isWrittenTo;
        }
        varIdent = varIdent->next.get();
    }
}

void ReferenceAnalyzer::MarkLValueExpr(Expr* expr)
{
    if (expr)
    {
        if (auto varIdent = expr->FetchVarIdent())
            MarkLValueVarIdent(varIdent);
    }
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReferenceAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    VisitStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Visit all forward declarations first */
    if (auto funcDecl = ast->funcDeclRef)
    {
        /* Don't use forward declaration for call stack */
        if (funcDecl->funcImplRef)
            funcDecl = funcDecl->funcImplRef;

        /* Check for recursive calls (if function is already on the call stack) */
        auto funcCallIt = std::find_if(
            funcCallStack_.begin(), funcCallStack_.end(),
            [funcDecl](FunctionCall* funcCall)
            {
                return (funcCall->GetFunctionImpl() == funcDecl);
            }
        );

        if (funcCallIt != funcCallStack_.end())
        {
            /* Pass call stack to report handler */
            ReportHandler::HintForNextReport("call stack:");
            for (auto funcCall : funcCallStack_)
                ReportHandler::HintForNextReport("  '" + funcCall->funcDeclRef->ToString(false) + "' (" + funcCall->area.Pos().ToString() + ")");

            /* Throw error message of recursive call */
            RuntimeErr("illegal recursive call of function '" + funcDecl->ToString() + "'", ast);
        }

        /* Mark function declaration as referenced */
        funcCallStack_.push_back(ast);
        {
            Visit(funcDecl);
        }
        funcCallStack_.pop_back();
    }

    /* Collect all used intrinsics (if they can not be inlined) */
    if (ast->intrinsic != Intrinsic::Undefined && !ast->flags(FunctionCall::canInlineIntrinsicWrapper))
        program_->RegisterIntrinsicUsage(ast->intrinsic, ast->arguments);

    /* Mark all arguments, that are assigned to output parameters, as l-values */
    ast->ForEachOutputArgument(
        [this](ExprPtr& argExpr)
        {
            MarkLValueExpr(argExpr.get());
        }
    );

    //TODO: also mark l-value arguments for intrinsic with output parameters!!!

    VISIT_DEFAULT(FunctionCall);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    VisitStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    if (Reachable(ast))
    {
        Visit(ast->typeDenoter->SymbolRef());
        VISIT_DEFAULT(TypeSpecifier);
    }
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    if (Reachable(ast))
    {
        Visit(ast->symbolRef);
        VISIT_DEFAULT(VarIdent);
    }
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (Reachable(ast))
    {
        Visit(ast->declStmntRef);
        Visit(ast->bufferDeclRef);
        VISIT_DEFAULT(VarDecl);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (Reachable(ast))
        VISIT_DEFAULT(StructDecl);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    if (Reachable(ast))
        Visit(ast->declStmntRef);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (Reachable(ast))
    {
        /* Is the forward declaration connected to its function implementation? */
        if (ast->IsForwardDecl())
        {
            if (ast->funcImplRef)
                Visit(ast->funcImplRef);
            else
                RuntimeErr("missing function implementation for '" + ast->ToString(false) + "'", ast);
        }
        else
        {
            /* Visit all forward declarations */
            for (auto funcForwardDecl : ast->funcForwardDeclRefs)
                Visit(funcForwardDecl);
        }

        PushFunctionDecl(ast);
        {
            VISIT_DEFAULT(FunctionDecl);
        }
        PopFunctionDecl();
    }
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (Reachable(ast))
        VISIT_DEFAULT(UniformBufferDecl);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    if (Reachable(ast))
    {
        if (auto genericTypeDenoter = ast->typeDenoter->genericTypeDenoter.get())
        {
            if (auto structTypeDen = genericTypeDenoter->As<StructTypeDenoter>())
            {
                /* Mark structure declaration of generic type denoter as referenced */
                Visit(structTypeDen->structDeclRef);
            }
        }

        VISIT_DEFAULT(BufferDeclStmnt);
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    if (IsLValueOp(ast->op))
        MarkLValueExpr(ast->expr.get());
    VISIT_DEFAULT(UnaryExpr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    if (IsLValueOp(ast->op))
        MarkLValueExpr(ast->expr.get());
    VISIT_DEFAULT(PostUnaryExpr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    if (auto varIdent = ast->FetchVarIdent())
    {
        /* Check if this symbol is the fragment coordinate (SV_Position/ gl_FragCoord) */
        while (varIdent)
        {
            if (auto varDecl = varIdent->FetchVarDecl())
            {
                if (varDecl->semantic == Semantic::FragCoord && shaderTarget_ == ShaderTarget::FragmentShader)
                {
                    /* Mark frag-coord usage in fragment program layout */
                    program_->layoutFragment.fragCoordUsed = true;
                    break;
                }
            }
            varIdent = varIdent->next.get();
        }

        if (ast->assignExpr)
            MarkLValueVarIdent(ast->varIdent.get());
    }

    VISIT_DEFAULT(VarAccessExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================