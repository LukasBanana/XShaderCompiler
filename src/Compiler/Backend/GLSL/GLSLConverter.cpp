/*
 * GLSLConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLConverter.h"
#include "GLSLHelper.h"
#include "AST.h"
#include "ASTFactory.h"
#include "Exception.h"
#include "Helper.h"


namespace Xsc
{


void GLSLConverter::Convert(
    Program& program, const ShaderTarget shaderTarget, const std::string& nameManglingPrefix)
{
    /* Store settings */
    shaderTarget_       = shaderTarget;
    nameManglingPrefix_ = nameManglingPrefix;

    /* Visit program AST */
    Visit(&program);
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
            RegisterReservedVarIdents(ast->entryPointRef->inputSemantics.varDeclRefs);
            break;
        case ShaderTarget::FragmentShader:
            RegisterReservedVarIdents(ast->entryPointRef->outputSemantics.varDeclRefs);
            break;
        case ShaderTarget::ComputeShader:
            RegisterReservedVarIdents(ast->entryPointRef->inputSemantics.varDeclRefs);
            RegisterReservedVarIdents(ast->entryPointRef->outputSemantics.varDeclRefs);
            break;
        default:
            break;
    }

    RegisterReservedVarIdents(ast->entryPointRef->inputSemantics.varDeclRefsSV);
    RegisterReservedVarIdents(ast->entryPointRef->outputSemantics.varDeclRefsSV);

    VISIT_DEFAULT(Program);

    //TODO: do not remove these statements, instead mark it as disabled code (otherwise symbol references to these statements are corrupted!)
    /* Remove all variables which are sampler state objects, since GLSL does not support sampler states */
    EraseAllIf(
        ast->globalStmnts,
        [&](const StmntPtr& stmnt)
        {
            if (stmnt->Type() == AST::Types::SamplerDeclStmnt)
                return true;
            if (auto varDeclStmnt = stmnt->As<VarDeclStmnt>())
                return VarTypeIsSampler(*varDeclStmnt->varType);
            return false;
        }
    );
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    RemoveDeadCode(ast->stmnts);

    VISIT_DEFAULT(CodeBlock);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    if (ast->intrinsic == Intrinsic::Saturate)
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

    /* Remove arguments which contain a sampler state object, since GLSL does not support sampler states */
    EraseAllIf(ast->arguments,
        [&](const ExprPtr& expr)
        {
            return ExprContainsSampler(*expr);
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

    //ConvertSemantic(ast->semantic);

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

    RemoveSamplerVarDeclStmnts(ast->members);

    if (ast->members.empty())
    {
        /* Add dummy member if the structure is empty (GLSL does not support empty structures) */
        auto dummyMember = ASTFactory::MakeVarDeclStmnt(DataType::Int, nameManglingPrefix_ + "dummy");
        ast->members.push_back(dummyMember);
    }
    else
    {
        PushStructDeclLevel();
        {
            VISIT_DEFAULT(StructDecl);
        }
        PopStructDeclLevel();
    }
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    currentFunctionDecl_ = ast;

    RemoveSamplerVarDeclStmnts(ast->parameters);

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
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    /* Remove all 'static' storage classes (reserved word in GLSL) */
    EraseAll(ast->storageClasses, StorageClass::Static);

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
    VISIT_DEFAULT(ReturnStmnt);

    /* Convert return expression if cast required */
    if (currentFunctionDecl_)
        ConvertExprIfCastRequired(ast->expr, *currentFunctionDecl_->returnType->typeDenoter->Get());
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

bool GLSLConverter::ExprContainsSampler(Expr& ast) const
{
    return ast.GetTypeDenoter()->Get()->IsSampler();
}

bool GLSLConverter::VarTypeIsSampler(VarType& ast) const
{
    return ast.typeDenoter->Get()->IsSampler();
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
                    return (varDecl != ast && varDecl->ident == ast->ident);
                }
            ) != reservedVarDecls_.end()
        )
    );
}

void GLSLConverter::RenameVarDecl(VarDecl* ast)
{
    /* Set new identifier for this variable */
    ast->ident = nameManglingPrefix_ + ast->ident;
    ast->flags << VarDecl::wasRenamed;
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
    /* Has variable identifier a system reference? */
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

void GLSLConverter::MakeVarIdentWithSystemSemanticLocal(VarIdent* ast)
{
    auto root = ast;

    while (ast)
    {
        /* Has current variable declaration a system semantic? */
        if (HasVarDeclOfVarIdentSystemSemantic(ast))
        {
            /*
            Remove all leading AST nodes until this one, to convert this
            variable identifer to an identifier for a local variable
            */
            while (root && !HasVarDeclOfVarIdentSystemSemantic(root))
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

std::unique_ptr<DataType> GLSLConverter::MustCastExprToDataType(TypeDenoter& targetTypeDen, TypeDenoter& sourceTypeDen)
{
    if (auto baseTargetTypeDen = targetTypeDen.As<BaseTypeDenoter>())
    {
        if (auto baseSourceTypeDen = sourceTypeDen.As<BaseTypeDenoter>())
        {
            auto dstType = baseTargetTypeDen->dataType;
            auto srcType = baseSourceTypeDen->dataType;

            /* Check for type mismatch */
            if ( ( IsUIntType    (dstType) && IsIntType     (srcType) ) ||
                 ( IsIntType     (dstType) && IsUIntType    (srcType) ) ||
                 ( IsRealType    (dstType) && IsIntegralType(srcType) ) ||
                 ( IsIntegralType(dstType) && IsRealType    (srcType) ) )
            {
                /* Cast to destination type */
                return MakeUnique<DataType>(dstType);
            }
        }
    }
    return nullptr;
}

void GLSLConverter::ConvertExprIfCastRequired(ExprPtr& expr, TypeDenoter& targetTypeDen)
{
    if (auto dataType = MustCastExprToDataType(targetTypeDen, *expr->GetTypeDenoter()->Get()))
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

void GLSLConverter::RemoveSamplerVarDeclStmnts(std::vector<VarDeclStmntPtr>& stmnts)
{
    /* Remove all variables which are sampler state objects, since GLSL does not support sampler states */
    EraseAllIf(
        stmnts,
        [&](const VarDeclStmntPtr& varDeclStmnt)
        {
            return VarTypeIsSampler(*varDeclStmnt->varType);
        }
    );
}

#if 0
void GLSLConverter::ConvertSemantic(IndexedSemantic& semantic)
{
    /* Convert fragment target to user defined semantic, because gl_FragData is only available for GLSL 1.10 */
    if (semantic == Semantic::Target)
        semantic = Semantic::UserDefined;
}
#endif


} // /namespace Xsc



// ================================================================================
