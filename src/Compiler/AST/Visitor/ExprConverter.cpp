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


void ExprConverter::Convert(Program& program, const Flags& conversionFlags, const NameMangling& nameMangling)
{
    /* Copy parameters */
    conversionFlags_    = conversionFlags;
    nameMangling_       = nameMangling;

    /* Visit program AST */
    if (conversionFlags_ != 0)
        Visit(&program);
}

// Returns the data type to which an expression must be casted, if the target data type and the source data type are incompatible.
static std::unique_ptr<DataType> MustCastExprToDataType(const DataType targetType, const DataType sourceType, bool matchTypeSize)
{
    /* Check for type mismatch */
    const auto targetDim = VectorTypeDim(targetType);
    const auto sourceDim = VectorTypeDim(sourceType);

    if ( ( targetDim != sourceDim && matchTypeSize ) ||
         ( IsBooleanType   (targetType) != IsBooleanType   (sourceType) ) ||
         ( IsUIntType      (targetType) && IsIntType       (sourceType) ) ||
         ( IsIntType       (targetType) && IsUIntType      (sourceType) ) ||
         ( IsRealType      (targetType) && IsIntegralType  (sourceType) ) ||
         ( IsIntegralType  (targetType) && IsRealType      (sourceType) ) ||
         ( IsDoubleRealType(targetType) != IsDoubleRealType(sourceType) ) )
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

static std::unique_ptr<DataType> MustCastExprToDataType(const TypeDenoter& targetTypeDen, const TypeDenoter& sourceTypeDen, bool matchTypeSize)
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

static void ConvertCastExpr(ExprPtr& expr, const DataType sourceType, const DataType targetType)
{
    /* If casting a vector to a higher dimension, zero expand because direct casts aren't supported */
    if (IsVectorType(sourceType) && IsVectorType(targetType))
    {
        auto targetDim = VectorTypeDim(targetType);
        auto sourceDim = VectorTypeDim(sourceType);

        if (sourceDim < targetDim)
        {
            /* Convert to cast expression and extend type constructor with sequential zero-literals (e.g. 'float3(v4)' => 'float4(v4, 0)') */
            auto typeDenoter = std::make_shared<BaseTypeDenoter>(targetType);

            std::vector<ExprPtr> args;
            args.push_back(expr);

            auto baseDataType = BaseDataType(targetType);
            for (auto i = sourceDim; i < targetDim; ++i)
                args.push_back(ASTFactory::MakeLiteralExpr(baseDataType, "0"));

            expr = ASTFactory::MakeTypeCtorCallExpr(typeDenoter, args);
        }
        else
        {
            /* Convert to cast expression with target data type*/
            expr = ASTFactory::ConvertExprBaseType(targetType, expr);
        }
    }
    else
    {
        /* Convert to cast expression with target data type */
        expr = ASTFactory::ConvertExprBaseType(targetType, expr);
    }
}

// Converts the expression to a cast expression if it is required for the specified target type.
void ExprConverter::ConvertExprIfCastRequired(ExprPtr& expr, const DataType targetType, bool matchTypeSize)
{
    if (expr)
    {
        const auto& sourceTypeDen = expr->GetTypeDenoter()->GetAliased();
        if (auto baseSourceTypeDen = sourceTypeDen.As<BaseTypeDenoter>())
        {
            if (auto dataType = MustCastExprToDataType(targetType, baseSourceTypeDen->dataType, matchTypeSize))
                ConvertCastExpr(expr, baseSourceTypeDen->dataType, *dataType);
        }
    }
}

void ExprConverter::ConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize)
{
    if (expr)
    {
        const auto& sourceTypeDen = expr->GetTypeDenoter()->GetAliased();
        if (auto dataType = MustCastExprToDataType(targetTypeDen, sourceTypeDen, matchTypeSize))
        {
            if (auto baseSourceTypeDen = sourceTypeDen.As<BaseTypeDenoter>())
                ConvertCastExpr(expr, baseSourceTypeDen->dataType, *dataType);
            else
            {
                /* Convert to cast expression with target data type if required */
                expr = ASTFactory::ConvertExprBaseType(*dataType, expr);
            }
        }
    }
}

void ExprConverter::ConvertExprIntoBracket(ExprPtr& expr)
{
    /* Move expression as sub expression into bracket */
    expr = ASTFactory::MakeBracketExpr(expr);
}

int ExprConverter::GetTextureDimFromExpr(Expr* expr, const AST* ast)
{
    if (expr)
    {
        const auto& typeDen = expr->GetTypeDenoter()->GetAliased();
        if (auto bufferTypeDen = typeDen.As<BufferTypeDenoter>())
        {
            /* Determine vector size for texture intrinsic parameters by texture buffer type */
            if (auto textureDim = GetBufferTypeTextureDim(bufferTypeDen->bufferType))
                return textureDim;
        }
        else if (auto samplerTypeDen = typeDen.As<SamplerTypeDenoter>())
        {
            /* Determine vector size for texture intrinsic parameters by sampler type */
            if (auto textureDim = GetSamplerTypeTextureDim(samplerTypeDen->samplerType))
                return textureDim;
        }
        RuntimeErr(R_FailedToGetTextureDim, ast);
    }
    RuntimeErr(R_FailedToGetTextureDim, ast);
}

std::string ExprConverter::GetMatrixSubscriptWrapperIdent(const NameMangling& nameMangling, const MatrixSubscriptUsage& subscriptUsage)
{
    return (nameMangling.temporaryPrefix + "subscript" + subscriptUsage.IndicesToString());
}


/*
 * ======= Private: =======
 */

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

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    if (conversionFlags_(ConvertLiteralHalfToFloat))
    {
        /* Convert half to float literal (e.g. "0.0h" to "0.0f") */
        if (ast->dataType == DataType::Half)
            ast->ConvertDataType(DataType::Float);
    }
}

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

    /* Convert condition expression if cast required */
    ExprConverter::ConvertExprIfCastRequired(ast->condExpr, DataType::Bool, false);
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
    auto lhsTypeDen = ast->lhsExpr->GetTypeDenoter()->GetSub();
    auto rhsTypeDen = ast->rhsExpr->GetTypeDenoter()->GetSub();

    auto commonTypeDen = TypeDenoter::FindCommonTypeDenoter(lhsTypeDen, rhsTypeDen);

    /* Ensure type sizes are cast only if necessary */
    bool matchTypeSize = true;
    if (ast->op == BinaryOp::Div)
    {
        if (rhsTypeDen->IsScalar())
            matchTypeSize = false;
    }
    else if (ast->op == BinaryOp::Mul)
    {
        if (lhsTypeDen->IsScalar() || rhsTypeDen->IsScalar())
            matchTypeSize = false;
    }

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
    /* Interlocked (atomic) intristics require actual buffer, and not their contents */
    Flags preVisitFlags = AllPreVisit;
    if (IsInterlockedIntristic(ast->intrinsic))
        preVisitFlags.Remove(ConvertImageAccess | ConvertTextureBracketOp);

    /* Convert mul intrinsic calls */
    if (ast->intrinsic == Intrinsic::Mul && ast->arguments.size() == 2)
    {
        /* Convert "mul" intrinsic to "dot" intrinsic for vector-vector multiplication */
        const auto& typeDenArg0 = ast->arguments[0]->GetTypeDenoter()->GetAliased();
        const auto& typeDenArg1 = ast->arguments[1]->GetTypeDenoter()->GetAliased();

        if (typeDenArg0.IsVector() && typeDenArg1.IsVector())
            ast->intrinsic = Intrinsic::Dot;
    }

    ConvertExpr(ast->prefixExpr, AllPreVisit);
    ConvertExprList(ast->arguments, preVisitFlags);
    {
        VISIT_DEFAULT(CallExpr);
    }
    ConvertExprList(ast->arguments, AllPostVisit);
    ConvertExpr(ast->prefixExpr, AllPostVisit);

    if (!IsInterlockedIntristic(ast->intrinsic))
    {
        ast->ForEachArgumentWithParameterType(
            [this](ExprPtr& funcArg, const TypeDenoter& paramTypeDen)
            {
                ConvertExprTargetType(funcArg, paramTypeDen);
            }
        );
    }
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

/* ----- Conversion ----- */

static TypeDenoterPtr MakeBufferAccessCallTypeDenoter(const DataType genericDataType)
{
    auto typeDenoter = std::make_shared<BaseTypeDenoter>();

    if (IsIntType(genericDataType))
        typeDenoter->dataType = DataType::Int4;
    else if (IsUIntType(genericDataType))
        typeDenoter->dataType = DataType::UInt4;
    else
        typeDenoter->dataType = DataType::Float4;

    return typeDenoter;
}

void ExprConverter::ConvertExpr(ExprPtr& expr, const Flags& flags)
{
    if (expr)
    {
        const auto enabled = Flags(flags & conversionFlags_);

        if (enabled(ConvertTextureIntrinsicVec4))
            ConvertExprTextureIntrinsicVec4(expr);

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

        if (enabled(ConvertMatrixSubscripts))
            ConvertExprMatrixSubscript(expr);

        if (enabled(ConvertUnaryExpr))
            ConvertExprIntoBracket(expr);

        if (enabled(ConvertTextureBracketOp))
            ConvertExprTextureBracketOp(expr);

        if (enabled(ConvertCompatibleStructs))
            ConvertExprCompatibleStruct(expr);
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

void ExprConverter::ConvertExprMatrixSubscript(ExprPtr& expr)
{
    if (auto objectExpr = expr->As<ObjectExpr>())
        ConvertExprMatrixSubscriptObject(expr, objectExpr);
}

void ExprConverter::ConvertExprMatrixSubscriptObject(ExprPtr& expr, ObjectExpr* objectExpr)
{
    if (!objectExpr->symbolRef && objectExpr->prefixExpr)
    {
        /* Get type denoter of prefix expression */
        const auto& prefixTypeDen = objectExpr->prefixExpr->GetTypeDenoter()->GetAliased();
        if (prefixTypeDen.IsMatrix())
        {
            auto prefixBaseTypeDen = prefixTypeDen.As<BaseTypeDenoter>();

            /* Get matrix subscript usage */
            const MatrixSubscriptUsage subscriptUsage(prefixBaseTypeDen->dataType, objectExpr->ident);

            if (IsScalarType(subscriptUsage.dataTypeOut) && !subscriptUsage.indices.empty())
            {
                /* Convert matrix subscript into array access */
                const auto subscriptIndex = subscriptUsage.indices.front();
                expr = ASTFactory::MakeArrayExpr(objectExpr->prefixExpr, { subscriptIndex.first, subscriptIndex.second });
            }
            else
            {
                /* Convert matrix subscript into function call to wrapper function */
                const auto wrapperIdent = ExprConverter::GetMatrixSubscriptWrapperIdent(nameMangling_, subscriptUsage); 
                expr = ASTFactory::MakeWrapperCallExpr(
                    wrapperIdent,
                    std::make_shared<BaseTypeDenoter>(subscriptUsage.dataTypeOut),
                    { objectExpr->prefixExpr }
                );
            }
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
conversion is wrong when the index expression contains a function call and the assignment is a compound!
e.g. "rwTex[getIndex()] += 2;" --> "imageStore(rwTex, getIndex(), imageLoad(rwTex, getIndex()) + 2)"
            ^~~~~~~~~~                                ^~~~~~~~~~                   ^~~~~~~~~~
results in two function calls! Thus the index expression must be moved into a separated statement.
*/
void ExprConverter::ConvertExprImageAccessArray(ExprPtr& expr, ArrayExpr* arrayExpr, AssignExpr* assignExpr)
{
    /* Fetch buffer type denoter from l-value prefix expression */
    auto prefixTypeDen = arrayExpr->prefixExpr->GetTypeDenoter()->GetSub();

    /* Get dimension of array type denoter (not of array expression) */
    std::size_t numDims = 0;
    if (auto arrayTypeDenoter = prefixTypeDen->As<ArrayTypeDenoter>())
    {
        numDims = arrayTypeDenoter->arrayDims.size();
        prefixTypeDen = arrayTypeDenoter->subTypeDenoter;
    }

    if (auto bufferTypeDen = prefixTypeDen->As<BufferTypeDenoter>())
    {
        if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
        {
            /* Is the buffer declaration a read/write texture? */
            const auto bufferType = bufferDecl->GetBufferType();
            if (IsRWImageBufferType(bufferType) && numDims < arrayExpr->NumIndices())
            {
                /* Get buffer type denoter from array indices of array access plus identifier */
                //TODO: not sure if the buffer type must be derived with 'GetSub(arrayExpr)' again here???
                #if 0
                auto bufferTypeDen = bufferDecl->GetTypeDenoter()->GetSub(arrayExpr);
                #endif
                if (auto genericBaseTypeDen = bufferTypeDen->genericTypeDenoter->As<BaseTypeDenoter>())
                {
                    arrayExpr->prefixExpr->flags << Expr::wasConverted;

                    /* Create a type denoter for the return value */
                    auto callTypeDen = MakeBufferAccessCallTypeDenoter(genericBaseTypeDen->dataType);

                    /* Make first argument expression */
                    ExprPtr arg0Expr;
                    if (numDims > 0)
                    {
                        std::vector<ExprPtr> arrayIndices;
                        for (std::size_t i = 0; i < numDims; ++i)
                            arrayIndices.push_back(arrayExpr->arrayIndices[i]);

                        arg0Expr = ASTFactory::MakeArrayExpr(arrayExpr->prefixExpr, std::move(arrayIndices));
                    }
                    else
                        arg0Expr = arrayExpr->prefixExpr;

                    /* Get second argument expression */
                    auto arg1Expr = arrayExpr->arrayIndices[numDims];

                    /* Cast to valid dimension */
                    auto textureDim = GetTextureDimFromExpr(arg0Expr.get(), expr.get());
                    ExprConverter::ConvertExprIfCastRequired(arg1Expr, VectorDataType(DataType::Int, textureDim), true);

                    ExprPtr exprOut;

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

                        /* Cast to valid 4D vector type */
                        if (IsIntType(genericBaseTypeDen->dataType))
                            ExprConverter::ConvertExprIfCastRequired(arg2Expr, DataType::Int4, true);
                        else if (IsUIntType(genericBaseTypeDen->dataType))
                            ExprConverter::ConvertExprIfCastRequired(arg2Expr, DataType::UInt4, true);
                        else
                            ExprConverter::ConvertExprIfCastRequired(arg2Expr, DataType::Float4, true);

                        /* Convert expression to intrinsic call */
                        exprOut = ASTFactory::MakeIntrinsicCallExpr(
                            Intrinsic::Image_Store, "imageStore", nullptr, { arg0Expr, arg1Expr, arg2Expr }
                        );
                    }
                    else
                    {
                        /* Convert expression to intrinsic call */
                        exprOut = ASTFactory::MakeIntrinsicCallExpr(
                            Intrinsic::Image_Load, "imageLoad", callTypeDen, { arg0Expr, arg1Expr }
                        );
                    }

                    if (arrayExpr->NumIndices() > numDims + 1)
                    {
                        /* Keep additional array indices (e.g. when "tex[idx][0]" is equivalent to "tex[idx].x") */
                        arrayExpr->prefixExpr = exprOut;
                        arrayExpr->arrayIndices.erase(arrayExpr->arrayIndices.begin(), arrayExpr->arrayIndices.begin() + numDims + 1);
                    }
                    else
                    {
                        /* Replace output expression with new expression (only at the end, otherwise 'arrayExpr' is a dangling pointer) */
                        expr = exprOut;
                    }
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
    auto prefixTypeDen = arrayExpr->prefixExpr->GetTypeDenoter()->GetSub();

    std::size_t numDims = 0;
    if (auto arrayTypeDenoter = prefixTypeDen->As<ArrayTypeDenoter>())
    {
        numDims = arrayTypeDenoter->arrayDims.size();
        prefixTypeDen = arrayTypeDenoter->subTypeDenoter;
    }

    if (auto bufferTypeDen = prefixTypeDen->As<BufferTypeDenoter>())
    {
        if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
        {
            /* Is the buffer declaration a sampler buffer? */
            const auto bufferType = bufferDecl->GetBufferType();
            if (bufferType == BufferType::Buffer && numDims < arrayExpr->NumIndices())
            {
                /* Get buffer type denoter from array indices of array access plus identifier */
                //TODO: not sure if the buffer type must be derived with 'GetSub(arrayExpr)' again here???
                #if 0
                auto bufferTypeDen = bufferDecl->GetTypeDenoter()->GetSub(arrayExpr);
                #endif
                if (auto genericBaseTypeDen = bufferTypeDen->genericTypeDenoter->As<BaseTypeDenoter>())
                {
                    /* Create a type denoter for the return value */
                    auto callTypeDen = MakeBufferAccessCallTypeDenoter(genericBaseTypeDen->dataType);

                    arrayExpr->prefixExpr->flags << Expr::wasConverted;

                    /* Get argument expression (last array index) */
                    auto argExpr = arrayExpr->arrayIndices.back();

                    /* Convert expression to intrinsic call */
                    auto callExpr = ASTFactory::MakeIntrinsicCallExpr(
                        Intrinsic::Texture_Load_1, "Load", callTypeDen, { argExpr }
                    );

                    if (numDims > 0)
                    {
                        std::vector<ExprPtr> arrayIndices;
                        for (std::size_t i = 0; i < numDims; ++i)
                            arrayIndices.push_back(arrayExpr->arrayIndices[i]);

                        callExpr->prefixExpr = ASTFactory::MakeArrayExpr(arrayExpr->prefixExpr, std::move(arrayIndices));
                    }
                    else
                        callExpr->prefixExpr = arrayExpr->prefixExpr;

                    expr = callExpr;
                }
            }
        }
    }
}

// Converts "log10" intrinsics (which are not supported in GLSL) to "log(x) / log(10)"
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
            ExprConverter::ConvertExprIfCastRequired(expr, targetTypeDen, matchTypeSize);

        if (auto initExpr = expr->As<InitializerExpr>())
        {
            /* Convert sub expressions for array type denoters */
            if (auto arrayTargetTypeDen = targetTypeDen.As<ArrayTypeDenoter>())
            {
                const auto& subTypeDen = arrayTargetTypeDen->subTypeDenoter->GetAliased();
                for (auto& expr : initExpr->exprs)
                    ConvertExprTargetType(expr, subTypeDen);
            }

            /* If enabled, convert initializer to a type constructor */
            if (conversionFlags_(ConvertInitializerToCtor))
                ConvertExprTargetTypeInitializer(expr, initExpr, targetTypeDen);
            else
                ConvertExprFormatInitializer(expr, initExpr, targetTypeDen);
        }
    }
}

void ExprConverter::ConvertExprTargetTypeInitializer(ExprPtr& expr, InitializerExpr* initExpr, const TypeDenoter& targetTypeDen)
{
    /* Convert initializer expression into type constructor */
    expr = ASTFactory::MakeTypeCtorCallExpr(targetTypeDen.Copy(), initExpr->exprs);
}

void ExprConverter::ConvertExprFormatInitializer(ExprPtr& expr, InitializerExpr* initExpr, const TypeDenoter& targetTypeDen)
{
    /* Re-format initializer expression */
    if (auto baseTargetTypeDen = targetTypeDen.As<BaseTypeDenoter>())
    {
        if (IsMatrixType(baseTargetTypeDen->dataType))
        {
            /* Get dimensions and number of entries from matrix type */
            const auto dims = MatrixTypeDim(baseTargetTypeDen->dataType);
            const auto numEntries = static_cast<std::size_t>(dims.first * dims.second);

            if (numEntries == initExpr->exprs.size())
            {
                /* Make vector type for matrix rows */
                auto rowTypeDenoter = std::make_shared<BaseTypeDenoter>();
                rowTypeDenoter->dataType = VectorDataType(BaseDataType(baseTargetTypeDen->dataType), dims.second);

                std::vector<ExprPtr> subInitExprs;

                std::size_t idx = 0;
                for (int row = 0; row < dims.first; ++row)
                {
                    /* Gather sub expressions from current matrix row */
                    std::vector<ExprPtr> rowInitExprs;
                    for (int col = 0; col < dims.second; ++col)
                        rowInitExprs.push_back(initExpr->exprs[idx++]);

                    /* Make new sub initializer expression with expected type denoter from matrix row */
                    auto rowInit = ASTFactory::MakeInitializerExpr(rowInitExprs);
                    {
                        rowInit->GetTypeDenoter(rowTypeDenoter.get());
                    }
                    subInitExprs.push_back(rowInit);
                }

                initExpr->exprs = std::move(subInitExprs);
            }
        }
    }
}

void ExprConverter::ConvertExprTextureBracketOp(ExprPtr& expr)
{
    if (!expr->flags(Expr::wasConverted) && expr->Type() == AST::Types::ArrayExpr)
    {
        if (auto arrayExpr = std::static_pointer_cast<ArrayExpr>(expr))
        {
            /*
            Split array expression if needed, e.g. when 'tex[1][idx][0]' is equivalent to 'tex[1][idx].r',
            i.e. the Texture Operator[] is not the last array index.
            */
            for (std::size_t i = 0; i + 1u < arrayExpr->NumIndices(); ++i)
            {
                auto typeDen = arrayExpr->prefixExpr->GetTypeDenoter()->GetSubArray(arrayExpr->NumIndices() - i - 1);
                if (auto bufferTypeDen = typeDen->As<BufferTypeDenoter>())
                {
                    if (!IsTextureBufferType(bufferTypeDen->bufferType))
                    {
                        /* Convert Texture Operator[] only for texture buffer types */
                        return;
                    }

                    if (i > 0)
                    {
                        arrayExpr = ASTFactory::MakeArrayExprSplit(arrayExpr, arrayExpr->NumIndices() - i);
                        ConvertExprTextureBracketOp(arrayExpr->prefixExpr);
                        expr = arrayExpr;
                    }
                    break;
                }
            }

            /* Has the prefix expression a Texture type denoter? */
            auto typeDen = arrayExpr->prefixExpr->GetTypeDenoter()->GetSubArray(arrayExpr->NumIndices() - 1);
            if (auto bufferTypeDen = typeDen->As<BufferTypeDenoter>())
            {
                if (!IsTextureBufferType(bufferTypeDen->bufferType))
                {
                    /* Convert Texture Operator[] only for texture buffer types */
                    return;
                }

                expr->flags << Expr::wasConverted;

                /* Make "Load" intrinsic call */
                auto callExpr = ASTFactory::MakeIntrinsicCallExpr(
                    Intrinsic::Texture_Load_1, "Load", nullptr,
                    { arrayExpr->arrayIndices.back() }
                );

                /* Use former expression as prefix for intrinsic call */
                ExprPtr prefixExpr;

                arrayExpr->arrayIndices.pop_back();

                if (arrayExpr->arrayIndices.empty())
                    callExpr->prefixExpr = arrayExpr->prefixExpr;
                else
                    callExpr->prefixExpr = expr;

                /* Reset type denoter of prefix expression (array index list has changed) */
                callExpr->prefixExpr->ResetTypeDenoter();

                /* Replace former expression with new intrinsic call */
                expr = callExpr;
            }
        }
    }
}

/*
Converts texture intrinsic calls that have a generic type lower than 4D-vectors.
E.g. "Texture2D<float2> tex; tex.Sample(...)" -> "tex.Sample(...).rg"
*/
void ExprConverter::ConvertExprTextureIntrinsicVec4(ExprPtr& expr)
{
    /* Is this a call expression? */
    if (auto callExpr = expr->As<CallExpr>())
    {
        /* Is this an intrinsic call? */
        const auto intrinsic = callExpr->intrinsic;
        if (intrinsic != Intrinsic::Undefined)
        {
            /* Is this a texture intrinsic? */
            bool isSampleIntrinsic = IsTextureSampleIntrinsic(intrinsic);
            if (IsTextureLoadIntrinsic(intrinsic) || isSampleIntrinsic || IsTextureGatherIntrisic(intrinsic))
            {
                /* Skip sample compare intrinsics as they return a scalar */
                if (!isSampleIntrinsic || !IsTextureCompareIntrinsic(intrinsic))
                {
                    /* Is the return type a base type denoter? */
                    const auto& typeDen = callExpr->GetTypeDenoter()->GetAliased();
                    if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
                    {
                        /* Has the return type less than 4-dimensions? */
                        const auto vecTypeDim = VectorTypeDim(baseTypeDen->dataType);
                        if (vecTypeDim >= 1 && vecTypeDim <= 3)
                        {
                            /* Append vector subscript to intrinsic call (use "rgba" instead of "xyzw" for color based purposes) */
                            const std::string vectorSubscript = "rgb";

                            expr = ASTFactory::MakeObjectExpr(
                                expr,
                                vectorSubscript.substr(0, static_cast<std::size_t>(vecTypeDim))
                            );
                        }
                    }
                }
            }
        }
    }
}

void ExprConverter::ConvertExprCompatibleStruct(ExprPtr& expr)
{
    /* Is this an object expression? */
    if (auto objectExpr = expr->As<ObjectExpr>())
    {
        if (objectExpr->prefixExpr && objectExpr->symbolRef)
        {
            /* Does the object expression refer to a variable? */
            if (auto varDecl = objectExpr->symbolRef->As<VarDecl>())
            {
                /* Has the prefix expression a struct type denoter? */
                const auto& prefixTypeDen = objectExpr->prefixExpr->GetTypeDenoter()->GetAliased();
                if (auto prefixStructTypeDen = prefixTypeDen.As<StructTypeDenoter>())
                {
                    if (auto structDecl = prefixStructTypeDen->structDeclRef)
                    {
                        /* Has the struct a compatible struct? */
                        if (auto compatStruct = structDecl->compatibleStructRef)
                        {
                            /* Map original variable to compatible variable by index */
                            auto idx = structDecl->MemberVarToIndex(varDecl);
                            if (auto compatVarDecl = compatStruct->IndexToMemberVar(idx))
                            {
                                /* Replace identifier by respective member of the type-compatible struct */
                                objectExpr->ReplaceSymbol(compatVarDecl);
                            }
                        }
                    }
                }
            }
        }
    }
}


} // /namespace Xsc



// ================================================================================
