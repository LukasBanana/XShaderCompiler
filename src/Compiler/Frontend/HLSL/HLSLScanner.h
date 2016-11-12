/*
 * HLSLScanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
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
        
        HLSLScanner(Log* log = nullptr);

        // Scanns the next token.
        TokenPtr Next() override;

    private:
        
        /* === Functions === */

        TokenPtr ScanToken() override;

        TokenPtr ScanIdentifier();
        TokenPtr ScanAssignShiftRelationOp(const char Chr);
        TokenPtr ScanPlusOp();
        TokenPtr ScanMinusOp();

};


} // /namespace Xsc


#endif



// ================================================================================