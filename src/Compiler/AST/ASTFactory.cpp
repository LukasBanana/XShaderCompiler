/*
 * ASTFactory.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTFactory.h"
#include "Helper.h"
#include "Exception.h"


namespace Xsc
{

namespace ASTFactory
{


template <typename T, typename... Args>
std::shared_ptr<T> MakeAST(Args&&... args)
{
    return MakeShared<T>(SourcePosition::ignore, std::forward<Args>(args)...);
}

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

CastExprPtr MakeCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr)
{
    auto ast = MakeAST<CastExpr>();
    {
        ast->typeExpr               = MakeAST<TypeNameExpr>();
        ast->typeExpr->typeDenoter  = typeDenoter;
        ast->expr                   = valueExpr;
    }
    return ast;
}

CastExprPtr MakeLiteralCastExpr(const TypeDenoterPtr& typeDenoter, const DataType literalType, const std::string& literalValue)
{
    auto literalExpr = MakeAST<LiteralExpr>();
    {
        literalExpr->dataType   = literalType;
        literalExpr->value      = literalValue;
    }
    return MakeCastExpr(typeDenoter, literalExpr);
}

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
            ast->typeExpr               = MakeAST<TypeNameExpr>();
            ast->typeExpr->typeDenoter  = MakeShared<BaseTypeDenoter>(dataType);
            ast->expr                   = subExpr;
        }
        return ast;
    }
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

VarTypePtr MakeVarType(const StructDeclPtr& structDecl)
{
    auto ast = MakeAST<VarType>();
    {
        ast->structDecl     = structDecl;
        ast->typeDenoter    = std::make_shared<StructTypeDenoter>(structDecl.get());
    }
    return ast;
}

VarTypePtr MakeVarType(const TypeDenoterPtr& typeDenoter)
{
    auto ast = MakeAST<VarType>();
    {
        ast->typeDenoter = typeDenoter;
    }
    return ast;
}

VarTypePtr MakeVarType(const DataType dataType)
{
    return MakeVarType(MakeShared<BaseTypeDenoter>(dataType));
}

VarDeclStmntPtr MakeVarDeclStmnt(const DataType dataType, const std::string& ident)
{
    auto ast = MakeAST<VarDeclStmnt>();
    {
        ast->varType = MakeVarType(dataType);

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


} // /namespace ASTFactory

} // /namespace Xsc



// ================================================================================
