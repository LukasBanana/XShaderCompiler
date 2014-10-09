/*
 * HLSLKeywords.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_KEYWORDS_H__
#define __HT_KEYWORDS_H__


#include "Token.h"

#include <map>
#include <string>


typedef std::map<std::string, Token::Types> KeywordMapType;

//! Returns the keywords map (which is an exception for identifiers).
const KeywordMapType& HLSLKeywords();


#endif



// ================================================================================