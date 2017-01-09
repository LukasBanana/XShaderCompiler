/*
 * GLSLConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLConverter.h"
#include "GLSLHelper.h"
#include "GLSLIntrinsics.h"
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
    if (ast->next && ast->symbolRef)
    {
        /* Does this identifier refer to a variable declaration? */
        if (auto varDecl = ast->symbolRef->As<VarDecl>())
        {
            /* Is its type denoter a structure? */
            auto varTypeDen = varDecl->declStmntRef->varType->typeDenoter.get();
            if (auto structTypeDen = varTypeDen->As<StructTypeDenoter>())
            {
                /* Must the structure be resolved? */
                if (MustResolveStruct(structTypeDen->structDeclRef))
                {
                    /* Remove first identifier */
                    ast->PopFront();
                }
                else
                {
                    /* Has a sub node a system value semantic? */
                    MakeVarIdentWithSystemSemanticLocal(ast);
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

    RenameReservedFunctionName(ast->ident, ast->renamedIdent);

    /* Must the initializer type denoter changed? */
    if (ast->initializer)
    {
        /* Convert initializer expression if cast required */
        ConvertExprIfCastRequired(ast->initializer, *ast->GetTypeDenoter()->Get());
    }

    VISIT_DEFAULT(VarDecl);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    LabelAnonymousStructDecl(ast);

    PushStructDeclLevel();
    {
        VISIT_DEFAULT(StructDecl);
    }
    PopStructDeclLevel();

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
    currentFunctionDecl_ = ast;

    RenameReservedFunctionName(ast->ident, ast->renamedIdent);

    if (ast->flags(FunctionDecl::isEntryPoint))
    {
        isInsideEntryPoint_ = true;
        {
            VISIT_DEFAULT(FunctionDecl);
        }
        isInsideEntryPoint_ = false;
    }
    else
        VISIT_DEFAULT(FunctionDecl);

    RemoveSamplerStateVarDeclStmnts(ast->parameters);
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
    if (auto funcCall = ASTFactory::FindSingleFunctionCall(ast->expr.get()))
    {
        /* Is this a special intrinsic function call? */
        if (funcCall->intrinsic == Intrinsic::SinCos)
            ast->expr = ASTFactory::MakeSeparatedSinCosFunctionCalls(*funcCall);
    }

    VISIT_DEFAULT(ExprStmnt);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    if (ast->expr)
    {
        /* Convert return expression if cast required */
        if (currentFunctionDecl_)
            ConvertExprIfCastRequired(ast->expr, *currentFunctionDecl_->returnType->typeDenoter->Get());
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

    /* Convert right-hand-side expression if cast required */
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
        /* Convert assignment expression if cast required */
        ConvertExprIfCastRequired(ast->assignExpr, *ast->GetTypeDenoter()->Get());
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for conversion --- */

void GLSLConverter::PushStructDeclLevel()
{
    ++structDeclLevel_;
}

void GLSLConverter::PopStructDeclLevel()
{
    --structDeclLevel_;
}

bool GLSLConverter::IsInsideStructDecl() const
{
    return (structDeclLevel_ > 0);
}

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

bool GLSLConverter::MustResolveStruct(StructDecl* ast) const
{
    return MustResolveStructForTarget(shaderTarget_, ast);
}

bool GLSLConverter::MustRenameVarDecl(VarDecl* ast) const
{
    /* Variable must be renamed if it's not inside a structure declaration and its name is reserved */
    return
    (
        !IsInsideStructDecl() &&
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
    ast->renamedIdent = nameManglingPrefix_ + ident;
    ast->flags << VarDecl::wasRenamed;
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

bool GLSLConverter::HasVarDeclOfVarIdentSystemSemantic(VarIdent* varIdent) const
{
    /* Has variable identifier a symbol reference? */
    if (varIdent->symbolRef)
    {
        /* Is this symbol reference a variable declaration? */
        if (auto varDecl = varIdent->symbolRef->As<VarDecl>())
        {
            /* Is semantic a system semantic? */
            return varDecl->flags(VarDecl::isSystemValue);
        }
    }
    return false;
}

bool GLSLConverter::HasGlobalInOutVarDecl(VarIdent* varIdent) const
{
    /* Has variable identifier a symbol reference? */
    if (varIdent->symbolRef)
    {
        /* Is this symbol reference a variable declaration? */
        if (auto varDecl = varIdent->symbolRef->As<VarDecl>())
        {
            if (IsGlobalInoutVarDecl(varDecl, program_->entryPointRef->inputSemantics.varDeclRefs))
                return true;
            if (IsGlobalInoutVarDecl(varDecl, program_->entryPointRef->outputSemantics.varDeclRefs))
                return true;
        }
    }
    return false;
}

bool GLSLConverter::IsGlobalInoutVarDecl(VarDecl* varDecl, const std::vector<VarDecl*>& varDeclRefs) const
{
    return (std::find(varDeclRefs.begin(), varDeclRefs.end(), varDecl) != varDeclRefs.end());
}

void GLSLConverter::MakeVarIdentWithSystemSemanticLocal(VarIdent* ast)
{
    auto root = ast;

    while (ast)
    {
        /* Has current variable declaration a system semantic? */
        if (HasVarDeclOfVarIdentSystemSemantic(ast) || HasGlobalInOutVarDecl(ast))
        {
            /*
            Remove all leading AST nodes until this one, to convert this
            variable identifer to an identifier for a local variable
            */
            while ( root && !( HasVarDeclOfVarIdentSystemSemantic(root) || HasGlobalInOutVarDecl(root) ) )
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
    if (isInsideEntryPoint_)
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

bool GLSLConverter::RenameReservedFunctionName(const std::string& ident, std::string& renamedIdent)
{
    const auto& reservedNames = ReservedGLSLNames();

    /* Perform name mangling on output identifier if the input identifier is a reserved name */
    auto it = reservedNames.find(ident);
    if (it != reservedNames.end())
    {
        renamedIdent = nameManglingPrefix_ + ident;
        return true;
    }

    return false;
}

/* ----- Conversion ----- */

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
            ConvertIntrinsicCallSample(ast);
            break;
        case Intrinsic::Texture_SampleLevel_3:
        case Intrinsic::Texture_SampleLevel_4:
        case Intrinsic::Texture_SampleLevel_5:
            ConvertIntrinsicCallSampleLevel(ast);
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

void GLSLConverter::ConvertIntrinsicCallSample(FunctionCall* ast)
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

void GLSLConverter::ConvertIntrinsicCallSampleLevel(FunctionCall* ast)
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

//TODO: this is incomplete
#if 0
void GLSLConverter::ConvertVectorSubscriptExpr(ExprPtr& expr)
{
    auto typeDenoter = expr->GetTypeDenoter()->Get();
    if (typeDenoter->IsVector())
    {
        if (auto suffixExpr = expr->As<SuffixExpr>())
        {
            //TODO...
        }
        else if (auto varAccessExpr = expr->As<VarAccessExpr>())
        {
            if (varAccessExpr->varIdent->next)
            {
                //TODO...
            }
        }
    }
}
#endif

//TODO: this is incomplete
#if 0
void GLSLConverter::ConvertExprIfConstructorRequired(ExprPtr& expr)
{
    if (auto initExpr = expr->As<InitializerExpr>())
        expr = ASTFactory::ConvertInitializerExprToTypeConstructor(initExpr);
}
#endif

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
