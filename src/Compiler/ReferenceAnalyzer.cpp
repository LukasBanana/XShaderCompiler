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


ReferenceAnalyzer::ReferenceAnalyzer(const ASTSymbolTable& symTable) :
    symTable_{ &symTable }
{
}

void ReferenceAnalyzer::MarkReferencesFromEntryPoint(FunctionDecl* ast, Program* program)
{
    program_ = program;
    ast->flags << FunctionDecl::isReferenced;
    Visit(ast);
}


/*
 * ======= Private: =======
 */

void ReferenceAnalyzer::MarkTextureReference(AST* ast, const std::string& texIdent)
{
    ast->flags << TextureDecl::isReferenced;

    auto texDecl = dynamic_cast<TextureDecl*>(ast);
    if (texDecl)
    {
        /* Mark individual texture identifier to be used */
        for (auto& tex : texDecl->names)
        {
            if (tex->ident == texIdent)
            {
                tex->flags << BufferDeclIdent::isReferenced;
                break;
            }
        }
    }
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReferenceAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Mark this function to be referenced */
    const auto& name = ast->name->ident;
    auto symbol = symTable_->Fetch(name);

    if (symbol)
    {
        if (symbol->Type() == AST::Types::FunctionDecl)
        {
            auto functionDecl = dynamic_cast<FunctionDecl*>(symbol);
            if (functionDecl)
            {
                /* Mark all forward declarations to this function */
                for (auto& forwardDecl : functionDecl->forwardDeclsRef)
                    forwardDecl->flags << FunctionDecl::isReferenced;
            }

            /* Mark this function and visit the entire function body */
            symbol->flags << FunctionDecl::isReferenced;
            Visit(symbol);
        }
        else if (symbol->Type() == AST::Types::TextureDecl)
            MarkTextureReference(symbol, name);
    }
    else
    {
        //TODO: remove this from the ReferenceAnalyzer; intrinsic references should not be flagged here!
        /* Check for intrinsic usage */
        if (name == "rcp")
            program_->flags << Program::rcpIntrinsicUsed;
        else if (name == "sincos")
            program_->flags << Program::sinCosIntrinsicUsed;
        else if (name == "clip")
            program_->flags << Program::clipIntrinsicUsed;
    }

    /* Visit arguments */
    Visit(ast->arguments);
}

IMPLEMENT_VISIT_PROC(Structure)
{
    /* Check if this function was already marked by this analyzer */
    if (!ast->flags(Structure::wasMarked))
    {
        ast->flags << Structure::wasMarked;

        /* Mark this structure to be referenced */
        ast->flags << Structure::isReferenced;

        /* Analyze structure members */
        Visit(ast->members);
    }
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    /* Check if this function was already marked by this analyzer */
    if (!ast->flags(FunctionDecl::wasMarked))
    {
        ast->flags << FunctionDecl::wasMarked;

        /* Default visitor */
        Visitor::VisitFunctionDecl(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    /* Check if this function was already marked by this analyzer */
    if (!ast->flags(BufferDeclStmnt::wasMarked))
    {
        ast->flags << BufferDeclStmnt::wasMarked;
        ast->flags << BufferDeclStmnt::isReferenced;

        /* Default visitor */
        Visitor::VisitBufferDeclStmnt(ast, args);
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Mark either texture object or uniform buffer */
    auto symbol = symTable_->Fetch(ast->varIdent->ident);
    if (symbol)
    {
        if (symbol->Type() == AST::Types::TextureDecl)
        {
            /* Mark texture object as referenced */
            MarkTextureReference(symbol, ast->varIdent->ident);
        }
        else if (symbol->Type() == AST::Types::VarDecl)
        {
            /* Mark uniform buffer as referenced */
            auto varDecl = dynamic_cast<VarDecl*>(symbol);
            if (varDecl && varDecl->uniformBufferRef)
                Visit(varDecl->uniformBufferRef);
        }
    }

    Visit(ast->assignExpr);
}

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(VarType)
{
    if (!ast->baseType.empty())
    {
        auto symbol = symTable_->Fetch(ast->baseType);
        if (symbol)
            Visit(symbol);
    }
    else if (ast->structType)
        Visit(ast->structType);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================