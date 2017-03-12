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

static Intrinsic CompareOpToIntrinsic(const BinaryOp op)
{
    switch (op)
    {
        case BinaryOp::Equal:           return Intrinsic::Equal;
        case BinaryOp::NotEqual:        return Intrinsic::NotEqual;
        case BinaryOp::Less:            return Intrinsic::LessThan;
        case BinaryOp::Greater:         return Intrinsic::GreaterThan;
        case BinaryOp::LessEqual:       return Intrinsic::LessThanEqual;
        case BinaryOp::GreaterEqual:    return Intrinsic::GreaterThanEqual;
        default:                        return Intrinsic::Undefined;
    }
}

void ExprConverter::ConvertExprVectorCompare(ExprPtr& expr)
{
    if (expr)
    {
        if (auto binaryExpr = expr->As<BinaryExpr>())
        {
            /* Convert vector comparison */
            if (IsCompareOp(binaryExpr->op))
            {
                auto typeDen = binaryExpr->GetTypeDenoter()->Get();
                if (typeDen->IsVector())
                {
                    /* Convert comparison operator into intrinsic */
                    auto intrinsic = CompareOpToIntrinsic(binaryExpr->op);
                    expr = ASTFactory::MakeIntrinsicCallExpr(
                        intrinsic, "vec_compare", nullptr,
                        { binaryExpr->lhsExpr, binaryExpr->rhsExpr }
                    );
                }
            }
        }
        else if (auto ternaryExpr = expr->As<TernaryExpr>())
        {
            /* Convert ternary to 'lerp' intrinsic, if the condition is a vector comparison */
            if (ternaryExpr->IsVectorCondition())
            {
                expr = ASTFactory::MakeIntrinsicCallExpr(
                    Intrinsic::Lerp, "lerp", nullptr,
                    { ternaryExpr->thenExpr, ternaryExpr->elseExpr, ternaryExpr->condExpr }
                );
            }
        }
    }
}

void ExprConverter::ConvertExprImageAccess(ExprPtr& expr)
{
    if (expr && !expr->flags(Expr::wasConverted))
    {
        /* Is this an array access to an image buffer or texture? */
        if (auto varAccessExpr = expr->As<VarAccessExpr>())
            ConvertExprImageAccessVarAccess(expr, varAccessExpr);
        else if (auto arrayAccessExpr = expr->As<ArrayAccessExpr>())
            ConvertExprImageAccessArrayAccess(expr, arrayAccessExpr);
    }
}

//private
void ExprConverter::ConvertExprImageAccessVarAccess(ExprPtr& expr, VarAccessExpr* varAccessExpr)
{
    /* Is this the variable a buffer declaration? */
    if (auto bufferDecl = varAccessExpr->varIdent->FetchSymbol<BufferDecl>())
    {
        /* Is the buffer declaration a read/write texture? */
        const auto bufferType = bufferDecl->GetBufferType();
        if (IsRWTextureBufferType(bufferType))
        {
            /* Get buffer type denoter from array indices of identifier */
            auto bufferTypeDen = bufferDecl->GetTypeDenoter()->GetFromArray(
                varAccessExpr->varIdent->arrayIndices.size()
            );

            if (!varAccessExpr->varIdent->arrayIndices.empty())
            {
                /* Translate reads to imageLoad() function call */
                if (varAccessExpr->assignOp == AssignOp::Undefined)
                {
                    /* Make first argument expression */
                    auto arg0Expr = ASTFactory::MakeVarAccessExpr(varAccessExpr->varIdent->ident, bufferDecl);
                    arg0Expr->flags << Expr::wasConverted;

                    /* Get second argument expression (last array index) */
                    auto arg1Expr = varAccessExpr->varIdent->arrayIndices.back();

                    /* Convert expression to intrinsic call */
                    expr = ASTFactory::MakeIntrinsicCallExpr(
                        Intrinsic::Image_Load, "imageLoad", bufferTypeDen, { arg0Expr, arg1Expr }
                    );
                }
                /* Translate writes to imageStore() function call */
                else // Write
                {
                    // TODO - Handle different kind of set operations
                }
            }
            else
                RuntimeErr("missing array index in operator of '" + bufferTypeDen->ToString() + "'", varAccessExpr);
        }
    }
}

//private
void ExprConverter::ConvertExprImageAccessArrayAccess(ExprPtr& expr, ArrayAccessExpr* arrayAccessExpr)
{
    /* Fetch variable identifier from inner sub expression */
    if (auto varIdent = arrayAccessExpr->expr->FetchVarIdent())
    {
        /* Is this the variable a buffer declaration? */
        if (auto bufferDecl = varIdent->FetchSymbol<BufferDecl>())
        {
            /* Is the buffer declaration a read/write texture? */
            const auto bufferType = bufferDecl->GetBufferType();
            if (IsRWTextureBufferType(bufferType))
            {
                /* Get buffer type denoter from array indices of array access plus identifier */
                auto bufferTypeDen = bufferDecl->GetTypeDenoter()->GetFromArray(
                    arrayAccessExpr->arrayIndices.size() + varIdent->arrayIndices.size()
                );

                if (!arrayAccessExpr->arrayIndices.empty())
                {
                    /* Make first argument expression */
                    auto arg0Expr = ASTFactory::MakeVarAccessExpr(varIdent->ident, bufferDecl);
                    arg0Expr->flags << Expr::wasConverted;

                    /* Get second argument expression (last array index) */
                    auto arg1Expr = arrayAccessExpr->arrayIndices.back();

                    /* Convert expression to intrinsic call */
                    expr = ASTFactory::MakeIntrinsicCallExpr(
                        Intrinsic::Image_Load, "imageLoad", bufferTypeDen, { arg0Expr, arg1Expr }
                    );
                }
                else
                    RuntimeErr("missing array index in operator of '" + bufferTypeDen->ToString() + "'", arrayAccessExpr);
            }
        }
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
    /* Move expression as sub expression into bracket */
    expr = ASTFactory::MakeBracketExpr(expr);
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

void ExprConverter::IfFlaggedConvertExprVectorCompare(ExprPtr& expr)
{
    if (conversionFlags_(ConvertVectorCompare))
        ConvertExprVectorCompare(expr);
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

void ExprConverter::IfFlaggedConvertExprImageAccess(ExprPtr& expr)
{
    if (conversionFlags_(ConvertImageAccess))
        ConvertExprImageAccess(expr);
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ExprConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    for (auto& funcArg : ast->arguments)
    {
        IfFlaggedConvertExprVectorCompare(funcArg);
        IfFlaggedConvertExprImageAccess(funcArg);
    }

    VISIT_DEFAULT(FunctionCall);

    for (auto& funcArg : ast->arguments)
        IfFlaggedConvertExprVectorSubscript(funcArg);

    ast->ForEachArgumentWithParameterType(
        [this](ExprPtr& funcArg, const TypeDenoter& paramTypeDen)
        {
            IfFlaggedConvertExprIfCastRequired(funcArg, paramTypeDen);
        }
    );
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (ast->initializer)
    {
        IfFlaggedConvertExprVectorCompare(ast->initializer);
        IfFlaggedConvertExprImageAccess(ast->initializer);

        VISIT_DEFAULT(VarDecl);

        IfFlaggedConvertExprVectorSubscript(ast->initializer);
        IfFlaggedConvertExprIfCastRequired(ast->initializer, *ast->GetTypeDenoter()->Get());
    }
    else
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
    IfFlaggedConvertExprVectorCompare(ast->condition);
    IfFlaggedConvertExprVectorCompare(ast->iteration);

    IfFlaggedConvertExprImageAccess(ast->condition);
    IfFlaggedConvertExprImageAccess(ast->iteration);

    VISIT_DEFAULT(ForLoopStmnt);
    
    IfFlaggedConvertExprVectorSubscript(ast->condition);
    IfFlaggedConvertExprVectorSubscript(ast->iteration);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    IfFlaggedConvertExprVectorCompare(ast->condition);

    VISIT_DEFAULT(WhileLoopStmnt);

    IfFlaggedConvertExprVectorSubscript(ast->condition);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    IfFlaggedConvertExprVectorCompare(ast->condition);
    IfFlaggedConvertExprImageAccess(ast->condition);

    VISIT_DEFAULT(DoWhileLoopStmnt);

    IfFlaggedConvertExprVectorSubscript(ast->condition);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    IfFlaggedConvertExprVectorCompare(ast->condition);
    IfFlaggedConvertExprImageAccess(ast->condition);

    VISIT_DEFAULT(IfStmnt);

    IfFlaggedConvertExprVectorSubscript(ast->condition);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    IfFlaggedConvertExprVectorCompare(ast->expr);
    IfFlaggedConvertExprImageAccess(ast->expr);

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
    IfFlaggedConvertExprVectorCompare(ast->condExpr);
    IfFlaggedConvertExprVectorCompare(ast->thenExpr);
    IfFlaggedConvertExprVectorCompare(ast->elseExpr);

    IfFlaggedConvertExprImageAccess(ast->condExpr);
    IfFlaggedConvertExprImageAccess(ast->thenExpr);
    IfFlaggedConvertExprImageAccess(ast->elseExpr);

    VISIT_DEFAULT(TernaryExpr);

    IfFlaggedConvertExprVectorSubscript(ast->condExpr);
    IfFlaggedConvertExprVectorSubscript(ast->thenExpr);
    IfFlaggedConvertExprVectorSubscript(ast->elseExpr);
}

// Convert right-hand-side expression (if cast required)
IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    IfFlaggedConvertExprVectorCompare(ast->lhsExpr);
    IfFlaggedConvertExprVectorCompare(ast->rhsExpr);

    IfFlaggedConvertExprImageAccess(ast->lhsExpr);
    IfFlaggedConvertExprImageAccess(ast->rhsExpr);

    VISIT_DEFAULT(BinaryExpr);
    
    IfFlaggedConvertExprVectorSubscript(ast->lhsExpr);
    IfFlaggedConvertExprVectorSubscript(ast->rhsExpr);

    bool matchTypeSize = (ast->op != BinaryOp::Mul && ast->op != BinaryOp::Div);

    auto commonTypeDen = TypeDenoter::FindCommonTypeDenoter(
        ast->lhsExpr->GetTypeDenoter()->Get(),
        ast->rhsExpr->GetTypeDenoter()->Get()
    );

    IfFlaggedConvertExprIfCastRequired(ast->lhsExpr, *commonTypeDen, matchTypeSize);
    IfFlaggedConvertExprIfCastRequired(ast->rhsExpr, *commonTypeDen, matchTypeSize);

    ast->ResetTypeDenoter();
}

// Wrap unary expression if the next sub expression is again an unary expression
IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    IfFlaggedConvertExprVectorCompare(ast->expr);
    IfFlaggedConvertExprImageAccess(ast->expr);

    VISIT_DEFAULT(UnaryExpr);

    IfFlaggedConvertExprVectorSubscript(ast->expr);

    if (ast->expr->Type() == AST::Types::UnaryExpr)
        IfFlaggedConvertExprIntoBracket(ast->expr);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    IfFlaggedConvertExprVectorCompare(ast->expr);
    IfFlaggedConvertExprImageAccess(ast->expr);

    VISIT_DEFAULT(BracketExpr);

    IfFlaggedConvertExprVectorSubscript(ast->expr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    IfFlaggedConvertExprVectorCompare(ast->expr);
    IfFlaggedConvertExprImageAccess(ast->expr);

    VISIT_DEFAULT(CastExpr);

    IfFlaggedConvertExprVectorSubscript(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    if (ast->assignExpr)
    {
        IfFlaggedConvertExprVectorCompare(ast->assignExpr);
        IfFlaggedConvertExprImageAccess(ast->assignExpr);

        VISIT_DEFAULT(VarAccessExpr);

        IfFlaggedConvertExprVectorSubscript(ast->assignExpr);
        IfFlaggedConvertExprIfCastRequired(ast->assignExpr, *ast->GetTypeDenoter()->Get());
    }
    else
        VISIT_DEFAULT(VarAccessExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
