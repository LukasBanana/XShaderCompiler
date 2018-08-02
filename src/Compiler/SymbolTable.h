/*
 * SymbolTable.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SYMBOL_TABLE_H
#define XSC_SYMBOL_TABLE_H


#include "AST.h"
#include <map>
#include <string>
#include <stack>
#include <vector>
#include <functional>


namespace Xsc
{


// Returns the ranked distance between the two strings.
unsigned int StringDistance(const std::string& a, const std::string& b);

[[noreturn]]
void RuntimeErrNoActiveScope();

[[noreturn]]
void RuntimeErrIdentAlreadyDeclared(const std::string& ident, const AST* prevDeclAST = nullptr);

template <typename SymbolType>
AST* FetchASTFromSymbol(const SymbolType& symbol);

template <typename T>
struct GenericDefaultValue
{
    static T Get()
    {
        return nullptr;
    }
};

template <>
struct GenericDefaultValue<bool>
{
    static bool Get()
    {
        return false;
    }
};

// Common symbol table class with a single scope.
template <typename SymbolType>
class SymbolTable
{
    
    public:
        
        // Callback function when a symbol is about to be overriden. Must return true to allow a symbol override.
        using OnOverrideProc = std::function<bool(SymbolType& prevSymbol)>;

        // Callback function when a symbol is about to be released from its scope.
        using OnReleaseProc = std::function<void(const SymbolType& symbol)>;

        // Search predicate function signature.
        using SearchPredicateProc = std::function<bool(const SymbolType& symbol)>;

        SymbolTable()
        {
            OpenScope();
        }

        // Opens a new scope.
        void OpenScope()
        {
            scopeStack_.push({});
            symTableAnonymous_.push_back({});
        }

        // Closes the active scope.
        void CloseScope(const OnReleaseProc& releaseProc = nullptr)
        {
            if (!scopeStack_.empty())
            {
                /* Remove all symbols from the table which are in the current scope */
                for (const auto& ident : scopeStack_.top())
                {
                    auto it = symTable_.find(ident);
                    if (it != symTable_.end())
                    {
                        /* Callback for released symbol */
                        if (releaseProc)
                            releaseProc(it->second.top().symbol);

                        /* Remove symbol from the top most scope level */
                        it->second.pop();
                        if (it->second.empty())
                        {
                            /* Remove symbol entry completely if it's reference list is empty */
                            symTable_.erase(it);
                        }
                    }
                }

                if (releaseProc)
                {
                    /* Release all symbols from the anonymous symbol table */
                    for (const auto& sym : symTableAnonymous_.back())
                        releaseProc(sym.symbol);
                }

                /* Decrease scope level */
                scopeStack_.pop();
                symTableAnonymous_.pop_back();
            }
        }

        /*
        Registers the specified symbol in the current scope (if the identifier is not empty).
        At least one scope must be open before symbols can be registered!
        */
        bool Register(const std::string& ident, SymbolType symbol, const OnOverrideProc& overrideProc = nullptr, bool throwOnFailure = true)
        {
            /* Validate input parameters */
            if (scopeStack_.empty())
                RuntimeErrNoActiveScope();

            if (ident.empty())
            {
                /* Register symbol in anonymous symbol table */
                symTableAnonymous_.back().push_back({ symbol, ScopeLevel() });
            }
            else
            {
                /* Check if identifier was already registered in the current scope */
                auto it = symTable_.find(ident);
                if (it != symTable_.end() && !it->second.empty())
                {
                    auto& entry = it->second.top();
                    if (entry.symbol && entry.scopeLevel == ScopeLevel())
                    {
                        /* Call override procedure and pass previous symbol entry as reference */
                        if (overrideProc && overrideProc(entry.symbol))
                            return true;
                        else if (throwOnFailure)
                            RuntimeErrIdentAlreadyDeclared(ident, FetchASTFromSymbol(entry.symbol));
                        else
                            return false;
                    }
                }

                /* Register new identifier */
                symTable_[ident].push({ symbol, ScopeLevel() });
                scopeStack_.top().push_back(ident);
            }

            return true;
        }

        // Returns the symbol with the specified identifer which is in the deepest scope, or null if there is no such symbol.
        SymbolType Fetch(const std::string& ident) const
        {
            auto it = symTable_.find(ident);
            if (it != symTable_.end() && !it->second.empty())
                return it->second.top().symbol;
            else
                return GenericDefaultValue<SymbolType>::Get();
        }

        // Returns the symbol with the specified identifer which is in the current scope, or null if there is no such symbol.
        SymbolType FetchFromCurrentScope(const std::string& ident) const
        {
            auto it = symTable_.find(ident);
            if (it != symTable_.end() && !it->second.empty())
            {
                const auto& sym = it->second.top();
                if (sym.scopeLevel == ScopeLevel())
                    return sym.symbol;
            }
            return GenericDefaultValue<SymbolType>::Get();
        }

        // Returns the first symbol in the scope hierarchy for which the search predicate returns true.
        SymbolType Find(const SearchPredicateProc& searchPredicate) const
        {
            if (searchPredicate)
            {
                /* Search symbol in identifiable symbol ist */
                for (const auto& sym : symTable_)
                {
                    if (!sym.second.empty())
                    {
                        const auto& symRef = sym.second.top().symbol;
                        if (searchPredicate(symRef))
                            return symRef;
                    }
                }

                /* Search symbol in anonymous symbol list */
                if (!symTableAnonymous_.empty())
                {
                    for (auto scope = symTableAnonymous_.rbegin(); scope != symTableAnonymous_.rend(); ++scope)
                    {
                        for (const auto& sym : *scope)
                        {
                            if (searchPredicate(sym.symbol))
                                return sym.symbol;
                        }
                    }
                }
            }
            return GenericDefaultValue<SymbolType>::Get();
        }

        // Returns an identifier that is similar to the specified identifier (for suggestions of typos)
        std::string FetchSimilar(const std::string& ident) const
        {
            /* Find similar identifiers */
            const std::string* similar = nullptr;
            unsigned int dist = ~0;

            for (const auto& symbol : symTable_)
            {
                auto d = StringDistance(ident, symbol.first);
                if (d < dist)
                {
                    similar = (&symbol.first);
                    dist = d;
                }
            }

            /* Check if the distance is not too large */
            if (similar != nullptr && dist < ident.size())
                return *similar;

            /* No similarities found */
            return "";
        }

        // Returns current scope level.
        std::size_t ScopeLevel() const
        {
            return scopeStack_.size();
        }

        // Returns true if the symbol table is currently inside the global scope (i.e. scope level = 1).
        bool InsideGlobalScope() const
        {
            return (ScopeLevel() == 1);
        }

    private:
        
        struct Symbol
        {
            SymbolType  symbol;
            std::size_t scopeLevel;
        };

        // Stores the scope stack for all identifiable symbols.
        std::map<std::string, std::stack<Symbol>>   symTable_;

        // Stores the scope stack for all anonymous symbols.
        std::vector<std::vector<Symbol>>            symTableAnonymous_;

        /*
        Stores all identifiers for the current stack.
        All these identifiers will be removed from "symTable_" when a scope will be closed.
        */
        std::stack<std::vector<std::string>>        scopeStack_;

};


// AST symbol table type.
using ASTSymbolTable = SymbolTable<AST*>;

// AST symbol class for the AST, that allows overloaded symbol (for functions).
class ASTSymbolOverload
{

    public:
    
        ASTSymbolOverload(const std::string& ident, AST* ast);

        // Adds the specified AST reference to this overloaded symbol, and return true if the overload is valid.
        bool AddSymbolRef(AST* ast);

        // Fetches any AST. If there is more than one reference, an std::runtime_error is thrown.
        AST* Fetch(bool throwOnFailure = true) const;

        // Returns the VarDecl AST node.
        VarDecl* FetchVarDecl(bool throwOnFailure = true) const;

        /*
        Fetches a type declaration (StructDecl, AliasDecl).
        If there is more than one reference or the type does not fit, an std::runtime_error is thrown.
        */
        Decl* FetchType(bool throwOnFailure = true) const;

        // Returns the FunctionDecl AST node (if the function is not overloaded).
        FunctionDecl* FetchFunctionDecl(bool throwOnFailure = true) const;

        // Returns the FunctionDecl AST node for the specified argument type denoter list (used to derive the overloaded function).
        FunctionDecl* FetchFunctionDecl(const std::vector<TypeDenoterPtr>& argTypeDenoters) const;

    private:

        std::string         ident_;
        std::vector<AST*>   refs_;

};

using ASTSymbolOverloadPtr = std::shared_ptr<ASTSymbolOverload>;

// AST symbol table type for ovloading.
using ASTSymbolOverloadTable = SymbolTable<ASTSymbolOverloadPtr>;


// Template to fetch AST node from a generic symbol type.
template <typename SymbolType>
AST* FetchASTFromSymbol(const SymbolType& symbol)
{
    return nullptr;
}

template <>
inline AST* FetchASTFromSymbol<ASTSymbolOverloadPtr>(const ASTSymbolOverloadPtr& symbol)
{
    return symbol->Fetch(false);
}


} // /namespace Xsc


#endif



// ================================================================================
