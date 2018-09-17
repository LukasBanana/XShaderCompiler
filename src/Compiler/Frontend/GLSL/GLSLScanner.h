/*
 * GLSLScanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_SCANNER_H
#define XSC_GLSL_SCANNER_H


#include "SLScanner.h"


namespace Xsc
{


// GLSL token scanner.
class GLSLScanner : public SLScanner
{

    public:

        GLSLScanner(Log* log = nullptr);

    private:

        TokenPtr ScanIdentifierOrKeyword(std::string&& spell) override;

};


} // /namespace Xsc


#endif



// ================================================================================