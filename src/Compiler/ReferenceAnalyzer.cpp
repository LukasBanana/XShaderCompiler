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
    Visit(entryPoint);
}


/*
 * ======= Private: =======
 */

bool ReferenceAnalyzer::Reachable(AST* ast)
{
    if (!ast->flags(AST::isReachableDone))
    {
        ast->flags << AST::isReachable;
        ast->flags << AST::isReachableDone;
        return true;
    }
    return false;
}

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