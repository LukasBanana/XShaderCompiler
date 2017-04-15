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
        const auto enabled = Flags(flags & conversionFlags_);

        if (enabled(ConvertLog10))
            ConvertExprIntrinsicCallLog10(expr);

        if (enabled(ConvertVectorCompare))
            ConvertExprVectorCompare(expr);

        if (enabled(ConvertImageAccess))
            ConvertExprImageAccess(expr);

        if (enabled(ConvertSamplerBufferAccess))
            ConvertExprSamplerBufferAccess(expr);

        if (enabled(ConvertVectorSubscripts))
            ConvertExprVectorSubscript(expr);

        if (enabled(ConvertUnaryExpr))
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

void ExprConverter::ConvertExprVectorCompare(ExprPtr& expr)
{
    if (auto unaryExpr = expr->As<UnaryExpr>())
        ConvertExprVectorCompareUnary(expr, unaryExpr);
    else if (auto binaryExpr = expr->As<BinaryExpr>())
        ConvertExprVectorCompareBinary(expr, binaryExpr);
    else if (auto ternaryExpr = expr->As<TernaryExpr>())
        ConvertExprVectorCompareTernary(expr, ternaryExpr);
}

void ExprConverter::ConvertExprVectorCompareUnary(ExprPtr& expr, UnaryExpr* unaryExpr)
{
    /* Convert vector condition */
    if (IsLogicalOp(unaryExpr->op))
    {
        const auto& typeDen = unaryExpr->GetTypeDenoter();
        if (typeDen->GetAliased().IsVector())
        {
            switch (unaryExpr->op)
            {
                case UnaryOp::LogicalNot:
                {
                    /* Convert unary-'NOT' expression into an intrinsic call */
                    expr = ASTFactory::MakeIntrinsicCallExpr(Intrinsic::Not, "not", typeDen, { unaryExpr->expr });
                }
                break;

                default:
                break;
            }
        }
    }
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
        else if (auto arrayExpr = expr->As<ArrayExpr>())
            ConvertExprImageAccessArray(expr, arrayExpr);
    }
}

void ExprConverter::ConvertExprImageAccessAssign(ExprPtr& expr, AssignExpr* assignExpr)
{
    if (auto arrayExpr = assignExpr->lvalueExpr->As<ArrayExpr>())
        ConvertExprImageAccessArray(expr, arrayExpr, assignExpr);
}

/*
~~~~~~~~~~ TODO: ~~~~~~~~~~
conversion is wrong when the index expression contains a function call and the assignemnt is a compound!
e.g. "rwTex[getIndex()] += 2;" --> "imageStore(rwTex, getIndex(), imageLoad(rwTex, getIndex()) + 2)"
results in two function calls! Thus the index expression must be moved into a separated statement.
*/
void ExprConverter::ConvertExprImageAccessArray(ExprPtr& expr, ArrayExpr* arrayExpr, AssignExpr* assignExpr)
{
    /* Fetch buffer type denoter from l-value prefix expression */
    const auto& prefixTypeDen = arrayExpr->prefixExpr->GetTypeDenoter()->GetAliased();

    if (auto bufferTypeDen = prefixTypeDen.As<BufferTypeDenoter>())
    {
        if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
        {
            /* Is the buffer declaration a read/write texture? */
            const auto bufferType = bufferDecl->GetBufferType();
            if (IsRWTextureBufferType(bufferType))
            {
                /* Get buffer type denoter from array indices of array access plus identifier */
                auto bufferTypeDen = bufferDecl->GetTypeDenoter()->GetSub(arrayExpr);
                if (auto baseBufferTypeDen = bufferTypeDen->As<BaseTypeDenoter>())
                {
                    /* Create a call denoter for the return value */
                    auto callTypeDen = ASTFactory::MakeBufferAccessCallTypeDenoter(*baseBufferTypeDen);

                    if (!arrayExpr->arrayIndices.empty())
                    {
                        /* Make first argument expression */
                        auto arg0Expr = arrayExpr->prefixExpr;
                        arg0Expr->flags << Expr::wasConverted;

                        /* Get second argument expression (last array index) */
                        auto arg1Expr = arrayExpr->arrayIndices.back();

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
                                    Intrinsic::Image_Load, "imageLoad", callTypeDen, { arg0Expr, arg1Expr }
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
                                Intrinsic::Image_Load, "imageLoad", callTypeDen, { arg0Expr, arg1Expr }
                            );
                        }
                    }
                    else
                        RuntimeErr(R_MissingArrayIndexInOp(bufferTypeDen->ToString()), arrayExpr);
                }
            }
        }
    }
}

void ExprConverter::ConvertExprSamplerBufferAccess(ExprPtr& expr)
{
    if (!expr->flags(Expr::wasConverted))
    {
        /* Is this an array access to a sampler buffer? */
        if (auto arrayExpr = expr->As<ArrayExpr>())
            ConvertExprSamplerBufferAccessArray(expr, arrayExpr);
    }
}

void ExprConverter::ConvertExprSamplerBufferAccessArray(ExprPtr& expr, ArrayExpr* arrayExpr)
{
    /* Fetch buffer type denoter from l-value prefix expression */
    const auto& prefixTypeDen = arrayExpr->prefixExpr->GetTypeDenoter()->GetAliased();
    if (auto bufferTypeDen = prefixTypeDen.As<BufferTypeDenoter>())
    {
        if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
        {
            /* Is the buffer declaration a sampler buffer? */
            const auto bufferType = bufferDecl->GetBufferType();
            if (bufferType == BufferType::Buffer)
            {
                /* Get buffer type denoter from array indices of array access plus identifier */
                auto bufferTypeDen = bufferDecl->GetTypeDenoter()->GetSub(arrayExpr);

                if (!arrayExpr->arrayIndices.empty())
                {
                    /* Make first argument expression */
                    auto arg0Expr = arrayExpr->prefixExpr;
                    arg0Expr->flags << Expr::wasConverted;

                    /* Get second argument expression (last array index) */
                    auto arg1Expr = arrayExpr->arrayIndices.back();

                    /* Convert expression to intrinsic call */
                    expr = ASTFactory::MakeIntrinsicCallExpr(
                        Intrinsic::Texture_Load_1, "texelFetch", bufferTypeDen, { arg0Expr, arg1Expr }
                    );
                }
                else
                    RuntimeErr(R_MissingArrayIndexInOp(bufferTypeDen->ToString()), arrayExpr);
            }
        }
    }
}

void ExprConverter::ConvertExprIntrinsicCallLog10(ExprPtr& expr)
{
    /* Is this a call expression to the "log10" intrinisc? */
    if (auto callExpr = expr->As<CallExpr>())
    {
        if (callExpr->intrinsic == Intrinsic::Log10 && callExpr->arguments.size() == 1)
        {
            /* Get argument type denoter */
            const auto& arg0 = callExpr->arguments.front();
            const auto& typeDen = arg0->GetTypeDenoter()->GetAliased();
            if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
            {
                /* Convert intrinsic type from 'log10' to 'log' */
                callExpr->intrinsic = Intrinsic::Log;

                /* Create "log(10)" expression */
                auto literalExpr = ASTFactory::MakeLiteralExpr(DataType::Int, "10");
                literalExpr->ConvertDataType(BaseDataType(baseTypeDen->dataType));

                auto rhsExpr = ASTFactory::MakeIntrinsicCallExpr(Intrinsic::Log, "log", nullptr, { literalExpr });

                /* Create binary expression for "log(x) / log(10)" */
                auto binaryExpr = ASTFactory::MakeBinaryExpr(expr, BinaryOp::Div, rhsExpr);

                expr = ASTFactory::MakeBracketExpr(binaryExpr);
            }
        }
    }
}

void ExprConverter::ConvertExprTargetType(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize)
{
    if (expr)
    {
        if (conversionFlags_(ConvertImplicitCasts))
            ConvertExprIfCastRequired(expr, targetTypeDen, matchTypeSize);

        if (conversionFlags_(ConvertInitializer))
        {
            if (auto initExpr = expr->As<InitializerExpr>())
                ConvertExprTargetTypeInitializer(expr, initExpr, targetTypeDen);
        }
    }
}

void ExprConverter::ConvertExprTargetTypeInitializer(ExprPtr& expr, InitializerExpr* initExpr, const TypeDenoter& targetTypeDen)
{
    /* Convert sub expressions for array type denoters */
    if (auto arrayTargetTypeDen = targetTypeDen.As<ArrayTypeDenoter>())
    {
        const auto& subTypeDen = arrayTargetTypeDen->subTypeDenoter->GetAliased();
        for (auto& expr : initExpr->exprs)
            ConvertExprTargetType(expr, subTypeDen);
    }

    /* Convert initializer expression into type constructor */
    expr = ASTFactory::MakeTypeCtorCallExpr(targetTypeDen.Copy(), initExpr->exprs);
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ExprConverter::Visit##AST_NAME(AST_NAME* ast, void* args)

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

        ConvertExprTargetType(ast->initializer, ast->GetTypeDenoter()->GetAliased());
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

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    ConvertExpr(ast->selector, AllPreVisit);
    {
        VISIT_DEFAULT(SwitchStmnt);
    }
    ConvertExpr(ast->selector, AllPostVisit);
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
            ConvertExprTargetType(ast->expr, funcDecl->returnType->GetTypeDenoter()->GetAliased());
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

    ConvertExprTargetType(ast->lhsExpr, *commonTypeDen, matchTypeSize);
    ConvertExprTargetType(ast->rhsExpr, *commonTypeDen, matchTypeSize);

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
        ConvertExpr(ast->expr, ConvertUnaryExpr);
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    /* Interlock (atomic) intristics require actual buffer, and not their contents */
    Flags preVisitFlags = AllPreVisit;
    if (IsInterlockedIntristic(ast->intrinsic))
        preVisitFlags.Remove(ConvertImageAccess);

    ConvertExpr(ast->prefixExpr, AllPreVisit);
    ConvertExprList(ast->arguments, preVisitFlags);
    {
        VISIT_DEFAULT(CallExpr);
    }
    ConvertExprList(ast->arguments, AllPostVisit);
    ConvertExpr(ast->prefixExpr, AllPostVisit);

    ast->ForEachArgumentWithParameterType(
        [this](ExprPtr& funcArg, const TypeDenoter& paramTypeDen)
        {
            ConvertExprTargetType(funcArg, paramTypeDen);
        }
    );
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

    ConvertExprTargetType(ast->rvalueExpr, ast->lvalueExpr->GetTypeDenoter()->GetAliased());
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    for (auto& expr : ast->arrayIndices)
        ConvertExpr(expr, AllPreVisit);
    
    VISIT_DEFAULT(ArrayExpr);
    
    for (auto& expr : ast->arrayIndices)
    {
        ConvertExpr(expr, AllPostVisit);
        
        /* Convert array index to integral type of same vector dimension */
        const auto& typeDen = expr->GetTypeDenoter()->GetAliased();
        if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
        {
            /* Convert either to 'uint' on default, or 'int' if the vector base type is 'int' */
            auto baseDataType = BaseDataType(baseTypeDen->dataType);
            if (baseDataType != DataType::Int)
                baseDataType = DataType::UInt;

            const auto intVecType = VectorDataType(baseDataType, VectorTypeDim(baseTypeDen->dataType));
            ConvertExprTargetType(expr, BaseTypeDenoter(intVecType));
        }
    }
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
