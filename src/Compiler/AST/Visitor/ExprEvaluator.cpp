/*
 * ExprEvaluator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ExprEvaluator.h"
#include "AST.h"
#include "Helper.h"
#include "Exception.h"
#include "ReportIdents.h"
#include <sstream>
#include <string>


namespace Xsc
{


Variant ExprEvaluator::Evaluate(Expr& expr, const OnObjectExprCallback& onObjectExprCallback)
{
    /* Reset internal state (with exceptions) */
    throwOnFailure_ = true;
    abort_          = false;

    SetObjectExprCallback(onObjectExprCallback);

    /* Visit expression AST */
    Visit(&expr);

    return Pop();
}

Variant ExprEvaluator::EvaluateOrDefault(Expr& expr, const Variant& defaultValue, const OnObjectExprCallback& onObjectExprCallback)
{
    /* Reset internal state (without exceptions) */
    throwOnFailure_ = false;
    abort_          = false;

    SetObjectExprCallback(onObjectExprCallback);

    /* Visit expression AST */
    Visit(&expr);

    if (auto value = Pop())
        return value;
    else
        return defaultValue;
}

void ExprEvaluator::Abort()
{
    abort_ = true;
}


/*
 * ======= Private: =======
 */

[[noreturn]]
static void IllegalExpr(const std::string& exprName, const AST* ast = nullptr)
{
    RuntimeErr(R_IllegalExprInConstExpr(exprName), ast);
}

void ExprEvaluator::Push(const Variant& v)
{
    if (!abort_)
        variantStack_.push(v);
}

Variant ExprEvaluator::Pop()
{
    /* Return dummy variant if process has been canceld */
    if (!abort_)
    {
        if (!variantStack_.empty())
        {
            /* Pop variant from stack */
            auto v = variantStack_.top();
            variantStack_.pop();
            return v;
        }
        else if (throwOnFailure_)
            RuntimeErr(R_StackUnderflow(R_ExprEvaluator));
        else
            Abort();
    }
    return {};
}

void ExprEvaluator::SetObjectExprCallback(const OnObjectExprCallback& callback)
{
    if (callback)
    {
        /* Set custom object expression callback */
        onObjectExprCallback_ = callback;
    }
    else
    {
        /* Default callback returns the initializer value of a variable */
        onObjectExprCallback_ = [](ObjectExpr* expr) -> Variant
        {
            /* Fetch variable from expression and return its initializer value */
            if (auto varDecl = expr->FetchVarDecl())
            {
                if (varDecl->HasStaticConstInitializer())
                    return varDecl->initializerValue;
            }
            return {};
        };
    }
}

Variant ExprEvaluator::EvaluateBinaryOp(const BinaryExpr* ast, Variant lhs, Variant rhs)
{
    switch (ast->op)
    {
        case BinaryOp::Undefined:
            if (throwOnFailure_)
                IllegalExpr(R_BinaryOp, ast);
            else
                Abort();
            break;

        case BinaryOp::LogicalAnd:
            return (lhs.ToBool() && rhs.ToBool());

        case BinaryOp::LogicalOr:
            return (lhs.ToBool() || rhs.ToBool());

        case BinaryOp::Or:
            return (lhs | rhs);

        case BinaryOp::Xor:
            return (lhs ^ rhs);

        case BinaryOp::And:
            return (lhs & rhs);

        case BinaryOp::LShift:
            return (lhs << rhs);

        case BinaryOp::RShift:
            return (lhs >> rhs);

        case BinaryOp::Add:
            return (lhs + rhs);

        case BinaryOp::Sub:
            return (lhs - rhs);

        case BinaryOp::Mul:
            return (lhs * rhs);

        case BinaryOp::Div:
            if (lhs.Type() == Variant::Types::Int && rhs.Int() == 0)
            {
                if (throwOnFailure_)
                    IllegalExpr(R_DivisionByZero, ast);
                else
                    Abort();
                break;
            }
            return (lhs / rhs);

        case BinaryOp::Mod:
            if (lhs.Type() == Variant::Types::Int && rhs.Int() == 0)
            {
                if (throwOnFailure_)
                    IllegalExpr(R_DivisionByZero, ast);
                else
                    Abort();
                break;
            }
            return (lhs % rhs);

        case BinaryOp::Equal:
            return (lhs == rhs);

        case BinaryOp::NotEqual:
            return (lhs != rhs);

        case BinaryOp::Less:
            return (lhs < rhs);

        case BinaryOp::Greater:
            return (lhs > rhs);

        case BinaryOp::LessEqual:
            return (lhs <= rhs);

        case BinaryOp::GreaterEqual:
            return (lhs >= rhs);
    }
    return {};
}

Variant ExprEvaluator::EvaluateUnaryOp(const UnaryExpr* ast, Variant rhs)
{
    switch (ast->op)
    {
        case UnaryOp::Undefined:
            if (throwOnFailure_)
                IllegalExpr(R_UnaryOp, ast);
            else
                Abort();
            break;

        case UnaryOp::LogicalNot:
            return (!rhs.ToBool());

        case UnaryOp::Not:
            return (~rhs);

        case UnaryOp::Nop:
            return (rhs);

        case UnaryOp::Negate:
            return (-rhs);

        case UnaryOp::Inc:
            return (++rhs);

        case UnaryOp::Dec:
            return (--rhs);
    }
    return {};
}

/* --- Expressions --- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ExprEvaluator::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(NullExpr)
{
    if (throwOnFailure_)
        IllegalExpr(R_DynamicArrayDim, ast);
    else
        Abort();
}

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    /* Only visit first sub-expression (when used as condExpr) */
    Visit(ast->exprs.front());
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    switch (ast->dataType)
    {
        case DataType::Bool:
        {
            if (ast->value == "true")
                Push(true);
            else if (ast->value == "false")
                Push(false);
            else if (throwOnFailure_)
                IllegalExpr(R_BoolLiteralValue(ast->value), ast);
            else
                Abort();
        }
        break;

        case DataType::Int:
        {
            Push(FromStringOrDefault<long long>(ast->value));
        }
        break;

        case DataType::UInt:
        {
            Push(static_cast<Variant::IntType>(FromStringOrDefault<unsigned long>(ast->value)));
        }
        break;

        case DataType::Half:
        case DataType::Float:
        case DataType::Double:
        {
            Push(FromStringOrDefault<double>(ast->value));
        }
        break;

        default:
        {
            if (throwOnFailure_)
                IllegalExpr(R_LiteralType(DataTypeToString(ast->dataType)), ast);
            else
                Abort();
        }
        break;
    }
}

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    if (throwOnFailure_)
        IllegalExpr(R_TypeSpecifier, ast);
    else
        Abort();
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Visit(ast->condExpr);
    if (auto cond = Pop())
    {
        if (cond.ToBool())
            Visit(ast->thenExpr);
        else
            Visit(ast->elseExpr);
    }
}

// EXPR OP EXPR
IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Visit(ast->lhsExpr);

    if (auto lhs = Pop())
    {
        Visit(ast->rhsExpr);
        if (auto rhs = Pop())
        {
            if (auto result = EvaluateBinaryOp(ast, lhs, rhs))
            {
                Push(result);
                return;
            }
        }
    }

    Push({});
}

// OP EXPR
IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Visit(ast->expr);

    if (auto rhs = Pop())
    {
        if (auto result = EvaluateUnaryOp(ast, rhs))
        {
            Push(result);
            return;
        }
    }

    Push({});
}

// EXPR OP
IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);

    if (auto lhs = Pop())
    {
        switch (ast->op)
        {
            case UnaryOp::Inc:
            case UnaryOp::Dec:
                /* Only return original value (post inc/dec will return the value BEFORE the operation) */
                Push(lhs);
                break;
            default:
                if (throwOnFailure_)
                    IllegalExpr(R_UnaryOp(UnaryOpToString(ast->op)), ast);
                else
                    Abort();
                break;
        }
    }
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    if (throwOnFailure_)
        IllegalExpr(R_FunctionCall, ast);
    else
        Abort();
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    if (throwOnFailure_)
        IllegalExpr(R_VarAssignment, ast);
    else
        Abort();
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    Push(onObjectExprCallback_(ast));
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    /* Evaluate prefix expression */
    Visit(ast->prefixExpr);

    if (auto value = Pop())
    {
        if (value.IsArray())
        {
            /* Find sub variant by array index */
            for (std::size_t i = 0, n = ast->arrayIndices.size(); i < n; ++i)
            {
                /* Evaluate array index expression */
                Visit(ast->arrayIndices[i]);

                if (auto arrayIdx = Pop())
                {
                    if (value.IsArray())
                    {
                        auto arrayIdxInt = static_cast<std::size_t>(arrayIdx.ToInt());
                        if (auto subValue = value.ArraySub(arrayIdxInt))
                        {
                            /* Continue evaluation with sub value */
                            value = std::move(subValue);
                        }
                    }
                    else
                        break;
                }
                else
                {
                    Abort();
                    return;
                }
            }

            Push(value);
            return;
        }
    }

    Abort();
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->expr);

    if (auto value = Pop())
    {
        if (auto baseTypeDen = ast->typeSpecifier->GetTypeDenoter()->As<BaseTypeDenoter>())
        {
            switch (baseTypeDen->dataType)
            {
                case DataType::Bool:
                {
                    Push(value.ToBool());
                }
                break;

                case DataType::Int:
                case DataType::UInt:
                {
                    Push(value.ToInt());
                }
                break;

                case DataType::Half:
                case DataType::Float:
                case DataType::Double:
                {
                    Push(value.ToReal());
                }
                break;

                default:
                {
                    if (throwOnFailure_)
                        IllegalExpr(R_TypeCast(DataTypeToString(baseTypeDen->dataType)), ast);
                    else
                        Abort();
                }
                break;
            }
        }
        else if (throwOnFailure_)
            IllegalExpr(R_TypeCast, ast);
        else
            Abort();
    }
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    std::vector<Variant> subValues;

    for (const auto& expr : ast->exprs)
    {
        Visit(expr);

        if (auto value = Pop())
        {
            /* Append variant of sub expression to array sub values */
            subValues.push_back(value);
        }
        else
        {
            /* Cancel evaluation */
            Abort();
            return;
        }
    }

    /* Push array variant with sub values */
    Push(Variant(std::move(subValues)));
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
