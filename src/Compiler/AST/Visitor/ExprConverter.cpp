/*
 * ExprConverter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ExprConverter.h"
#include "GLSLKeywords.h"
#include "AST.h"
#include "ASTFactory.h"
#include "Exception.h"
#include "Helper.h"


namespace Xsc
{


void ExprConverter::Convert(Program& program, const Flags& conversionFlags)
{
    /* Visit program AST */
    conversionFlags_ = conversionFlags;
    if (conversionFlags_ != 0)
        Visit(&program);
}

void ExprConverter::ConvertExprVectorSubscript(ExprPtr& expr)
{
    if (expr)
    {
        if (auto suffixExpr = expr->As<SuffixExpr>())
            ConvertExprVectorSubscriptSuffix(expr, suffixExpr);
        else
            ConvertExprVectorSubscriptVarIdent(expr, expr->FetchVarIdent());
    }
}

// Converts the expression to a cast expression if it is required for the specified target type.
void ExprConverter::ConvertExprIfCastRequired(ExprPtr& expr, const DataType targetType, bool matchTypeSize)
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

void ExprConverter::ConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize)
{
    if (auto dataType = MustCastExprToDataType(targetTypeDen, *expr->GetTypeDenoter()->Get(), matchTypeSize))
    {
        /* Convert to cast expression with target data type if required */
        expr = ASTFactory::ConvertExprBaseType(*dataType, expr);
    }
}

//TODO: this is incomplete
#if 0
void ExprConverter::ConvertExprIfConstructorRequired(ExprPtr& expr)
{
    if (auto initExpr = expr->As<InitializerExpr>())
        expr = ASTFactory::ConvertInitializerExprToTypeConstructor(initExpr);
}
#endif

void ExprConverter::ConvertExprIntoBracket(ExprPtr& expr)
{
    /* Insert bracket expression */
    auto bracketExpr = MakeShared<BracketExpr>(expr->area);
    {
        bracketExpr->expr = expr;
    }
    expr = bracketExpr;
}


/*
 * ======= Private: =======
 */

std::unique_ptr<DataType> ExprConverter::MustCastExprToDataType(const DataType targetType, const DataType sourceType, bool matchTypeSize)
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

std::unique_ptr<DataType> ExprConverter::MustCastExprToDataType(const TypeDenoter& targetTypeDen, const TypeDenoter& sourceTypeDen, bool matchTypeSize)
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

void ExprConverter::ConvertExprVectorSubscriptSuffix(ExprPtr& expr, SuffixExpr* suffixExpr)
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

void ExprConverter::ConvertExprVectorSubscriptVarIdent(ExprPtr& expr, VarIdent* varIdent)
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

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ExprConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Convert argument expressions */
    ast->ForEachArgumentWithParameter(
        [this](ExprPtr& funcArg, VarDeclPtr& funcParam)
        {
            auto paramTypeDen = funcParam->GetTypeDenoter()->Get();
            ConvertExprVectorSubscript(funcArg);
            ConvertExprIfCastRequired(funcArg, *paramTypeDen);
        }
    );

    VISIT_DEFAULT(FunctionCall);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    /* Must the initializer type denoter changed? */
    if (ast->initializer)
    {
        ConvertExprVectorSubscript(ast->initializer);
        ConvertExprIfCastRequired(ast->initializer, *ast->GetTypeDenoter()->Get());
    }

    VISIT_DEFAULT(VarDecl);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    PushFunctionDecl(ast);
    {
        VISIT_DEFAULT(FunctionDecl);
    }
    PopFunctionDecl();
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    ConvertExprVectorSubscript(ast->condition);
    ConvertExprVectorSubscript(ast->iteration);
    VISIT_DEFAULT(ForLoopStmnt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    ConvertExprVectorSubscript(ast->condition);
    VISIT_DEFAULT(WhileLoopStmnt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    ConvertExprVectorSubscript(ast->condition);
    VISIT_DEFAULT(DoWhileLoopStmnt);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    ConvertExprVectorSubscript(ast->condition);
    VISIT_DEFAULT(IfStmnt);
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

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    VISIT_DEFAULT(TernaryExpr);
    ConvertExprVectorSubscript(ast->condExpr);
    ConvertExprVectorSubscript(ast->thenExpr);
    ConvertExprVectorSubscript(ast->elseExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    VISIT_DEFAULT(BinaryExpr);

    /* Convert right-hand-side expression (if cast required) */
    ConvertExprVectorSubscript(ast->lhsExpr);
    ConvertExprVectorSubscript(ast->rhsExpr);
    ConvertExprIfCastRequired(ast->rhsExpr, *ast->lhsExpr->GetTypeDenoter()->Get());
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    /* Is the next sub expression again an unary expression? */
    if (ast->expr->Type() == AST::Types::UnaryExpr)
        ConvertExprIntoBracket(ast->expr);

    ConvertExprVectorSubscript(ast->expr);

    VISIT_DEFAULT(UnaryExpr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    ConvertExprVectorSubscript(ast->expr);
    VISIT_DEFAULT(CastExpr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    VISIT_DEFAULT(VarAccessExpr);

    if (ast->assignExpr)
    {
        ConvertExprVectorSubscript(ast->assignExpr);
        ConvertExprIfCastRequired(ast->assignExpr, *ast->GetTypeDenoter()->Get());
    }
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
