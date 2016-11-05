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
#include "SourceCode.h"
#include <iostream>
#include <map>


namespace HTLib
{


// Pre-processor to substitute macros and include directives.
class PreProcessor
{
    
    public:
        
        PreProcessor(IncludeHandler& includeHandler, Log* log = nullptr);

        std::shared_ptr<std::iostream> Process(const std::shared_ptr<SourceCode>& input);

    private:
        
        /* === Functions === */

        void Error(const std::string& msg);
        void ErrorUnexpected();
        void ErrorUnexpected(const std::string& hint);

        TokenPtr Accept(const Token::Types type);
        TokenPtr AcceptIt();

        void ParseProgram();

        void ParesComment();
        void ParseMisc();
        
        void ParseDirective();
        void ParseDirectiveDefine();
        void ParseDirectiveUndef();
        void ParseDirectiveInclude();
        void ParseDirectiveIf();
        void ParseDirectiveIfdef();
        void ParseDirectiveIfndef();
        void ParseDirectivePragma();

        // Returns the type of the next token.
        inline Token::Types Type() const
        {
            return tkn_->Type();
        }

        // Returns true if the next token is from the specified type.
        inline bool Is(const Token::Types type) const
        {
            return (Type() == type);
        }

        /* === Members === */

        IncludeHandler&                     includeHandler_;
        Log*                                log_                = nullptr;

        PreProcessorScanner                 scanner_;
        TokenPtr                            tkn_;

        std::shared_ptr<std::stringstream>  output_;

        //std::map<std::string, std::string>  definedSymbols_;

};


} // /namespace HTLib


#endif



// ================================================================================