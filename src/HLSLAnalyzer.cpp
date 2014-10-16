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
    const ShaderVersions shaderVersion,
    const std::string& localVarPrefix)
{
    if (!program)
        return false;

    /* Store parameters */
    entryPoint_     = entryPoint;
    shaderTarget_   = shaderTarget;
    shaderVersion_  = shaderVersion;
    localVarPrefix_ = localVarPrefix;

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

void HLSLAnalyzer::Warning(const std::string& msg, const AST* ast)
{
    if (log_)
    {
        if (ast)
            log_->Warning("warning (" + ast->pos.ToString() + ") : " + msg);
        else
            log_->Warning("warning : " + msg);
    }
}

void HLSLAnalyzer::OpenScope()
{
    symTable_.OpenScope();
}

void HLSLAnalyzer::CloseScope()
{
    symTable_.CloseScope();
}

void HLSLAnalyzer::Register(const std::string& ident, AST* ast, const OnOverrideProc& overrideProc)
{
    try
    {
        symTable_.Register(ident, ast, overrideProc);
    }
    catch (const std::exception& err)
    {
        Error(err.what(), ast);
    }
}

AST* HLSLAnalyzer::Fetch(const std::string& ident) const
{
    return symTable_.Fetch(ident);
}

AST* HLSLAnalyzer::Fetch(const VarIdentPtr& ident) const
{
    auto fullIdent = FullVarIdent(ident);
    return Fetch(fullIdent);
}

void HLSLAnalyzer::DecorateEntryInOut(VarDeclStmnt* ast, bool isInput)
{
    const auto structFlag = (isInput ? Structure::isShaderInput : Structure::isShaderOutput);

    /* Add flag to variable declaration statement */
    ast->flags << (isInput ? VarDeclStmnt::isShaderInput : VarDeclStmnt::isShaderOutput);

    /* Add flag to structure type */
    auto& varType = ast->varType;
    if (varType->structType)
        varType->structType->flags << structFlag;

    /* Add flag to optional symbol reference */
    auto& symbolRef = varType->symbolRef;
    if (symbolRef && symbolRef->Type() == AST::Types::Structure)
    {
        auto structType = dynamic_cast<Structure*>(symbolRef);
        if (structType)
        {
            structType->flags << structFlag;
            if (!ast->varDecls.empty())
                structType->aliasName = ast->varDecls.front()->name;
        }
    }
}

void HLSLAnalyzer::DecorateEntryInOut(VarType* ast, bool isInput)
{
    const auto structFlag = (isInput ? Structure::isShaderInput : Structure::isShaderOutput);

    /* Add flag to structure type */
    if (ast->structType)
        ast->structType->flags << structFlag;

    /* Add flag to optional symbol reference */
    auto& symbolRef = ast->symbolRef;
    if (symbolRef && symbolRef->Type() == AST::Types::Structure)
    {
        auto structType = dynamic_cast<Structure*>(symbolRef);
        if (structType)
            structType->flags << structFlag;
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
    OpenScope();

    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);

    CloseScope();
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    auto name = FullVarIdent(ast->name);

    /* Check if a specific intrinsic is used */
    if (name == "mul")
        program_->flags << Program::mulIntrinsicUsed;
    else
    {
        auto it = intrinsicMap_.find(name);
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
    if (!ast->name.empty())
    {
        Register(
            ast->name, ast,
            [](AST* symbol) -> bool
            {
                return symbol->Type() == AST::Types::StructDecl;//!TODO! Types::StructForwardDecl !!!
            }
        );
    }

    OpenScope();
    {
        for (auto& varDecl : ast->members)
            Visit(varDecl);
    }
    CloseScope();
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
    /* Register symbol name */
    Register(
        ast->name, ast,
        [](AST* symbol) -> bool
        {
            return symbol->Type() == AST::Types::FunctionDecl;//!TODO! Types::FunctionForwardDecl !!!
        }
    );

    /* Visit function header */
    for (auto& attrib : ast->attribs)
        Visit(attrib);

    Visit(ast->returnType);

    OpenScope();
    {
        for (auto& param : ast->parameters)
            Visit(param);

        /* Mark function as used when it's the main entry point */
        const auto isEntryPoint = (ast->name == entryPoint_);

        if (isEntryPoint)
        {
            /* Add flags */
            ast->flags << FunctionDecl::isEntryPoint;
            ast->flags << FunctionDecl::isUsed;

            /* Add flags to input- and output parameters of the main entry point */
            DecorateEntryInOut(ast->returnType.get(), false);
            for (auto& param : ast->parameters)
                DecorateEntryInOut(param.get(), true);
        }

        /* Visit function body */
        isInsideFunc_ = true;
        isInsideEntryPoint_ = isEntryPoint;
        {
            Visit(ast->codeBlock);
        }
        isInsideEntryPoint_ = false;
        isInsideFunc_ = false;
    }
    CloseScope();
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

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->initSmnt);
        Visit(ast->condition);
        Visit(ast->iteration);

        OpenScope();
        {
            Visit(ast->bodyStmnt);
        }
        CloseScope();
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->condition);

        OpenScope();
        {
            Visit(ast->bodyStmnt);
        }
        CloseScope();
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->codeBlock);
        Visit(ast->condition);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->condition);

        OpenScope();
        {
            Visit(ast->bodyStmnt);
        }
        CloseScope();
    }
    CloseScope();

    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    OpenScope();
    {
        Visit(ast->bodyStmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->selector);

        for (auto& switchCase : ast->cases)
            Visit(switchCase);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);

    for (auto& varDecl : ast->varDecls)
        Visit(varDecl);

    /* Decorate variable type */
    if (isInsideEntryPoint_ && ast->varDecls.empty())
    {
        auto symbolRef = ast->varType->symbolRef;
        if (symbolRef && symbolRef->Type() == AST::Types::Structure)
        {
            auto structType = dynamic_cast<Structure*>(symbolRef);
            if (structType && structType->flags(Structure::isShaderOutput) && structType->aliasName.empty())
            {
                /* Store alias name for shader output interface block */
                structType->aliasName = ast->varDecls.front()->name;
            }
        }
    }
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    // do nothing
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    // do nothing
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
    /* Decorate AST */
    auto symbol = Fetch(ast->varIdent);
    if (symbol)
    {
        if (symbol->Type() == AST::Types::VarDecl)
        {
            auto varDecl = dynamic_cast<VarDecl*>(symbol);
            if (varDecl && varDecl->flags(VarDecl::isInsideFunc))
                ast->varIdent->ident = localVarPrefix_ + ast->varIdent->ident;
        }
    }
    else
        Warning("undeclrated identifier \"" + FullVarIdent(ast->varIdent) + "\"", ast);
}

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
        /* Decorate variable type */
        auto symbol = Fetch(ast->baseType);
        if (symbol)
            ast->symbolRef = symbol;
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
    if (isInsideFunc_)
        ast->flags << VarDecl::isInsideFunc;

    for (auto& dim : ast->arrayDims)
        Visit(dim);
    for (auto& semantic : ast->semantics)
        Visit(semantic);

    Visit(ast->initializer);

    Register(ast->name, ast);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace HTLib



// ================================================================================