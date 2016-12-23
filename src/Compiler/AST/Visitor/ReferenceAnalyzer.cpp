/*
 * ReferenceAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReferenceAnalyzer.h"
#include "Exception.h"
#include "AST.h"


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
        if (ast->IsForwardDecl())
        {
            if (ast->funcImplRef)
                Visit(ast->funcImplRef);
            else
                RuntimeErr("missing function implementation for '" + ast->SignatureToString(false) + "'", ast);
        }

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

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Mark symbol as used */
    if (auto symbol = ast->varIdent->symbolRef)
    {
        symbol->flags << AST::isUsed;

        /* Check if this symbol is the fragment coordinate (SV_Position/ gl_FragCoord) */
        if (auto varDecl = symbol->As<VarDecl>())
        {
            if (varDecl->semantic == Semantic::Position && shaderTarget_ == ShaderTarget::FragmentShader)
            {
                /* Mark frag-coord usage in fragment program layout */
                program_->layoutFragment.fragCoordUsed = true;
            }
        }
    }

    VISIT_DEFAULT(VarAccessExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================