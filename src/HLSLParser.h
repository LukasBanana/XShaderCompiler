/*
 * HLSLParser.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_PARSER_H__
#define __HT_PARSER_H__


#include "HLSLScanner.h"


//! Syntax parser class.
class HLSLParser
{
    
    public:
        
        HLSLParser(Logger* log = nullptr);

        bool ParseSource(const std::shared_ptr<SourceCode>& source);

    private:
        
        HLSLScanner scanner_;

};


#endif



// ================================================================================