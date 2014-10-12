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
    EstablishMaps();
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

void HLSLAnalyzer::EstablishMaps()
{
    intrinsicMap_ = std::map<std::string, IntrinsicClasses>
    {
        { "InterlockedAdd",             IntrinsicClasses::Interlocked },
        { "InterlockedAnd",             IntrinsicClasses::Interlocked },
        { "InterlockedOr",              IntrinsicClasses::Interlocked },
        { "InterlockedXor",             IntrinsicClasses::Interlocked },
        { "InterlockedMin",             IntrinsicClasses::Interlocked },
        { "InterlockedMax",             IntrinsicClasses::Interlocked },
        { "InterlockedCompareExchange", IntrinsicClasses::Interlocked },
        { "InterlockedExchange",        IntrinsicClasses::Interlocked },
    };
}

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

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Check if a specific intrinsic is used */
    if (ast->name == "mul")
        program_->flags << Program::mulIntrinsicUsed;
    else
    {
        auto it = intrinsicMap_.find(ast->name);
        if (it != intrinsicMap_.end())
        {
            switch (it->second)
            {
                case IntrinsicClasses::Interlocked:
                    program_->flags << Program::interlockedIntrinsicsUsed;
                    break;
            }
        }
    }

    /* Analyze function arguments */
    for (auto& arg : ast->arguments)
        Visit(arg);
}

IMPLEMENT_VISIT_PROC(Structure)
{
    for (auto& varDecl : ast->members)
        Visit(varDecl);
}

/* --- Global declarations --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (ast->name == entryPoint_)
        ast->flags << FunctionDecl::isEntryPoint;

    for (auto& attrib : ast->attribs)
        Visit(attrib);

    Visit(ast->returnType);
    for (auto& param : ast->parameters)
        Visit(param);

    Visit(ast->codeBlock);
}

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
    // do nothing
}

/* --- Expressions --- */

//...

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(PackOffset)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(VarSemantic)
{
    // do nothing
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
    for (auto& index : ast->arrayIndices)
        Visit(index);
    Visit(ast->next);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    for (auto& dim : ast->arrayDims)
        Visit(dim);
    for (auto& semantic : ast->semantics)
        Visit(semantic);
    Visit(ast->initializer);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace HTLib



// ================================================================================