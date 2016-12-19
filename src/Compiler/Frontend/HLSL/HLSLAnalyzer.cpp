/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLAnalyzer.h"
#include "HLSLIntrinsics.h"
#include "HLSLKeywords.h"
#include "ConstExprEvaluator.h"
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
    entryPoint_             = inputDesc.entryPoint;
    secondaryEntryPoint_    = inputDesc.secondaryEntryPoint;
    shaderTarget_           = inputDesc.shaderTarget;
    versionIn_              = inputDesc.shaderVersion;
    shaderModel_            = GetShaderModel(inputDesc.shaderVersion);
    preferWrappers_         = outputDesc.options.preferWrappers;

    /* Decorate program AST */
    program_ = &program;

    Visit(&program);

    /* Check if secondary entry point has been found */
    if (!secondaryEntryPoint_.empty() && !secondaryEntryPointFound_)
        Warning("secondary entry point \"" + secondaryEntryPoint_ + "\" not found");
}


/*
 * ======= Private: =======
 */

void HLSLAnalyzer::ErrorIfAttributeNotFound(bool found, const std::string& attribDesc)
{
    if (!found)
        Error("missing '" + attribDesc + "' attribute for entry point", nullptr, HLSLErr::ERR_ATTRIBUTE);
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void HLSLAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Analyze context of the entire program */
    Visit(ast->globalStmnts);

    /* Check if fragment shader uses a slightly different screen space (VPOS vs. SV_Position) */
    if (shaderTarget_ == ShaderTarget::FragmentShader && versionIn_ <= InputShaderVersion::HLSL3)
        program_->layoutFragment.pixelCenterInteger = true;
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
            if (ast->varIdent->next)
            {
                /* Check if the function call refers to an intrinsic */
                auto intrIt = HLSLIntrinsics().find(ast->varIdent->next->ident);
                if (intrIt != HLSLIntrinsics().end())
                {
                    auto intrinsic = intrIt->second.intrinsic;

                    /* Analyze variable identifier (symbolRef is needed next) */
                    AnalyzeVarIdent(ast->varIdent.get());

                    /* Verify intrinsic for respective object class */
                    switch (ast->varIdent->symbolRef->Type())
                    {
                        case AST::Types::BufferDecl:
                            if (!IsTextureIntrinsic(intrinsic))
                                Error("invalid intrinsic '" + ast->varIdent->next->ident + "' for a texture object", ast);
                            break;

                        default:
                            break;
                    }

                    AnalyzeFunctionCallIntrinsic(ast, intrIt->second);
                }
                else
                    AnalyzeFunctionCallStandard(ast);
            }
            else
            {
                /* Check if the function call refers to an intrinsic */
                auto intrIt = HLSLIntrinsics().find(ast->varIdent->ident);
                if (intrIt != HLSLIntrinsics().end())
                    AnalyzeFunctionCallIntrinsic(ast, intrIt->second);
                else
                    AnalyzeFunctionCallStandard(ast);
            }
        }
    }
    PopFunctionCall();
}

IMPLEMENT_VISIT_PROC(TypeName)
{
    Visit(ast->structDecl);

    if (ast->typeDenoter)
        AnalyzeTypeDenoter(ast->typeDenoter, ast);
    else
        Error("missing variable type", ast);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    Register(ast->ident, ast);

    Visit(ast->arrayDims);

    AnalyzeSemantic(ast->semantic);

    /* Store references to members with system value semantic (SV_...) in all parent structures */
    if (ast->semantic.IsSystemValue())
    {
        for (auto structDecl : GetStructDeclStack())
            structDecl->systemValuesRef[ast->ident] = ast;
    }

    if (ast->initializer)
    {
        Visit(ast->initializer);

        /* Compare initializer type with var-decl type */
        ValidateTypeCastFrom(ast->initializer.get(), ast, "variable initialization");
    }
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    /* Register identifier for buffer */
    Register(ast->ident, ast);
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    /* Register identifier for sampler */
    Register(ast->ident, ast);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    /* Find base struct-decl */
    if (!ast->baseStructName.empty())
        ast->baseStructRef = FetchStructDeclFromIdent(ast->baseStructName);

    /* Register struct identifier in symbol table */
    Register(ast->ident, ast);

    PushStructDecl(ast);
    {
        if (ast->flags(StructDecl::isNestedStruct) && !ast->IsAnonymous())
            Error("nested structures must be anonymous", ast);

        OpenScope();
        {
            Visit(ast->members);
        }
        CloseScope();
    }
    PopStructDecl();

    /* Report warning if structure is empty */
    if (ast->NumMembers() == 0)
        Warning("'" + ast->SignatureToString() + "' is completely empty", ast);
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

    /* Check for entry points */
    const auto isEntryPoint             = (ast->ident == entryPoint_);
    const auto isSecondaryEntryPoint    = (ast->ident == secondaryEntryPoint_);

    if (isSecondaryEntryPoint)
        secondaryEntryPointFound_ = true;

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
        else if (isSecondaryEntryPoint)
            AnalyzeSecondaryEntryPoint(ast);

        /* Visit function body */
        PushFunctionDeclLevel(ast);
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
    /* Analyze generic type */
    AnalyzeTypeDenoter(ast->typeDenoter->genericTypeDenoter, ast);

    /* Analyze buffer declarations */
    Visit(ast->bufferDecls);
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    /* Validate buffer slots */
    if (ast->slotRegisters.size() > 1)
        Error("buffers can only be bound to one slot", ast->slotRegisters[1].get(), HLSLErr::ERR_BIND_INVALID);

    for (const auto& slotRegister : ast->slotRegisters)
    {
        if (slotRegister->shaderTarget != ShaderTarget::Undefined)
            Error("user-defined constant buffer slots can not be target specific", slotRegister.get(), HLSLErr::ERR_TARGET_INVALID);
    }

    for (auto& member : ast->members)
    {
        Visit(member);

        //TODO: move this to the HLSLParser!
        /* Decorate all members with a reference to this buffer declaration */
        for (auto& varDecl : member->varDecls)
            varDecl->bufferDeclRef = ast;
    }
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);
    Visit(ast->varDecls);

    /* Is the 'snorm' or 'unorm' type modifier specified? */
    if (ast->HasAnyTypeModifierOf({ TypeModifier::SNorm, TypeModifier::UNorm }))
    {
        /* Is this a floating-point type? */
        auto baseTypeDen = ast->varType->typeDenoter->As<BaseTypeDenoter>();
        if (!baseTypeDen || !IsRealType(baseTypeDen->dataType))
            Error("'snorm' and 'unorm' type modifiers can only be used for floating-point types", ast->varType.get());
    }

    #if 0
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
    #endif
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

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    Visit(ast->expr);

    /* Validate expression type by just calling the getter */
    GetTypeDenoterFrom(ast->expr.get());

    /* Analyze wrapper inlining for intrinsic calls */
    if (!preferWrappers_)
    {
        if (auto funcCallExpr = ast->expr->As<FunctionCallExpr>())
            AnalyzeIntrinsicWrapperInlining(funcCallExpr->call.get());
    }
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    if (ast->expr)
    {
        Visit(ast->expr);

        /* Validate expression type by just calling the getter */
        GetTypeDenoterFrom(ast->expr.get());

        //TODO: refactor this
        #if 1
        /* Analyze entry point return statement */
        if (InsideEntryPoint())
        {
            if (auto varAccessExpr = ast->expr->As<VarAccessExpr>())
            {
                if (auto varSymbolRef = varAccessExpr->varIdent->symbolRef)
                {
                    if (auto varDecl = varSymbolRef->As<VarDecl>())
                    {
                        if (varDecl->declStmntRef)
                        {
                            /*
                            Variable declaration statement has been found,
                            now find the structure object to add the alias name for the interface block.
                            */
                            if (auto structSymbolRef = varDecl->GetTypeDenoter()->SymbolRef())
                            {
                                if (auto structDecl = structSymbolRef->As<StructDecl>())
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
        #endif
    }
}

/* --- Expressions --- */

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
        ValidateTypeCastFrom(ast->assignExpr.get(), ast->varIdent.get(), "variable assignment");

        /* Is the variable a valid l-value? */
        if (auto constIdent = ast->varIdent->FirstConstVarIdent())
            Error("illegal assignment to l-value '" + constIdent->ident + "' that is declared as constant", ast);
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for context analysis --- */

void HLSLAnalyzer::AnalyzeFunctionCallStandard(FunctionCall* ast)
{
    if (ast->varIdent->next)
    {
        /* Analyze function identifier (if it's a member function) */
        AnalyzeVarIdent(ast->varIdent.get());
    }
    else
    {
        /* Fetch function declaration by arguments */
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

    /* Analyze special intrinsic types */
    using T = Intrinsic;

    struct IntrinsicConversion
    {
        T   standardIntrinsic;
        int numArgs;
        T   overloadedIntrinsic;
    };

    static const std::vector<IntrinsicConversion> intrinsicConversions
    {
        { T::AsUInt_1,              3, T::AsUInt_3              },
        { T::Tex1D_2,               4, T::Tex1D_4               },
        { T::Tex2D_2,               4, T::Tex2D_4               },
        { T::Tex3D_2,               4, T::Tex3D_4               },
        { T::TexCube_2,             4, T::TexCube_4             },
        { T::Texture_Load_1,        2, T::Texture_Load_2        },
        { T::Texture_Load_1,        3, T::Texture_Load_3        },
        { T::Texture_Sample_2,      3, T::Texture_Sample_3      },
        { T::Texture_Sample_2,      4, T::Texture_Sample_4      },
        { T::Texture_Sample_2,      5, T::Texture_Sample_5      },
        { T::Texture_SampleBias_3,  4, T::Texture_SampleBias_4  },
        { T::Texture_SampleBias_3,  5, T::Texture_SampleBias_5  },
        { T::Texture_SampleBias_3,  6, T::Texture_SampleBias_6  },
        { T::Texture_SampleCmp_3,   4, T::Texture_SampleCmp_4   },
        { T::Texture_SampleCmp_3,   5, T::Texture_SampleCmp_5   },
        { T::Texture_SampleCmp_3,   6, T::Texture_SampleCmp_6   },
        { T::Texture_SampleGrad_4,  5, T::Texture_SampleGrad_5  },
        { T::Texture_SampleGrad_4,  6, T::Texture_SampleGrad_6  },
        { T::Texture_SampleGrad_4,  7, T::Texture_SampleGrad_7  },
        { T::Texture_SampleLevel_3, 4, T::Texture_SampleLevel_4 },
        { T::Texture_SampleLevel_3, 5, T::Texture_SampleLevel_5 },
    };

    for (const auto& conversion : intrinsicConversions)
    {
        /* Is another overloaded version of the intrinsic used? */
        if (ast->intrinsic == conversion.standardIntrinsic && ast->arguments.size() == static_cast<std::size_t>(conversion.numArgs))
        {
            /* Convert intrinsic type */
            ast->intrinsic = conversion.overloadedIntrinsic;
            break;
        }
    }
}

void HLSLAnalyzer::AnalyzeIntrinsicWrapperInlining(FunctionCall* ast)
{
    /* Is this a 'clip'-intrinsic call? */
    if (ast->intrinsic == Intrinsic::Clip)
    {
        /* The wrapper function for this intrinsic can be inlined */
        ast->flags << FunctionCall::canInlineIntrinsicWrapper;
    }
}

/* ----- Variable identifier ----- */

void HLSLAnalyzer::AnalyzeVarIdent(VarIdent* varIdent)
{
    if (varIdent)
    {
        try
        {
            if (auto symbol = Fetch(varIdent->ident))
                AnalyzeVarIdentWithSymbol(varIdent, symbol);
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
        case AST::Types::FunctionDecl:
            //...
            break;
        case AST::Types::VarDecl:
            AnalyzeVarIdentWithSymbolVarDecl(varIdent, static_cast<VarDecl*>(symbol));
            break;
        case AST::Types::BufferDecl:
            //AnalyzeVarIdentWithSymbolBufferDecl(varIdent, static_cast<BufferDecl*>(symbol));
            break;
        case AST::Types::SamplerDecl:
            //AnalyzeVarIdentWithSymbolSamplerDecl(varIdent, static_cast<SamplerDecl*>(symbol));
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
        try
        {
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
        catch (const std::exception& e)
        {
            Error(e.what(), varIdent);
        }
    }
}

/*void HLSLAnalyzer::AnalyzeVarIdentWithSymbolBufferDecl(VarIdent* varIdent, BufferDecl* bufferDecl)
{
    //TODO...
}

void HLSLAnalyzer::AnalyzeVarIdentWithSymbolSamplerDecl(VarIdent* varIdent, SamplerDecl* samplerDecl)
{
    //TODO...
}*/

/* ----- Entry point ----- */

void HLSLAnalyzer::AnalyzeEntryPoint(FunctionDecl* funcDecl)
{
    /* Mark this function declaration with the entry point flag */
    if (funcDecl->flags.SetOnce(FunctionDecl::isEntryPoint))
    {
        /* Store reference to entry point in root AST node */
        program_->entryPointRef = funcDecl;

        /* Analyze function input/output */
        AnalyzeEntryPointInputOutput(funcDecl);

        /* Analyze entry pointer attributes (also possibly missing attributes such as "numthreads" for compute shaders) */
        AnalyzeEntryPointAttributes(funcDecl->attribs);
    }
}

void HLSLAnalyzer::AnalyzeEntryPointInputOutput(FunctionDecl* funcDecl)
{
    /* Analyze all function parameters */
    for (auto& param : funcDecl->parameters)
    {
        if (param->varDecls.size() == 1)
            AnalyzeEntryPointParameter(funcDecl, param.get());
        else
            Error("invalid number of variable declarations in function parameter", param.get());
    }

    /* Analyze function return type */
    auto returnTypeDen = funcDecl->returnType->typeDenoter->Get();
    if (auto structTypeDen = returnTypeDen->As<StructTypeDenoter>())
    {
        /* Analyze entry point output structure */
        AnalyzeEntryPointParameterInOutStruct(funcDecl, structTypeDen->structDeclRef, "", false);
    }
}

void HLSLAnalyzer::AnalyzeEntryPointParameter(FunctionDecl* funcDecl, VarDeclStmnt* param)
{
    auto varDecl = param->varDecls.front().get();

    if (param->isUniform)
    {
        /* Verify input only semantic */
        if (param->IsOutput())
            Error("uniforms can not be defined as output", varDecl);
    }
    else
    {
        /* Analyze input semantic */
        if (param->IsInput())
            AnalyzeEntryPointParameterInOut(funcDecl, varDecl, true);

        /* Analyze output semantic */
        if (param->IsOutput())
            AnalyzeEntryPointParameterInOut(funcDecl, varDecl, false);
    }
}

void HLSLAnalyzer::AnalyzeEntryPointParameterInOut(FunctionDecl* funcDecl, VarDecl* varDecl, bool input)
{
    auto varTypeDen = varDecl->GetTypeDenoter()->Get();
    if (auto structTypeDen = varTypeDen->As<StructTypeDenoter>())
    {
        /* Analyze entry point structure */
        AnalyzeEntryPointParameterInOutStruct(funcDecl, structTypeDen->structDeclRef, varDecl->ident, input);
    }
    else if (auto bufferTypeDen = varTypeDen->As<BufferTypeDenoter>())
    {
        /* Analyze entry point buffer */
        AnalyzeEntryPointParameterInOutBuffer(funcDecl, varDecl, bufferTypeDen, input);
    }
    else
    {
        /* Has the variable a system value semantic? */
        if (varDecl->semantic.IsValid())
        {
            if (varDecl->semantic.IsSystemValue())
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

void HLSLAnalyzer::AnalyzeEntryPointParameterInOutStruct(FunctionDecl* funcDecl, StructDecl* structDecl, const std::string& structAliasName, bool input)
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

void HLSLAnalyzer::AnalyzeEntryPointParameterInOutBuffer(FunctionDecl* funcDecl, VarDecl* varDecl, BufferTypeDenoter* bufferTypeDen, bool input)
{
    if (IsPatchBufferType(bufferTypeDen->bufferType))
    {
        //TODO...
    }
    else if (IsStreamBufferType(bufferTypeDen->bufferType))
    {
        //TODO...
    }
    else
        Error("illegal buffer type for entry pointer " + std::string(input ? "input" : "output"), varDecl);
}

void HLSLAnalyzer::AnalyzeEntryPointAttributes(const std::vector<AttributePtr>& attribs)
{
    switch (shaderTarget_)
    {
        case ShaderTarget::TessellationControlShader:
            AnalyzeEntryPointAttributesTessControlShader(attribs);
            break;
        case ShaderTarget::TessellationEvaluationShader:
            AnalyzeEntryPointAttributesTessEvaluationShader(attribs);
            break;
        case ShaderTarget::FragmentShader:
            AnalyzeEntryPointAttributesFragmentShader(attribs);
            break;
        case ShaderTarget::ComputeShader:
            AnalyzeEntryPointAttributesComputeShader(attribs);
            break;
        default:
            break;
    }
}

void HLSLAnalyzer::AnalyzeEntryPointAttributesTessControlShader(const std::vector<AttributePtr>& attribs)
{
    bool foundDomain                = false;
    bool foundOutputControlPoints   = false;
    bool foundOutputTopology        = false;
    bool foundPartitioning          = false;
    bool foundPatchConstantFunc     = false;

    /* Analyze required attributes */
    for (const auto& attr : attribs)
    {
        switch (attr->attributeType)
        {
            case AttributeType::Domain:
                AnalyzeAttributeDomain(attr.get());
                foundDomain = true;
                break;

            case AttributeType::OutputControlPoints:
                AnalyzeAttributeOutputControlPoints(attr.get());
                foundOutputControlPoints = true;
                break;

            case AttributeType::OutputTopology:
                AnalyzeAttributeOutputTopology(attr.get());
                foundOutputTopology = true;
                break;

            case AttributeType::Partitioning:
                AnalyzeAttributePartitioning(attr.get());
                foundPartitioning = true;
                break;

            case AttributeType::PatchConstantFunc:
                AnalyzeAttributePatchConstantFunc(attr.get());
                foundPatchConstantFunc = true;
                break;

            default:
                break;
        }
    }

    /* Check for missing attributes */
    ErrorIfAttributeNotFound(foundDomain, "domain(type)");
    ErrorIfAttributeNotFound(foundOutputControlPoints, "outputcontrolpoints(count)");
    ErrorIfAttributeNotFound(foundOutputTopology, "outputtopology(topology)");
    ErrorIfAttributeNotFound(foundPartitioning, "partitioning(mode)");
    ErrorIfAttributeNotFound(foundPatchConstantFunc, "patchconstantfunc(function)");
}

void HLSLAnalyzer::AnalyzeEntryPointAttributesTessEvaluationShader(const std::vector<AttributePtr>& attribs)
{
    bool foundDomain = false;

    /* Analyze required attributes */
    for (const auto& attr : attribs)
    {
        switch (attr->attributeType)
        {
            case AttributeType::Domain:
                AnalyzeAttributeDomain(attr.get());
                foundDomain = true;
                break;

            default:
                break;
        }
    }

    /* Check for missing attributes */
    ErrorIfAttributeNotFound(foundDomain, "domain(type)");
}

void HLSLAnalyzer::AnalyzeEntryPointAttributesFragmentShader(const std::vector<AttributePtr>& attribs)
{
    /* Analyze optional attributes */
    for (const auto& attr : attribs)
    {
        switch (attr->attributeType)
        {
            case AttributeType::EarlyDepthStencil:
                program_->layoutFragment.earlyDepthStencil = true;
                break;
            default:
                break;
        }
    }
}

void HLSLAnalyzer::AnalyzeEntryPointAttributesComputeShader(const std::vector<AttributePtr>& attribs)
{
    bool foundNumThreads = false;

    /* Analyze required attributes */
    for (const auto& attr : attribs)
    {
        switch (attr->attributeType)
        {
            case AttributeType::NumThreads:
                AnalyzeAttributeNumThreads(attr.get());
                foundNumThreads = true;
                break;

            default:
                break;
        }
    }

    /* Check for missing attributes */
    ErrorIfAttributeNotFound(foundNumThreads, "numthreads(x, y, z)");
}

/* ----- Inactive entry point ----- */

void HLSLAnalyzer::AnalyzeSecondaryEntryPoint(FunctionDecl* funcDecl)
{
    /* Mark this function declaration with the entry point flag */
    if (funcDecl->flags.SetOnce(FunctionDecl::isSecondaryEntryPoint))
    {
        /* Store reference to secondary entry point in root AST node */
        program_->layoutTessControl.patchConstFunctionRef = funcDecl;

        /* Analyze function input/output (use same visitor as for the main entry pointer here) */
        AnalyzeEntryPointInputOutput(funcDecl);

        /* Analyze secondary entry pointer attributes */
        AnalyzeSecondaryEntryPointAttributes(funcDecl->attribs);
    }
}

void HLSLAnalyzer::AnalyzeSecondaryEntryPointAttributes(const std::vector<AttributePtr>& attribs)
{
    /*
    The secondary entry point can be a function that is an entry point for another shader target.
    This is used to detect the entry point attributes from the tessellation-control shader,
    that are required for the tessellation-evaluation shader in GLSL (e.g. [partitioning(fractional_odd)] -> layout(fractional_odd_spacing)).
    */
    switch (shaderTarget_)
    {
        case ShaderTarget::TessellationEvaluationShader:
            AnalyzeSecondaryEntryPointAttributesTessEvaluationShader(attribs);
            break;
        default:
            break;
    }
}

void HLSLAnalyzer::AnalyzeSecondaryEntryPointAttributesTessEvaluationShader(const std::vector<AttributePtr>& attribs)
{
    /* Analyze optional attributes */
    for (const auto& attr : attribs)
    {
        switch (attr->attributeType)
        {
            case AttributeType::OutputTopology:
                AnalyzeAttributeOutputTopology(attr.get(), false);
                break;
            case AttributeType::Partitioning:
                AnalyzeAttributePartitioning(attr.get(), false);
                break;
            default:
                break;
        }
    }
}

/* ----- Attributes ----- */

bool HLSLAnalyzer::AnalyzeNumArgsAttribute(Attribute* ast, std::size_t expectedNumArgs, bool required)
{
    /* Validate number of arguments */
    auto numArgs = ast->arguments.size();

    if (numArgs < expectedNumArgs)
    {
        if (required)
        {
            Error(
                "too few arguments in attribute (expected " + std::to_string(expectedNumArgs) +
                ", but got " + std::to_string(numArgs) + ")", ast, HLSLErr::ERR_ATTRIBUTE
            );
        }
    }
    else if (numArgs > expectedNumArgs)
    {
        if (required)
        {
            Error(
                "too many arguments in attribute (expected " + std::to_string(expectedNumArgs) +
                ", but got " + std::to_string(numArgs) + ")", ast->arguments[expectedNumArgs].get(), HLSLErr::ERR_ATTRIBUTE
            );
        }
    }
    else
        return true;

    return false;
}

void HLSLAnalyzer::AnalyzeAttributeDomain(Attribute* ast, bool required)
{
    if (AnalyzeNumArgsAttribute(ast, 1, required))
    {
        AnalyzeAttributeValue(
            ast->arguments[0].get(),
            program_->layoutTessEvaluation.domainType,
            IsAttributeValueDomain,
            "expected domain type parameter to be \"tri\", \"quad\", or \"isolane\"",
            HLSLErr::ERR_HSATTRIBUTE_INVALID,
            required
        );
    }
}

void HLSLAnalyzer::AnalyzeAttributeOutputTopology(Attribute* ast, bool required)
{
    if (AnalyzeNumArgsAttribute(ast, 1, required))
    {
        AnalyzeAttributeValue(
            ast->arguments[0].get(),
            program_->layoutTessEvaluation.outputTopology,
            IsAttributeValueOutputTopology,
            "expected output topology parameter to be \"point\", \"line\", \"triangle_cw\", or \"triangle_ccw\"",
            HLSLErr::ERR_HSATTRIBUTE_INVALID,
            required
        );
    }
}

void HLSLAnalyzer::AnalyzeAttributePartitioning(Attribute* ast, bool required)
{
    if (AnalyzeNumArgsAttribute(ast, 1, required))
    {
        AnalyzeAttributeValue(
            ast->arguments[0].get(),
            program_->layoutTessEvaluation.partitioning,
            IsAttributeValuePartitioning,
            "expected partitioning mode parameter to be \"integer\", \"pow2\", \"fractional_even\", or \"fractional_odd\"",
            HLSLErr::ERR_HSATTRIBUTE_INVALID,
            required
        );
    }
}

void HLSLAnalyzer::AnalyzeAttributeOutputControlPoints(Attribute* ast)
{
    if (AnalyzeNumArgsAttribute(ast, 1))
    {
        /* Get integer literal value and convert to integer */
        auto countParamVariant = EvaluateConstExpr(*ast->arguments[0]);

        int countParam = -1;
        if (countParamVariant.Type() == Variant::Types::Int)
            countParam = static_cast<int>(countParamVariant.Int());

        if (countParam >= 0)
            program_->layoutTessControl.outputControlPoints = static_cast<unsigned int>(countParam);
        else
            Error("expected output control point parameter to be an unsigned integer", ast->arguments[0].get(), HLSLErr::ERR_ATTRIBUTE);
    }
}

void HLSLAnalyzer::AnalyzeAttributePatchConstantFunc(Attribute* ast)
{
    if (AnalyzeNumArgsAttribute(ast, 1))
    {
        auto literalExpr = ast->arguments[0]->As<LiteralExpr>();
        if (literalExpr && literalExpr->dataType == DataType::String)
        {
            /* Get string literal value, and fetch function name */
            auto literalValue = literalExpr->GetStringValue();

            /* Fetch patch constant function entry point */
            if (auto patchConstFunc = FetchFunctionDecl(literalValue))
            {
                /* Decorate patch constant function as reachable (since it's referenced by the main entry point) */
                AnalyzeSecondaryEntryPoint(patchConstFunc);
            }
            else
                Error("entry point '" + literalValue + "' for patch constant function not found", ast->arguments[0].get(), HLSLErr::ERR_ENTRYPOINT_NOT_FOUND);
        }
        else
            Error("expected patch constant function parameter to be a string literal", ast->arguments[0].get(), HLSLErr::ERR_ATTRIBUTE);
    }
}

void HLSLAnalyzer::AnalyzeAttributeNumThreads(Attribute* ast)
{
    if (AnalyzeNumArgsAttribute(ast, 3))
    {
        /* Evaluate and store all three thread counts in global layout */
        for (int i = 0; i < 3; ++i)
        {
            AnalyzeAttributeNumThreadsArgument(
                ast->arguments[i].get(),
                program_->layoutCompute.numThreads[i]
            );
        }
    }
}

void HLSLAnalyzer::AnalyzeAttributeNumThreadsArgument(Expr* ast, unsigned int& value)
{
    int exprValue = EvaluateConstExprInt(*ast);
    if (exprValue > 0)
        value = static_cast<unsigned int>(exprValue);
    else
        Error("number of threads must be greater than zero", ast);
}

void HLSLAnalyzer::AnalyzeAttributeValue(
    Expr* argExpr, AttributeValue& value, const OnValidAttributeValueProc& expectedValueFunc,
    const std::string& expectationDesc, const HLSLErr errorCode, bool required)
{
    std::string literalValue;
    if (!AnalyzeAttributeValuePrimary(argExpr, value, expectedValueFunc, literalValue) && required)
    {
        if (literalValue.empty())
            Error(expectationDesc, argExpr, errorCode);
        else
            Error(expectationDesc + ", but got '" + literalValue + "'", argExpr, errorCode);
    }
}

bool HLSLAnalyzer::AnalyzeAttributeValuePrimary(
    Expr* argExpr, AttributeValue& value, const OnValidAttributeValueProc& expectedValueFunc, std::string& literalValue)
{
    if (auto literalExpr = argExpr->As<LiteralExpr>())
    {
        /* Get string literal value, convert to enum entry, and search in expected value list */
        literalValue = literalExpr->GetStringValue();
        value = HLSLKeywordToAttributeValue(literalValue);
        return expectedValueFunc(value);
    }
    return false;
}

/* ----- Misc ----- */

void HLSLAnalyzer::AnalyzeSemantic(IndexedSemantic& semantic)
{
    if (semantic == Semantic::Position && shaderTarget_ != ShaderTarget::FragmentShader)
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
