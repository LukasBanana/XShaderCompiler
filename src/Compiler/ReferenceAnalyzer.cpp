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

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ReferenceAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    for (auto& globDecl : ast->globalDecls)
        Visit(globDecl);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);
}

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
        /* Check for intrinsic usage */
        if (name == "rcp")
            program_->flags << Program::rcpIntrinsicUsed;
        else if (name == "sincos")
            program_->flags << Program::sinCosIntrinsicUsed;
        else if (name == "clip")
            program_->flags << Program::clipIntrinsicUsed;
    }

    /* Visit arguments */
    for (auto& arg : ast->arguments)
        Visit(arg);
}

IMPLEMENT_VISIT_PROC(Structure)
{
    /* Check if this function was already marked by this analyzer */
    if (!ast->flags(FunctionDecl::wasMarked))
    {
        ast->flags << FunctionDecl::wasMarked;

        /* Mark this structure to be referenced */
        ast->flags << Structure::isReferenced;

        /* Analyze structure members */
        for (auto& member : ast->members)
            Visit(member);
    }
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);
}

/* --- Global declarations --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    /* Check if this function was already marked by this analyzer */
    if (!ast->flags(FunctionDecl::wasMarked))
    {
        ast->flags << FunctionDecl::wasMarked;

        /* Analyze function */
        Visit(ast->returnType);
        for (auto& param : ast->parameters)
            Visit(param);
        Visit(ast->codeBlock);
    }
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    /* Check if this function was already marked by this analyzer */
    if (!ast->flags(UniformBufferDecl::wasMarked))
    {
        ast->flags << UniformBufferDecl::wasMarked;

        /* Analyze uniform buffer */
        ast->flags << UniformBufferDecl::isReferenced;

        for (auto& member : ast->members)
            Visit(member);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    Visit(ast->structure);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    Visit(ast->initSmnt);
    Visit(ast->condition);
    Visit(ast->iteration);
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    Visit(ast->condition);
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    Visit(ast->bodyStmnt);
    Visit(ast->condition);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Visit(ast->condition);
    Visit(ast->bodyStmnt);
    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    Visit(ast->selector);

    for (auto& switchCase : ast->cases)
        Visit(switchCase);
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);
    for (auto& varDecl : ast->varDecls)
        Visit(varDecl);
}

IMPLEMENT_VISIT_PROC(AssignStmnt)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(FunctionCallStmnt)
{
    Visit(ast->call);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Visit(ast->expr);
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Visit(ast->firstExpr);
    Visit(ast->nextExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Visit(ast->lhsExpr);
    Visit(ast->rhsExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    Visit(ast->call);
    Visit(ast->varIdentSuffix);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Mark texture reference */
    auto symbol = symTable_->Fetch(ast->varIdent->ident);
    if (symbol)
    {
        if (symbol->Type() == AST::Types::TextureDecl)
            MarkTextureReference(symbol, ast->varIdent->ident);
        else if (symbol->Type() == AST::Types::VarDecl)
        {
            auto varDecl = dynamic_cast<VarDecl*>(symbol);
            if (varDecl && varDecl->uniformBufferRef)
                Visit(varDecl->uniformBufferRef);
        }
    }

    Visit(ast->assignExpr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    for (auto& expr : ast->exprs)
        Visit(expr);
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

IMPLEMENT_VISIT_PROC(VarIdent)
{
    for (auto& index : ast->arrayIndices)
        Visit(index);
    Visit(ast->next);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    for (auto& dim : ast->arrayDims)
        Visit(dim);
    Visit(ast->initializer);
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for analysis --- */

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


} // /namespace Xsc



// ================================================================================