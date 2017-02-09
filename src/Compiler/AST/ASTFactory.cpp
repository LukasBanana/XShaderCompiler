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

/* ----- Find functions ----- */

Expr* FindSingleExpr(Expr* ast, const AST::Types searchedExprType)
{
    while (ast)
    {
        if (ast->Type() == searchedExprType)
        {
            /* Searched expression found */
            return ast;
        }
        else if (ast->Type() == AST::Types::BracketExpr)
        {
            /* Continue search in inner bracket expression */
            ast = static_cast<BracketExpr*>(ast)->expr.get();
        }
        else
        {
            /* Other expressions are not allowed for this search => stop searching */
            break;
        }
    }
    return nullptr;
}

FunctionCallPtr FindSingleFunctionCall(Expr* ast)
{
    if (auto expr = FindSingleExpr(ast, AST::Types::FunctionCallExpr))
        return static_cast<FunctionCallExpr*>(expr)->call;
    return nullptr;
}

VarIdentPtr FindSingleVarIdent(Expr* ast)
{
    if (auto expr = FindSingleExpr(ast, AST::Types::VarAccessExpr))
        return static_cast<VarAccessExpr*>(expr)->varIdent;
    return nullptr;
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

//TODO: remove this (unused)
#if 0
ListExprPtr MakeSeparatedSinCosFunctionCalls(FunctionCall& funcCall)
{
    if (funcCall.arguments.size() == 3)
    {
        /*
        Convert "sincos(x, s, c)" expression into "s = sin(x), c = cos(x)" (ListExpr)
        see https://msdn.microsoft.com/en-us/library/windows/desktop/bb509652(v=vs.85).aspx
        */
        auto listExpr = MakeShared<ListExpr>(funcCall.area);

        auto arg0 = funcCall.arguments[0];
        auto arg1 = funcCall.arguments[1];
        auto arg2 = funcCall.arguments[2];

        if (auto varOutSin = ASTFactory::FindSingleVarIdent(arg1.get()))
        {
            /* Make "s = sin(x)" expression */
            auto sinExpr = MakeShared<VarAccessExpr>(funcCall.area);
            sinExpr->varIdent   = varOutSin;
            sinExpr->assignOp   = AssignOp::Set;
            sinExpr->assignExpr = MakeIntrinsicCallExpr(Intrinsic::Sin, "sin", arg1->GetTypeDenoter(), { arg0 });
            listExpr->firstExpr = sinExpr;
        }
        else
            RuntimeErr("single variable identifier expected in intrinsic 'sincos'", arg1.get());

        if (auto varOutCos = ASTFactory::FindSingleVarIdent(arg2.get()))
        {
            /* Make "c = cos(x)" expression */
            auto cosExpr = MakeShared<VarAccessExpr>(funcCall.area);
            cosExpr->varIdent   = varOutCos;
            cosExpr->assignOp   = AssignOp::Set;
            cosExpr->assignExpr = MakeIntrinsicCallExpr(Intrinsic::Cos, "cos", arg2->GetTypeDenoter(), { arg0 });
            listExpr->nextExpr = cosExpr;
        }
        else
            RuntimeErr("single variable identifier expected in intrinsic 'sincos'", arg2.get());

        return listExpr;
    }
    else
        RuntimeErr("invalid number of arguments in intrinsic", &funcCall);
}
#endif

CastExprPtr MakeCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr)
{
    auto ast = MakeAST<CastExpr>();
    {
        ast->typeSpecifier              = MakeAST<TypeNameExpr>();
        ast->typeSpecifier->typeName    = MakeTypeName(typeDenoter);
        ast->expr                       = valueExpr;
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

TypeSpecifierPtr MakeTypeName(const StructDeclPtr& structDecl)
{
    auto ast = MakeAST<TypeSpecifier>();
    {
        ast->structDecl     = structDecl;
        ast->typeDenoter    = std::make_shared<StructTypeDenoter>(structDecl.get());
    }
    return ast;
}

TypeSpecifierPtr MakeTypeName(const TypeDenoterPtr& typeDenoter)
{
    auto ast = MakeAST<TypeSpecifier>();
    {
        ast->typeDenoter = typeDenoter;
    }
    return ast;
}

TypeSpecifierPtr MakeTypeName(const DataType dataType)
{
    return MakeTypeName(MakeShared<BaseTypeDenoter>(dataType));
}

VarDeclStmntPtr MakeVarDeclStmnt(const DataType dataType, const std::string& ident)
{
    auto ast = MakeAST<VarDeclStmnt>();
    {
        ast->typeSpecifier = MakeTypeName(dataType);

        auto varDecl = MakeAST<VarDecl>();
        {
            varDecl->ident          = ident;
            varDecl->declStmntRef   = ast.get();
        }
        ast->varDecls.push_back(varDecl);
    }
    return ast;
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

VarAccessExprPtr MakeVarAccessExpr(const std::string& ident, AST* symbolRef)
{
    auto ast = MakeAST<VarAccessExpr>();
    {
        ast->varIdent = MakeVarIdent(ident, symbolRef);
    }
    return ast;
}

static ExprPtr MakeConstructorListExprPrimarySingle(const LiteralExprPtr& literalExpr, const TypeDenoterPtr& typeDen)
{
    if (auto structTypeDen = typeDen->As<StructTypeDenoter>())
    {
        /* Get the type denoter of all structure members */
        auto structDecl = structTypeDen->structDeclRef;

        std::vector<TypeDenoterPtr> memberTypeDens;
        structDecl->CollectMemberTypeDenoters(memberTypeDens);

        /* Generate list expression with N copies literals (where N is the number of struct members) */
        return MakeCastExpr(typeDen, MakeConstructorListExpr(literalExpr, memberTypeDens));
    }
    else
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

        ast->firstExpr = MakeConstructorListExprPrimarySingle(literalExpr, (*typeDensBegin)->Get());
        ast->nextExpr = MakeConstructorListExprPrimary(literalExpr, typeDensBegin + 1, typeDensEnd);

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

std::vector<ExprPtr> MakeArrayIndices(const std::vector<int>& arrayIndices)
{
    std::vector<ExprPtr> exprs;

    for (auto index : arrayIndices)
        exprs.push_back(MakeLiteralExpr(DataType::Int, std::to_string(index)));

    return exprs;
}

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

    return ast;
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
        /* Convert data type of literal expression */
        auto literalExpr = std::static_pointer_cast<LiteralExpr>(subExpr);
        literalExpr->ConvertDataType(dataType);
        return literalExpr;
    }
    else
    {
        /* Make new cast expression */
        auto ast = MakeShared<CastExpr>(subExpr->area);
        {
            ast->typeSpecifier              = MakeAST<TypeNameExpr>();
            ast->typeSpecifier->typeName    = MakeTypeName(MakeShared<BaseTypeDenoter>(dataType));
            ast->expr                       = subExpr;
        }
        return ast;
    }
}

ArrayDimensionPtr ConvertExprToArrayDimension(const ExprPtr& expr)
{
    auto ast = MakeAST<ArrayDimension>();

    if (expr)
    {
        ast->area = expr->area;
        ast->expr = expr;
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

//TODO: this is incomplete
#if 0
FunctionCallExprPtr ConvertInitializerExprToTypeConstructor(InitializerExpr* expr)
{
    if (expr && !expr->exprs.empty())
    {
        auto ast = MakeASTWithOrigin<FunctionCallExpr>(expr);
        {
            ast->call = MakeASTWithOrigin<FunctionCall>(expr);

            ast->call->typeDenoter = expr->
        }
        return ast;
    }
}
#endif


} // /namespace ASTFactory

} // /namespace Xsc



// ================================================================================
