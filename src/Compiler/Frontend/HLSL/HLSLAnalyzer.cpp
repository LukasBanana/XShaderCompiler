/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLAnalyzer.h"
#include "HLSLIntrinsics.h"
#include "HLSLKeywords.h"
#include "Exception.h"
#include "Helper.h"
#include "ReportIdents.h"


namespace Xsc
{


/*
 * Internal functions
 */

static ShaderVersion GetShaderModel(const InputShaderVersion v)
{
    switch (v)
    {
        case InputShaderVersion::Cg:    return { 3, 1 };
        case InputShaderVersion::HLSL3: return { 3, 1 };
        case InputShaderVersion::HLSL4: return { 4, 1 };
        case InputShaderVersion::HLSL5: return { 5, 1 };
        case InputShaderVersion::HLSL6: return { 6, 0 };
        default:                        return { 0, 0 };
    }
}


/*
 * HLSLAnalyzer class
 */

HLSLAnalyzer::HLSLAnalyzer(Log* log) :
    Analyzer{ log }
{
}

void HLSLAnalyzer::DecorateASTPrimary(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store parameters */
    entryPoint_             = inputDesc.entryPoint;
    secondaryEntryPoint_    = (inputDesc.shaderTarget == ShaderTarget::TessellationEvaluationShader ? inputDesc.secondaryEntryPoint : "");
    shaderTarget_           = inputDesc.shaderTarget;
    versionIn_              = inputDesc.shaderVersion;
    shaderModel_            = GetShaderModel(inputDesc.shaderVersion);
    preferWrappers_         = outputDesc.options.preferWrappers;

    /* Decorate program AST */
    program_ = &program;

    Visit(&program);

    /* Check if secondary entry point has been found */
    if (!secondaryEntryPoint_.empty() && !secondaryEntryPointFound_ && WarnEnabled(Warnings::UnlocatedObjects))
        Warning(R_SecondEntryPointNotFound(secondaryEntryPoint_));

    /* Analyze remaining shader model 3 semantic */
    AnalyzeSemanticSM3Remaining();
}


/*
 * ======= Private: =======
 */

void HLSLAnalyzer::ErrorIfAttributeNotFound(bool found, const std::string& attribDesc)
{
    if (!found)
        Error(R_MissingAttributeForEntryPoint(attribDesc), nullptr);
}

bool HLSLAnalyzer::IsD3D9ShaderModel() const
{
    return (versionIn_ <= InputShaderVersion::HLSL3);
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void HLSLAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Analyze context of the entire program */
    for (auto it = ast->globalStmnts.begin(); it != ast->globalStmnts.end(); ++it)
    {
        auto stmnt = it->get();
        
        /* Visit current global statement */
        Visit(stmnt);

        if (auto uniformBufferDecl = stmnt->As<UniformBufferDecl>())
        {
            /* Move all non-variable-declaration statements from buffer declaration into global scope */
            for (auto itSub = uniformBufferDecl->localStmnts.begin(); itSub != uniformBufferDecl->localStmnts.end();)
            {
                auto subStmnt = itSub->get();
                if (subStmnt->Type() != AST::Types::VarDeclStmnt)
                {
                    /* Move current sub statement from uniform buffer into global scope */
                    ++it;
                    it = ast->globalStmnts.insert(it, *itSub);
                    itSub = uniformBufferDecl->localStmnts.erase(itSub);
                }
                else
                    ++itSub;
            }
        }
    }

    /* Check if fragment shader uses a slightly different screen space (VPOS vs. SV_Position) */
    if (shaderTarget_ == ShaderTarget::FragmentShader && IsD3D9ShaderModel())
        program_->layoutFragment.pixelCenterInteger = true;
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    Visit(ast->stmnts);
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

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    AnalyzeTypeSpecifier(ast);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    AnalyzeVarDecl(ast);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    /* Register identifier for buffer */
    Register(ast->ident, ast);
    Visit(ast->arrayDims);
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    /* Register identifier for sampler */
    Register(ast->ident, ast);
    Visit(ast->arrayDims);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    /* Find base struct-decl */
    if (!ast->baseStructName.empty())
        ast->baseStructRef = FetchStructDeclFromIdent(ast->baseStructName);

    if (!GetStructDeclStack().empty())
    {
        /* Mark structure as nested structure */
        ast->flags << StructDecl::isNestedStruct;

        /* Add reference of this structure to all parent structures */
        for (auto parentStruct : GetStructDeclStack())
            parentStruct->nestedStructDeclRefs.push_back(ast);
    }

    /* Report warnings for member variables, that shadow a base member */
    if (WarnEnabled(Warnings::DeclarationShadowing) && ast->baseStructRef)
    {
        for (const auto& varDeclStmnt : ast->varMembers)
        {
            /* Remove all duplicate variables in current declaration statement */
            for (const auto& varDecl : varDeclStmnt->varDecls)
            {
                /* Does the base structure has a variable with the same identifier? */
                const StructDecl* varDeclOwner = nullptr;
                if (ast->baseStructRef->FetchVarDecl(varDecl->ident, &varDeclOwner))
                    Warning(R_DeclShadowsMemberOfBase(varDecl->ident, varDeclOwner->ToString()), varDecl.get());
            }
        }
    }

    /* Register struct identifier in symbol table */
    Register(ast->ident, ast);

    PushStructDecl(ast);
    {
        if (ast->flags(StructDecl::isNestedStruct) && !ast->IsAnonymous())
            Error(R_NestedStructsMustBeAnonymous, ast);

        OpenScope();
        {
            Visit(ast->localStmnts);
        }
        CloseScope();
    }
    PopStructDecl();
    
    /* Report warning if structure is empty */
    if (WarnEnabled(Warnings::EmptyStatementBody) && ast->NumMemberVariables() == 0)
        Warning(R_TypeHasNoMemberVariables(ast->ToString()), ast);
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
    AnalyzeSemanticFunctionReturn(ast->semantic);

    /* Visit attributes */
    Visit(ast->attribs);

    /* Visit function return type */
    Visit(ast->returnType);

    /* Analyze parameter type denoters (required before function can be registered in symbol table) */
    for (auto& param : ast->parameters)
        AnalyzeTypeDenoter(param->typeSpecifier->typeDenoter, param->typeSpecifier.get());

    /* Only use global symbol table for non-member functions */
    if (!ast->IsMemberFunction())
    {
        /* Register function declaration in symbol table (after return type and parameter types) */
        Register(ast->ident, ast);
    }

    OpenScope();
    {
        /* Analyze parameters (especially their types) */
        for (auto& param : ast->parameters)
            AnalyzeParameter(param.get());

        /* Special case for the main entry point */
        if (isEntryPoint)
            AnalyzeEntryPoint(ast);
        else if (isSecondaryEntryPoint)
            AnalyzeSecondaryEntryPoint(ast);

        /* Visit function body (without new scope) */
        PushFunctionDecl(ast);
        {
            Visit(ast->codeBlock);
        }
        PopFunctionDecl();

        /* Analyze last statement of function body ('isEndOfFunction' flag), and control paths */
        AnalyzeFunctionEndOfScopes(*ast);
        AnalyzeFunctionControlPath(*ast);
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
        Error(R_BufferCanOnlyHaveOneSlot, ast->slotRegisters[1].get());

    for (const auto& slotRegister : ast->slotRegisters)
    {
        if (slotRegister->shaderTarget != ShaderTarget::Undefined)
            Error(R_UserCBuffersCantBeTargetSpecific, slotRegister.get());
    }

    PushUniformBufferDecl(ast);
    {
        Visit(ast->localStmnts);
    }
    PopUniformBufferDecl();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    /* Global variables are implicitly constant (or rather uniform) */
    if (InsideGlobalScope() && !InsideUniformBufferDecl())
        ast->MakeImplicitConst();

    /* Analyze type specifier and variable declarations */
    Visit(ast->typeSpecifier);
    Visit(ast->varDecls);

    /* Is the 'snorm' or 'unorm' type modifier specified? */
    if (ast->HasAnyTypeModifierOf({ TypeModifier::SNorm, TypeModifier::UNorm }))
    {
        /* Is this a floating-point type? */
        auto baseTypeDen = ast->typeSpecifier->typeDenoter->As<BaseTypeDenoter>();
        if (!baseTypeDen || !IsRealType(baseTypeDen->dataType))
            Error(R_IllegalUseOfNormModifiers, ast->typeSpecifier.get());
    }

    /* Decorate structure types with their parents */
    if (auto parentStructDecl = ActiveStructDecl())
    {
        const auto& typeDen = ast->typeSpecifier->GetTypeDenoter()->GetAliased();
        if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
        {
            if (auto structDecl = structTypeDen->structDeclRef)
            {
                /*
                Add parent structure (active structure declaration)
                as reference to set of parents of the variable's structure type
                */
                structDecl->parentStructDeclRefs.insert(parentStructDecl);
            }
        }
    }
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->attribs);
    OpenScope();
    {
        Visit(ast->codeBlock);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    WarningOnNullStmnt(ast->bodyStmnt, "for loop");

    Visit(ast->attribs);

    if (WarnEnabled(Warnings::DeclarationShadowing))
    {
        /*
        Scope rules inside for-loop are different in HLSL compared to C++ or other languages!
        Variable declarations inside a for-loop header, that conflict with previously defined variables, will result in a warning.
        */
        std::map<const AST*, std::string> astIdentPairs;
        ast->initStmnt->CollectDeclIdents(astIdentPairs);

        for (const auto& it : astIdentPairs)
        {
            if (auto symbol = FetchFromCurrentScopeOrNull(it.second))
            {
                if (symbol->Type() == AST::Types::VarDecl || symbol->Type() == AST::Types::BufferDecl || symbol->Type() == AST::Types::SamplerDecl)
                {
                    /* Report warning of conflicting variable declaration */
                    Warning(R_DeclShadowsPreviousLocal(it.second, symbol->area.Pos().ToString()), it.first);
                }
            }
        }
    }

    OpenScope();
    {
        Visit(ast->initStmnt);
        AnalyzeConditionalExpression(ast->condition.get());
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
        AnalyzeConditionalExpression(ast->condition.get());
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
        AnalyzeConditionalExpression(ast->condition.get());
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    WarningOnNullStmnt(ast->bodyStmnt, "if");

    Visit(ast->attribs);

    OpenScope();
    {
        AnalyzeConditionalExpression(ast->condition.get());
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
        if (auto callExpr = ast->expr->As<CallExpr>())
            AnalyzeIntrinsicWrapperInlining(callExpr);
    }
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    /* Check if return expression matches the function return type */
    TypeDenoterPtr returnTypeDen;

    if (auto funcDecl = ActiveFunctionDecl())
    {
        if ((returnTypeDen = funcDecl->returnType->GetTypeDenoter()) != nullptr)
        {
            if (returnTypeDen->IsVoid())
            {
                if (ast->expr)
                    Error(R_IllegalExprInReturnForVoidFunc, ast->expr.get());
            }
            else
            {
                if (!ast->expr)
                    Error(R_MissingExprInReturnForFunc(returnTypeDen->ToString()), ast);
            }
        }
    }
    else
        Error(R_ReturnOutsideFuncDecl, ast);

    /* Analyze return expression */
    if (ast->expr)
    {
        Visit(ast->expr);

        /* Validate expression type by just calling the getter */
        if (auto exprTypeDen = GetTypeDenoterFrom(ast->expr.get()))
        {
            if (returnTypeDen)
                ValidateTypeCast(*exprTypeDen, *returnTypeDen, R_ReturnExpression, ast->expr.get());
        }

        /* Analyze entry point return statement (if a structure is returned from the entry point) */
        if (InsideEntryPoint())
            AnalyzeEntryPointOutput(ast->expr.get());
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    if (IsLValueOp(ast->op))
    {
        PushLValueExpr(ast);
        {
            Visit(ast->expr);
        }
        PopLValueExpr();

        AnalyzeLValueExpr(ast->expr.get(), ast);
    }
    else
        Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    if (IsLValueOp(ast->op))
    {
        PushLValueExpr(ast);
        {
            Visit(ast->expr);
        }
        PopLValueExpr();

        AnalyzeLValueExpr(ast->expr.get(), ast);
    }
    else
        Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    AnalyzeCallExpr(ast);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    PushLValueExpr(ast);
    {
        Visit(ast->lvalueExpr);
    }
    PopLValueExpr();

    Visit(ast->rvalueExpr);

    ValidateTypeCastFrom(ast->rvalueExpr.get(), ast->lvalueExpr.get(), R_VarAssignment);
    AnalyzeLValueExpr(ast->lvalueExpr.get(), ast);
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    AnalyzeObjectExpr(ast, reinterpret_cast<PrefixArgs*>(args));
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    AnalyzeArrayExpr(ast);
}

#undef IMPLEMENT_VISIT_PROC

/* ----- Declarations ----- */

void HLSLAnalyzer::AnalyzeVarDecl(VarDecl* varDecl)
{
    if (varDecl->namespaceExpr)
        AnalyzeVarDeclStaticMember(varDecl);
    else
        AnalyzeVarDeclLocal(varDecl);
}

void HLSLAnalyzer::AnalyzeVarDeclLocal(VarDecl* varDecl, bool registerVarIdent)
{
    /* Register variable identifier in symbol table (if enabled) */
    if (registerVarIdent)
        Register(varDecl->ident, varDecl);

    /* Analyze array dimensions and semantic */
    AnalyzeArrayDimensionList(varDecl->arrayDims);
    AnalyzeSemanticVarDecl(varDecl->semantic, varDecl);

    /* Store references to members with system value semantic (SV_...) in all parent structures */
    if (varDecl->semantic.IsSystemValue())
    {
        for (auto structDecl : GetStructDeclStack())
            structDecl->systemValuesRef[varDecl->ident] = varDecl;
    }

    if (varDecl->initializer)
    {
        Visit(varDecl->initializer);

        /* Compare initializer type with var-decl type */
        ValidateTypeCastFrom(varDecl->initializer.get(), varDecl, R_VarInitialization);

        if (varDecl->structDeclRef)
            Error(R_MemberVarsCantHaveDefaultValues(varDecl->ToString()), varDecl->initializer.get());
    }
}

void HLSLAnalyzer::AnalyzeVarDeclStaticMember(VarDecl* varDecl)
{
    /* Analyze static namespace prefix */
    if (InsideGlobalScope())
        Visit(varDecl->namespaceExpr);
    else
        Error(R_StaticMembersCantBeDefinedInGlob(varDecl->ToString()), varDecl->namespaceExpr.get());

    /* Analyze variable declaration, but do not register identifier */
    AnalyzeVarDeclLocal(varDecl, false);

    /* Analyze variable definition with static member declaration */
    if (auto typeDen = GetTypeDenoterFrom(varDecl->namespaceExpr.get()))
    {
        if (auto structTypeDen = typeDen->GetAliased().As<StructTypeDenoter>())
        {
            /* Fetch variable declaration from structure type */
            if (auto memberVarDecl = FetchVarDeclFromStruct(*structTypeDen, varDecl->ident, varDecl))
            {
                /* Is this member variable already defined? */
                if (auto prevVarDef = memberVarDecl->staticMemberVarRef)
                {
                    /* Report error of redefinition static member variable */
                    ReportHandler::HintForNextReport(R_PrevDefinitionAt(prevVarDef->area.Pos().ToString()));
                    Error(R_StaticMemberVarRedef(varDecl->ToString()), varDecl);
                }
                /* Is this member variable delcared as static? */
                else if (!memberVarDecl->IsStatic())
                {
                    /* Report error of illegal defnition of a non-static member variable */
                    ReportHandler::HintForNextReport(R_DeclaredAt(memberVarDecl->area.Pos().ToString()));
                    Error(R_IllegalDefOfNonStaticMemberVar(varDecl->ToString()), varDecl);
                }
                else
                {
                    /* Validate declaration type match (without array types from the VarDecl array indices) */
                    if (auto declVarType = GetTypeDenoterFrom(memberVarDecl->declStmntRef->typeSpecifier.get()))
                    {
                        if (auto defVarType = GetTypeDenoterFrom(varDecl->declStmntRef->typeSpecifier.get()))
                        {
                            if (!declVarType->Equals(*defVarType))
                            {
                                /* Report error of type mismatch */
                                ReportHandler::HintForNextReport(R_DeclaredAt(memberVarDecl->area.Pos().ToString()));
                                Error(R_DeclTypeDiffersFromDefType(declVarType->ToString(), defVarType->ToString()), varDecl);
                            }
                            else if (!memberVarDecl->arrayDims.empty())
                            {
                                /* Report error (HLSL does not allow array declaration for static member varaibles, but in there definition) */
                                Error(R_ArrayTypeCanOnlyAppearInDef(memberVarDecl->ToString()), memberVarDecl);
                            }
                            else
                            {
                                /* Decorate variable declaration with its definition */
                                memberVarDecl->staticMemberVarRef = varDecl;
                                varDecl->staticMemberVarRef = memberVarDecl;

                                /* Copy array type from definition to declaration, and reset type denoter */
                                memberVarDecl->arrayDims = varDecl->arrayDims;
                                memberVarDecl->ResetTypeDenoter();
                            }
                        }
                    }
                }
            }
        }
    }
}

/* ----- Call expressions ----- */

void HLSLAnalyzer::AnalyzeCallExpr(CallExpr* expr)
{
    try
    {
        /* Analyze prefix expression first */
        if (expr->prefixExpr)
        {
            /* Visit prefix expression first */
            Visit(expr->prefixExpr);

            /* Analyze functin call with type denoter from prefix expression */
            const auto& prefixTypeDen = expr->prefixExpr->GetTypeDenoter()->GetAliased();
            AnalyzeCallExprPrimary(expr, &prefixTypeDen);
        }
        else
        {
            /* Analyze function call */
            AnalyzeCallExprPrimary(expr);
        }
    }
    catch (const ASTRuntimeError& e)
    {
        Error(e.what(), e.GetAST());
    }
    catch (const std::exception& e)
    {
        Error(e.what(), expr);
    }
}

void HLSLAnalyzer::AnalyzeCallExprPrimary(CallExpr* callExpr, const TypeDenoter* prefixTypeDenoter)
{
    PushCallExpr(callExpr);
    {
        /* Analyze function arguments first */
        Visit(callExpr->arguments);

        /* Then analyze function name */
        if (!callExpr->ident.empty())
        {
            /* Is this an intrinsic function call? */
            auto intrIt = HLSLIntrinsicAdept::GetIntrinsicMap().find(callExpr->ident);
            if (intrIt != HLSLIntrinsicAdept::GetIntrinsicMap().end())
            {
                /* Analyze function call of intrinsic */
                AnalyzeCallExprIntrinsic(callExpr, intrIt->second, callExpr->isStatic, prefixTypeDenoter);
            }
            else
            {
                /* Analyze function call of standard function declaration */
                AnalyzeCallExprFunction(callExpr, callExpr->isStatic, callExpr->prefixExpr.get(), prefixTypeDenoter);
            }
        }

        /* Analyze all l-value arguments that are assigned to output parameters */
        callExpr->ForEachOutputArgument(
            [this](ExprPtr& argExpr)
            {
                AnalyzeLValueExpr(argExpr.get());
            }
        );
    }
    PopCallExpr();
}

void HLSLAnalyzer::AnalyzeCallExprFunction(
    CallExpr* callExpr, bool isStatic, const Expr* prefixExpr, const TypeDenoter* prefixTypeDenoter)
{
    if (prefixTypeDenoter)
    {
        /* Fetch function declaration from prefix type */
        if (auto structTypeDen = prefixTypeDenoter->As<StructTypeDenoter>())
        {
            /* Fetch function declaration from struct prefix type */
            callExpr->funcDeclRef = FetchFunctionDeclFromStruct(*structTypeDen, callExpr->ident, callExpr->arguments, callExpr);
        }
        else
        {
            /* Report error on class intrinsic for wrong type */
            Error(
                R_InvalidMemberFuncForType(callExpr->ident, prefixTypeDenoter->ToString()),
                callExpr
            );
        }
    }
    else
    {
        /* Fetch function declaration with identifier from function call */
        if (auto symbol = FetchFunctionDecl(callExpr->ident, callExpr->arguments, callExpr))
        {
            /* Decorate function call with symbol reference */
            callExpr->funcDeclRef = symbol;
        }
    }

    if (auto funcDecl = callExpr->funcDeclRef)
    {
        /* Check if static/non-static access is allowed */
        if (prefixExpr && AnalyzeStaticAccessExpr(prefixExpr, isStatic, callExpr))
        {
            /* Check if function call and function declaration are equally static/non-static */
            if (isStatic)
            {
                if (!funcDecl->IsStatic())
                    Error(R_IllegalStaticFuncCall(funcDecl->ToString()), callExpr);
            }
            else
            {
                if (funcDecl->IsStatic())
                    Error(R_IllegalNonStaticFuncCall(funcDecl->ToString()), callExpr);
            }
        }

        /* Fetch default argument expressions of all remaining parmeters */
        for (std::size_t i = callExpr->arguments.size(), n = funcDecl->parameters.size(); i < n; ++i)
        {
            auto param = funcDecl->parameters[i].get();
            if (!param->varDecls.empty())
            {
                auto paramVar = param->varDecls.front().get();
                if (auto initExpr = paramVar->initializer.get())
                    callExpr->defaultArgumentRefs.push_back(initExpr);
                else
                    Error(R_MissingInitializerForDefaultParam(paramVar->ident), paramVar);
            }
        }
    }
}

void HLSLAnalyzer::AnalyzeCallExprIntrinsic(CallExpr* callExpr, const HLSLIntrinsicEntry& intr, bool isStatic, const TypeDenoter* prefixTypeDenoter)
{
    const auto intrinsic = intr.intrinsic;

    /* Decoarte function call with intrinsic ID */
    AnalyzeCallExprIntrinsicPrimary(callExpr, intr);

    /* No intrinsics can be called static */
    if (isStatic)
        Error(R_IllegalStaticIntrinsicCall(callExpr->ident), callExpr);

    if (IsGlobalIntrinsic(intrinsic))
    {
        if (prefixTypeDenoter)
        {
            /* Report error on global intrinsic with prefix expression */
            Error(
                R_InvalidGlobalIntrinsicForType(callExpr->ident, prefixTypeDenoter->ToString()),
                callExpr
            );
        }
    }
    else
    {
        if (prefixTypeDenoter)
        {
            if (auto bufferTypeDen = prefixTypeDenoter->As<BufferTypeDenoter>())
            {
                /* Analyze member function call with buffer prefix type */
                AnalyzeCallExprIntrinsicFromBufferType(callExpr, bufferTypeDen->bufferType);

                /* Mark buffers used in compare intrinsics */
                if (IsTextureCompareIntrinsic(intrinsic))
                {
                    if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
                        bufferDecl->flags << BufferDecl::isUsedForCompare;
                }
            }
            else
            {
                /* Report error on class intrinsic for wrong type */
                Error(
                    R_InvalidClassIntrinsicForType(callExpr->ident, prefixTypeDenoter->ToString()),
                    callExpr
                );
            }
        }
        else
        {
            /* Report error on non-global intrinsic without prefix expression */
            Error(R_InvalidClassIntrinsic(callExpr->ident), callExpr);
        }
    }
}

void HLSLAnalyzer::AnalyzeCallExprIntrinsicPrimary(CallExpr* callExpr, const HLSLIntrinsicEntry& intr)
{
    /* Check shader input version */
    if (shaderModel_ < intr.minShaderModel)
    {
        Error(
            R_InvalidShaderModelForIntrinsic(
                callExpr->ident, intr.minShaderModel.ToString(), shaderModel_.ToString()
            ),
            callExpr
        );
    }

    /* Decorate AST with intrinsic ID */
    callExpr->intrinsic = intr.intrinsic;

    /* Analyze special intrinsic types */
    using T = Intrinsic;

    struct IntrinsicConversion
    {
        T           standardIntrinsic;
        std::size_t numArgs;
        T           overloadedIntrinsic;
    };

    //TODO: move this into a different file (maybe HLSLIntrinsics.cpp)
    static const std::vector<IntrinsicConversion> intrinsicConversions
    {
        { T::AsUInt_1,                      3, T::AsUInt_3                      },
        { T::Tex1D_2,                       4, T::Tex1D_4                       },
        { T::Tex2D_2,                       4, T::Tex2D_4                       },
        { T::Tex3D_2,                       4, T::Tex3D_4                       },
        { T::TexCube_2,                     4, T::TexCube_4                     },
        { T::Texture_Load_1,                2, T::Texture_Load_2                },
        { T::Texture_Load_1,                3, T::Texture_Load_3                },

        { T::Texture_Gather_2,              3, T::Texture_Gather_3              },
        { T::Texture_Gather_2,              4, T::Texture_Gather_4              },
        { T::Texture_GatherRed_2,           3, T::Texture_GatherRed_3           },
        { T::Texture_GatherRed_2,           4, T::Texture_GatherRed_4 },
        { T::Texture_GatherRed_2,           6, T::Texture_GatherRed_6           },
        { T::Texture_GatherRed_2,           7, T::Texture_GatherRed_7           },
        { T::Texture_GatherGreen_2,         3, T::Texture_GatherGreen_3         },
        { T::Texture_GatherGreen_2,         4, T::Texture_GatherGreen_4         },
        { T::Texture_GatherGreen_2,         6, T::Texture_GatherGreen_6         },
        { T::Texture_GatherGreen_2,         7, T::Texture_GatherGreen_7         },
        { T::Texture_GatherBlue_2,          3, T::Texture_GatherBlue_3          },
        { T::Texture_GatherBlue_2,          4, T::Texture_GatherBlue_4          },
        { T::Texture_GatherBlue_2,          6, T::Texture_GatherBlue_6          },
        { T::Texture_GatherBlue_2,          7, T::Texture_GatherBlue_7          },
        { T::Texture_GatherAlpha_2,         3, T::Texture_GatherAlpha_3         },
        { T::Texture_GatherAlpha_2,         4, T::Texture_GatherAlpha_4         },
        { T::Texture_GatherAlpha_2,         6, T::Texture_GatherAlpha_6         },
        { T::Texture_GatherAlpha_2,         7, T::Texture_GatherAlpha_7         },

        { T::Texture_GatherCmp_3,           4, T::Texture_GatherCmp_4           },
        { T::Texture_GatherCmp_3,           5, T::Texture_GatherCmp_5           },
        { T::Texture_GatherCmpRed_3,        4, T::Texture_GatherCmpRed_4        },
        { T::Texture_GatherCmpRed_3,        5, T::Texture_GatherCmpRed_5        },
        { T::Texture_GatherCmpRed_3,        7, T::Texture_GatherCmpRed_7        },
        { T::Texture_GatherCmpRed_3,        8, T::Texture_GatherCmpRed_8        },
        { T::Texture_GatherCmpGreen_3,      4, T::Texture_GatherCmpGreen_4      },
        { T::Texture_GatherCmpGreen_3,      5, T::Texture_GatherCmpGreen_5      },
        { T::Texture_GatherCmpGreen_3,      7, T::Texture_GatherCmpGreen_7      },
        { T::Texture_GatherCmpGreen_3,      8, T::Texture_GatherCmpGreen_8      },
        { T::Texture_GatherCmpBlue_3,       4, T::Texture_GatherCmpBlue_4       },
        { T::Texture_GatherCmpBlue_3,       5, T::Texture_GatherCmpBlue_5       },
        { T::Texture_GatherCmpBlue_3,       7, T::Texture_GatherCmpBlue_7       },
        { T::Texture_GatherCmpBlue_3,       8, T::Texture_GatherCmpBlue_8       },
        { T::Texture_GatherCmpAlpha_3,      4, T::Texture_GatherCmpAlpha_4      },
        { T::Texture_GatherCmpAlpha_3,      5, T::Texture_GatherCmpAlpha_5      },
        { T::Texture_GatherCmpAlpha_3,      7, T::Texture_GatherCmpAlpha_7      },
        { T::Texture_GatherCmpAlpha_3,      8, T::Texture_GatherCmpAlpha_8      },

        { T::Texture_Sample_2,              3, T::Texture_Sample_3              },
        { T::Texture_Sample_2,              4, T::Texture_Sample_4              },
        { T::Texture_Sample_2,              5, T::Texture_Sample_5              },
        { T::Texture_SampleBias_3,          4, T::Texture_SampleBias_4          },
        { T::Texture_SampleBias_3,          5, T::Texture_SampleBias_5          },
        { T::Texture_SampleBias_3,          6, T::Texture_SampleBias_6          },
        { T::Texture_SampleCmp_3,           4, T::Texture_SampleCmp_4           },
        { T::Texture_SampleCmp_3,           5, T::Texture_SampleCmp_5           },
        { T::Texture_SampleCmp_3,           6, T::Texture_SampleCmp_6           },
        { T::Texture_SampleGrad_4,          5, T::Texture_SampleGrad_5          },
        { T::Texture_SampleGrad_4,          6, T::Texture_SampleGrad_6          },
        { T::Texture_SampleGrad_4,          7, T::Texture_SampleGrad_7          },
        { T::Texture_SampleLevel_3,         4, T::Texture_SampleLevel_4         },
        { T::Texture_SampleLevel_3,         5, T::Texture_SampleLevel_5         },
    };

    for (const auto& conversion : intrinsicConversions)
    {
        /* Is another overloaded version of the intrinsic used? */
        if (callExpr->intrinsic == conversion.standardIntrinsic && callExpr->arguments.size() == conversion.numArgs)
        {
            /* Convert intrinsic type */
            callExpr->intrinsic = conversion.overloadedIntrinsic;
            break;
        }
    }
}

void HLSLAnalyzer::AnalyzeCallExprIntrinsicFromBufferType(const CallExpr* callExpr, const BufferType bufferType)
{
    const auto intrinsic = callExpr->intrinsic;
    const auto& ident = callExpr->ident;

    if (IsTextureBufferType(bufferType))
    {
        /* Check if texture intrinsic is used to texture buffer type */
        if (!IsTextureIntrinsic(intrinsic))
            Error(R_InvalidIntrinsicForTexture(ident), callExpr);
    }
    else if (IsRWTextureBufferType(bufferType))
    {
        /* Check if image load/store intrinsic is used to RW-texture buffer type */
        if (!IsImageIntrinsic(intrinsic))
            Error(R_InvalidIntrinsicForRWTexture(ident), callExpr);
    }
    else if (IsStreamBufferType(bufferType))
    {
        /* Check if stream-output intrinsic is used to stream-output buffer type */
        if (IsStreamOutputIntrinsic(intrinsic))
        {
            /* Check for entry point output parameters with "StreamOutput::Append" intrinsic */
            if (InsideEntryPoint() && intrinsic == Intrinsic::StreamOutput_Append)
            {
                for (const auto& arg : callExpr->arguments)
                    AnalyzeEntryPointOutput(arg.get());
            }
        }
        else
            Error(R_InvalidIntrinsicForStreamOutput(ident), callExpr);
    }

    /* Ensure gather intrinsics are used only on supported types */
    if(IsGatherIntrisic(intrinsic))
    {
        switch(bufferType)
        {
        case BufferType::Texture2D:
        case BufferType::Texture2DArray:
            break;
        case BufferType::TextureCube:
        case BufferType::TextureCubeArray:
            /* Only overloads with no offset supported */
            if(GetGatherIntrinsicOffsetParamCount(intrinsic) != 0)
                Error(R_InvalidClassIntrinsicForType(ident, BufferTypeToString(bufferType)), callExpr);
            break;
        default:
            Error(R_InvalidClassIntrinsicForType(ident, BufferTypeToString(bufferType)), callExpr);
            break;
        }
    }

    /* Ensure load intrinsics are only used on supported types */
    if(intrinsic == Intrinsic::Texture_Load_1)
    {
        /* Sample index is required for MS textures */
        if (bufferType == BufferType::Texture2DMS || bufferType == BufferType::Texture2DMSArray)
            Error(R_InvalidClassIntrinsicForType(ident, BufferTypeToString(bufferType)), callExpr);
    }
    else if(intrinsic == Intrinsic::Texture_Load_2)
    {
        /* Buffer loads only support the one parameter version */
        if (bufferType == BufferType::Buffer)
            Error(R_InvalidClassIntrinsicForType(ident, BufferTypeToString(bufferType)), callExpr);
    }
    else if(intrinsic == Intrinsic::Texture_Load_3)
    {
        /* Sample index + offset overload is only supported for MS textures */
        if(bufferType != BufferType::Texture2DMS && bufferType != BufferType::Texture2DMSArray)
            Error(R_InvalidClassIntrinsicForType(ident, BufferTypeToString(bufferType)), callExpr);
    }
}

void HLSLAnalyzer::AnalyzeIntrinsicWrapperInlining(CallExpr* callExpr)
{
    /* Is this a 'clip'-intrinsic call? */
    if (callExpr->intrinsic == Intrinsic::Clip)
    {
        /* The wrapper function for this intrinsic can be inlined */
        callExpr->flags << CallExpr::canInlineIntrinsicWrapper;
    }
}

/* ----- Object expressions ----- */

void HLSLAnalyzer::AnalyzeObjectExpr(ObjectExpr* expr, PrefixArgs* args)
{
    /* Analyze prefix expression first */
    if (expr->prefixExpr)
    {
        /* Visit prefix expression first (and pass static state as input argument) */
        PrefixArgs prefixArgs;
        {
            prefixArgs.inIsPostfixStatic    = expr->isStatic;
            prefixArgs.outPrefixBaseStruct  = nullptr;
        }
        Visit(expr->prefixExpr, &prefixArgs);

        /* Get type denoter from prefix expression */
        const auto& prefixTypeDen = expr->prefixExpr->GetTypeDenoter()->GetAliased();

        if (auto structTypeDen = prefixTypeDen.As<StructTypeDenoter>())
        {
            /* Analyze object expression with struct prefix type */
            if (args != nullptr && args->inIsPostfixStatic && !expr->isStatic)
            {
                /* Analyze object expression as base structure namespace */
                AnalyzeObjectExprBaseStructDeclFromStruct(expr, *args, *structTypeDen);
            }
            else
            {
                /* Analyze object expression as structure member */
                AnalyzeObjectExprVarDeclFromStruct(expr, prefixArgs.outPrefixBaseStruct, *structTypeDen);
            }
        }
        else if (prefixTypeDen.IsBase())
        {
            /* Just query the type denoter for the object expression */
            GetTypeDenoterFrom(expr);
        }
    }
    else
    {
        if (auto symbol = FetchDecl(expr->ident, expr))
        {
            /* Decorate object expression with symbol reference */
            expr->symbolRef = symbol;

            /* Pass structure to output argument */
            if (args)
            {
                if (auto structDecl = symbol->As<StructDecl>())
                    args->outPrefixBaseStruct = structDecl;
            }

            /* Mark is 'read from' if this object expression is not part of an l-value expression */
            if (ActiveLValueExpr() == nullptr)
                symbol->flags << Decl::isReadFrom;
        }
    }
}

void HLSLAnalyzer::AnalyzeObjectExprVarDeclFromStruct(ObjectExpr* expr, StructDecl* baseStructDecl, const StructTypeDenoter& structTypeDen)
{
    if (baseStructDecl)
    {
        /* Fetch struct member variable declaration from next identifier */
        expr->symbolRef = FetchVarDeclFromStruct(baseStructDecl, expr->ident, expr);

        /* Now check, if the referenced symbol can be accessed from the current context */
        if (expr->symbolRef)
        {
            if (auto varDecl = expr->symbolRef->As<VarDecl>())
            {
                if (!varDecl->IsStatic())
                {
                    /* Check if the referenced symbol is a non-static member variable of a structure, that is a base to the active structure */
                    if (auto activeStructDecl = ActiveStructDecl())
                    {
                        if (baseStructDecl->IsBaseOf(activeStructDecl, true))
                            return;
                    }

                    /* Check if the prefix is a base structure namespace expression */
                    if (expr->prefixExpr && expr->prefixExpr->flags(ObjectExpr::isBaseStructNamespace))
                        return;
                }
            }
        }
    }
    else
    {
        /* Fetch struct member variable declaration from next identifier */
        expr->symbolRef = FetchVarDeclFromStruct(structTypeDen, expr->ident, expr);
    }

    /* Check if struct member and identifier are both static or non-static */
    if (expr->symbolRef && expr->prefixExpr)
    {
        /* Check if static/non-static access is allowed */
        if (AnalyzeStaticAccessExpr(expr->prefixExpr.get(), expr->isStatic, expr))
        {
            /* Check if member and declaration object are equally static/non-static */
            AnalyzeStaticTypeSpecifier(expr->symbolRef->FetchTypeSpecifier(), expr->ident, expr, expr->isStatic);
        }
    }
}

void HLSLAnalyzer::AnalyzeObjectExprBaseStructDeclFromStruct(ObjectExpr* expr, PrefixArgs& outputArgs, const StructTypeDenoter& structTypeDen)
{
    if (auto structDecl = structTypeDen.structDeclRef)
    {
        /* Fetch base structure from next identifier */
        if (auto symbol = structDecl->FetchBaseStructDecl(expr->ident))
        {
            /* Store symbol reference in object expression and pass it as output argument */
            expr->symbolRef = symbol;
            expr->flags << ObjectExpr::isBaseStructNamespace;
            outputArgs.outPrefixBaseStruct = symbol;
        }
        else
            Error(R_IdentIsNotBaseOf(expr->ident, structDecl->ToString()), expr);
    }
}

bool HLSLAnalyzer::AnalyzeStaticAccessExpr(const Expr* prefixExpr, bool isStatic, const AST* ast)
{
    if (prefixExpr)
    {
        /* This function returns true, if the specified expression is an object expression with a typename (i.e. structure or alias name) */
        auto IsObjectExprWithTypename = [](const Expr& expr) -> bool
        {
            if (auto objectExpr = expr.As<ObjectExpr>())
            {
                if (auto symbol = objectExpr->symbolRef)
                {
                    /* Fetch type declaration from symbol reference */
                    return (symbol->Type() == AST::Types::StructDecl || symbol->Type() == AST::Types::AliasDecl);
                }
            }
            return false;
        };

        /* Fetch static type expression from prefix expression */
        if (auto staticTypeExpr = AST::GetAs<ObjectExpr>(prefixExpr->Find(IsObjectExprWithTypename, SearchLValue)))
        {
            if (!isStatic)
            {
                Error(R_IllegalNonStaticAccessToType(staticTypeExpr->ident), ast);
                return false;
            }
        }
        else
        {
            if (isStatic)
            {
                Error(R_IllegalStaticAccessToNonType, ast);
                return false;
            }
        }
    }
    return true;
}

bool HLSLAnalyzer::AnalyzeStaticTypeSpecifier(const TypeSpecifier* typeSpecifier, const std::string& ident, const Expr* expr, bool isStatic)
{
    if (typeSpecifier)
    {
        if (typeSpecifier->HasAnyStorageClassesOf({ StorageClass::Static }))
        {
            if (!isStatic)
            {
                Error(R_IllegalNonStaticAccessToMember(ident), expr);
                return false;
            }
        }
        else
        {
            if (isStatic)
            {
                Error(R_IllegalStaticAccessToMember(ident), expr);
                return false;
            }
        }
    }
    return true;
}

void HLSLAnalyzer::AnalyzeLValueExpr(const Expr* expr, const AST* ast)
{
    if (expr)
    {
        /* Fetch l-value from expression */
        if (auto lvalueExpr = expr->FetchLValueExpr())
            AnalyzeLValueExprObject(lvalueExpr, ast);
        else
            Error(R_IllegalRValueAssignment, (ast != nullptr ? ast : expr));
    }
}

void HLSLAnalyzer::AnalyzeLValueExprObject(const ObjectExpr* objectExpr, const AST* ast)
{
    /* Analyze prefix expression as l-value */
    AnalyzeLValueExpr(objectExpr->prefixExpr.get(), ast);

    if (auto symbol = objectExpr->symbolRef)
    {
        if (auto varDecl = symbol->As<VarDecl>())
        {
            if (varDecl->declStmntRef->IsConstOrUniform())
            {
                /* Construct error message depending if the variable is implicitly or explicitly declared as constant */
                Error(
                    R_IllegalLValueAssignmentToConst(
                        objectExpr->ident,
                        (varDecl->declStmntRef->flags(VarDeclStmnt::isImplicitConst) ? R_Implicitly : "")
                    ),
                    (ast != nullptr ? ast : objectExpr)
                );
            }
        }
    }
    else
    {
        Error(
            R_MissingObjectExprSymbolRef(objectExpr->ident),
            (ast != nullptr ? ast : objectExpr)
        );
    }
}

/* ----- Array expressions ----- */

void HLSLAnalyzer::AnalyzeArrayExpr(ArrayExpr* expr)
{
    /* Visit prefix and array index expressions */
    Visit(expr->prefixExpr.get());
    Visit(expr->arrayIndices);

    /* Just query the type denoter for the array access expression */
    GetTypeDenoterFrom(expr);

    /* Validate types of array indices */
    for (auto& arrayIndex : expr->arrayIndices)
    {
        if (auto typeDen = GetTypeDenoterFrom(arrayIndex.get()))
        {
            /* Get vector dimension from type denoter */
            const auto& typeDenAliased = typeDen->GetAliased();
            if (auto baseTypeDen = typeDenAliased.As<BaseTypeDenoter>())
            {
                /* Validate type cast from array index to integral vector */
                const auto vectorDim = VectorTypeDim(baseTypeDen->dataType);
                const auto intVecType = VectorDataType(DataType::Int, vectorDim);
                ValidateTypeCast(*typeDen, BaseTypeDenoter(intVecType), R_ArrayIndex, arrayIndex.get());
            }
            else
                Error(R_ArrayIndexMustHaveBaseType(typeDenAliased.ToString()), arrayIndex.get());
        }
    }
}

/* ----- Entry point ----- */

void HLSLAnalyzer::AnalyzeEntryPoint(FunctionDecl* funcDecl)
{
    /* Mark this function declaration with the entry point flag */
    if (funcDecl->flags.SetOnce(FunctionDecl::isEntryPoint))
    {
        /* Decorate AST root node with reference to entry point */
        program_->entryPointRef = funcDecl;

        /* Add all parameter structures to entry point */
        for (auto& param : funcDecl->parameters)
        {
            const auto& typeDenoter = param->typeSpecifier->GetTypeDenoter()->GetAliased();
            if (auto structTypeDen = typeDenoter.As<StructTypeDenoter>())
            {
                if (auto structDecl = structTypeDen->structDeclRef)
                    funcDecl->paramStructs.push_back({ nullptr, param->varDecls.front().get(), structDecl });
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
            Error(R_InvalidVarDeclCountInParam, param.get());
    }

    /* Analyze function return type */
    const auto& returnTypeDen = funcDecl->returnType->GetTypeDenoter()->GetAliased();
    if (auto structTypeDen = returnTypeDen.As<StructTypeDenoter>())
    {
        /* Analyze entry point output structure */
        AnalyzeEntryPointParameterInOutStruct(funcDecl, structTypeDen->structDeclRef, false);
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

    /* Override all output semantics if the function has a return type semantic */
    if (funcDecl->semantic.IsValid() && !funcDecl->outputSemantics.Empty())
    {
        int semanticIndex = 0;

        funcDecl->outputSemantics.ForEach(
            [&](VarDecl* varDecl)
            {
                varDecl->semantic = IndexedSemantic(funcDecl->semantic, semanticIndex);
                ++semanticIndex;
            }
        );

        funcDecl->outputSemantics.UpdateDistribution();

        funcDecl->semantic.Reset();
    }

    /* Check if there are duplicate output semantics */
    std::map<IndexedSemantic, int> outputSemanticCounter;

    funcDecl->outputSemantics.ForEach(
        [&outputSemanticCounter](VarDecl* varDecl)
        {
            auto it = outputSemanticCounter.find(varDecl->semantic);
            if (it != outputSemanticCounter.end())
                ++it->second;
            else
                outputSemanticCounter.insert({ varDecl->semantic, 1 });
        }
    );

    for (const auto it : outputSemanticCounter)
    {
        if (it.second > 1)
            Error(R_DuplicateUseOfOutputSemantic(it.first.ToString()));
    }
}

void HLSLAnalyzer::AnalyzeEntryPointParameter(FunctionDecl* funcDecl, VarDeclStmnt* param)
{
    auto varDecl = param->varDecls.front().get();

    if (param->IsUniform())
    {
        /* Verify input only semantic */
        if (param->IsOutput())
            Error(R_UniformCantBeOutput, varDecl);
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
        varTypeDen = varDecl->GetTypeDenoter()->GetSub();

    if (auto structTypeDen = varTypeDen->As<StructTypeDenoter>())
    {
        /* Analyze entry point structure */
        AnalyzeEntryPointParameterInOutStruct(funcDecl, structTypeDen->structDeclRef, input);
    }
    else if (auto bufferTypeDen = varTypeDen->As<BufferTypeDenoter>())
    {
        /* Analyze entry point buffer */
        AnalyzeEntryPointParameterInOutBuffer(funcDecl, varDecl, bufferTypeDen, input);
    }
    else if (auto arrayTypeDen = varTypeDen->As<ArrayTypeDenoter>())
    {
        /* Analyze sub type of array type denoter */
        AnalyzeEntryPointParameterInOut(funcDecl, varDecl, input, arrayTypeDen->subTypeDenoter);
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
            const auto primitiveType = varDecl->declStmntRef->typeSpecifier->primitiveType;
            if (primitiveType != PrimitiveType::Undefined)
                program_->layoutGeometry.inputPrimitive = primitiveType;
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
        Error(R_MissingSemanticInEntryPointParam(varDecl->ident), varDecl);

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

void HLSLAnalyzer::AnalyzeEntryPointParameterInOutStruct(FunctionDecl* funcDecl, StructDecl* structDecl, bool input)
{
    if (structDecl)
    {
        /* Analyze all structure members */
        //TODO: refactor this!
        #if 1
        for (auto& member : structDecl->varMembers)
        {
            for (auto& memberVar : member->varDecls)
                AnalyzeEntryPointParameterInOut(funcDecl, memberVar.get(), input);
        }
        #else
        //TODO: this may produce duplicate input/output variables!
        structDecl->ForEachVarDecl(
            [&](VarDeclPtr& varDecl)
            {
                AnalyzeEntryPointParameterInOut(funcDecl, varDecl.get(), input);
            }
        );
        #endif

        /* Mark structure as shader input/output */
        if (input)
            structDecl->AddFlagsRecursive(StructDecl::isShaderInput);
        else
            structDecl->AddFlagsRecursive(StructDecl::isShaderOutput);
    }
}

void HLSLAnalyzer::AnalyzeEntryPointParameterInOutBuffer(FunctionDecl* funcDecl, VarDecl* varDecl, BufferTypeDenoter* bufferTypeDen, bool input)
{
    if (IsStreamBufferType(bufferTypeDen->bufferType) || IsPatchBufferType(bufferTypeDen->bufferType))
    {
        /* Analyze generic type of buffer type denoter */
        if (bufferTypeDen->genericTypeDenoter)
            AnalyzeEntryPointParameterInOut(funcDecl, varDecl, input, bufferTypeDen->genericTypeDenoter);
        else
            Error(R_MissingGenericTypeDen(BufferTypeToString(bufferTypeDen->bufferType)), varDecl);
    }
    else
        Error(R_IllegalBufferTypeForEntryPoint(input ? R_Input : R_Output), varDecl);
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
    auto FindSemantics = [&](const std::vector<Semantic>& presentSemantics, const std::vector<Semantic>& searchSemantics, const JoinableString& reportIdent)
    {
        for (auto sem : presentSemantics)
        {
            if (IsSystemSemantic(sem) && std::find(searchSemantics.begin(), searchSemantics.end(), sem) == searchSemantics.end())
                Error(reportIdent(SemanticToString(sem), funcDecl->ident), funcDecl);
        }
    };

    auto ValidateInSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(inSemantics, semantics, R_InvalidInputSemanticInEntryPoint);
    };

    auto ValidateOutSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(outSemantics, semantics, R_InvalidOutputSemanticInEntryPoint);
    };
    
    /*auto RequiredInSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(semantics, inSemantics, R_MissingInputSemanticInEntryPoint);
    };*/

    auto RequiredOutSemantics = [&](const std::vector<Semantic>& semantics)
    {
        FindSemantics(semantics, outSemantics, R_MissingOutputSemanticInEntryPoint);
    };

    using T = Semantic;

    #define COMMON_SEMANTICS \
        T::InstanceID, T::DepthGreaterEqual, T::DepthLessEqual, T::VertexID, T::PrimitiveID

    #define COMMON_SEMANTICS_EX \
        COMMON_SEMANTICS, T::ClipDistance, T::CullDistance

    switch (shaderTarget_)
    {
        case ShaderTarget::VertexShader:
            if (IsD3D9ShaderModel())
            {
                ValidateInSemantics({ COMMON_SEMANTICS, T::VertexPosition, T::PointSize, T::Target });
                ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize, T::Target });
            }
            else
            {
                ValidateInSemantics({ COMMON_SEMANTICS });
                ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize });
            }
            break;

        case ShaderTarget::TessellationControlShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize, T::OutputControlPointID });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize, T::InsideTessFactor, T::TessFactor });
            break;

        case ShaderTarget::TessellationEvaluationShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize, T::InsideTessFactor, T::TessFactor, T::DomainLocation });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize });
            break;

        case ShaderTarget::GeometryShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize, T::GSInstanceID });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::VertexPosition, T::PointSize, T::IsFrontFace, T::ViewportArrayIndex, T::RenderTargetArrayIndex });
            break;

        case ShaderTarget::FragmentShader:
            if (!IsD3D9ShaderModel())
            {
                ValidateInSemantics({ COMMON_SEMANTICS_EX, T::Coverage, T::InnerCoverage, T::Depth, T::SampleIndex, T::RenderTargetArrayIndex, T::FragCoord, T::IsFrontFace });
                ValidateOutSemantics({ COMMON_SEMANTICS_EX, T::Coverage, T::InnerCoverage, T::Depth, T::SampleIndex, T::RenderTargetArrayIndex, T::Target, T::StencilRef });
            }
            RequiredOutSemantics({ T::Target });
            break;

        case ShaderTarget::ComputeShader:
            ValidateInSemantics({ COMMON_SEMANTICS_EX, T::GroupID, T::GroupIndex, T::GroupThreadID, T::DispatchThreadID });
            ValidateOutSemantics({ COMMON_SEMANTICS_EX });
            break;

        default:
            break;
    }

    #undef COMMON_SEMANTICS
    #undef COMMON_SEMANTICS_EX
}

/*
~~~~~~~~~ TODO: refactore 'program_->entryPointRef->paramStructs' !!! ~~~~~~~~~~
*/

void HLSLAnalyzer::AnalyzeEntryPointOutput(Expr* expr)
{
    if (auto objectExpr = expr->FetchLValueExpr())
    {
        if (auto varDecl = objectExpr->FetchVarDecl())
        {
            //TODO: replace by "AddFlagsRecursive" if it's sure, that it can safely be used!
            #if 0
            varDecl->flags << VarDecl::isEntryPointOutput;
            #else
            /* Mark variable (and all members of its struct type, if it has one) as entry-point output */
            varDecl->AddFlagsRecursive(VarDecl::isEntryPointOutput);
            #endif

            if (auto structSymbolRef = varDecl->GetTypeDenoter()->GetAliased().SymbolRef())
            {
                if (auto structDecl = structSymbolRef->As<StructDecl>())
                {
                    /* Add variable as instance to this structure as entry point output */
                    structDecl->AddShaderOutputInstance(varDecl);

                    /* Add variable as parameter-structure to entry point */
                    if (program_->entryPointRef)
                        program_->entryPointRef->paramStructs.push_back({ expr, varDecl, structDecl });
                        
                    /* Mark variable as local variable of the entry-point */
                    varDecl->flags << VarDecl::isEntryPointLocal;
                }
            }
        }
    }
}

/* ----- Inactive entry point ----- */

void HLSLAnalyzer::AnalyzeSecondaryEntryPoint(FunctionDecl* funcDecl, bool isPatchConstantFunc)
{
    /* Mark this function declaration with the entry point flag */
    if (funcDecl->flags.SetOnce(FunctionDecl::isSecondaryEntryPoint))
    {
        /* Store reference to patch constant function in root AST node */
        if (isPatchConstantFunc)
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

    //TODO: add "AttributeTypeToString" function

    if (numArgs < expectedNumArgs)
    {
        if (required)
        {
            Error(
                R_TooFewArgsForAttribute(""/*AttributeTypeToString(ast->attributeType)*/, expectedNumArgs, numArgs),
                ast
            );
        }
    }
    else if (numArgs > expectedNumArgs)
    {
        if (required)
        {
            Error(
                R_TooManyArgsForAttribute(""/*AttributeTypeToString(ast->attributeType)*/, expectedNumArgs, numArgs),
                ast
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
            R_ExpectedDomainTypeParamToBe,
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
            R_ExpectedOutputTopologyParamToBe,
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
            R_ExpectedPartitioningModeParamToBe,
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
            Error(R_ExpectedOutputCtrlPointParamToBe, ast->arguments[0].get());
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
                AnalyzeSecondaryEntryPoint(patchConstFunc, true);
            }
            else
                Error(R_EntryPointForPatchFuncNotFound(literalValue), ast->arguments[0].get());
        }
        else
            Error(R_ExpectedPatchFuncParamToBe, ast->arguments[0].get());
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
            Error(R_MaxVertexCountMustBeGreaterZero, ast);
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
        Error(R_NumThreadsMustBeGreaterZero, ast);
}

void HLSLAnalyzer::AnalyzeAttributeValue(
    Expr* argExpr, AttributeValue& value, const OnValidAttributeValueProc& expectedValueFunc,
    const std::string& expectationDesc, bool required)
{
    std::string literalValue;
    if (!AnalyzeAttributeValuePrimary(argExpr, value, expectedValueFunc, literalValue) && required)
        Error(expectationDesc + R_ButGot(literalValue), argExpr);
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

/* ----- Semantic ----- */

/*
~~~~~~~~~~ TODO: ~~~~~~~~~~
if this semantic is used as input or output can not be determined,
if the variable is inside a structure!
*/
void HLSLAnalyzer::AnalyzeSemantic(IndexedSemantic& semantic)
{
    if (semantic == Semantic::FragCoord && shaderTarget_ != ShaderTarget::FragmentShader)
    {
        /* Convert shader semantic to VertexPosition */
        semantic = IndexedSemantic(Semantic::VertexPosition, semantic.Index());
    }
}

void HLSLAnalyzer::AnalyzeSemanticSM3(IndexedSemantic& semantic, bool input)
{
    /* Convert some system value semantics to a user defined semantic (e.g. vertex input POSITION[n]) */
    if ( ( shaderTarget_ == ShaderTarget::VertexShader   && semantic == Semantic::VertexPosition && input ) ||
         ( shaderTarget_ == ShaderTarget::VertexShader   && semantic == Semantic::Target                  ) ||
         ( shaderTarget_ == ShaderTarget::FragmentShader && semantic == Semantic::Target         && input ) ||
         (                                                  semantic == Semantic::PointSize      && input ) )
    {
        /* Make this a user defined semantic */
        switch (semantic)
        {
            case Semantic::PointSize:
                semantic.MakeUserDefined("PSIZE");
                break;
            case Semantic::VertexPosition:
                semantic.MakeUserDefined("POSITION");
                break;
            case Semantic::Target:
                semantic.MakeUserDefined("COLOR");
                break;
            default:
                break;
        }
    }
}

void HLSLAnalyzer::AnalyzeSemanticSM3Remaining()
{
    if (!varDeclSM3Semantics_.empty())
    {
        /* Analyze remaining shader model 3 semantics */
        for (auto varDecl : varDeclSM3Semantics_)
            AnalyzeSemanticSM3(varDecl->semantic, varDecl->flags(VarDecl::isShaderInput));

        /* Update distribution of semantics with and without system values */
        if (auto entryPoint = program_->entryPointRef)
        {
            entryPoint->inputSemantics.UpdateDistribution();
            entryPoint->outputSemantics.UpdateDistribution();
        }
    }
}

void HLSLAnalyzer::AnalyzeSemanticVarDecl(IndexedSemantic& semantic, VarDecl* varDecl)
{
    AnalyzeSemantic(semantic);

    if (IsD3D9ShaderModel())
    {
        if ( semantic == Semantic::VertexPosition ||
             semantic == Semantic::Target         ||
             semantic == Semantic::PointSize )
        {
            /* Add variable to shader model 3 semantics (will be analyzed after main analysis) */
            varDeclSM3Semantics_.insert(varDecl);
        }
    }
}

void HLSLAnalyzer::AnalyzeSemanticFunctionReturn(IndexedSemantic& semantic)
{
    AnalyzeSemantic(semantic);
    if (IsD3D9ShaderModel())
        AnalyzeSemanticSM3(semantic, false);
}

/* ----- Misc ----- */

void HLSLAnalyzer::AnalyzeArrayDimensionList(const std::vector<ArrayDimensionPtr>& arrayDims)
{
    Visit(arrayDims);

    for (std::size_t i = 1; i < arrayDims.size(); ++i)
    {
        auto dim = arrayDims[i].get();
        if (dim->HasDynamicSize())
            Error(R_SecondaryArrayDimMustBeExplicit, dim);
    }
}

void HLSLAnalyzer::AnalyzeParameter(VarDeclStmnt* param)
{
    /* Default visitor for parameter */
    Visit(param);

    /* Analyze parameter type specifier */
    AnalyzeTypeSpecifierForParameter(param->typeSpecifier.get());

    /* Check for structure definition */
    if (auto structDecl = param->typeSpecifier->structDecl.get())
        Error(R_StructsCantBeDefinedInParam(structDecl->ToString()), param);
}


} // /namespace Xsc



// ================================================================================
