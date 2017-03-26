/*
 * Converter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Converter.h"
#include "AST.h"
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
 * ======= Private: =======
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
    if (!selfParamStack_.empty())
        return selfParamStack_.pop_back();
    else
        throw std::underflow_error(R_SelfParamLevelUnderflow);
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


} // /namespace Xsc



// ================================================================================
