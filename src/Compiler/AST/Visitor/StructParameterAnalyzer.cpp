/*
 * StructParameterAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "StructParameterAnalyzer.h"
#include "AST.h"
#include <algorithm>


namespace Xsc
{


void StructParameterAnalyzer::MarkStructsFromEntryPoint(Program& program, const ShaderTarget shaderTarget)
{
    entryPoint_     = program.entryPointRef;
    shaderTarget_   = shaderTarget;

    /* Visit all entry points */
    Visit(program.entryPointRef);
    Visit(program.layoutTessControl.patchConstFunctionRef);
}

void StructParameterAnalyzer::MarkStructsFromEntryPoint(FunctionDecl& funcDecl, const ShaderTarget shaderTarget)
{
    entryPoint_     = (&funcDecl);
    shaderTarget_   = shaderTarget;

    /* Visit all entry points */
    Visit(entryPoint_);
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

void StructParameterAnalyzer::VisitStmtList(const std::vector<StmtPtr>& stmts)
{
    for (auto& stmt : stmts)
    {
        if (!stmt->flags(AST::isDeadCode))
            Visit(stmt);
    }
}

bool StructParameterAnalyzer::IsVariableAnEntryPointParameter(VarDeclStmt* var) const
{
    /* Is the variable a parameter of the entry point? */
    const auto& entryPointParams = entryPoint_->parameters;

    auto entryPointIt = std::find_if(
        entryPointParams.begin(), entryPointParams.end(),
        [var](const VarDeclStmtPtr& param)
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
    VisitStmtList(ast->stmts);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    VisitStmtList(ast->stmts);
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    Visit(ast->typeDenoter->SymbolRef());
    VISIT_DEFAULT(TypeSpecifier);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (NotVisited(ast))
    {
        auto IsVarEntryPointIO = [](const VarDecl* varDecl)
        {
            //TODO: refactor this!
            #if 1
            return varDecl->flags(VarDecl::isEntryPointOutput);
            #else
            return (varDecl->flags(VarDecl::isShaderInput) || varDecl->flags(VarDecl::isShaderOutput));
            #endif
        };

        /* Returns true, if the current structure declaration (if there is one) is marked as shader input/output */
        auto InsideShaderIOStruct = [this]() -> bool
        {
            if (auto structDecl = ActiveStructDecl())
                return structDecl->flags(StructDecl::isShaderInput | StructDecl::isShaderOutput);
            else
                return false;
        };

        /* Is the variable declaration inside a shader input/output structure? */
        if (!InsideShaderIOStruct())
        {
            /* Is a variable declaration NOT used as entry point return value? */
            if (!IsActiveFunctionDeclEntryPoint() || !IsVarEntryPointIO(ast) || shaderTarget_ == ShaderTarget::GeometryShader)
            {
                auto declStmt = ast->declStmtRef;

                /* Has this variable declaration statement a struct type? */
                if (auto structDecl = declStmt->typeSpecifier->GetStructDeclRef())
                {
                    /* Is the structure used for more than one instance? */
                    if (!IsActiveFunctionDeclEntryPoint() || !IsVarEntryPointIO(ast) || structDecl->HasMultipleShaderOutputInstances())
                    {
                        /* Is this variable NOT a parameter of the entry point? */
                        if (!IsVariableAnEntryPointParameter(declStmt))
                        {
                            /* Mark structure to be used as non-entry-point-parameter */
                            structDecl->AddFlagsRecursiveParents(StructDecl::isNonEntryPointParam);
                        }
                    }
                }
            }
        }

        Visit(ast->declStmtRef);
        Visit(ast->bufferDeclRef);

        VISIT_DEFAULT(VarDecl);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (NotVisited(ast))
    {
        /* If the structure has any member functions, it can not be resolved as entry-point structure */
        if (ast->NumMemberFunctions() > 0)
            ast->AddFlagsRecursiveParents(StructDecl::isNonEntryPointParam);

        PushStructDecl(ast);
        {
            VISIT_DEFAULT(StructDecl);
        }
        PopStructDecl();
    }
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    if (NotVisited(ast))
        Visit(ast->declStmtRef);
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
                    structDecl->AddFlagsRecursiveParents(StructDecl::isNonEntryPointParam);
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

IMPLEMENT_VISIT_PROC(BufferDeclStmt)
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

        VISIT_DEFAULT(BufferDeclStmt);
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(CallExpr)
{
    Visit(ast->GetFunctionDecl());
    VISIT_DEFAULT(CallExpr);
}

IMPLEMENT_VISIT_PROC(IdentExpr)
{
    if (NotVisited(ast))
    {
        Visit(ast->symbolRef);
        VISIT_DEFAULT(IdentExpr);
    }
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
