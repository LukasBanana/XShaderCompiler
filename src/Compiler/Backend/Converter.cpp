/*
 * Converter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Converter.h"
#include "AST.h"
#include "ASTFactory.h"
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

void Converter::VisitScopedStmnt(StmntPtr& stmnt)
{
    VisitScopedStmntsFromHandler({ stmnt });
}

void Converter::VisitScopedStmntList(std::vector<StmntPtr>& stmnts)
{
    VisitScopedStmntsFromHandler({ stmnts });
}

void Converter::InsertStmntBefore(const StmntPtr& stmnt)
{
    ActiveStmntScopeHandler().InsertStmntBefore(stmnt);
}

void Converter::InsertStmntAfter(const StmntPtr& stmnt)
{
    ActiveStmntScopeHandler().InsertStmntAfter(stmnt);
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

void Converter::RemoveDeadCode(std::vector<StmntPtr>& stmnts)
{
    for (auto it = stmnts.begin(); it != stmnts.end();)
    {
        if ((*it)->flags(AST::isDeadCode))
            it = stmnts.erase(it);
        else
            ++it;
    }
}


/*
 * ======= Private: =======
 */

void Converter::VisitScopedStmntsFromHandler(const StmntScopeHandler& handler)
{
    stmntScopeHandlerStack_.push(handler);
    {
        /* Use active scope handler */
        auto& activeHandler = ActiveStmntScopeHandler();

        /* Visit all statement from the scope handler */
        while (auto stmnt = activeHandler.Next())
            Visit(stmnt);
    }
    stmntScopeHandlerStack_.pop();
}

Converter::StmntScopeHandler& Converter::ActiveStmntScopeHandler()
{
    if (stmntScopeHandlerStack_.empty())
        throw std::underflow_error(R_NoActiveStmntScopeHandler);
    else
        return stmntScopeHandlerStack_.top();
}


/*
 * StmntScopeHandler class
 */

Converter::StmntScopeHandler::StmntScopeHandler(StmntPtr& stmnt) :
    stmnt_ { &stmnt }
{
}

Converter::StmntScopeHandler::StmntScopeHandler(std::vector<StmntPtr>& stmnts) :
    stmntList_ { &stmnts }
{
}

Stmnt* Converter::StmntScopeHandler::Next()
{
    if (stmntList_)
    {
        if (idx_ < stmntList_->size())
        {
            /* Return statement from the list, and increase index */
            return stmntList_->at(idx_++).get();
        }
    }
    else if (stmnt_)
    {
        if (idx_ == 0)
        {
            /* Only return the single statement once */
            ++idx_;
            return stmnt_->get();
        }
    }
    return nullptr;
}

void Converter::StmntScopeHandler::InsertStmntBefore(const StmntPtr& stmnt)
{
    EnsureStmntList();

    //TODO...
}

void Converter::StmntScopeHandler::InsertStmntAfter(const StmntPtr& stmnt)
{
    EnsureStmntList();

    //TODO...
}

void Converter::StmntScopeHandler::EnsureStmntList()
{
    if (stmnt_ && !stmntList_)
    {
        /* Make new code block statement to replace the single statement with */
        auto singleStmnt    = *stmnt_;
        auto codeBlockStmnt = ASTFactory::MakeCodeBlockStmnt(singleStmnt);

        /* Set reference to statement list of the code block */
        stmntList_ = &(codeBlockStmnt->codeBlock->stmnts);

        /* Replace original single statement with code block statement */
        *stmnt_ = codeBlockStmnt;
    }
}


} // /namespace Xsc



// ================================================================================
