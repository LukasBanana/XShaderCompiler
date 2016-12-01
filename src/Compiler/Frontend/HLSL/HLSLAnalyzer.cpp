/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLAnalyzer.h"
#include "HLSLIntrinsics.h"
#include "EndOfScopeAnalyzer.h"
#include "Exception.h"
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
}

void HLSLAnalyzer::DecorateASTPrimary(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store parameters */
    entryPoint_     = inputDesc.entryPoint;
    shaderTarget_   = inputDesc.shaderTarget;
    versionIn_      = inputDesc.shaderVersion;
    shaderModel_    = GetShaderModel(inputDesc.shaderVersion);

    /* Decorate program AST */
    program_ = &program;

    Visit(&program);
}


/*
 * ======= Private: =======
 */

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

IMPLEMENT_VISIT_PROC(VarSemantic)
{
    AnalyzeSemantic(ast->semantic);
}

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

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    Register(ast->ident, ast);

    Visit(ast->arrayDims);

    for (auto& varSem : ast->semantics)
    {
        Visit(varSem);

        /* Store references to members with system value semantic (SV_...) in all parent structures */
        if (IsSystemSemantic(varSem->semantic))
        {
            for (auto structDecl : GetStructDeclStack())
                structDecl->systemValuesRef[ast->ident] = ast;
        }
    }

    if (ast->initializer)
    {
        Visit(ast->initializer);

        /* Compare initializer type with var-decl type */
        ValidateTypeCastFrom(ast->initializer.get(), ast);
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    /* Find base struct-decl */
    if (!ast->baseStructName.empty())
        ast->baseStructRef = FetchStructDeclFromIdent(ast->baseStructName);

    /* Register struct identifier in symbol table */
    Register(ast->ident, ast);

    PushStructDecl(ast);
    OpenScope();
    {
        Visit(ast->members);
    }
    CloseScope();
    PopStructDecl();
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

    /* Analyze function return semantic */
    AnalyzeSemantic(ast->semantic);

    /* Register function declaration in symbol table */
    Register(ast->ident, ast);

    /* Visit attributes */
    Visit(ast->attribs);

    /* Visit function header */
    Visit(ast->returnType);

    OpenScope();
    {
        Visit(ast->parameters);

        /* Special case for the main entry point */
        if (isEntryPoint)
            AnalyzeEntryPoint(ast);

        /* Visit function body */
        PushFunctionDeclLevel(isEntryPoint);
        {
            Visit(ast->codeBlock);
        }
        PopFunctionDeclLevel();

        /* Analyze last statement of function body ('isEndOfFunction' flag) */
        AnalyzeEndOfScopes(*ast);
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

#if 0
IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);
    Visit(ast->varDecls);

    /* Decorate variable type */
    if (InsideEntryPoint() && ast->varDecls.empty())
    {
        if (auto symbol = ast->varType->symbolRef)
        {
            if (auto structDecl = symbol->As<StructDecl>())
            {
                if (structDecl->flags(StructDecl::isShaderOutput) && structDecl->aliasName.empty())
                {
                    /* Store alias name for shader output interface block */
                    structDecl->aliasName = ast->varDecls.front()->ident;
                }
            }
        }
    }
}
#endif

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

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    Visit(ast->expr);

    /* Validate expression type by just calling the getter */
    GetTypeDenoterFrom(ast->expr.get());
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Visit(ast->expr);

    /* Validate expression type by just calling the getter */
    GetTypeDenoterFrom(ast->expr.get());

    //TODO: refactor this
    #if 1
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
    #endif
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
    AnalyzeTypeDenoter(ast->typeDenoter, ast);
}

IMPLEMENT_VISIT_PROC(SuffixExpr)
{
    Visit(ast->expr);

    /* Left-hand-side of the suffix expression must be either from type structure or base (for vector subscript) */
    auto typeDenoter = ast->expr->GetTypeDenoter()->Get();

    if (auto structTypeDen = typeDenoter->As<StructTypeDenoter>())
    {
        /* Fetch struct member variable declaration from next identifier */
        if (auto memberVarDecl = FetchFromStructDecl(*structTypeDen, ast->varIdent->ident, ast->varIdent.get()))
        {
            /* Analyzer next identifier with fetched symbol */
            AnalyzeVarIdentWithSymbol(ast->varIdent.get(), memberVarDecl);
        }
    }
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    AnalyzeVarIdent(ast->varIdent.get());

    if (ast->assignExpr)
    {
        Visit(ast->assignExpr);
        ValidateTypeCastFrom(ast->assignExpr.get(), ast->varIdent.get());
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for context analysis --- */

#if 0

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

#endif

VarSemanticPtr HLSLAnalyzer::FetchSystemValueSemantic(const std::vector<VarSemanticPtr>& varSemantics) const
{
    for (auto& varSem : varSemantics)
    {
        if (IsSystemSemantic(varSem->semantic))
            return varSem;
    }
    return nullptr;
}

VarSemanticPtr HLSLAnalyzer::FetchUserDefinedSemantic(const std::vector<VarSemanticPtr>& varSemantics) const
{
    for (auto& varSem : varSemantics)
    {
        if (!IsSystemSemantic(varSem->semantic))
            return varSem;
    }
    return nullptr;
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
        try
        {
            auto symbol = Fetch(varIdent->ident);
            if (symbol)
                AnalyzeVarIdentWithSymbol(varIdent, symbol);
            else
                ErrorUndeclaredIdent(varIdent->ident, varIdent);
        }
        catch (const ASTRuntimeError& e)
        {
            Error(e.what(), e.GetAST());
        }
        catch (const std::exception& e)
        {
            Error(e.what(), varIdent);
        }
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
        case AST::Types::StructDecl:
            //...
            break;
        case AST::Types::AliasDecl:
            //...
            break;
        default:
            Error("invalid symbol reference to variable identifier '" + varIdent->ToString() + "'", varIdent);
            break;
    }
}

void HLSLAnalyzer::AnalyzeVarIdentWithSymbolVarDecl(VarIdent* varIdent, VarDecl* varDecl)
{
    /* Decorate next identifier */
    if (varIdent->next)
    {
        /* Has variable a struct type denoter? */
        auto varTypeDen = varDecl->GetTypeDenoter()->GetFromArray(varIdent->arrayIndices.size());
        if (auto structTypeDen = varTypeDen->As<StructTypeDenoter>())
        {
            /* Fetch struct member variable declaration from next identifier */
            if (auto memberVarDecl = FetchFromStructDecl(*structTypeDen, varIdent->next->ident, varIdent))
            {
                /* Analyzer next identifier with fetched symbol */
                AnalyzeVarIdentWithSymbol(varIdent->next.get(), memberVarDecl);
            }
        }
    }

    //TODO: refactor analysis of system value semantics (SV_...)
    #if 1
    /* Check if this identifier contains a system semantic (SV_...) */
    if (auto systemSemantic = FetchSystemValueSemantic(varDecl->semantics))
    {
        varIdent->systemSemantic = systemSemantic->semantic;

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
                        {
                            if (auto identSemantic = FetchSystemValueSemantic(systemVal->second->semantics))
                                ident->systemSemantic = identSemantic->semantic;
                        }

                        /* Check next identifier */
                        ident = ident->next.get();
                    }
                }
            }
        }
    }
    #endif
}

void HLSLAnalyzer::AnalyzeVarIdentWithSymbolTextureDecl(VarIdent* varIdent, TextureDecl* textureDecl)
{
    //TODO...
}

void HLSLAnalyzer::AnalyzeVarIdentWithSymbolSamplerDecl(VarIdent* varIdent, SamplerDecl* samplerDecl)
{
    //TODO...
}

void HLSLAnalyzer::AnalyzeEntryPoint(FunctionDecl* funcDecl)
{
    /* Store reference to entry point in root AST node */
    program_->entryPointRef = funcDecl;

    /* Mark this function declaration with the entry point flag */
    funcDecl->flags << FunctionDecl::isEntryPoint;

    /* Analyze all function parameters */
    for (auto& param : funcDecl->parameters)
    {
        if (param->varDecls.size() == 1)
            AnalyzeEntryPointParameter(funcDecl, param.get());
        else
            Error("invalid number of variable declarations in function parameter", param.get());
    }

    /* Analyze return type */
    auto returnTypeDen = funcDecl->returnType->typeDenoter->Get();
    if (auto structTypeDen = returnTypeDen->As<StructTypeDenoter>())
    {
        /* Analyze entry point output structure */
        AnalyzeEntryPointStructInOut(funcDecl, structTypeDen->structDeclRef, "", false);
    }

    //TODO: refactor this
    #if 0
    /* Decorate program's input and output semantics */
    program_->outputSemantics.returnType = funcDecl->returnType.get();
    program_->outputSemantics.functionSemantic = funcDecl->semantic;

    /* Add flags to input- and output parameters of the main entry point */
    DecorateEntryInOut(funcDecl->returnType.get(), false);
    for (auto& param : funcDecl->parameters)
        DecorateEntryInOut(param.get(), true);

    /* Check if fragment shader use a slightly different screen space (VPOS vs. SV_Position) */
    if (shaderTarget_ == ShaderTarget::FragmentShader && versionIn_ <= InputShaderVersion::HLSL3)
        program_->flags << Program::hasSM3ScreenSpace;
    #endif
}

void HLSLAnalyzer::AnalyzeEntryPointParameter(FunctionDecl* funcDecl, VarDeclStmnt* param)
{
    auto varDecl = param->varDecls.front().get();

    /* Analyze input semantic */
    if (param->IsInput())
        AnalyzeEntryPointParameterInOut(funcDecl, varDecl, true);

    /* Analyze output semantic */
    if (param->IsOutput())
        AnalyzeEntryPointParameterInOut(funcDecl, varDecl, false);
}

void HLSLAnalyzer::AnalyzeEntryPointParameterInOut(FunctionDecl* funcDecl, VarDecl* varDecl, bool input)
{
    auto varTypeDen = varDecl->GetTypeDenoter()->Get();
    if (auto structTypeDen = varTypeDen->As<StructTypeDenoter>())
    {
        /* Analyze entry point structure */
        AnalyzeEntryPointStructInOut(funcDecl, structTypeDen->structDeclRef, varDecl->ident, input);
    }
    else
    {
        /* Has the variable a system value semantic? */
        if (auto varSemantic = varDecl->FirstSemantic())
        {
            if (IsSystemSemantic(varSemantic->semantic))
                varDecl->flags << VarDecl::isSystemValue;
        }
        else
            Error("missing semantic in parameter '" + varDecl->ident + "' of entry point", varDecl);

        /* Add variable declaration to the global input/output semantics */
        if (input)
        {
            funcDecl->inputSemantics.Add(varDecl);
            varDecl->flags << VarDecl::isShaderInput;
        }
        else
        {
            funcDecl->outputSemantics.Add(varDecl);
            varDecl->flags << VarDecl::isShaderOutput;
        }
    }
}

void HLSLAnalyzer::AnalyzeEntryPointStructInOut(FunctionDecl* funcDecl, StructDecl* structDecl, const std::string& structAliasName, bool input)
{
    /* Set structure alias name */
    structDecl->aliasName = structAliasName;

    /* Analyze all structure members */
    for (auto& member : structDecl->members)
    {
        for (auto& memberVar : member->varDecls)
            AnalyzeEntryPointParameterInOut(funcDecl, memberVar.get(), input);
    }

    /* Mark structure as shader input/output */
    if (input)
        structDecl->flags << StructDecl::isShaderInput;
    else
        structDecl->flags << StructDecl::isShaderOutput;
}

void HLSLAnalyzer::AnalyzeSemantic(IndexedSemantic& semantic)
{
    if (semantic == Semantic::Position && shaderTarget_ == ShaderTarget::VertexShader)
    {
        /* Convert shader semantic to VertexPosition */
        semantic = IndexedSemantic(Semantic::VertexPosition, semantic.Index());
    }
}

void HLSLAnalyzer::AnalyzeEndOfScopes(FunctionDecl& funcDecl)
{
    /* Analyze end of scopes from function body */
    EndOfScopeAnalyzer scopeAnalyzer;
    scopeAnalyzer.MarkEndOfScopesFromFunction(funcDecl);
}


} // /namespace Xsc



// ================================================================================
