/*
 * HLSLKeywords.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_KEYWORDS_H
#define XSC_KEYWORDS_H


#include "Token.h"

#include <map>
#include <string>


namespace Xsc
{


using KeywordMapType = std::map<std::string, Token::Types>;

// Returns the keywords map (which is an exception for identifiers).
const KeywordMapType& HLSLKeywords();


} // /namespace Xsc


#endif



// ================================================================================