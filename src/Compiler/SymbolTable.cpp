/*
 * SymbolTable.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SymbolTable.h"
#include "Exception.h"
#include "ReportHandler.h"
#include "ReportIdents.h"
#include <algorithm>
#include <cctype>


namespace Xsc
{


/*
 * Global (and internal) functions
 */

static unsigned int StringDistancePrimary(const std::string& lhs, const std::string& rhs, unsigned int shiftOnUneq)
{
    static const unsigned int diffUneqCaseEq    = 1;
    static const unsigned int diffUneq          = 2;

    /* Check for case sensitive differences */
    unsigned int diff = 0, sim = 0;
    std::size_t shift = 0;

    for (std::size_t i = 0; (i + shift < lhs.size() && i < rhs.size()); ++i)
    {
        auto a = lhs[i + shift];
        auto b = rhs[i];

        if (a == b)
            sim += diffUneq;
        else
        {
            if (std::toupper(a) == std::toupper(b))
            {
                diff += diffUneqCaseEq;
                sim += diffUneqCaseEq;
            }
            else
            {
                diff += diffUneq;
                if (shiftOnUneq > 0)
                {
                    --shiftOnUneq;
                    ++shift;
                }
            }
        }
    }

    return (diff >= sim ? ~0 : diff);
}

unsigned int StringDistance(const std::string& a, const std::string& b)
{
    static const unsigned int maxDist    = ~0;
    static const unsigned int maxLenDiff = 3;
    static const unsigned int maxShift   = 2;

    if (a == b)
        return 0;

    unsigned int dist = maxDist;

    if ( ( a.size() == b.size() ) ||
         ( a.size() > b.size() && a.size() <= b.size() + maxLenDiff ) ||
         ( b.size() > a.size() && b.size() <= a.size() + maxLenDiff ) )
    {
        for (unsigned int shift = 0; shift <= maxShift; ++shift)
        {
            dist = std::min(
                {
                    dist,
                    StringDistancePrimary(a, b, shift),
                    StringDistancePrimary(b, a, shift)
                }
            );
        }
    }

    return dist;
}

[[noreturn]]
void RuntimeErrNoActiveScope()
{
    RuntimeErr(R_NoActiveScopeToRegisterSymbol);
}

[[noreturn]]
void RuntimeErrIdentAlreadyDeclared(const std::string& ident)
{
    RuntimeErr(R_IdentAlreadyDeclared(ident));
}


/*
 * ASTSymbolOverload class
 */

ASTSymbolOverload::ASTSymbolOverload(const std::string& ident, AST* ast) :
    ident_ { ident }
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
                    newFuncDecl->SetFuncImplRef(funcDecl);
                    break;
                }
            }
            return true;
        }
        else
        {
            /* Are all previous declarations forward declarations, or are the function signatures different? */
            for (auto& ref : refs_)
            {
                auto funcDecl = static_cast<FunctionDecl*>(ref);
                if (funcDecl->EqualsSignature(*newFuncDecl))
                {
                    if (funcDecl->IsForwardDecl())
                    {
                        /* Decorate forward declaration with the new function implementation */
                        funcDecl->SetFuncImplRef(newFuncDecl);

                        /* Replace reference with the new function declaration */
                        ref = newFuncDecl;
                        return true;
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

AST* ASTSymbolOverload::Fetch(bool throwOnFailure) const
{
    if (throwOnFailure)
    {
        if (refs_.empty())
            RuntimeErr(R_UndefinedSymbol(ident_));
        if (refs_.size() > 1)
            RuntimeErr(R_AmbiguousSymbol(ident_));
        return refs_.front();
    }
    else
        return (refs_.size() == 1 ? refs_.front() : nullptr);
}

VarDecl* ASTSymbolOverload::FetchVarDecl(bool throwOnFailure) const
{
    if (auto ref = Fetch(throwOnFailure))
    {
        if (auto varDecl = ref->As<VarDecl>())
            return varDecl;
        if (throwOnFailure)
            RuntimeErr(R_IdentIsNotVar(ident_));
    }
    return nullptr;
}

Decl* ASTSymbolOverload::FetchType(bool throwOnFailure) const
{
    if (auto ref = Fetch(throwOnFailure))
    {
        auto type = ref->Type();
        if (type == AST::Types::StructDecl || type == AST::Types::AliasDecl)
            return static_cast<Decl*>(ref);
        if (throwOnFailure)
            RuntimeErr(R_IdentIsNotType(ident_));
    }
    return nullptr;
}

FunctionDecl* ASTSymbolOverload::FetchFunctionDecl(bool throwOnFailure) const
{
    auto ref = Fetch(throwOnFailure);
    if (auto funcDecl = ref->As<FunctionDecl>())
        return funcDecl;
    else if (throwOnFailure)
        RuntimeErr(R_IdentIsNotFunc(ident_));
    return nullptr;
}

FunctionDecl* ASTSymbolOverload::FetchFunctionDecl(const std::vector<TypeDenoterPtr>& argTypeDenoters) const
{
    if (refs_.empty())
        RuntimeErr(R_UndefinedSymbol(ident_));
    if (refs_.front()->Type() != AST::Types::FunctionDecl)
        RuntimeErr(R_IdentIsNotFunc(ident_));

    /* Convert symbol references to function declaration pointers */
    std::vector<FunctionDecl*> funcDeclList;
    funcDeclList.reserve(refs_.size());

    for (auto ref : refs_)
    {
        if (auto funcDecl = ref->As<FunctionDecl>())
            funcDeclList.push_back(funcDecl);
        else
            RuntimeErr(R_AmbiguousSymbol(ident_));
    }

    /* Fetch function declaration from list */
    return FunctionDecl::FetchFunctionDeclFromList(funcDeclList, ident_, argTypeDenoters);
}


} // /namespace Xsc



// ================================================================================