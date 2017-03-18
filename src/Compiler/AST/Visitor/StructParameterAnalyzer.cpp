/*
 * StructParameterAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "StructParameterAnalyzer.h"
#include "AST.h"
#include <algorithm>


namespace Xsc
{


void StructParameterAnalyzer::MarkStructsFromEntryPoint(Program& program, const ShaderTarget shaderTarget)
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

bool StructParameterAnalyzer::NotVisited(const AST* ast)
{
    if (visitSet_.find(ast) == visitSet_.end())
    {
        visitSet_.insert(ast);
        return true;
    }
    return false;
}

void StructParameterAnalyzer::VisitStmntList(const std::vector<StmntPtr>& stmnts)
{
    for (auto& stmnt : stmnts)
    {
        if (!stmnt->flags(AST::isDeadCode))
            Visit(stmnt);
    }
}

bool StructParameterAnalyzer::IsVariableAnEntryPointParameter(VarDeclStmnt* var) const
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

bool StructParameterAnalyzer::IsActiveFunctionDeclEntryPoint() const
{
    if (auto funcDecl = ActiveFunctionDecl())
        return funcDecl->flags(FunctionDecl::isEntryPoint);
    else
        return false;
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void StructParameterAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

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
    if (NotVisited(ast))
    {
        Visit(ast->typeDenoter->SymbolRef());
        VISIT_DEFAULT(TypeSpecifier);
    }
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (NotVisited(ast))
    {
        /* Is a variable declaration NOT used as entry point return value? */
        if (!IsActiveFunctionDeclEntryPoint() || !ast->flags(VarDecl::isEntryPointOutput) || shaderTarget_ == ShaderTarget::GeometryShader)
        {
            auto declStmnt = ast->declStmntRef;

            /* Has this variable statement a struct type? */
            if (auto structDecl = declStmnt->typeSpecifier->GetStructDeclRef())
            {
                /* Is the structure used for more than one instance? */
                if (!IsActiveFunctionDeclEntryPoint() || !ast->flags(VarDecl::isEntryPointOutput) || structDecl->HasMultipleShaderOutputInstances())
                {
                    /* Is this variable NOT a parameter of the entry point? */
                    if (!IsVariableAnEntryPointParameter(declStmnt))
                    {
                        /* Mark structure to be used as non-entry-point-parameter */
                        structDecl->flags << StructDecl::isNonEntryPointParam;
                    }
                }
            }
        }

        Visit(ast->declStmntRef);
        Visit(ast->bufferDeclRef);

        VISIT_DEFAULT(VarDecl);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (NotVisited(ast))
        VISIT_DEFAULT(StructDecl);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    if (NotVisited(ast))
        Visit(ast->declStmntRef);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (NotVisited(ast))
    {
        /* Is a variable declaration NOT used as entry point return value? */
        if (!ast->flags(FunctionDecl::isEntryPoint) || shaderTarget_ == ShaderTarget::GeometryShader)
        {
            /* Has the return type specifier a struct type? */
            if (auto structDecl = ast->returnType->GetStructDeclRef())
            {
                /* Is the structure used for more than one instance? */
                if (!ast->flags(FunctionDecl::isEntryPoint) || structDecl->HasMultipleShaderOutputInstances())
                {
                    /* Mark structure to be used as non-entry-point-parameter */
                    structDecl->flags << StructDecl::isNonEntryPointParam;
                }
            }
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
    if (NotVisited(ast))
        VISIT_DEFAULT(UniformBufferDecl);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    if (NotVisited(ast))
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

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    if (NotVisited(ast))
    {
        Visit(ast->symbolRef);
        VISIT_DEFAULT(ObjectExpr);
    }
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================