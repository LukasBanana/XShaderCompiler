/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLAnalyzer.h"
#include "HLSLIntrinsics.h"
#include "Helper.h"


namespace Xsc
{


static ShaderVersion GetShaderModel(const InputShaderVersion v)
{
    switch (v)
    {
        case InputShaderVersion::HLSL3: return { 3, 0 };
        case InputShaderVersion::HLSL4: return { 4, 0 };
        case InputShaderVersion::HLSL5: return { 5, 0 };
    }
    return { 1, 0 };
}

HLSLAnalyzer::HLSLAnalyzer(Log* log) :
    Analyzer{ log }
{
    DeclareIntrinsics();
}

void HLSLAnalyzer::DecorateASTPrimary(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store parameters */
    entryPoint_     = inputDesc.entryPoint;
    shaderTarget_   = inputDesc.shaderTarget;
    versionIn_      = inputDesc.shaderVersion;
    shaderModel_    = GetShaderModel(inputDesc.shaderVersion);
    localVarPrefix_ = outputDesc.formatting.prefix;

    /* Decorate program AST */
    program_ = &program;

    Visit(&program);
}


/*
 * ======= Private: =======
 */

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
    PushFunctionCall(ast);
    {
        /* Analyze function arguments first */
        Visit(ast->arguments);

        /* Then analyze function name */
        if (ast->varIdent)
        {
            auto name = ast->varIdent->ToString();

            /* Check if the function call refers to an intrinsic */
            auto intrIt = HLSLIntrinsics().find(name);
            if (intrIt != HLSLIntrinsics().end())
                AnalyzeFunctionCallIntrinsic(ast, intrIt->second);
            else
                AnalyzeFunctionCallStandard(ast);
        }
    }
    PopFunctionCall();
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

        //TODO: move this to the HLSLParser!
        /* Decorate all members with a reference to this buffer declaration */
        for (auto& varDecl : member->varDecls)
            varDecl->bufferDeclRef = ast;
    }
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    /* Register all texture declarations (register only in the statement) */
    for (auto& textureDecl : ast->textureDecls)
        Register(textureDecl->ident, textureDecl.get());
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    /* Register all sampler declarations (register only in the statement) */
    for (auto& samplerDecl : ast->samplerDecls)
        Register(samplerDecl->ident, samplerDecl.get());
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
    AnalyzeVarIdent(ast->varIdent.get());
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
    AnalyzeVarIdent(ast->varIdent.get());
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

//TODO: replace this function by an enum for all HLSL semantics
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

//TODO: replace this function by an enum for all HLSL semantics
bool HLSLAnalyzer::IsSystemValueSemnatic(std::string semantic) const
{
    return (semantic.size() > 3 && ToUpper(semantic.substr(0, 3)) == "SV_");
}

void HLSLAnalyzer::AnalyzeFunctionCallStandard(FunctionCall* ast)
{
    /* Decorate function identifier (if it's a member function) */
    if (ast->varIdent->next)
    {
        AnalyzeVarIdent(ast->varIdent.get());

        //TODO: refactor member functions!
        #if 1
        if (auto symbol = ast->varIdent->symbolRef)
        {
            if (symbol->Type() == AST::Types::TextureDecl)
                ast->flags << FunctionCall::isTexFunc;
        }
        #endif
    }
    else
    {
        /* Fetch function declaratino by arguments */
        ast->funcDeclRef = FetchFunctionDecl(ast->varIdent->ident, ast->arguments, ast);
    }
}

void HLSLAnalyzer::AnalyzeFunctionCallIntrinsic(FunctionCall* ast, const HLSLIntrinsicEntry& intr)
{
    /* Check shader input version */
    if (shaderModel_ < intr.minShaderModel)
    {
        Warning(
            "intrinsic '" + ast->varIdent->ToString() + "' requires shader model " + intr.minShaderModel.ToString() +
            ", but only " + shaderModel_.ToString() + " is specified", ast
        );
    }

    /* Decorate AST with intrinsic ID */
    ast->intrinsic = intr.intrinsic;

    //TODO: refactor analysis of intrinsic function call arguments!
    #if 1

    /* Check if a specific intrinsic is used */
    switch (intr.intrinsic)
    {
        case Intrinsic::Mul:
        {
            /* Validate number of arguments */
            for (std::size_t i = 2; i < ast->arguments.size(); ++i)
                Error("too many arguments in \"mul\" intrinsic", ast->arguments[i].get());
        }
        break;

        default:
        {
            if (intr.intrinsic >= Intrinsic::InterlockedAdd && intr.intrinsic <= Intrinsic::InterlockedXor)
            {
                if (ast->arguments.size() < 2)
                    Error("interlocked intrinsics must have at least 2 arguments", ast);
                //program_->flags << Program::interlockedIntrinsicsUsed;
            }
        }
        break;
    }

    #endif
}

void HLSLAnalyzer::AnalyzeVarIdent(VarIdent* varIdent)
{
    if (varIdent)
    {
        auto symbol = Fetch(varIdent->ident);
        if (symbol)
            AnalyzeVarIdentWithSymbol(varIdent, symbol);
        else
            ErrorUndeclaredIdent(varIdent->ident, varIdent);
    }
}

void HLSLAnalyzer::AnalyzeVarIdentWithSymbol(VarIdent* varIdent, AST* symbol)
{
    /* Decorate variable identifier with this symbol */
    varIdent->symbolRef = symbol;

    switch (symbol->Type())
    {
        case AST::Types::VarDecl:
            AnalyzeVarIdentWithSymbolVarDecl(varIdent, static_cast<VarDecl*>(symbol));
            break;
        case AST::Types::TextureDecl:
            AnalyzeVarIdentWithSymbolTextureDecl(varIdent, static_cast<TextureDecl*>(symbol));
            break;
        case AST::Types::SamplerDecl:
            AnalyzeVarIdentWithSymbolSamplerDecl(varIdent, static_cast<SamplerDecl*>(symbol));
            break;
        default:
            Error("invalid symbol reference to variable identifier '" + varIdent->ToString() + "'", varIdent);
            break;
    }
}

//TODO: refactor this function
void HLSLAnalyzer::AnalyzeVarIdentWithSymbolVarDecl(VarIdent* varIdent, VarDecl* varDecl)
{
    /* Check if this identifier contains a system semantic (SV_...) */
    FetchSystemValueSemantic(varDecl->semantics, varIdent->systemSemantic);

    #if 1//??????
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
    #endif

    /* Append prefix to local variables */
    if (varDecl->flags(VarDecl::isInsideFunc))
        varIdent->ident = localVarPrefix_ + varIdent->ident;
}

void HLSLAnalyzer::AnalyzeVarIdentWithSymbolTextureDecl(VarIdent* varIdent, TextureDecl* textureDecl)
{
    //TODO...
}

//TODO: refactor this function
void HLSLAnalyzer::AnalyzeVarIdentWithSymbolSamplerDecl(VarIdent* varIdent, SamplerDecl* samplerDecl)
{
    /* Exchange sampler object by its respective texture object () */
    auto currentFunc = ActiveFunctionCall();
    if (currentFunc && currentFunc->flags(FunctionCall::isTexFunc))
        varIdent->ident = currentFunc->varIdent->ident;
}


} // /namespace Xsc



// ================================================================================