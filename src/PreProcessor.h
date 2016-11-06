/*
 * PreProcessor.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_PRE_PROCESSOR_H
#define HTLIB_PRE_PROCESSOR_H


#include "HT/Translator.h"
#include "HT/Log.h"
#include "PreProcessorScanner.h"
#include "Parser.h"
#include "SourceCode.h"
#include <iostream>
#include <map>
#include <set>


namespace HTLib
{


/*
Pre-processor to substitute macros and include directives.
The preprocessor works on something similar to a Concrete Syntax Tree (CST) rather than an Abstract Syntax Tree (AST).
This is because the output is not an intermediate representation but rather concrete source code.
Therefore, all white spaces and new-line characters must not be ignored.
All other parsers and analyzers only work on an AST.
*/
class PreProcessor : public Parser
{
    
    public:
        
        PreProcessor(IncludeHandler& includeHandler, Log* log = nullptr);

        std::shared_ptr<std::iostream> Process(const std::shared_ptr<SourceCode>& input);

    private:
        
        /* === Structures === */

        using TokenString = std::vector<TokenPtr>;

        struct DefinedSymbol
        {
            std::vector<std::string>    parameters;
            TokenString                 tokenString;
        };

        using DefinedSymbolPtr = std::shared_ptr<DefinedSymbol>;

        /* === Functions === */

        ScannerPtr MakeScanner() override;

        DefinedSymbolPtr MakeSymbol(const std::string& ident);

        // Returns true if the specified symbol is defined.
        bool IsDefined(const std::string& ident) const;

        // Compares the two token strings (ignores white-space, new-line, and comment tokens).
        bool CompareTokenStrings(const TokenString& lhs, const TokenString& rhs) const;

        void OutputTokenString(const TokenString& tokenString);

        /* === Parse functions === */

        void ParseProgram();

        void ParesComment();
        void ParseIdent();
        void ParseMisc();
        
        void ParseDirective();
        void ParseDirectiveDefine();
        void ParseDirectiveUndef();
        void ParseDirectiveInclude();
        void ParseDirectiveIf();
        void ParseDirectiveIfdef();
        void ParseDirectiveIfndef();
        void ParseDirectiveElif();
        void ParseDirectiveElse();
        void ParseDirectiveEndif();
        void ParseDirectivePragma();
        void ParseDirectiveLine();
        void ParseDirectiveError(const TokenPtr& directiveToken);

        TokenString ParseTokenString();

        /* === Members === */

        IncludeHandler&                         includeHandler_;

        PreProcessorScanner                     scanner_;

        std::shared_ptr<std::stringstream>      output_;

        std::map<std::string, DefinedSymbolPtr> definedSymbols_;

        std::set<std::string>                   onceIncluded_;

};


} // /namespace HTLib


#endif



// ================================================================================