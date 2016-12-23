/*
 * SymbolTable.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SymbolTable.h"
#include "Exception.h"
#include "ReportHandler.h"
#include <algorithm>


namespace Xsc
{


ASTSymbolOverload::ASTSymbolOverload(const std::string& ident, AST* ast) :
    ident_{ ident }
{
    /* Add initial reference */
    refs_.push_back(ast);
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
        
        /* Is the new declaration a forward declaration? */
        auto newFuncDecl = static_cast<FunctionDecl*>(ast);
        if (newFuncDecl->IsForwardDecl())
        {
            /* Decorate new forward declaration with the function implementation (if already registered in this symbol table) */
            for (auto ref : refs_)
            {
                auto funcDecl = static_cast<FunctionDecl*>(ref);
                if (!funcDecl->IsForwardDecl() && funcDecl->EqualsSignature(*newFuncDecl))
                {
                    newFuncDecl->funcImplRef = funcDecl;
                    break;
                }
            }
            return true;
        }
        else
        {
            /* Are all previous declarations forward declarations, or are the function signatures different? */
            for (auto ref : refs_)
            {
                auto funcDecl = static_cast<FunctionDecl*>(ref);
                if (funcDecl->EqualsSignature(*newFuncDecl))
                {
                    if (funcDecl->IsForwardDecl())
                    {
                        /* Decorate forward declaration with the new function implementation */
                        funcDecl->funcImplRef = newFuncDecl;
                    }
                    else
                    {
                        /* Duplicate function implementations found */
                        return false;
                    }
                }
            }
        } // endif newFuncDecl->IsForwardDecl
    }

    /* Add AST reference to list */
    refs_.push_back(ast);

    return true;
}

AST* ASTSymbolOverload::Fetch(bool throwOnFailure)
{
    if (throwOnFailure)
    {
        if (refs_.empty())
            RuntimeErr("undefined symbol '" + ident_ + "'");
        if (refs_.size() > 1)
            RuntimeErr("symbol '" + ident_ + "' is ambiguous");
        return refs_.front();
    }
    else
        return (refs_.size() == 1 ? refs_.front() : nullptr);
}

AST* ASTSymbolOverload::FetchVar(bool throwOnFailure)
{
    if (auto ref = Fetch(throwOnFailure))
    {
        auto type = ref->Type();
        if (type == AST::Types::VarDecl || type == AST::Types::BufferDecl || type == AST::Types::SamplerDecl)
            return ref;
        if (throwOnFailure)
            RuntimeErr("identifier '" + ident_ + "' does not name a variable, buffer, or sampler");
    }
    return nullptr;
}

VarDecl* ASTSymbolOverload::FetchVarDecl(bool throwOnFailure)
{
    if (auto ref = Fetch(throwOnFailure))
    {
        if (auto varDecl = ref->As<VarDecl>())
            return varDecl;
        if (throwOnFailure)
            RuntimeErr("identifier '" + ident_ + "' does not name a variable");
    }
    return nullptr;
}

AST* ASTSymbolOverload::FetchType(bool throwOnFailure)
{
    if (auto ref = Fetch(throwOnFailure))
    {
        auto type = ref->Type();
        if (type == AST::Types::StructDecl || type == AST::Types::AliasDecl)
            return ref;
        if (throwOnFailure)
            RuntimeErr("identifier '" + ident_ + "' does not name a type");
    }
    return nullptr;
}

FunctionDecl* ASTSymbolOverload::FetchFunctionDecl(bool throwOnFailure)
{
    auto ref = Fetch(throwOnFailure);
    if (auto funcDecl = ref->As<FunctionDecl>())
        return funcDecl;
    else if (throwOnFailure)
        RuntimeErr("identifier '" + ident_ + "' does not name a function");
    return nullptr;
}

FunctionDecl* ASTSymbolOverload::FetchFunctionDecl(const std::vector<TypeDenoterPtr>& argTypeDenoters)
{
    if (refs_.empty())
        RuntimeErr("undefined symbol '" + ident_ + "'");
    if (refs_.front()->Type() != AST::Types::FunctionDecl)
        RuntimeErr("identifier '" + ident_ + "' does not name a function");

    /* Validate number of arguments for function call */
    const auto numArgs = argTypeDenoters.size();

    if (!ValidateNumArgsForFunctionDecl(numArgs))
    {
        RuntimeErr(
            "function '" + ident_ + "' does not take " + std::to_string(numArgs) + " " +
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
    {
        /* Construct descriptive string for argument type names */
        std::string argTypeNames;

        if (numArgs > 0)
        {
            for (std::size_t i = 0; i < numArgs; ++i)
            {
                argTypeNames += argTypeDenoters[i]->ToString();
                if (i + 1 < numArgs)
                    argTypeNames += ", ";
            }
        }
        else
            argTypeNames = "void";

        /* Add candidate signatures to report hints */
        if (!funcDeclCandidates.empty())
        {
            ReportHandler::HintForNextReport("candidates are:");
            for (auto funcDecl : funcDeclCandidates)
                ReportHandler::HintForNextReport("  '" + funcDecl->SignatureToString(false) + "'");
        }

        /* Throw runtime error */
        RuntimeErr(
            "ambiguous function call (" + std::to_string(funcDeclCandidates.size()) +
            " candidates) with arguments: (" + argTypeNames + ")"
        );
    }

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
    FunctionDecl& funcDecl, const std::vector<TypeDenoterPtr>& typeDens, bool implicitTypeConversion)
{
    auto numArgs = typeDens.size();
    if (numArgs >= funcDecl.NumMinArgs() && numArgs <= funcDecl.NumMaxArgs())
    {
        for (std::size_t i = 0, n = std::min(typeDens.size(), funcDecl.parameters.size()); i < n; ++i)
        {
            /* Match argument type denoter to parameter */
            if (!funcDecl.MatchParameterWithTypeDenoter(i, *typeDens[i], implicitTypeConversion))
                return false;
        }
        return true;
    }
    return false;
}


} // /namespace Xsc



// ================================================================================