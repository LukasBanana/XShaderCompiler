/*
 * HLSLScanner.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLScanner.h"
#include "HLSLKeywords.h"
#include "ReportIdents.h"


namespace Xsc
{


HLSLScanner::HLSLScanner(bool enableCgKeywords, Log* log) :
    SLScanner         { log              },
    enableCgKeywords_ { enableCgKeywords }
{
    FeatureSupport features;
    {
        features.acceptInfConst = true;
    }
    SetFeatureSupport(features);
}


/*
 * ======= Private: =======
 */

TokenPtr HLSLScanner::ScanIdentifierOrKeyword(std::string&& spell)
{
    /* Scan reserved words */
    auto it = HLSLKeywords().find(spell);
    if (it != HLSLKeywords().end())
    {
        if (it->second == Token::Types::Reserved)
            Error(R_KeywordReservedForFutureUse(spell));
        else if (it->second == Token::Types::Unsupported)
            Error(R_KeywordNotSupportedYet(spell));
        else
            return Make(it->second, spell);
    }

    /* Scan reserved extended words (if Cg keywords are enabled) */
    if (enableCgKeywords_)
    {
        auto it = HLSLKeywordsExtCg().find(spell);
        if (it != HLSLKeywordsExtCg().end())
            return Make(it->second, spell);
    }

    /* Return as identifier */
    return Make(Tokens::Ident, spell);
}


} // /namespace Xsc



// ================================================================================