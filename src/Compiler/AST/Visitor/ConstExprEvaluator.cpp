/*
 * ConstExprEvaluator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ConstExprEvaluator.h"
#include "AST.h"
#include "Helper.h"
#include "Exception.h"
#include <sstream>


namespace Xsc
{


Variant ConstExprEvaluator::EvaluateExpr(Expr& ast, const OnVarAccessCallback& onVarAccessCallback)
{
    onVarAccessCallback_ = (onVarAccessCallback ? onVarAccessCallback : [](VarAccessExpr* ast) { return Variant(Variant::IntType(0)); });
    Visit(&ast);
    return Pop();
}


/*
 * ======= Private: =======
 */

[[noreturn]]
static void IllegalExpr(const std::string& exprName, const AST* ast = nullptr)
{
    RuntimeErr("illegal " + exprName + " in constant expression", ast);
}

void ConstExprEvaluator::Push(const Variant& v)
{
    variantStack_.push(v);
}

Variant ConstExprEvaluator::Pop()
{
    if (variantStack_.empty())
        throw std::runtime_error("stack underflow in expression evaluator");
    auto v = variantStack_.top();
    variantStack_.pop();
    return v;
}

/* --- Expressions --- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void ConstExprEvaluator::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(NullExpr)
{
    IllegalExpr("dynamic array dimension", ast);
}

IMPLEMENT_VISIT_PROC(ListExpr)
{
    /* Only visit first sub-expression (when used as condExpr) */
    Visit(ast->firstExpr);
    //Visit(ast->nextExpr);
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
            else
                IllegalExpr("boolean literal value '" + ast->value + "'", ast);
        }
        break;

        case DataType::Int:
        {
            Push(FromString<Variant::IntType>(ast->value));
        }
        break;

        case DataType::UInt:
        {
            Push(Variant::IntType(FromString<unsigned int>(ast->value)));
        }
        break;

        case DataType::Half:
        case DataType::Float:
        case DataType::Double:
        {
            Push(FromString<Variant::RealType>(ast->value));
        }
        break;

        default:
        {
            IllegalExpr("literal type '" + DataTypeToString(ast->dataType) + "'", ast);
        }
        break;
    }
}

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
    IllegalExpr("type specifier", ast);
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Visit(ast->condExpr);
    auto cond = Pop();

    if (cond.ToBool())
        Visit(ast->thenExpr);
    else
        Visit(ast->elseExpr);
}

// EXPR OP EXPR
IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Visit(ast->lhsExpr);
    Visit(ast->rhsExpr);

    auto rhs = Pop();
    auto lhs = Pop();

    switch (ast->op)
    {
        case BinaryOp::Undefined:
            IllegalExpr("binary operator", ast);
            break;
        case BinaryOp::LogicalAnd:
            Push(lhs.ToBool() && rhs.ToBool());
            break;
        case BinaryOp::LogicalOr:
            Push(lhs.ToBool() || rhs.ToBool());
            break;
        case BinaryOp::Or:
            Push(lhs | rhs);
            break;
        case BinaryOp::Xor:
            Push(lhs ^ rhs);
            break;
        case BinaryOp::And:
            Push(lhs & rhs);
            break;
        case BinaryOp::LShift:
            Push(lhs << rhs);
            break;
        case BinaryOp::RShift:
            Push(lhs >> rhs);
            break;
        case BinaryOp::Add:
            Push(lhs + rhs);
            break;
        case BinaryOp::Sub:
            Push(lhs - rhs);
            break;
        case BinaryOp::Mul:
            Push(lhs * rhs);
            break;
        case BinaryOp::Div:
            if (lhs.Type() == Variant::Types::Int && rhs.Int() == 0)
                IllegalExpr("division by zero", ast);
            Push(lhs / rhs);
            break;
        case BinaryOp::Mod:
            if (lhs.Type() == Variant::Types::Int && rhs.Int() == 0)
                IllegalExpr("division by zero", ast);
            Push(lhs % rhs);
            break;
        case BinaryOp::Equal:
            Push(lhs == rhs);
            break;
        case BinaryOp::NotEqual:
            Push(lhs != rhs);
            break;
        case BinaryOp::Less:
            Push(lhs < rhs);
            break;
        case BinaryOp::Greater:
            Push(lhs > rhs);
            break;
        case BinaryOp::LessEqual:
            Push(lhs <= rhs);
            break;
        case BinaryOp::GreaterEqual:
            Push(lhs >= rhs);
            break;
    }
}

// OP EXPR
IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Visit(ast->expr);

    auto rhs = Pop();

    switch (ast->op)
    {
        case UnaryOp::Undefined:
            IllegalExpr("unary operator", ast);
            break;
        case UnaryOp::LogicalNot:
            Push(!rhs.ToBool());
            break;
        case UnaryOp::Not:
            Push(~rhs);
            break;
        case UnaryOp::Nop:
            Push(rhs);
            break;
        case UnaryOp::Negate:
            Push(-rhs);
            break;
        case UnaryOp::Inc:
            Push(++rhs);
            break;
        case UnaryOp::Dec:
            Push(--rhs);
            break;
    }
}

// EXPR OP
IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);

    auto lhs = Pop();

    switch (ast->op)
    {
        case UnaryOp::Inc:
        case UnaryOp::Dec:
            /* Only return original value (post inc/dec will return the value BEFORE the operation) */
            Push(lhs);
            break;
        default:
            IllegalExpr("unary operator '" + UnaryOpToString(ast->op) + "'", ast);
            break;
    }
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    IllegalExpr("function call", ast);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(SuffixExpr)
{
    //TODO: incomplete!
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(ArrayAccessExpr)
{
    //TODO: incomplete!
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->expr);

    auto value = Pop();

    if (auto baseTypeDen = ast->typeExpr->GetTypeDenoter()->As<BaseTypeDenoter>())
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
                IllegalExpr("type cast '" + DataTypeToString(baseTypeDen->dataType) + "'", ast);
            }
            break;
        }
    }
    else
        IllegalExpr("type cast", ast);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    Push(onVarAccessCallback_(ast));
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    IllegalExpr("initializer list", ast);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
