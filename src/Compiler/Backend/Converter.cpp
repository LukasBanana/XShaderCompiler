/*
 * Converter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Converter.h"
#include "AST.h"
#include "ASTFactory.h"
#include "Helper.h"
#include "ReportIdents.h"
#include <algorithm>


namespace Xsc
{


bool Converter::ConvertAST(Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    program_        = (&program);
    nameMangling_   = outputDesc.nameMangling;

    ConvertASTPrimary(program, inputDesc, outputDesc);

    return true;
}


/*
 * ======= Protected: =======
 */

/* ----- Scope functions ----- */

void Converter::OpenScope()
{
    symTable_.OpenScope();
}

void Converter::CloseScope()
{
    symTable_.CloseScope();
}

void Converter::Register(const std::string& ident)
{
    symTable_.Register(ident, true);
}

bool Converter::Fetch(const std::string& ident) const
{
    return symTable_.Fetch(ident);
}

bool Converter::FetchFromCurrentScope(const std::string& ident) const
{
    return symTable_.FetchFromCurrentScope(ident);
}

/* ----- Self parameter ----- */

void Converter::PushSelfParameter(VarDecl* parameter)
{
    selfParamStack_.push_back(parameter);
}

void Converter::PopSelfParameter()
{
    if (selfParamStack_.empty())
        throw std::underflow_error(R_SelfParamStackUnderflow);
    else
        return selfParamStack_.pop_back();
}

VarDecl* Converter::ActiveSelfParameter() const
{
    return (selfParamStack_.empty() ? nullptr : selfParamStack_.back());
}

/* ----- Name mangling ----- */

void Converter::RenameIdent(Identifier& ident)
{
    ident.AppendPrefix(nameMangling_.temporaryPrefix);
}

void Converter::RenameIdentObfuscated(Identifier& ident)
{
    /* Set identifier to "_{ObfuscatinoCounter}", and increase the counter */
    ident = "_" + std::to_string(obfuscationCounter_);
    ++obfuscationCounter_;
}

void Converter::RenameIdentOf(Decl* declObj)
{
    RenameIdent(declObj->ident);
}

void Converter::RenameIdentOfInOutVarDecls(const std::vector<VarDecl*>& varDecls, bool input, bool useSemanticOnly)
{
    for (auto varDecl : varDecls)
    {
        if (useSemanticOnly)
            varDecl->ident = varDecl->semantic.ToString();
        else if (input)
            varDecl->ident = nameMangling_.inputPrefix + varDecl->semantic.ToString();
        else
            varDecl->ident = nameMangling_.outputPrefix + varDecl->semantic.ToString();
    }
}

void Converter::LabelAnonymousDecl(Decl* declObj)
{
    if (declObj && declObj->IsAnonymous())
    {
        /* Set identifier to "{TempPrefix}_anonym{AnonymousCounter}", and increase the counter */
        declObj->ident = nameMangling_.temporaryPrefix + "anonym" + std::to_string(anonymCounter_);
        ++anonymCounter_;
    }
}

/* ----- Code injection ----- */

void Converter::VisitScopedStmt(StmtPtr& stmt, void* args)
{
    VisitScopedStmtsFromHandler({ stmt }, args);
}

void Converter::VisitScopedStmtList(std::vector<StmtPtr>& stmtList, void* args)
{
    VisitScopedStmtsFromHandler({ stmtList }, args);
}

void Converter::InsertStmtBefore(const StmtPtr& stmt, bool globalScope)
{
    if (globalScope)
        stmtScopeHandlerGlobalRef_->InsertStmtBefore(stmt);
    else
        ActiveStmtScopeHandler().InsertStmtBefore(stmt);
}

void Converter::InsertStmtAfter(const StmtPtr& stmt, bool globalScope)
{
    if (globalScope)
        stmtScopeHandlerGlobalRef_->InsertStmtAfter(stmt);
    else
        ActiveStmtScopeHandler().InsertStmtAfter(stmt);
}

void Converter::MoveNestedStructDecls(std::vector<StmtPtr>& localStmts, bool globalScope)
{
    for (auto it = localStmts.begin(); it != localStmts.end();)
    {
        if (auto varDeclStmt = (*it)->As<VarDeclStmt>())
        {
            /* Does the variable declaration has a nested structure declaration? */
            if (varDeclStmt->typeSpecifier->structDecl != nullptr)
            {
                /* Make global structure declaration statement */
                auto structDeclStmt = ASTFactory::MakeStructDeclStmt(
                    ExchangeWithNull(varDeclStmt->typeSpecifier->structDecl)
                );

                /* Insert the new statement */
                InsertStmtBefore(structDeclStmt, globalScope);
            }
        }
        else if (auto basicDeclStmt = (*it)->As<BasicDeclStmt>())
        {
            if (basicDeclStmt->declObject->Type() == AST::Types::StructDecl)
            {
                /* Move entire statement to the upper scope */
                InsertStmtBefore(*it, globalScope);

                /* Remove statement from the list */
                it = localStmts.erase(it);

                continue;
            }
        }

        /* Next statement */
        ++it;
    }
}

/* ----- Misc ----- */

bool Converter::IsGlobalInOutVarDecl(VarDecl* varDecl) const
{
    if (varDecl)
    {
        /* Is this variable a global input/output variable? */
        auto entryPoint = program_->entryPointRef;
        return (entryPoint->inputSemantics.Contains(varDecl) || entryPoint->outputSemantics.Contains(varDecl));
    }
    return false;
}

bool Converter::IsSamplerStateTypeDenoter(const TypeDenoterPtr& typeDenoter) const
{
    if (typeDenoter)
    {
        if (auto samplerTypeDen = typeDenoter->GetAliased().As<SamplerTypeDenoter>())
        {
            /* Is the sampler type a sampler-state type? */
            return IsSamplerStateType(samplerTypeDen->samplerType);
        }
    }
    return false;
}

void Converter::RemoveDeadCode(std::vector<StmtPtr>& stmts)
{
    for (auto it = stmts.begin(); it != stmts.end();)
    {
        if ((*it)->flags(AST::isDeadCode))
            it = stmts.erase(it);
        else
            ++it;
    }
}

std::string Converter::MakeTempVarIdent()
{
    /* Return identifier for temporary variable, and increase counter of temporary variables */
    return nameMangling_.temporaryPrefix + "temp" + std::to_string(tempVarCounter_++);
}


/*
 * ======= Private: =======
 */

void Converter::VisitScopedStmtsFromHandler(const StmtScopeHandler& handler, void* args)
{
    /* Push scope handler onto stack */
    stmtScopeHandlerStack_.push(handler);

    if (!stmtScopeHandlerGlobalRef_)
        stmtScopeHandlerGlobalRef_ = &(stmtScopeHandlerStack_.top());

    /* Use active scope handler */
    auto& activeHandler = ActiveStmtScopeHandler();

    /* Visit all statements from the scope handler */
    while (auto stmt = activeHandler.Next())
        Visit(stmt, args);

    /* Pop scope handler from stack */
    stmtScopeHandlerStack_.pop();

    if (stmtScopeHandlerStack_.empty())
        stmtScopeHandlerGlobalRef_ = nullptr;
}

Converter::StmtScopeHandler& Converter::ActiveStmtScopeHandler()
{
    if (stmtScopeHandlerStack_.empty())
        throw std::underflow_error(R_NoActiveStmtScopeHandler);
    else
        return stmtScopeHandlerStack_.top();
}


/*
 * StmtScopeHandler class
 */

Converter::StmtScopeHandler::StmtScopeHandler(StmtPtr& stmt) :
    stmt_ { &stmt }
{
}

Converter::StmtScopeHandler::StmtScopeHandler(std::vector<StmtPtr>& stmts) :
    stmtList_ { &stmts }
{
}

Stmt* Converter::StmtScopeHandler::Next()
{
    if (stmtList_)
    {
        if (idx_ < stmtList_->size())
        {
            /* Return statement from the list, and increase index */
            return stmtList_->at(idx_++).get();
        }
    }
    else if (stmt_)
    {
        if (idx_ == 0)
        {
            /* Only return the single statement once */
            ++idx_;
            return stmt_->get();
        }
    }
    return nullptr;
}

void Converter::StmtScopeHandler::InsertStmtBefore(const StmtPtr& stmt)
{
    EnsureStmtList();

    if (idx_ > 0)
        InsertStmtAt(stmt, idx_ - 1);
    else
        InsertStmtAt(stmt, idx_);

    ++idx_;
}

void Converter::StmtScopeHandler::InsertStmtAfter(const StmtPtr& stmt)
{
    EnsureStmtList();
    InsertStmtAt(stmt, idx_);
}

void Converter::StmtScopeHandler::EnsureStmtList()
{
    if (!stmtList_)
    {
        if (!stmt_)
            throw std::runtime_error(R_MissingScopedStmtRef);

        /* Make new code block statement to replace the single statement with */
        auto singleStmt    = *stmt_;
        auto scopeStmt = ASTFactory::MakeScopeStmt(singleStmt);

        /* Set reference to statement list of the code block */
        stmtList_ = &(scopeStmt->codeBlock->stmts);

        /* Replace original single statement with code block statement */
        *stmt_ = scopeStmt;
    }
}

void Converter::StmtScopeHandler::InsertStmtAt(const StmtPtr& stmt, std::size_t pos)
{
    if (stmtList_)
    {
        if (pos < stmtList_->size())
            stmtList_->insert(stmtList_->begin() + pos, stmt);
        else
            stmtList_->push_back(stmt);
    }
}


} // /namespace Xsc



// ================================================================================
