/*
 * HLSLScanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_SCANNER_H
#define XSC_HLSL_SCANNER_H


#include "SLScanner.h"


namespace Xsc
{


// HLSL token scanner.
class HLSLScanner : public SLScanner
{
    
    public:
        
        HLSLScanner(bool enableCgKeywords, Log* log = nullptr);

    private:

        TokenPtr ScanIdentifierOrKeyword(std::string&& spell) override;

        /* === Members === */

        bool enableCgKeywords_ = false;

};


} // /namespace Xsc


#endif



// ================================================================================