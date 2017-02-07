/*
 * GLSLConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLConverter.h"
#include "GLSLKeywords.h"
#include "AST.h"
#include "ASTFactory.h"
#include "Exception.h"
#include "Helper.h"


namespace Xsc
{


void GLSLConverter::Convert(
    Program& program, const ShaderTarget shaderTarget, const std::string& nameManglingPrefix, const Options& options)
{
    /* Store settings */
    shaderTarget_       = shaderTarget;
    program_            = (&program);
    nameManglingPrefix_ = nameManglingPrefix;
    options_            = options;

    /* Visit program AST */
    Visit(program_);
}


/*
 * ======= Private: =======
 */

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Register all input and output semantic variables as reserved identifiers */
    switch (shaderTarget_)
    {
        case ShaderTarget::VertexShader:
            RenameInOutVarIdents(ast->entryPointRef->outputSemantics.varDeclRefs);
            RegisterReservedVarIdents(ast->entryPointRef->inputSemantics.varDeclRefs);
            RegisterReservedVarIdents(ast->entryPointRef->outputSemantics.varDeclRefs);
            break;
        case ShaderTarget::FragmentShader:
            RenameInOutVarIdents(ast->entryPointRef->inputSemantics.varDeclRefs);
            RegisterReservedVarIdents(ast->entryPointRef->inputSemantics.varDeclRefs);
            RegisterReservedVarIdents(ast->entryPointRef->outputSemantics.varDeclRefs);
            break;
        default:
            RenameInOutVarIdents(ast->entryPointRef->inputSemantics.varDeclRefs);
            RenameInOutVarIdents(ast->entryPointRef->outputSemantics.varDeclRefs);
            RegisterReservedVarIdents(ast->entryPointRef->inputSemantics.varDeclRefs);
            RegisterReservedVarIdents(ast->entryPointRef->outputSemantics.varDeclRefs);
            break;
    }

    RegisterReservedVarIdents(ast->entryPointRef->inputSemantics.varDeclRefsSV);
    RegisterReservedVarIdents(ast->entryPointRef->outputSemantics.varDeclRefsSV);

    VISIT_DEFAULT(Program);

    //TODO: do not remove these statements, instead mark it as disabled code (otherwise symbol references to these statements are corrupted!)
    #if 1
    /* Remove all variables which are sampler state objects, since GLSL does not support sampler states */
    MoveAllIf(
        ast->globalStmnts,
        program_->disabledAST,
        [&](const StmntPtr& stmnt)
        {
            if (stmnt->Type() == AST::Types::SamplerDeclStmnt)
                return true;
            if (auto varDeclStmnt = stmnt->As<VarDeclStmnt>())
                return IsSamplerStateTypeDenoter(varDeclStmnt->varType->GetTypeDenoter());
            return false;
        }
    );
    #endif
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    RemoveDeadCode(ast->stmnts);

    UnrollStmnts(ast->stmnts);

    VISIT_DEFAULT(CodeBlock);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    if (ast->intrinsic != Intrinsic::Undefined)
        ConvertIntrinsicCall(ast);

    /* Remove arguments which contain a sampler state object, since GLSL does not support sampler states */
    MoveAllIf(
        ast->arguments,
        program_->disabledAST,
        [&](const ExprPtr& expr)
        {
            return IsSamplerStateTypeDenoter(expr->GetTypeDenoter());
        }
    );

    /* Convert argument expressions */
    ast->ForEachArgumentWithParameter(
        [this](ExprPtr& funcArg, VarDeclPtr& funcParam)
        {
            auto paramTypeDen = funcParam->GetTypeDenoter()->Get();
            ConvertExprVectorSubscript(funcArg);
            ConvertExprIfCastRequired(funcArg, *paramTypeDen);
        }
    );

    /* Insert texture object as parameter into intrinsic arguments */
    if (IsTextureIntrinsic(ast->intrinsic))
    {
        auto texObjectArg = ASTFactory::MakeVarAccessExpr(ast->varIdent->ident, ast->varIdent->symbolRef);
        ast->arguments.insert(ast->arguments.begin(), texObjectArg);
    }

    VISIT_DEFAULT(FunctionCall);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    RemoveDeadCode(ast->stmnts);

    VISIT_DEFAULT(SwitchCase);
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    /* Has the variable identifier a next identifier? */
    if (ast->next)
    {
        /* Does this identifier refer to a variable declaration? */
        if (auto varDecl = ast->FetchVarDecl())
        {
            /* Is its type denoter a structure? */
            auto varTypeDen = varDecl->declStmntRef->varType->typeDenoter.get();
            if (auto structTypeDen = varTypeDen->As<StructTypeDenoter>())
            {
                /* Can the structure be resolved */
                auto structDecl = structTypeDen->structDeclRef;
                if (structDecl->flags(StructDecl::isNonEntryPointParam))
                {
                    /* Mark variable identifier to be immutable */
                    ast->flags << VarIdent::isImmutable;
                }
                else
                {
                    /* Pop front identifier node for global input/output variables */
                    PopFrontOfGlobalInOutVarIdent(ast);
                }
            }
        }
    }
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    /* Must this variable be renamed with name mangling? */
    if (MustRenameVarDecl(ast))
        RenameVarDecl(ast);

    RenameReservedKeyword(ast->ident, ast->renamedIdent);

    /* Must the initializer type denoter changed? */
    if (ast->initializer)
    {
        ConvertExprVectorSubscript(ast->initializer);
        ConvertExprIfCastRequired(ast->initializer, *ast->GetTypeDenoter()->Get());
    }

    VISIT_DEFAULT(VarDecl);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    LabelAnonymousStructDecl(ast);

    PushStructDecl(ast);
    {
        VISIT_DEFAULT(StructDecl);
    }
    PopStructDecl();

    RemoveSamplerStateVarDeclStmnts(ast->members);

    /* Is this an empty structure? */
    if (ast->members.empty())
    {
        /* Add dummy member if the structure is empty (GLSL does not support empty structures) */
        auto dummyMember = ASTFactory::MakeVarDeclStmnt(DataType::Int, nameManglingPrefix_ + "dummy");
        ast->members.push_back(dummyMember);
    }
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    PushFunctionDecl(ast);
    {
        ConvertFunctionDecl(ast);
    }
    PopFunctionDecl();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    /* Remove 'static' storage class (reserved word in GLSL) */
    ast->storageClasses.erase(StorageClass::Static);

    VISIT_DEFAULT(VarDeclStmnt);
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    /* Add name to structure declaration, if the structure is anonymous */
    if (ast->structDecl && ast->structDecl->ident.empty() && !ast->aliasDecls.empty())
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

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    MakeCodeBlockInEntryPointReturnStmnt(ast->bodyStmnt);

    VISIT_DEFAULT(ForLoopStmnt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    MakeCodeBlockInEntryPointReturnStmnt(ast->bodyStmnt);

    VISIT_DEFAULT(WhileLoopStmnt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    MakeCodeBlockInEntryPointReturnStmnt(ast->bodyStmnt);

    VISIT_DEFAULT(DoWhileLoopStmnt);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    MakeCodeBlockInEntryPointReturnStmnt(ast->bodyStmnt);

    VISIT_DEFAULT(IfStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    /* Ensure a code block as body statement (if the body is a return statement within the entry point) */
    MakeCodeBlockInEntryPointReturnStmnt(ast->bodyStmnt);

    VISIT_DEFAULT(ElseStmnt);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    ConvertExprVectorSubscript(ast->expr);
    VISIT_DEFAULT(ExprStmnt);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    if (ast->expr)
    {
        /* Convert return expression */
        ConvertExprVectorSubscript(ast->expr);
        if (ActiveFunctionDecl())
            ConvertExprIfCastRequired(ast->expr, *ActiveFunctionDecl()->returnType->typeDenoter->Get());
    }

    VISIT_DEFAULT(ReturnStmnt);
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    /* Replace 'h' and 'H' suffix with 'f' suffix */
    auto& s = ast->value;

    if (!s.empty())
    {
        if (s.back() == 'h' || s.back() == 'H')
        {
            s.back() = 'f';
            ast->dataType = DataType::Float;
        }
    }

    VISIT_DEFAULT(LiteralExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    VISIT_DEFAULT(BinaryExpr);

    /* Convert right-hand-side expression (if cast required) */
    ConvertExprIfCastRequired(ast->rhsExpr, *ast->lhsExpr->GetTypeDenoter()->Get());
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    /* Is the next sub expression again an unary expression? */
    if (ast->expr->Type() == AST::Types::UnaryExpr)
    {
        /* Insert bracket expression */
        auto bracketExpr = MakeShared<BracketExpr>(ast->area);
        
        bracketExpr->expr = ast->expr;

        ast->expr = bracketExpr;
    }

    VISIT_DEFAULT(UnaryExpr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    /* Check if the expression must be extended for a struct c'tor */
    if (auto structTypeDen = ast->typeExpr->GetTypeDenoter()->Get()->As<StructTypeDenoter>())
    {
        /* Get the type denoter of all structure members */
        auto structDecl = structTypeDen->structDeclRef;

        std::vector<TypeDenoterPtr> memberTypeDens;
        structDecl->CollectMemberTypeDenoters(memberTypeDens);

        /* Convert sub expression for structure c'tor */
        if (ast->expr->Type() == AST::Types::LiteralExpr)
        {
            /* Generate list expression with N copies literals (where N is the number of struct members) */
            auto literalExpr = std::static_pointer_cast<LiteralExpr>(ast->expr);
            ast->expr = ASTFactory::MakeConstructorListExpr(literalExpr, memberTypeDens);
        }
        /*else if ()
        {
            //TODO: temporary variable must be created and inserted before this expression,
            //      especially whan the sub expression contains a function call!
            //...
        }*/
    }
    
    VISIT_DEFAULT(CastExpr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    VISIT_DEFAULT(VarAccessExpr);

    if (ast->assignExpr)
    {
        /* Convert assignment expression */
        ConvertExprVectorSubscript(ast->assignExpr);
        ConvertExprIfCastRequired(ast->assignExpr, *ast->GetTypeDenoter()->Get());
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for conversion --- */

bool GLSLConverter::IsSamplerStateTypeDenoter(const TypeDenoterPtr& typeDenoter) const
{
    if (typeDenoter)
    {
        if (auto samplerTypeDen = typeDenoter->Get()->As<SamplerTypeDenoter>())
        {
            /* Is the sampler type a sampler-state type? */
            return IsSamplerStateType(samplerTypeDen->samplerType);
        }
    }
    return false;
}

bool GLSLConverter::MustRenameVarDecl(VarDecl* ast) const
{
    /* Variable must be renamed if it's not inside a structure declaration and its name is reserved */
    return
    (
        !InsideStructDecl() &&
        !ast->flags(VarDecl::isShaderInput) &&
        (
            std::find_if(
                reservedVarDecls_.begin(), reservedVarDecls_.end(),
                [ast](VarDecl* varDecl)
                {
                    return (varDecl != ast && varDecl->FinalIdent() == ast->FinalIdent());
                }
            ) != reservedVarDecls_.end()
        )
    );
}

void GLSLConverter::RenameVarDecl(VarDecl* ast, const std::string& ident)
{
    /* Set new identifier for this variable */
    ast->renamedIdent = (nameManglingPrefix_ + ident);
}

void GLSLConverter::RenameVarDecl(VarDecl* ast)
{
    RenameVarDecl(ast, ast->FinalIdent());
}

void GLSLConverter::RenameInOutVarIdents(const std::vector<VarDecl*>& varDecls)
{
    for (auto varDecl : varDecls)
        RenameVarDecl(varDecl, "vary_" + varDecl->semantic.ToString());
}

void GLSLConverter::LabelAnonymousStructDecl(StructDecl* ast)
{
    if (ast->IsAnonymous())
    {
        ast->ident = nameManglingPrefix_ + "anonymous_struct" + std::to_string(anonymousStructCounter_);
        ++anonymousStructCounter_;
    }
}

bool GLSLConverter::HasGlobalInOutVarDecl(VarIdent* varIdent) const
{
    /* Has variable identifier a reference to a variable declaration? */
    if (auto varDecl = varIdent->FetchVarDecl())
    {
        /* Is this variable a global input/output variable? */
        auto entryPoint = program_->entryPointRef;
        if (entryPoint->inputSemantics.Contains(varDecl) || entryPoint->outputSemantics.Contains(varDecl))
            return true;
    }
    return false;
}

void GLSLConverter::PopFrontOfGlobalInOutVarIdent(VarIdent* ast)
{
    auto root = ast;

    while (ast)
    {
        /* Refers the current identifier to a global input/output variable? */
        if (HasGlobalInOutVarDecl(ast))
        {
            /*
            Remove all leading AST nodes until this one, to convert this
            variable identifer to an identifier for a local variable
            */
            while (root && !HasGlobalInOutVarDecl(root))
            {
                root->PopFront();
                root = root->next.get();
            }

            /* Stop conversion process */
            break;
        }

        /* Continue search in next node */
        ast = ast->next.get();
    }
}

void GLSLConverter::MakeCodeBlockInEntryPointReturnStmnt(StmntPtr& bodyStmnt)
{
    /* Is this statement within the entry point? */
    if (InsideEntryPoint())
    {
        if (bodyStmnt->Type() == AST::Types::ReturnStmnt)
        {
            auto codeBlockStmnt = MakeShared<CodeBlockStmnt>(bodyStmnt->area);

            codeBlockStmnt->codeBlock = MakeShared<CodeBlock>(bodyStmnt->area);
            codeBlockStmnt->codeBlock->stmnts.push_back(bodyStmnt);

            bodyStmnt = codeBlockStmnt;
        }
    }
}

void GLSLConverter::RegisterReservedVarIdents(const std::vector<VarDecl*>& varDecls)
{
    for (auto& varDecl : varDecls)
    {
        /* Also new variables for reserved identifiers must be renamed if the name is already reserved */
        if (MustRenameVarDecl(varDecl))
            RenameVarDecl(varDecl);

        /* Add variable to reserved identifiers */
        reservedVarDecls_.push_back(varDecl);
    }
}

std::unique_ptr<DataType> GLSLConverter::MustCastExprToDataType(const DataType targetType, const DataType sourceType, bool matchTypeSize)
{
    /* Check for type mismatch */
    if ( ( matchTypeSize && VectorTypeDim(targetType) != VectorTypeDim(sourceType) ) ||
         ( IsUIntType    (targetType) && IsIntType     (sourceType) ) ||
         ( IsIntType     (targetType) && IsUIntType    (sourceType) ) ||
         ( IsRealType    (targetType) && IsIntegralType(sourceType) ) ||
         ( IsIntegralType(targetType) && IsRealType    (sourceType) ) )
    {
        /* Cast to target type required */
        return MakeUnique<DataType>(targetType);
    }
    return nullptr;
}

std::unique_ptr<DataType> GLSLConverter::MustCastExprToDataType(const TypeDenoter& targetTypeDen, const TypeDenoter& sourceTypeDen, bool matchTypeSize)
{
    if (auto baseTargetTypeDen = targetTypeDen.As<BaseTypeDenoter>())
    {
        if (auto baseSourceTypeDen = sourceTypeDen.As<BaseTypeDenoter>())
        {
            return MustCastExprToDataType(
                baseTargetTypeDen->dataType,
                baseSourceTypeDen->dataType,
                matchTypeSize
            );
        }
    }
    return nullptr;
}

void GLSLConverter::RemoveDeadCode(std::vector<StmntPtr>& stmnts)
{
    for (auto it = stmnts.begin(); it != stmnts.end();)
    {
        if ((*it)->flags(AST::isDeadCode))
            it = stmnts.erase(it);
        else
            ++it;
    }
}

void GLSLConverter::RemoveSamplerStateVarDeclStmnts(std::vector<VarDeclStmntPtr>& stmnts)
{
    /* Move all variables to disabled code which are sampler state objects, since GLSL does not support sampler states */
    MoveAllIf(
        stmnts,
        program_->disabledAST,
        [&](const VarDeclStmntPtr& varDeclStmnt)
        {
            return IsSamplerStateTypeDenoter(varDeclStmnt->varType->GetTypeDenoter());
        }
    );
}

bool GLSLConverter::RenameReservedKeyword(const std::string& ident, std::string& renamedIdent)
{
    if (options_.obfuscate)
    {
        /* Set output identifier to an obfuscated number */
        renamedIdent = "_" + std::to_string(obfuscationCounter_++);
        return true;
    }
    else
    {
        const auto& reservedKeywords = ReservedGLSLKeywords();

        /* Perform name mangling on output identifier if the input identifier is a reserved name */
        auto it = reservedKeywords.find(ident);
        if (it != reservedKeywords.end())
        {
            renamedIdent = nameManglingPrefix_ + ident;
            return true;
        }

        return false;
    }
}

/* ----- Conversion ----- */

void GLSLConverter::ConvertFunctionDecl(FunctionDecl* ast)
{
    RenameReservedKeyword(ast->ident, ast->renamedIdent);

    if (ast->flags(FunctionDecl::isEntryPoint))
        ConvertFunctionDeclEntryPoint(ast);
    else
        ConvertFunctionDeclDefault(ast);

    RemoveSamplerStateVarDeclStmnts(ast->parameters);
}

void GLSLConverter::ConvertFunctionDeclDefault(FunctionDecl* ast)
{
    /* Default visitor */
    Visitor::VisitFunctionDecl(ast, nullptr);
}

void GLSLConverter::ConvertFunctionDeclEntryPoint(FunctionDecl* ast)
{
    /* Propagate array parameter declaration to input/output semantics */
    for (auto param : ast->parameters)
    {
        if (!param->varDecls.empty())
        {
            auto varDecl = param->varDecls.front();
            auto typeDen = varDecl->GetTypeDenoter()->Get();
            if (auto arrayTypeDen = typeDen->As<ArrayTypeDenoter>())
            {
                /* Mark this member and all structure members as dynamic array */
                varDecl->flags << VarDecl::isDynamicArray;

                if (auto structBaseTypeDen = arrayTypeDen->baseTypeDenoter->Get()->As<StructTypeDenoter>())
                {
                    if (structBaseTypeDen->structDeclRef)
                    {
                        structBaseTypeDen->structDeclRef->ForEachVarDecl(
                            [](VarDecl* member)
                            {
                                member->flags << VarDecl::isDynamicArray;
                            }
                        );
                    }
                }
            }
        }
    }

    /* Default visitor */
    Visitor::VisitFunctionDecl(ast, nullptr);
}

void GLSLConverter::ConvertIntrinsicCall(FunctionCall* ast)
{
    switch (ast->intrinsic)
    {
        case Intrinsic::Saturate:
            ConvertIntrinsicCallSaturate(ast);
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
        case Intrinsic::StreamOutput_Append:
            ConvertIntrinsicCallStreamOutputAppend(ast);
            break;
        default:
            break;
    }
}

void GLSLConverter::ConvertIntrinsicCallSaturate(FunctionCall* ast)
{
    /* Convert "saturate(x)" to "clamp(x, genType(0), genType(1))" */
    if (ast->arguments.size() == 1)
    {
        auto argTypeDen = ast->arguments.front()->GetTypeDenoter()->Get();
        if (argTypeDen->IsBase())
        {
            ast->intrinsic = Intrinsic::Clamp;
            ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, DataType::Int, "0"));
            ast->arguments.push_back(ASTFactory::MakeLiteralCastExpr(argTypeDen, DataType::Int, "1"));
        }
        else
            RuntimeErr("invalid argument type denoter in intrinsic 'saturate'", ast->arguments.front().get());
    }
    else
        RuntimeErr("invalid number of arguments in intrinsic 'saturate'", ast);
}

static int GetTextureVectorSizeFromIntrinsicCall(FunctionCall* ast)
{
    /* Get buffer object from sample intrinsic call */
    if (auto symbolRef = ast->varIdent->symbolRef)
    {
        if (auto bufferDecl = symbolRef->As<BufferDecl>())
        {
            /* Determine vector size for texture intrinsic parametes */
            switch (bufferDecl->GetBufferType())
            {
                case BufferType::Texture1D:
                    return 1;
                case BufferType::Texture1DArray:
                case BufferType::Texture2D:
                case BufferType::Texture2DMS:
                    return 2;
                case BufferType::Texture2DArray:
                case BufferType::Texture2DMSArray:
                case BufferType::Texture3D:
                case BufferType::TextureCube:
                    return 3;
                case BufferType::TextureCubeArray:
                    return 4;
                default:
                    break;
            }
        }
    }
    return 0;
}

void GLSLConverter::ConvertIntrinsicCallTextureSample(FunctionCall* ast)
{
    /* Determine vector size for texture intrinsic */
    if (auto vectorSize = GetTextureVectorSizeFromIntrinsicCall(ast))
    {
        /* Convert arguments */
        auto& args = ast->arguments;

        /* Ensure argument: float[1,2,3,4] Location */
        if (args.size() >= 2)
            ConvertExprIfCastRequired(args[1], VectorDataType(DataType::Float, vectorSize), true);

        /* Ensure argument: int[1,2,3] Offset */
        if (args.size() >= 3)
            ConvertExprIfCastRequired(args[2], VectorDataType(DataType::Int, vectorSize), true);
    }
}

void GLSLConverter::ConvertIntrinsicCallTextureSampleLevel(FunctionCall* ast)
{
    /* Determine vector size for texture intrinsic */
    if (auto vectorSize = GetTextureVectorSizeFromIntrinsicCall(ast))
    {
        /* Convert arguments */
        auto& args = ast->arguments;

        /* Ensure argument: float[1,2,3,4] Location */
        if (args.size() >= 2)
            ConvertExprIfCastRequired(args[1], VectorDataType(DataType::Float, vectorSize), true);

        /* Ensure argument: int[1,2,3] Offset */
        if (args.size() >= 4)
            ConvertExprIfCastRequired(args[3], VectorDataType(DataType::Int, vectorSize), true);
    }
}

void GLSLConverter::ConvertIntrinsicCallStreamOutputAppend(FunctionCall* ast)
{
    /* Remove all arguments form this function call */
    MoveAll(ast->arguments, program_->disabledAST);
}

void GLSLConverter::ConvertExprVectorSubscript(ExprPtr& expr)
{
    if (expr)
    {
        if (auto suffixExpr = expr->As<SuffixExpr>())
            ConvertExprVectorSubscriptSuffix(expr, suffixExpr);
        else
            ConvertExprVectorSubscriptVarIdent(expr, expr->FetchVarIdent());
    }
}

void GLSLConverter::ConvertExprVectorSubscriptSuffix(ExprPtr& expr, SuffixExpr* suffixExpr)
{
    /* Get type denoter of sub expression */
    auto typeDen        = suffixExpr->expr->GetTypeDenoter()->Get();
    auto suffixIdentRef = &(suffixExpr->varIdent);
    auto varIdent       = suffixExpr->varIdent.get();

    /* Remove outer most vector subscripts from scalar types (i.e. 'func().xxx.xyz' to '((float3)func()).xyz' */
    while (varIdent)
    {
        if (varIdent->symbolRef)
        {
            /* Get type denoter for current variable identifier */
            typeDen = varIdent->GetExplicitTypeDenoter(false);
            suffixIdentRef = &(varIdent->next);
        }
        else if (typeDen->IsVector())
        {
            /* Get type denoter for current variable identifier from vector subscript */
            typeDen = varIdent->GetTypeDenoterFromSubscript(*typeDen);
            suffixIdentRef = &(varIdent->next);
        }
        else if (typeDen->IsScalar())
        {
            /* Drop suffix (but store shared pointer) */
            auto suffixIdent = *suffixIdentRef;
            suffixIdentRef->reset();

            /* Convert vector subscript to cast expression */
            auto vectorTypeDen = suffixIdent->GetTypeDenoterFromSubscript(*typeDen);

            /* Drop outer suffix expression if there is no suffix identifier (i.e. suffixExpr->varIdent) */
            ExprPtr castExpr;
            if (suffixExpr->varIdent)
                expr = ASTFactory::MakeCastOrSuffixCastExpr(vectorTypeDen, expr, suffixIdent->next);
            else
                expr = ASTFactory::MakeCastOrSuffixCastExpr(vectorTypeDen, suffixExpr->expr, suffixIdent->next);

            /* Repeat conversion until not vector subscripts remains */
            ConvertExprVectorSubscript(expr);
            return;
        }

        /* Go to next identifier */
        varIdent = varIdent->next.get();
    }
}

void GLSLConverter::ConvertExprVectorSubscriptVarIdent(ExprPtr& expr, VarIdent* varIdent)
{
    /* Remove outer most vector subscripts from scalar types (i.e. 'scalarValue.xxx.xyz' to '((float3)scalarValue).xyz' */
    while (varIdent && varIdent->next)
    {
        if (!varIdent->next->symbolRef)
        {
            auto typeDen = varIdent->GetExplicitTypeDenoter(false);
            if (typeDen->IsScalar())
            {
                /* Drop suffix (but store shared pointer) */
                auto suffixIdent = varIdent->next;
                varIdent->next.reset();

                /* Convert vector subscript to cast expression */
                auto vectorTypeDen = suffixIdent->GetTypeDenoterFromSubscript(*typeDen);
                expr = ASTFactory::MakeCastOrSuffixCastExpr(vectorTypeDen, expr, suffixIdent->next);

                /* Repeat conversion until not vector subscripts remains */
                ConvertExprVectorSubscript(expr);
                return;
            }
        }

        /* Go to next identifier */
        varIdent = varIdent->next.get();
    }
}

//TODO: this is incomplete
#if 0
void GLSLConverter::ConvertExprIfConstructorRequired(ExprPtr& expr)
{
    if (auto initExpr = expr->As<InitializerExpr>())
        expr = ASTFactory::ConvertInitializerExprToTypeConstructor(initExpr);
}
#endif

// Converts the expression to a cast expression if it is required for the specified target type.
void GLSLConverter::ConvertExprIfCastRequired(ExprPtr& expr, const DataType targetType, bool matchTypeSize)
{
    if (auto baseSourceTypeDen = expr->GetTypeDenoter()->Get()->As<BaseTypeDenoter>())
    {
        if (auto dataType = MustCastExprToDataType(targetType, baseSourceTypeDen->dataType, matchTypeSize))
        {
            /* Convert to cast expression with target data type if required */
            expr = ASTFactory::ConvertExprBaseType(*dataType, expr);
        }
    }
}

void GLSLConverter::ConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize)
{
    if (auto dataType = MustCastExprToDataType(targetTypeDen, *expr->GetTypeDenoter()->Get(), matchTypeSize))
    {
        /* Convert to cast expression with target data type if required */
        expr = ASTFactory::ConvertExprBaseType(*dataType, expr);
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
    auto typeDen = varDecl->GetTypeDenoter()->Get();

    if (auto arrayTypeDen = typeDen->As<ArrayTypeDenoter>())
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


} // /namespace Xsc



// ================================================================================
