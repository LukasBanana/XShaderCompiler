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

void VisitorTracker::PushFunctionDecl(FunctionDecl* funcDecl)
{
    funcDeclStack_.push(funcDecl);
    if (funcDecl->flags(FunctionDecl::isEntryPoint))
        stackLevelOfEntryPoint_ = funcDeclStack_.size();
    else if (funcDecl->flags(FunctionDecl::isSecondaryEntryPoint))
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

void VisitorTracker::PushCallExpr(CallExpr* callExpr)
{
    callExprStack_.push(callExpr);
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

void VisitorTracker::PushStructDecl(StructDecl* structDecl)
{
    structDeclStack_.push_back(structDecl);
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

void VisitorTracker::PushUniformBufferDecl(UniformBufferDecl* uniformBufferDecl)
{
    uniformBufferDeclStack_.push_back(uniformBufferDecl);
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

/* ----- Variable declaration statement tracker ----- */

void VisitorTracker::PushVarDeclStmnt(VarDeclStmnt* varDeclStmnt)
{
    varDeclStmntStack_.push(varDeclStmnt);
}

void VisitorTracker::PopVarDeclStmnt()
{
    if (!varDeclStmntStack_.empty())
        varDeclStmntStack_.pop();
    else
        throw std::underflow_error(R_VarDeclStmntStackUnderflow);
}

bool VisitorTracker::InsideVarDeclStmnt() const
{
    return (!varDeclStmntStack_.empty());
}

VarDeclStmnt* VisitorTracker::ActiveVarDeclStmnt() const
{
    return (varDeclStmntStack_.empty() ? nullptr : varDeclStmntStack_.top());
}


} // /namespace Xsc



// ================================================================================