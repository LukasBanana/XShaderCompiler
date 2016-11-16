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


void ReferenceAnalyzer::MarkReferencesFromEntryPoint(FunctionDecl* entryPoint)
{
    entryPoint->flags << FunctionDecl::isReferenced;
    Visit(entryPoint);
}


/*
 * ======= Private: =======
 */

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReferenceAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    Visit(ast->funcDeclRef);

    Visitor::VisitFunctionCall(ast, args);

    //TODO: remove this from the ReferenceAnalyzer; intrinsic references should not be flagged here!
    #if 0
    if (ast->varIdent)
    {
        /* Mark this function to be referenced */
        const auto& ident = ast->varIdent->ident;

        /* Check for intrinsic usage */
        if (ident == "rcp")
            program_->flags << Program::rcpIntrinsicUsed;
        else if (ident == "sincos")
            program_->flags << Program::sinCosIntrinsicUsed;
        else if (ident == "clip")
            program_->flags << Program::clipIntrinsicUsed;
    }
    #endif
}

IMPLEMENT_VISIT_PROC(VarType)
{
    Visit(ast->symbolRef);
    Visitor::VisitVarType(ast, args);
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    Visit(ast->symbolRef);

    Visitor::VisitVarIdent(ast, args);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (!ast->flags(VarDecl::wasMarked))
    {
        ast->flags << VarDecl::wasMarked;
        ast->flags << VarDecl::isReferenced;
        
        Visit(ast->declStmntRef);
        Visit(ast->bufferDeclRef);

        Visitor::VisitVarDecl(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (!ast->flags(StructDecl::wasMarked))
    {
        ast->flags << StructDecl::wasMarked;
        ast->flags << StructDecl::isReferenced;

        Visitor::VisitStructDecl(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(TextureDecl)
{
    ast->flags << TextureDecl::isReferenced;
    Visit(ast->declStmntRef);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (!ast->flags(FunctionDecl::wasMarked))
    {
        ast->flags << FunctionDecl::wasMarked;
        ast->flags << FunctionDecl::isReferenced;

        Visitor::VisitFunctionDecl(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    if (!ast->flags(BufferDeclStmnt::wasMarked))
    {
        ast->flags << BufferDeclStmnt::wasMarked;
        ast->flags << BufferDeclStmnt::isReferenced;

        Visitor::VisitBufferDeclStmnt(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    ast->flags << TextureDeclStmnt::isReferenced;
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================