/*
 * SymbolTable.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
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
        
        // Override symbol callback procedure. Must return true to allow a symbol override.
        using OnOverrideProc = std::function<bool(SymbolType& prevSymbol)>;

        SymbolTable()
        {
            OpenScope();
        }

        // Opens a new scope.
        void OpenScope()
        {
            scopeStack_.push({});
        }

        // Closes the active scope.
        void CloseScope()
        {
            if (!scopeStack_.empty())
            {
                /* Remove all symbols from the table which are in the current scope */
                for (const auto& ident : scopeStack_.top())
                {
                    auto it = symTable_.find(ident);
                    if (it != symTable_.end())
                    {
                        /* Remove symbol from the top most scope level */
                        it->second.pop();
                        if (it->second.empty())
                        {
                            /* Remove symbol entry completely if it's reference list is empty */
                            symTable_.erase(it);
                        }
                    }
                }
                scopeStack_.pop();
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
                throw std::runtime_error("no active scope to register symbol");
            if (ident.empty())
                return false;

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
                        throw std::runtime_error("identifier '" + ident + "' already declared in this scope");
                    else
                        return false;
                }
            }

            /* Register new identifier */
            symTable_[ident].push({ symbol, ScopeLevel() });
            scopeStack_.top().push_back(ident);

            return true;
        }

        /*
        Returns the symbol with the specified identifer which is in
        the deepest scope, or null if there is no such symbol.
        */
        SymbolType Fetch(const std::string& ident) const
        {
            auto it = symTable_.find(ident);
            if (it != symTable_.end() && !it->second.empty())
                return it->second.top().symbol;
            else
                return GenericDefaultValue<SymbolType>::Get();
        }

        // Returns current scope level.
        std::size_t ScopeLevel() const
        {
            return scopeStack_.size();
        }

    private:
        
        struct Symbol
        {
            SymbolType  symbol;
            std::size_t scopeLevel;
        };

        // Stores the scope stack for all identifiers.
        std::map<std::string, std::stack<Symbol>>   symTable_;

        /*
        Stores all identifiers for the current stack.
        All these identifiers will be removed from "symTable_" when a scope will be closed.
        */
        std::stack<std::vector<std::string>>        scopeStack_;

};


// AST symbol table type.
using ASTSymbolTable = SymbolTable<AST*>;


class ASTSymbolOverload
{

    public:
    
        ASTSymbolOverload(const std::string& ident, AST* ast);

        // Adds the specified AST reference to this overloaded symbol, and return true if the overload is valid.
        bool AddSymbolRef(AST* ast);

        // Fetches any AST. If there is more than one reference, an std::runtime_error is thrown.
        AST* Fetch(bool throwOnFailure = true);

        /*
        Fetches a variable declaration (VarDecl, BufferDecl, SamplerDecl).
        If there is more than one reference or the type does not fit, an std::runtime_error is thrown.
        */
        AST* FetchVar(bool throwOnFailure = true);

        // Returns the VarDecl AST node.
        VarDecl* FetchVarDecl(bool throwOnFailure = true);

        /*
        Fetches a type declaration (StructDecl, AliasDecl).
        If there is more than one reference or the type does not fit, an std::runtime_error is thrown.
        */
        AST* FetchType(bool throwOnFailure = true);

        // Returns the FunctionDecl AST node (if the function is not overloaded).
        FunctionDecl* FetchFunctionDecl(bool throwOnFailure = true);

        // Returns the FunctionDecl AST node for the specified argument type denoter list (used to derive the overloaded function).
        FunctionDecl* FetchFunctionDecl(const std::vector<TypeDenoterPtr>& argTypeDenoters);

    private:

        bool ValidateNumArgsForFunctionDecl(std::size_t numArgs);

        bool MatchFunctionDeclWithArgs(
            FunctionDecl& funcDecl,
            const std::vector<TypeDenoterPtr>& typeDens,
            bool implicitTypeConversion
        );

        std::string         ident_;
        std::vector<AST*>   refs_;

};

using ASTSymbolOverloadPtr = std::shared_ptr<ASTSymbolOverload>;

// AST symbol table type for ovloading.
using ASTSymbolOverloadTable = SymbolTable<ASTSymbolOverloadPtr>;


} // /namespace Xsc


#endif



// ================================================================================