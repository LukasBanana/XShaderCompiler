/*
 * ReferenceAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReferenceAnalyzer.h"
#include "AST.h"


namespace Xsc
{


void ReferenceAnalyzer::MarkReferencesFromEntryPoint(Program& program)
{
    program_ = &program;
    Visit(program.entryPointRef);
}


/*
 * ======= Private: =======
 */

bool ReferenceAnalyzer::Reachable(AST* ast)
{
    if (!ast->flags(AST::isReachable))
    {
        ast->flags << AST::isReachable;
        return true;
    }
    return false;
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

IMPLEMENT_VISIT_PROC(VarType)
{
    if (Reachable(ast))
    {
        Visit(ast->symbolRef);
        VISIT_DEFAULT(VarType);
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
        VISIT_DEFAULT(FunctionDecl);
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (Reachable(ast))
        VISIT_DEFAULT(UniformBufferDecl);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    if (Reachable(ast))
        VISIT_DEFAULT(BufferDeclStmnt);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================