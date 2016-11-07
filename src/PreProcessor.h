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
#include "TokenString.h"
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
Therefore, all white spaces and new-line characters must NOT be ignored.
All other parsers and analyzers only work on an AST.
*/
class PreProcessor : public Parser
{
    
    public:
        
        PreProcessor(IncludeHandler& includeHandler, Log* log = nullptr);

        std::shared_ptr<std::iostream> Process(const SourceCodePtr& input);

    private:
        
        /* === Structures === */

        struct Macro
        {
            std::vector<std::string>    parameters;
            TokenPtrString              tokenString;
        };

        struct IfBlock
        {
            TokenPtr        directiveToken;
            SourceCodePtr   directiveSource;
            bool            active         = true;
            bool            expectEndif    = false;
        };

        using MacroPtr = std::shared_ptr<Macro>;

        /* === Functions === */

        ScannerPtr MakeScanner() override;

        // Defines a new macro with the specified identifier.
        MacroPtr MakeMacro(const std::string& ident);

        // Returns true if the specified symbol is defined.
        bool IsDefined(const std::string& ident) const;

        void PushIfBlock(const TokenPtr& directiveToken, bool active = false, bool expectEndif = false);
        void PopIfBlock();

        // Returns the if-block state from the top of the stack. If the stack is empty, the default state is returned.
        IfBlock TopIfBlock() const;

        // Ignores all tokens until the next line is reached.
        void SkipToNextLine();

        /* === Parse functions === */

        void ParseProgram();

        void ParesComment();
        void ParseIdent();
        void ParseIdentArgumentsForMacro(const Macro& macro);
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

        TokenPtrString ParseDirectiveTokenString();

        /* === Members === */

        IncludeHandler&                     includeHandler_;

        std::shared_ptr<std::stringstream>  output_;

        std::map<std::string, MacroPtr>     macros_;
        std::set<std::string>               onceIncluded_;

        /*
        Stack to store the info which if-block in the hierarchy is active.
        Once an if-block is inactive, all subsequent if-blocks are inactive, too.
        */
        std::stack<IfBlock>                 ifBlockStack_;

};


} // /namespace HTLib


#endif



// ================================================================================