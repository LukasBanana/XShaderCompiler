/*
 * HLSLScanner.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_HLSL_SCANNER_H
#define HTLIB_HLSL_SCANNER_H


#include "Scanner.h"


namespace HTLib
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

        TokenPtr ScanDirective();
        TokenPtr ScanIdentifier();
        TokenPtr ScanAssignShiftRelationOp(const char Chr);
        TokenPtr ScanPlusOp();
        TokenPtr ScanMinusOp();

};


} // /namespace HTLib


#endif



// ================================================================================