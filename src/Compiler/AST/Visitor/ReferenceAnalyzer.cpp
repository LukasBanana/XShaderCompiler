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

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReferenceAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Mark function declaration as referenced */
    Visit(ast->funcDeclRef);

    /* Collect all used intrinsics */
    if (ast->intrinsic != Intrinsic::Undefined)
        program_->usedIntrinsics.insert(ast->intrinsic);

    Visitor::VisitFunctionCall(ast, args);
}

IMPLEMENT_VISIT_PROC(VarType)
{
    if (Reachable(ast))
    {
        Visit(ast->symbolRef);
        Visitor::VisitVarType(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    if (Reachable(ast))
    {
        Visit(ast->symbolRef);
        Visitor::VisitVarIdent(ast, args);
    }
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (Reachable(ast))
    {
        Visit(ast->declStmntRef);
        Visit(ast->bufferDeclRef);
        Visitor::VisitVarDecl(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (Reachable(ast))
        Visitor::VisitStructDecl(ast, args);
}

IMPLEMENT_VISIT_PROC(TextureDecl)
{
    if (Reachable(ast))
        Visit(ast->declStmntRef);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (Reachable(ast))
        Visitor::VisitFunctionDecl(ast, args);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    if (Reachable(ast))
        Visitor::VisitBufferDeclStmnt(ast, args);
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    if (Reachable(ast))
        Visitor::VisitTextureDeclStmnt(ast, args);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================