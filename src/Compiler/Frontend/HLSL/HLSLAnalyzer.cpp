/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLAnalyzer.h"
#include "HLSLIntrinsics.h"
#include <algorithm>


namespace Xsc
{


HLSLAnalyzer::HLSLAnalyzer(Log* log) :
    Analyzer{ log }
{
    EstablishMaps();
    DeclareIntrinsics();
}

void HLSLAnalyzer::DecorateASTPrimary(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store parameters */
    entryPoint_     = inputDesc.entryPoint;
    shaderTarget_   = inputDesc.shaderTarget;
    versionIn_      = inputDesc.shaderVersion;
    localVarPrefix_ = outputDesc.formatting.prefix;

    /* Decorate program AST */
    program_ = &program;

    Visit(&program);
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

void HLSLAnalyzer::DeclareIntrinsics()
{
    #if 0//TODO: currently for testing only

    static std::set<std::shared_ptr<IntrinsicDecl>> s_intrinsics;

    auto ast = std::make_shared<IntrinsicDecl>();

    ast->ident = "mul";

    Register(ast->ident, ast.get());

    s_intrinsics.insert(ast);

    #endif
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void HLSLAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Analyze context of the entire program */
    Visit(ast->globalStmnts);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    OpenScope();
    {
        Visit(ast->stmnts);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Analyze function arguments first */
    PushFunctionCall(ast);
    {
        Visit(ast->arguments);
    }
    PopFunctionCall();

    if (ast->varIdent)
    {
        auto name = ast->varIdent->ToString();

        /* Check if the function call refers to an intrinsic */
        auto intrIt = HLSLIntrinsics().find(name);
        if (intrIt != HLSLIntrinsics().end())
            AnalyzeFunctionCallIntrinsic(ast, name, intrIt->second);
        else
            AnalyzeFunctionCallStandard(ast);
    }
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    Register(ast->ident, ast);

    if (InsideFunctionDecl())
        ast->flags << VarDecl::isInsideFunc;

    Visit(ast->arrayDims);

    for (auto& semantic : ast->semantics)
    {
        Visit(semantic);

        /* Store references to members with system value semantic (SV_...) in all parent structures */
        if (IsSystemValueSemnatic(semantic->semantic))
        {
            for (auto& structDecl : structStack_)
                structDecl->systemValuesRef[ast->ident] = ast;
        }
    }

    if (ast->initializer)
    {
        Visit(ast->initializer);

        /* Compare initializer type with var-decl type */
        if (auto initTypeDen = GetTypeDenoterFrom(ast->initializer.get()))
        {
            if (auto declTypeDen = GetTypeDenoterFrom(ast))
                ValidateTypeCast(*initTypeDen, *declTypeDen, ast->initializer.get());
        }
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    /* Find base struct-decl */
    if (!ast->baseStructName.empty())
        ast->baseStructRef = FetchStructDeclFromIdent(ast->baseStructName);

    /* Register struct identifier in symbol table */
    Register(ast->ident, ast);

    structStack_.push_back(ast);

    OpenScope();
    {
        Visit(ast->members);
    }
    CloseScope();

    structStack_.pop_back();
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    AnalyzeTypeDenoter(ast->typeDenoter, ast);

    /* Register type-alias identifier in symbol table */
    Register(ast->ident, ast);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    GetReportHandler().PushContextDesc(ast->SignatureToString(false));

    const auto isEntryPoint = (ast->ident == entryPoint_);

    /* Register function declaration in symbol table */
    Register(ast->ident, ast);

    /* Visit attributes */
    Visit(ast->attribs);

    /* Visit function header */
    Visit(ast->returnType);

    OpenScope();
    {
        Visit(ast->parameters);

        //TODO: refactor this part!
        #if 1

        /* Special case for the main entry point */
        if (isEntryPoint)
        {
            program_->entryPointRef = ast;

            /* Add flags */
            ast->flags << FunctionDecl::isEntryPoint;

            /* Decorate program's input and output semantics */
            for (auto& param : ast->parameters)
                program_->inputSemantics.parameters.push_back(param.get());

            program_->outputSemantics.returnType = ast->returnType.get();
            program_->outputSemantics.functionSemantic = ast->semantic;

            /* Add flags to input- and output parameters of the main entry point */
            DecorateEntryInOut(ast->returnType.get(), false);
            for (auto& param : ast->parameters)
                DecorateEntryInOut(param.get(), true);

            /* Check if fragment shader use a slightly different screen space (VPOS vs. SV_Position) */
            if (shaderTarget_ == ShaderTarget::FragmentShader && versionIn_ <= InputShaderVersion::HLSL3)
                program_->flags << Program::hasSM3ScreenSpace;
        }

        #endif

        /* Visit function body */
        PushFunctionDeclLevel(isEntryPoint);
        {
            Visit(ast->codeBlock);
        }
        PopFunctionDeclLevel();
    }
    CloseScope();

    GetReportHandler().PopContextDesc();
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    for (auto& member : ast->members)
    {
        Visit(member);

        /* Decorate all members with a reference to this buffer declaration */
        for (auto& varDecl : member->varDecls)
            varDecl->bufferDeclRef = ast;
    }
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    /* Register all texture declarations */
    for (auto& name : ast->textureDecls)
        Register(name->ident, ast);
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    /* Register all sampler declarations */
    for (auto& name : ast->samplerDecls)
        Register(name->ident, ast);
}

IMPLEMENT_VISIT_PROC(StructDeclStmnt)
{
    Visit(ast->structDecl);
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);
    Visit(ast->varDecls);

    /* Decorate variable type */
    if (InsideEntryPoint() && ast->varDecls.empty())
    {
        auto symbolRef = ast->varType->symbolRef;
        if (symbolRef && symbolRef->Type() == AST::Types::StructDecl)
        {
            auto structDecl = dynamic_cast<StructDecl*>(symbolRef);
            if (structDecl && structDecl->flags(StructDecl::isShaderOutput) && structDecl->aliasName.empty())
            {
                /* Store alias name for shader output interface block */
                structDecl->aliasName = ast->varDecls.front()->ident;
            }
        }
    }
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    WarningOnNullStmnt(ast->bodyStmnt, "for loop");

    Visit(ast->attribs);

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
    WarningOnNullStmnt(ast->bodyStmnt, "while loop");

    Visit(ast->attribs);

    OpenScope();
    {
        Visit(ast->condition);
        Visit(ast->bodyStmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    WarningOnNullStmnt(ast->bodyStmnt, "do-while loop");

    Visit(ast->attribs);

    OpenScope();
    {
        Visit(ast->bodyStmnt);
        Visit(ast->condition);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    WarningOnNullStmnt(ast->bodyStmnt, "if");

    Visit(ast->attribs);

    OpenScope();
    {
        Visit(ast->condition);
        Visit(ast->bodyStmnt);
    }
    CloseScope();

    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    WarningOnNullStmnt(ast->bodyStmnt, "else");

    OpenScope();
    {
        Visit(ast->bodyStmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    Visit(ast->attribs);

    OpenScope();
    {
        Visit(ast->selector);
        Visit(ast->cases);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(AssignStmnt)
{
    DecorateVarObjectSymbol(ast);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    Visit(ast->expr);

    /* Validate expression type by just calling the getter */
    GetTypeDenoterFrom(ast->expr.get());
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Visit(ast->expr);

    /* Analyze entry point return statement */
    if (InsideEntryPoint() && ast->expr->Type() == AST::Types::VarAccessExpr)
    {
        auto varAccessExpr = dynamic_cast<VarAccessExpr*>(ast->expr.get());
        if (varAccessExpr && varAccessExpr->varIdent->symbolRef)
        {
            auto varObject = varAccessExpr->varIdent->symbolRef;
            if (varObject->Type() == AST::Types::VarDecl)
            {
                auto varDecl = dynamic_cast<VarDecl*>(varObject);
                if (varDecl && varDecl->declStmntRef && varDecl->declStmntRef->varType)
                {
                    /*
                    Variable declaration statement has been found,
                    now find the structure object to add the alias name for the interface block.
                    */
                    auto varType = varDecl->declStmntRef->varType.get();
                    if (varType->symbolRef && varType->symbolRef->Type() == AST::Types::StructDecl)
                    {
                        auto structDecl = dynamic_cast<StructDecl*>(varType->symbolRef);
                        if (structDecl)
                        {
                            /* Store alias name for the interface block */
                            structDecl->aliasName = varAccessExpr->varIdent->ident;

                            /*
                            Don't generate code for this variable declaration,
                            because this variable is now already used as interface block.
                            */
                            varDecl->flags << VarDecl::disableCodeGen;
                        }
                    }
                }
            }
        }
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Decorate AST */
    DecorateVarObjectSymbol(ast);

    /* Visit optional assign expression */
    Visit(ast->assignExpr);
}

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(VarType)
{
    Visit(ast->structDecl);

    if (ast->typeDenoter)
    {
        AnalyzeTypeDenoter(ast->typeDenoter, ast);

        //TODO: remove this
        #if 1
        if (!ast->typeDenoter->Ident().empty())
        {
            /* Decorate variable type */
            auto symbol = Fetch(ast->typeDenoter->Ident());
            if (symbol)
                ast->symbolRef = symbol;
        }
        #endif
    }
    else
        Error("missing variable type", ast);
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for context analysis --- */

//INCOMPLETE!
void HLSLAnalyzer::DecorateEntryInOut(VarDeclStmnt* ast, bool isInput)
{
    const auto structFlag = (isInput ? StructDecl::isShaderInput : StructDecl::isShaderOutput);

    /* Add flag to variable declaration statement */
    ast->flags << (isInput ? VarDeclStmnt::isShaderInput : VarDeclStmnt::isShaderOutput);

    /* Add flag to structure type */
    auto& varType = ast->varType;
    if (varType->structDecl)
        varType->structDecl->flags << structFlag;

    /* Add flag to optional symbol reference */
    auto& symbolRef = varType->symbolRef;
    if (symbolRef && symbolRef->Type() == AST::Types::StructDecl)
    {
        auto structDecl = dynamic_cast<StructDecl*>(symbolRef);
        if (structDecl)
        {
            structDecl->flags << structFlag;
            if (!ast->varDecls.empty())
            {
                /*
                Set structure alias name;
                This will be the name of the shader interface block
                */
                structDecl->aliasName = ast->varDecls.front()->ident;
            }
        }
    }
}

//INCOMPLETE!
void HLSLAnalyzer::DecorateEntryInOut(VarType* ast, bool isInput)
{
    const auto structFlag = (isInput ? StructDecl::isShaderInput : StructDecl::isShaderOutput);

    /* Add flag to structure type */
    if (ast->structDecl)
        ast->structDecl->flags << structFlag;

    /* Add flag to optional symbol reference */
    auto& symbolRef = ast->symbolRef;
    if (symbolRef && symbolRef->Type() == AST::Types::StructDecl)
    {
        auto structDecl = dynamic_cast<StructDecl*>(symbolRef);
        if (structDecl)
            structDecl->flags << structFlag;
    }
}

//TODO: refacotre this function!!!
void HLSLAnalyzer::DecorateVarObject(AST* symbol, VarIdent* varIdent)
{
    /* Decorate variable identifier with this symbol */
    varIdent->symbolRef = symbol;

    if (symbol->Type() == AST::Types::VarDecl)
    {
        auto varDecl = dynamic_cast<VarDecl*>(symbol);
        if (varDecl)
        {
            /* Check if this identifier contains a system semantic (SV_...) */
            FetchSystemValueSemantic(varDecl->semantics, varIdent->systemSemantic);

            /* Check if the next identifiers contain a system semantic in their respective structure member */
            if (varDecl->declStmntRef)
            {
                auto varTypeSymbol = varDecl->declStmntRef->varType->symbolRef;
                if (varTypeSymbol && varTypeSymbol->Type() == AST::Types::StructDecl)
                {
                    auto structSymbol = dynamic_cast<StructDecl*>(varTypeSymbol);
                    if (structSymbol)
                    {
                        auto ident = varIdent->next.get();
                        while (ident)
                        {
                            /* Search member in structure */
                            auto systemVal = structSymbol->systemValuesRef.find(ident->ident);
                            if (systemVal != structSymbol->systemValuesRef.end())
                                FetchSystemValueSemantic(systemVal->second->semantics, ident->systemSemantic);

                            /* Check next identifier */
                            ident = ident->next.get();
                        }
                    }
                }
            }

            /* Append prefix to local variables */
            if (varDecl->flags(VarDecl::isInsideFunc))
                varIdent->ident = localVarPrefix_ + varIdent->ident;
        }
    }
    else if (symbol->Type() == AST::Types::SamplerDeclStmnt)
    {
        /* Exchange sampler object by its respective texture object () */
        auto samplerDecl = dynamic_cast<SamplerDeclStmnt*>(symbol);
        auto currentFunc = ActiveFunctionCall();
        if (samplerDecl && currentFunc && currentFunc->flags(FunctionCall::isTexFunc))
            varIdent->ident = currentFunc->varIdent->ident;
    }
}

bool HLSLAnalyzer::FetchSystemValueSemantic(const std::vector<VarSemanticPtr>& varSemantics, std::string& semanticName) const
{
    for (auto& semantic : varSemantics)
    {
        if (IsSystemValueSemnatic(semantic->semantic))
        {
            semanticName = semantic->semantic;
            return true;
        }
    }
    return false;
}

bool HLSLAnalyzer::IsSystemValueSemnatic(std::string semantic) const
{
    if (semantic.size() > 3)
    {
        std::transform(semantic.begin(), semantic.begin() + 2, semantic.begin(), ::toupper);
        return semantic.substr(0, 3) == "SV_";
    }
    return false;
}

void HLSLAnalyzer::AnalyzeFunctionCallStandard(FunctionCall* ast)
{
    /* Decorate function identifier (if it's a member function) */
    if (ast->varIdent->next)
    {
        //TODO: refactor member functions!

        auto symbol = Fetch(ast->varIdent->ident);
        if (symbol)
        {
            if (symbol->Type() == AST::Types::TextureDeclStmnt)
                ast->flags << FunctionCall::isTexFunc;
        }
        else
            ErrorUndeclaredIdent(ast->varIdent->ident, ast);
    }
    else
    {
        /* Fetch function declaratino by arguments */
        ast->funcDeclRef = FetchFunctionDecl(ast->varIdent->ident, ast->arguments, ast);
    }
}

void HLSLAnalyzer::AnalyzeFunctionCallIntrinsic(FunctionCall* ast, const std::string& ident, const HLSLIntrinsicEntry& intr)
{
    //TODO: refactor analysis of intrinsic function call arguments!

    /* Check if a specific intrinsic is used */
    if (ident == "mul")
    {
        ast->flags << FunctionCall::isMulFunc;

        /* Validate number of arguments */
        for (std::size_t i = 2; i < ast->arguments.size(); ++i)
            Error("too many arguments in \"mul\" intrinsic", ast->arguments[i].get());
    }
    else if (ident == "rcp")
        ast->flags << FunctionCall::isRcpFunc;
    else
    {
        auto it = intrinsicMap_.find(ident);
        if (it != intrinsicMap_.end())
        {
            switch (it->second)
            {
                case IntrinsicClasses::Interlocked:
                    ast->flags << FunctionCall::isAtomicFunc;
                    if (ast->arguments.size() < 2)
                        Error("interlocked intrinsics must have at least 2 arguments", ast);
                    //program_->flags << Program::interlockedIntrinsicsUsed;
                    break;
            }
        }
    }
}

/* --- Helper templates for context analysis --- */

template <typename T>
void HLSLAnalyzer::DecorateVarObjectSymbol(T ast)
{
    auto symbol = Fetch(ast->varIdent->ident);
    if (symbol)
        DecorateVarObject(symbol, ast->varIdent.get());
    else
        ErrorUndeclaredIdent(ast->varIdent->ident, ast);
}


} // /namespace Xsc



// ================================================================================