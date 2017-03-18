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
    const auto& sourceTypeDen = expr->GetTypeDenoter()->GetAliased();
    if (auto baseSourceTypeDen = sourceTypeDen.As<BaseTypeDenoter>())
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
    if (auto dataType = MustCastExprToDataType(targetTypeDen, expr->GetTypeDenoter()->GetAliased(), matchTypeSize))
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
    if (expr)
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
}

void ExprConverter::ConvertExprList(std::vector<ExprPtr>& exprList, const Flags& flags)
{
    for (auto& expr : exprList)
        ConvertExpr(expr, flags);
}

void ExprConverter::ConvertExprVectorSubscript(ExprPtr& expr)
{
    if (auto objectExpr = expr->As<ObjectExpr>())
        ConvertExprVectorSubscriptObject(expr, objectExpr);
}

void ExprConverter::ConvertExprVectorSubscriptObject(ExprPtr& expr, ObjectExpr* objectExpr)
{
    if (!objectExpr->symbolRef && objectExpr->prefixExpr)
    {
        /* Get type denoter of prefix expression */
        const auto& prefixTypeDen = objectExpr->prefixExpr->GetTypeDenoter()->GetAliased();
        if (prefixTypeDen.IsScalar())
        {
            /* Convert vector subscript to cast expression */
            auto vectorTypeDen = objectExpr->GetTypeDenoterFromSubscript();

            /* Convert to cast expression */
            expr = ASTFactory::MakeCastExpr(vectorTypeDen, objectExpr->prefixExpr);
        }
    }
}

//TODO: make this a public function -> move to "ASTEnums.h/.cpp" files
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
    if (auto binaryExpr = expr->As<BinaryExpr>())
        ConvertExprVectorCompareBinary(expr, binaryExpr);
    else if (auto ternaryExpr = expr->As<TernaryExpr>())
        ConvertExprVectorCompareTernary(expr, ternaryExpr);
}

void ExprConverter::ConvertExprVectorCompareBinary(ExprPtr& expr, BinaryExpr* binaryExpr)
{
    /* Convert vector comparison */
    if (IsCompareOp(binaryExpr->op))
    {
        const auto& typeDen = binaryExpr->GetTypeDenoter()->GetAliased();
        if (typeDen.IsVector())
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
    if (!expr->flags(Expr::wasConverted))
    {
        /* Is this an array access to an image buffer or texture? */
        if (auto assignExpr = expr->As<AssignExpr>())
            ConvertExprImageAccessAssign(expr, assignExpr);
        else if (auto arrayAccessExpr = expr->As<ArrayAccessExpr>())
            ConvertExprImageAccessArrayAccess(expr, arrayAccessExpr);
    }
}

void ExprConverter::ConvertExprImageAccessAssign(ExprPtr& expr, AssignExpr* assignExpr)
{
    if (auto arrayAccessExpr = assignExpr->lvalueExpr->As<ArrayAccessExpr>())
        ConvertExprImageAccessArrayAccess(expr, arrayAccessExpr, assignExpr);
}

/*
~~~~~~~~~~ TODO: ~~~~~~~~~~
conversion is wrong when the index expression contains a function call and the assignemnt is a compound!
e.g. "rwTex[getIndex()] += 2;" --> "imageStore(rwTex, getIndex(), imageLoad(rwTex, getIndex()) + 2)"
results in two function calls! Thus the index expression must be moved into a separated statement.
*/
void ExprConverter::ConvertExprImageAccessArrayAccess(ExprPtr& expr, ArrayAccessExpr* arrayAccessExpr, AssignExpr* assignExpr)
{
    /* Fetch buffer type denoter from l-value prefix expression */
    const auto& prefixTypeDen = arrayAccessExpr->prefixExpr->GetTypeDenoter()->GetAliased();
    if (auto bufferTypeDen = prefixTypeDen.As<BufferTypeDenoter>())
    {
        if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
        {
            /* Is the buffer declaration a read/write texture? */
            const auto bufferType = bufferDecl->GetBufferType();
            if (IsRWTextureBufferType(bufferType))
            {
                /* Get buffer type denoter from array indices of array access plus identifier */
                auto bufferTypeDen = bufferDecl->GetTypeDenoter()->GetSub(arrayAccessExpr);

                if (!arrayAccessExpr->arrayIndices.empty())
                {
                    /* Make first argument expression */
                    auto arg0Expr = arrayAccessExpr->prefixExpr;
                    arg0Expr->flags << Expr::wasConverted;

                    /* Get second argument expression (last array index) */
                    auto arg1Expr = arrayAccessExpr->arrayIndices.back();

                    if (assignExpr)
                    {
                        /* Get third argument expression (store value) */
                        ExprPtr arg2Expr;

                        if (assignExpr->op == AssignOp::Set)
                        {
                            /* Take r-value expression for standard assignemnt */
                            arg2Expr = assignExpr->rvalueExpr;
                        }
                        else 
                        {
                            /* Make compound assignment with an image-load instruction first */
                            auto lhsExpr = ASTFactory::MakeIntrinsicCallExpr(
                                Intrinsic::Image_Load, "imageLoad", bufferTypeDen, { arg0Expr, arg1Expr }
                            );

                            auto rhsExpr = assignExpr->rvalueExpr;

                            const auto binaryOp = AssignOpToBinaryOp(assignExpr->op);

                            arg2Expr = ASTFactory::MakeBinaryExpr(lhsExpr, binaryOp, rhsExpr);
                        }

                        /* Convert expression to intrinsic call */
                        expr = ASTFactory::MakeIntrinsicCallExpr(
                            Intrinsic::Image_Store, "imageStore", nullptr, { arg0Expr, arg1Expr, arg2Expr }
                        );
                    }
                    else
                    {
                        /* Convert expression to intrinsic call */
                        expr = ASTFactory::MakeIntrinsicCallExpr(
                            Intrinsic::Image_Load, "imageLoad", bufferTypeDen, { arg0Expr, arg1Expr }
                        );
                    }
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

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    ConvertExprList(ast->arguments, AllPreVisit);
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

        IfFlaggedConvertExprIfCastRequired(ast->initializer, ast->GetTypeDenoter()->GetAliased());
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
            IfFlaggedConvertExprIfCastRequired(ast->expr, funcDecl->returnType->GetTypeDenoter()->GetAliased());
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
        ast->lhsExpr->GetTypeDenoter()->GetSub(),
        ast->rhsExpr->GetTypeDenoter()->GetSub()
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

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    ConvertExpr(ast->prefixExpr, AllPreVisit);
    {
        VISIT_DEFAULT(FunctionCallExpr);
    }
    ConvertExpr(ast->prefixExpr, AllPostVisit);
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

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    ConvertExpr(ast->prefixExpr, AllPreVisit);
    {
        VISIT_DEFAULT(ObjectExpr);
    }
    ConvertExpr(ast->prefixExpr, AllPostVisit);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    ConvertExpr(ast->lvalueExpr, AllPreVisit);
    ConvertExpr(ast->rvalueExpr, AllPreVisit);
    {
        VISIT_DEFAULT(AssignExpr);
    }
    ConvertExpr(ast->lvalueExpr, AllPostVisit);
    ConvertExpr(ast->rvalueExpr, AllPostVisit);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
