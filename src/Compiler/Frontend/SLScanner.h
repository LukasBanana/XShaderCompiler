/*
 * SLScanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SL_SCANNER_H
#define XSC_SL_SCANNER_H


#include "Scanner.h"


namespace Xsc
{


// Common shading-language token scanner.
class SLScanner : public Scanner
{
    
    public:
        
        SLScanner(Log* log = nullptr);

        // Scanns the next token.
        TokenPtr Next() override;

    protected:

        virtual TokenPtr ScanIdentifierOrKeyword(std::string&& spell) = 0;

    private:
        
        /* === Functions === */

        TokenPtr ScanToken() override;

        TokenPtr ScanDirective();
        TokenPtr ScanIdentifier();
        TokenPtr ScanAssignShiftRelationOp(const char Chr);
        TokenPtr ScanPlusOp();
        TokenPtr ScanMinusOp();

};


} // /namespace Xsc


#endif



// ================================================================================