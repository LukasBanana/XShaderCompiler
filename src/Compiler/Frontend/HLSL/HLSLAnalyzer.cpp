/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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
                    if (AnalyzeMemberIntrinsic(intrinsic, ast))
                        AnalyzeFunctionCallIntrinsic(ast, intrIt->second);
                }
                else
                    AnalyzeFunctionCallStandard(ast);
            }
            else
            {
                /* Does the function call refer to an intrinsic? */
                auto intrIt = HLSLIntrinsics().find(ast->varIdent->ident);
                if (intrIt != HLSLIntrinsics().end())
                {
                    /* Is this a global intrinsic? */
                    if (IsGlobalIntrinsic(intrIt->second.intrinsic))
                        AnalyzeFunctionCallIntrinsic(ast, intrIt->second);
                    else
                        AnalyzeFunctionCallStandard(ast);
                }
                else
                    AnalyzeFunctionCallStandard(ast);
            }
        }

        /* Analyze all l-value arguments that are assigned to output parameters */
        ast->ForEachOutputArgument(
            [this](Expr* argExpr)
            {
                AnalyzeLValueExpr(argExpr, argExpr);
            }
        );
    }
    PopFunctionCall();
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    if (ast->expr && ast->expr->Type() != AST::Types::NullExpr)
    {
        Visit(ast->expr);

        /* Evalutate constant expression and store as array dimension size */
        auto value = EvaluateConstExprInt(*ast->expr);

        ast->size = value;
    }
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

    AnalyzeArrayDimensionList(ast->arrayDims);
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
        Warning("'" + ast->ToString() + "' is completely empty", ast);
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
    GetReportHandler().PushContextDesc(ast->ToString());

    /* Check for entry points */
    const auto isEntryPoint             = (ast->ident == entryPoint_);
    const auto isSecondaryEntryPoint    = (ast->ident == secondaryEntryPoint_);

    if (isSecondaryEntryPoint && !ast->IsForwardDecl())
        secondaryEntryPointFound_ = true;

    /* Analyze function return semantic */
    AnalyzeSemantic(ast->semantic);

    /* Register function declaration in symbol table */
    Register(ast->ident, ast);

    /* Visit attributes */
    Visit(ast->attribs);

    /* Visit function return type */
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
    /* Check if return expression matches the function return type */
    if (auto funcDecl = ActiveFunctionDecl())
    {
        if (auto returnTypeDen = funcDecl->returnType->GetTypeDenoter())
        {
            if (returnTypeDen->IsVoid())
            {
                if (ast->expr)
                    Error("illegal expression in return statement for function with 'void' return type", ast->expr.get());
            }
            else
            {
                if (!ast->expr)
                    Error("missing expression in return statement for function with '" + returnTypeDen->ToString() + "' return type", ast);
            }
        }
    }
    else
        Error("return statement outside function declaration", ast);

    /* Analyze return expression */
    if (ast->expr)
    {
        Visit(ast->expr);

        /* Validate expression type by just calling the getter */
        GetTypeDenoterFrom(ast->expr.get());

        /* Analyze entry point return statement (if a structure is returned from the entry point) */
        if (InsideEntryPoint())
        {
            if (auto varDecl = ast->expr->FetchVarDecl())
            {
                /* Mark variable as entry-pointer return */
                varDecl->flags << VarDecl::isEntryPointReturn;

                if (auto structSymbolRef = varDecl->GetTypeDenoter()->Get()->SymbolRef())
                {
                    if (auto structDecl = structSymbolRef->As<StructDecl>())
                    {
                        /* Add variable as parameter-structure to entry point */
                        if (program_->entryPointRef)
                            program_->entryPointRef->paramStructs.push_back({ varDecl, structDecl });
                        
                        /* Mark variable as local variable of the entry-point */
                        varDecl->flags << VarDecl::isEntryPointLocal;
                    }
                }
            }
        }
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Visit(ast->expr);

    if (IsLValueOp(ast->op))
        AnalyzeLValueExpr(ast->expr.get(), ast);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);

    if (IsLValueOp(ast->op))
        AnalyzeLValueExpr(ast->expr.get(), ast);
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
        ValidateTypeCastFrom(ast->assignExpr.get(), ast->varIdent.get(), "variable assignment");
        AnalyzeLValueVarIdent(ast->varIdent.get(), ast);
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

        /* Also connect function declaration with the identifier of the function call */
        ast->varIdent->symbolRef = ast->funcDeclRef;

        if (auto funcDecl = ast->funcDeclRef)
        {
            /* Fetch argument expressions of all remaining parmeters */
            for (std::size_t i = ast->arguments.size(), n = funcDecl->parameters.size(); i < n; ++i)
            {
                auto param = funcDecl->parameters[i].get();
                if (!param->varDecls.empty())
                {
                    auto paramVar = param->varDecls.front().get();
                    if (auto initExpr = paramVar->initializer.get())
                        ast->defaultArgumentRefs.push_back(initExpr);
                    else
                        Error("missing initializer expression for default parameter '" + paramVar->ident + "'", paramVar);
                }
            }
        }
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

bool HLSLAnalyzer::AnalyzeMemberIntrinsic(const Intrinsic intrinsic, const FunctionCall* ast)
{
    if (auto symbolRef = ast->varIdent->symbolRef)
    {
        if (auto varDecl = symbolRef->As<VarDecl>())
        {
            /* Analyze member intrinsic for buffer type */
            auto typeDen = varDecl->GetTypeDenoter()->Get();
            if (auto bufferTypeDen = typeDen->As<BufferTypeDenoter>())
            {
                if (AnalyzeMemberIntrinsicBuffer(intrinsic, bufferTypeDen->bufferType, ast->varIdent->next->ident, ast))
                    return true;
            }
        }
        else if (auto bufferDecl = symbolRef->As<BufferDecl>())
        {
            /* Analyze member intrinsic for buffer type */
            if (AnalyzeMemberIntrinsicBuffer(intrinsic, bufferDecl->GetBufferType(), ast->varIdent->next->ident, ast))
                return true;
        }
    }

    /* Intrinsic not found in an object class */
    Error("intrinsic '" + ast->varIdent->next->ident + "' not declared in object '" + ast->varIdent->ident + "'", ast);
    return false;
}

bool HLSLAnalyzer::AnalyzeMemberIntrinsicBuffer(const Intrinsic intrinsic, const BufferType bufferType, const std::string& ident, const AST* ast)
{
    if (IsTextureBufferType(bufferType))
    {
        if (!IsTextureIntrinsic(intrinsic))
            Error("invalid intrinsic '" + ident + "' for texture object", ast);
        else
            return true;
    }
    else if (IsStorageBufferType(bufferType))
    {
        //TODO
        /*if (!IsStorageBufferIntrinsic(intrinsic))
            Error("invalid intrinsic '" + ident + "' for storage-buffer object", ast);
        else
            return true;*/
    }
    else if (IsStreamBufferType(bufferType))
    {
        if (!IsStreamOutputIntrinsic(intrinsic))
            Error("invalid intrinsic '" + ident + "' for stream-output object", ast);
        else
            return true;
    }
    return false;
}

/* ----- Variable identifier ----- */

void HLSLAnalyzer::AnalyzeVarIdent(VarIdent* varIdent)
{
    /* Analyze variable identifier itself */
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

    /* Analyze array indices */
    while (varIdent)
    {
        Visit(varIdent->arrayIndices);
        varIdent = varIdent->next.get();
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

void HLSLAnalyzer::AnalyzeLValueVarIdent(VarIdent* varIdent, const AST* ast)
{
    while (varIdent)
    {
        if (auto varDecl = varIdent->FetchVarDecl())
        {
            /* Is the variable declared as constant? */
            if (varDecl->declStmntRef->IsConst())
            {
                Error(
                    "illegal assignment to l-value '" + varIdent->ident + "' that is declared as constant",
                    (ast != nullptr ? ast : varIdent), HLSLErr::ERR_LVALUE_EXPECTED
                );
            }
        }
        varIdent = varIdent->next.get();
    }
}

void HLSLAnalyzer::AnalyzeLValueExpr(Expr* expr, const AST* ast)
{
    if (auto varIdent = expr->FetchVarIdent())
        AnalyzeLValueVarIdent(varIdent, ast);
    else
        Error("illegal assignment to r-value expression", ast, HLSLErr::ERR_LVALUE_EXPECTED);
}

/* ----- Entry point ----- */

void HLSLAnalyzer::AnalyzeEntryPoint(FunctionDecl* funcDecl)
{
    /* Mark this function declaration with the entry point flag */
    if (funcDecl->flags.SetOnce(FunctionDecl::isEntryPoint))
    {
        /* Store reference to entry point in root AST node */
        program_->entryPointRef = funcDecl;

        /* Add all parameter structures to entry point */
        for (auto& param : funcDecl->parameters)
        {
            if (auto varType = param->varType->GetTypeDenoter()->Get())
            {
                if (auto structTypeDen = varType->As<StructTypeDenoter>())
                {
                    if (auto structDecl = structTypeDen->structDeclRef)
                        funcDecl->paramStructs.push_back({ param->varDecls.front().get(), structDecl });
                }
            }
        }

        /* Analyze function input/output */
        AnalyzeEntryPointInputOutput(funcDecl);

        /* Analyze entry point attributes (also possibly missing attributes such as "numthreads" for compute shaders) */
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
    if (auto returnTypeDen = funcDecl->returnType->GetTypeDenoter()->Get())
    {
        if (auto structTypeDen = returnTypeDen->As<StructTypeDenoter>())
        {
            /* Analyze entry point output structure */
            AnalyzeEntryPointParameterInOutStruct(funcDecl, structTypeDen->structDeclRef, "", false);
        }
    }

    /*
    Analyze system-value semantics.
    -> Only for the main entry point, because this function is also used for
       the optional secondary entry point, which might have another shader target.
    */
    if (funcDecl->flags(FunctionDecl::isEntryPoint))
    {
        std::vector<Semantic> inSemantics, outSemantics;

        for (const auto& param : funcDecl->inputSemantics.varDeclRefsSV)
            inSemantics.push_back(param->semantic);

        for (const auto& param : funcDecl->outputSemantics.varDeclRefsSV)
            outSemantics.push_back(param->semantic);

        if (IsSystemSemantic(funcDecl->semantic))
            outSemantics.push_back(funcDecl->semantic);

        AnalyzeEntryPointSemantics(funcDecl, inSemantics, outSemantics);
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
        /* Analyze either output or input semantic ('inout' is interpreted as output) */
        if (param->IsOutput())
            AnalyzeEntryPointParameterInOut(funcDecl, varDecl, false);
        else if (param->IsInput())
            AnalyzeEntryPointParameterInOut(funcDecl, varDecl, true);
    }
}

void HLSLAnalyzer::AnalyzeEntryPointParameterInOut(FunctionDecl* funcDecl, VarDecl* varDecl, bool input, TypeDenoterPtr varTypeDen)
{
    /* Get type denoter from variable (if not already set) */
    if (!varTypeDen)
        varTypeDen = varDecl->GetTypeDenoter()->Get();

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
    else if (auto arrayTypeDen = varTypeDen->As<ArrayTypeDenoter>())
    {
        /* Analyze base type of array type denoter */
        AnalyzeEntryPointParameterInOut(funcDecl, varDecl, input, arrayTypeDen->baseTypeDenoter);
    }
    else
    {
        /* Analyze single variable as input/output parameter */
        AnalyzeEntryPointParameterInOutVariable(funcDecl, varDecl, input);
    }

    /* Special case for geometry shader */
    if (shaderTarget_ == ShaderTarget::GeometryShader)
    {
        if (input)
        {
            /* Fetch geometry input primitive type */
            if (varDecl->declStmntRef->primitiveType != PrimitiveType::Undefined)
                program_->layoutGeometry.inputPrimitive = varDecl->declStmntRef->primitiveType;
        }
        else
        {
            /* Fetch geometry output primitive type */
            if (auto bufferTypeDen = varTypeDen->As<BufferTypeDenoter>())
            {
                if (IsStreamBufferType(bufferTypeDen->bufferType))
                    program_->layoutGeometry.outputPrimitive = bufferTypeDen->bufferType;
            }
        }
    }
}

void HLSLAnalyzer::AnalyzeEntryPointParameterInOutVariable(FunctionDecl* funcDecl, VarDecl* varDecl, bool input)
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
        if (bufferTypeDen->genericTypeDenoter)
        {
            /* Analyze generic type of buffer type denoter */
            AnalyzeEntryPointParameterInOut(funcDecl, varDecl, input, bufferTypeDen->genericTypeDenoter);
        }
        else
            Error("missing generic type denoter in stream buffer", varDecl);
    }
    else
        Error("illegal buffer type for entry point " + std::string(input ? "input" : "output"), varDecl);
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
        case ShaderTarget::GeometryShader:
            AnalyzeEntryPointAttributesGeometryShader(attribs);
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

void HLSLAnalyzer::AnalyzeEntryPointAttributesGeometryShader(const std::vector<AttributePtr>& attribs)
{
    /* Analyze optional attributes */
    for (const auto& attr : attribs)
    {
        switch (attr->attributeType)
        {
            case AttributeType::MaxVertexCount:
                AnalyzeAttributeMaxVertexCount(attr.get());
                break;
            default:
                break;
        }
    }
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

void HLSLAnalyzer::AnalyzeEntryPointSemantics(FunctionDecl* funcDecl, const std::vector<Semantic>& inSemantics, const std::vector<Semantic>& outSemantics)
{
    auto FindSemantics = [&](const std::vector<Semantic>& presentSemantics, const std::vector<Semantic>& searchSemantics, const std::string& desc)
    {
        for (auto sem : presentSemantics)
        {
            if (std::find(searchSemantics.begin(), searchSemantics.end(), sem) == searchSemantics.end())
                Error(desc + " semantic '" + SemanticToString(sem) + "' in entry point '" + funcDecl->ident + "'", funcDecl);
        }
    };

    auto ValidateInSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(inSemantics, semantics, "invalid input");
    };

    auto ValidateOutSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(outSemantics, semantics, "invalid output");
    };

    /*auto RequiredInSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(semantics, inSemantics, "missing input");
    };*/

    auto RequiredOutSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(semantics, outSemantics, "missing output");
    };

    using T = Semantic;

    #define COMMON_SEMANTICS \
        T::InstanceID, T::DepthGreaterEqual, T::DepthLessEqual, T::VertexID, T::PrimitiveID

    #define COMMON_SEMANTICS_EX \
        COMMON_SEMANTICS, T::ClipDistance, T::CullDistance

    switch (shaderTarget_)
    {
        case ShaderTarget::VertexShader:
            ValidateInSemantics({ COMMON_SEMANTICS });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition });
            break;

        case ShaderTarget::TessellationControlShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::OutputControlPointID });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::InsideTessFactor, T::TessFactor });
            break;

        case ShaderTarget::TessellationEvaluationShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::InsideTessFactor, T::TessFactor, T::DomainLocation });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition });
            break;

        case ShaderTarget::GeometryShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::GSInstanceID });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::IsFrontFace, T::ViewportArrayIndex });
            break;

        case ShaderTarget::FragmentShader:
            if (versionIn_ >= InputShaderVersion::HLSL4)
            {
                ValidateInSemantics({ COMMON_SEMANTICS_EX, T::Coverage, T::InnerCoverage, T::Depth, T::SampleIndex, T::RenderTargetArrayIndex, T::Position, T::IsFrontFace });
                ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::Coverage, T::InnerCoverage, T::Depth, T::SampleIndex, T::RenderTargetArrayIndex, T::Target, T::StencilRef });
            }
            RequiredOutSemantics({ T::Target });
            break;

        case ShaderTarget::ComputeShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::GroupID, T::GroupIndex, T::GroupThreadID, T::DispatchThreadID });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::RenderTargetArrayIndex });
            break;

        default:
            break;
    }

    #undef COMMON_SEMANTICS
    #undef COMMON_SEMANTICS_EX
}

/* ----- Inactive entry point ----- */

void HLSLAnalyzer::AnalyzeSecondaryEntryPoint(FunctionDecl* funcDecl)
{
    /* Mark this function declaration with the entry point flag */
    if (funcDecl->flags.SetOnce(FunctionDecl::isSecondaryEntryPoint))
    {
        /* Store reference to secondary entry point in root AST node */
        program_->layoutTessControl.patchConstFunctionRef = funcDecl;

        /* Analyze function input/output (use same visitor as for the main entry point here) */
        AnalyzeEntryPointInputOutput(funcDecl);

        /* Analyze secondary entry point attributes */
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

void HLSLAnalyzer::AnalyzeAttributeMaxVertexCount(Attribute* ast)
{
    if (AnalyzeNumArgsAttribute(ast, 1))
    {
        int exprValue = EvaluateConstExprInt(*ast->arguments[0]);
        if (exprValue > 0)
            program_->layoutGeometry.maxVertices = static_cast<unsigned int>(exprValue);
        else
            Error("maximal vertex count must be greater than zero", ast);
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

void HLSLAnalyzer::AnalyzeArrayDimensionList(const std::vector<ArrayDimensionPtr>& arrayDims)
{
    Visit(arrayDims);

    for (std::size_t i = 1; i < arrayDims.size(); ++i)
    {
        auto dim = arrayDims[i].get();
        if (dim->HasDynamicSize())
            Error("secondary array dimensions must be explicit", dim, HLSLErr::ERR_ARRAY_IMPLICIT_ORDER);
    }
}


} // /namespace Xsc



// ================================================================================
