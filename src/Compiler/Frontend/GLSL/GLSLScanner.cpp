/*
 * GLSLScanner.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLScanner.h"
#include "GLSLKeywords.h"
#include "ReportIdents.h"


namespace Xsc
{


GLSLScanner::GLSLScanner(Log* log) :
    SLScanner { log }
{
}


/*
 * ======= Private: =======
 */

TokenPtr GLSLScanner::ScanIdentifierOrKeyword(std::string&& spell)
{
    /* Scan reserved words */
    auto it = GLSLKeywords().find(spell);
    if (it != GLSLKeywords().end())
    {
        if (it->second == Token::Types::Reserved)
            Error(R_KeywordReservedForFutureUse(spell));
        else if (it->second == Token::Types::Unsupported)
            Error(R_KeywordNotSupportedYet(spell));
        else
            return Make(it->second, spell);
    }

    /* Return as identifier */
    return Make(Tokens::Ident, spell);
}


} // /namespace Xsc



// ================================================================================