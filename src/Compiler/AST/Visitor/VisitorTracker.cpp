/*
 * VisitorTracker.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VisitorTracker.h"
#include "AST.h"
#include "ReportIdents.h"


namespace Xsc
{


/*
 * ======= Protected: =======
 */

/* ----- Function declaration tracker ----- */

void VisitorTracker::PushFunctionDecl(FunctionDecl* ast)
{
    funcDeclStack_.push(ast);
    if (ast->flags(FunctionDecl::isEntryPoint))
        stackLevelOfEntryPoint_ = funcDeclStack_.size();
    else if (ast->flags(FunctionDecl::isSecondaryEntryPoint))
        stackLevelOf2ndEntryPoint_ = funcDeclStack_.size();
}

void VisitorTracker::PopFunctionDecl()
{
    if (!funcDeclStack_.empty())
    {
        if (stackLevelOfEntryPoint_ == funcDeclStack_.size())
            stackLevelOfEntryPoint_ = ~0;
        if (stackLevelOf2ndEntryPoint_ == funcDeclStack_.size())
            stackLevelOf2ndEntryPoint_ = ~0;
        funcDeclStack_.pop();
    }
    else
        throw std::underflow_error(R_FuncDeclStackUnderflow);
}

bool VisitorTracker::InsideFunctionDecl() const
{
    return (!funcDeclStack_.empty());
}

bool VisitorTracker::InsideEntryPoint() const
{
    return (funcDeclStack_.size() >= stackLevelOfEntryPoint_);
}

bool VisitorTracker::InsideSecondaryEntryPoint() const
{
    return (funcDeclStack_.size() >= stackLevelOf2ndEntryPoint_);
}

FunctionDecl* VisitorTracker::ActiveFunctionDecl() const
{
    return (funcDeclStack_.empty() ? nullptr : funcDeclStack_.top());
}

StructDecl* VisitorTracker::ActiveFunctionStructDecl() const
{
    if (auto funcDecl = ActiveFunctionDecl())
        return funcDecl->structDeclRef;
    else
        return nullptr;
}

/* ----- Call expression tracker ----- */

void VisitorTracker::PushCallExpr(CallExpr* ast)
{
    callExprStack_.push(ast);
}

void VisitorTracker::PopCallExpr()
{
    if (!callExprStack_.empty())
        callExprStack_.pop();
    else
        throw std::underflow_error(R_CallExprStackUnderflow);
}

CallExpr* VisitorTracker::ActiveCallExpr() const
{
    return (callExprStack_.empty() ? nullptr : callExprStack_.top());
}

/* ----- L-value expression tracker ----- */

void VisitorTracker::PushLValueExpr(Expr* expr)
{
    lvalueExprStack_.push(expr);
}

void VisitorTracker::PopLValueExpr()
{
    if (!lvalueExprStack_.empty())
        lvalueExprStack_.pop();
    else
        throw std::runtime_error(R_LValueExprStackUnderflow);
}

Expr* VisitorTracker::ActiveLValueExpr() const
{
    return (lvalueExprStack_.empty() ? nullptr : lvalueExprStack_.top());
}

/* ----- Structure declaration tracker ----- */

void VisitorTracker::PushStructDecl(StructDecl* ast)
{
    structDeclStack_.push_back(ast);
}

void VisitorTracker::PopStructDecl()
{
    if (!structDeclStack_.empty())
        structDeclStack_.pop_back();
    else
        throw std::underflow_error(R_StructDeclStackUnderflow);
}

bool VisitorTracker::InsideStructDecl() const
{
    return (!structDeclStack_.empty());
}

StructDecl* VisitorTracker::ActiveStructDecl() const
{
    return (structDeclStack_.empty() ? nullptr : structDeclStack_.back());
}

/* ----- Structure declaration tracker ----- */

void VisitorTracker::PushUniformBufferDecl(UniformBufferDecl* ast)
{
    uniformBufferDeclStack_.push_back(ast);
}

void VisitorTracker::PopUniformBufferDecl()
{
    if (!uniformBufferDeclStack_.empty())
        uniformBufferDeclStack_.pop_back();
    else
        throw std::underflow_error(R_UniformBufferDeclStackUnderflow);
}

bool VisitorTracker::InsideUniformBufferDecl() const
{
    return (!uniformBufferDeclStack_.empty());
}


} // /namespace Xsc



// ================================================================================