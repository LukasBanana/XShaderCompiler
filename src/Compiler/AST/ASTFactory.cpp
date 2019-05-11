/*
 * ASTFactory.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTFactory.h"
#include "Helper.h"
#include "Exception.h"
#include "Variant.h"


namespace Xsc
{

namespace ASTFactory
{


// Makes a new AST node with 'SourcePosition::ignore' as source position.
template <typename T, typename... Args>
std::shared_ptr<T> MakeAST(Args&&... args)
{
    return std::make_shared<T>(SourcePosition::ignore, std::forward<Args>(args)...);
}

// Makes a new AST node and takes the source origin from the first parameter.
template <typename T, typename Origin, typename... Args>
std::shared_ptr<T> MakeASTWithOrigin(const Origin& origin, Args&&... args)
{
    return std::make_shared<T>(origin->area, std::forward<Args>(args)...);
}

/* ----- Make functions ----- */

CallExprPtr MakeIntrinsicCallExpr(
    const Intrinsic intrinsic, const std::string& ident, const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments)
{
    auto ast = MakeAST<CallExpr>();
    {
        ast->ident          = ident;
        ast->typeDenoter    = typeDenoter;
        ast->arguments      = arguments;
        ast->intrinsic      = intrinsic;
    }
    return ast;
}

CallExprPtr MakeTextureSamplerBindingCallExpr(const ExprPtr& textureObjectExpr, const ExprPtr& samplerObjectExpr)
{
    auto ast = MakeAST<CallExpr>();
    {
        const auto& typeDen = textureObjectExpr->GetTypeDenoter()->GetAliased();
        if (auto bufferTypeDen = typeDen.As<BufferTypeDenoter>())
        {
            ast->typeDenoter    = std::make_shared<SamplerTypeDenoter>(TextureTypeToSamplerType(bufferTypeDen->bufferType));
            ast->arguments      = { textureObjectExpr, samplerObjectExpr };
        }
    }
    return ast;
}

CallExprPtr MakeTypeCtorCallExpr(const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments)
{
    auto ast = MakeAST<CallExpr>();
    {
        ast->typeDenoter    = typeDenoter;
        ast->arguments      = arguments;
    }
    return ast;
}

CallExprPtr MakeWrapperCallExpr(const std::string& funcIdent, const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments)
{
    auto ast = MakeAST<CallExpr>();
    {
        ast->ident          = funcIdent;
        ast->typeDenoter    = typeDenoter;
        ast->arguments      = arguments;
        ast->flags << CallExpr::isWrapperCall;
    }
    return ast;
}

InitializerExprPtr MakeInitializerExpr(const std::vector<ExprPtr>& exprs)
{
    auto ast = MakeAST<InitializerExpr>();
    {
        ast->exprs = exprs;
    }
    return ast;
}

CastExprPtr MakeCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr)
{
    auto ast = MakeAST<CastExpr>();
    {
        ast->typeSpecifier          = MakeTypeSpecifier(typeDenoter);
        ast->typeSpecifier->area    = valueExpr->area;
        ast->expr                   = valueExpr;
    }
    return ast;
}

CastExprPtr MakeLiteralCastExpr(const TypeDenoterPtr& typeDenoter, const DataType literalType, const std::string& literalValue)
{
    return MakeCastExpr(typeDenoter, MakeLiteralExpr(literalType, literalValue));
}

BinaryExprPtr MakeBinaryExpr(const ExprPtr& lhsExpr, const BinaryOp op, const ExprPtr& rhsExpr)
{
    auto ast = MakeAST<BinaryExpr>();
    {
        ast->lhsExpr    = lhsExpr;
        ast->op         = op;
        ast->rhsExpr    = rhsExpr;
    }
    return ast;
}

LiteralExprPtr MakeLiteralExpr(const DataType literalType, const std::string& literalValue)
{
    auto ast = MakeAST<LiteralExpr>();
    {
        ast->dataType   = literalType;
        ast->value      = literalValue;
    }
    return ast;
}

LiteralExprPtr MakeLiteralExprOrNull(const Variant& literalValue)
{
    switch (literalValue.Type())
    {
        case Variant::Types::Bool:
            return MakeLiteralExpr(DataType::Bool, std::to_string(literalValue.Bool()));
        case Variant::Types::Int:
            return MakeLiteralExpr(DataType::Int, std::to_string(literalValue.Int()));
        case Variant::Types::Real:
            return MakeLiteralExpr(DataType::Float, std::to_string(literalValue.Real()));
        default:
            return nullptr;
    }
}

AliasDeclStmtPtr MakeBaseTypeAlias(const DataType dataType, const std::string& ident)
{
    auto ast = MakeAST<AliasDeclStmt>();
    {
        auto aliasDecl = MakeAST<AliasDecl>();
        {
            aliasDecl->ident        = ident;
            aliasDecl->typeDenoter  = std::make_shared<BaseTypeDenoter>(dataType);
            aliasDecl->declStmtRef = ast.get();
        }
        ast->aliasDecls.push_back(aliasDecl);
    }
    return ast;
}

TypeSpecifierPtr MakeTypeSpecifier(const StructDeclPtr& structDecl)
{
    auto ast = MakeAST<TypeSpecifier>();
    {
        ast->structDecl     = structDecl;
        ast->typeDenoter    = std::make_shared<StructTypeDenoter>(structDecl.get());
    }
    ast->area = ast->structDecl->area;
    return ast;
}

TypeSpecifierPtr MakeTypeSpecifier(const TypeDenoterPtr& typeDenoter)
{
    auto ast = MakeAST<TypeSpecifier>();
    {
        ast->typeDenoter = typeDenoter;
    }
    return ast;
}

TypeSpecifierPtr MakeTypeSpecifier(const DataType dataType)
{
    return MakeTypeSpecifier(std::make_shared<BaseTypeDenoter>(dataType));
}

VarDeclStmtPtr MakeVarDeclStmt(const TypeSpecifierPtr& typeSpecifier, const std::string& ident, const ExprPtr& initializer)
{
    auto ast = MakeAST<VarDeclStmt>();
    {
        ast->typeSpecifier = typeSpecifier;

        auto varDecl = MakeAST<VarDecl>();
        {
            varDecl->ident          = ident;
            varDecl->initializer    = initializer;
            varDecl->declStmtRef   = ast.get();
        }
        ast->varDecls.push_back(varDecl);
    }
    return ast;
}

VarDeclStmtPtr MakeVarDeclStmt(const DataType dataType, const std::string& ident, const ExprPtr& initializer)
{
    return MakeVarDeclStmt(MakeTypeSpecifier(dataType), ident, initializer);
}

VarDeclStmtPtr MakeVarDeclStmtSplit(const VarDeclStmtPtr& varDeclStmt, std::size_t idx)
{
    if (varDeclStmt->varDecls.size() >= 2 && idx < varDeclStmt->varDecls.size())
    {
        /* Move VarDecl out of statement */
        auto varDecl = varDeclStmt->varDecls[idx];
        varDeclStmt->varDecls.erase(varDeclStmt->varDecls.begin() + idx);

        /* Create new statement */
        auto ast = MakeAST<VarDeclStmt>();
        {
            ast->flags          = varDeclStmt->flags;
            ast->typeSpecifier  = varDeclStmt->typeSpecifier;
            ast->varDecls.push_back(varDecl);
        }
        return ast;
    }
    return varDeclStmt;
}

IdentExprPtr MakeIdentExpr(const ExprPtr& prefixExpr, const std::string& ident, Decl* symbolRef)
{
    auto ast = MakeAST<IdentExpr>();
    {
        ast->prefixExpr = prefixExpr;
        ast->ident      = ident;
        ast->symbolRef  = symbolRef;
    }
    return ast;
}

IdentExprPtr MakeIdentExpr(const std::string& ident, Decl* symbolRef)
{
    return MakeIdentExpr(nullptr, ident, symbolRef);
}

IdentExprPtr MakeIdentExpr(Decl* symbolRef)
{
    return MakeIdentExpr(symbolRef->ident.Original(), symbolRef);
}

SubscriptExprPtr MakeSubscriptExpr(const ExprPtr& prefixExpr, std::vector<ExprPtr>&& arrayIndices)
{
    auto ast = MakeAST<SubscriptExpr>();
    {
        ast->prefixExpr     = prefixExpr;
        ast->arrayIndices   = std::move(arrayIndices);
    }
    return ast;
}

SubscriptExprPtr MakeSubscriptExpr(const ExprPtr& prefixExpr, const std::vector<int>& arrayIndices)
{
    return MakeSubscriptExpr(prefixExpr, MakeArrayIndices(arrayIndices));
}

SubscriptExprPtr MakeSubscriptExpr(
    const ExprPtr& prefixExpr,
    const std::vector<ExprPtr>::const_iterator& arrayIndicesBegin,
    const std::vector<ExprPtr>::const_iterator& arrayIndicesEnd)
{
    auto ast = MakeAST<SubscriptExpr>();
    {
        ast->prefixExpr = prefixExpr;
        ast->arrayIndices.insert(
            ast->arrayIndices.end(),
            arrayIndicesBegin,
            arrayIndicesEnd
        );
    }
    return ast;
}

SubscriptExprPtr MakeSubscriptExprSplit(const SubscriptExprPtr& subscriptExpr, std::size_t splitArrayIndex)
{
    if (subscriptExpr != nullptr && splitArrayIndex > 0 && splitArrayIndex < subscriptExpr->NumIndices())
    {
        /* Make main array expression */
        auto ast = MakeSubscriptExpr(
            MakeSubscriptExpr(
                subscriptExpr->prefixExpr,
                subscriptExpr->arrayIndices.begin(),
                subscriptExpr->arrayIndices.begin() + splitArrayIndex
            ),
            subscriptExpr->arrayIndices.begin() + splitArrayIndex,
            subscriptExpr->arrayIndices.end()
        );

        ast->area = subscriptExpr->area;;

        return ast;
    }
    return subscriptExpr;
}

RegisterPtr MakeRegister(int slot, const RegisterType registerType)
{
    auto ast = MakeAST<Register>();
    {
        ast->registerType   = registerType;
        ast->slot           = slot;
    }
    return ast;
}

BracketExprPtr MakeBracketExpr(const ExprPtr& expr)
{
    auto ast = MakeASTWithOrigin<BracketExpr>(expr);
    {
        ast->expr = expr;
    }
    return ast;
}

/*
TODO:
This is currently being used to convert a scalar-to-struct cast expression
into a struct-constructor expression (e.g. "(S)0" -> "S(0, 0, 0)").
This is done by using a list-expression instead of an argument list for the constructor.
This should be changed, because a list-expression is not meant to be used as argument list!
-> see GLSLConverter::VisitCastExpr
*/
#if 1

static ExprPtr MakeConstructorListExprPrimarySingle(const ExprPtr& expr, const TypeDenoterPtr& typeDen)
{
    if (auto structTypeDen = typeDen->As<StructTypeDenoter>())
    {
        if (auto structDecl = structTypeDen->structDeclRef)
        {
            /* Get the type denoter of all structure members */
            std::vector<TypeDenoterPtr> memberTypeDens;
            structDecl->CollectMemberTypeDenoters(memberTypeDens, false);

            /* Generate list expression with N copies of the literal (where N is the number of struct members) */
            return MakeCastExpr(typeDen, MakeConstructorListExpr(expr, memberTypeDens));
        }
    }
    else if (auto baseTypeDen = typeDen->As<BaseTypeDenoter>())
    {
        if (!baseTypeDen->IsScalar())
        {
            /* Make a cast expression for this vector or matrix type */
            return MakeCastExpr(typeDen, expr);
        }
    }
    return expr;
}

static ExprPtr MakeConstructorListExprPrimary(
    const ExprPtr& expr,
    std::vector<TypeDenoterPtr>::const_iterator typeDensBegin,
    std::vector<TypeDenoterPtr>::const_iterator typeDensEnd)
{
    if (typeDensBegin + 1 != typeDensEnd)
    {
        auto ast = MakeAST<SequenceExpr>();
        {
            ast->Append(MakeConstructorListExprPrimarySingle(expr, (*typeDensBegin)->GetSub()));
            ast->Append(MakeConstructorListExprPrimary(expr, typeDensBegin + 1, typeDensEnd));
        }
        return ast;
    }
    else
        return MakeConstructorListExprPrimarySingle(expr, (*typeDensBegin)->GetSub());
}

ExprPtr MakeConstructorListExpr(const ExprPtr& expr, const std::vector<TypeDenoterPtr>& listTypeDens)
{
    if (listTypeDens.empty())
        return expr;
    else
        return MakeConstructorListExprPrimary(expr, listTypeDens.begin(), listTypeDens.end());
}

#endif

ExprStmtPtr MakeAssignStmt(const ExprPtr& lvalueExpr, const ExprPtr& rvalueExpr, const AssignOp op)
{
    auto ast = MakeAST<ExprStmt>();
    {
        auto assignExpr = MakeAST<AssignExpr>();
        {
            assignExpr->lvalueExpr  = lvalueExpr;
            assignExpr->op          = op;
            assignExpr->rvalueExpr  = rvalueExpr;
        }
        ast->expr = assignExpr;
    }
    return ast;
}

ExprStmtPtr MakeArrayAssignStmt(VarDecl* varDecl, const std::vector<int>& arrayIndices, const ExprPtr& assignExpr)
{
    return MakeAssignStmt(MakeSubscriptExpr(MakeIdentExpr(varDecl), arrayIndices), assignExpr);
}

ArrayDimensionPtr MakeArrayDimension(int arraySize)
{
    auto ast = MakeAST<ArrayDimension>();
    {
        if (arraySize > 0)
        {
            ast->expr = MakeLiteralExpr(DataType::Int, std::to_string(arraySize));
            ast->size = arraySize;
        }
        else
        {
            ast->expr = MakeAST<NullExpr>();
            ast->size = 0;
        }
    }
    return ast;
}

ScopeStmtPtr MakeScopeStmt(const StmtPtr& stmt)
{
    auto ast = MakeASTWithOrigin<ScopeStmt>(stmt);
    {
        ast->codeBlock = MakeASTWithOrigin<CodeBlock>(stmt);
        ast->codeBlock->stmts.push_back(stmt);
    }
    return ast;
}

BasicDeclStmtPtr MakeStructDeclStmt(const StructDeclPtr& structDecl)
{
    auto ast = MakeAST<BasicDeclStmt>();
    {
        ast->declObject = structDecl;
        structDecl->declStmtRef = ast.get();
    }
    return ast;
}

UniformBufferDeclPtr MakeUniformBufferDecl(const std::string& ident, int bindingSlot, const UniformBufferType bufferType)
{
    auto ast = MakeAST<UniformBufferDecl>();
    {
        ast->ident      = ident;
        ast->bufferType = bufferType;
        ast->slotRegisters.push_back(MakeRegister(bindingSlot, RegisterType::ConstantBuffer));
    }
    return ast;
}

/* ----- Make list functions ----- */

std::vector<ExprPtr> MakeArrayIndices(const std::vector<int>& arrayIndices)
{
    std::vector<ExprPtr> exprs;

    for (auto index : arrayIndices)
        exprs.push_back(MakeLiteralExpr(DataType::Int, std::to_string(index)));

    return exprs;
}

std::vector<ArrayDimensionPtr> MakeArrayDimensionList(const std::vector<int>& arraySizes)
{
    std::vector<ArrayDimensionPtr> arrayDims;

    for (auto dim : arraySizes)
        arrayDims.push_back(MakeArrayDimension(dim));

    return arrayDims;
}

/* ----- Convert functions ----- */

ExprPtr ConvertExprBaseType(const DataType dataType, const ExprPtr& subExpr)
{
    if (subExpr->Type() == AST::Types::LiteralExpr && IsScalarType(dataType))
    {
        /* Convert data type into literal expression */
        auto ast = std::static_pointer_cast<LiteralExpr>(subExpr);
        {
            ast->ConvertDataType(dataType);
        }
        return ast;
    }
    else
    {
        /* Make new cast expression */
        auto ast = MakeASTWithOrigin<CastExpr>(subExpr);
        {
            ast->typeSpecifier          = MakeTypeSpecifier(std::make_shared<BaseTypeDenoter>(dataType));
            ast->typeSpecifier->area    = subExpr->area;
            ast->expr                   = subExpr;
        }
        return ast;
    }
}

ArrayDimensionPtr ConvertExprToArrayDimension(const ExprPtr& expr)
{
    auto ast = MakeAST<ArrayDimension>();
    {
        if (expr)
        {
            ast->area = expr->area;
            ast->expr = expr;
        }
    }
    return ast;
}

std::vector<ArrayDimensionPtr> ConvertExprListToArrayDimensionList(const std::vector<ExprPtr>& exprs)
{
    std::vector<ArrayDimensionPtr> arrayDims;

    for (const auto& expr : exprs)
        arrayDims.push_back(ConvertExprToArrayDimension(expr));

    return arrayDims;
}


} // /namespace ASTFactory

} // /namespace Xsc



// ================================================================================
