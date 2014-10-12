/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLAnalyzer.h"
#include "HLSLTree.h"


namespace HTLib
{


HLSLAnalyzer::HLSLAnalyzer(Logger* log) :
    log_{ log }
{
}

bool HLSLAnalyzer::DecorateAST(
    Program* program,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const ShaderVersions shaderVersion)
{
    if (!program)
        return false;

    /* Store parameters */
    entryPoint_     = entryPoint;
    shaderTarget_   = shaderTarget;
    shaderVersion_  = shaderVersion;

    /* Decorate program AST */
    hasErrors_ = false;
    program_ = program;

    Visit(program);

    return !hasErrors_;
}


/*
 * ======= Private: =======
 */

void HLSLAnalyzer::Error(const std::string& msg, const AST* ast)
{
    hasErrors_ = true;
    if (log_)
    {
        if (ast)
            log_->Error("context error (" + ast->pos.ToString() + ") : " + msg);
        else
            log_->Error("context error : " + msg);
    }
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(className) \
    void HLSLAnalyzer::Visit##className(className* ast, void* args)

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

IMPLEMENT_VISIT_PROC(Structure)
{
    for (auto& varDecl : ast->members)
        Visit(varDecl);
}

/* --- Global declarations --- */

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    if (ast->bufferType != "cbuffer")
        Error("buffer type \"" + ast->bufferType + "\" currently not supported", ast);

    for (auto& member : ast->members)
        Visit(member);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    Visit(ast->structure);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);

    for (auto& varDecl : ast->varDecls)
        Visit(varDecl);
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
}

/* --- Expressions --- */

//...

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(PackOffset)
{
}

IMPLEMENT_VISIT_PROC(VarSemantic)
{
}

IMPLEMENT_VISIT_PROC(VarType)
{
    if (!ast->baseType.empty())
    {
        //...
    }
    else if (ast->structType)
        Visit(ast->structType);
    else
        Error("missing variable type", ast);
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace HTLib



// ================================================================================