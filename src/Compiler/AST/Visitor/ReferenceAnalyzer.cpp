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

bool ReferenceAnalyzer::IsVariableAnEntryPointParameter(VarDeclStmnt* var) const
{
    /* Is the variable a parameter of the entry point? */
    const auto& entryPointParams = program_->entryPointRef->parameters;

    auto entryPointIt = std::find_if(
        entryPointParams.begin(), entryPointParams.end(),
        [var](const VarDeclStmntPtr& param)
        {
            return (param.get() == var);
        }
    );

    return (entryPointIt != entryPointParams.end());
}

void ReferenceAnalyzer::PushFunctionDecl(FunctionDecl* funcDecl)
{
    funcDeclStack_.push(funcDecl);
}

void ReferenceAnalyzer::PopFunctionDecl()
{
    funcDeclStack_.pop();
}

bool ReferenceAnalyzer::IsInsideEntryPoint() const
{
    return (!funcDeclStack_.empty() && funcDeclStack_.top()->flags(FunctionDecl::isEntryPoint));
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
    {
        /* Insert argument types (only base types) into usage list */
        IntrinsicUsage::ArgumentList argList;
        {
            for (auto& arg : ast->arguments)
            {
                auto typeDen = arg->GetTypeDenoter()->Get();
                if (auto baseTypeDen = typeDen->As<BaseTypeDenoter>())
                    argList.argTypes.push_back(baseTypeDen->dataType);
            }
        }
        program_->usedIntrinsics[ast->intrinsic].argLists.insert(argList);
    }

    if (ast->funcDeclRef)
    {
        /* Mark all arguments, that are assigned to output parameters, as l-values */
        const auto& arguments = ast->arguments;
        const auto& parameters = ast->funcDeclRef->parameters;

        for (std::size_t i = 0, n = std::min(arguments.size(), parameters.size()); i < n; ++i)
        {
            if (parameters[i]->IsOutput())
            {
                if (auto varDecl = arguments[i]->FetchVarDecl())
                    varDecl->flags << (AST::isUsed | VarDecl::isWrittenTo);
            }
        }
    }

    VISIT_DEFAULT(FunctionCall);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    VisitStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(TypeName)
{
    if (Reachable(ast))
    {
        Visit(ast->typeDenoter->SymbolRef());
        VISIT_DEFAULT(TypeName);
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

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    /* Has this variable statement a struct type? */
    auto typeDen = ast->varType->GetTypeDenoter()->Get();
    if (auto structTypeDen = typeDen->As<StructTypeDenoter>())
    {
        if (auto structDecl = structTypeDen->structDeclRef)
        {
            /* Is this variable NOT a parameter of the entry point? */
            if (!IsVariableAnEntryPointParameter(ast))
            {
                /* Mark structure to be used as non-entry-point-parameter */
                structDecl->flags << StructDecl::isNonEntryPointParam;
            }
        }
    }

    VISIT_DEFAULT(VarDeclStmnt);
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    if (auto symbol = ast->varIdent->symbolRef)
    {
        /* Mark symbol as used */
        symbol->flags << AST::isUsed;

        /* Check if this symbol is the fragment coordinate (SV_Position/ gl_FragCoord) */
        if (auto varDecl = symbol->As<VarDecl>())
        {
            if (varDecl->semantic == Semantic::Position && shaderTarget_ == ShaderTarget::FragmentShader)
            {
                /* Mark frag-coord usage in fragment program layout */
                program_->layoutFragment.fragCoordUsed = true;
            }
            
            if (ast->assignExpr)
            {
                /* Mark variable as l-value */
                varDecl->flags << VarDecl::isWrittenTo;
            }
        }
    }

    VISIT_DEFAULT(VarAccessExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================