/*
 * SLScanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
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

        struct FeatureSupport
        {
            bool acceptInfConst = false;
        };

    protected:

        virtual TokenPtr ScanIdentifierOrKeyword(std::string&& spell) = 0;

        // Sets the language features this scanner supports.
        inline void SetFeatureSupport(const FeatureSupport& features)
        {
            features_ = features;
        }

    private:

        TokenPtr ScanToken() override;

        TokenPtr ScanDirective();
        TokenPtr ScanIdentifier();
        TokenPtr ScanAssignShiftRelationOp(const char Chr);
        TokenPtr ScanPlusOp();
        TokenPtr ScanMinusOp();

    private:

        FeatureSupport features_;

};


} // /namespace Xsc


#endif



// ================================================================================