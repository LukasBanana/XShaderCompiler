/*
 * ReferenceAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReferenceAnalyzer.h"
#include "Exception.h"
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

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReferenceAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    VisitStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Mark function declaration as referenced */
    Visit(ast->funcDeclRef);

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
                RuntimeErr("missing function implementation for '" + ast->SignatureToString(false) + "'", ast);
        }

        if (ast->flags(FunctionDecl::isEntryPoint))
        {
            isInsideEntryPoint_ = true;
            {
                VISIT_DEFAULT(FunctionDecl);
            }
            isInsideEntryPoint_ = false;
        }
        else
            VISIT_DEFAULT(FunctionDecl);
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

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    #if 0
    if (isInsideEntryPoint_)
    {
        /* Is a variable declaration NOT used as entry point return value? */
        //TODO...
    }
    else
    #endif
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