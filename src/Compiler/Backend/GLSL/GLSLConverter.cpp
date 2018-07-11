/*
 * GLSLConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLConverter.h"
#include "GLSLKeywords.h"
#include "ExprConverter.h"
#include "AST.h"
#include "ASTFactory.h"
#include "Exception.h"
#include "Helper.h"
#include "ReportIdents.h"
#include <utility>


namespace Xsc
{


/*
 * Internal members
 */

static const char* g_stdNameSelfParam   = "self";
static const char* g_stdNameBaseMember  = "base";
static const char* g_stdNameDummy       = "dummy";


/*
 * Internal structures
 */

struct CodeBlockStmntArgs
{
    bool disableNewScope;
};


/*
 * GLSLConverter class
 */

bool GLSLConverter::ConvertVarDeclType(VarDecl& varDecl)
{
    if (varDecl.semantic.IsSystemValue())
    {
        /* Convert data type for system value semantics */
        const auto dataType = SemanticToGLSLDataType(varDecl.semantic);
        if (dataType != DataType::Undefined)
            return ConvertVarDeclBaseTypeDenoter(varDecl, dataType);
    }
    return false;
}

bool GLSLConverter::ConvertVarDeclBaseTypeDenoter(VarDecl& varDecl, const DataType dataType)
{
    if (auto varDeclStmnt = varDecl.declStmntRef)
    {
        if (auto varTypeDen = varDecl.GetTypeDenoter()->GetAliased().As<BaseTypeDenoter>())
        {
            if (varTypeDen->dataType != dataType)
            {
                auto newVarTypeDen = std::make_shared<BaseTypeDenoter>(dataType);

                varDeclStmnt->typeSpecifier->typeDenoter = newVarTypeDen;
                varDeclStmnt->typeSpecifier->ResetTypeDenoter();

                /* Set custom type denoter for this variable */
                varDecl.SetCustomTypeDenoter(newVarTypeDen);

                /*
                ~~~~~~~~~~~~~~~ TODO: ~~~~~~~~~~~~~~~
                split declaration statement into three parts:
                1. All variables before this one
                2. This variable
                3. All variables after this one

                Example of converting 'b' (Before):
                    "uint a, b = a, c = b;"

                Example of converting 'b' (After):
                    "uint a; int b = (int)a; uint c = (uint)b;"
                */

                return true;
            }
        }
    }
    return false;
}


/*
 * ======= Private: =======
 */

void GLSLConverter::ConvertASTPrimary(Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store settings */
    shaderTarget_       = inputDesc.shaderTarget;
    versionOut_         = outputDesc.shaderVersion;
    options_            = outputDesc.options;
    autoBinding_        = outputDesc.options.autoBinding;
    autoBindingSlot_    = outputDesc.options.autoBindingStartSlot;
    separateSamplers_   = outputDesc.options.separateSamplers;

    /* Visit program AST */
    Visit(&program);
}

bool GLSLConverter::IsVKSL() const
{
    return IsLanguageVKSL(versionOut_);
}

bool GLSLConverter::UseSeparateSamplers() const
{
    return ( IsVKSL() && separateSamplers_ );
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    auto entryPoint = ast->entryPointRef;

    /* Register all input and output semantic variables as reserved identifiers */
    switch (shaderTarget_)
    {
        case ShaderTarget::VertexShader:
            if (GetNameMangling().useAlwaysSemantics)
                RenameIdentOfInOutVarDecls(entryPoint->inputSemantics.varDeclRefs, true, true);
            RenameIdentOfInOutVarDecls(entryPoint->outputSemantics.varDeclRefs, false);
            break;

        case ShaderTarget::FragmentShader:
            RenameIdentOfInOutVarDecls(entryPoint->inputSemantics.varDeclRefs, true);
            if (GetNameMangling().useAlwaysSemantics)
                RenameIdentOfInOutVarDecls(entryPoint->outputSemantics.varDeclRefs, false, true);
            break;

        default:
            RenameIdentOfInOutVarDecls(entryPoint->inputSemantics.varDeclRefs, true);
            RenameIdentOfInOutVarDecls(entryPoint->outputSemantics.varDeclRefs, false);
            break;
    }

    RegisterGlobalDeclIdents(entryPoint->inputSemantics.varDeclRefs);
    RegisterGlobalDeclIdents(entryPoint->outputSemantics.varDeclRefs);

    RegisterGlobalDeclIdents(entryPoint->inputSemantics.varDeclRefsSV);
    RegisterGlobalDeclIdents(entryPoint->outputSemantics.varDeclRefsSV);

    /* Add missing interpolation modifiers */
    if (shaderTarget_ != ShaderTarget::VertexShader && shaderTarget_ != ShaderTarget::ComputeShader)
        AddMissingInterpModifiers(entryPoint->inputSemantics.varDeclRefs);
    if (shaderTarget_ != ShaderTarget::FragmentShader && shaderTarget_ != ShaderTarget::ComputeShader)
        AddMissingInterpModifiers(entryPoint->outputSemantics.varDeclRefs);

    VisitScopedStmntList(ast->globalStmnts);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    RemoveDeadCode(ast->stmnts);

    UnrollStmnts(ast->stmnts);

    VisitScopedStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    Visit(ast->prefixExpr);

    if (ast->intrinsic != Intrinsic::Undefined)
    {
        /* Insert prefix expression as first argument into function call, if this is a texture intrinsic call */
        if (IsTextureIntrinsic(ast->intrinsic) && ast->prefixExpr)
        {
            /* Move object prefix into argument list as first argument */
            ast->PushPrefixToArguments();

            if (UseSeparateSamplers() && !IsTextureLoadIntrinsic(ast->intrinsic) && ast->arguments.size() >= 2)
            {
                /* Is second argument a sampler state object? */
                auto arg1Expr = ast->arguments[1].get();
                if (IsSamplerStateTypeDenoter(arg1Expr->GetTypeDenoter()))
                {
                    /* Merge texture object and sampler state arguments into texture/sampler binding call */
                    ast->MergeArguments(0, ASTFactory::MakeTextureSamplerBindingCallExpr);
                }
            }
        }
    }

    if (!UseSeparateSamplers())
    {
        /* Remove arguments which contain a sampler state object, since GLSL does not support sampler states */
        MoveAllIf(
            ast->arguments,
            GetProgram()->disabledAST,
            [&](const ExprPtr& expr)
            {
                return IsSamplerStateTypeDenoter(expr->GetTypeDenoter());
            }
        );
    }

    if (ast->intrinsic != Intrinsic::Undefined)
        ConvertIntrinsicCall(ast);
    else
        ConvertFunctionCall(ast);

    VISIT_DEFAULT(CallExpr);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    RemoveDeadCode(ast->stmnts);

    Visit(ast->expr);
    VisitScopedStmntList(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    /* Replace compatible struct types */
    auto typeDen = ast->typeDenoter->GetSub();
    if (auto structTypeDen = typeDen->As<StructTypeDenoter>())
    {
        if (auto structDecl = structTypeDen->structDeclRef)
        {
            if (structDecl->compatibleStructRef)
            {
                /* Replace original struct reference by its compatible struct type */
                structTypeDen->structDeclRef = structDecl->compatibleStructRef;

                /* Drop original structure declaration */
                GetProgram()->disabledAST.emplace_back(std::move(ast->structDecl));
            }
        }
    }

    VISIT_DEFAULT(TypeSpecifier);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    /* Rename static member variables */
    if (ast->IsStatic())
    {
        if (auto structDecl = ast->structDeclRef)
        {
            /* Rename function to "{TempPrefix}{StructName}_{VarName}" */
            ast->ident = structDecl->ident + "_" + ast->ident;
            ast->ident.AppendPrefix(GetNameMangling().namespacePrefix);
        }
    }

    RegisterDeclIdent(ast);
    VISIT_DEFAULT(VarDecl);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    RegisterDeclIdent(ast);
    ConvertSlotRegisters(ast->slotRegisters);
    VISIT_DEFAULT(BufferDecl);
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    RegisterDeclIdent(ast);
    ConvertSlotRegisters(ast->slotRegisters);
    VISIT_DEFAULT(SamplerDecl);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (!ast->IsAnonymous())
    {
        while (Fetch(ast->ident))
            RenameIdentOf(ast);

        RegisterDeclIdent(ast);
    }

    LabelAnonymousDecl(ast);
    RenameReservedKeyword(ast->ident);

    if (auto baseStruct = ast->baseStructRef)
    {
        /* Insert member of 'base' object */
        auto baseMemberTypeDen  = std::make_shared<StructTypeDenoter>(baseStruct);
        auto baseMemberType     = ASTFactory::MakeTypeSpecifier(baseMemberTypeDen);
        auto baseMember         = ASTFactory::MakeVarDeclStmnt(baseMemberType, GetNameMangling().namespacePrefix + g_stdNameBaseMember);

        baseMember->flags << VarDeclStmnt::isBaseMember;
        baseMember->varDecls.front()->structDeclRef = ast;

        ast->localStmnts.insert(ast->localStmnts.begin(), baseMember);
        ast->varMembers.insert(ast->varMembers.begin(), baseMember);
    }

    PushStructDecl(ast);
    OpenScope();
    {
        VisitScopedStmntList(ast->localStmnts);
    }
    CloseScope();
    PopStructDecl();

    if (!UseSeparateSamplers())
        RemoveSamplerStateVarDeclStmnts(ast->varMembers);

    /* Moved nested struct declarations out of the uniform buffer declaration */
    MoveNestedStructDecls(ast->localStmnts, false);

    /* Is this an empty structure? */
    if (ast->NumMemberVariables(true) == 0)
    {
        /* Add dummy member if the structure is empty (GLSL does not support empty structures) */
        auto dummyMember = ASTFactory::MakeVarDeclStmnt(DataType::Int, GetNameMangling().temporaryPrefix + g_stdNameDummy);
        ast->varMembers.push_back(dummyMember);
    }
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    PushFunctionDecl(ast);
    OpenScope();
    {
        ConvertFunctionDecl(ast);
    }
    CloseScope();
    PopFunctionDecl();
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    PushUniformBufferDecl(ast);
    {
        ConvertSlotRegisters(ast->slotRegisters);
        VisitScopedStmntList(ast->localStmnts);

        /* Moved nested struct declarations out of the uniform buffer declaration */
        MoveNestedStructDecls(ast->localStmnts, true);
    }
    PopUniformBufferDecl();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    /* Push optional array dimensions at the front of all variable declarations */
    auto subTypeDen = ast->typeSpecifier->typeDenoter;

    while (subTypeDen)
    {
        const auto& typeDen = subTypeDen->GetAliased();
        if (auto arrayTypeDen = typeDen.As<ArrayTypeDenoter>())
        {
            /* Copy array dimensions and insert them into front of all variable declarations */
            for (auto& varDecl : ast->varDecls)
            {
                varDecl->arrayDims.insert(
                    varDecl->arrayDims.begin(),
                    arrayTypeDen->arrayDims.begin(),
                    arrayTypeDen->arrayDims.end()
                );
            }

            /* Get last sub type denoter */
            subTypeDen = arrayTypeDen->subTypeDenoter;
        }
        else
            break;
    }

    /* Take latest sub type denoter */
    ast->typeSpecifier->typeDenoter = subTypeDen;

    if (InsideUniformBufferDecl())
    {
        /* Swap 'row_major' with 'column_major' storage layout for matrix types */
        const auto& typeDen = ast->typeSpecifier->typeDenoter->GetAliased();
        if (typeDen.IsMatrix())
            ast->typeSpecifier->SwapMatrixStorageLayout(TypeModifier::RowMajor);
    }

    if (!InsideFunctionDecl())
    {
        /* Remove const type modifier from variables that are out of local function scope */
        ast->typeSpecifier->typeModifiers.erase(TypeModifier::Const);
    }

    VISIT_DEFAULT(VarDeclStmnt);
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    /* Add name to structure declaration, if the structure is anonymous */
    if (ast->structDecl && ast->structDecl->ident.Empty() && !ast->aliasDecls.empty())
    {
        /* Use first alias name as structure name (alias names will disappear in GLSL output) */
        ast->structDecl->ident = ast->aliasDecls.front()->ident;

        /* Update type denoters of all alias declarations */
        for (auto& aliasDecl : ast->aliasDecls)
            aliasDecl->typeDenoter->SetIdentIfAnonymous(ast->structDecl->ident);
    }

    VISIT_DEFAULT(AliasDeclStmnt);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    bool disableNewScope = (args != nullptr ? reinterpret_cast<CodeBlockStmntArgs*>(args)->disableNewScope : false);

    if (!disableNewScope)
    {
        OpenScope();
        {
            Visit(ast->codeBlock);
        }
        CloseScope();
    }
    else
        Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    ConvertEntryPointReturnStmntToCodeBlock(ast->bodyStmnt);

    OpenScope();
    {
        Visit(ast->initStmnt);
        Visit(ast->condition);
        Visit(ast->iteration);

        if (ast->bodyStmnt->Type() == AST::Types::CodeBlockStmnt)
        {
            /* Do NOT open a new scope for the body code block in GLSL */
            CodeBlockStmntArgs bodyStmntArgs;
            bodyStmntArgs.disableNewScope = true;
            VisitScopedStmnt(ast->bodyStmnt, &bodyStmntArgs);
        }
        else
            VisitScopedStmnt(ast->bodyStmnt);
    }
    CloseScope();

    /* Ensure boolean scalar type for condition */
    ExprConverter::ConvertExprIfCastRequired(ast->condition, DataType::Bool);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    ConvertEntryPointReturnStmntToCodeBlock(ast->bodyStmnt);

    OpenScope();
    {
        Visit(ast->condition);
        VisitScopedStmnt(ast->bodyStmnt);
    }
    CloseScope();

    /* Ensure boolean scalar type for condition */
    ExprConverter::ConvertExprIfCastRequired(ast->condition, DataType::Bool);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    ConvertEntryPointReturnStmntToCodeBlock(ast->bodyStmnt);

    OpenScope();
    {
        VisitScopedStmnt(ast->bodyStmnt);
        Visit(ast->condition);
    }
    CloseScope();

    /* Ensure boolean scalar type for condition */
    ExprConverter::ConvertExprIfCastRequired(ast->condition, DataType::Bool);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    ConvertEntryPointReturnStmntToCodeBlock(ast->bodyStmnt);

    OpenScope();
    {
        Visit(ast->condition);
        VisitScopedStmnt(ast->bodyStmnt);
        Visit(ast->elseStmnt);
    }
    CloseScope();

    /* Ensure boolean scalar type for condition */
    ExprConverter::ConvertExprIfCastRequired(ast->condition, DataType::Bool);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    ConvertEntryPointReturnStmntToCodeBlock(ast->bodyStmnt);

    OpenScope();
    {
        VisitScopedStmnt(ast->bodyStmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    OpenScope();
    {
        VISIT_DEFAULT(SwitchStmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    VISIT_DEFAULT(ReturnStmnt);

    /* Check for cast expressions in entry-point return statements */
    if (InsideEntryPoint() && ast->expr != nullptr)
    {
        if (auto castExpr = AST::GetAs<CastExpr>(ast->expr->FindFirstOf(AST::Types::CastExpr)))
        {
            const auto& typeDen = castExpr->GetTypeDenoter();
            if (auto structTypeDen = typeDen->GetAliased().As<StructTypeDenoter>())
            {
                if (auto structDecl = structTypeDen->structDeclRef)
                {
                    /* Convert cast expression to assignment of structure members */
                    ConvertEntryPointReturnStmnt(*ast, structDecl, typeDen, castExpr->expr);
                }
            }
        }
    }
}

/* --- Expressions --- */

//TODO:
//  move this to "ExprConverter" class,
//  and make a correct conversion of "CastExpr" for a struct-constructor (don't use SequenceExpr here)
#if 1

IMPLEMENT_VISIT_PROC(CastExpr)
{
    /* Call default visitor first, then convert to avoid multiple conversions on its sub expressions */
    VISIT_DEFAULT(CastExpr);

    /* Check if the expression must be extended for a struct c'tor */
    const auto& typeDen = ast->typeSpecifier->GetTypeDenoter()->GetAliased();
    if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
    {
        if (auto structDecl = structTypeDen->structDeclRef)
        {
            /* Get the type denoter of all structure members */
            std::vector<TypeDenoterPtr> memberTypeDens;
            structDecl->CollectMemberTypeDenoters(memberTypeDens, false);

            /* Convert sub expression for structure c'tor */
            if (ast->expr->FindFirstOf(AST::Types::CallExpr))
            {
                /* Generate temporary variable with call expression, and insert its declaration statement before the cast expression */
                auto tempVarIdent           = MakeTempVarIdent();
                auto tempVarTypeSpecifier   = ASTFactory::MakeTypeSpecifier(ast->expr->GetTypeDenoter());
                auto tempVarDeclStmnt       = ASTFactory::MakeVarDeclStmnt(tempVarTypeSpecifier, tempVarIdent, ast->expr);
                auto tempVarExpr            = ASTFactory::MakeObjectExpr(tempVarDeclStmnt->varDecls.front().get());

                ast->expr = ASTFactory::MakeConstructorListExpr(tempVarExpr, memberTypeDens);

                InsertStmntBefore(tempVarDeclStmnt);
            }
            else
            {
                /* Generate list expression with N copies of the expression (where N is the number of struct members) */
                ast->expr = ASTFactory::MakeConstructorListExpr(ast->expr, memberTypeDens);
            }
        }
    }
    else if (auto arrayTypeDen = typeDen.As<ArrayTypeDenoter>())
    {
        //TODO...
    }
}

#endif

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    ConvertObjectExpr(ast);
    VISIT_DEFAULT(ObjectExpr);
}

#undef IMPLEMENT_VISIT_PROC

/* ----- Scope functions ----- */

void GLSLConverter::RegisterDeclIdent(Decl* obj, bool global)
{
    /* Rename declaration object if required */
    if (MustRenameDeclIdent(obj))
        RenameIdentOf(obj);

    /* Rename declaration object if it has a reserved keyword */
    RenameReservedKeyword(obj->ident);

    if (global)
    {
        /* Append object to global reserved declaration objects */
        globalReservedDecls_.push_back(obj);
    }
    else
    {
        /* Register identifier in symbol table */
        try
        {
            Register(obj->ident);
        }
        catch (const std::exception& e)
        {
            //TODO: throw an "internal error"
            RuntimeErr(e.what(), obj);
        }
    }
}

void GLSLConverter::RegisterGlobalDeclIdents(const std::vector<VarDecl*>& varDecls)
{
    for (auto varDecl : varDecls)
        RegisterDeclIdent(varDecl, true);
}

/* --- Helper functions for conversion --- */

bool GLSLConverter::MustRenameDeclIdent(const Decl* obj) const
{
    if (auto varDeclObj = obj->As<VarDecl>())
    {
        /*
        Variables must be renamed if they are not inside a structure declaration and their names are reserved,
        or the identifier has already been declared in the current scope
        */
        if (InsideStructDecl() || varDeclObj->flags(VarDecl::isShaderInput))
            return false;

        /* Does the declaration object has a globally reserved identifier? */
        const auto it = std::find_if(
            globalReservedDecls_.begin(), globalReservedDecls_.end(),
            [varDeclObj](const Decl* compareObj)
            {
                return (compareObj->ident == varDeclObj->ident);
            }
        );

        if (it != globalReservedDecls_.end())
        {
            /* Is the declaration object the reserved variable? */
            return (*it != obj);
        }
    }

    /* Check if identifier has already been declared in the current scope */
    if (FetchFromCurrentScope(obj->ident))
        return true;

    return false;
}

void GLSLConverter::RemoveSamplerStateVarDeclStmnts(std::vector<VarDeclStmntPtr>& stmnts)
{
    /* Move all variables to disabled code which are sampler state objects, since GLSL does not support sampler states */
    MoveAllIf(
        stmnts,
        GetProgram()->disabledAST,
        [&](const VarDeclStmntPtr& varDeclStmnt)
        {
            return IsSamplerStateTypeDenoter(varDeclStmnt->typeSpecifier->GetTypeDenoter());
        }
    );
}

bool GLSLConverter::RenameReservedKeyword(Identifier& ident)
{
    if (options_.obfuscate)
    {
        /* Set output identifier to an obfuscated number */
        RenameIdentObfuscated(ident);
        return true;
    }
    else
    {
        const auto& reservedKeywords = ReservedGLSLKeywords();

        /* Perform name mangling on output identifier if the input identifier is a reserved name */
        auto it = reservedKeywords.find(ident);
        if (it != reservedKeywords.end())
        {
            ident.AppendPrefix(GetNameMangling().reservedWordPrefix);
            return true;
        }

        /* Check if identifier begins with "gl_" */
        if (ident.Final().compare(0, 3, "gl_") == 0)
        {
            ident.AppendPrefix(GetNameMangling().reservedWordPrefix);
            return true;
        }

        return false;
    }
}

/* ----- Function declaration ----- */

void GLSLConverter::ConvertFunctionDecl(FunctionDecl* ast)
{
    /* Convert member function to global function */
    VarDecl* selfParamVar = nullptr;

    if (auto structDecl = ast->structDeclRef)
    {
        if (!ast->IsStatic())
        {
            /* Insert parameter of 'self' object */
            auto selfParamTypeDen   = std::make_shared<StructTypeDenoter>(structDecl);
            auto selfParamType      = ASTFactory::MakeTypeSpecifier(selfParamTypeDen);
            auto selfParam          = ASTFactory::MakeVarDeclStmnt(selfParamType, GetNameMangling().namespacePrefix + g_stdNameSelfParam);

            selfParam->flags << VarDeclStmnt::isSelfParameter;

            ast->parameters.insert(ast->parameters.begin(), selfParam);

            selfParamVar = selfParam->varDecls.front().get();
        }
    }

    if (selfParamVar)
        PushSelfParameter(selfParamVar);

    RenameReservedKeyword(ast->ident);

    if (ast->flags(FunctionDecl::isEntryPoint))
        ConvertFunctionDeclEntryPoint(ast);
    else
        ConvertFunctionDeclDefault(ast);

    if (!UseSeparateSamplers())
        RemoveSamplerStateVarDeclStmnts(ast->parameters);

    if (selfParamVar)
        PopSelfParameter();
}

void GLSLConverter::ConvertFunctionDeclDefault(FunctionDecl* ast)
{
    /* Default visitor */
    Visitor::VisitFunctionDecl(ast, nullptr);
}

static void AddFlagsToStructMembers(const TypeDenoter& typeDen, const Flags& flags)
{
    if (auto structTypeDen = typeDen.GetAliased().As<StructTypeDenoter>())
    {
        if (auto structDecl = structTypeDen->structDeclRef)
        {
            structDecl->ForEachVarDecl(
                [&flags](VarDeclPtr& member)
                {
                    member->flags << flags;
                }
            );
        }
    }
}

static BufferType GetBufferTypeOrDefault(const TypeDenoter& typeDen)
{
    if (auto bufferTypeDen = typeDen.GetAliased().As<BufferTypeDenoter>())
        return bufferTypeDen->bufferType;
    else
        return BufferType::Undefined;
}

/*
Returns true if the specified type results in a dynamic array in GLSL,
e.g. "InputPatch<float4> positions" becomes "vec4 positions[]"
*/
static bool IsTypeDynamicArrayInGLSL(const TypeDenoter& typeDen)
{
    return (typeDen.IsArray() || IsPatchBufferType(GetBufferTypeOrDefault(typeDen)));
}

void GLSLConverter::ConvertFunctionDeclEntryPoint(FunctionDecl* ast)
{
    /* Propagate array parameter declaration to input/output semantics */
    for (auto param : ast->parameters)
    {
        if (!param->varDecls.empty())
        {
            auto varDecl = param->varDecls.front();

            const auto& typeDen = varDecl->GetTypeDenoter()->GetAliased();
            if (IsTypeDynamicArrayInGLSL(typeDen))
            {
                /* Mark this member and all structure members as dynamic array */
                varDecl->flags << VarDecl::isDynamicArray;

                if (auto subTypeDen = typeDen.FetchSubTypeDenoter())
                    AddFlagsToStructMembers(*subTypeDen, VarDecl::isDynamicArray);
            }
        }
    }

    /* Default visitor */
    Visitor::VisitFunctionDecl(ast, nullptr);
}

/* ----- Call expressions ----- */

void GLSLConverter::ConvertIntrinsicCall(CallExpr* ast)
{
    switch (ast->intrinsic)
    {
        case Intrinsic::InterlockedAdd:
        case Intrinsic::InterlockedAnd:
        case Intrinsic::InterlockedOr:
        case Intrinsic::InterlockedXor:
        case Intrinsic::InterlockedMin:
        case Intrinsic::InterlockedMax:
        case Intrinsic::InterlockedCompareExchange:
        case Intrinsic::InterlockedExchange:
            ConvertIntrinsicCallImageAtomic(ast);
            break;

        case Intrinsic::Saturate:
            ConvertIntrinsicCallSaturate(ast);
            break;

        case Intrinsic::Tex1DLod:
        case Intrinsic::Tex2DLod:
        case Intrinsic::Tex3DLod:
        case Intrinsic::TexCubeLod:
            ConvertIntrinsicCallTextureLOD(ast);
            break;

        case Intrinsic::Texture_Sample_2:
        case Intrinsic::Texture_Sample_3:
        case Intrinsic::Texture_Sample_4:
        case Intrinsic::Texture_Sample_5:
            ConvertIntrinsicCallTextureSample(ast);
            break;

        case Intrinsic::Texture_SampleLevel_3:
        case Intrinsic::Texture_SampleLevel_4:
        case Intrinsic::Texture_SampleLevel_5:
            ConvertIntrinsicCallTextureSampleLevel(ast);
            break;

        case Intrinsic::Texture_Load_1:
        case Intrinsic::Texture_Load_2:
        case Intrinsic::Texture_Load_3:
            ConvertIntrinsicCallTextureLoad(ast);
            break;

        default:
            if (IsTextureGatherIntrisic(ast->intrinsic))
                ConvertIntrinsicCallGather(ast);
            else if (IsTextureCompareIntrinsic(ast->intrinsic))
                ConvertIntrinsicCallSampleCmp(ast);
            break;
    }
}

void GLSLConverter::ConvertIntrinsicCallSaturate(CallExpr* ast)
{
    /* Convert "saturate(x)" to "clamp(x, genType(0), genType(1))" */
    if (ast->arguments.size() == 1)
    {
        auto argTypeDen = ast->arguments.front()->GetTypeDenoter()->GetSub();
        if (argTypeDen->IsBase())
        {
            ast->intrinsic = Intrinsic::Clamp;
            ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, DataType::Int, "0"));
            ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, DataType::Int, "1"));
        }
        else
            RuntimeErr(R_InvalidIntrinsicArgType(ast->ident), ast->arguments.front().get());
    }
    else
        RuntimeErr(R_InvalidIntrinsicArgCount(ast->ident, 1, ast->arguments.size()), ast);
}

static int GetTextureDimFromIntrinsicCall(CallExpr* ast)
{
    /* Get buffer object from sample intrinsic call */
    if (!ast->arguments.empty())
        return ExprConverter::GetTextureDimFromExpr(ast->arguments.front().get(), ast);
    else
        RuntimeErr(R_FailedToGetTextureDim, ast);
}

void GLSLConverter::ConvertIntrinsicCallTextureLOD(CallExpr* ast)
{
    /* Convert "tex1Dlod(s, t)" to "textureLod(s, t.xyz, t.w)" (also for tex2Dlod, tex3Dlod, and texCUBElod) */
    if (ast->arguments.size() == 2)
    {
        auto& args = ast->arguments;

        /* Determine vector size for texture intrinsic */
        if (auto textureDim = ExprConverter::GetTextureDimFromExpr(args[0].get()))
        {
            /* Convert arguments */
            ExprConverter::ConvertExprIfCastRequired(args[1], DataType::Float4, true);

            /* Generate temporary variable with second argument, and insert its declaration statement before the intrinsic call */
            auto tempVarIdent           = MakeTempVarIdent();
            auto tempVarTypeSpecifier   = ASTFactory::MakeTypeSpecifier(args[1]->GetTypeDenoter());
            auto tempVarDeclStmnt       = ASTFactory::MakeVarDeclStmnt(tempVarTypeSpecifier, tempVarIdent, args[1]);
            auto tempVarExpr            = ASTFactory::MakeObjectExpr(tempVarIdent, tempVarDeclStmnt->varDecls.front().get());

            InsertStmntBefore(tempVarDeclStmnt);

            const std::string vectorSubscript = "xyzw";

            auto subExpr    = ASTFactory::MakeObjectExpr(tempVarDeclStmnt->varDecls.front().get());
            auto arg1Expr   = ASTFactory::MakeObjectExpr(subExpr, vectorSubscript.substr(0, textureDim));
            auto arg2Expr   = ASTFactory::MakeObjectExpr(subExpr, "w");

            args[1] = arg1Expr;
            args.push_back(arg2Expr);
        }
    }
    else
        RuntimeErr(R_InvalidIntrinsicArgCount(ast->ident, 2, ast->arguments.size()), ast);
}

void GLSLConverter::ConvertIntrinsicCallTextureSample(CallExpr* ast)
{
    /* Determine vector size for texture intrinsic */
    if (auto textureDim = GetTextureDimFromIntrinsicCall(ast))
    {
        /* Convert arguments */
        auto& args = ast->arguments;

        /* Ensure argument: float[1,2,3,4] Location */
        if (args.size() >= 2)
            ExprConverter::ConvertExprIfCastRequired(args[1], VectorDataType(DataType::Float, textureDim), true);

        /* Ensure argument: int[1,2,3] Offset */
        if (args.size() >= 3)
            ExprConverter::ConvertExprIfCastRequired(args[2], VectorDataType(DataType::Int, textureDim), true);
    }
}

void GLSLConverter::ConvertIntrinsicCallTextureSampleLevel(CallExpr* ast)
{
    /* Determine vector size for texture intrinsic */
    if (auto textureDim = GetTextureDimFromIntrinsicCall(ast))
    {
        /* Convert arguments */
        auto& args = ast->arguments;

        /* Ensure argument: float[1,2,3,4] Location */
        if (args.size() >= 2)
            ExprConverter::ConvertExprIfCastRequired(args[1], VectorDataType(DataType::Float, textureDim), true);

        /* Ensure argument: int[1,2,3] Offset */
        if (args.size() >= 4)
            ExprConverter::ConvertExprIfCastRequired(args[3], VectorDataType(DataType::Int, textureDim), true);
    }
}

void GLSLConverter::ConvertIntrinsicCallTextureLoad(CallExpr* ast)
{
    /* Determine vector size for texture intrinsic */
    if (auto textureDim = GetTextureDimFromIntrinsicCall(ast))
    {
        /* Validate number of arguments are in [2, 3] */
        auto& args = ast->arguments;
        if (args.size() < 2)
        {
            RuntimeErr(R_InvalidIntrinsicArgCount(ast->ident, 2, ast->arguments.size()), ast);
            return;
        }

        /* Get type denoter from texture object (first argument after it has been moved from the prefix) */
        const auto& typeDen = args[0]->GetTypeDenoter()->GetAliased();
        if (auto bufferTypeDen = typeDen.As<BufferTypeDenoter>())
        {
            if (bufferTypeDen->bufferType == BufferType::Texture2DMS || bufferTypeDen->bufferType == BufferType::Texture2DMSArray)
            {
                /* GLSL doesn't support offset for MS textures */
                if (ast->intrinsic == Intrinsic::Texture_Load_3)
                    RuntimeErr(R_FailedToMapClassIntrinsicOverload(ast->ident, BufferTypeToString(bufferTypeDen->bufferType)), ast);

                /* Ensure argument: int Sample */
                if (args.size() >= 3)
                    ExprConverter::ConvertExprIfCastRequired(args[2], DataType::Int, true);
            }
            else if (bufferTypeDen->bufferType == BufferType::Buffer)
            {
                /* No conversion for buffer loads */
            }
            else
            {
                /* Is second argument a 2D-vector? */
                const auto& arg1TypeDen = args[1]->GetTypeDenoter()->GetAliased();
                auto arg1BaseTypeDen = arg1TypeDen.As<BaseTypeDenoter>();

                if (arg1BaseTypeDen != nullptr && VectorTypeDim(arg1BaseTypeDen->dataType) < 3)
                {
                    /* Insert "0" literal after index argument (e.g. "texelFetch(tex, index)" to "texelFetch(tex, index, 0)") */
                    auto idxArgExpr = ASTFactory::MakeLiteralExpr(DataType::Int, "0");
                    args.insert(args.begin() + 2, idxArgExpr);
                }
                else
                {
                    /* Break up the location argument into separate coordinate and LOD arguments */
                    auto tempVarIdent           = MakeTempVarIdent();
                    auto tempVarTypeSpecifier   = ASTFactory::MakeTypeSpecifier(args[1]->GetTypeDenoter());
                    auto tempVarDeclStmnt       = ASTFactory::MakeVarDeclStmnt(tempVarTypeSpecifier, tempVarIdent, args[1]);

                    /* Insert temporary varibale declaration before the current statement, and visit this AST node for conversion */
                    InsertStmntBefore(tempVarDeclStmnt);
                    Visit(tempVarDeclStmnt);

                    /* Make new argument expressions */
                    const std::string vectorSubscript = "xyzw";

                    auto prefixExpr = ASTFactory::MakeObjectExpr(tempVarDeclStmnt->varDecls.front().get());
                    auto arg1Expr   = ASTFactory::MakeObjectExpr(prefixExpr, vectorSubscript.substr(0, textureDim));
                    auto arg2Expr   = ASTFactory::MakeObjectExpr(prefixExpr, vectorSubscript.substr(textureDim, 1));

                    args[1] = arg1Expr;
                    args.insert(args.begin() + 2, arg2Expr);

                    ExprConverter::ConvertExprIfCastRequired(args[2], DataType::Int, true);

                    /* Ensure argument: int[1,2,3] Offset */
                    if (args.size() >= 4)
                        ExprConverter::ConvertExprIfCastRequired(args[3], VectorDataType(DataType::Int, textureDim), true);
                }
            }

            /* Ensure argument: int[1,2,3] Location */
            ExprConverter::ConvertExprIfCastRequired(args[1], VectorDataType(DataType::Int, textureDim), true);
        }
    }
}

void GLSLConverter::ConvertIntrinsicCallImageAtomic(CallExpr* ast)
{
    auto& args = ast->arguments;

    if (args.size() >= 2)
    {
        DataType baseDataType = DataType::Int;
        BufferType bufferType = BufferType::Buffer;

        /* Convert "atomic*" to "imageAtomic*" for buffer types */
        const auto& arg0Expr = args.front();
        if (auto arg0ArrayExpr = arg0Expr->As<ArrayExpr>())
        {
            auto prefixTypeDen = arg0ArrayExpr->prefixExpr->GetTypeDenoter()->GetSub();

            std::size_t numDims = 0;
            if (auto arrayTypeDenoter = prefixTypeDen->As<ArrayTypeDenoter>())
            {
                numDims = arrayTypeDenoter->arrayDims.size();
                prefixTypeDen = arrayTypeDenoter->subTypeDenoter;
            }

            if (auto bufferTypeDen = prefixTypeDen->As<BufferTypeDenoter>())
            {
                bufferType = bufferTypeDen->bufferType;

                if (bufferTypeDen->genericTypeDenoter != nullptr)
                {
                    if (auto genericBaseTypeDen = bufferTypeDen->genericTypeDenoter->As<BaseTypeDenoter>())
                        baseDataType = BaseDataType(genericBaseTypeDen->dataType);
                }

                /* Is the buffer declaration a read/write texture? */
                if (IsRWImageBufferType(bufferType) && numDims < arg0ArrayExpr->NumIndices())
                {
                    /* Map interlocked intrinsic to image atomic intrinsic */
                    ast->intrinsic = InterlockedToImageAtomicIntrinsic(ast->intrinsic);

                    /* Insert array indices from object identifier after first argument */
                    args.insert(args.begin() + 1, arg0ArrayExpr->arrayIndices.back());

                    /* Replace by array sub-expression without the last index */
                    if (numDims > 0)
                    {
                        std::vector<ExprPtr> arrayIndices;
                        for (std::size_t i = 0; i < numDims; ++i)
                            arrayIndices.push_back(arg0ArrayExpr->arrayIndices[i]);

                        args.front() = ASTFactory::MakeArrayExpr(arg0ArrayExpr->prefixExpr, std::move(arrayIndices));
                    }
                    else
                        args.front() = arg0ArrayExpr->prefixExpr;
                }
            }
        }

        /* Cast arguments if required, for both image and non-image atomics */
        std::size_t dataArgOffset = 1;
        if (IsRWImageBufferType(bufferType))
        {
            /* Cast location argument */
            const auto numDims = GetBufferTypeTextureDim(bufferType);
            ExprConverter::ConvertExprIfCastRequired(args[1], VectorDataType(DataType::Int, numDims), true);
            dataArgOffset = 2;
        }

        if (args.size() >= (dataArgOffset + 1))
            ExprConverter::ConvertExprIfCastRequired(args[dataArgOffset], baseDataType, true);

        if(ast->intrinsic == Intrinsic::Image_AtomicCompSwap && args.size() >= (dataArgOffset + 2))
            ExprConverter::ConvertExprIfCastRequired(args[dataArgOffset + 1], baseDataType, true);
    }
}

void GLSLConverter::ConvertIntrinsicCallGather(CallExpr* ast)
{
    bool isCompare = IsTextureCompareIntrinsic(ast->intrinsic);

    /* If using separate offsets for each sample, convert four arguments into a single 4-element array argument */
    auto offsetCount = GetGatherIntrinsicOffsetParamCount(ast->intrinsic);
    if (offsetCount == 4)
    {
        /* Check if we have the valid number of arguments */
        int offsetArgStart  = (isCompare ? 3 : 2);
        int offsetArgEnd    = offsetArgStart + 4;

        if (ast->arguments.size() < static_cast<std::size_t>(offsetArgEnd))
        {
            RuntimeErr(R_InvalidIntrinsicArgCount(ast->ident, offsetArgEnd, ast->arguments.size()), ast);
            return;
        }

        /* Determine the type of the array */
        auto baseTypeDenoter = std::make_shared<BaseTypeDenoter>();
        baseTypeDenoter->dataType = DataType::Int2;

        std::vector<ArrayDimensionPtr> arrayDims;
        arrayDims.push_back(ASTFactory::MakeArrayDimension(4));

        auto arrayTypeDenoter = std::make_shared<ArrayTypeDenoter>(baseTypeDenoter, arrayDims);

        /* Place the arguments into the array */
        std::vector<ExprPtr> arrayCtorArguments;
        for (int i = offsetArgStart; i < offsetArgEnd; ++i)
        {
            auto& argument = ast->arguments[i];
            ExprConverter::ConvertExprIfCastRequired(argument, DataType::Int2, true);

            arrayCtorArguments.push_back(argument);
        }

        auto arrayCtorExpr = ASTFactory::MakeTypeCtorCallExpr(arrayTypeDenoter, arrayCtorArguments);

        /* Remove offset arguments and add the array argument in their place */
        ast->arguments.erase(ast->arguments.begin() + offsetArgStart, ast->arguments.begin() + offsetArgEnd);
        ast->arguments.insert(ast->arguments.begin() + offsetArgStart, arrayCtorExpr);
    }

    /* Add a component index argument based on the intrinsic type */
    if (!isCompare)
    {
        int componentIdx = GetGatherIntrinsicComponentIndex(ast->intrinsic);
        auto componentArgExpr = ASTFactory::MakeLiteralExpr(DataType::Int, std::to_string(componentIdx));

        ast->arguments.push_back(componentArgExpr);
    }
}

void GLSLConverter::ConvertIntrinsicCallSampleCmp(CallExpr* ast)
{
    /* Convert arguments */
    auto& args = ast->arguments;

    /* Determine vector size for texture intrinsic */
    if (auto textureDim = GetTextureDimFromIntrinsicCall(ast))
    {
        /* Ensure argument: float[1,2,3,4] Location */
        if (args.size() >= 2)
            ExprConverter::ConvertExprIfCastRequired(args[1], VectorDataType(DataType::Float, textureDim), true);

        /* Ensure argument: int[1,2,3] Offset */
        if (args.size() >= 4)
            ExprConverter::ConvertExprIfCastRequired(args[3], VectorDataType(DataType::Int, textureDim), true);

        /* Cast and move the compare argument as the last component of the Location argument */
        if (args.size() >= 3)
        {
            /* Ensure argument: float CompareValue */
            ExprConverter::ConvertExprIfCastRequired(args[2], DataType::Float, true);

            /* Not enough room if texture dimension is 4 (cube array), in which case the argument remains as is */
            if (textureDim < 4)
            {
                DataType targetType = VectorDataType(DataType::Float, textureDim + 1);
                auto typeDenoter = std::make_shared<BaseTypeDenoter>(targetType);

                args[1] = ASTFactory::MakeTypeCtorCallExpr(typeDenoter, { args[1], args[2] });
                args.erase(args.begin() + 2);
            }
        }
    }

    /* Insert '0' as the mip level parameter */
    if (IsTextureCompareLevelZeroIntrinsic(ast->intrinsic))
        args.insert(args.begin() + 2, ASTFactory::MakeLiteralExpr(DataType::Float, "0"));
}

void GLSLConverter::ConvertFunctionCall(CallExpr* ast)
{
    if (auto funcDecl = ast->GetFunctionDecl())
    {
        if (funcDecl->IsMemberFunction())
        {
            if (funcDecl->IsStatic())
            {
                /* Drop prefix expression, since GLSL does not only allow member functions */
                ast->prefixExpr.reset();
            }
            else
            {
                /* Get structure from prefix expression or active structure declaration */
                StructDecl* callerStructDecl = nullptr;

                if (ast->prefixExpr)
                {
                    const auto& typeDen = ast->prefixExpr->GetTypeDenoter()->GetAliased();
                    if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
                        callerStructDecl = structTypeDen->structDeclRef;
                }
                else
                    callerStructDecl = ActiveStructDecl();

                /* Insert 'self' or 'base' prefix if necessary */
                ConvertObjectPrefixStructMember(ast->prefixExpr, funcDecl->structDeclRef, callerStructDecl, (ast->prefixExpr == nullptr));

                /* Move prefix expression as argument into the function call */
                if (!ast->PushPrefixToArguments())
                    RuntimeErr(R_MissingSelfParamForMemberFunc(funcDecl->ToString()), ast);
            }
        }
    }
}

/* ----- Entry point ----- */

/*
~~~~~~~~~~~~~~~ TODO: refactor this ~~~~~~~~~~~~~~~
*/
void GLSLConverter::ConvertEntryPointStructPrefix(ExprPtr& expr, ObjectExpr* objectExpr)
{
    auto nonBracketExpr = expr->FindFirstNotOf(AST::Types::BracketExpr);
    if (auto prefixExpr = nonBracketExpr->As<ObjectExpr>())
        ConvertEntryPointStructPrefixObject(expr, prefixExpr, objectExpr);
    else if (auto prefixExpr = nonBracketExpr->As<ArrayExpr>())
        ConvertEntryPointStructPrefixArray(expr, prefixExpr, objectExpr);
}

// Marks the object expression as 'immutable', if the specified structure is a non-entry-point (NEP) parameter
static bool MakeObjectExprImmutableForNEPStruct(ObjectExpr* objectExpr, const StructDecl* structDecl)
{
    if (structDecl)
    {
        if (structDecl->flags(StructDecl::isNonEntryPointParam))
        {
            /* Mark object expression as immutable */
            objectExpr->flags << ObjectExpr::isImmutable;
            return true;
        }
    }
    return false;
}

void GLSLConverter::ConvertEntryPointStructPrefixObject(ExprPtr& expr, ObjectExpr* prefixExpr, ObjectExpr* objectExpr)
{
    /* Does this l-value refer to a variable declaration? */
    if (auto varDecl = prefixExpr->FetchVarDecl())
    {
        /* Is its type denoter a structure? */
        const auto& varTypeDen = varDecl->GetTypeDenoter()->GetAliased();
        if (auto structTypeDen = varTypeDen.As<StructTypeDenoter>())
        {
            /* Can the structure be resolved? */
            if (!MakeObjectExprImmutableForNEPStruct(objectExpr, structTypeDen->structDeclRef))
            {
                /* Drop prefix expression for global input/output variables */
                if (IsGlobalInOutVarDecl(objectExpr->FetchVarDecl()))
                    expr.reset();
            }
        }
    }
}

void GLSLConverter::ConvertEntryPointStructPrefixArray(ExprPtr& expr, ArrayExpr* prefixExpr, ObjectExpr* objectExpr)
{
    /* Does this l-value refer to a variable declaration? */
    if (auto varDecl = prefixExpr->prefixExpr->FetchVarDecl())
    {
        /* Is its type denoter an array of structures? */
        const auto& varTypeDen = varDecl->GetTypeDenoter()->GetAliased();
        if (auto arrayTypeDen = varTypeDen.As<ArrayTypeDenoter>())
        {
            const auto& varSubTypeDen = arrayTypeDen->subTypeDenoter->GetAliased();
            if (auto structTypeDen = varSubTypeDen.As<StructTypeDenoter>())
            {
                /* Can the structure be resolved? */
                MakeObjectExprImmutableForNEPStruct(objectExpr, structTypeDen->structDeclRef);
            }
        }
    }
}

void GLSLConverter::ConvertEntryPointReturnStmnt(ReturnStmnt& ast, StructDecl* structDecl, const TypeDenoterPtr& typeDen, const ExprPtr& typeConstructor)
{
    if (typeConstructor)
    {
        if (auto sequenceExpr = typeConstructor->As<SequenceExpr>())
            ConvertEntryPointReturnStmntSequenceExpr(ast, structDecl, typeDen, *sequenceExpr);
        else
            ConvertEntryPointReturnStmntCommonExpr(ast, structDecl, typeDen, typeConstructor);
    }
}

void GLSLConverter::ConvertEntryPointReturnStmntSequenceExpr(ReturnStmnt& ast, StructDecl* structDecl, const TypeDenoterPtr& typeDen, const SequenceExpr& typeConstructor)
{
    /* Make variable declaration of structure type */
    auto varIdent       = MakeTempVarIdent();
    auto varDeclStmnt   = ASTFactory::MakeVarDeclStmnt(ASTFactory::MakeTypeSpecifier(typeDen), varIdent);
    auto varDecl        = varDeclStmnt->varDecls.front().get();

    /* Mark variable as an entry-point local (will be removed in code generation) and entry-point output */
    varDecl->flags << (VarDecl::isEntryPointLocal | VarDecl::isEntryPointOutput);

    InsertStmntBefore(varDeclStmnt);

    /* Convert new statement */
    Visit(varDeclStmnt);

    /* Make member assignments */
    auto prefixExpr = ASTFactory::MakeObjectExpr(varDecl);

    std::size_t idx = 0;

    structDecl->ForEachVarDecl(
        [&](VarDeclPtr& varDecl)
        {
            if (idx < typeConstructor.exprs.size())
            {
                /* Make assignment statement for structure member */
                auto assignStmnt = ASTFactory::MakeAssignStmnt(
                    ASTFactory::MakeObjectExpr(prefixExpr, varDecl->ident.Original(), varDecl.get()),
                    typeConstructor.exprs[idx++]
                );

                InsertStmntBefore(assignStmnt);

                /* Convert new statement */
                Visit(assignStmnt);
            }
        }
    );

    /* Finally convert return statement */
    ast.expr = prefixExpr;

    /* Add variable as instance to this structure as entry point output */
    structDecl->AddShaderOutputInstance(varDecl);

    /* Add variable as parameter-structure to entry point */
    if (auto entryPoint = GetProgram()->entryPointRef)
        entryPoint->paramStructs.push_back({ ast.expr.get(), varDecl, structDecl });
}

void GLSLConverter::ConvertEntryPointReturnStmntCommonExpr(ReturnStmnt& ast, StructDecl* structDecl, const TypeDenoterPtr& typeDen, const ExprPtr& typeConstructor)
{
    /* Convert common expression into sequence expression to make use of the primary conversion function */
    SequenceExpr sequenceExpr(SourcePosition::ignore);
    {
        sequenceExpr.exprs.push_back(typeConstructor);
    }
    ConvertEntryPointReturnStmntSequenceExpr(ast, structDecl, typeDen, sequenceExpr);
}

void GLSLConverter::ConvertEntryPointReturnStmntToCodeBlock(StmntPtr& stmnt)
{
    /* Is this statement within the entry point? */
    if (InsideEntryPoint())
    {
        if (stmnt->Type() == AST::Types::ReturnStmnt)
        {
            /* Convert statement into a code block statement */
            stmnt = ASTFactory::MakeCodeBlockStmnt(stmnt);
        }
    }
}

void GLSLConverter::AddMissingInterpModifiers(const std::vector<VarDecl*>& varDecls)
{
    for (auto varDecl : varDecls)
    {
        if (auto baseTypeDen = varDecl->GetTypeDenoter()->GetAliased().As<BaseTypeDenoter>())
        {
            if (auto typeSpecifier = varDecl->FetchTypeSpecifier())
            {
                if (IsIntegralType(baseTypeDen->dataType))
                {
                    /* GLSL requires 'flat' modifier for integer types */
                    typeSpecifier->interpModifiers.insert(InterpModifier::NoInterpolation);
                }
            }
        }
    }
}

/* ----- Object expressions ----- */

void GLSLConverter::ConvertObjectExpr(ObjectExpr* objectExpr)
{
    /* Does this object expression refer to a static variable declaration? */
    if (auto varDecl = objectExpr->FetchVarDecl())
    {
        /*
        Don't convert 'self'-parameter again!
        But 'base'-member can be converted again, for base member access like 'object.Base::member'.
        */
        const auto varFlags = varDecl->declStmntRef->flags;
        if (!varFlags(VarDeclStmnt::isSelfParameter))
        {
            if (varDecl->IsStatic())
                ConvertObjectExprStaticVar(objectExpr);
            else
                ConvertObjectExprDefault(objectExpr);
        }
    }
    else
        ConvertObjectExprDefault(objectExpr);
}

void GLSLConverter::ConvertObjectExprStaticVar(ObjectExpr* objectExpr)
{
    /* Remove prefix from static variable access */
    objectExpr->prefixExpr.reset();
}

void GLSLConverter::ConvertObjectExprDefault(ObjectExpr* objectExpr)
{
    /* Convert prefix expression if it's the identifier of an entry-point struct instance */
    if (objectExpr->prefixExpr)
        ConvertEntryPointStructPrefix(objectExpr->prefixExpr, objectExpr);

    if (objectExpr->prefixExpr)
    {
        if (objectExpr->isStatic)
        {
            /* Convert prefix expression if it's a base structure namespace expression (e.g. "obj.BaseStruct::member" -> "obj.xsn_base.member") */
            ConvertObjectPrefixNamespace(objectExpr->prefixExpr, objectExpr);
        }
        else
        {
            /* Convert prefix expression if the object refers to a member variable of a base structure */
            ConvertObjectPrefixBaseStruct(objectExpr->prefixExpr, objectExpr);
        }
    }

    /* Add 'self'-parameter to front, if the variable refers to a member of the active structure */
    if (!objectExpr->prefixExpr)
        ConvertObjectPrefixSelfParam(objectExpr->prefixExpr, objectExpr);
}

void GLSLConverter::ConvertObjectPrefixStructMember(ExprPtr& prefixExpr, const StructDecl* ownerStructDecl, const StructDecl* callerStructDecl, bool useSelfParam)
{
    /* Does this variable belong to its structure type directly, or to a base structure? */
    if (ownerStructDecl && callerStructDecl)
    {
        if (ownerStructDecl->IsBaseOf(callerStructDecl, true))
        {
            if (useSelfParam)
            {
                if (auto selfParam = ActiveSelfParameter())
                {
                    /* Make the 'self'-parameter the new prefix expression */
                    prefixExpr = ASTFactory::MakeObjectExpr(selfParam);
                }
            }
        }

        /* Insert 'base' member prefixes */
        InsertBaseMemberPrefixes(prefixExpr, ownerStructDecl, callerStructDecl);
    }
}

void GLSLConverter::ConvertObjectPrefixSelfParam(ExprPtr& prefixExpr, ObjectExpr* objectExpr)
{
    /* Is this object a member of the active owner structure (like 'this->memberVar')? */
    if (auto activeStructDecl = ActiveStructDecl())
    {
        if (auto selfParam = ActiveSelfParameter())
        {
            if (auto varDecl = objectExpr->FetchVarDecl())
            {
                if (auto ownerStructDecl = varDecl->structDeclRef)
                {
                    /* Make the 'self'-parameter the new prefix expression */
                    prefixExpr = ASTFactory::MakeObjectExpr(selfParam);

                    /* Insert 'base' member prefixes */
                    InsertBaseMemberPrefixes(prefixExpr, ownerStructDecl, activeStructDecl);
                }
            }
        }
    }
}

void GLSLConverter::ConvertObjectPrefixBaseStruct(ExprPtr& prefixExpr, ObjectExpr* objectExpr)
{
    const auto& prefixTypeDen = prefixExpr->GetTypeDenoter()->GetAliased();
    if (auto prefixStructTypeDen = prefixTypeDen.As<StructTypeDenoter>())
    {
        if (auto activeStructDecl = prefixStructTypeDen->structDeclRef)
        {
            if (auto varDecl = objectExpr->FetchVarDecl())
            {
                /* Insert 'self' or 'base' prefix if necessary */
                ConvertObjectPrefixStructMember(prefixExpr, varDecl->structDeclRef, activeStructDecl, false);
            }
        }
    }
}

void GLSLConverter::ConvertObjectPrefixNamespace(ExprPtr& prefixExpr, ObjectExpr* objectExpr)
{
    /* Is the prefix expression a base structure namespace expression? */
    if (auto prefixObjectExpr = prefixExpr->As<ObjectExpr>())
    {
        /* Get base structure namespace */
        if (auto baseStructDecl = prefixObjectExpr->FetchSymbol<StructDecl>())
        {
            if (prefixObjectExpr->prefixExpr)
            {
                /* Fetch 'base'-member from prefix structure type */
                const auto& prefixTypeDen = prefixObjectExpr->prefixExpr->GetTypeDenoter()->GetAliased();
                if (auto prefixStructTypeDen = prefixTypeDen.As<StructTypeDenoter>())
                {
                    if (auto activeStructDecl = prefixStructTypeDen->structDeclRef)
                        ConvertObjectPrefixNamespaceStruct(prefixObjectExpr, objectExpr, baseStructDecl, activeStructDecl);
                }
            }
            else
            {
                /* Fetch 'base'-member from active structure declaration */
                if (auto activeStructDecl = ActiveStructDecl())
                    ConvertObjectPrefixNamespaceStruct(prefixObjectExpr, objectExpr, baseStructDecl, activeStructDecl);
            }
        }
    }
}

void GLSLConverter::ConvertObjectPrefixNamespaceStruct(ObjectExpr* prefixObjectExpr, ObjectExpr* objectExpr, const StructDecl* baseStructDecl, const StructDecl* activeStructDecl)
{
    if (activeStructDecl == baseStructDecl)
    {
        /* Remove this redundant prefix */
        objectExpr->isStatic    = false;
        objectExpr->prefixExpr  = prefixObjectExpr->prefixExpr;
    }
    else
    {
        /* Convert prefix expression from base struct namespace to 'base'-member */
        if (auto baseMember = activeStructDecl->FetchBaseMember())
        {
            objectExpr->isStatic        = false;
            prefixObjectExpr->symbolRef = baseMember;
            prefixObjectExpr->ident     = baseMember->ident.Original();
        }

        /* Insert further 'base' members until specified base structure namespace has reached */
        while (true)
        {
            /* Get next base structure */
            activeStructDecl = activeStructDecl->baseStructRef;
            if (!activeStructDecl || activeStructDecl == baseStructDecl)
                break;

            if (auto baseMember = activeStructDecl->FetchBaseMember())
            {
                /* Insert next 'base'-member object expression */
                objectExpr->prefixExpr = ASTFactory::MakeObjectExpr(objectExpr->prefixExpr, baseMember->ident, baseMember);
            }
            else
                break;
        }
    }
}

void GLSLConverter::InsertBaseMemberPrefixes(ExprPtr& prefixExpr, const StructDecl* ownerStructDecl, const StructDecl* callerStructDecl)
{
    if (ownerStructDecl->IsBaseOf(callerStructDecl))
    {
        while (callerStructDecl && callerStructDecl != ownerStructDecl)
        {
            /* Insert 'base' member object expression */
            if (auto baseMember = callerStructDecl->FetchBaseMember())
                prefixExpr = ASTFactory::MakeObjectExpr(prefixExpr, baseMember->ident.Original(), baseMember);

            /* Check for next base structure */
            callerStructDecl = callerStructDecl->baseStructRef;
        }
    }
}

/* ----- Unrolling ----- */

void GLSLConverter::UnrollStmnts(std::vector<StmntPtr>& stmnts)
{
    for (auto it = stmnts.begin(); it != stmnts.end();)
    {
        std::vector<StmntPtr> unrolledStmnts;

        auto ast = it->get();
        if (auto varDeclStmnt = ast->As<VarDeclStmnt>())
        {
            if (options_.unrollArrayInitializers)
                UnrollStmntsVarDecl(unrolledStmnts, varDeclStmnt);
        }

        ++it;

        if (!unrolledStmnts.empty())
        {
            it = stmnts.insert(it, unrolledStmnts.begin(), unrolledStmnts.end());
            it += unrolledStmnts.size();
        }
    }
}

void GLSLConverter::UnrollStmntsVarDecl(std::vector<StmntPtr>& unrolledStmnts, VarDeclStmnt* ast)
{
    /* Unroll all array initializers */
    for (const auto& varDecl : ast->varDecls)
    {
        if (varDecl->initializer)
            UnrollStmntsVarDeclInitializer(unrolledStmnts, varDecl.get());
    }
}

void GLSLConverter::UnrollStmntsVarDeclInitializer(std::vector<StmntPtr>& unrolledStmnts, VarDecl* varDecl)
{
    const auto& typeDen = varDecl->GetTypeDenoter()->GetAliased();
    if (auto arrayTypeDen = typeDen.As<ArrayTypeDenoter>())
    {
        /* Get initializer expression */
        if (auto initExpr = varDecl->initializer->As<InitializerExpr>())
        {
            /* Get dimension sizes of array type denoter */
            auto dimSizes = arrayTypeDen->GetDimensionSizes();
            std::vector<int> arrayIndices(dimSizes.size(), 0);

            /* Generate array element assignments until no further array index can be fetched */
            do
            {
                /* Fetch sub expression from initializer */
                auto subExpr = initExpr->FetchSubExpr(arrayIndices);

                /* Make new statement for current array element assignment */
                auto assignStmnt = ASTFactory::MakeArrayAssignStmnt(varDecl, arrayIndices, subExpr);

                /* Append new statement to list */
                unrolledStmnts.push_back(assignStmnt);
            }
            while (initExpr->NextArrayIndices(arrayIndices));

            /* Remove initializer after unrolling */
            varDecl->initializer.reset();
        }
    }
}

/* ----- Misc ----- */

void GLSLConverter::ConvertSlotRegisters(std::vector<RegisterPtr>& slotRegisters)
{
    if (autoBinding_)
    {
        slotRegisters.clear();
        slotRegisters.push_back(ASTFactory::MakeRegister(autoBindingSlot_++));
    }
}


} // /namespace Xsc



// ================================================================================
