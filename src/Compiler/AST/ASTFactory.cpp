/*
 * ASTFactory.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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


template <typename T, typename... Args>
std::shared_ptr<T> MakeAST(Args&&... args)
{
    return MakeShared<T>(SourcePosition::ignore, std::forward<Args>(args)...);
}

template <typename T, typename Origin, typename... Args>
std::shared_ptr<T> MakeASTWithOrigin(Origin origin, Args&&... args)
{
    return MakeShared<T>(origin->area, std::forward<Args>(args)...);
}

/* ----- Make functions ----- */

FunctionCallExprPtr MakeIntrinsicCallExpr(
    const Intrinsic intrinsic, const std::string& ident, const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments)
{
    auto ast = MakeAST<FunctionCallExpr>();
    {
        auto funcCall = MakeAST<FunctionCall>();
        {
            funcCall->varIdent          = MakeAST<VarIdent>();
            funcCall->varIdent->ident   = ident;
            funcCall->typeDenoter       = typeDenoter;
            funcCall->arguments         = arguments;
            funcCall->intrinsic         = intrinsic;
        }
        ast->call = funcCall;
    }
    return ast;
}

static SamplerType TextureTypeToSamplerType(const BufferType t)
{
    switch (t)
    {
        case BufferType::Texture1D:
        case BufferType::Texture1DArray:
            return SamplerType::Sampler1D;
        case BufferType::Texture2D:
        case BufferType::Texture2DArray:
            return SamplerType::Sampler2D;
        case BufferType::Texture3D:
            return SamplerType::Sampler3D;
        case BufferType::TextureCube:
        case BufferType::TextureCubeArray:
            return SamplerType::SamplerCube;
        default:
            return SamplerType::Undefined;
    }
}

FunctionCallExprPtr MakeTextureSamplerBindingCallExpr(const ExprPtr& textureObjectExpr, const ExprPtr& samplerObjectExpr)
{
    auto ast = MakeAST<FunctionCallExpr>();
    {
        auto funcCall = MakeAST<FunctionCall>();
        {
            auto typeDen = textureObjectExpr->GetTypeDenoter()->Get();
            if (auto bufferTypeDen = typeDen->As<BufferTypeDenoter>())
            {
                funcCall->typeDenoter = std::make_shared<SamplerTypeDenoter>(TextureTypeToSamplerType(bufferTypeDen->bufferType));
                funcCall->arguments.push_back(textureObjectExpr);
                funcCall->arguments.push_back(samplerObjectExpr);
            }
        }
        ast->call = funcCall;
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

SuffixExprPtr MakeSuffixExpr(const ExprPtr& expr, const VarIdentPtr& varIdent)
{
    auto ast = MakeASTWithOrigin<SuffixExpr>(expr);
    {
        ast->expr       = expr;
        ast->varIdent   = varIdent;
    }
    return ast;
}

ExprPtr MakeCastOrSuffixCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr, const VarIdentPtr& suffixVarIdent)
{
    auto castExpr = ASTFactory::MakeCastExpr(typeDenoter, valueExpr);
    if (suffixVarIdent)
        return MakeSuffixExpr(castExpr, suffixVarIdent);
    else
        return castExpr;
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

LiteralExprPtr MakeLiteralExpr(const Variant& literalValue)
{
    switch (literalValue.Type())
    {
        case Variant::Types::Bool:
            return MakeLiteralExpr(DataType::Bool, std::to_string(literalValue.Bool()));
        case Variant::Types::Int:
            return MakeLiteralExpr(DataType::Int, std::to_string(literalValue.Int()));
        case Variant::Types::Real:
            return MakeLiteralExpr(DataType::Float, std::to_string(literalValue.Real()));
    }
    return MakeLiteralExpr(DataType::Int, "0");
}

AliasDeclStmntPtr MakeBaseTypeAlias(const DataType dataType, const std::string& ident)
{
    auto ast = MakeAST<AliasDeclStmnt>();
    {
        auto aliasDecl = MakeAST<AliasDecl>();
        {
            aliasDecl->ident        = ident;
            aliasDecl->typeDenoter  = MakeShared<BaseTypeDenoter>(dataType);
            aliasDecl->declStmntRef = ast.get();
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
    return MakeTypeSpecifier(MakeShared<BaseTypeDenoter>(dataType));
}

VarDeclStmntPtr MakeVarDeclStmnt(const TypeSpecifierPtr& typeSpecifier, const std::string& ident)
{
    auto ast = MakeAST<VarDeclStmnt>();
    {
        ast->typeSpecifier = typeSpecifier;

        auto varDecl = MakeAST<VarDecl>();
        {
            varDecl->ident          = ident;
            varDecl->declStmntRef   = ast.get();
        }
        ast->varDecls.push_back(varDecl);
    }
    return ast;
}

VarDeclStmntPtr MakeVarDeclStmnt(const DataType dataType, const std::string& ident)
{
    return MakeVarDeclStmnt(MakeTypeSpecifier(dataType), ident);
}

VarIdentPtr MakeVarIdent(const std::string& ident, AST* symbolRef)
{
    auto ast = MakeAST<VarIdent>();
    {
        ast->ident      = ident;
        ast->symbolRef  = symbolRef;
    }
    return ast;
}

VarIdentPtr MakeVarIdentFirst(const VarIdent& varIdent)
{
    auto ast = MakeAST<VarIdent>();
    {
        ast->ident          = varIdent.ident;
        ast->arrayIndices   = varIdent.arrayIndices;
        ast->symbolRef      = varIdent.symbolRef;
    }
    return ast;
}

VarIdentPtr MakeVarIdentWithoutLast(const VarIdent& varIdent)
{
    if (varIdent.next)
    {
        auto ast = MakeAST<VarIdent>();
        {
            ast->ident          = varIdent.ident;
            ast->arrayIndices   = varIdent.arrayIndices;
            ast->symbolRef      = varIdent.symbolRef;
            ast->next           = MakeVarIdentWithoutLast(*varIdent.next);
        }
        return ast;
    }
    else
        return nullptr;
}

VarIdentPtr MakeVarIdentPushFront(const std::string& firstIdent, AST* symbolRef, const VarIdentPtr& next)
{
    auto ast = MakeAST<VarIdent>();
    {
        ast->ident          = firstIdent;
        ast->next           = next;
        ast->symbolRef      = symbolRef;
    }
    return ast;
}

VarAccessExprPtr MakeVarAccessExpr(const VarIdentPtr& varIdent)
{
    auto ast = MakeAST<VarAccessExpr>();
    {
        ast->varIdent = varIdent;
    }
    return ast;
}

VarAccessExprPtr MakeVarAccessExpr(const std::string& ident, AST* symbolRef)
{
    return MakeVarAccessExpr(MakeVarIdent(ident, symbolRef));
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

static ExprPtr MakeConstructorListExprPrimarySingle(const LiteralExprPtr& literalExpr, const TypeDenoterPtr& typeDen)
{
    if (auto structTypeDen = typeDen->As<StructTypeDenoter>())
    {
        if (auto structDecl = structTypeDen->structDeclRef)
        {
            /* Get the type denoter of all structure members */
            std::vector<TypeDenoterPtr> memberTypeDens;
            structDecl->CollectMemberTypeDenoters(memberTypeDens);

            /* Generate list expression with N copies of the literal (where N is the number of struct members) */
            return MakeCastExpr(typeDen, MakeConstructorListExpr(literalExpr, memberTypeDens));
        }
    }
    else if (auto baseTypeDen = typeDen->As<BaseTypeDenoter>())
    {
        if (!baseTypeDen->IsScalar())
        {
            /* Make a cast expression for this vector or matrix type */
            return MakeCastExpr(typeDen, literalExpr);
        }
    }
    return literalExpr;
}

static ExprPtr MakeConstructorListExprPrimary(
    const LiteralExprPtr& literalExpr,
    std::vector<TypeDenoterPtr>::const_iterator typeDensBegin,
    std::vector<TypeDenoterPtr>::const_iterator typeDensEnd)
{
    if (typeDensBegin + 1 != typeDensEnd)
    {
        auto ast = MakeAST<ListExpr>();
        {
            ast->firstExpr  = MakeConstructorListExprPrimarySingle(literalExpr, (*typeDensBegin)->Get());
            ast->nextExpr   = MakeConstructorListExprPrimary(literalExpr, typeDensBegin + 1, typeDensEnd);
        }
        return ast;
    }
    else
        return MakeConstructorListExprPrimarySingle(literalExpr, (*typeDensBegin)->Get());
}

ExprPtr MakeConstructorListExpr(const LiteralExprPtr& literalExpr, const std::vector<TypeDenoterPtr>& listTypeDens)
{
    if (listTypeDens.empty())
        return nullptr;
    else
        return MakeConstructorListExprPrimary(literalExpr, listTypeDens.begin(), listTypeDens.end());
}

#endif

ExprStmntPtr MakeArrayAssignStmnt(VarDecl* varDecl, const std::vector<int>& arrayIndices, const ExprPtr& assignExpr)
{
    auto ast = MakeAST<ExprStmnt>();
    {
        auto expr = MakeAST<VarAccessExpr>();
        {
            expr->varIdent                  = MakeVarIdent(varDecl->ident, varDecl);
            expr->varIdent->arrayIndices    = MakeArrayIndices(arrayIndices);
            expr->assignOp                  = AssignOp::Set;
            expr->assignExpr                = assignExpr;
        }
        ast->expr = expr;
    }
    return ast;
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
        auto ast = MakeShared<CastExpr>(subExpr->area);
        {
            ast->typeSpecifier          = MakeTypeSpecifier(MakeShared<BaseTypeDenoter>(dataType));
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
