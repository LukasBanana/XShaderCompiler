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
#include <stack>
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

        void PushIfBlock(bool active = false);
        void PopIfBlock();

        // Returns true if the if-block on top of the stack is active or if the stack is empty.
        bool IsTopIfBlockActive() const;

        // Ignores all tokens until the next line is reached.
        void SkipToNextLine();

        /* === Parse functions === */

        void ParseProgram();

        void ParesComment();
        void ParseIdent();
        void ParseMisc();
        
        void ParseDirective();
        void ParseAnyIfDirectiveAndSkipValidation();
        void ParseDirectiveDefine();
        void ParseDirectiveUndef();
        void ParseDirectiveInclude();
        void ParseDirectiveIf(bool skipEvaluation = false);
        void ParseDirectiveIfdef(bool skipEvaluation = false);
        void ParseDirectiveIfndef(bool skipEvaluation = false);
        void ParseDirectiveElif(bool skipEvaluation = false);
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

        /*
        Stack to store the info which if-block in the hierarchy is active.
        Once an if-block is inactive, all subsequent if-blocks are inactive, too.
        */
        std::stack<bool>                        activeIfBlockStack_;

};


} // /namespace HTLib


#endif



// ================================================================================