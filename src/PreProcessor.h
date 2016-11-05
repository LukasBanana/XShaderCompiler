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


namespace HTLib
{


// Pre-processor to substitute macros and include directives.
class PreProcessor : public Parser
{
    
    public:
        
        PreProcessor(IncludeHandler& includeHandler, Log* log = nullptr);

        std::shared_ptr<std::iostream> Process(const std::shared_ptr<SourceCode>& input);

    private:
        
        /* === Structures === */

        struct DefinedSymbol
        {
            std::vector<std::string>    parameters;
            std::string                 value;
        };

        using DefinedSymbolPtr = std::shared_ptr<DefinedSymbol>;

        /* === Functions === */

        Scanner& GetScanner() override;

        void Warning(const std::string& msg);

        DefinedSymbolPtr MakeSymbol(const std::string& ident);

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
        void ParseDirectivePragma();

        /* === Members === */

        IncludeHandler&                         includeHandler_;
        Log*                                    log_                = nullptr;

        PreProcessorScanner                     scanner_;

        std::shared_ptr<std::stringstream>      output_;

        std::map<std::string, DefinedSymbolPtr> definedSymbols_;

};


} // /namespace HTLib


#endif



// ================================================================================