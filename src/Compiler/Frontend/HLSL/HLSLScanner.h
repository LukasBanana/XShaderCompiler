/*
 * HLSLScanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_SCANNER_H
#define XSC_HLSL_SCANNER_H


#include "Scanner.h"


namespace Xsc
{


// HLSL token scanner.
class HLSLScanner : public Scanner
{
    
    public:
        
        HLSLScanner(bool enableCgKeywords, Log* log = nullptr);

        // Scanns the next token.
        TokenPtr Next() override;

    private:
        
        /* === Functions === */

        TokenPtr ScanToken() override;

        TokenPtr ScanDirective();
        TokenPtr ScanIdentifier();
        TokenPtr ScanAssignShiftRelationOp(const char Chr);
        TokenPtr ScanPlusOp();
        TokenPtr ScanMinusOp();

        /* === Members === */

        bool enableCgKeywords_ = false;

};


} // /namespace Xsc


#endif



// ================================================================================