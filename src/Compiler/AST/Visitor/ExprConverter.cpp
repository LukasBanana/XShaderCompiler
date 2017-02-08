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
    auto targetDim = VectorTypeDim(targetType);
    auto sourceDim = VectorTypeDim(sourceType);

    if ( ( targetDim != sourceDim && matchTypeSize ) ||
         (  IsUIntType      (targetType) &&  IsIntType       (sourceType) ) ||
         (  IsIntType       (targetType) &&  IsUIntType      (sourceType) ) ||
         (  IsRealType      (targetType) &&  IsIntegralType  (sourceType) ) ||
         (  IsIntegralType  (targetType) &&  IsRealType      (sourceType) ) ||
         ( !IsDoubleRealType(targetType) &&  IsDoubleRealType(sourceType) ) ||
         (  IsDoubleRealType(targetType) && !IsDoubleRealType(sourceType) ) )
    {
        if (targetDim != sourceDim && !matchTypeSize)
        {
            /* Return target base type with source dimension as required cast type */
            return MakeUnique<DataType>(VectorDataType(BaseDataType(targetType), VectorTypeDim(sourceType)));
        }
        else
        {
            /* Return target type as required cast type */
            return MakeUnique<DataType>(targetType);
        }
    }

    /* No type cast required */
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
            /* Store shared pointer */
            auto suffixIdent = *suffixIdentRef;

            /* Convert vector subscript to cast expression */
            auto vectorTypeDen = suffixIdent->GetTypeDenoterFromSubscript(*typeDen);

            /* Now drop suffix (shared pointer remain if 'GetTypeDenoterFromSubscript' throws and local 'suffixIdent' is the only reference!) */
            suffixIdentRef->reset();

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
                /* Store shared pointer */
                auto suffixIdent = varIdent->next;

                /* Convert vector subscript to cast expression */
                auto vectorTypeDen = suffixIdent->GetTypeDenoterFromSubscript(*typeDen);

                /* Now drop suffix (shared pointer remain if 'GetTypeDenoterFromSubscript' throws and local 'suffixIdent' is the only reference!) */
                varIdent->next.reset();

                /* Convert to cast expression */
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

void ExprConverter::IfFlaggedConvertExprVectorSubscript(ExprPtr& expr)
{
    if (conversionFlags_(ConvertVectorSubscripts))
        ConvertExprVectorSubscript(expr);
}

void ExprConverter::IfFlaggedConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize)
{
    if (conversionFlags_(ConvertImplicitCasts))
        ConvertExprIfCastRequired(expr, targetTypeDen, matchTypeSize);
}

void ExprConverter::IfFlaggedConvertExprIntoBracket(ExprPtr& expr)
{
    if (conversionFlags_(WrapUnaryExpr))
        ConvertExprIntoBracket(expr);
}

TypeDenoterPtr ExprConverter::FindCommonTypeDenoter(const TypeDenoterPtr& lhsTypeDen, const TypeDenoterPtr& rhsTypeDen)
{
    /* Scalar and Scalar */
    if (lhsTypeDen->IsScalar() && rhsTypeDen->IsScalar())
        return FindCommonTypeDenoterScalarAndScalar(lhsTypeDen->As<BaseTypeDenoter>(), rhsTypeDen->As<BaseTypeDenoter>());

    /* Scalar and Vector */
    if (lhsTypeDen->IsScalar() && rhsTypeDen->IsVector())
        return FindCommonTypeDenoterScalarAndVector(lhsTypeDen->As<BaseTypeDenoter>(), rhsTypeDen->As<BaseTypeDenoter>());

    /* Vector and Scalar */
    if (lhsTypeDen->IsVector() && rhsTypeDen->IsScalar())
        return FindCommonTypeDenoterScalarAndVector(rhsTypeDen->As<BaseTypeDenoter>(), lhsTypeDen->As<BaseTypeDenoter>());

    /* Vector and Vector */
    if (lhsTypeDen->IsVector() && rhsTypeDen->IsVector())
        return FindCommonTypeDenoterVectorAndVector(lhsTypeDen->As<BaseTypeDenoter>(), rhsTypeDen->As<BaseTypeDenoter>());

    /* Default type */
    return FindCommonTypeDenoterAnyAndAny(lhsTypeDen.get(), rhsTypeDen.get());
}

static DataType HighestOrderDataType(DataType lhs, DataType rhs, DataType highestType = DataType::Float) //Double//Float
{
    /*
    Return data type with highest order of both types: max{ lhs, rhs },
    where order is the integral enum value (bool < int < uint < float ...)
    */
    auto highestOrder = std::max({ static_cast<int>(lhs), static_cast<int>(rhs) });
    auto clampedOrder = std::min({ highestOrder, static_cast<int>(highestType) });
    return static_cast<DataType>(clampedOrder);
}

TypeDenoterPtr ExprConverter::FindCommonTypeDenoterScalarAndScalar(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen)
{
    auto commonType = HighestOrderDataType(lhsTypeDen->dataType, rhsTypeDen->dataType);
    return std::make_shared<BaseTypeDenoter>(commonType);
}

TypeDenoterPtr ExprConverter::FindCommonTypeDenoterScalarAndVector(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen)
{
    auto commonType = HighestOrderDataType(lhsTypeDen->dataType, BaseDataType(rhsTypeDen->dataType));
    auto rhsDim = VectorTypeDim(rhsTypeDen->dataType);
    return std::make_shared<BaseTypeDenoter>(VectorDataType(commonType, rhsDim));
}

TypeDenoterPtr ExprConverter::FindCommonTypeDenoterVectorAndVector(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen)
{
    auto commonType = HighestOrderDataType(BaseDataType(lhsTypeDen->dataType), BaseDataType(rhsTypeDen->dataType));
    auto lhsDim = VectorTypeDim(lhsTypeDen->dataType);
    auto rhsDim = VectorTypeDim(rhsTypeDen->dataType);
    auto highestDim = std::max(lhsDim, rhsDim);
    return std::make_shared<BaseTypeDenoter>(VectorDataType(commonType, highestDim));
}

TypeDenoterPtr ExprConverter::FindCommonTypeDenoterAnyAndAny(TypeDenoter* lhsTypeDen, TypeDenoter* rhsTypeDen)
{
    /* Always use type of left hand side */
    return lhsTypeDen->Get();
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ExprConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    VISIT_DEFAULT(FunctionCall);

    for (auto& funcArg : ast->arguments)
        IfFlaggedConvertExprVectorSubscript(funcArg);

    ast->ForEachArgumentWithParameter(
        [this](ExprPtr& funcArg, VarDeclPtr& funcParam)
        {
            auto paramTypeDen = funcParam->GetTypeDenoter()->Get();
            IfFlaggedConvertExprIfCastRequired(funcArg, *paramTypeDen);
        }
    );
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    VISIT_DEFAULT(VarDecl);
    if (ast->initializer)
    {
        IfFlaggedConvertExprVectorSubscript(ast->initializer);
        IfFlaggedConvertExprIfCastRequired(ast->initializer, *ast->GetTypeDenoter()->Get());
    }
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
    VISIT_DEFAULT(ForLoopStmnt);
    IfFlaggedConvertExprVectorSubscript(ast->condition);
    IfFlaggedConvertExprVectorSubscript(ast->iteration);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    VISIT_DEFAULT(WhileLoopStmnt);
    IfFlaggedConvertExprVectorSubscript(ast->condition);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    VISIT_DEFAULT(DoWhileLoopStmnt);
    IfFlaggedConvertExprVectorSubscript(ast->condition);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    VISIT_DEFAULT(IfStmnt);
    IfFlaggedConvertExprVectorSubscript(ast->condition);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    VISIT_DEFAULT(ExprStmnt);
    IfFlaggedConvertExprVectorSubscript(ast->expr);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    VISIT_DEFAULT(ReturnStmnt);
    if (ast->expr)
    {
        /* Convert return expression */
        IfFlaggedConvertExprVectorSubscript(ast->expr);
        if (auto funcDecl = ActiveFunctionDecl())
            IfFlaggedConvertExprIfCastRequired(ast->expr, *(funcDecl->returnType->GetTypeDenoter()->Get()));
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    VISIT_DEFAULT(TernaryExpr);
    IfFlaggedConvertExprVectorSubscript(ast->condExpr);
    IfFlaggedConvertExprVectorSubscript(ast->thenExpr);
    IfFlaggedConvertExprVectorSubscript(ast->elseExpr);
}

// Convert right-hand-side expression (if cast required)
IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    VISIT_DEFAULT(BinaryExpr);
    
    IfFlaggedConvertExprVectorSubscript(ast->lhsExpr);
    IfFlaggedConvertExprVectorSubscript(ast->rhsExpr);

    bool matchTypeSize = (ast->op != BinaryOp::Mul && ast->op != BinaryOp::Div);

    auto commonTypeDen = FindCommonTypeDenoter(ast->lhsExpr->GetTypeDenoter()->Get(), ast->rhsExpr->GetTypeDenoter()->Get());
    IfFlaggedConvertExprIfCastRequired(ast->lhsExpr, *commonTypeDen, matchTypeSize);
    IfFlaggedConvertExprIfCastRequired(ast->rhsExpr, *commonTypeDen, matchTypeSize);

    ast->ResetTypeDenoter();
}

// Wrap unary expression if the next sub expression is again an unary expression
IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    VISIT_DEFAULT(UnaryExpr);
    IfFlaggedConvertExprVectorSubscript(ast->expr);
    if (ast->expr->Type() == AST::Types::UnaryExpr)
        IfFlaggedConvertExprIntoBracket(ast->expr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    VISIT_DEFAULT(CastExpr);
    IfFlaggedConvertExprVectorSubscript(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    VISIT_DEFAULT(VarAccessExpr);
    if (ast->assignExpr)
    {
        IfFlaggedConvertExprVectorSubscript(ast->assignExpr);
        IfFlaggedConvertExprIfCastRequired(ast->assignExpr, *ast->GetTypeDenoter()->Get());
    }
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
