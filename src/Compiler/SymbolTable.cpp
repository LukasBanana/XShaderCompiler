/*
 * SymbolTable.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SymbolTable.h"
#include <algorithm>


namespace Xsc
{


ASTSymbolOverload::ASTSymbolOverload(const std::string& ident) :
    ident_{ ident }
{
}

bool ASTSymbolOverload::AddSymbolRef(AST* ast)
{
    if (!ast)
        return false;

    /* Is this the first symbol reference? */
    if (!refs_.empty())
    {
        /* Is this a redefinition of another AST type? */
        if (refs_.front()->Type() != ast->Type())
            return false;

        /* Can this type of symbol be overloaded? */
        if (ast->Type() != AST::Types::FunctionDecl)
            return false;
    }

    /* Add AST reference to list */
    refs_.push_back(ast);

    return true;
}

AST* ASTSymbolOverload::Fetch()
{
    if (refs_.empty())
        throw std::runtime_error("undefined symbol '" + ident_ + "'");
    if (refs_.size() > 1)
        throw std::runtime_error("symbol '" + ident_ + "' is ambiguous");
    return refs_.front();
}

AST* ASTSymbolOverload::FetchVar()
{
    auto ref = Fetch();
    auto type = ref->Type();
    if (type != AST::Types::VarDecl && type != AST::Types::TextureDecl && type != AST::Types::SamplerDecl)
        throw std::runtime_error("identifier '" + ident_ + "' does not name a type");
    return ref;
}

AST* ASTSymbolOverload::FetchType()
{
    auto ref = Fetch();
    auto type = ref->Type();
    if (type != AST::Types::StructDecl && type != AST::Types::AliasDecl)
        throw std::runtime_error("identifier '" + ident_ + "' does not name a type");
    return ref;
}

FunctionDecl* ASTSymbolOverload::FetchFunctionDecl(const std::vector<const TypeDenoter*>& argTypeDenoters)
{
    if (refs_.empty())
        throw std::runtime_error("undefined symbol '" + ident_ + "'");
    if (refs_.front()->Type() != AST::Types::FunctionDecl)
        throw std::runtime_error("identifier '" + ident_ + "' does not name a function");

    /* Validate number of arguments for function call */
    const auto numArgs = argTypeDenoters.size();

    if (!ValidateNumArgsForFunctionDecl(numArgs))
    {
        throw std::runtime_error(
            "function does not take " + std::to_string(numArgs) + " " +
            std::string(numArgs == 1 ? "parameter" : "parameters")
        );
    }

    /* Find best fit with explicit argument types */
    std::vector<FunctionDecl*> funcDeclCandidates;

    for (auto ref : refs_)
    {
        auto funcDecl = static_cast<FunctionDecl*>(ref);
        if (MatchFunctionDeclWithArgs(*funcDecl, argTypeDenoters, false))
            funcDeclCandidates.push_back(funcDecl);
    }

    /* Nothing found? -> find first fit with implicit argument types */
    if (funcDeclCandidates.empty())
    {
        for (auto ref : refs_)
        {
            auto funcDecl = static_cast<FunctionDecl*>(ref);
            if (MatchFunctionDeclWithArgs(*funcDecl, argTypeDenoters, true))
                funcDeclCandidates.push_back(funcDecl);
        }
    }

    /* Check for ambiguous function call */
    if (funcDeclCandidates.size() != 1)
        throw std::runtime_error("ambiguous function call");

    return funcDeclCandidates.front();
}


/*
 * ======= Private: =======
 */

bool ASTSymbolOverload::ValidateNumArgsForFunctionDecl(std::size_t numArgs)
{
    for (auto ref : refs_)
    {
        /* Are the number of arguments sufficient? */
        auto funcDecl = static_cast<FunctionDecl*>(ref);
        if (numArgs >= funcDecl->NumMinArgs() && numArgs <= funcDecl->NumMaxArgs())
            return true;
    }
    return false;
}

bool ASTSymbolOverload::MatchFunctionDeclWithArgs(
    FunctionDecl& funcDecl, const std::vector<const TypeDenoter*>& typeDens, bool implicitTypeConversion)
{
    auto numArgs = typeDens.size();
    if (numArgs >= funcDecl.NumMinArgs() && numArgs <= funcDecl.NumMaxArgs())
    {
        for (std::size_t i = 0; i < funcDecl.parameters.size(); ++i)
        {
            /* Get type denoters to compare */
            auto paramTypeDen = funcDecl.parameters[i]->varType->typeDenoter.get();
            auto argTypeDen = typeDens[i];

            /* Check for explicit compatability: are they equal? */
            if (!argTypeDen->Equals(*paramTypeDen))
            {
                if (implicitTypeConversion)
                {
                    /* Check for implicit compatability: is it castable? */
                    if (!argTypeDen->IsCastableTo(*paramTypeDen))
                        return false;
                }
                else
                    return false;
            }
        }
    }
    return false;
}


} // /namespace Xsc



// ================================================================================