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
#include "ReportIdents.h"


namespace Xsc
{


void ExprConverter::Convert(Program& program, const Flags& conversionFlags)
{
    /* Visit program AST */
    conversionFlags_ = conversionFlags;
    if (conversionFlags_ != 0)
        Visit(&program);
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

/* ----- Conversion ----- */

void ExprConverter::ConvertExpr(ExprPtr& expr, const Flags& flags)
{
    auto enabled = Flags(flags & conversionFlags_);

    if (enabled(ConvertVectorCompare))
        ConvertExprVectorCompare(expr);

    if (enabled(ConvertImageAccess))
        ConvertExprImageAccess(expr);

    if (enabled(ConvertVectorSubscripts))
        ConvertExprVectorSubscript(expr);

    if (enabled(WrapUnaryExpr))
        ConvertExprIntoBracket(expr);
}

void ExprConverter::ConvertExprList(std::vector<ExprPtr>& exprList, const Flags& flags)
{
    for (auto& expr : exprList)
        ConvertExpr(expr, flags);
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
            ConvertExprVectorCompareBinary(expr, binaryExpr);
        else if (auto ternaryExpr = expr->As<TernaryExpr>())
            ConvertExprVectorCompareTernary(expr, ternaryExpr);
    }
}

void ExprConverter::ConvertExprVectorCompareBinary(ExprPtr& expr, BinaryExpr* binaryExpr)
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

void ExprConverter::ConvertExprVectorCompareTernary(ExprPtr& expr, TernaryExpr* ternaryExpr)
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

static BinaryOp AssignOpToBinaryOp(const AssignOp op)
{
    switch (op)
    {
        case AssignOp::Add:         return BinaryOp::Add;
        case AssignOp::Sub:         return BinaryOp::Sub;
        case AssignOp::Mul:         return BinaryOp::Mul;
        case AssignOp::Div:         return BinaryOp::Div;
        case AssignOp::Mod:         return BinaryOp::Mod;
        case AssignOp::LShift:      return BinaryOp::LShift;
        case AssignOp::RShift:      return BinaryOp::RShift;
        case AssignOp::Or:          return BinaryOp::Or;
        case AssignOp::And:         return BinaryOp::And;
        case AssignOp::Xor:         return BinaryOp::Xor;
        default:                    return BinaryOp::Undefined;
    }
}

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
                    /* Make first argument expression */
                    auto arg0Expr = ASTFactory::MakeVarAccessExpr(varAccessExpr->varIdent->ident, bufferDecl);
                    arg0Expr->flags << Expr::wasConverted;

                    /* Get second argument expression (last array index) */
                    auto arg1Expr = varAccessExpr->varIdent->arrayIndices.back();

                    /* Get third argument expression (store value) */
                    ExprPtr arg2Expr;

                    /* If its a normal assignment, then assign expression is the store value */
                    if(varAccessExpr->assignOp == AssignOp::Set)
                        arg2Expr = varAccessExpr->assignExpr;
                    else 
                    {
                        /* Otherwise must be compound assignment, in which case we need to do a read as well */
                        auto lhsExpr = ASTFactory::MakeIntrinsicCallExpr(
                            Intrinsic::Image_Load, "imageLoad", bufferTypeDen, { arg0Expr, arg1Expr }
                        );

                        auto rhsExpr = varAccessExpr->assignExpr;
                        BinaryOp binaryOp = AssignOpToBinaryOp(varAccessExpr->assignOp);

                        arg2Expr = ASTFactory::MakeBinaryExpr(lhsExpr, binaryOp, rhsExpr);
                    }

                    /* Convert expression to intrinsic call */
                    expr = ASTFactory::MakeIntrinsicCallExpr(
                        Intrinsic::Image_Store, "imageStore", nullptr, { arg0Expr, arg1Expr, arg2Expr }
                    );
                }
            }
            else
                RuntimeErr(R_MissingArrayIndexInOp(bufferTypeDen->ToString()), varAccessExpr);
        }
    }
}

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
                    RuntimeErr(R_MissingArrayIndexInOp(bufferTypeDen->ToString()), arrayAccessExpr);
            }
        }
    }
}

void ExprConverter::IfFlaggedConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize)
{
    if (conversionFlags_(ConvertImplicitCasts))
        ConvertExprIfCastRequired(expr, targetTypeDen, matchTypeSize);
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ExprConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

bool IsInterlockedIntristic(Intrinsic intrisic)
{
    switch(intrisic)
    {
    case Intrinsic::InterlockedAdd:
    case Intrinsic::InterlockedAnd:
    case Intrinsic::InterlockedOr:
    case Intrinsic::InterlockedXor:
    case Intrinsic::InterlockedMin:
    case Intrinsic::InterlockedMax:
    case Intrinsic::InterlockedCompareExchange:
    case Intrinsic::InterlockedExchange:
        return true;
    default:
        return false;
    }
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    Flags preVisitFlags = AllPreVisit;

    /** Interlock (atomic) intristics require actual buffer, and not their contents. */
    if (IsInterlockedIntristic(ast->intrinsic))
        preVisitFlags = preVisitFlags & ~ConvertImageAccess;

    ConvertExprList(ast->arguments, preVisitFlags);
    {
        VISIT_DEFAULT(FunctionCall);
    }
    ConvertExprList(ast->arguments, AllPostVisit);

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
        ConvertExpr(ast->initializer, AllPreVisit);
        {
            VISIT_DEFAULT(VarDecl);
        }
        ConvertExpr(ast->initializer, AllPostVisit);

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
    ConvertExpr(ast->condition, AllPreVisit);
    ConvertExpr(ast->iteration, AllPreVisit);
    {
        VISIT_DEFAULT(ForLoopStmnt);
    }
    ConvertExpr(ast->condition, AllPostVisit);
    ConvertExpr(ast->iteration, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    ConvertExpr(ast->condition, AllPreVisit);
    {
        VISIT_DEFAULT(WhileLoopStmnt);
    }
    ConvertExpr(ast->condition, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    ConvertExpr(ast->condition, AllPreVisit);
    {
        VISIT_DEFAULT(DoWhileLoopStmnt);
    }
    ConvertExpr(ast->condition, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    ConvertExpr(ast->condition, AllPreVisit);
    {
        VISIT_DEFAULT(IfStmnt);
    }
    ConvertExpr(ast->condition, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    ConvertExpr(ast->expr, AllPreVisit);
    {
        VISIT_DEFAULT(ExprStmnt);
    }
    ConvertExpr(ast->expr, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    if (ast->expr)
    {
        ConvertExpr(ast->expr, AllPreVisit);
        {
            VISIT_DEFAULT(ReturnStmnt);
        }
        ConvertExpr(ast->expr, AllPostVisit);

        if (auto funcDecl = ActiveFunctionDecl())
            IfFlaggedConvertExprIfCastRequired(ast->expr, *(funcDecl->returnType->GetTypeDenoter()->Get()));
    }
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    ConvertExpr(ast->condExpr, AllPreVisit);
    ConvertExpr(ast->thenExpr, AllPreVisit);
    ConvertExpr(ast->elseExpr, AllPreVisit);
    {
        VISIT_DEFAULT(TernaryExpr);
    }
    ConvertExpr(ast->condExpr, AllPostVisit);
    ConvertExpr(ast->thenExpr, AllPostVisit);
    ConvertExpr(ast->elseExpr, AllPostVisit);
}

// Convert right-hand-side expression (if cast required)
IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    ConvertExpr(ast->lhsExpr, AllPreVisit);
    ConvertExpr(ast->rhsExpr, AllPreVisit);
    {
        VISIT_DEFAULT(BinaryExpr);
    }
    ConvertExpr(ast->lhsExpr, AllPostVisit);
    ConvertExpr(ast->rhsExpr, AllPostVisit);

    /* Convert sub expressions if cast required, then reset type denoter */
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
    ConvertExpr(ast->expr, AllPreVisit);
    {
        VISIT_DEFAULT(UnaryExpr);
    }
    ConvertExpr(ast->expr, AllPostVisit);

    if (ast->expr->Type() == AST::Types::UnaryExpr)
        ConvertExpr(ast->expr, WrapUnaryExpr);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    ConvertExpr(ast->expr, AllPreVisit);
    {
        VISIT_DEFAULT(BracketExpr);
    }
    ConvertExpr(ast->expr, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    ConvertExpr(ast->expr, AllPreVisit);
    {
        VISIT_DEFAULT(CastExpr);
    }
    ConvertExpr(ast->expr, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    if (ast->assignExpr)
    {
        ConvertExpr(ast->assignExpr, AllPreVisit);
        {
            VISIT_DEFAULT(VarAccessExpr);
        }
        ConvertExpr(ast->assignExpr, AllPostVisit);

        IfFlaggedConvertExprIfCastRequired(ast->assignExpr, *ast->GetTypeDenoter()->Get());
    }
    else
        VISIT_DEFAULT(VarAccessExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
