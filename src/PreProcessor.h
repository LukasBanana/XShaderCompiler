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

        


        /* === Members === */

        IncludeHandler&                     includeHandler_;
        Log*                                log_                = nullptr;

        PreProcessorScanner                 scanner_;

        std::shared_ptr<std::iostream>      output_;

        std::map<std::string, std::string>  symbols_;

};


} // /namespace HTLib


#endif



// ================================================================================