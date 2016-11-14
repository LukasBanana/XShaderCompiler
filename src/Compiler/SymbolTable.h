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


// Common symbol table class with a single scope.
template <typename SymbolType>
class SymbolTable
{
    
    public:
        
        // Override symbol callback procedure. Must return true to allow a symbol override.
        using OnOverrideProc = std::function<bool (SymbolType symbol)>;

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
        void Register(const std::string& ident, SymbolType symbol, const OnOverrideProc& overrideProc = nullptr)
        {
            /* Validate input parameters */
            if (scopeStack_.empty())
                throw std::runtime_error("no active scope to register symbol");
            if (ident.empty())
                return;

            /* Check if identifier was already registered in the current scope */
            auto it = symTable_.find(ident);
            if (it != symTable_.end() && !it->second.empty())
            {
                auto& entry = it->second.top();
                if (entry.symbol && entry.scopeLevel == ScopeLevel())
                {
                    if (overrideProc && overrideProc(entry.symbol))
                    {
                        /* Override symbol in this scope */
                        entry.symbol = symbol;
                        return;
                    }
                    else
                        throw std::runtime_error("identifier '" + ident + "' already declared in this scope");
                }
            }

            /* Register new identifier */
            symTable_[ident].push({ symbol, ScopeLevel() });
            scopeStack_.top().push_back(ident);
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
            return nullptr;
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


} // /namespace Xsc


#endif



// ================================================================================