/*
 * PreProcessorScanner.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_PRE_PROCESSOR_SCANNER_H
#define XSC_PRE_PROCESSOR_SCANNER_H


#include "Scanner.h"


namespace Xsc
{


// Pre-processor token scanner.
class PreProcessorScanner : public Scanner
{
    
    public:
        
        PreProcessorScanner(Log* log = nullptr);

        TokenPtr Next() override;

    private:
        
        /* === Functions === */

        TokenPtr ScanToken() override;

        TokenPtr ScanDirective();
        TokenPtr ScanIdentifier();

};


} // /namespace Xsc


#endif



// ================================================================================