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
#include "ReportIdents.h"
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
    return (ast ? ast->flags.SetOnce(AST::isReachable) : false);
}

void ReferenceAnalyzer::VisitStmntList(const std::vector<StmntPtr>& stmnts)
{
    for (auto& stmnt : stmnts)
    {
        if (!stmnt->flags(AST::isDeadCode))
            Visit(stmnt);
    }
}

void ReferenceAnalyzer::MarkLValueExpr(const Expr* expr)
{
    if (expr)
    {
        if (auto objectExpr = expr->As<ObjectExpr>())
            MarkLValueExprObject(objectExpr);
        else if (auto bracketExpr = expr->As<BracketExpr>())
            MarkLValueExpr(bracketExpr->expr.get());
        else if (auto arrayExpr = expr->As<ArrayExpr>())
            MarkLValueExpr(arrayExpr->prefixExpr.get());
    }
}

void ReferenceAnalyzer::MarkLValueExprObject(const ObjectExpr* objectExpr)
{
    if (objectExpr)
    {
        /* Mark prefix expression as l-value */
        MarkLValueExpr(objectExpr->prefixExpr.get());

        if (auto symbol = objectExpr->symbolRef)
        {
            /* Mark symbol that it is written to */
            symbol->flags << Decl::isWrittenTo;
        }
    }
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReferenceAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    VisitStmntList(ast->stmnts);
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
    {
        /* Only visit member variables (functions must only be visited by a call expression) */
        Visit(ast->varMembers);
        Reachable(ast->declStmntRef);
    }
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    if (Reachable(ast))
        Visit(ast->declStmntRef);
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    if (Reachable(ast))
        Visit(ast->declStmntRef);
}

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
                RuntimeErr(R_MissingFuncImpl(ast->ToString(false)), ast);
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

        /* Mark parent node as reachable */
        Reachable(ast->declStmntRef);
    }
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (Reachable(ast))
    {
        VISIT_DEFAULT(UniformBufferDecl);
        //Reachable(ast->declStmntRef);
    }
}

/* --- Declaration statements --- */

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

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    Reachable(ast);
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

IMPLEMENT_VISIT_PROC(CallExpr)
{
    /* Visit all forward declarations first */
    if (auto funcDecl = ast->funcDeclRef)
    {
        /* Don't use forward declaration for call stack */
        if (funcDecl->funcImplRef)
            funcDecl = funcDecl->funcImplRef;

        /* Check for recursive calls (if function is already on the call stack) */
        auto funcCallIt = std::find_if(
            callExprStack_.begin(), callExprStack_.end(),
            [funcDecl](CallExpr* callExpr)
            {
                return (callExpr->GetFunctionImpl() == funcDecl);
            }
        );

        if (funcCallIt != callExprStack_.end())
        {
            /* Pass call stack to report handler */
            ReportHandler::HintForNextReport(R_CallStack + ":");
            for (auto funcCall : callExprStack_)
                ReportHandler::HintForNextReport("  '" + funcCall->funcDeclRef->ToString(false) + "' (" + funcCall->area.Pos().ToString() + ")");

            /* Throw error message of recursive call */
            RuntimeErr(R_IllegalRecursiveCall(funcDecl->ToString()), ast);
        }

        /* Mark function declaration as referenced */
        callExprStack_.push_back(ast);
        {
            Visit(funcDecl);
        }
        callExprStack_.pop_back();

        /* Mark owner struct as referenced */
        if (funcDecl)
        {
            if (auto structDecl = funcDecl->structDeclRef)
                Visit(structDecl);
        }
    }

    if (ast->intrinsic != Intrinsic::Undefined)
    {
        /* Mark RW buffers used in read operations */
        if ( ( ast->intrinsic >= Intrinsic::Image_AtomicAdd && ast->intrinsic <= Intrinsic::Image_AtomicExchange ) || ast->intrinsic == Intrinsic::Image_Load )
        {
            if (!ast->arguments.empty())
            {
                const auto& typeDen = ast->arguments[0]->GetTypeDenoter()->GetAliased();
                if (auto bufferTypeDen = typeDen.As<BufferTypeDenoter>())
                {
                    if (IsRWImageBufferType(bufferTypeDen->bufferType))
                    {
                        if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
                            bufferDecl->flags << BufferDecl::isUsedForImageRead;
                    }
                }
            }
        }

        /* Collect all used intrinsics (if they can not be inlined) */
        if (!ast->flags(CallExpr::canInlineIntrinsicWrapper))
            program_->RegisterIntrinsicUsage(ast->intrinsic, ast->arguments);
    }

    /* Mark all arguments, that are assigned to output parameters, as l-values */
    ast->ForEachOutputArgument(
        [this](ExprPtr& argExpr)
        {
            MarkLValueExpr(argExpr.get());
        }
    );

    VISIT_DEFAULT(CallExpr);
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    /* Check if this symbol is the fragment coordinate (SV_Position/ gl_FragCoord) */
    if (auto varDecl = ast->FetchVarDecl())
    {
        if (varDecl->semantic == Semantic::FragCoord && shaderTarget_ == ShaderTarget::FragmentShader)
        {
            /* Mark frag-coord usage in fragment program layout */
            program_->layoutFragment.fragCoordUsed = true;
        }
    }

    /* Fetch used matrix subscripts */
    if (ast->prefixExpr)
    {
        const auto& prefixTypeDen = ast->prefixExpr->GetTypeDenoter()->GetAliased();
        if (prefixTypeDen.IsMatrix())
        {
            auto prefixBaseTypeDen = prefixTypeDen.As<BaseTypeDenoter>();
            program_->usedMatrixSubscripts.insert({ prefixBaseTypeDen->dataType, ast->ident });
        }
    }

    /* Visit symbol reference and sub nodes */
    Visit(ast->symbolRef);

    VISIT_DEFAULT(ObjectExpr);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    /* Mark l-value expression */
    MarkLValueExpr(ast->lvalueExpr.get());

    VISIT_DEFAULT(AssignExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================